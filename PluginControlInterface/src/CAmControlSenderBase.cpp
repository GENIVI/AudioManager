/**
 *  Copyright (c) 2012 BMW
 *
 *  \author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
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

#include "CAmControlSenderBase.h"
#include <cassert>
#include <algorithm>
#include "shared/CAmDltWrapper.h"
#include "control/IAmControlReceive.h"

using namespace am;

extern "C" IAmControlSend* PluginControlInterfaceFactory()
{
    return (new CAmControlSenderBase());
}

extern "C" void destroyControlPluginInterface(IAmControlSend* controlSendInterface)
{
    delete controlSendInterface;
}

CAmControlSenderBase::CAmControlSenderBase() :
        mControlReceiveInterface(NULL), //
        mListOpenConnections(), //
        mListOpenDisconnections(), //
        mListOpenVolumeChanges()
{
}

CAmControlSenderBase::~CAmControlSenderBase()
{
}

am_Error_e CAmControlSenderBase::startupController(IAmControlReceive *controlreceiveinterface)
{
    assert(controlreceiveinterface);
    mControlReceiveInterface = controlreceiveinterface;
    //here is a good place to insert SystemProperties into the database...
    //and might be a good place to insert the Source and Sink CLasses as well
    return E_NOT_USED;
}

void CAmControlSenderBase::setControllerReady()
{
    //here is a good place to insert Source and SinkClasses into the database...
    mControlReceiveInterface->setRoutingReady();
    mControlReceiveInterface->setCommandReady();
}

am_Error_e CAmControlSenderBase::hookUserConnectionRequest(const am_sourceID_t sourceID, const am_sinkID_t sinkID, am_mainConnectionID_t & mainConnectionID)
{
    std::vector<am_Route_s> listRoutes;
    std::vector<am_connectionID_t> listConnectionIDs;
    am_Handle_s handle;
    mControlReceiveInterface->getRoute(true, sourceID, sinkID, listRoutes);
    if (listRoutes.empty())
        return (E_NOT_POSSIBLE);

    std::vector<am_MainConnection_s> listAllMainConnections;
    mControlReceiveInterface->getListMainConnections(listAllMainConnections);
    std::vector<am_MainConnection_s>::iterator itAll(listAllMainConnections.begin());
    for (; itAll != listAllMainConnections.end(); ++itAll)
    {
        if (itAll->sinkID == sinkID && itAll->sourceID == sourceID)
            return (E_ALREADY_EXISTS);
    }

    std::vector<handleStatus> listHandleStaus;
    std::vector<am_RoutingElement_s>::iterator it(listRoutes[0].route.begin());
    for (; it != listRoutes[0].route.end(); ++it)
    {
        am_connectionID_t connectionID;
        mControlReceiveInterface->connect(handle, connectionID, it->connectionFormat, it->sourceID, it->sinkID);
        handleStatus status;
        status.handle = handle;
        status.status = false;
        listHandleStaus.push_back(status);
        listConnectionIDs.push_back(connectionID);
    }
    am_MainConnection_s mainConnectionData;
    mainConnectionData.mainConnectionID = 0;
    mainConnectionData.sinkID = sinkID;
    mainConnectionData.sourceID = sourceID;
    mainConnectionData.connectionState = CS_CONNECTING;
    mainConnectionData.delay = 0;
    mainConnectionData.listConnectionID = listConnectionIDs;
    mControlReceiveInterface->enterMainConnectionDB(mainConnectionData, mainConnectionID);
    mainConnectionSet set;
    set.connectionID = mainConnectionID;
    set.listHandleStaus = listHandleStaus;
    mListOpenConnections.push_back(set);
    return E_OK;
}

am_Error_e CAmControlSenderBase::hookUserDisconnectionRequest(const am_mainConnectionID_t connectionID)
{
    //first check if there is a connectionID like that
    am_MainConnection_s mainConnection;
    am_Error_e error;
    if ((error = mControlReceiveInterface->getMainConnectionInfoDB(connectionID, mainConnection)) != E_OK)
    {
        return error;
    }

    std::vector<am_connectionID_t>::iterator it(mainConnection.listConnectionID.begin());
    std::vector<handleStatus> listHandleStaus;
    for (; it != mainConnection.listConnectionID.end(); ++it)
    {
        handleStatus status;
        status.status = false;
        mControlReceiveInterface->disconnect(status.handle, *it);
        listHandleStaus.push_back(status);
    }
    mainConnectionSet set;
    set.connectionID = connectionID;
    set.listHandleStaus = listHandleStaus;
    mListOpenDisconnections.push_back(set);
    return E_OK;
}

am_Error_e CAmControlSenderBase::hookUserSetMainSinkSoundProperty(const am_sinkID_t sinkID, const am_MainSoundProperty_s & soundProperty)
{
    if (sinkID == 0)
        return E_NON_EXISTENT;

    mainSinkSoundPropertySet set;
    set.sinkID = sinkID;
    set.mainSoundProperty = soundProperty;
    am_SoundProperty_s sp;
    //I know this is bad - just for the reference, ok?
    sp.type = static_cast<am_SoundPropertyType_e>(soundProperty.type);
    sp.value = soundProperty.value;
    am_Error_e error;
    if ((error = mControlReceiveInterface->setSinkSoundProperty(set.handle, sinkID, sp)) != E_OK)
    {
        return error;
    }
    mListMainSoundPropertyChanges.push_back(set);
    return E_OK;
}

am_Error_e CAmControlSenderBase::hookUserSetMainSourceSoundProperty(const am_sourceID_t sourceID, const am_MainSoundProperty_s & soundProperty)
{
    (void) sourceID;
    (void) soundProperty;
    return E_NOT_USED;
}

am_Error_e CAmControlSenderBase::hookUserSetSystemProperty(const am_SystemProperty_s & property)
{
    (void) property;
    return E_NOT_USED;
}

am_Error_e CAmControlSenderBase::hookUserVolumeChange(const am_sinkID_t SinkID, const am_mainVolume_t newVolume)
{
    assert(SinkID!=0);
    mainVolumeSet set;
    set.sinkID = SinkID;
    set.mainVolume = newVolume;
    am_Error_e error;
    if ((error = mControlReceiveInterface->setSinkVolume(set.handle, SinkID, newVolume, RAMP_GENIVI_DIRECT, 20)) != E_OK)
    {
        return error;
    }
    mListOpenVolumeChanges.push_back(set);
    return E_OK;
}

am_Error_e CAmControlSenderBase::hookUserVolumeStep(const am_sinkID_t SinkID, const int16_t increment)
{
    assert(SinkID!=0);
    mainVolumeSet set;
    set.sinkID = SinkID;
    am_Error_e error;
    am_Sink_s sink;
    mControlReceiveInterface->getSinkInfoDB(SinkID, sink);
    set.mainVolume = sink.volume + increment;
    if ((error = mControlReceiveInterface->setSinkVolume(set.handle, SinkID, set.mainVolume, RAMP_GENIVI_DIRECT, 20)) != E_OK)
    {
        return error;
    }
    mListOpenVolumeChanges.push_back(set);
    return E_OK;
}

am_Error_e CAmControlSenderBase::hookUserSetSinkMuteState(const am_sinkID_t sinkID, const am_MuteState_e muteState)
{
    assert(sinkID!=0);

    mainVolumeSet set;
    set.sinkID = sinkID;
    am_Error_e error;
    am_Sink_s sink;
    mControlReceiveInterface->getSinkInfoDB(sinkID, sink);

    if (muteState == MS_MUTED)
    {
        set.mainVolume = sink.mainVolume;
        if ((error = mControlReceiveInterface->setSinkVolume(set.handle, sinkID, 0, RAMP_GENIVI_DIRECT, 20)) != E_OK)
        {
            return error;
        }
    }
    else
    {
        set.mainVolume=sink.mainVolume;
        if ((error = mControlReceiveInterface->setSinkVolume(set.handle, sinkID, set.mainVolume, RAMP_GENIVI_DIRECT, 20)) != E_OK)
        {
            return error;
        }
    }
    mListOpenVolumeChanges.push_back(set);
    mControlReceiveInterface->changeSinkMuteStateDB(muteState,sinkID);
    return (E_OK);
}

am_Error_e CAmControlSenderBase::hookSystemRegisterDomain(const am_Domain_s & domainData, am_domainID_t & domainID)
{
    //this application does not do anything with it -> but some product might want to take influence here
    return mControlReceiveInterface->enterDomainDB(domainData, domainID);
}

am_Error_e CAmControlSenderBase::hookSystemDeregisterDomain(const am_domainID_t domainID)
{
    //this application does not do anything with it -> but some product might want to take influence here
    return mControlReceiveInterface->removeDomainDB(domainID);
}

void CAmControlSenderBase::hookSystemDomainRegistrationComplete(const am_domainID_t domainID)
{
    (void) domainID;
}

am_Error_e CAmControlSenderBase::hookSystemRegisterSink(const am_Sink_s & sinkData, am_sinkID_t & sinkID)
{
    //this application does not do anything with it -> but some product might want to take influence here
    return mControlReceiveInterface->enterSinkDB(sinkData, sinkID);
}

am_Error_e CAmControlSenderBase::hookSystemDeregisterSink(const am_sinkID_t sinkID)
{
    //this application does not do anything with it -> but some product might want to take influence here
    return mControlReceiveInterface->removeSinkDB(sinkID);
}

am_Error_e CAmControlSenderBase::hookSystemRegisterSource(const am_Source_s & sourceData, am_sourceID_t & sourceID)
{
    //this application does not do anything with it -> but some product might want to take influence here
    return mControlReceiveInterface->enterSourceDB(sourceData, sourceID);
}

am_Error_e CAmControlSenderBase::hookSystemDeregisterSource(const am_sourceID_t sourceID)
{
    //this application does not do anything with it -> but some product might want to take influence here
    return mControlReceiveInterface->removeSourceDB(sourceID);
}

am_Error_e CAmControlSenderBase::hookSystemRegisterGateway(const am_Gateway_s & gatewayData, am_gatewayID_t & gatewayID)
{
    //this application does not do anything with it -> but some product might want to take influence here
    return mControlReceiveInterface->enterGatewayDB(gatewayData, gatewayID);
}

am_Error_e CAmControlSenderBase::hookSystemDeregisterGateway(const am_gatewayID_t gatewayID)
{
    //this application does not do anything with it -> but some product might want to take influence here
    return mControlReceiveInterface->removeGatewayDB(gatewayID);
}

am_Error_e CAmControlSenderBase::hookSystemRegisterCrossfader(const am_Crossfader_s & crossfaderData, am_crossfaderID_t & crossfaderID)
{
    //this application does not do anything with it -> but some product might want to take influence here
    return mControlReceiveInterface->enterCrossfaderDB(crossfaderData, crossfaderID);
}

am_Error_e CAmControlSenderBase::hookSystemDeregisterCrossfader(const am_crossfaderID_t crossfaderID)
{
    //this application does not do anything with it -> but some product might want to take influence here
    return mControlReceiveInterface->removeCrossfaderDB(crossfaderID);
}

void CAmControlSenderBase::hookSystemSinkVolumeTick(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume)
{
    (void) handle;
    (void) sinkID;
    (void) volume;
}

void CAmControlSenderBase::hookSystemSourceVolumeTick(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume)
{
    (void) handle;
    (void) sourceID;
    (void) volume;
}

void CAmControlSenderBase::hookSystemInterruptStateChange(const am_sourceID_t sourceID, const am_InterruptState_e interruptState)
{
    (void) sourceID;
    (void) interruptState;
}

void CAmControlSenderBase::hookSystemSinkAvailablityStateChange(const am_sinkID_t sinkID, const am_Availability_s & availability)
{
    (void) sinkID;
    (void) availability;
}

void CAmControlSenderBase::hookSystemSourceAvailablityStateChange(const am_sourceID_t sourceID, const am_Availability_s & availability)
{
    (void) sourceID;
    (void) availability;
}

void CAmControlSenderBase::hookSystemDomainStateChange(const am_domainID_t domainID, const am_DomainState_e state)
{
    (void) domainID;
    (void) state;
}

void CAmControlSenderBase::hookSystemReceiveEarlyData(const std::vector<am_EarlyData_s> & data)
{
    (void) data;
}

void CAmControlSenderBase::hookSystemSpeedChange(const am_speed_t speed)
{
    (void) speed;
}

void CAmControlSenderBase::hookSystemTimingInformationChanged(const am_mainConnectionID_t mainConnectionID, const am_timeSync_t time)
{
    (void) mainConnectionID;
    (void) time;
}

void CAmControlSenderBase::cbAckConnect(const am_Handle_s handle, const am_Error_e errorID)
{
    (void) errorID;
    //\todo:error checking
    std::vector<mainConnectionSet>::iterator it(mListOpenConnections.begin());
    for (; it != mListOpenConnections.end(); ++it)
    {
        std::vector<handleStatus>::iterator hit;
        handleStatus status;
        status.status = true;
        status.handle = handle;
        hit = std::find_if(it->listHandleStaus.begin(), it->listHandleStaus.end(), findHandle(status));
        if (hit == it->listHandleStaus.end())
            continue;
        hit->status = true;
        if (it->listHandleStaus.end() == std::find_if(it->listHandleStaus.begin(), it->listHandleStaus.end(), checkHandle(status)))
        {
            mControlReceiveInterface->changeMainConnectionStateDB(it->connectionID, CS_CONNECTED);
            mListOpenConnections.erase(it);
            break;
        }
    }
}

void CAmControlSenderBase::cbAckDisconnect(const am_Handle_s handle, const am_Error_e errorID)
{
    (void) errorID;
    //\todo:error checking
    std::vector<mainConnectionSet>::iterator it(mListOpenDisconnections.begin());
    for (; it != mListOpenDisconnections.end(); ++it)
    {
        std::vector<handleStatus>::iterator hit;
        handleStatus status;
        status.status = true;
        status.handle = handle;
        hit = std::find_if(it->listHandleStaus.begin(), it->listHandleStaus.end(), findHandle(status));
        if (hit == it->listHandleStaus.end())
            continue;
        hit->status = true;
        if (it->listHandleStaus.end() == std::find_if(it->listHandleStaus.begin(), it->listHandleStaus.end(), checkHandle(status)))
        {
            mControlReceiveInterface->removeMainConnectionDB(it->connectionID);
            mListOpenDisconnections.erase(it);
            break;
        }
    }
}

void CAmControlSenderBase::cbAckCrossFade(const am_Handle_s handle, const am_HotSink_e hostsink, const am_Error_e error)
{
    (void) handle;
    (void) hostsink;
    (void) error;
}

void CAmControlSenderBase::cbAckSetSinkVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error)
{
    (void) error;
    (void) volume;
    //\todo:error checking
    std::vector<mainVolumeSet>::iterator it(mListOpenVolumeChanges.begin());
    for (; it != mListOpenVolumeChanges.end(); ++it)
    {
        if (handle.handle == it->handle.handle)
        {
            mControlReceiveInterface->changeSinkMainVolumeDB(it->mainVolume, it->sinkID);
        }
    }
}

void CAmControlSenderBase::cbAckSetSourceVolumeChange(const am_Handle_s handle, const am_volume_t voulme, const am_Error_e error)
{
    (void) error;
    (void) voulme;
    (void) handle;
}

void CAmControlSenderBase::cbAckSetSourceState(const am_Handle_s handle, const am_Error_e error)
{
    (void) error;
    (void) handle;
}

void CAmControlSenderBase::cbAckSetSourceSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
    (void) error;
    (void) handle;
}

void CAmControlSenderBase::cbAckSetSinkSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
    (void) error;
    //\todo:error checking
    std::vector<mainSinkSoundPropertySet>::iterator it(mListMainSoundPropertyChanges.begin());
    for (; it != mListMainSoundPropertyChanges.end(); ++it)
    {
        if (handle.handle == it->handle.handle)
        {
            mControlReceiveInterface->changeMainSinkSoundPropertyDB(it->mainSoundProperty, it->sinkID);
        }
    }
}

void CAmControlSenderBase::cbAckSetSourceSoundProperties(const am_Handle_s handle, const am_Error_e error)
{
    (void) error;
    (void) handle;
}

void CAmControlSenderBase::cbAckSetSinkSoundProperties(const am_Handle_s handle, const am_Error_e error)
{
    (void) error;
    (void) handle;
}

void CAmControlSenderBase::setControllerRundown()
{
}

am_Error_e CAmControlSenderBase::getConnectionFormatChoice(const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_Route_s listRoute, const std::vector<am_ConnectionFormat_e> listPossibleConnectionFormats, std::vector<am_ConnectionFormat_e> & listPrioConnectionFormats)
{
    (void) sourceID;
    (void) sinkID;
    (void) listRoute;
    //ok, this is cheap. In a real product you have your preferences, right?
    listPrioConnectionFormats = listPossibleConnectionFormats;
    return (E_OK);
}

void CAmControlSenderBase::getInterfaceVersion(std::string & version) const
{
    version = ControlSendVersion;
}

void CAmControlSenderBase::confirmCommandReady()
{
    logInfo("ControlSenderPlugin got Routing Ready confirmed");
}

void CAmControlSenderBase::confirmRoutingReady()
{
    logInfo("ControlSenderPlugin got Command Ready confirmed");
}

void CAmControlSenderBase::confirmCommandRundown()
{
    logInfo("ControlSenderPlugin got Routing Rundown confirmed");
}

void CAmControlSenderBase::confirmRoutingRundown()
{
    logInfo("ControlSenderPlugin got Command Rundown confirmed");
}

