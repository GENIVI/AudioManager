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

#ifndef ROUTINGINTERFACETEST_H_
#define ROUTINGINTERFACETEST_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include "CAmDatabaseHandlerMap.h"
#include "CAmControlReceiver.h"
#include "CAmRoutingReceiver.h"
#include "CAmControlSender.h"
#include "CAmRouter.h"
#include "../IAmRoutingBackdoor.h"
#include "../IAmCommandBackdoor.h"
#include "../IAmControlBackdoor.h"
#include "../CAmCommonFunctions.h"
#include "../MockIAmRoutingSend.h"
#include "../MockIAmControlSend.h"
#include "CAmSocketHandler.h"

namespace am
{

class CAmRoutingInterfaceTest: public ::testing::Test
{
public:
    CAmRoutingInterfaceTest();
    ~CAmRoutingInterfaceTest();
    std::vector<std::string> plistRoutingPluginDirs;
    std::vector<std::string> plistCommandPluginDirs;
    CAmSocketHandler pSocketHandler;
    CAmDatabaseHandlerMap pDatabaseHandler;
    CAmRoutingSender pRoutingSender;
    CAmRoutingReceiver pRoutingReceiver;
    CAmCommandSender pCommandSender;
    CAmControlSender pControlSender;
    CAmRouter pRouter;
    MockIAmRoutingSend pMockInterface;
    MockIAmControlSend pMockControlInterface;
    IAmRoutingBackdoor pRoutingInterfaceBackdoor;
    IAmCommandBackdoor pCommandInterfaceBackdoor;
    IAmControlBackdoor pControlInterfaceBackdoor;
    CAmControlReceiver pControlReceiver;
    CAmCommonFunctions pCF;
    void SetUp();
    void TearDown();
};

}

#endif /* ROUTINGINTERFACETEST_H_ */
