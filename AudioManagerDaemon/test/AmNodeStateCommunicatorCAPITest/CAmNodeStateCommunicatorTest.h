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
* \author Aleksandar Donchev, aleksander.donchev@partner.bmw.de BMW 2013
 *
 * \file CAmNodeStateCommunicatorTest.h
 * For further information see http://www.genivi.org/.
 *
 */


#ifndef CAMNODESTATECOMMUNICATORTEST_H_
#define CAMNODESTATECOMMUNICATORTEST_H_

#include "CAmNodeStateCommunicatorCAPI.h"
#include "CAmControlSender.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../IAmControlBackdoor.h"
#include "../MockIAmControlSend.h"
#include "CAmCommonAPIWrapper.h"

using namespace testing;
using namespace am;

class CAmEnvironment : public ::testing::Environment
{
public:
    IAmControlBackdoor pControlInterfaceBackdoor;
    CAmControlSender pControlSender;
    CAmSocketHandler iSocketHandler;
    CAmCommonAPIWrapper *wrapper;
    CAmNodeStateCommunicatorCAPI nsmController;
    pthread_t pNsmThread, pMainLoopThread;
    CAmEnvironment();
    ~CAmEnvironment();
    void SetUp();
    // Override this to define how to tear down the environment.
    void TearDown();
    void waitUntilAvailable(unsigned short seconds);
};

class CAmNodeStateCommunicatorTest:public ::testing::Test
{
public:
    MockIAmControlSend pMockControlInterface;
    CAmNodeStateCommunicatorTest();
    virtual ~CAmNodeStateCommunicatorTest();
    void SetUp();
    void TearDown();
};


#endif /* CAMNODESTATECOMMUNICATORTEST_H_ */
