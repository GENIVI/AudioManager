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

#include "shared/CAmDltWrapper.h"
#include "shared/CAmDbusWrapper.h"
#include "CAmRoutingSenderDbus.h"
#include "CAmDbusSend.h"

#define PULSE_INTERFACE_TARGET "org.genivi.pulse"
#define PULSE_REGISTER_PATH "/pulse"
#define PULSE_INTERFACE_NAME "org.genivi.pulse"

DLT_DECLARE_CONTEXT(routingDbus)

extern "C" IAmRoutingSend* PluginRoutingInterfaceDbusFactory()
{
    return (new CAmRoutingSenderDbus());
}

extern "C" void destroyRoutingPluginInterfaceDbus(IAmRoutingSend* routingSendInterface)
{
    delete routingSendInterface;
}

CAmRoutingSenderDbus::CAmRoutingSenderDbus()
    : 	mDBusMessageHandler(),
        mRoutingReceiverShadow(),
        mDBusWrapper(NULL),
        mRoutingReceiveInterface(NULL)
{
    CAmDltWrapper::instance()->registerContext(routingDbus, "DRS", "DBus Plugin");
    log(&routingDbus, DLT_LOG_INFO, "RoutingSender constructed");
}

CAmRoutingSenderDbus::~CAmRoutingSenderDbus()
{
    log(&routingDbus, DLT_LOG_INFO, "RoutingSender destructed");
    CAmDltWrapper::instance()->unregisterContext(routingDbus);
}

am_Error_e CAmRoutingSenderDbus::asyncAbort(const am_Handle_s handle)
{
    CAmDbusSend send = CAmDbusSend(connection,PULSE_INTERFACE_TARGET,PULSE_REGISTER_PATH, PULSE_INTERFACE_NAME, "abort");
    send.appendInteger(handle.handleType);
    send.appendInteger(handle.handle);
    send.sendReply();
    return (E_OK);
}

am_Error_e CAmRoutingSenderDbus::asyncConnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_ConnectionFormat_e connectionFormat)
{
    (void) connectionFormat;
    log(&routingDbus, DLT_LOG_INFO, "pulse async connect");
    CAmDbusSend send = CAmDbusSend(connection,PULSE_INTERFACE_TARGET,PULSE_REGISTER_PATH, PULSE_INTERFACE_NAME, "connect");
    send.appendInteger(handle.handle);
    send.appendInteger(connectionID);
    send.appendInteger(sourceID);
    send.appendInteger(sinkID);
    send.sendReply();
    return (E_OK);
}

am_Error_e CAmRoutingSenderDbus::asyncDisconnect(const am_Handle_s handle, const am_connectionID_t connectionID)
{
    log(&routingDbus, DLT_LOG_INFO, "pulse async disconnect");
    CAmDbusSend send = CAmDbusSend(connection,PULSE_INTERFACE_TARGET,PULSE_REGISTER_PATH, PULSE_INTERFACE_NAME, "disconnect");
    send.appendInteger(handle.handle);
    send.appendInteger(connectionID);
    send.sendReply();
    return (E_OK);
}

am_Error_e CAmRoutingSenderDbus::asyncSetSinkVolume(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time)
{
    log(&routingDbus, DLT_LOG_INFO, "pulse set sink volume");
    CAmDbusSend send = CAmDbusSend(connection,PULSE_INTERFACE_TARGET,PULSE_REGISTER_PATH, PULSE_INTERFACE_NAME, "setSinkVolume");
    send.appendInteger(handle.handle);
    send.appendInteger(sinkID);
    send.appendInteger(volume);
    send.appendInteger(ramp);
    send.appendInteger(time);
    send.sendReply();
    return (E_OK);
}

am_Error_e CAmRoutingSenderDbus::asyncSetSourceVolume(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time)
{
    log(&routingDbus, DLT_LOG_INFO, "pulse set source volume");
    CAmDbusSend send = CAmDbusSend(connection,PULSE_INTERFACE_TARGET,PULSE_REGISTER_PATH, PULSE_INTERFACE_NAME, "setSourceVolume");
    send.appendInteger(handle.handle);
    send.appendInteger(sourceID);
    send.appendInteger(volume);
    send.appendInteger(ramp);
    send.appendInteger(time);
    send.sendReply();
    return (E_OK);
}

am_Error_e CAmRoutingSenderDbus::asyncSetSourceState(const am_Handle_s handle, const am_sourceID_t sourceID, const am_SourceState_e state)
{
    (void) handle;
    (void) sourceID;
    (void) state;
    return (E_NOT_USED);
}

am_Error_e CAmRoutingSenderDbus::asyncCrossFade(const am_Handle_s handle, const am_crossfaderID_t crossfaderID, const am_HotSink_e hotSink, const am_RampType_e rampType, const am_time_t time)
{
    (void) handle;
    (void) crossfaderID;
    (void) hotSink;
    (void) rampType;
    (void) time;
    return (E_NOT_USED);
}

am_Error_e CAmRoutingSenderDbus::setDomainState(const am_domainID_t domainID, const am_DomainState_e domainState)
{
    (void) domainID;
    (void) domainState;
    return (E_NOT_USED);
}

am_Error_e CAmRoutingSenderDbus::startupInterface(IAmRoutingReceive *routingreceiveinterface)
{
    log(&routingDbus, DLT_LOG_INFO, "startupInterface called");
    mRoutingReceiveInterface=routingreceiveinterface;
    mRoutingReceiverShadow.setRoutingReceiver(mRoutingReceiveInterface);
    mRoutingReceiveInterface->getDBusConnectionWrapper(mDBusWrapper);
    //assert(mDBusWrapper!=NULL);
    mDBusWrapper->getDBusConnection(connection);
    //assert(connection!=NULL);
    mDBusMessageHandler.setDBusConnection(connection);
    return (E_OK);
}

void CAmRoutingSenderDbus::setRoutingReady(const uint16_t handle)
{
    (void) handle;
    log(&routingDbus, DLT_LOG_INFO, "sending systemReady signal");
    mDBusMessageHandler.initSignal(std::string(ROUTING_NODE),"signal_systemReady");
    mDBusMessageHandler.sendMessage();
}

void CAmRoutingSenderDbus::setRoutingRundown(const uint16_t handle)
{
    (void) handle;
}

am_Error_e CAmRoutingSenderDbus::returnBusName(std::string & BusName) const
{
    BusName = "DbusPlugin";
    return (E_OK);
}

am_Error_e CAmRoutingSenderDbus::asyncSetSinkSoundProperty(const am_Handle_s handle, const am_sinkID_t sinkID, const am_SoundProperty_s & soundProperty)
{
    log(&routingDbus, DLT_LOG_INFO, "pulse set sink sound property ");
    CAmDbusSend send = CAmDbusSend(connection,PULSE_INTERFACE_TARGET,PULSE_REGISTER_PATH, PULSE_INTERFACE_NAME, "setSinkSoundProperty");
    send.appendInteger(handle.handle);
    send.appendInteger(soundProperty.type);
    send.appendInteger(soundProperty.value);
    send.appendInteger(sinkID);
    send.sendReply();
    return (E_OK);
}

am_Error_e CAmRoutingSenderDbus::asyncSetSinkSoundProperties(const am_Handle_s handle, const am_sinkID_t sinkID, const std::vector<am_SoundProperty_s> & listSoundProperties)
{
    (void) handle;
    (void) sinkID;
    (void) listSoundProperties;
    return (E_NOT_USED);
}

am_Error_e CAmRoutingSenderDbus::asyncSetSourceSoundProperty(const am_Handle_s handle, const am_sourceID_t sourceID, const am_SoundProperty_s & soundProperty)
{
    (void) handle;
    (void) sourceID;
    (void) soundProperty;
    return (E_NOT_USED);
}

am_Error_e CAmRoutingSenderDbus::asyncSetSourceSoundProperties(const am_Handle_s handle, const am_sourceID_t sourceID, const std::vector<am_SoundProperty_s> & listSoundProperties)
{
    (void) handle;
    (void) sourceID;
    (void) listSoundProperties;
    return (E_NOT_USED);
}

void CAmRoutingSenderDbus::getInterfaceVersion(std::string & version) const
{
    version=RoutingSendVersion;
}



