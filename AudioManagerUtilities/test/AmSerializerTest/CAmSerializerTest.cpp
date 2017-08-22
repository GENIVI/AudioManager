/**
 * SPDX license identifier: MPL-2.0
 *
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
 * \author Christian Linke, christian.linke@bmw.de BMW 2011,2012
 *
 * For further information see http://www.genivi.org/.
 *
 */

#include <cstdio>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/un.h>
#include <sys/poll.h>

#include "CAmSocketHandler.h"
#include "CAmSerializer.h"
#include "CAmSerializerTest.h"

using namespace testing;
using namespace am;

CAmTimerSockethandlerController::CAmTimerSockethandlerController(CAmSocketHandler *myHandler, const timespec &timeout) :
        MockIAmTimerCb(), mpSocketHandler(myHandler), mUpdateTimeout(timeout), pTimerCallback(this, &CAmTimerSockethandlerController::timerCallback)
{
}

am::CAmTimerSockethandlerController::~CAmTimerSockethandlerController()
{
}

void am::CAmTimerSockethandlerController::timerCallback(sh_timerHandle_t handle, void* userData)
{
    MockIAmTimerCb::timerCallback(handle, userData);
    mpSocketHandler->stop_listening();
}

CAmSerializerTest::CAmSerializerTest()
{
}

CAmSerializerTest::~CAmSerializerTest()
{
}

void CAmSerializerTest::SetUp()
{

}

void CAmSerializerTest::TearDown()
{
}

struct SerializerData
{

    std::string testStr;
    int result;
    MockIAmSerializerCb *pSerCb;
    CAmSocketHandler *pSocketHandler;
    V2::CAmSerializer *pSerializer;
};

void* ptSerializerSync(void* data)
{
    SerializerData *pData = (SerializerData*) data;
    std::string testStr(pData->testStr);
    bool result = false;
    int r = 0;
    const uint32_t ten = 10;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    pData->pSerializer->syncCall(pData->pSerCb, &MockIAmSerializerCb::check);
    pData->pSerializer->syncCall(pData->pSerCb, &MockIAmSerializerCb::checkInt, pData->result);
    pData->pSerializer->syncCall(pData->pSerCb, &MockIAmSerializerCb::dispatchData, result, ten, pData->testStr);        
#pragma GCC diagnostic pop
    return (NULL);
}

void* ptSerializerASync(void* data)
{
    SerializerData *pData = (SerializerData*) data;
    std::string testStr;
    bool result = false;
    int r = 0;
    const uint32_t ten = 10;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
    for (uint32_t i = 0; i < 5; i++)
    {
        testStr = pData->testStr;
        pData->pSerializer->asyncCall(pData->pSerCb, &MockIAmSerializerCb::dispatchData, i, testStr);
    }
    pData->testStr = testStr;
    pData->pSerializer->asyncInvocation(std::bind([]()->bool
    {   return 1;}));
    pData->pSerializer->asyncInvocation(std::bind([](const int i, int & result)
    {   result = i*10;}, 1, std::ref(r)));

    pData->pSerializer->asyncCall(pData->pSerCb, &MockIAmSerializerCb::check);
    pData->pSerializer->asyncCall(pData->pSerCb, &MockIAmSerializerCb::check);

    pData->pSerializer->asyncCall(pData->pSerCb, &MockIAmSerializerCb::checkInt);
        
#pragma GCC diagnostic pop
    return (NULL);
}

ACTION(ActionDispatchData){
arg1="DispatchData";
}

TEST(CAmSerializerTest, syncTest)
{
    pthread_t serThread;

    MockIAmSerializerCb serCb;
    CAmSocketHandler myHandler;
    std::string testStr("testStr");
    V2::CAmSerializer serializer(&myHandler);
    sh_timerHandle_t handle;
    timespec timeout4;
    timeout4.tv_nsec = 0;
    timeout4.tv_sec = 3;
    CAmTimerSockethandlerController testCallback4(&myHandler, timeout4);
    myHandler.addTimer(timeout4, &testCallback4.pTimerCallback, handle, NULL);
    EXPECT_CALL(testCallback4,timerCallback(handle,NULL)).Times(1);

    SerializerData serializerData;
    serializerData.result = 0;
    serializerData.testStr = testStr;
    serializerData.pSerCb = &serCb;
    serializerData.pSocketHandler = &myHandler;
    serializerData.pSerializer = &serializer;
    pthread_create(&serThread, NULL, ptSerializerSync, &serializerData);

    EXPECT_CALL(serCb,check()).Times(1);
    EXPECT_CALL(serCb,checkInt()).Times(1).WillRepeatedly(Return(100));
    EXPECT_CALL(serCb,dispatchData(10,testStr)).Times(1).WillRepeatedly(DoAll(ActionDispatchData(), Return(true)));

    myHandler.start_listenting();

    pthread_join(serThread, NULL);
    ASSERT_TRUE(serializerData.testStr == "DispatchData");
    ASSERT_TRUE(serializerData.result == 100);
}

TEST(CAmSerializerTest, asyncTest)
{
    pthread_t serThread;

    MockIAmSerializerCb serCb;
    CAmSocketHandler myHandler;
    std::string testStr("testStr");
    V2::CAmSerializer serializer(&myHandler);
    sh_timerHandle_t handle;
    timespec timeout4;
    timeout4.tv_nsec = 0;
    timeout4.tv_sec = 3;
    CAmTimerSockethandlerController testCallback4(&myHandler, timeout4);
    myHandler.addTimer(timeout4, &testCallback4.pTimerCallback, handle, NULL);
    EXPECT_CALL(testCallback4,timerCallback(handle,NULL)).Times(1);

    SerializerData serializerData;
    serializerData.result = 0;
    serializerData.testStr = testStr;
    serializerData.pSerCb = &serCb;
    serializerData.pSocketHandler = &myHandler;
    serializerData.pSerializer = &serializer;
    pthread_create(&serThread, NULL, ptSerializerASync, &serializerData);

    EXPECT_CALL(serCb,check()).Times(2);
    EXPECT_CALL(serCb,checkInt()).Times(1).WillRepeatedly(Return(100));
    for (int i = 0; i < 5; i++)
        EXPECT_CALL(serCb,dispatchData(i,testStr)).WillOnce(DoAll(ActionDispatchData(), Return(true)));

    myHandler.start_listenting();

    pthread_join(serThread, NULL);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

