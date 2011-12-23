/*
 * sockethandlerTest.h
 *
 *  Created on: Dec 19, 2011
 *      Author: christian
 */

#ifndef SOCKETHANDLERTEST_H_
#define SOCKETHANDLERTEST_H_

#include <gtest/gtest.h>
#include <queue>
#include "SocketHandler.h"

namespace am {

class SamplePlugin
{
public:
	SamplePlugin(SocketHandler *mySocketHandler);
	virtual ~SamplePlugin() {};
	void connectSocket(const pollfd pollfd,const sh_pollHandle_t handle, void* userData);
	void receiveData(const pollfd pollfd,const sh_pollHandle_t handle, void* userData);
	bool dispatchData(const sh_pollHandle_t handle, void* userData);
	bool check(const sh_pollHandle_t handle, void* userData);
	shPollFired_T<SamplePlugin> connectFiredCB;
	shPollFired_T<SamplePlugin> receiveFiredCB;
	shPollDispatch_T<SamplePlugin> sampleDispatchCB;
	shPollCheck_T<SamplePlugin> sampleCheckCB;
private:
	SocketHandler *mSocketHandler;
	sh_pollHandle_t mConnecthandle,mReceiveHandle;
	std::queue<std::string> msgList;
};


class timerCallBack
{
public:
	timerCallBack(SocketHandler *SocketHandler);
	virtual ~timerCallBack();
	void timer1Callback(sh_timerHandle_t handle,void * userData);
	void timer2Callback(sh_timerHandle_t handle,void * userData);
	void timer3Callback(sh_timerHandle_t handle,void * userData);
	void timer4Callback(sh_timerHandle_t handle,void * userData);
	shTimerCallBack_T<timerCallBack> pTimer1Callback;
	shTimerCallBack_T<timerCallBack> pTimer2Callback;
	shTimerCallBack_T<timerCallBack> pTimer3Callback;
	shTimerCallBack_T<timerCallBack> pTimer4Callback;
	SocketHandler *mSocketHandler;
};

class sockethandlerTest: public ::testing::Test
{
public:
	sockethandlerTest();
	virtual ~sockethandlerTest();
	void SetUp();
	void TearDown();
};

} /* namespace am */
#endif /* SOCKETHANDLERTEST_H_ */
