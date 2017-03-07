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
 * \author Aleksandar Donchev, aleksander.donchev@partner.bmw.de BMW 2013, 2014
 *
 * For further information see http://www.genivi.org/.
 *
 */

#ifndef MAPTEST_H_
#define MAPTEST_H_

#define UNIT_TEST 1

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include "CAmDatabaseHandlerMap.h"
#include "CAmControlReceiver.h"
#include "CAmControlSender.h"
#include "CAmRoutingSender.h"
#include "CAmRouter.h"
#include "CAmSocketHandler.h"
#include "../IAmControlBackdoor.h"
#include "../IAmCommandBackdoor.h"
#include "../CAmCommonFunctions.h"
#include "../MockIAmControlSend.h"
#include "../MockIAmCommandSend.h"

namespace am
{

    class CAmRouterMapTest: public ::testing::Test
    {
    public:
        CAmRouterMapTest();
        ~CAmRouterMapTest();
        std::vector<std::string> plistRoutingPluginDirs;
        std::vector<std::string> plistCommandPluginDirs;
        CAmSocketHandler pSocketHandler;
        CAmControlSender pControlSender;
        CAmDatabaseHandlerMap pDatabaseHandler;
        CAmRouter pRouter;
        CAmRoutingSender pRoutingSender;
        CAmCommandSender pCommandSender;
        MockIAmCommandSend pMockInterface;
        MockIAmControlSend pMockControlInterface;
        IAmRoutingBackdoor pRoutingInterfaceBackdoor;
        IAmCommandBackdoor pCommandInterfaceBackdoor;
        IAmControlBackdoor pControlInterfaceBackdoor;
        CAmControlReceiver pControlReceiver;
        CAmCommonFunctions pCF;
        void SetUp();
        void TearDown();

        void createMainConnectionSetup();

        void enterDomainDB(const std::string & domainName, am_domainID_t & domainID);
        void enterSourceDB(const std::string & sourceName, const am_domainID_t domainID, const std::vector<am_CustomConnectionFormat_t> & connectionFormats,
                am_sourceID_t & sourceID);
        void enterSinkDB(const std::string & sinkName, const am_domainID_t domainID, const std::vector<am_CustomConnectionFormat_t> & connectionFormats,
                am_sinkID_t & sinkID);
        void enterGatewayDB(const std::string & gwName, const am_domainID_t domainSourceID, const am_domainID_t domainSinkID,
                const std::vector<am_CustomConnectionFormat_t> & sourceConnectionFormats,
                const std::vector<am_CustomConnectionFormat_t> & sinkConnectionFormats, const std::vector<bool> & matrix, const am_sourceID_t & sourceID,
                const am_sinkID_t & sinkID, am_gatewayID_t & gatewayID);
        void enterConverterDB(const std::string & gwName, const am_domainID_t domainID,
                const std::vector<am_CustomConnectionFormat_t> & sourceConnectionFormats,
                const std::vector<am_CustomConnectionFormat_t> & sinkConnectionFormats, const std::vector<bool> & matrix, const am_sourceID_t & sourceID,
                const am_sinkID_t & sinkID, am_converterID_t & converterID);
        am_Error_e getRoute(const bool onlyfree, const bool shouldReload, const am_sourceID_t sourceID, const am_sinkID_t sinkID,
                std::vector<am_Route_s>& returnList, const unsigned countCycles = 0, const unsigned pathsCount = MAX_ROUTING_PATHS);
        am_Error_e getRoute(const bool onlyfree, const bool shouldReload, const am_Source_s & aSource, const am_Sink_s & aSink,
                std::vector<am_Route_s> & listRoutes, const unsigned countCycles = 0, const unsigned pathsCount = MAX_ROUTING_PATHS);
        am_Error_e getAllPaths(const bool onlyfree, const am_sourceID_t sourceID, const am_sinkID_t sinkID, std::vector<am_Route_s> & resultPath,
                const unsigned countCycles = 0, const unsigned pathsCount = MAX_ROUTING_PATHS);
    };

}

#endif /* MAPTEST_H_ */
