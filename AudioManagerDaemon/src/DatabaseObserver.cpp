/**
 * Copyright (C) 2011, BMW AG
 *
 * GeniviAudioMananger AudioManagerDaemon
 *
 * \file DatabaseObserver.cpp
 *
 * \date 20-Oct-2011 3:42:04 PM
 * \author Christian Mueller (christian.ei.mueller@bmw.de)
 *
 * \section License
 * GNU Lesser General Public License, version 2.1, with special exception (GENIVI clause)
 * Copyright (C) 2011, BMW AG Christian Mueller  Christian.ei.mueller@bmw.de
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License, version 2.1, for more details.
 * You should have received a copy of the GNU Lesser General Public License, version 2.1, along with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 * Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may also be applicable to programs even in cases in which the program is not a library in the technical sense.
 * Linking AudioManager statically or dynamically with other modules is making a combined work based on AudioManager. You may license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to license your linked modules under the GNU Lesser General Public License, version 2.1, you may use the program under the following exception.
 * As a special exception, the copyright holders of AudioManager give you permission to combine AudioManager with software programs or libraries that are released under any license unless such a combination is not permitted by the license of such a software program or library. You may copy and distribute such a system following the terms of the GNU Lesser General Public License, version 2.1, including this special exception, for AudioManager and the licenses of the other code concerned.
 * Note that people who make modified versions of AudioManager are not obligated to grant this special exception for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, version 2.1, gives permission to release a modified version without this exception; this exception also makes it possible to release a modified version which carries forward this exception.
 *
 */

#include "DatabaseObserver.h"
#include <string.h>
#include <cassert>
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include "CommandSender.h"
#include "RoutingSender.h"
#include "TelnetServer.h"
#include "DLTWrapper.h"

using namespace am;

DatabaseObserver::DatabaseObserver(CommandSender *iCommandSender, RoutingSender *iRoutingSender, SocketHandler *iSocketHandler) :
        receiverCallbackT(this, &DatabaseObserver::receiverCallback), //
        dispatcherCallbackT(this, &DatabaseObserver::dispatcherCallback), //
        checkerCallbackT(this, &DatabaseObserver::checkerCallback), //
        mCommandSender(iCommandSender), //
        mRoutingSender(iRoutingSender), //
        mSocketHandler(iSocketHandler), //
        mPipe(), //
        mHandle(), //
        mQueue()
{
    commonConstructor();
}

DatabaseObserver::DatabaseObserver(CommandSender *iCommandSender, RoutingSender *iRoutingSender, SocketHandler *iSocketHandler, TelnetServer *iTelnetServer) :
        receiverCallbackT(this, &DatabaseObserver::receiverCallback), //
        dispatcherCallbackT(this, &DatabaseObserver::dispatcherCallback), //
        checkerCallbackT(this, &DatabaseObserver::checkerCallback), //
        mCommandSender(iCommandSender), //
        mRoutingSender(iRoutingSender), //
        mTelnetServer(iTelnetServer), //
        mSocketHandler(iSocketHandler), //
        mPipe(), //
        mHandle(), //
        mQueue()
{
    assert(mTelnetServer!=0);
    commonConstructor();
}

void DatabaseObserver::pipeCommand(const do_msg_s & message)
{
    mQueue.push(message);

    if (write(mPipe[1], &message.msgID, sizeof(do_msg_s)) == -1)
    {
        logError("DatabaseObserver::msgID pipe write failed, error code:", strerror(errno));
    }
}

void DatabaseObserver::commonConstructor()
{
    assert(mCommandSender!=0);
    assert(mRoutingSender!=0);
    assert(mSocketHandler!=0);
    if (pipe(mPipe) == -1)
    {
        logError("RoutingReceiverAsyncShadow::setRoutingInterface could not create pipe!:");
        throw "could not create pipe";
    }

    short event = 0;
    event |= POLLIN;
    mSocketHandler->addFDPoll(mPipe[0], event, NULL, &receiverCallbackT, &checkerCallbackT, &dispatcherCallbackT, NULL, mHandle);
}

DatabaseObserver::~DatabaseObserver()
{
}

void DatabaseObserver::newSink(am_Sink_s sink)
{
    mRoutingSender->addSinkLookup(sink);
    do_msg_s msg;
    msg.msgID = MDO_cbNumberOfSinksChanged;
    pipeCommand(msg);

}

void DatabaseObserver::newSource(am_Source_s source)
{
    mRoutingSender->addSourceLookup(source);
    do_msg_s msg;
    msg.msgID = MDO_cbNumberOfSourcesChanged;
    pipeCommand(msg);
}

void DatabaseObserver::newDomain(am_Domain_s domain)
{
    mRoutingSender->addDomainLookup(domain);
}

void DatabaseObserver::newGateway(am_Gateway_s gateway)
{
    (void) gateway;
    //todo: implement something
}

void DatabaseObserver::newCrossfader(am_Crossfader_s crossfader)
{
    mRoutingSender->addCrossfaderLookup(crossfader);
}

void DatabaseObserver::removedSink(am_sinkID_t sinkID)
{
    mRoutingSender->removeSinkLookup(sinkID);
    do_msg_s msg;
    msg.msgID = MDO_cbNumberOfSinksChanged;
    pipeCommand(msg);
}

void DatabaseObserver::removedSource(am_sourceID_t sourceID)
{
    mRoutingSender->removeSourceLookup(sourceID);
    do_msg_s msg;
    msg.msgID = MDO_cbNumberOfSourcesChanged;
    pipeCommand(msg);
}

void DatabaseObserver::removeDomain(am_domainID_t domainID)
{
    mRoutingSender->removeDomainLookup(domainID);
}

void DatabaseObserver::removeGateway(am_gatewayID_t gatewayID)
{
    (void) gatewayID;
    //todo: implement something
}

void DatabaseObserver::removeCrossfader(am_crossfaderID_t crossfaderID)
{
    mRoutingSender->removeCrossfaderLookup(crossfaderID);
}

void DatabaseObserver::numberOfMainConnectionsChanged()
{
    do_msg_s msg;
    msg.msgID = MDO_cbNumberOfMainConnectionsChanged;
    pipeCommand(msg);
}

void DatabaseObserver::numberOfSinkClassesChanged()
{
    do_msg_s msg;
    msg.msgID = MDO_cbNumberOfSinkClassesChanged;
    pipeCommand(msg);
}

void DatabaseObserver::numberOfSourceClassesChanged()
{
    do_msg_s msg;
    msg.msgID = MDO_cbNumberOfSourceClassesChanged;
    pipeCommand(msg);
}

void DatabaseObserver::mainConnectionStateChanged(const am_mainConnectionID_t connectionID, const am_ConnectionState_e connectionState)
{
    do_msg_s msg;
    msg.msgID = MDO_cbMainConnectionStateChanged;
    msg.parameters.connectionStateChanged.connectionID = connectionID;
    msg.parameters.connectionStateChanged.connectionState = connectionState;
    pipeCommand(msg);
}

void DatabaseObserver::mainSinkSoundPropertyChanged(const am_sinkID_t sinkID, const am_MainSoundProperty_s SoundProperty)
{
    do_msg_s msg;
    msg.msgID = MDO_cbMainSinkSoundPropertyChanged;
    msg.parameters.mainSinkSoundPropertyChanged.sinkID = sinkID;
    msg.parameters.mainSinkSoundPropertyChanged.SoundProperty = SoundProperty;
    pipeCommand(msg);
}

void DatabaseObserver::mainSourceSoundPropertyChanged(const am_sourceID_t sourceID, const am_MainSoundProperty_s & SoundProperty)
{
    do_msg_s msg;
    msg.msgID = MDO_cbMainSourceSoundPropertyChanged;
    msg.parameters.mainSourceSoundPropertyChanged.sourceID = sourceID;
    msg.parameters.mainSourceSoundPropertyChanged.SoundProperty = SoundProperty;
    pipeCommand(msg);
}

void DatabaseObserver::sinkAvailabilityChanged(const am_sinkID_t sinkID, const am_Availability_s & availability)
{
    do_msg_s msg;
    msg.msgID = MDO_cbSinkAvailabilityChanged;
    msg.parameters.sinkAvailabilityChanged.sinkID = sinkID;
    msg.parameters.sinkAvailabilityChanged.availability = availability;
    pipeCommand(msg);
}

void DatabaseObserver::sourceAvailabilityChanged(const am_sourceID_t sourceID, const am_Availability_s & availability)
{
    do_msg_s msg;
    msg.msgID = MDO_cbSourceAvailabilityChanged;
    msg.parameters.sourceAvailabilityChanged.sourceID = sourceID;
    msg.parameters.sourceAvailabilityChanged.availability = availability;
    pipeCommand(msg);
}

void DatabaseObserver::volumeChanged(const am_sinkID_t sinkID, const am_mainVolume_t volume)
{
    do_msg_s msg;
    msg.msgID = MDO_cbVolumeChanged;
    msg.parameters.volumeChanged.sinkID = sinkID;
    msg.parameters.volumeChanged.volume = volume;
    pipeCommand(msg);
}

void DatabaseObserver::sinkMuteStateChanged(const am_sinkID_t sinkID, const am_MuteState_e muteState)
{
    do_msg_s msg;
    msg.msgID = MDO_cbSinkMuteStateChanged;
    msg.parameters.sinkMuteStateChanged.sinkID = sinkID;
    msg.parameters.sinkMuteStateChanged.muteState = muteState;
    pipeCommand(msg);
}

void DatabaseObserver::systemPropertyChanged(const am_SystemProperty_s & SystemProperty)
{
    do_msg_s msg;
    msg.msgID = MDO_cbSystemPropertyChanged;
    msg.parameters.systemProperty = SystemProperty;
    pipeCommand(msg);
}

void DatabaseObserver::timingInformationChanged(const am_mainConnectionID_t mainConnection, const am_timeSync_t time)
{
    do_msg_s msg;
    msg.msgID = MDO_cbTimingInformationChanged;
    msg.parameters.timingInformationChanged.mainConnection = mainConnection;
    msg.parameters.timingInformationChanged.time = time;
    pipeCommand(msg);
    //todo:inform the controller via controlsender
}

void DatabaseObserver::receiverCallback(const pollfd pollfd, const sh_pollHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    //it is no really important what to read here, we will read the queue later...
    char buffer[10];
    if (read(pollfd.fd, buffer, 10) == -1)
    {
        logError("DatabaseObserver::receiverCallback could not read pipe!");
    }
}

bool DatabaseObserver::dispatcherCallback(const sh_pollHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    do_msg_s msg;

    msg = mQueue.front();
    mQueue.pop();

    switch (msg.msgID)
    {
    case MDO_cbNumberOfSinksChanged:
        mCommandSender->cbNumberOfSinksChanged();
        break;
    case MDO_cbNumberOfSourcesChanged:
        mCommandSender->cbNumberOfSourcesChanged();
        break;
    case MDO_cbNumberOfMainConnectionsChanged:
        mCommandSender->cbNumberOfMainConnectionsChanged();
        break;
    case MDO_cbNumberOfSinkClassesChanged:
        mCommandSender->cbNumberOfSinkClassesChanged();
        break;
    case MDO_cbNumberOfSourceClassesChanged:
        mCommandSender->cbNumberOfSourceClassesChanged();
        break;
    case MDO_cbMainConnectionStateChanged:
        mCommandSender->cbMainConnectionStateChanged(msg.parameters.connectionStateChanged.connectionID, msg.parameters.connectionStateChanged.connectionState);
        break;
    case MDO_cbMainSinkSoundPropertyChanged:
        mCommandSender->cbMainSinkSoundPropertyChanged(msg.parameters.mainSinkSoundPropertyChanged.sinkID, msg.parameters.mainSinkSoundPropertyChanged.SoundProperty);
        break;
    case MDO_cbMainSourceSoundPropertyChanged:
        mCommandSender->cbMainSourceSoundPropertyChanged(msg.parameters.mainSourceSoundPropertyChanged.sourceID, msg.parameters.mainSourceSoundPropertyChanged.SoundProperty);
        break;
    case MDO_cbSinkAvailabilityChanged:
        mCommandSender->cbSinkAvailabilityChanged(msg.parameters.sinkAvailabilityChanged.sinkID, msg.parameters.sinkAvailabilityChanged.availability);
        break;
    case MDO_cbSourceAvailabilityChanged:
        mCommandSender->cbSourceAvailabilityChanged(msg.parameters.sourceAvailabilityChanged.sourceID, msg.parameters.sourceAvailabilityChanged.availability);
        break;
    case MDO_cbVolumeChanged:
        mCommandSender->cbVolumeChanged(msg.parameters.volumeChanged.sinkID, msg.parameters.volumeChanged.volume);
        break;
    case MDO_cbSinkMuteStateChanged:
        mCommandSender->cbSinkMuteStateChanged(msg.parameters.sinkMuteStateChanged.sinkID, msg.parameters.sinkMuteStateChanged.muteState);
        break;
    case MDO_cbSystemPropertyChanged:
        mCommandSender->cbSystemPropertyChanged(msg.parameters.systemProperty);
        break;
    case MDO_cbTimingInformationChanged:
        mCommandSender->cbTimingInformationChanged(msg.parameters.timingInformationChanged.mainConnection, msg.parameters.timingInformationChanged.time);
        break;
    default:
        logError("Something went totally wrong in DatabaseObserver::dispatcherCallback");
        break;
    }

    if (mQueue.size() > 0)
        return (true);
    return (false);
}

bool DatabaseObserver::checkerCallback(const sh_pollHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    if (mQueue.size() > 0)
        return (true);
    return (false);
}

