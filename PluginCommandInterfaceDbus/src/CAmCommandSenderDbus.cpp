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

#include "CAmCommandSenderDbus.h"
#include <algorithm>
#include <string>
#include <vector>
#include <cassert>
#include <set>
#include "CAmDbusMessageHandler.h"
#include "shared/CAmDltWrapper.h"


using namespace am;
DLT_DECLARE_CONTEXT(commandDbus)


/**
 * factory for plugin loading
 */
extern "C" IAmCommandSend* PluginCommandInterfaceDbusFactory()
{
    CAmDltWrapper::instance()->registerContext(commandDbus, "DBP", "DBus Plugin");
    return (new CAmCommandSenderDbus());
}

/**
 * destroy instance of commandSendInterface
 */
extern "C" void destroyRoutingPluginInterfaceDbus(IAmCommandSend* commandSendInterface)
{
    delete commandSendInterface;
}

CAmCommandSenderDbus::CAmCommandSenderDbus() :
        mCAmDbusMessageHandler(), //
        mIAmCommandReceiverShadow(), //
        mpCAmDbusWrapper(NULL), //
        mpIAmCommandReceive(NULL), //
        mReady(false)
{
    log(&commandDbus, DLT_LOG_INFO, "DbusCommandSender constructor called");
}

CAmCommandSenderDbus::~CAmCommandSenderDbus()
{
    log(&commandDbus, DLT_LOG_INFO, "DbusCommandSender destructed");
    CAmDltWrapper::instance()->unregisterContext(commandDbus);
}

am_Error_e CAmCommandSenderDbus::startupInterface(IAmCommandReceive* commandreceiveinterface)
{
    log(&commandDbus, DLT_LOG_INFO, "startupInterface called");

    mpIAmCommandReceive = commandreceiveinterface;
    mIAmCommandReceiverShadow.setCommandReceiver(mpIAmCommandReceive);
    mpIAmCommandReceive->getDBusConnectionWrapper(mpCAmDbusWrapper);
    assert(mpCAmDbusWrapper!=NULL);
    DBusConnection * connection;
    mpCAmDbusWrapper->getDBusConnection(connection);
    assert(connection!=NULL);
    mCAmDbusMessageHandler.setDBusConnection(connection);
    return (E_OK);
}

void CAmCommandSenderDbus::setCommandReady(const uint16_t handle)
{
    //todo:implement handle handling
    log(&commandDbus, DLT_LOG_INFO, "cbCommunicationReady called");
    mReady = true;
    mpIAmCommandReceive->confirmCommandReady(handle);
}

void CAmCommandSenderDbus::setCommandRundown(const uint16_t handle)
{
    log(&commandDbus, DLT_LOG_INFO, "cbCommunicationRundown called");
    mReady = false;
    mpIAmCommandReceive->confirmCommandRundown(handle);
    /**
     * todo: implement DbusCommandSender::cbCommunicationRundown()
     */
}

void CAmCommandSenderDbus::cbNewMainConnection(const am_MainConnectionType_s& mainConnection)
{
    (void)mainConnection;
    //todo: change xml and interface to differetiate between new connection and removed one
    log(&commandDbus, DLT_LOG_INFO, "cbNumberOfMainConnectionsChanged called");

    if (mReady)
    {
        mCAmDbusMessageHandler.initSignal(std::string(MY_NODE), std::string("NumberOfMainConnectionsChanged"));
        mCAmDbusMessageHandler.sendMessage();
    }
}

void CAmCommandSenderDbus::cbRemovedMainConnection(const am_mainConnectionID_t mainConnection)
{
    (void)mainConnection;
//todo: change xml and interface to differetiate between new connection and removed one
    log(&commandDbus, DLT_LOG_INFO, "cbNumberOfMainConnectionsChanged called");

    if (mReady)
    {
        mCAmDbusMessageHandler.initSignal(std::string(MY_NODE), std::string("NumberOfMainConnectionsChanged"));
        mCAmDbusMessageHandler.sendMessage();
    }
}

void CAmCommandSenderDbus::cbNewSink(const am_SinkType_s& sink)
{
    log(&commandDbus, DLT_LOG_INFO, "cbNewSink called");

    if (mReady)
    {
        mCAmDbusMessageHandler.initSignal(std::string(MY_NODE), "SinkAdded");
        mCAmDbusMessageHandler.append(sink);

        log(&commandDbus, DLT_LOG_INFO, "send signal SinkAdded");
        mCAmDbusMessageHandler.sendMessage();
    }
}

void CAmCommandSenderDbus::cbRemovedSink(const am_sinkID_t sinkID)
{
    //todo: check if this really works!
    log(&commandDbus, DLT_LOG_INFO, "cbRemovedSink called");

    if (mReady)
    {
        mCAmDbusMessageHandler.initSignal(std::string(MY_NODE), "SinkRemoved");
        mCAmDbusMessageHandler.append(sinkID);

        log(&commandDbus, DLT_LOG_INFO, "send signal SinkAdded");
        mCAmDbusMessageHandler.sendMessage();
    }
}

void CAmCommandSenderDbus::cbNewSource(const am_SourceType_s& source)
{
    log(&commandDbus, DLT_LOG_INFO, "cbNumberOfSourcesChanged called");

    if (mReady)
    {
        mCAmDbusMessageHandler.initSignal(std::string(MY_NODE), "SourceAdded");
        mCAmDbusMessageHandler.append(source);

        log(&commandDbus, DLT_LOG_INFO, "send signal SourceAdded");
        mCAmDbusMessageHandler.sendMessage();
    }
}

void am::CAmCommandSenderDbus::cbRemovedSource(const am_sourceID_t source)
{
    if (mReady)
    {
        mCAmDbusMessageHandler.initSignal(std::string(MY_NODE), "SourceRemoved");
        mCAmDbusMessageHandler.append(source);

        log(&commandDbus, DLT_LOG_INFO, "send signal SourceRemoved");

        mCAmDbusMessageHandler.sendMessage();
    }
}

void CAmCommandSenderDbus::cbNumberOfSinkClassesChanged()
{
    log(&commandDbus, DLT_LOG_INFO, "cbNumberOfSinkClassesChanged called");

    if (mReady)
    {
        mCAmDbusMessageHandler.initSignal(std::string(MY_NODE), std::string("NumberOfSinkClassesChanged"));
        mCAmDbusMessageHandler.sendMessage();
    }
}

void CAmCommandSenderDbus::cbNumberOfSourceClassesChanged()
{
    log(&commandDbus, DLT_LOG_INFO, "cbNumberOfSourceClassesChanged called");

    if (mReady)
    {
        mCAmDbusMessageHandler.initSignal(std::string(MY_NODE), std::string("NumberOfSourceClassesChanged"));
        mCAmDbusMessageHandler.sendMessage();
    }
}

void CAmCommandSenderDbus::cbMainConnectionStateChanged(const am_mainConnectionID_t connectionID, const am_ConnectionState_e connectionState)
{
    log(&commandDbus, DLT_LOG_INFO, "cbMainConnectionStateChanged called, connectionID=", connectionID, "connectionState=", connectionState);

    if (mReady)
    {
        mCAmDbusMessageHandler.initSignal(std::string(MY_NODE), std::string("MainConnectionStateChanged"));
        mCAmDbusMessageHandler.append((dbus_uint16_t) connectionID);
        mCAmDbusMessageHandler.append((dbus_int16_t) connectionState);
        mCAmDbusMessageHandler.sendMessage();
    }
}

void CAmCommandSenderDbus::cbMainSinkSoundPropertyChanged(const am_sinkID_t sinkID, const am_MainSoundProperty_s & soundProperty)
{
    log(&commandDbus, DLT_LOG_INFO, "cbMainSinkSoundPropertyChanged called, sinkID", sinkID, "SoundProperty.type", soundProperty.type, "SoundProperty.value", soundProperty.value);

    if (mReady)
    {
        mCAmDbusMessageHandler.initSignal(std::string(MY_NODE), std::string("MainSinkSoundPropertyChanged"));
        mCAmDbusMessageHandler.append((dbus_uint16_t) sinkID);
        mCAmDbusMessageHandler.append(soundProperty);
        mCAmDbusMessageHandler.sendMessage();
    }
}

void CAmCommandSenderDbus::cbMainSourceSoundPropertyChanged(const am_sourceID_t sourceID, const am_MainSoundProperty_s & SoundProperty)
{
    log(&commandDbus, DLT_LOG_INFO, "cbMainSourceSoundPropertyChanged called, sourceID", sourceID, "SoundProperty.type", SoundProperty.type, "SoundProperty.value", SoundProperty.value);

    if (mReady)
    {
        mCAmDbusMessageHandler.initSignal(std::string(MY_NODE), std::string("MainSourceSoundPropertyChanged"));
        mCAmDbusMessageHandler.append((dbus_uint16_t) sourceID);
        mCAmDbusMessageHandler.append(SoundProperty);
        mCAmDbusMessageHandler.sendMessage();
    }
}

void CAmCommandSenderDbus::cbSinkAvailabilityChanged(const am_sinkID_t sinkID, const am_Availability_s & availability)
{
    log(&commandDbus, DLT_LOG_INFO, "cbSinkAvailabilityChanged called, sinkID", sinkID, "availability.availability", availability.availability, "SoundProperty.reason", availability.availabilityReason);

    if (mReady)
    {
        mCAmDbusMessageHandler.initSignal(std::string(MY_NODE), std::string("SinkAvailabilityChanged"));
        mCAmDbusMessageHandler.append((dbus_uint16_t) sinkID);
        mCAmDbusMessageHandler.append(availability);
        mCAmDbusMessageHandler.sendMessage();
    }
}

void CAmCommandSenderDbus::cbSourceAvailabilityChanged(const am_sourceID_t sourceID, const am_Availability_s & availability)
{
    log(&commandDbus, DLT_LOG_INFO, "cbSourceAvailabilityChanged called, sourceID", sourceID, "availability.availability", availability.availability, "SoundProperty.reason", availability.availabilityReason);

    if (mReady)
    {
        mCAmDbusMessageHandler.initSignal(std::string(MY_NODE), std::string("SourceAvailabilityChanged"));
        mCAmDbusMessageHandler.append((dbus_uint16_t) sourceID);
        mCAmDbusMessageHandler.append(availability);
        mCAmDbusMessageHandler.sendMessage();
    }
}

void CAmCommandSenderDbus::cbVolumeChanged(const am_sinkID_t sinkID, const am_mainVolume_t volume)
{
    log(&commandDbus, DLT_LOG_INFO, "cbVolumeChanged called, sinkID", sinkID, "volume", volume);

    if (mReady)
    {
        mCAmDbusMessageHandler.initSignal(std::string(MY_NODE), std::string("VolumeChanged"));
        mCAmDbusMessageHandler.append((dbus_uint16_t) sinkID);
        mCAmDbusMessageHandler.append((dbus_int16_t) volume);
        mCAmDbusMessageHandler.sendMessage();
    }
}

void CAmCommandSenderDbus::cbSinkMuteStateChanged(const am_sinkID_t sinkID, const am_MuteState_e muteState)
{
    log(&commandDbus, DLT_LOG_INFO, "cbSinkMuteStateChanged called, sinkID", sinkID, "muteState", muteState);

    if (mReady)
    {
        mCAmDbusMessageHandler.initSignal(std::string(MY_NODE), std::string("SinkMuteStateChanged"));
        mCAmDbusMessageHandler.append((dbus_uint16_t) sinkID);
        mCAmDbusMessageHandler.append((dbus_int16_t) muteState);
        mCAmDbusMessageHandler.sendMessage();
    }
}

void CAmCommandSenderDbus::cbSystemPropertyChanged(const am_SystemProperty_s & SystemProperty)
{
    log(&commandDbus, DLT_LOG_INFO, "cbSystemPropertyChanged called, SystemProperty.type", SystemProperty.type, "SystemProperty.value", SystemProperty.value);

    if (mReady)
    {
        mCAmDbusMessageHandler.initSignal(std::string(MY_NODE), std::string("SystemPropertyChanged"));
        mCAmDbusMessageHandler.append(SystemProperty);
        mCAmDbusMessageHandler.sendMessage();
    }
}

void am::CAmCommandSenderDbus::cbTimingInformationChanged(const am_mainConnectionID_t mainConnectionID, const am_timeSync_t time)
{
    log(&commandDbus, DLT_LOG_INFO, "cbTimingInformationChanged called, mainConnectionID=", mainConnectionID, "time=", time);

    if (mReady)
    {
        mCAmDbusMessageHandler.initSignal(std::string(MY_NODE), std::string("TimingInformationChanged"));
        mCAmDbusMessageHandler.append((dbus_uint16_t) mainConnectionID);
        mCAmDbusMessageHandler.append((dbus_int16_t) time);
        mCAmDbusMessageHandler.sendMessage();
    }
}

void CAmCommandSenderDbus::getInterfaceVersion(std::string & version) const
{
    version = CommandSendVersion;
}

