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

#ifndef SERIALIZERTEST_H_
#define SERIALIZERTEST_H_

#define WITH_DLT

#include <ctime>
#include <chrono>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <queue>
#include "CAmSocketHandler.h"

namespace am
{

    class IAmSerializerCb
    {
    public:
        virtual ~IAmSerializerCb()
        {
        }
        virtual bool dispatchData(const uint32_t handle, std::string & outString)=0;
        virtual void check()=0;
        virtual int checkInt()=0;
    };

    class IAmTimerCb
    {
    public:
        virtual ~IAmTimerCb()
        {
        }
        virtual void timerCallback(sh_timerHandle_t handle, void * userData)=0;
    };

    class MockIAmTimerCb: public IAmTimerCb
    {
    public:
        MOCK_CONST_METHOD2(timerCallback,
                void(sh_timerHandle_t handle, void *userData));
    };

    class CAmTimerSockethandlerController: public MockIAmTimerCb
    {
        CAmSocketHandler *mpSocketHandler;
        timespec mUpdateTimeout;
    public:
        explicit CAmTimerSockethandlerController(CAmSocketHandler *SocketHandler, const timespec &timeout);
        virtual ~CAmTimerSockethandlerController();

        void timerCallback(sh_timerHandle_t handle, void * userData);

        TAmShTimerCallBack<CAmTimerSockethandlerController> pTimerCallback;
    };

    class MockIAmSerializerCb: public IAmSerializerCb
    {
    public:
        MOCK_METHOD2(dispatchData,
                bool(const uint32_t handle, std::string & outString));
        MOCK_METHOD0(check,
                void());
        MOCK_METHOD0(checkInt,
                int());
    };

    class CAmSerializerTest: public ::testing::Test
    {
    public:
        CAmSerializerTest();
        ~CAmSerializerTest();
        void SetUp();
        void TearDown();
    };

} /* namespace am */
#endif /* SOCKETHANDLERTEST_H_ */
