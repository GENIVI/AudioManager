/**
 * Copyright (C) 2011, BMW AG
 *
 * GeniviAudioMananger AudioManagerDaemon
 *
 * \file RoutingReceiverAsyncShadow.cpp
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

#include "RoutingReceiverAsyncShadow.h"
#include <cassert>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/un.h>
#include <errno.h>
#include <string>
#include "DLTWrapper.h"

using namespace am;

pthread_mutex_t RoutingReceiverAsyncShadow::mMutex = PTHREAD_MUTEX_INITIALIZER;

RoutingReceiverAsyncShadow::RoutingReceiverAsyncShadow():
asyncMsgReceive(this, &RoutingReceiverAsyncShadow::asyncMsgReceiver), //
        asyncDispatch(this, &RoutingReceiverAsyncShadow::asyncDispatcher), //
        asyncCheck(this, &RoutingReceiverAsyncShadow::asyncChecker), //
        mSocketHandler(), //
        mRoutingReceiveInterface(), //
mHandle (), //
mPipe()
{

}

RoutingReceiverAsyncShadow::~RoutingReceiverAsyncShadow()
{
}

void RoutingReceiverAsyncShadow::ackConnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_Error_e error)
{
    assert(mPipe[0]!=0);
    //put the data in the queue:
    a_connect_s temp;
    temp.handle = handle;
    temp.connectionID = connectionID;
    temp.error = error;
    //then we make a message out of it:
    msg_s msg;
    msg.msgID = MSG_ACKCONNECT;
    msg.parameters.connect = temp;
    //here we share data !
    pthread_mutex_lock(&mMutex);
    mQueue.push(msg);
    pthread_mutex_unlock(&mMutex);

    //ok, fire the signal that data needs to be received !
    if (write(mPipe[1], &msg.msgID, sizeof(msgID_e)) == -1)
    {
        logError("RoutingReceiverAsyncShadow::ackConnect write failed, error code:", strerror(errno));
    }
}

void RoutingReceiverAsyncShadow::ackDisconnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_Error_e error)
{
    assert(mPipe[0]!=0);
    //put the data in the queue:
    a_connect_s temp;
    temp.handle = handle;
    temp.connectionID = connectionID;
    temp.error = error;
    //then we make a message out of it:
    msg_s msg;
    msg.msgID = MSG_ACKDISCONNECT;
    msg.parameters.connect = temp;
    //here we share data !
    pthread_mutex_lock(&mMutex);
    mQueue.push(msg);
    pthread_mutex_unlock(&mMutex);

    //ok, fire the signal that data needs to be received !
    if (write(mPipe[1], &msg.msgID, sizeof(msgID_e)) == -1)
    {
        logError("RoutingReceiverAsyncShadow::ackDisconnect write failed, error code:", strerror(errno));
    }
}

void RoutingReceiverAsyncShadow::ackSetSinkVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error)
{
    assert(mPipe[0]!=0);
    //put the data in the queue:
    a_volume_s temp;
    temp.handle = handle;
    temp.volume = volume;
    temp.error = error;
    //then we make a message out of it:
    msg_s msg;
    msg.msgID = MSG_ACKSETSINKVOLUMECHANGE;
    msg.parameters.volume = temp;
    //here we share data !
    pthread_mutex_lock(&mMutex);
    mQueue.push(msg);
    pthread_mutex_unlock(&mMutex);

    //ok, fire the signal that data needs to be received !
    if (write(mPipe[1], &msg.msgID, sizeof(msgID_e)) == -1)
    {
        logError("RoutingReceiverAsyncShadow::ackSetSinkVolumeChange write failed, error code:", strerror(errno));
    }
}

void RoutingReceiverAsyncShadow::ackSetSourceVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error)
{
    assert(mPipe[0]!=0);
    //put the data in the queue:
    a_volume_s temp;
    temp.handle = handle;
    temp.volume = volume;
    temp.error = error;
    //then we make a message out of it:
    msg_s msg;
    msg.msgID = MSG_ACKSETSOURCEVOLUMECHANGE;
    msg.parameters.volume = temp;
    //here we share data !
    pthread_mutex_lock(&mMutex);
    mQueue.push(msg);
    pthread_mutex_unlock(&mMutex);

    //ok, fire the signal that data needs to be received !
    if (write(mPipe[1], &msg.msgID, sizeof(msgID_e)) == -1)
    {
        logError("RoutingReceiverAsyncShadow::ackSetSourceVolumeChange write failed, error code:", strerror(errno));
    }
}

void RoutingReceiverAsyncShadow::ackSetSourceState(const am_Handle_s handle, const am_Error_e error)
{
    assert(mPipe[0]!=0);
    //put the data in the queue:
    a_handle_s temp;
    temp.handle = handle;
    temp.error = error;
    //then we make a message out of it:
    msg_s msg;
    msg.msgID = MSG_ACKSETSOURCESTATE;
    msg.parameters.handle = temp;
    //here we share data !
    pthread_mutex_lock(&mMutex);
    mQueue.push(msg);
    pthread_mutex_unlock(&mMutex);

    //ok, fire the signal that data needs to be received !
    if (write(mPipe[1], &msg.msgID, sizeof(msgID_e)) == -1)
    {
        logError("RoutingReceiverAsyncShadow::ackSetSourceState write failed, error code:", strerror(errno));
    }
}

void RoutingReceiverAsyncShadow::ackSetSinkSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
    assert(mPipe[0]!=0);
    //put the data in the queue:
    a_handle_s temp;
    temp.handle = handle;
    temp.error = error;
    //then we make a message out of it:
    msg_s msg;
    msg.msgID = MSG_ACKSETSINKSOUNDPROPERTY;
    msg.parameters.handle = temp;
    //here we share data !
    pthread_mutex_lock(&mMutex);
    mQueue.push(msg);
    pthread_mutex_unlock(&mMutex);

    //ok, fire the signal that data needs to be received !
    if (write(mPipe[1], &msg.msgID, sizeof(msgID_e)) == -1)
    {
        logError("RoutingReceiverAsyncShadow::ackSetSinkSoundProperty write failed, error code:", strerror(errno));
    }
}

void RoutingReceiverAsyncShadow::ackSetSourceSoundProperty(const am_Handle_s handle, const am_Error_e error)
{
    assert(mPipe[0]!=0);
    //put the data in the queue:
    a_handle_s temp;
    temp.handle = handle;
    temp.error = error;
    //then we make a message out of it:
    msg_s msg;
    msg.msgID = MSG_ACKSETSOURCESOUNDPROPERTY;
    msg.parameters.handle = temp;
    //here we share data !
    pthread_mutex_lock(&mMutex);
    mQueue.push(msg);
    pthread_mutex_unlock(&mMutex);

    //ok, fire the signal that data needs to be received !
    if (write(mPipe[1], &msg.msgID, sizeof(msgID_e)) == -1)
    {
        logError("RoutingReceiverAsyncShadow::ackSetSourceSoundProperty write failed, error code:", strerror(errno));
    }
}

void RoutingReceiverAsyncShadow::ackCrossFading(const am_Handle_s handle, const am_HotSink_e hotSink, const am_Error_e error)
{
    assert(mPipe[0]!=0);
    //put the data in the queue:
    a_crossfading_s temp;
    temp.handle = handle;
    temp.hotSink = hotSink;
    temp.error = error;
    //then we make a message out of it:
    msg_s msg;
    msg.msgID = MSG_ACKCROSSFADING;
    msg.parameters.crossfading = temp;
    //here we share data !
    pthread_mutex_lock(&mMutex);
    mQueue.push(msg);
    pthread_mutex_unlock(&mMutex);

    //ok, fire the signal that data needs to be received !
    if (write(mPipe[1], &msg.msgID, sizeof(msgID_e)) == -1)
    {
        logError("RoutingReceiverAsyncShadow::ackCrossFading write failed, error code:", strerror(errno));
    }
}

void RoutingReceiverAsyncShadow::ackSourceVolumeTick(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume)
{
    assert(mPipe[0]!=0);
    //put the data in the queue:
    a_sourceVolumeTick_s temp;
    temp.sourceID = sourceID;
    temp.handle = handle;
    temp.volume = volume;
    //then we make a message out of it:
    msg_s msg;
    msg.msgID = MSG_ACKSOURCEVOLUMETICK;
    msg.parameters.sourceVolumeTick = temp;
    //here we share data !
    pthread_mutex_lock(&mMutex);
    mQueue.push(msg);
    pthread_mutex_unlock(&mMutex);

    //ok, fire the signal that data needs to be received !
    if (write(mPipe[1], &msg.msgID, sizeof(msgID_e)) == -1)
    {
        logError("RoutingReceiverAsyncShadow::ackSourceVolumeTick write failed, error code:", strerror(errno));
    }
}

void RoutingReceiverAsyncShadow::ackSinkVolumeTick(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume)
{
    assert(mPipe[0]!=0);
    //put the data in the queue:
    a_sinkVolumeTick_s temp;
    temp.sinkID = sinkID;
    temp.handle = handle;
    temp.volume = volume;
    //then we make a message out of it:
    msg_s msg;
    msg.msgID = MSG_ACKSINKVOLUMETICK;
    msg.parameters.sinkVolumeTick = temp;
    //here we share data !
    pthread_mutex_lock(&mMutex);
    mQueue.push(msg);
    pthread_mutex_unlock(&mMutex);

    //ok, fire the signal that data needs to be received !
    if (write(mPipe[1], &msg.msgID, sizeof(msgID_e)) == -1)
    {
        logError("RoutingReceiverAsyncShadow::ackSinkVolumeTick write failed, error code:", strerror(errno));
    }
}

void RoutingReceiverAsyncShadow::hookInterruptStatusChange(const am_sourceID_t sourceID, const am_InterruptState_e interruptState)
{
    assert(mPipe[0]!=0);
    //put the data in the queue:
    a_interruptStatusChange_s temp;
    temp.sourceID = sourceID;
    temp.interruptState = interruptState;

    //then we make a message out of it:
    msg_s msg;
    msg.msgID = MSG_HOOKINTERRUPTSTATUSCHANGE;
    msg.parameters.interruptStatusChange = temp;
    //here we share data !
    pthread_mutex_lock(&mMutex);
    mQueue.push(msg);
    pthread_mutex_unlock(&mMutex);

    //ok, fire the signal that data needs to be received !
    if (write(mPipe[1], &msg.msgID, sizeof(msgID_e)) == -1)
    {
        logError("RoutingReceiverAsyncShadow::hookInterruptStatusChange write failed, error code:", strerror(errno));
    }
}

void RoutingReceiverAsyncShadow::hookSinkAvailablityStatusChange(const am_sinkID_t sinkID, const am_Availability_s & availability)
{
    assert(mPipe[0]!=0);
    //put the data in the queue:
    a_sinkAvailability_s temp;
    temp.sinkID = sinkID;
    temp.availability = availability;

    //then we make a message out of it:
    msg_s msg;
    msg.msgID = MSG_HOOKSINKAVAILABLITYSTATUSCHANGE;
    msg.parameters.sinkAvailability = temp;
    //here we share data !
    pthread_mutex_lock(&mMutex);
    mQueue.push(msg);
    pthread_mutex_unlock(&mMutex);

    //ok, fire the signal that data needs to be received !
    if (write(mPipe[1], &msg.msgID, sizeof(msgID_e)) == -1)
    {
        logError("RoutingReceiverAsyncShadow::hookSinkAvailablityStatusChange write failed, error code:", strerror(errno));
    }
}

void RoutingReceiverAsyncShadow::hookSourceAvailablityStatusChange(const am_sourceID_t sourceID, const am_Availability_s & availability)
{
    assert(mPipe[0]!=0);
    //put the data in the queue:
    a_sourceAvailability_s temp;
    temp.sourceID = sourceID;
    temp.availability = availability;

    //then we make a message out of it:
    msg_s msg;
    msg.msgID = MSG_HOOKSOURCEAVAILABLITYSTATUSCHANGE;
    msg.parameters.sourceAvailability = temp;
    //here we share data !
    pthread_mutex_lock(&mMutex);
    mQueue.push(msg);
    pthread_mutex_unlock(&mMutex);

    //ok, fire the signal that data needs to be received !
    if (write(mPipe[1], &msg.msgID, sizeof(msgID_e)) == -1)
    {
        logError("RoutingReceiverAsyncShadow::hookSourceAvailablityStatusChange write failed, error code:", strerror(errno));
    }
}

void RoutingReceiverAsyncShadow::hookDomainStateChange(const am_domainID_t domainID, const am_DomainState_e domainState)
{
    assert(mPipe[0]!=0);
    //put the data in the queue:
    a_hookDomainStateChange_s temp;
    temp.domainID = domainID;
    temp.state = domainState;

    //then we make a message out of it:
    msg_s msg;
    msg.msgID = MSG_HOOKDOMAINSTATECHANGE;
    msg.parameters.domainStateChange = temp;
    //here we share data !
    pthread_mutex_lock(&mMutex);
    mQueue.push(msg);
    pthread_mutex_unlock(&mMutex);

    //ok, fire the signal that data needs to be received !
    if (write(mPipe[1], &msg.msgID, sizeof(msgID_e)) == -1)
    {
        logError("RoutingReceiverAsyncShadow::hookDomainStateChange write failed, error code:", strerror(errno));
    }
}

void RoutingReceiverAsyncShadow::hookTimingInformationChanged(const am_connectionID_t connectionID, const am_timeSync_t delay)
{
    assert(mPipe[0]!=0);
    //put the data in the queue:
    a_timingInfoChanged_s temp;
    temp.connectionID = connectionID;
    temp.delay = delay;

    //then we make a message out of it:
    msg_s msg;
    msg.msgID = MSG_HOOKTIMINGINFORMATIONCHANGED;
    msg.parameters.timingInfoChange = temp;
    //here we share data !
    pthread_mutex_lock(&mMutex);
    mQueue.push(msg);
    pthread_mutex_unlock(&mMutex);

    //ok, fire the signal that data needs to be received !
    if (write(mPipe[1], &msg.msgID, sizeof(msgID_e)) == -1)
    {
        logError("RoutingReceiverAsyncShadow::hookTimingInformationChanged write failed, error code:", strerror(errno));
    }
}

void RoutingReceiverAsyncShadow::asyncMsgReceiver(const pollfd pollfd, const sh_pollHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    //it is no really important what to read here, we will read the queue later...
    char buffer[10];
    if (read(pollfd.fd, buffer, 10) == -1)
    {
        logError("RoutingReceiverAsyncShadow::asyncMsgReceiver could not read!");
    }
}

bool RoutingReceiverAsyncShadow::asyncDispatcher(const sh_pollHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    msg_s msg;

    //ok, let's receive, first lock
    pthread_mutex_lock(&mMutex);
    msg = mQueue.front();
    mQueue.pop();
    pthread_mutex_unlock(&mMutex);

    //check for the message:
    switch (msg.msgID)
    {
    case MSG_ACKCONNECT:
        mRoutingReceiveInterface->ackConnect(msg.parameters.connect.handle, msg.parameters.connect.connectionID, msg.parameters.connect.error);
        break;
    case MSG_ACKDISCONNECT:
        mRoutingReceiveInterface->ackDisconnect(msg.parameters.connect.handle, msg.parameters.connect.connectionID, msg.parameters.connect.error);
        break;
    case MSG_ACKSETSINKVOLUMECHANGE:
        mRoutingReceiveInterface->ackSetSinkVolumeChange(msg.parameters.volume.handle, msg.parameters.volume.volume, msg.parameters.volume.error);
        break;
    case MSG_ACKSETSOURCEVOLUMECHANGE:
        mRoutingReceiveInterface->ackSetSourceVolumeChange(msg.parameters.volume.handle, msg.parameters.volume.volume, msg.parameters.volume.error);
        break;
    case MSG_ACKSETSOURCESTATE:
        mRoutingReceiveInterface->ackSetSourceState(msg.parameters.handle.handle, msg.parameters.handle.error);
        break;
    case MSG_ACKSETSINKSOUNDPROPERTY:
        mRoutingReceiveInterface->ackSetSinkSoundProperty(msg.parameters.handle.handle, msg.parameters.handle.error);
        break;
    case MSG_ACKSETSOURCESOUNDPROPERTY:
        mRoutingReceiveInterface->ackSetSourceSoundProperty(msg.parameters.handle.handle, msg.parameters.handle.error);
        break;
    case MSG_ACKCROSSFADING:
        mRoutingReceiveInterface->ackCrossFading(msg.parameters.crossfading.handle, msg.parameters.crossfading.hotSink, msg.parameters.crossfading.error);
        break;
    case MSG_ACKSOURCEVOLUMETICK:
        mRoutingReceiveInterface->ackSourceVolumeTick(msg.parameters.sourceVolumeTick.handle, msg.parameters.sourceVolumeTick.sourceID, msg.parameters.sourceVolumeTick.volume);
        break;
    case MSG_ACKSINKVOLUMETICK:
        mRoutingReceiveInterface->ackSinkVolumeTick(msg.parameters.sinkVolumeTick.handle, msg.parameters.sinkVolumeTick.sinkID, msg.parameters.sinkVolumeTick.volume);
        break;
    case MSG_HOOKINTERRUPTSTATUSCHANGE:
        mRoutingReceiveInterface->hookInterruptStatusChange(msg.parameters.interruptStatusChange.sourceID, msg.parameters.interruptStatusChange.interruptState);
        break;
    case MSG_HOOKSINKAVAILABLITYSTATUSCHANGE:
        mRoutingReceiveInterface->hookSinkAvailablityStatusChange(msg.parameters.sinkAvailability.sinkID, msg.parameters.sinkAvailability.availability);
        break;
    case MSG_HOOKSOURCEAVAILABLITYSTATUSCHANGE:
        mRoutingReceiveInterface->hookSourceAvailablityStatusChange(msg.parameters.sourceAvailability.sourceID, msg.parameters.sourceAvailability.availability);
        break;
    case MSG_HOOKDOMAINSTATECHANGE:
        mRoutingReceiveInterface->hookDomainStateChange(msg.parameters.domainStateChange.domainID, msg.parameters.domainStateChange.state);
        break;
    case MSG_HOOKTIMINGINFORMATIONCHANGED:
        mRoutingReceiveInterface->hookTimingInformationChanged(msg.parameters.timingInfoChange.connectionID, msg.parameters.timingInfoChange.delay);
        break;
    default:
        logError("RoutingReceiverAsyncShadow::asyncDispatcher unknown message was received:", msg.msgID);
        break;
    }

    bool retVal = false;
    pthread_mutex_lock(&mMutex);
    if (mQueue.size() > 0)
        retVal = true;
    pthread_mutex_unlock(&mMutex);

    return (retVal);
}

bool RoutingReceiverAsyncShadow::asyncChecker(const sh_pollHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    bool returnVal = false;
    pthread_mutex_lock(&mMutex);
    if (mQueue.size() > 0)
        returnVal = true;
    pthread_mutex_unlock(&mMutex);
    return (returnVal);
}

am_Error_e RoutingReceiverAsyncShadow::setRoutingInterface(RoutingReceiveInterface *receiveInterface)
{
    assert(receiveInterface!=0);
    mRoutingReceiveInterface = receiveInterface;
    mRoutingReceiveInterface->getSocketHandler(mSocketHandler);
    if (pipe(mPipe) == -1)
    {
        logError("RoutingReceiverAsyncShadow::setRoutingInterface could not create pipe!:");
        return (E_UNKNOWN);
    }

    short event = 0;
    event |= POLLIN;
    mSocketHandler->addFDPoll(mPipe[0], event, NULL, &asyncMsgReceive, &asyncCheck, &asyncDispatch, NULL, mHandle);
    return (E_OK);
}
