/*
 * sockethandlerTest.h
 *
 *  Created on: Dec 19, 2011
 *      Author: christian
 */

#ifndef SOCKETHANDLERTEST_H_
#define SOCKETHANDLERTEST_H_

#include <gtest/gtest.h>
#include "SocketHandler.h"

namespace am {

class fdCallBack
{
public:
	fdCallBack(SocketHandler *SocketHandler);
	virtual ~fdCallBack();
	void connectSocket(int fd,const short events,void * userData);
	void handleSocketData(int fd,const short events,void * userData);
	TSpecificPollCallback<fdCallBack> pSocketDataCallback;
	TSpecificPollCallback<fdCallBack> pSocketConnectionCallback;
private:
	int mSocketConnection;
	SocketHandler *mSocketHandler;
};

class timerCallBack
{
public:
	timerCallBack(SocketHandler *SocketHandler);
	virtual ~timerCallBack();
	void timer1Callback(SocketHandler::sh_timerHandle_t handle,void * userData);
	void timer2Callback(SocketHandler::sh_timerHandle_t handle,void * userData);
	void timer3Callback(SocketHandler::sh_timerHandle_t handle,void * userData);
	void timer4Callback(SocketHandler::sh_timerHandle_t handle,void * userData);
	TSpecificTimerCallback<timerCallBack> pTimer1Callback;
	TSpecificTimerCallback<timerCallBack> pTimer2Callback;
	TSpecificTimerCallback<timerCallBack> pTimer3Callback;
	TSpecificTimerCallback<timerCallBack> pTimer4Callback;
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
