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
 * \author Aleksandar Donchev, aleksander.donchev@partner.bmw.de BMW 2013
 *
 * For further information see http://www.genivi.org/.
 *
 */

#ifndef MAPHANDLERTEST_H_
#define MAPHANDLERTEST_H_

#define UNIT_TEST 1

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "CAmSocketHandler.h"
#include "CAmDatabaseHandlerMap.h"
#include "CAmControlReceiver.h"
#include "CAmControlSender.h"
#include "CAmTestDatabaseObserver.h"
#include "CAmRoutingSender.h"
#include "CAmRouter.h"
#include "CAmControlSender.h"
#include "MockDatabaseObserver.h"
#include "../IAmControlBackdoor.h"
#include "../IAmCommandBackdoor.h"
#include "../CAmCommonFunctions.h"
#include "../MockIAmControlSend.h"
#include "../MockIAmCommandSend.h"


namespace am
{

class CAmMapBasicTest : public ::testing::Test
{
public:
    CAmMapBasicTest();
    ~CAmMapBasicTest();
    CAmSocketHandler pSocketHandler;
    CAmDatabaseHandlerMap pDatabaseHandler;
    std::vector<std::string> plistRoutingPluginDirs;
    std::vector<std::string> plistCommandPluginDirs;
    CAmRoutingSender pRoutingSender;
    CAmCommandSender pCommandSender;
    IAmRoutingBackdoor pRoutingInterfaceBackdoor;
    IAmCommandBackdoor pCommandInterfaceBackdoor;
    
    CAmControlSender pControlSender;
    CAmRouter pRouter;
    CAmControlReceiver pControlReceiver;
    CAmCommonFunctions pCF;
    void SetUp();
    void TearDown();
    void createMainConnectionSetup(am_mainConnectionID_t & mainConnectionID, am_MainConnection_s & mainConnection);
};

class CAmMapHandlerTest: public CAmMapBasicTest
{
public:
	CAmMapHandlerTest();
	~CAmMapHandlerTest();
    MockIAmCommandSend pMockInterface;
    CAmDatabaseObserver mMockObserver;
};

}

#endif /* MAPHANDLERTEST_H_ */
