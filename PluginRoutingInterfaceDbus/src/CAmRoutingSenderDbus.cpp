/**
 *  Copyright (c) 2012 BMW
 *  Copyright (c) copyright 2011-2012 Aricent® Group  and its licensors
 *
 *  \author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
 *  \author Sampreeth Ramavana
 *
 *  \copyright
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction,
 *  including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 *  subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 *  THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *  For further information see http://www.genivi.org/.
 */

#include "CAmRoutingSenderDbus.h"
#include <cassert>
#include <map>
#include "CAmDbusSend.h"
#include "shared/CAmDltWrapper.h"
#include "shared/CAmDbusWrapper.h"

namespace am
{
DLT_DECLARE_CONTEXT(routingDbus)

extern "C" IAmRoutingSend* PluginRoutingInterfaceDbusFactory()
{
    CAmDltWrapper::instance()->registerContext(routingDbus, "DRS", "DBus Plugin");
    return (new CAmRoutingSenderDbus());
}

extern "C" void destroyRoutingPluginInterfaceDbus(IAmRoutingSend* routingSendInterface)
{
    delete routingSendInterface;
}

CAmRoutingSenderDbus::CAmRoutingSenderDbus() :
        mpCAmDBusWrapper(), //
        mpIAmRoutingReceive(), //
        mpDBusConnection(), //
        mCAmRoutingDBusMessageHandler(), //
        mIAmRoutingReceiverShadowDbus(this)
{
    log(&routingDbus, DLT_LOG_INFO, "RoutingSender constructed");
}

CAmRoutingSenderDbus::~CAmRoutingSenderDbus()
{
    log(&routingDbus, DLT_LOG_INFO, "RoutingSender destructed");
    CAmDltWrapper::instance()->unregisterContext(routingDbus);
}

am_Error_e CAmRoutingSenderDbus::startupInterface(IAmRoutingReceive* pIAmRoutingReceive)
{
    log(&routingDbus, DLT_LOG_INFO, "startupInterface called");
    mpIAmRoutingReceive = pIAmRoutingReceive;
    mIAmRoutingReceiverShadowDbus.setRoutingReceiver(mpIAmRoutingReceive);
    mpIAmRoutingReceive->getDBusConnectionWrapper(mpCAmDBusWrapper);
    assert(mpCAmDBusWrapper!=NULL);
    mpCAmDBusWrapper->getDBusConnection(mpDBusConnection);
    assert(mpDBusConnection!=NULL);
    mCAmRoutingDBusMessageHandler.setDBusConnection(mpDBusConnection);
    return (E_OK);
}

void CAmRoutingSenderDbus::getInterfaceVersion(std::string & version) const
{
    version = RoutingSendVersion;
}

void CAmRoutingSenderDbus::setRoutingReady(const uint16_t handle)
{
    log(&routingDbus, DLT_LOG_INFO, "sending routingReady signal");
    mCAmRoutingDBusMessageHandler.initSignal(std::string(ROUTING_NODE), "setRoutingReady");
    mCAmRoutingDBusMessageHandler.sendMessage();
    mIAmRoutingReceiverShadowDbus.gotReady(mMapDomains.size(),handle);
}

void CAmRoutingSenderDbus::setRoutingRundown(const uint16_t handle)
{
    mCAmRoutingDBusMessageHandler.initSignal(std::string(ROUTING_NODE), "setRoutingRundown");
    mCAmRoutingDBusMessageHandler.sendMessage();
    mIAmRoutingReceiverShadowDbus.gotRundown(mMapDomains.size(),handle);
}

am_Error_e CAmRoutingSenderDbus::asyncAbort(const am_Handle_s handle)
{
    log(&routingDbus, DLT_LOG_INFO, "CAmRoutingSenderDbus::asyncAbort called");
    mapHandles_t::iterator iter = mMapHandles.begin();
    iter = mMapHandles.find(handle.handle);
    if (iter != mMapHandles.end())
    {
        CAmRoutingDbusSend send(mpDBusConnection, iter->second.busname, iter->second.path, iter->second.interface, "asyncAbort");
        send.append(handle.handle);
        return (send.send());
    }
    log(&routingDbus, DLT_LOG_ERROR, "CAmRoutingSenderDbus::asyncAbort could not find interface");
    return (E_UNKNOWN);

}

am_Error_e CAmRoutingSenderDbus::asyncConnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_ConnectionFormat_e connectionFormat)
{
    log(&routingDbus, DLT_LOG_INFO, "CAmRoutingSenderDbus::asyncConnect called");
    mapSources_t::iterator iter = mMapSources.begin();
    iter = mMapSources.find(sourceID);
    if (iter != mMapSources.end())
    {
        CAmRoutingDbusSend send(mpDBusConnection, iter->second.busname, iter->second.path, iter->second.interface, "asyncConnect");
        send.append(handle.handle);
        send.append(connectionID);
        send.append(sourceID);
        send.append(sinkID);
        send.append(static_cast<int16_t>(connectionFormat));
        mMapConnections.insert(std::make_pair(connectionID, (iter->second)));
        mMapHandles.insert(std::make_pair(+handle.handle, iter->second));
        return (send.send());
    }
    log(&routingDbus, DLT_LOG_ERROR, "CAmRoutingSenderDbus::asyncConnect could not find interface");
    return (E_UNKNOWN);
}

am_Error_e CAmRoutingSenderDbus::asyncDisconnect(const am_Handle_s handle, const am_connectionID_t connectionID)
{
    log(&routingDbus, DLT_LOG_INFO, "CAmRoutingSenderDbus::asyncDisconnect called");
    mapConnections_t::iterator iter = mMapConnections.begin();
    iter = mMapConnections.find(connectionID);
    if (iter != mMapConnections.end())
    {
        CAmRoutingDbusSend send(mpDBusConnection, iter->second.busname, iter->second.path, iter->second.interface, "asyncDisconnect");
        send.append(handle.handle);
        send.append(connectionID);
        mMapHandles.insert(std::make_pair(+handle.handle, iter->second));
        return (send.send());
    }
    log(&routingDbus, DLT_LOG_ERROR, "CAmRoutingSenderDbus::asyncDisconnect could not find interface");
    return (E_UNKNOWN);
}

am_Error_e CAmRoutingSenderDbus::asyncSetSinkVolume(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time)
{
    log(&routingDbus, DLT_LOG_INFO, "CAmRoutingSenderDbus::asyncSetSinkVolume called");
    mapSinks_t::iterator iter = mMapSinks.begin();
    iter = mMapSinks.find(sinkID);
    if (iter != mMapSinks.end())
    {
        CAmRoutingDbusSend send(mpDBusConnection, iter->second.busname, iter->second.path, iter->second.interface, "asyncSetSinkVolume");
        send.append(handle.handle);
        send.append(sinkID);
        send.append(volume);
        send.append(static_cast<int16_t>(ramp));
        send.append(time);
        mMapHandles.insert(std::make_pair(+handle.handle, iter->second));
        return (send.send());
    }
    log(&routingDbus, DLT_LOG_ERROR, "CAmRoutingSenderDbus::asyncSetSinkVolume could not find interface");
    return (E_UNKNOWN);
}

am_Error_e CAmRoutingSenderDbus::asyncSetSourceVolume(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time)
{
    log(&routingDbus, DLT_LOG_INFO, "CAmRoutingSenderDbus::asyncSetSourceVolume called");
    mapSources_t::iterator iter = mMapSources.begin();
    iter = mMapSources.find(sourceID);
    if (iter != mMapSources.end())
    {
        CAmRoutingDbusSend send(mpDBusConnection, iter->second.busname, iter->second.path, iter->second.interface, "asyncSetSourceVolume");
        send.append(handle.handle);
        send.append(sourceID);
        send.append(volume);
        send.append(static_cast<int16_t>(ramp));
        send.append(time);
        mMapHandles.insert(std::make_pair(+handle.handle, iter->second));
        return (send.send());
    }
    log(&routingDbus, DLT_LOG_ERROR, "CAmRoutingSenderDbus::asyncSetSourceVolume could not find interface");
    return (E_UNKNOWN);
}

am_Error_e CAmRoutingSenderDbus::asyncSetSourceState(const am_Handle_s handle, const am_sourceID_t sourceID, const am_SourceState_e state)
{
    log(&routingDbus, DLT_LOG_INFO, "CAmRoutingSenderDbus::asyncSetSourceState called");
    mapSources_t::iterator iter = mMapSources.begin();
    iter = mMapSources.find(sourceID);
    if (iter != mMapSources.end())
    {
        CAmRoutingDbusSend send(mpDBusConnection, iter->second.busname, iter->second.path, iter->second.interface, "asyncSetSourceState");
        send.append(handle.handle);
        send.append(sourceID);
        send.append(static_cast<int16_t>(state));
        mMapHandles.insert(std::make_pair(+handle.handle, iter->second));
        return (send.send());
    }
    log(&routingDbus, DLT_LOG_ERROR, "CAmRoutingSenderDbus::asyncSetSourceState could not find interface");
    return (E_UNKNOWN);
}

am_Error_e CAmRoutingSenderDbus::asyncSetSinkSoundProperties(const am_Handle_s handle, const am_sinkID_t sinkID, const std::vector<am_SoundProperty_s>& listSoundProperties)
{
    log(&routingDbus, DLT_LOG_INFO, "CAmRoutingSenderDbus::asyncSetSinkSoundProperties called");
    mapSinks_t::iterator iter = mMapSinks.begin();
    iter = mMapSinks.find(sinkID);
    if (iter != mMapSinks.end())
    {
        CAmRoutingDbusSend send(mpDBusConnection, iter->second.busname, iter->second.path, iter->second.interface, "asyncSetSinkSoundProperties");
        send.append(handle.handle);
        send.append(sinkID);
        send.append(listSoundProperties);
        mMapHandles.insert(std::make_pair(+handle.handle, iter->second));
        return (send.send());
    }
    log(&routingDbus, DLT_LOG_ERROR, "CAmRoutingSenderDbus::asyncSetSinkSoundProperties could not find interface");
    return (E_UNKNOWN);
}

am_Error_e CAmRoutingSenderDbus::asyncSetSinkSoundProperty(const am_Handle_s handle, const am_sinkID_t sinkID, const am_SoundProperty_s& soundProperty)
{
    log(&routingDbus, DLT_LOG_INFO, "CAmRoutingSenderDbus::asyncSetSinkSoundProperty called");
    mapSinks_t::iterator iter = mMapSinks.begin();
    iter = mMapSinks.find(sinkID);
    if (iter != mMapSinks.end())
    {
        CAmRoutingDbusSend send(mpDBusConnection, iter->second.busname, iter->second.path, iter->second.interface, "asyncSetSinkSoundProperty");
        send.append(handle.handle);
        send.append(sinkID);
        send.append(soundProperty);
        mMapHandles.insert(std::make_pair(+handle.handle, iter->second));
        return (send.send());
    }
    log(&routingDbus, DLT_LOG_ERROR, "CAmRoutingSenderDbus::asyncSetSinkSoundProperty could not find interface");
    return (E_UNKNOWN);
}

am_Error_e CAmRoutingSenderDbus::asyncSetSourceSoundProperties(const am_Handle_s handle, const am_sourceID_t sourceID, const std::vector<am_SoundProperty_s>& listSoundProperties)
{
    log(&routingDbus, DLT_LOG_INFO, "CAmRoutingSenderDbus::asyncSetSourceSoundProperties called");
    mapSources_t::iterator iter = mMapSources.begin();
    iter = mMapSources.find(sourceID);
    if (iter != mMapSources.end())
    {
        CAmRoutingDbusSend send(mpDBusConnection, iter->second.busname, iter->second.path, iter->second.interface, "asyncSetSourceSoundProperties");
        send.append(handle.handle);
        send.append(sourceID);
        send.append(listSoundProperties);
        mMapHandles.insert(std::make_pair(+handle.handle, iter->second));
        return (send.send());
    }
    log(&routingDbus, DLT_LOG_ERROR, "CAmRoutingSenderDbus::asyncSetSourceSoundProperties could not find interface");
    return (E_UNKNOWN);
}

am_Error_e CAmRoutingSenderDbus::asyncSetSourceSoundProperty(const am_Handle_s handle, const am_sourceID_t sourceID, const am_SoundProperty_s& soundProperty)
{
    log(&routingDbus, DLT_LOG_INFO, "CAmRoutingSenderDbus::asyncSetSourceSoundProperty called");
    mapSources_t::iterator iter = mMapSources.begin();
    iter = mMapSources.find(sourceID);
    if (iter != mMapSources.end())
    {
        CAmRoutingDbusSend send(mpDBusConnection, iter->second.busname, iter->second.path, iter->second.interface, "asyncSetSourceSoundProperty");
        send.append(handle.handle);
        send.append(sourceID);
        send.append(soundProperty);
        mMapHandles.insert(std::make_pair(+handle.handle, iter->second));
        return (send.send());
    }
    log(&routingDbus, DLT_LOG_ERROR, "CAmRoutingSenderDbus::asyncSetSourceSoundProperty could not find interface");
    return (E_UNKNOWN);
}

am_Error_e CAmRoutingSenderDbus::asyncCrossFade(const am_Handle_s handle, const am_crossfaderID_t crossfaderID, const am_HotSink_e hotSink, const am_RampType_e rampType, const am_time_t time)
{
    (void)handle;
    (void)crossfaderID;
    (void)hotSink;
    (void)rampType;
    (void)time;
    //todo implement
    return (E_NON_EXISTENT);
}

am_Error_e CAmRoutingSenderDbus::setDomainState(const am_domainID_t domainID, const am_DomainState_e domainState)
{
    log(&routingDbus, DLT_LOG_INFO, "CAmRoutingSenderDbus::setDomainState called");
    mapDomain_t::iterator iter = mMapDomains.begin();
    iter = mMapDomains.find(domainID);
    if (iter != mMapDomains.end())
    {
        CAmRoutingDbusSend send(mpDBusConnection, iter->second.busname, iter->second.path, iter->second.interface, "setDomainState");
        send.append(domainID);
        send.append(static_cast<int16_t>(domainState));
        return (send.send());
    }
    log(&routingDbus, DLT_LOG_ERROR, "CAmRoutingSenderDbus::setDomainState could not find interface");
    return (E_UNKNOWN);
}

am_Error_e CAmRoutingSenderDbus::returnBusName(std::string& BusName) const
{
    BusName = "DbusRoutingPlugin";
    return (E_OK);
}

void CAmRoutingSenderDbus::removeHandle(uint16_t handle)
{
    mMapHandles.erase(handle);
}

void CAmRoutingSenderDbus::addDomainLookup(am_domainID_t domainID, rs_lookupData_s lookupData)
{
    mMapDomains.insert(std::make_pair(domainID, lookupData));
}

void CAmRoutingSenderDbus::addSourceLookup(am_sourceID_t sourceID, am_domainID_t domainID)
{
    mapDomain_t::iterator iter(mMapDomains.begin());
    iter = mMapDomains.find(domainID);
    if (iter != mMapDomains.end())
    {
        mMapSources.insert(std::make_pair(sourceID, iter->second));
    }
}

void CAmRoutingSenderDbus::addSinkLookup(am_sinkID_t sinkID, am_domainID_t domainID)
{
    mapDomain_t::iterator iter(mMapDomains.begin());
    iter = mMapDomains.find(domainID);
    if (iter != mMapDomains.end())
    {
        mMapSinks.insert(std::make_pair(sinkID, iter->second));
    }
}

void CAmRoutingSenderDbus::removeDomainLookup(am_domainID_t domainID)
{
    mMapHandles.erase(domainID);
}

void CAmRoutingSenderDbus::removeSourceLookup(am_sourceID_t sourceID)
{
    mMapHandles.erase(sourceID);
}

void CAmRoutingSenderDbus::removeSinkLookup(am_sinkID_t sinkID)
{
    mMapHandles.erase(sinkID);
}

}

