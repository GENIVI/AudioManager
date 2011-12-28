/*
 * RoutingReceiverAsyncShadow.h
 *
 *  Created on: Dec 23, 2011
 *      Author: christian
 */

#ifndef ROUTINGRECEIVERASYNCSHADOW_H_
#define ROUTINGRECEIVERASYNCSHADOW_H_

#include <routing/RoutingReceiveInterface.h>
#include <SocketHandler.h>
#include <pthread.h>
#include <queue>

namespace am {

/**
 * Threadsafe shadow of the RoutingReceiverInterface
 * Register and deregister Functions are sychronous so they do not show up here...
 */
class RoutingReceiverAsyncShadow
{
public:
	RoutingReceiverAsyncShadow();
	virtual ~RoutingReceiverAsyncShadow();
	void ackConnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_Error_e error) ;
	void ackDisconnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_Error_e error) ;
	void ackSetSinkVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error) ;
	void ackSetSourceVolumeChange(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error) ;
	void ackSetSourceState(const am_Handle_s handle, const am_Error_e error) ;
	void ackSetSinkSoundProperty(const am_Handle_s handle, const am_Error_e error) ;
	void ackSetSourceSoundProperty(const am_Handle_s handle, const am_Error_e error) ;
	void ackCrossFading(const am_Handle_s handle, const am_HotSink_e hotSink, const am_Error_e error) ;
	void ackSourceVolumeTick(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume) ;
	void ackSinkVolumeTick(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume) ;
	void hookInterruptStatusChange(const am_sourceID_t sourceID, const am_InterruptState_e interruptState) ;
	void hookSinkAvailablityStatusChange(const am_sinkID_t sinkID, const am_Availability_s& availability) ;
	void hookSourceAvailablityStatusChange(const am_sourceID_t sourceID, const am_Availability_s& availability) ;
	void hookDomainStateChange(const am_domainID_t domainID, const am_DomainState_e domainState) ;
	void hookTimingInformationChanged(const am_connectionID_t connectionID, const am_timeSync_t delay) ;

	am_Error_e setRoutingInterface(RoutingReceiveInterface *receiveInterface);
	void asyncMsgReceiver(const pollfd pollfd,const sh_pollHandle_t handle, void* userData);
	bool asyncDispatcher(const sh_pollHandle_t handle, void* userData);
	bool asyncChecker(const sh_pollHandle_t handle, void* userData);

	shPollFired_T<RoutingReceiverAsyncShadow> asyncMsgReceive;
	shPollDispatch_T<RoutingReceiverAsyncShadow> asyncDispatch;
	shPollCheck_T<RoutingReceiverAsyncShadow> asyncCheck;

private:
	enum msgID_e
	{
		MSG_ACKCONNECT,
		MSG_ACKDISCONNECT,
		MSG_ACKSETSINKVOLUMECHANGE,
		MSG_ACKSETSOURCEVOLUMECHANGE,
		MSG_ACKSETSOURCESTATE,
		MSG_ACKSETSINKSOUNDPROPERTY,
		MSG_ACKSETSOURCESOUNDPROPERTY,
		MSG_ACKCROSSFADING,
		MSG_ACKSOURCEVOLUMETICK,
		MSG_ACKSINKVOLUMETICK,
		MSG_HOOKINTERRUPTSTATUSCHANGE,
		MSG_HOOKSINKAVAILABLITYSTATUSCHANGE,
		MSG_HOOKSOURCEAVAILABLITYSTATUSCHANGE,
		MSG_HOOKDOMAINSTATECHANGE,
		MSG_HOOKTIMINGINFORMATIONCHANGED
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
