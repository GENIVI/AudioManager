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

#ifndef ROUTINGINTERFACETEST_H_
#define ROUTINGINTERFACETEST_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "CAmDatabaseHandler.h"
#include "CAmControlReceiver.h"
#include "CAmRoutingReceiver.h"
#include "CAmDatabaseObserver.h"
#include "CAmControlSender.h"
#include "CAmRoutingSender.h"
#include "CAmRouter.h"
#include "../IAmRoutingBackdoor.h"
#include "../IAmCommandBackdoor.h"
#include "../IAmControlBackdoor.h"
#include "../CAmCommonFunctions.h"
#include "../MockIAmRoutingSend.h"
#include "../MockIAmControlSend.h"
#include "shared/CAmSocketHandler.h"
#include "shared/CAmDbusWrapper.h"

namespace am
{

class CAmControlInterfaceTest: public ::testing::Test
{
public:
    CAmControlInterfaceTest();
    ~CAmControlInterfaceTest();
    CAmSocketHandler pSocketHandler;
    CAmDbusWrapper* pDBusWrapper;
    std::vector<std::string> plistCommandPluginDirs;
    std::vector<std::string> plistRoutingPluginDirs;
    CAmDatabaseHandler pDatabaseHandler;
    CAmRoutingSender pRoutingSender;
    CAmCommandSender pCommandSender;
    MockIAmControlSend pMockControlInterface;
    MockIAmRoutingSend pMockRoutingInterface;
    IAmRoutingBackdoor pRoutingInterfaceBackdoor;
    IAmCommandBackdoor pCommandInterfaceBackdoor;
    IAmControlBackdoor pControlInterfaceBackdoor;
    CAmControlSender pControlSender;
    CAmRouter pRouter;
    CAmDatabaseObserver pDatabaseObserver;
    CAmControlReceiver pControlReceiver;
    CAmRoutingReceiver pRoutingReceiver;
    CAmCommonFunctions pCF;
    void SetUp();
    void TearDown();
};

}

#endif /* ROUTINGINTERFACETEST_H_ */
