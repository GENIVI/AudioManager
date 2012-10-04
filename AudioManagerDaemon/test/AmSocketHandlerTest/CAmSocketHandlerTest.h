/**
 * Copyright (C) 2012, BMW AG
 *
 * This file is part of GENIVI Project AudioManager.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
 *
 * For further information see http://www.genivi.org/.
 *
 */

#ifndef SOCKETHANDLERTEST_H_
#define SOCKETHANDLERTEST_H_

#include "gtest/gtest.h"
#include <queue>
#include "shared/CAmSocketHandler.h"

namespace am
{

class CAmSamplePlugin
{
public:
    enum sockType_e
    {
        UNIX, INET
    };
    CAmSamplePlugin(CAmSocketHandler *mySocketHandler, sockType_e socketType);
    ~CAmSamplePlugin()
    {
    }
    ;
    void connectSocket(const pollfd pollfd, const sh_pollHandle_t handle, void* userData);
    void receiveData(const pollfd pollfd, const sh_pollHandle_t handle, void* userData);
    bool dispatchData(const sh_pollHandle_t handle, void* userData);
    bool check(const sh_pollHandle_t handle, void* userData);
    TAmShPollFired<CAmSamplePlugin> connectFiredCB;
    TAmShPollFired<CAmSamplePlugin> receiveFiredCB;
    TAmShPollDispatch<CAmSamplePlugin> sampleDispatchCB;
    TAmShPollCheck<CAmSamplePlugin> sampleCheckCB;
private:
    CAmSocketHandler *mSocketHandler;
    sh_pollHandle_t mConnecthandle, mReceiveHandle;
    std::queue<std::string> msgList;
};

class CAmTimerCb
{
public:
    CAmTimerCb(CAmSocketHandler *SocketHandler);
    virtual ~CAmTimerCb();
    void timer1Callback(sh_timerHandle_t handle, void * userData);
    void timer2Callback(sh_timerHandle_t handle, void * userData);
    void timer3Callback(sh_timerHandle_t handle, void * userData);
    void timer4Callback(sh_timerHandle_t handle, void * userData);
    TAmShTimerCallBack<CAmTimerCb> pTimer1Callback;
    TAmShTimerCallBack<CAmTimerCb> pTimer2Callback;
    TAmShTimerCallBack<CAmTimerCb> pTimer3Callback;
    TAmShTimerCallBack<CAmTimerCb> pTimer4Callback;
    CAmSocketHandler *mSocketHandler;
};

class CAmSocketHandlerTest: public ::testing::Test
{
public:
    CAmSocketHandlerTest();
    ~CAmSocketHandlerTest();
    void SetUp();
    void TearDown();
};

} /* namespace am */
#endif /* SOCKETHANDLERTEST_H_ */
