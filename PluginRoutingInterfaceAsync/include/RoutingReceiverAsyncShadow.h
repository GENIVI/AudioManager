/**
 * Copyright (C) 2011, BMW AG
 *
 * GeniviAudioMananger AudioManagerDaemon
 *
 * \file testRoutingItnerfaceAsync.h
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

#ifndef ROUTINGRECEIVERASYNCSHADOW_H_
#define ROUTINGRECEIVERASYNCSHADOW_H_

#include <routing/RoutingReceiveInterface.h>
#include <SocketHandler.h>
#include <pthread.h>
#include <queue>

namespace am
{

/**
 * Threadsafe shadow of the RoutingReceiverInterface
 * Register and deregister Functions are sychronous so they do not show up here...
 */
class RoutingReceiverAsyncShadow
{
public:
    RoutingReceiverAsyncShadow();
    virtual ~RoutingReceiverAsyncShadow();
    void ackConnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_Error_e error);
    void ackDisconnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_Error_e error);
    void ackSetSinkVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error);
    void ackSetSourceVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error);
    void ackSetSourceState(const am_Handle_s handle, const am_Error_e error);
    void ackSetSinkSoundProperty(const am_Handle_s handle, const am_Error_e error);
    void ackSetSourceSoundProperty(const am_Handle_s handle, const am_Error_e error);
    void ackCrossFading(const am_Handle_s handle, const am_HotSink_e hotSink, const am_Error_e error);
    void ackSourceVolumeTick(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume);
    void ackSinkVolumeTick(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume);
    void hookInterruptStatusChange(const am_sourceID_t sourceID, const am_InterruptState_e interruptState);
    void hookSinkAvailablityStatusChange(const am_sinkID_t sinkID, const am_Availability_s& availability);
    void hookSourceAvailablityStatusChange(const am_sourceID_t sourceID, const am_Availability_s& availability);
    void hookDomainStateChange(const am_domainID_t domainID, const am_DomainState_e domainState);
    void hookTimingInformationChanged(const am_connectionID_t connectionID, const am_timeSync_t delay);

    am_Error_e setRoutingInterface(RoutingReceiveInterface *receiveInterface);
    void asyncMsgReceiver(const pollfd pollfd, const sh_pollHandle_t handle, void* userData);
    bool asyncDispatcher(const sh_pollHandle_t handle, void* userData);
    bool asyncChecker(const sh_pollHandle_t handle, void* userData);

    shPollFired_T<RoutingReceiverAsyncShadow> asyncMsgReceive;
    shPollDispatch_T<RoutingReceiverAsyncShadow> asyncDispatch;
    shPollCheck_T<RoutingReceiverAsyncShadow> asyncCheck;

private:
    enum msgID_e
    {
        MSG_ACKCONNECT, MSG_ACKDISCONNECT, MSG_ACKSETSINKVOLUMECHANGE, MSG_ACKSETSOURCEVOLUMECHANGE, MSG_ACKSETSOURCESTATE, MSG_ACKSETSINKSOUNDPROPERTY, MSG_ACKSETSOURCESOUNDPROPERTY, MSG_ACKCROSSFADING, MSG_ACKSOURCEVOLUMETICK, MSG_ACKSINKVOLUMETICK, MSG_HOOKINTERRUPTSTATUSCHANGE, MSG_HOOKSINKAVAILABLITYSTATUSCHANGE, MSG_HOOKSOURCEAVAILABLITYSTATUSCHANGE, MSG_HOOKDOMAINSTATECHANGE, MSG_HOOKTIMINGINFORMATIONCHANGED
    };

    struct a_connect_s
    {
        am_Handle_s handle;
        am_connectionID_t connectionID;
        am_Error_e error;
    };

    struct a_volume_s
    {
        am_Handle_s handle;
        am_volume_t volume;
        am_Error_e error;
    };

    struct a_handle_s
    {
        am_Handle_s handle;
        am_Error_e error;
    };

    struct a_crossfading_s
    {
        am_Handle_s handle;
        am_HotSink_e hotSink;
        am_Error_e error;
    };

    struct a_sourceVolumeTick_s
    {
        am_sourceID_t sourceID;
        am_Handle_s handle;
        am_volume_t volume;
    };

    struct a_sinkVolumeTick_s
    {
        am_sinkID_t sinkID;
        am_Handle_s handle;
        am_volume_t volume;
    };

    struct a_interruptStatusChange_s
    {
        am_sourceID_t sourceID;
        am_InterruptState_e interruptState;
    };

    struct a_sinkAvailability_s
    {
        am_sinkID_t sinkID;
        am_Availability_s availability;
    };

    struct a_sourceAvailability_s
    {
        am_sourceID_t sourceID;
        am_Availability_s availability;
    };

    struct a_hookDomainStateChange_s
    {
        am_domainID_t domainID;
        am_DomainState_e state;
    };

    struct a_timingInfoChanged_s
    {
        am_connectionID_t connectionID;
        am_timeSync_t delay;
    };

    union parameter_u
    {
        a_connect_s connect;
        a_volume_s volume;
        a_handle_s handle;
        a_crossfading_s crossfading;
        a_sourceVolumeTick_s sourceVolumeTick;
        a_sinkVolumeTick_s sinkVolumeTick;
        a_interruptStatusChange_s interruptStatusChange;
        a_sinkAvailability_s sinkAvailability;
        a_sourceAvailability_s sourceAvailability;
        a_hookDomainStateChange_s domainStateChange;
        a_timingInfoChanged_s timingInfoChange;
    };

    struct msg_s
    {
        msgID_e msgID;
        parameter_u parameters;
    };

    SocketHandler *mSocketHandler;
    RoutingReceiveInterface *mRoutingReceiveInterface;
    std::queue<msg_s> mQueue;
    static pthread_mutex_t mMutex;
    sh_pollHandle_t mHandle;
    int mPipe[2];
};

} /* namespace am */
#endif /* ROUTINGRECEIVERASYNCSHADOW_H_ */
