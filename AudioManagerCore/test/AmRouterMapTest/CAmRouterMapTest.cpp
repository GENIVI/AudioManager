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

#include <ctime>
#include <chrono>
#include "CAmRouterMapTest.h"
#include <string.h>
#include "CAmDltWrapper.h"
#include "CAmCommandLineSingleton.h"

TCLAP::SwitchArg enableDebug("V", "logDlt", "print DLT logs to stdout or dlt-daemon default off", false);

using namespace am;
using namespace testing;

CAmRouterMapTest::CAmRouterMapTest() :
        plistRoutingPluginDirs(), //
                plistCommandPluginDirs(), //
                pSocketHandler(), //
                pControlSender(), //
                pDatabaseHandler(),
                pRouter(&pDatabaseHandler, &pControlSender), //
                pRoutingSender(plistRoutingPluginDirs, dynamic_cast<IAmDatabaseHandler*>(&pDatabaseHandler)), //
                pCommandSender(plistCommandPluginDirs, &pSocketHandler), //
                pMockInterface(), //
                pMockControlInterface(), //
                pRoutingInterfaceBackdoor(), //
                pCommandInterfaceBackdoor(), //
                pControlInterfaceBackdoor(), //
                pControlReceiver(&pDatabaseHandler, &pRoutingSender, &pCommandSender, &pSocketHandler, &pRouter)
{
    pDatabaseHandler.registerObserver(&pRoutingSender);
    pDatabaseHandler.registerObserver(&pCommandSender);
    pDatabaseHandler.registerObserver(&pRouter);
    pCommandInterfaceBackdoor.injectInterface(&pCommandSender, &pMockInterface);
    pControlInterfaceBackdoor.replaceController(&pControlSender, &pMockControlInterface);
}

CAmRouterMapTest::~CAmRouterMapTest()
{

}

void CAmRouterMapTest::SetUp()
{
    logInfo("Routing Test started ");
    am_Domain_s domain;
    pCF.createDomain(domain);
    am_domainID_t forgetDomain;
    am_sinkClass_t forgetSinkClassID;
    am_SinkClass_s sinkClass;
    sinkClass.name = "TestSinkClass";
    sinkClass.sinkClassID = 1;
    am_sourceClass_t forgetSourceClassID;
    am_SourceClass_s sourceClass;
    sourceClass.name = "TestSourceClass";
    sourceClass.sourceClassID = 1;
    domain.domainID = 4;
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain, forgetDomain));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkClass, forgetSinkClassID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(forgetSourceClassID, sourceClass));
}

void CAmRouterMapTest::TearDown()
{
}

ACTION(returnConnectionFormat){
arg4=arg3;
}

void CAmRouterMapTest::enterDomainDB(const std::string & domainName, am_domainID_t & domainID)
{
    am_Domain_s domain1;
    domain1.domainID = 0;
    domain1.name = domainName;
    domain1.busname = "domain1bus";
    domain1.state = DS_CONTROLLED;
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain1, domainID));
}

void CAmRouterMapTest::enterSourceDB(const std::string & sourceName, const am_domainID_t domainID,
        const std::vector<am_CustomConnectionFormat_t> & connectionFormats, am_sourceID_t & sourceID)
{
    am_Source_s source;
    source.domainID = domainID;
    source.name = sourceName;
    source.sourceState = SS_ON;
    source.sourceID = 0;
    source.sourceClassID = 5;
    source.listConnectionFormats = connectionFormats;
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source, sourceID));
}

void CAmRouterMapTest::enterSinkDB(const std::string & sinkName, const am_domainID_t domainID,
        const std::vector<am_CustomConnectionFormat_t> & connectionFormats, am_sinkID_t & sinkID)
{
    am_Sink_s sink;
    sink.domainID = domainID;
    sink.name = sinkName;
    sink.sinkID = 0;
    sink.sinkClassID = 5;
    sink.muteState = MS_MUTED;
    sink.listConnectionFormats = connectionFormats;
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink, sinkID));
}

void CAmRouterMapTest::enterGatewayDB(const std::string & gwName, const am_domainID_t domainSourceID, const am_domainID_t domainSinkID,
        const std::vector<am_CustomConnectionFormat_t> & sourceConnectionFormats, const std::vector<am_CustomConnectionFormat_t> & sinkConnectionFormats,
        const std::vector<bool> & matrix, const am_sourceID_t & sourceID, const am_sinkID_t & sinkID, am_gatewayID_t & gatewayID)
{
    am_Gateway_s gateway;
    gateway.controlDomainID = domainSourceID;
    gateway.gatewayID = 0;
    gateway.sinkID = sinkID;
    gateway.sourceID = sourceID;
    gateway.domainSourceID = domainSourceID;
    gateway.domainSinkID = domainSinkID;
    gateway.listSinkFormats = sinkConnectionFormats;
    gateway.listSourceFormats = sourceConnectionFormats;
    gateway.convertionMatrix = matrix;
    gateway.name = gwName;
    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway, gatewayID));
}

void CAmRouterMapTest::enterConverterDB(const std::string & gwName, const am_domainID_t domainID,
        const std::vector<am_CustomConnectionFormat_t> & sourceConnectionFormats, const std::vector<am_CustomConnectionFormat_t> & sinkConnectionFormats,
        const std::vector<bool> & matrix, const am_sourceID_t & sourceID, const am_sinkID_t & sinkID, am_converterID_t & converterID)
{
    am_Converter_s converter;
    converter.converterID = 0;
    converter.sinkID = sinkID;
    converter.sourceID = sourceID;
    converter.domainID = domainID;
    converter.listSinkFormats = sinkConnectionFormats;
    converter.listSourceFormats = sourceConnectionFormats;
    converter.convertionMatrix = matrix;
    converter.name = gwName;
    ASSERT_EQ(E_OK, pDatabaseHandler.enterConverterDB(converter, converterID));
}

am_Error_e CAmRouterMapTest::getRoute(const bool onlyfree, const bool shouldReload, const am_Source_s & aSource, const am_Sink_s & aSink,
        std::vector<am_Route_s> & listRoutes, const unsigned countCycles, const unsigned pathsCount)
{
    return getRoute(onlyfree, shouldReload, aSource.sourceID, aSink.sinkID, listRoutes, countCycles, pathsCount);
}

am_Error_e CAmRouterMapTest::getRoute(const bool onlyfree, const bool shouldReload, const am_sourceID_t sourceID, const am_sinkID_t sinkID,
        std::vector<am_Route_s>& returnList, const unsigned countCycles, const unsigned pathsCount)
{
    pRouter.setMaxAllowedCycles(countCycles);
    pRouter.setMaxPathCount(pathsCount);
    std::ios_base::fmtflags oldflags = std::cout.flags();
    std::streamsize oldprecision = std::cout.precision();
    auto t_start = std::chrono::high_resolution_clock::now();
    if (shouldReload)
        pRouter.load();

    am_Error_e error = pRouter.getRoute(onlyfree, sourceID, sinkID, returnList);
    auto t_end = std::chrono::high_resolution_clock::now();
    std::cout << std::fixed << std::setprecision(2);
    std::cout << returnList.size() << " routes from " << sourceID << " to " << sinkID;
    std::cout << " in " << std::chrono::duration<double, std::milli>(t_end - t_start).count() << " ms\n";
    std::cout.flags(oldflags);
    std::cout.precision(oldprecision);
    return error;
}

am_Error_e CAmRouterMapTest::getAllPaths(const bool onlyfree, const am_sourceID_t sourceID, const am_sinkID_t sinkID, std::vector<am_Route_s> & resultPath,
        const unsigned countCycles, const unsigned pathsCount)
{
    std::ios_base::fmtflags oldflags = std::cout.flags();
    std::streamsize oldprecision = std::cout.precision();
    auto t_start = std::chrono::high_resolution_clock::now();
    if (pRouter.getUpdateGraphNodesAction())
        pRouter.load();
    CAmRoutingNode* sourceNode = pRouter.sourceNodeWithID(sourceID);
    CAmRoutingNode* sinkNode = pRouter.sinkNodeWithID(sinkID);

    if (!sourceNode || !sinkNode)
        return E_NON_EXISTENT;

    am_Error_e error = pRouter.getFirstNShortestPaths(onlyfree, countCycles, pathsCount, *sourceNode, *sinkNode, resultPath);
    auto t_end = std::chrono::high_resolution_clock::now();
    std::cout << std::fixed << std::setprecision(2);
    std::cout << resultPath.size() << " routes from " << sourceNode->getData().data.source->sourceID << " to " << sinkNode->getData().data.sink->sinkID;
    std::cout << " in " << std::chrono::duration<double, std::milli>(t_end - t_start).count() << " ms\n";
    std::cout.flags(oldflags);
    std::cout.precision(oldprecision);
    return error;
}

TEST_F(CAmRouterMapTest,checkInsertedDomain)
{
    std::vector<am_domainID_t> domains;
    ASSERT_TRUE(CAmRouter::shouldGoInDomain(domains, 22, 0));
    domains.push_back(22);
    ASSERT_TRUE(CAmRouter::shouldGoInDomain(domains, 22, 0));
    domains.push_back(22);
    ASSERT_TRUE(CAmRouter::shouldGoInDomain(domains, 22, 0));
    ASSERT_TRUE(CAmRouter::shouldGoInDomain(domains, 50, 0));
    domains.push_back(30);
    ASSERT_TRUE(CAmRouter::shouldGoInDomain(domains, 30, 0));
    ASSERT_FALSE(CAmRouter::shouldGoInDomain(domains, 22, 0));
    domains.push_back(30);
    ASSERT_TRUE(CAmRouter::shouldGoInDomain(domains, 30, 0));
    ASSERT_FALSE(CAmRouter::shouldGoInDomain(domains, 22, 0));
    ASSERT_TRUE(CAmRouter::shouldGoInDomain(domains, 60, 0));
}

//test that checks just sinks and source in a domain but connectionformats do not match
TEST_F(CAmRouterMapTest,simpleRoute2withDomainNoMatchFormats)
{
    EXPECT_CALL(pMockControlInterface,getConnectionFormatChoice(_,_,_,_,_)).WillRepeatedly(DoAll(returnConnectionFormat(), Return(E_OK)));

    //initialize 2 domains
    am_Domain_s domain1;
    am_domainID_t domainID1;

    domain1.domainID = 0;
    domain1.name = "domain1";
    domain1.busname = "domain1bus";
    domain1.state = DS_CONTROLLED;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain1, domainID1));

    am_Source_s source;
    am_sourceID_t sourceID;
    am_SourceClass_s sourceclass;

    sourceclass.name = "sClass";
    sourceclass.sourceClassID = 5;

    source.domainID = domainID1;
    source.name = "source1";
    source.sourceState = SS_ON;
    source.sourceID = 0;
    source.sourceClassID = 5;
    source.listConnectionFormats.push_back(CF_GENIVI_MONO);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(source.sourceClassID, sourceclass));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source, sourceID));

    am_Sink_s sink;
    am_sinkID_t sinkID;
    am_SinkClass_s sinkclass;
    sinkclass.sinkClassID = 5;
    sinkclass.name = "sname";

    sink.domainID = domainID1;
    sink.name = "sink1";
    sink.sinkID = 0;
    sink.sinkClassID = 5;
    sink.muteState = MS_MUTED;
    sink.listConnectionFormats.push_back(CF_GENIVI_ANALOG);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkclass, sinkclass.sinkClassID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink, sinkID));

    std::vector<am_Route_s> listRoutes;
    std::vector<am_RoutingElement_s> listRoutingElements;
    am_RoutingElement_s hopp1;

    hopp1.sinkID = sinkID;
    hopp1.sourceID = sourceID;
    hopp1.domainID = domainID1;
    hopp1.connectionFormat = source.listConnectionFormats[0];

    listRoutingElements.push_back(hopp1);

    am_Route_s compareRoute;
    compareRoute.route = listRoutingElements;
    compareRoute.sinkID = sinkID;
    compareRoute.sourceID = sourceID;

    am::am_Source_s sourceDb;
    am::am_Sink_s sinkDb;
    pDatabaseHandler.getSinkInfoDB(sinkID, sinkDb);
    pDatabaseHandler.getSourceInfoDB(sourceID, sourceDb);
    listRoutes.clear();
    ASSERT_EQ(getRoute(true, false, sourceDb, sinkDb, listRoutes), E_NOT_POSSIBLE);
    ASSERT_EQ(static_cast<uint>(0), listRoutes.size());
}

//test that checks just sinks and source in a domain
TEST_F(CAmRouterMapTest,simpleRoute2withDomain)
{
    EXPECT_CALL(pMockControlInterface,getConnectionFormatChoice(_,_,_,_,_)).WillRepeatedly(DoAll(returnConnectionFormat(), Return(E_OK)));

    //initialize 2 domains
    am_Domain_s domain1;
    am_domainID_t domainID1;

    domain1.domainID = 0;
    domain1.name = "domain1";
    domain1.busname = "domain1bus";
    domain1.state = DS_CONTROLLED;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain1, domainID1));

    am_Source_s source;
    am_sourceID_t sourceID;
    am_SourceClass_s sourceclass;

    sourceclass.name = "sClass";
    sourceclass.sourceClassID = 5;

    source.domainID = domainID1;
    source.name = "source1";
    source.sourceState = SS_ON;
    source.sourceID = 0;
    source.sourceClassID = 5;
    source.listConnectionFormats.push_back(CF_GENIVI_ANALOG);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(source.sourceClassID, sourceclass));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source, sourceID));

    am_Sink_s sink;
    am_sinkID_t sinkID;
    am_SinkClass_s sinkclass;
    sinkclass.sinkClassID = 5;
    sinkclass.name = "sname";

    sink.domainID = domainID1;
    sink.name = "sink1";
    sink.sinkID = 0;
    sink.sinkClassID = 5;
    sink.muteState = MS_MUTED;
    sink.listConnectionFormats.push_back(CF_GENIVI_ANALOG);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkclass, sinkclass.sinkClassID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink, sinkID));

    std::vector<am_Route_s> listRoutes;
    std::vector<am_RoutingElement_s> listRoutingElements;
    am_RoutingElement_s hopp1;

    hopp1.sinkID = sinkID;
    hopp1.sourceID = sourceID;
    hopp1.domainID = domainID1;
    hopp1.connectionFormat = source.listConnectionFormats[0];

    listRoutingElements.push_back(hopp1);

    am_Route_s compareRoute;
    compareRoute.route = listRoutingElements;
    compareRoute.sinkID = sinkID;
    compareRoute.sourceID = sourceID;

    am::am_Source_s sourceDb;
    am::am_Sink_s sinkDb;
    pDatabaseHandler.getSinkInfoDB(sinkID, sinkDb);
    pDatabaseHandler.getSourceInfoDB(sourceID, sourceDb);
    listRoutes.clear();
    ASSERT_EQ(getRoute(true, false, sourceDb, sinkDb, listRoutes), E_OK);
    ASSERT_EQ(static_cast<uint>(1), listRoutes.size());
    ASSERT_TRUE(pCF.compareRoute(compareRoute, listRoutes[0]));
}

//test that checks just 2 domains, one sink one source with only one connection format each
TEST_F(CAmRouterMapTest,simpleRoute2DomainsOnlyFree)
{
    EXPECT_CALL(pMockControlInterface,getConnectionFormatChoice(_,_,_,_,_)).WillRepeatedly(DoAll(returnConnectionFormat(), Return(E_OK)));

    //initialize 2 domains
    am_Domain_s domain1, domain2;
    am_domainID_t domainID1, domainID2;

    domain1.domainID = 0;
    domain1.name = "domain1";
    domain1.busname = "domain1bus";
    domain1.state = DS_CONTROLLED;
    domain2.domainID = 0;
    domain2.name = "domain2";
    domain2.busname = "domain2bus";
    domain2.state = DS_CONTROLLED;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain1, domainID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain2, domainID2));

    am_Source_s source, gwSource;
    am_sourceID_t sourceID, gwSourceID;
    am_SourceClass_s sourceclass;

    sourceclass.name = "sClass";
    sourceclass.sourceClassID = 5;

    source.domainID = domainID1;
    source.name = "source1";
    source.sourceState = SS_ON;
    source.sourceID = 0;
    source.sourceClassID = 5;
    source.listConnectionFormats.push_back(CF_GENIVI_ANALOG);

    gwSource.domainID = domainID2;
    gwSource.name = "gwsource1";
    gwSource.sourceState = SS_ON;
    gwSource.sourceID = 0;
    gwSource.sourceClassID = 5;
    gwSource.listConnectionFormats.push_back(CF_GENIVI_MONO);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(source.sourceClassID, sourceclass));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source, sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource, gwSourceID));

    am_Sink_s sink, gwSink;
    am_sinkID_t sinkID, gwSinkID;
    am_SinkClass_s sinkclass;
    sinkclass.sinkClassID = 5;
    sinkclass.name = "sname";

    sink.domainID = domainID2;
    sink.name = "sink1";
    sink.sinkID = 0;
    sink.sinkClassID = 5;
    sink.muteState = MS_MUTED;
    sink.listConnectionFormats.push_back(CF_GENIVI_MONO);

    gwSink.domainID = domainID1;
    gwSink.name = "gwSink";
    gwSink.sinkID = 0;
    gwSink.sinkClassID = 5;
    gwSink.muteState = MS_MUTED;
    gwSink.listConnectionFormats.push_back(CF_GENIVI_ANALOG);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkclass, sinkclass.sinkClassID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink, sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink, gwSinkID));

    am_Gateway_s gateway;
    am_gatewayID_t gatewayID;

    gateway.controlDomainID = domainID1;
    gateway.gatewayID = 0;
    gateway.sinkID = gwSinkID;
    gateway.sourceID = gwSourceID;
    gateway.domainSourceID = domainID2;
    gateway.domainSinkID = domainID1;
    gateway.listSinkFormats = gwSink.listConnectionFormats;
    gateway.listSourceFormats = gwSource.listConnectionFormats;
    gateway.convertionMatrix.push_back(true);
    gateway.name = "gateway";

    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway, gatewayID));

    std::vector<am_Route_s> listRoutes;
    std::vector<am_RoutingElement_s> listRoutingElements;
    am_RoutingElement_s hopp1;
    am_RoutingElement_s hopp2;

    hopp1.sinkID = gwSinkID;
    hopp1.sourceID = sourceID;
    hopp1.domainID = domainID1;
    hopp1.connectionFormat = source.listConnectionFormats[0];

    hopp2.sinkID = sinkID;
    hopp2.sourceID = gwSourceID;
    hopp2.domainID = domainID2;
    hopp2.connectionFormat = sink.listConnectionFormats[0];

    listRoutingElements.push_back(hopp1);
    listRoutingElements.push_back(hopp2);

    am_Route_s compareRoute;
    compareRoute.route = listRoutingElements;
    compareRoute.sinkID = sinkID;
    compareRoute.sourceID = sourceID;

    am::am_Source_s sourceDb;
    am::am_Sink_s sinkDb;
    pDatabaseHandler.getSinkInfoDB(sinkID, sinkDb);
    pDatabaseHandler.getSourceInfoDB(sourceID, sourceDb);

    ASSERT_EQ(getRoute(true, false, sourceID, sinkID, listRoutes), E_OK);
    ASSERT_EQ(static_cast<uint>(1), listRoutes.size());
    ASSERT_TRUE(pCF.compareRoute(compareRoute, listRoutes[0]));
}

//test that checks just 2 domains, one sink one source with only one connection format each
TEST_F(CAmRouterMapTest,simpleRoute2DomainsOnlyFreeNotFree)
{
    EXPECT_CALL(pMockControlInterface,getConnectionFormatChoice(_,_,_,_,_)).WillRepeatedly(DoAll(returnConnectionFormat(), Return(E_OK)));

    //initialize 2 domains
    am_Domain_s domain1, domain2;
    am_domainID_t domainID1, domainID2;

    domain1.domainID = 0;
    domain1.name = "domain1";
    domain1.busname = "domain1bus";
    domain1.state = DS_CONTROLLED;
    domain2.domainID = 0;
    domain2.name = "domain2";
    domain2.busname = "domain2bus";
    domain2.state = DS_CONTROLLED;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain1, domainID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain2, domainID2));

    am_Source_s source, gwSource;
    am_sourceID_t sourceID, gwSourceID;
    am_SourceClass_s sourceclass;

    sourceclass.name = "sClass";
    sourceclass.sourceClassID = 5;

    source.domainID = domainID1;
    source.name = "source1";
    source.sourceState = SS_ON;
    source.sourceID = 0;
    source.sourceClassID = 5;
    source.listConnectionFormats.push_back(CF_GENIVI_ANALOG);

    gwSource.domainID = domainID2;
    gwSource.name = "gwsource1";
    gwSource.sourceState = SS_ON;
    gwSource.sourceID = 0;
    gwSource.sourceClassID = 5;
    gwSource.listConnectionFormats.push_back(CF_GENIVI_MONO);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(source.sourceClassID, sourceclass));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source, sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource, gwSourceID));

    am_Sink_s sink, gwSink;
    am_sinkID_t sinkID, gwSinkID;
    am_SinkClass_s sinkclass;
    sinkclass.sinkClassID = 5;
    sinkclass.name = "sname";

    sink.domainID = domainID2;
    sink.name = "sink1";
    sink.sinkID = 0;
    sink.sinkClassID = 5;
    sink.muteState = MS_MUTED;
    sink.listConnectionFormats.push_back(CF_GENIVI_MONO);

    gwSink.domainID = domainID1;
    gwSink.name = "gwSink";
    gwSink.sinkID = 0;
    gwSink.sinkClassID = 5;
    gwSink.muteState = MS_MUTED;
    gwSink.listConnectionFormats.push_back(CF_GENIVI_ANALOG);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkclass, sinkclass.sinkClassID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink, sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink, gwSinkID));

    am_Gateway_s gateway;
    am_gatewayID_t gatewayID;

    gateway.controlDomainID = domainID1;
    gateway.gatewayID = 0;
    gateway.sinkID = gwSinkID;
    gateway.sourceID = gwSourceID;
    gateway.domainSourceID = domainID2;
    gateway.domainSinkID = domainID1;
    gateway.listSinkFormats = gwSink.listConnectionFormats;
    gateway.listSourceFormats = gwSource.listConnectionFormats;
    gateway.convertionMatrix.push_back(true);
    gateway.name = "gateway";

    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway, gatewayID));

    std::vector<am_Route_s> listRoutes;
    std::vector<am_RoutingElement_s> listRoutingElements;
    am_RoutingElement_s hopp1;
    am_RoutingElement_s hopp2;

    hopp1.sinkID = gwSinkID;
    hopp1.sourceID = sourceID;
    hopp1.domainID = domainID1;
    hopp1.connectionFormat = source.listConnectionFormats[0];

    hopp2.sinkID = sinkID;
    hopp2.sourceID = gwSourceID;
    hopp2.domainID = domainID2;
    hopp2.connectionFormat = sink.listConnectionFormats[0];

    listRoutingElements.push_back(hopp1);
    listRoutingElements.push_back(hopp2);

    am_Route_s compareRoute;
    compareRoute.route = listRoutingElements;
    compareRoute.sinkID = sinkID;
    compareRoute.sourceID = sourceID;

    am_Connection_s connection, connection1;
    am_connectionID_t id1, id2;
    connection.sourceID = sourceID;
    connection.sinkID = gwSinkID;
    connection.connectionFormat = CF_GENIVI_ANALOG;
    connection.connectionID = 0;
    connection1.sourceID = gwSourceID;
    connection1.sinkID = sinkID;
    connection1.connectionFormat = CF_GENIVI_ANALOG;
    connection1.connectionID = 0;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterConnectionDB(connection, id1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterConnectionDB(connection1, id2));

    am::am_Source_s sourceDb;
    am::am_Sink_s sinkDb;
    pDatabaseHandler.getSinkInfoDB(sinkID, sinkDb);
    pDatabaseHandler.getSourceInfoDB(sourceID, sourceDb);
    listRoutes.clear();
    ASSERT_EQ(getRoute(true, false, sourceDb, sinkDb, listRoutes), E_NOT_POSSIBLE);
    ASSERT_EQ(static_cast<uint>(0), listRoutes.size());

    listRoutes.clear();
    ASSERT_EQ(getRoute(false, false, sourceDb, sinkDb, listRoutes), E_OK);
    ASSERT_EQ(static_cast<uint>(1), listRoutes.size());
    ASSERT_TRUE(pCF.compareRoute(compareRoute, listRoutes[0]));
}

//test that checks just 2 domains, with gateway for each direction (possible circular route)
TEST_F(CAmRouterMapTest,simpleRoute2DomainsCircularGWOnlyFree)
{
    EXPECT_CALL(pMockControlInterface,getConnectionFormatChoice(_,_,_,_,_)).WillRepeatedly(DoAll(returnConnectionFormat(), Return(E_OK)));

    //initialize 2 domains
    am_Domain_s domain1, domain2;
    am_domainID_t domainID1, domainID2;

    domain1.domainID = 0;
    domain1.name = "domain1";
    domain1.busname = "domain1bus";
    domain1.state = DS_CONTROLLED;
    domain2.domainID = 0;
    domain2.name = "domain2";
    domain2.busname = "domain2bus";
    domain2.state = DS_CONTROLLED;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain1, domainID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain2, domainID2));

    am_Source_s source, gwSource, gwSource2;
    am_sourceID_t sourceID, gwSourceID, gwSourceID2;
    am_SourceClass_s sourceclass;

    sourceclass.name = "sClass";
    sourceclass.sourceClassID = 5;

    source.domainID = domainID1;
    source.name = "source1";
    source.sourceState = SS_ON;
    source.sourceID = 0;
    source.sourceClassID = 5;
    source.listConnectionFormats.push_back(CF_GENIVI_ANALOG);

    gwSource.domainID = domainID2;
    gwSource.name = "gwsource1";
    gwSource.sourceState = SS_ON;
    gwSource.sourceID = 0;
    gwSource.sourceClassID = 5;
    gwSource.listConnectionFormats.push_back(CF_GENIVI_MONO);

    gwSource2.domainID = domainID1;
    gwSource2.name = "gwsource2";
    gwSource2.sourceState = SS_ON;
    gwSource2.sourceID = 0;
    gwSource2.sourceClassID = 5;
    gwSource2.listConnectionFormats.push_back(CF_GENIVI_MONO);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(source.sourceClassID, sourceclass));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source, sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource, gwSourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource2, gwSourceID2));

    am_Sink_s sink, gwSink, gwSink2;
    am_sinkID_t sinkID, gwSinkID, gwSinkID2;
    am_SinkClass_s sinkclass;
    sinkclass.sinkClassID = 5;
    sinkclass.name = "sname";

    sink.domainID = domainID2;
    sink.name = "sink1";
    sink.sinkID = 0;
    sink.sinkClassID = 5;
    sink.muteState = MS_MUTED;
    sink.listConnectionFormats.push_back(CF_GENIVI_MONO);

    gwSink.domainID = domainID1;
    gwSink.name = "gwSink";
    gwSink.sinkID = 0;
    gwSink.sinkClassID = 5;
    gwSink.muteState = MS_MUTED;
    gwSink.listConnectionFormats.push_back(CF_GENIVI_ANALOG);

    gwSink2.domainID = domainID2;
    gwSink2.name = "gwSink2";
    gwSink2.sinkID = 0;
    gwSink2.sinkClassID = 5;
    gwSink2.muteState = MS_MUTED;
    gwSink2.listConnectionFormats.push_back(CF_GENIVI_MONO);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkclass, sinkclass.sinkClassID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink, sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink, gwSinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink2, gwSinkID2));

    am_Gateway_s gateway, gateway2;
    am_gatewayID_t gatewayID, gatewayID2;

    gateway.controlDomainID = domainID1;
    gateway.gatewayID = 0;
    gateway.sinkID = gwSinkID;
    gateway.sourceID = gwSourceID;
    gateway.domainSourceID = domainID2;
    gateway.domainSinkID = domainID1;
    gateway.listSinkFormats = gwSink.listConnectionFormats;
    gateway.listSourceFormats = gwSource.listConnectionFormats;
    gateway.convertionMatrix.push_back(true);
    gateway.name = "gateway";

    gateway2.controlDomainID = domainID1;
    gateway2.gatewayID = 0;
    gateway2.sinkID = gwSinkID2;
    gateway2.sourceID = gwSourceID2;
    gateway2.domainSourceID = domainID1;
    gateway2.domainSinkID = domainID2;
    gateway2.listSinkFormats = gwSink2.listConnectionFormats;
    gateway2.listSourceFormats = gwSource2.listConnectionFormats;
    gateway2.convertionMatrix.push_back(true);
    gateway2.name = "gateway2";

    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway, gatewayID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway2, gatewayID2));

    std::vector<am_Route_s> listRoutes;
    std::vector<am_RoutingElement_s> listRoutingElements;
    am_RoutingElement_s hopp1;
    am_RoutingElement_s hopp2;

    hopp1.sinkID = gwSinkID;
    hopp1.sourceID = sourceID;
    hopp1.domainID = domainID1;
    hopp1.connectionFormat = source.listConnectionFormats[0];

    hopp2.sinkID = sinkID;
    hopp2.sourceID = gwSourceID;
    hopp2.domainID = domainID2;
    hopp2.connectionFormat = sink.listConnectionFormats[0];

    listRoutingElements.push_back(hopp1);
    listRoutingElements.push_back(hopp2);

    am_Route_s compareRoute;
    compareRoute.route = listRoutingElements;
    compareRoute.sinkID = sinkID;
    compareRoute.sourceID = sourceID;

    am::am_Source_s sourceDb;
    am::am_Sink_s sinkDb;
    pDatabaseHandler.getSinkInfoDB(sinkID, sinkDb);
    pDatabaseHandler.getSourceInfoDB(sourceID, sourceDb);
    listRoutes.clear();
    ASSERT_EQ(getRoute(true, false, sourceDb, sinkDb, listRoutes), E_OK);
    ASSERT_EQ(static_cast<uint>(1), listRoutes.size());
    ASSERT_TRUE(pCF.compareRoute(compareRoute, listRoutes[0]));
}

//test that checks 3 domains, one sink one source, longer lists of connectionformats.
TEST_F(CAmRouterMapTest,simpleRoute3DomainsListConnectionFormats_2)
{
    EXPECT_CALL(pMockControlInterface,getConnectionFormatChoice(_,_,_,_,_)).WillRepeatedly(DoAll(returnConnectionFormat(), Return(E_OK)));

    //initialize 2 domains
    am_Domain_s domain1, domain2, domain3;
    am_domainID_t domainID1, domainID2, domainID3;

    domain1.domainID = 0;
    domain1.name = "domain1";
    domain1.busname = "domain1bus";
    domain1.state = DS_CONTROLLED;
    domain2.domainID = 0;
    domain2.name = "domain2";
    domain2.busname = "domain2bus";
    domain2.state = DS_CONTROLLED;
    domain3.domainID = 0;
    domain3.name = "domain3";
    domain3.busname = "domain3bus";
    domain3.state = DS_CONTROLLED;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain1, domainID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain2, domainID2));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain3, domainID3));

    am_Source_s source, gwSource, gwSource1;
    am_sourceID_t sourceID, gwSourceID, gwSourceID1;
    am_SourceClass_s sourceclass;

    sourceclass.name = "sClass";
    sourceclass.sourceClassID = 5;

    source.domainID = domainID1;
    source.name = "source1";
    source.sourceState = SS_ON;
    source.sourceID = 0;
    source.sourceClassID = 5;
    source.listConnectionFormats.push_back(CF_GENIVI_STEREO);
    source.listConnectionFormats.push_back(CF_GENIVI_MONO);

    gwSource.domainID = domainID2;
    gwSource.name = "gwsource1";
    gwSource.sourceState = SS_ON;
    gwSource.sourceID = 0;
    gwSource.sourceClassID = 5;
    gwSource.listConnectionFormats.push_back(CF_GENIVI_MONO);
    gwSource.listConnectionFormats.push_back(CF_GENIVI_STEREO);
    gwSource.listConnectionFormats.push_back(CF_GENIVI_ANALOG);

    gwSource1.domainID = domainID3;
    gwSource1.name = "gwsource2";
    gwSource1.sourceState = SS_ON;
    gwSource1.sourceID = 0;
    gwSource1.sourceClassID = 5;
    gwSource1.listConnectionFormats.push_back(CF_GENIVI_STEREO);
    gwSource1.listConnectionFormats.push_back(CF_GENIVI_MONO);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(source.sourceClassID, sourceclass));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source, sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource, gwSourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource1, gwSourceID1));

    am_Sink_s sink, gwSink, gwSink1;
    am_sinkID_t sinkID, gwSinkID, gwSinkID1;

    sink.domainID = domainID3;
    sink.name = "sink1";
    sink.sinkID = 0;
    sink.sinkClassID = 5;
    sink.muteState = MS_MUTED;
    sink.listConnectionFormats.push_back(CF_GENIVI_MONO);
    am_SinkClass_s sinkclass;
    sinkclass.sinkClassID = 5;
    sinkclass.name = "sname";

    gwSink.domainID = domainID1;
    gwSink.name = "gwSink";
    gwSink.sinkID = 0;
    gwSink.sinkClassID = 5;
    gwSink.muteState = MS_MUTED;
    gwSink.listConnectionFormats.push_back(CF_GENIVI_STEREO);
    gwSink.listConnectionFormats.push_back(CF_GENIVI_MONO);

    gwSink1.domainID = domainID2;
    gwSink1.name = "gwSink1";
    gwSink1.sinkID = 0;
    gwSink1.sinkClassID = 5;
    gwSink1.muteState = MS_MUTED;
    gwSink1.listConnectionFormats.push_back(CF_GENIVI_ANALOG);
    gwSink1.listConnectionFormats.push_back(CF_GENIVI_STEREO);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkclass, sinkclass.sinkClassID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink, sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink, gwSinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink1, gwSinkID1));

    am_Gateway_s gateway, gateway1;
    am_gatewayID_t gatewayID, gatewayID1;

    gateway.controlDomainID = domainID1;
    gateway.gatewayID = 0;
    gateway.sinkID = gwSinkID;
    gateway.sourceID = gwSourceID;
    gateway.domainSourceID = domainID2;
    gateway.domainSinkID = domainID1;
    gateway.listSinkFormats = gwSink.listConnectionFormats;
    gateway.listSourceFormats = gwSource.listConnectionFormats;
    gateway.convertionMatrix.push_back(false);
    gateway.convertionMatrix.push_back(false);
    gateway.convertionMatrix.push_back(false);
    gateway.convertionMatrix.push_back(true);
    gateway.convertionMatrix.push_back(true);
    gateway.convertionMatrix.push_back(false);
    gateway.name = "gateway";

    gateway1.controlDomainID = domainID2;
    gateway1.gatewayID = 0;
    gateway1.sinkID = gwSinkID1;
    gateway1.sourceID = gwSourceID1;
    gateway1.domainSourceID = domainID3;
    gateway1.domainSinkID = domainID2;
    gateway1.listSinkFormats = gwSink1.listConnectionFormats;
    gateway1.listSourceFormats = gwSource1.listConnectionFormats;
    gateway1.convertionMatrix.push_back(false);
    gateway1.convertionMatrix.push_back(false);
    gateway1.convertionMatrix.push_back(false);
    gateway1.convertionMatrix.push_back(true);
    gateway1.name = "gateway1";

    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway, gatewayID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway1, gatewayID1));

    std::vector<am_Route_s> listRoutes;
    std::vector<am_RoutingElement_s> listRoutingElements;
    am_RoutingElement_s hopp1;
    am_RoutingElement_s hopp2;
    am_RoutingElement_s hopp3;

    hopp1.sourceID = sourceID;
    hopp1.sinkID = gwSinkID;
    hopp1.domainID = domainID1;
    hopp1.connectionFormat = source.listConnectionFormats[1];

    hopp2.sourceID = gwSourceID;
    hopp2.sinkID = gwSinkID1;
    hopp2.domainID = domainID2;
    hopp2.connectionFormat = gwSink1.listConnectionFormats[1];

    hopp3.sourceID = gwSourceID1;
    hopp3.sinkID = sinkID;
    hopp3.domainID = domainID3;
    hopp3.connectionFormat = sink.listConnectionFormats[0];

    listRoutingElements.push_back(hopp1);
    listRoutingElements.push_back(hopp2);
    listRoutingElements.push_back(hopp3);

    am_Route_s compareRoute;
    compareRoute.route = listRoutingElements;
    compareRoute.sinkID = sinkID;
    compareRoute.sourceID = sourceID;

    am::am_Source_s sourceDb;
    am::am_Sink_s sinkDb;
    pDatabaseHandler.getSinkInfoDB(sinkID, sinkDb);
    pDatabaseHandler.getSourceInfoDB(sourceID, sourceDb);
    ASSERT_EQ(getRoute(false, false, sourceDb, sinkDb, listRoutes, 0), E_OK);
    ASSERT_EQ(static_cast<uint>(1), listRoutes.size());
    ASSERT_TRUE(pCF.compareRoute(compareRoute, listRoutes[0]));
}

//test that checks 3 domains, one sink one source, longer lists of connectionformats.
TEST_F(CAmRouterMapTest,simpleRoute3DomainsListConnectionFormats_1)
{
    EXPECT_CALL(pMockControlInterface,getConnectionFormatChoice(_,_,_,_,_)).WillRepeatedly(DoAll(returnConnectionFormat(), Return(E_OK)));

    //initialize 2 domains
    am_Domain_s domain1, domain2, domain3;
    am_domainID_t domainID1, domainID2, domainID3;

    domain1.domainID = 0;
    domain1.name = "domain1";
    domain1.busname = "domain1bus";
    domain1.state = DS_CONTROLLED;
    domain2.domainID = 0;
    domain2.name = "domain2";
    domain2.busname = "domain2bus";
    domain2.state = DS_CONTROLLED;
    domain3.domainID = 0;
    domain3.name = "domain3";
    domain3.busname = "domain3bus";
    domain3.state = DS_CONTROLLED;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain1, domainID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain2, domainID2));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain3, domainID3));

    am_Source_s source, gwSource, gwSource1;
    am_sourceID_t sourceID, gwSourceID, gwSourceID1;

    source.domainID = domainID1;
    source.name = "source1";
    source.sourceState = SS_ON;
    source.sourceID = 0;
    source.sourceClassID = 5;
    source.listConnectionFormats.push_back(CF_GENIVI_STEREO);
    source.listConnectionFormats.push_back(CF_GENIVI_MONO);
    am_SourceClass_s sourceclass;

    sourceclass.name = "sClass";
    sourceclass.sourceClassID = 5;

    gwSource.domainID = domainID2;
    gwSource.name = "gwsource1";
    gwSource.sourceState = SS_ON;
    gwSource.sourceID = 0;
    gwSource.sourceClassID = 5;
    gwSource.listConnectionFormats.push_back(CF_GENIVI_STEREO);
    gwSource.listConnectionFormats.push_back(CF_GENIVI_MONO);
    gwSource.listConnectionFormats.push_back(CF_GENIVI_ANALOG);

    gwSource1.domainID = domainID3;
    gwSource1.name = "gwsource2";
    gwSource1.sourceState = SS_ON;
    gwSource1.sourceID = 0;
    gwSource1.sourceClassID = 5;
    gwSource1.listConnectionFormats.push_back(CF_GENIVI_MONO);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(source.sourceClassID, sourceclass));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source, sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource, gwSourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource1, gwSourceID1));

    am_Sink_s sink, gwSink, gwSink1;
    am_sinkID_t sinkID, gwSinkID, gwSinkID1;
    am_SinkClass_s sinkclass;
    sinkclass.sinkClassID = 5;
    sinkclass.name = "sname";

    sink.domainID = domainID3;
    sink.name = "sink1";
    sink.sinkID = 0;
    sink.sinkClassID = 5;
    sink.muteState = MS_MUTED;
    sink.listConnectionFormats.push_back(CF_GENIVI_MONO);

    gwSink.domainID = domainID1;
    gwSink.name = "gwSink";
    gwSink.sinkID = 0;
    gwSink.sinkClassID = 5;
    gwSink.muteState = MS_MUTED;
    gwSink.listConnectionFormats.push_back(CF_GENIVI_STEREO);
    gwSink.listConnectionFormats.push_back(CF_GENIVI_MONO);

    gwSink1.domainID = domainID2;
    gwSink1.name = "gwSink1";
    gwSink1.sinkID = 0;
    gwSink1.sinkClassID = 5;
    gwSink1.muteState = MS_MUTED;
    gwSink1.listConnectionFormats.push_back(CF_GENIVI_ANALOG);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkclass, sinkclass.sinkClassID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink, sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink, gwSinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink1, gwSinkID1));

    am_Gateway_s gateway, gateway1;
    am_gatewayID_t gatewayID, gatewayID1;

    gateway.controlDomainID = domainID1;
    gateway.gatewayID = 0;
    gateway.sinkID = gwSinkID;
    gateway.sourceID = gwSourceID;
    gateway.domainSourceID = domainID2;
    gateway.domainSinkID = domainID1;
    gateway.listSinkFormats = gwSink.listConnectionFormats;
    gateway.listSourceFormats = gwSource.listConnectionFormats;
    gateway.convertionMatrix.push_back(false);
    gateway.convertionMatrix.push_back(false);
    gateway.convertionMatrix.push_back(false);
    gateway.convertionMatrix.push_back(false);
    gateway.convertionMatrix.push_back(true);
    gateway.convertionMatrix.push_back(false);
    gateway.name = "gateway";

    gateway1.controlDomainID = domainID2;
    gateway1.gatewayID = 0;
    gateway1.sinkID = gwSinkID1;
    gateway1.sourceID = gwSourceID1;
    gateway1.domainSourceID = domainID3;
    gateway1.domainSinkID = domainID2;
    gateway1.listSinkFormats = gwSink1.listConnectionFormats;
    gateway1.listSourceFormats = gwSource1.listConnectionFormats;
    gateway1.convertionMatrix.push_back(true);
    gateway1.name = "gateway";

    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway, gatewayID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway1, gatewayID1));

    std::vector<am_Route_s> listRoutes;
    std::vector<am_RoutingElement_s> listRoutingElements;
    am_RoutingElement_s hopp1;
    am_RoutingElement_s hopp2;
    am_RoutingElement_s hopp3;

    hopp1.sourceID = sourceID;
    hopp1.sinkID = gwSinkID;
    hopp1.domainID = domainID1;
    hopp1.connectionFormat = source.listConnectionFormats[0];

    hopp2.sourceID = gwSourceID;
    hopp2.sinkID = gwSinkID1;
    hopp2.domainID = domainID2;
    hopp2.connectionFormat = gwSink1.listConnectionFormats[0];

    hopp3.sourceID = gwSourceID1;
    hopp3.sinkID = sinkID;
    hopp3.domainID = domainID3;
    hopp3.connectionFormat = sink.listConnectionFormats[0];

    listRoutingElements.push_back(hopp1);
    listRoutingElements.push_back(hopp2);
    listRoutingElements.push_back(hopp3);

    am_Route_s compareRoute;
    compareRoute.route = listRoutingElements;
    compareRoute.sinkID = sinkID;
    compareRoute.sourceID = sourceID;

    am::am_Source_s sourceDb;
    am::am_Sink_s sinkDb;
    pDatabaseHandler.getSinkInfoDB(sinkID, sinkDb);
    pDatabaseHandler.getSourceInfoDB(sourceID, sourceDb);
    ASSERT_EQ(getRoute(false, false, sourceDb, sinkDb, listRoutes, 0), E_OK);
    ASSERT_EQ(static_cast<uint>(1), listRoutes.size());
    ASSERT_TRUE(pCF.compareRoute(compareRoute, listRoutes[0]));
}

//test that checks 3 domains, one sink one source, longer lists of connectionformats.
TEST_F(CAmRouterMapTest,simpleRoute3DomainsListConnectionFormats)
{
    EXPECT_CALL(pMockControlInterface,getConnectionFormatChoice(_,_,_,_,_)).WillRepeatedly(DoAll(returnConnectionFormat(), Return(E_OK)));

    //initialize 2 domains
    am_Domain_s domain1, domain2, domain3;
    am_domainID_t domainID1, domainID2, domainID3;

    domain1.domainID = 0;
    domain1.name = "domain1";
    domain1.busname = "domain1bus";
    domain1.state = DS_CONTROLLED;
    domain2.domainID = 0;
    domain2.name = "domain2";
    domain2.busname = "domain2bus";
    domain2.state = DS_CONTROLLED;
    domain3.domainID = 0;
    domain3.name = "domain3";
    domain3.busname = "domain3bus";
    domain3.state = DS_CONTROLLED;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain1, domainID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain2, domainID2));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain3, domainID3));

    am_Source_s source, gwSource, gwSource1;
    am_sourceID_t sourceID, gwSourceID, gwSourceID1;
    am_SourceClass_s sourceclass;

    sourceclass.name = "sClass";
    sourceclass.sourceClassID = 5;

    source.domainID = domainID1;
    source.name = "source1";
    source.sourceState = SS_ON;
    source.sourceID = 0;
    source.sourceClassID = 5;
    source.listConnectionFormats.push_back(CF_GENIVI_STEREO);
    source.listConnectionFormats.push_back(CF_GENIVI_MONO);

    gwSource.domainID = domainID2;
    gwSource.name = "gwsource1";
    gwSource.sourceState = SS_ON;
    gwSource.sourceID = 0;
    gwSource.sourceClassID = 5;
    gwSource.listConnectionFormats.push_back(CF_GENIVI_ANALOG);

    gwSource1.domainID = domainID3;
    gwSource1.name = "gwsource2";
    gwSource1.sourceState = SS_ON;
    gwSource1.sourceID = 0;
    gwSource1.sourceClassID = 5;
    gwSource1.listConnectionFormats.push_back(CF_GENIVI_MONO);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(source.sourceClassID, sourceclass));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source, sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource, gwSourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource1, gwSourceID1));

    am_Sink_s sink, gwSink, gwSink1;
    am_sinkID_t sinkID, gwSinkID, gwSinkID1;

    am_SinkClass_s sinkclass;
    sinkclass.sinkClassID = 5;
    sinkclass.name = "sname";

    sink.domainID = domainID3;
    sink.name = "sink1";
    sink.sinkID = 0;
    sink.sinkClassID = 5;
    sink.muteState = MS_MUTED;
    sink.listConnectionFormats.push_back(CF_GENIVI_MONO);

    gwSink.domainID = domainID1;
    gwSink.name = "gwSink";
    gwSink.sinkID = 0;
    gwSink.sinkClassID = 5;
    gwSink.muteState = MS_MUTED;
    gwSink.listConnectionFormats.push_back(CF_GENIVI_STEREO);
    gwSink.listConnectionFormats.push_back(CF_GENIVI_MONO);

    gwSink1.domainID = domainID2;
    gwSink1.name = "gwSink1";
    gwSink1.sinkID = 0;
    gwSink1.sinkClassID = 5;
    gwSink1.muteState = MS_MUTED;
    gwSink1.listConnectionFormats.push_back(CF_GENIVI_ANALOG);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkclass, sinkclass.sinkClassID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink, sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink, gwSinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink1, gwSinkID1));

    am_Gateway_s gateway, gateway1;
    am_gatewayID_t gatewayID, gatewayID1;

    gateway.controlDomainID = domainID1;
    gateway.gatewayID = 0;
    gateway.sinkID = gwSinkID;
    gateway.sourceID = gwSourceID;
    gateway.domainSourceID = domainID2;
    gateway.domainSinkID = domainID1;
    gateway.listSinkFormats = gwSink.listConnectionFormats;
    gateway.listSourceFormats = gwSource.listConnectionFormats;
    gateway.convertionMatrix.push_back(false);
    gateway.convertionMatrix.push_back(true);
    gateway.name = "gateway";

    gateway1.controlDomainID = domainID2;
    gateway1.gatewayID = 0;
    gateway1.sinkID = gwSinkID1;
    gateway1.sourceID = gwSourceID1;
    gateway1.domainSourceID = domainID3;
    gateway1.domainSinkID = domainID2;
    gateway1.listSinkFormats = gwSink1.listConnectionFormats;
    gateway1.listSourceFormats = gwSource1.listConnectionFormats;
    gateway1.convertionMatrix.push_back(true);
    gateway1.name = "gateway";

    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway, gatewayID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway1, gatewayID1));

    std::vector<am_Route_s> listRoutes;
    std::vector<am_RoutingElement_s> listRoutingElements;
    am_RoutingElement_s hopp1;
    am_RoutingElement_s hopp2;
    am_RoutingElement_s hopp3;

    hopp1.sourceID = sourceID;
    hopp1.sinkID = gwSinkID;
    hopp1.domainID = domainID1;
    hopp1.connectionFormat = source.listConnectionFormats[1];

    hopp2.sourceID = gwSourceID;
    hopp2.sinkID = gwSinkID1;
    hopp2.domainID = domainID2;
    hopp2.connectionFormat = gwSink1.listConnectionFormats[0];

    hopp3.sourceID = gwSourceID1;
    hopp3.sinkID = sinkID;
    hopp3.domainID = domainID3;
    hopp3.connectionFormat = sink.listConnectionFormats[0];

    listRoutingElements.push_back(hopp1);
    listRoutingElements.push_back(hopp2);
    listRoutingElements.push_back(hopp3);

    am_Route_s compareRoute;
    compareRoute.route = listRoutingElements;
    compareRoute.sinkID = sinkID;
    compareRoute.sourceID = sourceID;

    am::am_Source_s sourceDb;
    am::am_Sink_s sinkDb;
    pDatabaseHandler.getSinkInfoDB(sinkID, sinkDb);
    pDatabaseHandler.getSourceInfoDB(sourceID, sourceDb);
    ASSERT_EQ(getRoute(false, false, sourceDb, sinkDb, listRoutes, 0), E_OK);
    ASSERT_EQ(static_cast<uint>(1), listRoutes.size());
    ASSERT_TRUE(pCF.compareRoute(compareRoute, listRoutes[0]));
}

//test that checks 4 domains, one sink and one source but there are 2 routes because there are 2 gateways
TEST_F(CAmRouterMapTest,simpleRoute4Domains2Routes)
{
    EXPECT_CALL(pMockControlInterface,getConnectionFormatChoice(_,_,_,_,_)).WillRepeatedly(DoAll(returnConnectionFormat(), Return(E_OK)));

    //initialize 2 domains
    am_Domain_s domain1, domain2, domain3, domain4;
    am_domainID_t domainID1, domainID2, domainID3, domainID4;

    domain1.domainID = 0;
    domain1.name = "domain1";
    domain1.busname = "domain1bus";
    domain1.state = DS_CONTROLLED;
    domain2.domainID = 0;
    domain2.name = "domain2";
    domain2.busname = "domain2bus";
    domain2.state = DS_CONTROLLED;
    domain3.domainID = 0;
    domain3.name = "domain3";
    domain3.busname = "domain3bus";
    domain3.state = DS_CONTROLLED;
    domain4.domainID = 0;
    domain4.name = "domain4";
    domain4.busname = "domain4bus";
    domain4.state = DS_CONTROLLED;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain1, domainID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain2, domainID2));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain3, domainID3));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain4, domainID4));

    am_Source_s source, gwSource, gwSource1, gwSource2, gwSource3;
    am_sourceID_t sourceID, gwSourceID, gwSourceID1, gwSourceID2, gwSourceID3;
    am_SourceClass_s sourceclass;

    sourceclass.name = "sClass";
    sourceclass.sourceClassID = 5;

    source.domainID = domainID1;
    source.name = "source1";
    source.sourceState = SS_ON;
    source.sourceID = 0;
    source.sourceClassID = 5;
    source.listConnectionFormats.push_back(CF_GENIVI_MONO);

    gwSource.domainID = domainID2;
    gwSource.name = "gwsource1";
    gwSource.sourceState = SS_ON;
    gwSource.sourceID = 0;
    gwSource.sourceClassID = 5;
    gwSource.listConnectionFormats.push_back(CF_GENIVI_ANALOG);

    gwSource1.domainID = domainID3;
    gwSource1.name = "gwsource2";
    gwSource1.sourceState = SS_ON;
    gwSource1.sourceID = 0;
    gwSource1.sourceClassID = 5;
    gwSource1.listConnectionFormats.push_back(CF_GENIVI_MONO);

    gwSource2.domainID = domainID4;
    gwSource2.name = "gwsource3";
    gwSource2.sourceState = SS_OFF;
    gwSource2.sourceID = 0;
    gwSource2.sourceClassID = 5;
    gwSource2.listConnectionFormats.push_back(CF_GENIVI_STEREO);

    gwSource3.domainID = domainID3;
    gwSource3.name = "gwsource4";
    gwSource3.sourceState = SS_OFF;
    gwSource3.sourceID = 0;
    gwSource3.sourceClassID = 5;
    gwSource3.listConnectionFormats.push_back(CF_GENIVI_MONO);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(source.sourceClassID, sourceclass));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source, sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource, gwSourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource1, gwSourceID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource2, gwSourceID2));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource3, gwSourceID3));

    am_Sink_s sink, gwSink, gwSink1, gwSink2, gwSink3;
    am_sinkID_t sinkID, gwSinkID, gwSinkID1, gwSinkID2, gwSinkID3;

    gwSink.domainID = domainID1;
    gwSink.name = "gwSink";
    gwSink.sinkID = 0;
    gwSink.sinkClassID = 5;
    gwSink.muteState = MS_MUTED;
    gwSink.listConnectionFormats.push_back(CF_GENIVI_MONO);

    gwSink1.domainID = domainID2;
    gwSink1.name = "gwSink1";
    gwSink1.sinkID = 0;
    gwSink1.sinkClassID = 5;
    gwSink1.muteState = MS_MUTED;
    gwSink1.listConnectionFormats.push_back(CF_GENIVI_ANALOG);

    gwSink2.domainID = domainID3;
    gwSink2.name = "gwSink2";
    gwSink2.sinkID = 0;
    gwSink2.sinkClassID = 5;
    gwSink2.muteState = MS_MUTED;
    gwSink2.listConnectionFormats.push_back(CF_GENIVI_MONO);

    gwSink3.domainID = domainID2;
    gwSink3.name = "gwSink3";
    gwSink3.sinkID = 0;
    gwSink3.sinkClassID = 5;
    gwSink3.muteState = MS_MUTED;
    gwSink3.listConnectionFormats.push_back(CF_GENIVI_ANALOG);

    sink.domainID = domainID4;
    sink.name = "sink1";
    sink.sinkID = 0;
    sink.sinkClassID = 5;
    sink.muteState = MS_MUTED;
    sink.listConnectionFormats.push_back(CF_GENIVI_STEREO);
    am_SinkClass_s sinkclass;
    sinkclass.sinkClassID = 5;
    sinkclass.name = "sname";

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkclass, sinkclass.sinkClassID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink, sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink, gwSinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink1, gwSinkID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink2, gwSinkID2));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink3, gwSinkID3));

    am_Gateway_s gateway, gateway1, gateway2, gateway3;
    am_gatewayID_t gatewayID, gatewayID1, gatewayID2, gatewayID3;

    gateway.controlDomainID = domainID1;
    gateway.gatewayID = 0;
    gateway.sinkID = gwSinkID;
    gateway.sourceID = gwSourceID;
    gateway.domainSourceID = domainID2;
    gateway.domainSinkID = domainID1;
    gateway.listSinkFormats = gwSink.listConnectionFormats;
    gateway.listSourceFormats = gwSource.listConnectionFormats;
    gateway.convertionMatrix.push_back(true);
    gateway.name = "gateway";

    gateway1.controlDomainID = domainID2;
    gateway1.gatewayID = 0;
    gateway1.sinkID = gwSinkID1;
    gateway1.sourceID = gwSourceID1;
    gateway1.domainSourceID = domainID3;
    gateway1.domainSinkID = domainID2;
    gateway1.listSinkFormats = gwSink1.listConnectionFormats;
    gateway1.listSourceFormats = gwSource1.listConnectionFormats;
    gateway1.convertionMatrix.push_back(true);
    gateway1.name = "gateway1";

    gateway2.controlDomainID = domainID3;
    gateway2.gatewayID = 0;
    gateway2.sinkID = gwSinkID2;
    gateway2.sourceID = gwSourceID2;
    gateway2.domainSourceID = domainID4;
    gateway2.domainSinkID = domainID3;
    gateway2.listSinkFormats = gwSink2.listConnectionFormats;
    gateway2.listSourceFormats = gwSource2.listConnectionFormats;
    gateway2.convertionMatrix.push_back(true);
    gateway2.name = "gateway2";

    gateway3.controlDomainID = domainID2;
    gateway3.gatewayID = 0;
    gateway3.sinkID = gwSinkID3;
    gateway3.sourceID = gwSourceID3;
    gateway3.domainSourceID = domainID3;
    gateway3.domainSinkID = domainID2;
    gateway3.listSinkFormats = gwSink3.listConnectionFormats;
    gateway3.listSourceFormats = gwSource3.listConnectionFormats;
    gateway3.convertionMatrix.push_back(true);
    gateway3.name = "gateway3";

    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway, gatewayID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway1, gatewayID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway2, gatewayID2));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway3, gatewayID3));

    std::vector<am_Route_s> listRoutes;
    std::vector<am_RoutingElement_s> listRoutingElements, listRoutingElements1;
    am_RoutingElement_s hopp1;
    am_RoutingElement_s hopp2;
    am_RoutingElement_s hopp3;
    am_RoutingElement_s hopp4;
    am_RoutingElement_s hopp2alt;
    am_RoutingElement_s hopp3alt;

    hopp1.sourceID = sourceID;
    hopp1.sinkID = gwSinkID;
    hopp1.domainID = domainID1;
    hopp1.connectionFormat = source.listConnectionFormats[0];

    hopp2.sourceID = gwSourceID;
    hopp2.sinkID = gwSinkID1;
    hopp2.domainID = domainID2;
    hopp2.connectionFormat = gwSink1.listConnectionFormats[0];

    hopp3.sourceID = gwSourceID1;
    hopp3.sinkID = gwSinkID2;
    hopp3.domainID = domainID3;
    hopp3.connectionFormat = gwSink2.listConnectionFormats[0];

    hopp4.sourceID = gwSourceID2;
    hopp4.sinkID = sinkID;
    hopp4.domainID = domainID4;
    hopp4.connectionFormat = sink.listConnectionFormats[0];

    hopp2alt.sourceID = gwSourceID;
    hopp2alt.sinkID = gwSinkID3;
    hopp2alt.domainID = domainID2;
    hopp2alt.connectionFormat = gwSink3.listConnectionFormats[0];

    hopp3alt.sourceID = gwSourceID3;
    hopp3alt.sinkID = gwSinkID2;
    hopp3alt.domainID = domainID3;
    hopp3alt.connectionFormat = gwSink2.listConnectionFormats[0];

    listRoutingElements.push_back(hopp1);
    listRoutingElements.push_back(hopp2);
    listRoutingElements.push_back(hopp3);
    listRoutingElements.push_back(hopp4);
    listRoutingElements1.push_back(hopp1);
    listRoutingElements1.push_back(hopp2alt);
    listRoutingElements1.push_back(hopp3alt);
    listRoutingElements1.push_back(hopp4);

    am_Route_s compareRoute, compareRoute1;
    compareRoute.route = listRoutingElements;
    compareRoute.sinkID = sinkID;
    compareRoute.sourceID = sourceID;

    compareRoute1.route = listRoutingElements1;
    compareRoute1.sinkID = sinkID;
    compareRoute1.sourceID = sourceID;

    am::am_Source_s sourceDb;
    am::am_Sink_s sinkDb;
    pDatabaseHandler.getSinkInfoDB(sinkID, sinkDb);
    pDatabaseHandler.getSourceInfoDB(sourceID, sourceDb);
    ASSERT_EQ(getRoute(false, false, sourceDb, sinkDb, listRoutes, 0), E_OK);
    ASSERT_EQ(static_cast<uint>(2), listRoutes.size());

    bool containsRoute1 = std::find_if(listRoutes.begin(), listRoutes.end(), [&](const am_Route_s & ref)
    {
        return pCF.compareRoute(compareRoute, ref);
    }) != listRoutes.end();
    bool containsRoute2 = std::find_if(listRoutes.begin(), listRoutes.end(), [&](const am_Route_s & ref)
    {
        return pCF.compareRoute(compareRoute1, ref);
    }) != listRoutes.end();

    ASSERT_TRUE(containsRoute1);
    ASSERT_TRUE(containsRoute2);
}

//test that checks 3 domains, one sink one source but the connectionformat of third domains do not fit.
TEST_F(CAmRouterMapTest,simpleRoute3DomainsNoConnection)
{
    EXPECT_CALL(pMockControlInterface,getConnectionFormatChoice(_,_,_,_,_)).WillRepeatedly(DoAll(returnConnectionFormat(), Return(E_OK)));

    //initialize 2 domains
    am_Domain_s domain1, domain2, domain3;
    am_domainID_t domainID1, domainID2, domainID3;

    domain1.domainID = 0;
    domain1.name = "domain1";
    domain1.busname = "domain1bus";
    domain1.state = DS_CONTROLLED;
    domain2.domainID = 0;
    domain2.name = "domain2";
    domain2.busname = "domain2bus";
    domain2.state = DS_CONTROLLED;
    domain3.domainID = 0;
    domain3.name = "domain3";
    domain3.busname = "domain3bus";
    domain3.state = DS_CONTROLLED;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain1, domainID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain2, domainID2));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain3, domainID3));

    am_Source_s source, gwSource, gwSource1;
    am_sourceID_t sourceID, gwSourceID, gwSourceID1;

    source.domainID = domainID1;
    source.name = "source1";
    source.sourceState = SS_ON;
    source.sourceID = 0;
    source.sourceClassID = 5;
    source.listConnectionFormats.push_back(CF_GENIVI_MONO);
    am_SourceClass_s sourceclass;

    sourceclass.name = "sClass";
    sourceclass.sourceClassID = 5;

    gwSource.domainID = domainID2;
    gwSource.name = "gwsource1";
    gwSource.sourceState = SS_ON;
    gwSource.sourceID = 0;
    gwSource.sourceClassID = 5;
    gwSource.listConnectionFormats.push_back(CF_GENIVI_ANALOG);

    gwSource1.domainID = domainID3;
    gwSource1.name = "gwsource2";
    gwSource1.sourceState = SS_ON;
    gwSource1.sourceID = 0;
    gwSource1.sourceClassID = 5;
    gwSource1.listConnectionFormats.push_back(CF_GENIVI_MONO);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(source.sourceClassID, sourceclass));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source, sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource, gwSourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource1, gwSourceID1));

    am_Sink_s sink, gwSink, gwSink1;
    am_sinkID_t sinkID, gwSinkID, gwSinkID1;

    sink.domainID = domainID3;
    sink.name = "sink1";
    sink.sinkID = 0;
    sink.sinkClassID = 5;
    sink.muteState = MS_MUTED;
    sink.listConnectionFormats.push_back(CF_GENIVI_STEREO);
    am_SinkClass_s sinkclass;
    sinkclass.sinkClassID = 5;
    sinkclass.name = "sname";

    gwSink.domainID = domainID1;
    gwSink.name = "gwSink";
    gwSink.sinkID = 0;
    gwSink.sinkClassID = 5;
    gwSink.muteState = MS_MUTED;
    gwSink.listConnectionFormats.push_back(CF_GENIVI_MONO);

    gwSink1.domainID = domainID2;
    gwSink1.name = "gwSink1";
    gwSink1.sinkID = 0;
    gwSink1.sinkClassID = 5;
    gwSink1.muteState = MS_MUTED;
    gwSink1.listConnectionFormats.push_back(CF_GENIVI_ANALOG);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkclass, sinkclass.sinkClassID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink, sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink, gwSinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink1, gwSinkID1));

    am_Gateway_s gateway, gateway1;
    am_gatewayID_t gatewayID, gatewayID1;

    gateway.controlDomainID = domainID1;
    gateway.gatewayID = 0;
    gateway.sinkID = gwSinkID;
    gateway.sourceID = gwSourceID;
    gateway.domainSourceID = domainID2;
    gateway.domainSinkID = domainID1;
    gateway.listSinkFormats = gwSink.listConnectionFormats;
    gateway.listSourceFormats = gwSource.listConnectionFormats;
    gateway.convertionMatrix.push_back(true);
    gateway.name = "gateway";

    gateway1.controlDomainID = domainID2;
    gateway1.gatewayID = 0;
    gateway1.sinkID = gwSinkID1;
    gateway1.sourceID = gwSourceID1;
    gateway1.domainSourceID = domainID3;
    gateway1.domainSinkID = domainID2;
    gateway1.listSinkFormats = gwSink1.listConnectionFormats;
    gateway1.listSourceFormats = gwSource1.listConnectionFormats;
    gateway1.convertionMatrix.push_back(true);
    gateway1.name = "gateway";

    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway, gatewayID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway1, gatewayID1));

    std::vector<am_Route_s> listRoutes;
    std::vector<am_RoutingElement_s> listRoutingElements;
    am_RoutingElement_s hopp1;
    am_RoutingElement_s hopp2;
    am_RoutingElement_s hopp3;

    hopp1.sourceID = sourceID;
    hopp1.sinkID = gwSinkID;
    hopp1.domainID = domainID1;
    hopp1.connectionFormat = source.listConnectionFormats[0];

    hopp2.sourceID = gwSourceID;
    hopp2.sinkID = gwSinkID1;
    hopp2.domainID = domainID2;
    hopp2.connectionFormat = gwSink1.listConnectionFormats[0];

    hopp3.sourceID = gwSourceID1;
    hopp3.sinkID = sinkID;
    hopp3.domainID = domainID3;
    hopp3.connectionFormat = sink.listConnectionFormats[0];

    listRoutingElements.push_back(hopp1);
    listRoutingElements.push_back(hopp2);
    listRoutingElements.push_back(hopp3);

    am_Route_s compareRoute;
    compareRoute.route = listRoutingElements;
    compareRoute.sinkID = sinkID;
    compareRoute.sourceID = sourceID;

    am::am_Source_s sourceDb;
    am::am_Sink_s sinkDb;
    pDatabaseHandler.getSinkInfoDB(sinkID, sinkDb);
    pDatabaseHandler.getSourceInfoDB(sourceID, sourceDb);
    listRoutes.clear();
    ASSERT_EQ(getRoute(false, false, sourceDb, sinkDb, listRoutes, 0), E_NOT_POSSIBLE);
    ASSERT_EQ(static_cast<uint>(0), listRoutes.size());
}

//test that checks just 2 domains, one sink one source with only one connection format each
TEST_F(CAmRouterMapTest,simpleRoute2Domains)
{
    EXPECT_CALL(pMockControlInterface,getConnectionFormatChoice(_,_,_,_,_)).WillRepeatedly(DoAll(returnConnectionFormat(), Return(E_OK)));

    //initialize 2 domains
    am_Domain_s domain1, domain2;
    am_domainID_t domainID1, domainID2;

    domain1.domainID = 0;
    domain1.name = "domain1";
    domain1.busname = "domain1bus";
    domain1.state = DS_CONTROLLED;
    domain2.domainID = 0;
    domain2.name = "domain2";
    domain2.busname = "domain2bus";
    domain2.state = DS_CONTROLLED;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain1, domainID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain2, domainID2));

    am_Source_s source, gwSource;
    am_sourceID_t sourceID, gwSourceID;

    source.domainID = domainID1;
    source.name = "source1";
    source.sourceState = SS_ON;
    source.sourceID = 0;
    source.sourceClassID = 5;
    source.listConnectionFormats.push_back(CF_GENIVI_ANALOG);
    am_SourceClass_s sourceclass;

    sourceclass.name = "sClass";
    sourceclass.sourceClassID = 5;

    gwSource.domainID = domainID2;
    gwSource.name = "gwsource1";
    gwSource.sourceState = SS_ON;
    gwSource.sourceID = 0;
    gwSource.sourceClassID = 5;
    gwSource.listConnectionFormats.push_back(CF_GENIVI_MONO);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(source.sourceClassID, sourceclass));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source, sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource, gwSourceID));

    am_Sink_s sink, gwSink;
    am_sinkID_t sinkID, gwSinkID;
    am_SinkClass_s sinkclass;
    sinkclass.sinkClassID = 5;
    sinkclass.name = "sname";

    sink.domainID = domainID2;
    sink.name = "sink1";
    sink.sinkID = 0;
    sink.sinkClassID = 5;
    sink.muteState = MS_MUTED;
    sink.listConnectionFormats.push_back(CF_GENIVI_MONO);

    gwSink.domainID = domainID1;
    gwSink.name = "gwSink";
    gwSink.sinkID = 0;
    gwSink.sinkClassID = 5;
    gwSink.muteState = MS_MUTED;
    gwSink.listConnectionFormats.push_back(CF_GENIVI_ANALOG);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkclass, sinkclass.sinkClassID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink, sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink, gwSinkID));

    am_Gateway_s gateway;
    am_gatewayID_t gatewayID;

    gateway.controlDomainID = domainID1;
    gateway.gatewayID = 0;
    gateway.sinkID = gwSinkID;
    gateway.sourceID = gwSourceID;
    gateway.domainSourceID = domainID2;
    gateway.domainSinkID = domainID1;
    gateway.listSinkFormats = gwSink.listConnectionFormats;
    gateway.listSourceFormats = gwSource.listConnectionFormats;
    gateway.convertionMatrix.push_back(true);
    gateway.name = "gateway";

    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway, gatewayID));

    std::vector<am_Route_s> listRoutes;
    std::vector<am_RoutingElement_s> listRoutingElements;
    am_RoutingElement_s hopp1;
    am_RoutingElement_s hopp2;

    hopp1.sinkID = gwSinkID;
    hopp1.sourceID = sourceID;
    hopp1.domainID = domainID1;
    hopp1.connectionFormat = source.listConnectionFormats[0];

    hopp2.sinkID = sinkID;
    hopp2.sourceID = gwSourceID;
    hopp2.domainID = domainID2;
    hopp2.connectionFormat = sink.listConnectionFormats[0];

    listRoutingElements.push_back(hopp1);
    listRoutingElements.push_back(hopp2);

    am_Route_s compareRoute;
    compareRoute.route = listRoutingElements;
    compareRoute.sinkID = sinkID;
    compareRoute.sourceID = sourceID;

    am::am_Source_s sourceDb;
    am::am_Sink_s sinkDb;
    pDatabaseHandler.getSinkInfoDB(sinkID, sinkDb);
    pDatabaseHandler.getSourceInfoDB(sourceID, sourceDb);
    ASSERT_EQ(getRoute(false, false, sourceDb, sinkDb, listRoutes, 0), E_OK);
    ASSERT_EQ(static_cast<uint>(1), listRoutes.size());
    ASSERT_TRUE(pCF.compareRoute(compareRoute, listRoutes[0]));
}

//test that checks just 2 domains, one sink one source but the connectionformat of source
TEST_F(CAmRouterMapTest,simpleRoute2DomainsNoMatchConnectionFormats)
{
    EXPECT_CALL(pMockControlInterface,getConnectionFormatChoice(_,_,_,_,_)).WillRepeatedly(DoAll(returnConnectionFormat(), Return(E_OK)));

    //initialize 2 domains
    am_Domain_s domain1, domain2;
    am_domainID_t domainID1, domainID2;

    domain1.domainID = 0;
    domain1.name = "domain1";
    domain1.busname = "domain1bus";
    domain1.state = DS_CONTROLLED;
    domain2.domainID = 0;
    domain2.name = "domain2";
    domain2.busname = "domain2bus";
    domain2.state = DS_CONTROLLED;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain1, domainID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain2, domainID2));

    am_Source_s source, gwSource;
    am_sourceID_t sourceID, gwSourceID;

    source.domainID = domainID1;
    source.name = "source1";
    source.sourceState = SS_ON;
    source.sourceID = 0;
    source.sourceClassID = 5;
    source.listConnectionFormats.push_back(CF_GENIVI_STEREO);
    am_SourceClass_s sourceclass;

    sourceclass.name = "sClass";
    sourceclass.sourceClassID = 5;

    gwSource.domainID = domainID2;
    gwSource.name = "gwsource1";
    gwSource.sourceState = SS_ON;
    gwSource.sourceID = 0;
    gwSource.sourceClassID = 5;
    gwSource.listConnectionFormats.push_back(CF_GENIVI_MONO);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(source.sourceClassID, sourceclass));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source, sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource, gwSourceID));

    am_Sink_s sink, gwSink;
    am_sinkID_t sinkID, gwSinkID;
    am_SinkClass_s sinkclass;
    sinkclass.sinkClassID = 5;
    sinkclass.name = "sname";

    sink.domainID = domainID2;
    sink.name = "sink1";
    sink.sinkID = 0;
    sink.sinkClassID = 5;
    sink.muteState = MS_MUTED;
    sink.listConnectionFormats.push_back(CF_GENIVI_MONO);

    gwSink.domainID = domainID1;
    gwSink.name = "gwSink";
    gwSink.sinkID = 0;
    gwSink.sinkClassID = 5;
    gwSink.muteState = MS_MUTED;
    gwSink.listConnectionFormats.push_back(CF_GENIVI_ANALOG);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkclass, sinkclass.sinkClassID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink, sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink, gwSinkID));

    am_Gateway_s gateway;
    am_gatewayID_t gatewayID;

    gateway.controlDomainID = domainID1;
    gateway.gatewayID = 0;
    gateway.sinkID = gwSinkID;
    gateway.sourceID = gwSourceID;
    gateway.domainSourceID = domainID2;
    gateway.domainSinkID = domainID1;
    gateway.listSinkFormats = gwSink.listConnectionFormats;
    gateway.listSourceFormats = gwSource.listConnectionFormats;
    gateway.convertionMatrix.push_back(true);
    gateway.name = "gateway";

    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway, gatewayID));

    std::vector<am_Route_s> listRoutes;

    am::am_Source_s sourceDb;
    am::am_Sink_s sinkDb;
    pDatabaseHandler.getSinkInfoDB(sinkID, sinkDb);
    pDatabaseHandler.getSourceInfoDB(sourceID, sourceDb);
    ASSERT_EQ(getRoute(false, false, sourceDb, sinkDb, listRoutes, 0), E_NOT_POSSIBLE);
    ASSERT_EQ(static_cast<uint>(0), listRoutes.size());
}

//test that checks 3 domains, one sink one source.
TEST_F(CAmRouterMapTest,simpleRoute3Domains)
{
    EXPECT_CALL(pMockControlInterface,getConnectionFormatChoice(_,_,_,_,_)).WillRepeatedly(DoAll(returnConnectionFormat(), Return(E_OK)));

    //initialize 2 domains
    am_Domain_s domain1, domain2, domain3;
    am_domainID_t domainID1, domainID2, domainID3;

    domain1.domainID = 0;
    domain1.name = "domain1";
    domain1.busname = "domain1bus";
    domain1.state = DS_CONTROLLED;
    domain2.domainID = 0;
    domain2.name = "domain2";
    domain2.busname = "domain2bus";
    domain2.state = DS_CONTROLLED;
    domain3.domainID = 0;
    domain3.name = "domain3";
    domain3.busname = "domain3bus";
    domain3.state = DS_CONTROLLED;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain1, domainID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain2, domainID2));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain3, domainID3));

    am_Source_s source, gwSource, gwSource1;
    am_sourceID_t sourceID, gwSourceID, gwSourceID1;
    am_SourceClass_s sourceclass;

    sourceclass.name = "sClass";
    sourceclass.sourceClassID = 5;

    source.domainID = domainID1;
    source.name = "source1";
    source.sourceState = SS_ON;
    source.sourceID = 0;
    source.sourceClassID = 5;
    source.listConnectionFormats.push_back(CF_GENIVI_MONO);

    gwSource.domainID = domainID2;
    gwSource.name = "gwsource1";
    gwSource.sourceState = SS_ON;
    gwSource.sourceID = 0;
    gwSource.sourceClassID = 5;
    gwSource.listConnectionFormats.push_back(CF_GENIVI_ANALOG);

    gwSource1.domainID = domainID3;
    gwSource1.name = "gwsource2";
    gwSource1.sourceState = SS_ON;
    gwSource1.sourceID = 0;
    gwSource1.sourceClassID = 5;
    gwSource1.listConnectionFormats.push_back(CF_GENIVI_MONO);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(source.sourceClassID, sourceclass));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source, sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource, gwSourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource1, gwSourceID1));

    am_Sink_s sink, gwSink, gwSink1;
    am_sinkID_t sinkID, gwSinkID, gwSinkID1;
    am_SinkClass_s sinkclass;
    sinkclass.sinkClassID = 5;
    sinkclass.name = "sname";

    sink.domainID = domainID3;
    sink.name = "sink1";
    sink.sinkID = 0;
    sink.sinkClassID = 5;
    sink.muteState = MS_MUTED;
    sink.listConnectionFormats.push_back(CF_GENIVI_MONO);

    gwSink.domainID = domainID1;
    gwSink.name = "gwSink";
    gwSink.sinkID = 0;
    gwSink.sinkClassID = 5;
    gwSink.muteState = MS_MUTED;
    gwSink.listConnectionFormats.push_back(CF_GENIVI_MONO);

    gwSink1.domainID = domainID2;
    gwSink1.name = "gwSink1";
    gwSink1.sinkID = 0;
    gwSink1.sinkClassID = 5;
    gwSink1.muteState = MS_MUTED;
    gwSink1.listConnectionFormats.push_back(CF_GENIVI_ANALOG);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkclass, sinkclass.sinkClassID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink, sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink, gwSinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink1, gwSinkID1));

    am_Gateway_s gateway, gateway1;
    am_gatewayID_t gatewayID, gatewayID1;

    gateway.controlDomainID = domainID1;
    gateway.gatewayID = 0;
    gateway.sinkID = gwSinkID;
    gateway.sourceID = gwSourceID;
    gateway.domainSourceID = domainID2;
    gateway.domainSinkID = domainID1;
    gateway.listSinkFormats = gwSink.listConnectionFormats;
    gateway.listSourceFormats = gwSource.listConnectionFormats;
    gateway.convertionMatrix.push_back(true);
    gateway.name = "gateway";

    gateway1.controlDomainID = domainID2;
    gateway1.gatewayID = 0;
    gateway1.sinkID = gwSinkID1;
    gateway1.sourceID = gwSourceID1;
    gateway1.domainSourceID = domainID3;
    gateway1.domainSinkID = domainID2;
    gateway1.listSinkFormats = gwSink1.listConnectionFormats;
    gateway1.listSourceFormats = gwSource1.listConnectionFormats;
    gateway1.convertionMatrix.push_back(true);
    gateway1.name = "gateway";

    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway, gatewayID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway1, gatewayID1));

    std::vector<am_Route_s> listRoutes;
    std::vector<am_RoutingElement_s> listRoutingElements;
    am_RoutingElement_s hopp1;
    am_RoutingElement_s hopp2;
    am_RoutingElement_s hopp3;

    hopp1.sourceID = sourceID;
    hopp1.sinkID = gwSinkID;
    hopp1.domainID = domainID1;
    hopp1.connectionFormat = source.listConnectionFormats[0];

    hopp2.sourceID = gwSourceID;
    hopp2.sinkID = gwSinkID1;
    hopp2.domainID = domainID2;
    hopp2.connectionFormat = gwSink1.listConnectionFormats[0];

    hopp3.sourceID = gwSourceID1;
    hopp3.sinkID = sinkID;
    hopp3.domainID = domainID3;
    hopp3.connectionFormat = sink.listConnectionFormats[0];

    listRoutingElements.push_back(hopp1);
    listRoutingElements.push_back(hopp2);
    listRoutingElements.push_back(hopp3);

    am_Route_s compareRoute;
    compareRoute.route = listRoutingElements;
    compareRoute.sinkID = sinkID;
    compareRoute.sourceID = sourceID;

    am::am_Source_s sourceDb;
    am::am_Sink_s sinkDb;
    pDatabaseHandler.getSinkInfoDB(sinkID, sinkDb);
    pDatabaseHandler.getSourceInfoDB(sourceID, sourceDb);
    listRoutes.clear();
    ASSERT_EQ(getRoute(false, false, sourceDb, sinkDb, listRoutes, 0), E_OK);
    ASSERT_EQ(static_cast<uint>(1), listRoutes.size());
    ASSERT_TRUE(pCF.compareRoute(compareRoute, listRoutes[0]));
}

//test that checks 4 domains, one sink and one source.
TEST_F(CAmRouterMapTest,simpleRoute4Domains)
{
    EXPECT_CALL(pMockControlInterface,getConnectionFormatChoice(_,_,_,_,_)).WillRepeatedly(DoAll(returnConnectionFormat(), Return(E_OK)));

    //initialize 2 domains
    am_Domain_s domain1, domain2, domain3, domain4;
    am_domainID_t domainID1, domainID2, domainID3, domainID4;

    domain1.domainID = 0;
    domain1.name = "domain1";
    domain1.busname = "domain1bus";
    domain1.state = DS_CONTROLLED;
    domain2.domainID = 0;
    domain2.name = "domain2";
    domain2.busname = "domain2bus";
    domain2.state = DS_CONTROLLED;
    domain3.domainID = 0;
    domain3.name = "domain3";
    domain3.busname = "domain3bus";
    domain3.state = DS_CONTROLLED;
    domain4.domainID = 0;
    domain4.name = "domain4";
    domain4.busname = "domain4bus";
    domain4.state = DS_CONTROLLED;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain1, domainID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain2, domainID2));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain3, domainID3));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain4, domainID4));

    am_Source_s source, gwSource, gwSource1, gwSource2;
    am_sourceID_t sourceID, gwSourceID, gwSourceID1, gwSourceID2;
    am_SourceClass_s sourceclass;

    sourceclass.name = "sClass";
    sourceclass.sourceClassID = 5;

    source.domainID = domainID1;
    source.name = "source1";
    source.sourceState = SS_ON;
    source.sourceID = 0;
    source.sourceClassID = 5;
    source.listConnectionFormats.push_back(CF_GENIVI_MONO);

    gwSource.domainID = domainID2;
    gwSource.name = "gwsource1";
    gwSource.sourceState = SS_ON;
    gwSource.sourceID = 0;
    gwSource.sourceClassID = 5;
    gwSource.listConnectionFormats.push_back(CF_GENIVI_ANALOG);

    gwSource1.domainID = domainID3;
    gwSource1.name = "gwsource2";
    gwSource1.sourceState = SS_ON;
    gwSource1.sourceID = 0;
    gwSource1.sourceClassID = 5;
    gwSource1.listConnectionFormats.push_back(CF_GENIVI_MONO);

    gwSource2.domainID = domainID4;
    gwSource2.name = "gwsource3";
    gwSource2.sourceState = SS_OFF;
    gwSource2.sourceID = 0;
    gwSource2.sourceClassID = 5;
    gwSource2.listConnectionFormats.push_back(CF_GENIVI_STEREO);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(source.sourceClassID, sourceclass));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source, sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource, gwSourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource1, gwSourceID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource2, gwSourceID2));

    am_Sink_s sink, gwSink, gwSink1, gwSink2;
    am_sinkID_t sinkID, gwSinkID, gwSinkID1, gwSinkID2;

    gwSink.domainID = domainID1;
    gwSink.name = "gwSink";
    gwSink.sinkID = 0;
    gwSink.sinkClassID = 5;
    gwSink.muteState = MS_MUTED;
    gwSink.listConnectionFormats.push_back(CF_GENIVI_MONO);

    gwSink1.domainID = domainID2;
    gwSink1.name = "gwSink1";
    gwSink1.sinkID = 0;
    gwSink1.sinkClassID = 5;
    gwSink1.muteState = MS_MUTED;
    gwSink1.listConnectionFormats.push_back(CF_GENIVI_ANALOG);

    gwSink2.domainID = domainID3;
    gwSink2.name = "gwSink2";
    gwSink2.sinkID = 0;
    gwSink2.sinkClassID = 5;
    gwSink2.muteState = MS_MUTED;
    gwSink2.listConnectionFormats.push_back(CF_GENIVI_MONO);

    sink.domainID = domainID4;
    sink.name = "sink1";
    sink.sinkID = 0;
    sink.sinkClassID = 5;
    sink.muteState = MS_MUTED;
    sink.listConnectionFormats.push_back(CF_GENIVI_STEREO);
    am_SinkClass_s sinkclass;
    sinkclass.sinkClassID = 5;
    sinkclass.name = "sname";

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkclass, sinkclass.sinkClassID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink, sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink, gwSinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink1, gwSinkID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink2, gwSinkID2));

    am_Gateway_s gateway, gateway1, gateway2;
    am_gatewayID_t gatewayID, gatewayID1, gatewayID2;

    gateway.controlDomainID = domainID1;
    gateway.gatewayID = 0;
    gateway.sinkID = gwSinkID;
    gateway.sourceID = gwSourceID;
    gateway.domainSourceID = domainID2;
    gateway.domainSinkID = domainID1;
    gateway.listSinkFormats = gwSink.listConnectionFormats;
    gateway.listSourceFormats = gwSource.listConnectionFormats;
    gateway.convertionMatrix.push_back(true);
    gateway.name = "gateway";

    gateway1.controlDomainID = domainID2;
    gateway1.gatewayID = 0;
    gateway1.sinkID = gwSinkID1;
    gateway1.sourceID = gwSourceID1;
    gateway1.domainSourceID = domainID3;
    gateway1.domainSinkID = domainID2;
    gateway1.listSinkFormats = gwSink1.listConnectionFormats;
    gateway1.listSourceFormats = gwSource1.listConnectionFormats;
    gateway1.convertionMatrix.push_back(true);
    gateway1.name = "gateway1";

    gateway2.controlDomainID = domainID3;
    gateway2.gatewayID = 0;
    gateway2.sinkID = gwSinkID2;
    gateway2.sourceID = gwSourceID2;
    gateway2.domainSourceID = domainID4;
    gateway2.domainSinkID = domainID3;
    gateway2.listSinkFormats = gwSink2.listConnectionFormats;
    gateway2.listSourceFormats = gwSource2.listConnectionFormats;
    gateway2.convertionMatrix.push_back(true);
    gateway2.name = "gateway2";

    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway, gatewayID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway1, gatewayID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway2, gatewayID2));

    std::vector<am_Route_s> listRoutes;
    std::vector<am_RoutingElement_s> listRoutingElements;
    am_RoutingElement_s hopp1;
    am_RoutingElement_s hopp2;
    am_RoutingElement_s hopp3;
    am_RoutingElement_s hopp4;

    hopp1.sourceID = sourceID;
    hopp1.sinkID = gwSinkID;
    hopp1.domainID = domainID1;
    hopp1.connectionFormat = source.listConnectionFormats[0];

    hopp2.sourceID = gwSourceID;
    hopp2.sinkID = gwSinkID1;
    hopp2.domainID = domainID2;
    hopp2.connectionFormat = gwSink1.listConnectionFormats[0];

    hopp3.sourceID = gwSourceID1;
    hopp3.sinkID = gwSinkID2;
    hopp3.domainID = domainID3;
    hopp3.connectionFormat = gwSink2.listConnectionFormats[0];

    hopp4.sourceID = gwSourceID2;
    hopp4.sinkID = sinkID;
    hopp4.domainID = domainID4;
    hopp4.connectionFormat = sink.listConnectionFormats[0];

    listRoutingElements.push_back(hopp1);
    listRoutingElements.push_back(hopp2);
    listRoutingElements.push_back(hopp3);
    listRoutingElements.push_back(hopp4);

    am_Route_s compareRoute;
    compareRoute.route = listRoutingElements;
    compareRoute.sinkID = sinkID;
    compareRoute.sourceID = sourceID;

    am::am_Source_s sourceDb;
    am::am_Sink_s sinkDb;
    pDatabaseHandler.getSinkInfoDB(sinkID, sinkDb);
    pDatabaseHandler.getSourceInfoDB(sourceID, sourceDb);
    listRoutes.clear();
    ASSERT_EQ(getRoute(false, false, sourceDb, sinkDb, listRoutes, 0), E_OK);
    ASSERT_EQ(static_cast<uint>(1), listRoutes.size());
    ASSERT_TRUE(pCF.compareRoute(compareRoute, listRoutes[0]));
}

TEST_F(CAmRouterMapTest,getAllowedFormatsFromConvMatrix)
{
    std::vector<bool> convertionMatrix;
    convertionMatrix.push_back(1);
    convertionMatrix.push_back(0);
    convertionMatrix.push_back(0);
    convertionMatrix.push_back(1);
    convertionMatrix.push_back(1);
    convertionMatrix.push_back(0);

    std::vector<am_CustomConnectionFormat_t> listSourceFormats;
    listSourceFormats.push_back(CF_GENIVI_ANALOG);
    listSourceFormats.push_back(CF_GENIVI_STEREO);

    std::vector<am_CustomConnectionFormat_t> listSinkFormats;
    listSinkFormats.push_back(CF_GENIVI_MONO);
    listSinkFormats.push_back(CF_GENIVI_AUTO);
    listSinkFormats.push_back(CF_GENIVI_STEREO);

    std::vector<am_CustomConnectionFormat_t> sourceFormats;
    std::vector<am_CustomConnectionFormat_t> sinkFormats;

    ASSERT_TRUE(CAmRouter::getAllowedFormatsFromConvMatrix(convertionMatrix, listSourceFormats, listSinkFormats, sourceFormats, sinkFormats));

    ASSERT_TRUE(sourceFormats.size() == 3);
    ASSERT_TRUE(sinkFormats.size() == 3);
    ASSERT_TRUE(sourceFormats.at(0) == CF_GENIVI_ANALOG);
    ASSERT_TRUE(sourceFormats.at(1) == CF_GENIVI_STEREO);
    ASSERT_TRUE(sourceFormats.at(2) == CF_GENIVI_ANALOG);
    ASSERT_TRUE(sinkFormats.at(0) == CF_GENIVI_MONO);
    ASSERT_TRUE(sinkFormats.at(1) == CF_GENIVI_AUTO);
    ASSERT_TRUE(sinkFormats.at(2) == CF_GENIVI_STEREO);

    sinkFormats.clear();
    sourceFormats.clear();
    convertionMatrix.clear();
    listSinkFormats.clear();
    listSourceFormats.clear();

    convertionMatrix.push_back(1);
    listSinkFormats.push_back(CF_GENIVI_STEREO);
    listSourceFormats.push_back(CF_GENIVI_STEREO);

    ASSERT_TRUE(CAmRouter::getAllowedFormatsFromConvMatrix(convertionMatrix, listSourceFormats, listSinkFormats, sourceFormats, sinkFormats));

    sinkFormats.clear();
    sourceFormats.clear();
    convertionMatrix.clear();
    listSinkFormats.clear();
    listSourceFormats.clear();

    convertionMatrix.push_back(1);
    convertionMatrix.push_back(0);
    listSourceFormats.push_back(CF_GENIVI_STEREO);
    listSinkFormats.push_back(CF_GENIVI_STEREO);

    ASSERT_FALSE(CAmRouter::getAllowedFormatsFromConvMatrix(convertionMatrix, listSourceFormats, listSinkFormats, sourceFormats, sinkFormats));

    sinkFormats.clear();
    sourceFormats.clear();
    convertionMatrix.clear();
    listSinkFormats.clear();
    listSourceFormats.clear();

    convertionMatrix.push_back(1);
    listSinkFormats.push_back(CF_GENIVI_STEREO);

    ASSERT_FALSE(CAmRouter::getAllowedFormatsFromConvMatrix(convertionMatrix, listSourceFormats, listSinkFormats, sourceFormats, sinkFormats));
}

TEST_F(CAmRouterMapTest,route1Domain1Source1Sink)
{
    EXPECT_CALL(pMockControlInterface,getConnectionFormatChoice(_,_,_,_,_)).WillRepeatedly(DoAll(returnConnectionFormat(), Return(E_OK)));

    am_domainID_t domainID1;
    enterDomainDB("domain1", domainID1);

    am_sourceID_t sourceID;
    std::vector<am_CustomConnectionFormat_t> cf1;
    cf1.push_back(CF_GENIVI_STEREO);
    cf1.push_back(CF_GENIVI_ANALOG);
    am_SourceClass_s sourceclass;

    sourceclass.name = "sClass";
    sourceclass.sourceClassID = 5;
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(sourceclass.sourceClassID, sourceclass));
    enterSourceDB("source1", domainID1, cf1, sourceID);

    am_sinkID_t sinkID;
    std::vector<am_CustomConnectionFormat_t> cf2;
    cf2.push_back(CF_GENIVI_ANALOG);
    cf2.push_back(CF_GENIVI_MONO);
    am_SinkClass_s sinkclass;
    sinkclass.sinkClassID = 5;
    sinkclass.name = "sname";
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkclass, sinkclass.sinkClassID));
    enterSinkDB("sink1", domainID1, cf2, sinkID);

    am::am_Source_s source;
    am::am_Sink_s sink;

    pDatabaseHandler.getSinkInfoDB(sinkID, sink);
    pDatabaseHandler.getSourceInfoDB(sourceID, source);

    std::vector<am_Route_s> listRoutes;
    std::vector<am_RoutingElement_s> listRoutingElements;
    am_RoutingElement_s hopp1;

    hopp1.sourceID = sourceID;
    hopp1.sinkID = sinkID;
    hopp1.domainID = domainID1;
    hopp1.connectionFormat = cf2[0];

    listRoutingElements.push_back(hopp1);

    am_Route_s compareRoute;
    compareRoute.route = listRoutingElements;
    compareRoute.sinkID = sinkID;
    compareRoute.sourceID = sourceID;

    ASSERT_EQ(getRoute(false, false, source, sink, listRoutes, 0), E_OK);
    ASSERT_EQ(static_cast<uint>(1), listRoutes.size());
    ASSERT_TRUE(pCF.compareRoute(compareRoute, listRoutes[0]));
}

TEST_F(CAmRouterMapTest,route1Domain1Source1Converter1Sink)
{
    EXPECT_CALL(pMockControlInterface,getConnectionFormatChoice(_,_,_,_,_)).WillRepeatedly(DoAll(returnConnectionFormat(), Return(E_OK)));

    am_domainID_t domainID1;
    enterDomainDB("domain1", domainID1);

    am_sourceID_t sourceID;
    std::vector<am_CustomConnectionFormat_t> cf1;
    cf1.push_back(CF_GENIVI_STEREO);
    cf1.push_back(CF_GENIVI_AUTO);
    am_SourceClass_s sourceclass;

    sourceclass.name = "sClass";
    sourceclass.sourceClassID = 5;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(sourceclass.sourceClassID, sourceclass));
    enterSourceDB("source1", domainID1, cf1, sourceID);

    am_sinkID_t sinkID1, sinkID2;
    std::vector<am_CustomConnectionFormat_t> cf2;
    cf2.push_back(CF_GENIVI_MONO);
    cf2.push_back(CF_GENIVI_ANALOG);
    am_SinkClass_s sinkclass;
    sinkclass.sinkClassID = 5;
    sinkclass.name = "sname";

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkclass, sinkclass.sinkClassID));
    enterSinkDB("sink1", domainID1, cf2, sinkID1);
    enterSinkDB("sink2", domainID1, cf2, sinkID2);

    am_sourceID_t gwSourceID;
    std::vector<am_CustomConnectionFormat_t> cf3;
    cf3.push_back(CF_GENIVI_MONO);
    cf3.push_back(CF_GENIVI_ANALOG);
    enterSourceDB("gwSource1", domainID1, cf3, gwSourceID);

    am_sinkID_t gwSinkID;
    std::vector<am_CustomConnectionFormat_t> cf4;
    cf4.push_back(CF_GENIVI_STEREO);
    cf4.push_back(CF_GENIVI_ANALOG);
    enterSinkDB("gwSink1", domainID1, cf4, gwSinkID);

    am_converterID_t converterID;
    std::vector<bool> matrix;
    matrix.resize(4, false);
    matrix[0] = (true);
    matrix[1] = (true);
    enterConverterDB("converter", domainID1, cf3, cf4, matrix, gwSourceID, gwSinkID, converterID);

    am::am_Source_s source;
    am::am_Sink_s sink;

    pDatabaseHandler.getSinkInfoDB(sinkID1, sink);
    pDatabaseHandler.getSourceInfoDB(sourceID, source);

    std::vector<am_Route_s> listRoutes;
    std::vector<am_RoutingElement_s> listRoutingElements;
    am_RoutingElement_s hopp1;
    am_RoutingElement_s hopp2;

    hopp1.sourceID = sourceID;
    hopp1.sinkID = gwSinkID;
    hopp1.domainID = domainID1;
    hopp1.connectionFormat = CF_GENIVI_STEREO;

    hopp2.sourceID = gwSourceID;
    hopp2.sinkID = sinkID1;
    hopp2.domainID = domainID1;
    hopp2.connectionFormat = CF_GENIVI_MONO;

    listRoutingElements.push_back(hopp1);
    listRoutingElements.push_back(hopp2);

    am_Route_s compareRoute;
    compareRoute.route = listRoutingElements;
    compareRoute.sinkID = sinkID1;
    compareRoute.sourceID = sourceID;

    ASSERT_EQ(getRoute(false, false, source, sink, listRoutes, 0), E_OK);
    ASSERT_EQ(static_cast<uint>(1), listRoutes.size());
    ASSERT_TRUE(pCF.compareRoute(compareRoute, listRoutes[0]));
}

TEST_F(CAmRouterMapTest,route1Domain1Source3Converters1Sink)
{
    EXPECT_CALL(pMockControlInterface,getConnectionFormatChoice(_,_,_,_,_)).WillRepeatedly(DoAll(returnConnectionFormat(), Return(E_OK)));

    am_SourceClass_s sourceclass;

    sourceclass.name = "sClass";
    sourceclass.sourceClassID = 5;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(sourceclass.sourceClassID, sourceclass));

    am_SinkClass_s sinkclass;
    sinkclass.sinkClassID = 5;
    sinkclass.name = "sname";

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkclass, sinkclass.sinkClassID));

    am_domainID_t domainID1;
    enterDomainDB("domain1", domainID1);

    std::vector<am_CustomConnectionFormat_t> cf1;
    cf1.push_back(CF_GENIVI_STEREO);
    std::vector<am_CustomConnectionFormat_t> cf2;
    cf2.push_back(CF_GENIVI_MONO);
    std::vector<am_CustomConnectionFormat_t> cf3;
    cf3.push_back(CF_GENIVI_AUTO);

    am_sourceID_t sourceID;
    enterSourceDB("source1", domainID1, cf1, sourceID);

    am_sinkID_t sinkID;
    enterSinkDB("sink1", domainID1, cf3, sinkID);

    am_sourceID_t gwSourceID;
    enterSourceDB("gwSource1", domainID1, cf2, gwSourceID);
    am_sinkID_t gwSinkID;
    enterSinkDB("gwSink1", domainID1, cf1, gwSinkID);
    am_converterID_t converterID;
    std::vector<bool> matrix;
    matrix.push_back(true);
    enterConverterDB("converter1", domainID1, cf2, cf1, matrix, gwSourceID, gwSinkID, converterID);

    am_sourceID_t gwSourceID1;
    enterSourceDB("gwSource2", domainID1, cf2, gwSourceID1);
    am_sinkID_t gwSinkID1;
    enterSinkDB("gwSink2", domainID1, cf1, gwSinkID1);
    am_converterID_t converterID1;
    enterConverterDB("converter2", domainID1, cf2, cf1, matrix, gwSourceID1, gwSinkID1, converterID1);

    am_sourceID_t gwSourceID2;
    enterSourceDB("gwSource3", domainID1, cf3, gwSourceID2);
    am_sinkID_t gwSinkID2;
    enterSinkDB("gwSink3", domainID1, cf2, gwSinkID2);
    am_converterID_t converterID2;
    enterConverterDB("converter3", domainID1, cf3, cf2, matrix, gwSourceID2, gwSinkID2, converterID2);

    am::am_Source_s source;
    am::am_Sink_s sink;
    pDatabaseHandler.getSinkInfoDB(sinkID, sink);
    pDatabaseHandler.getSourceInfoDB(sourceID, source);

    std::vector<am_Route_s> listRoutes;
    std::vector<am_RoutingElement_s> listRoutingElements1;
    std::vector<am_RoutingElement_s> listRoutingElements2;
    am_RoutingElement_s hopp11;
    am_RoutingElement_s hopp12;
    am_RoutingElement_s hopp13;
    am_RoutingElement_s hopp21;
    am_RoutingElement_s hopp22;

    hopp11.sourceID = sourceID;
    hopp11.sinkID = gwSinkID;
    hopp11.domainID = domainID1;
    hopp11.connectionFormat = CF_GENIVI_STEREO;

    hopp12.sourceID = gwSourceID;
    hopp12.sinkID = gwSinkID2;
    hopp12.domainID = domainID1;
    hopp12.connectionFormat = CF_GENIVI_MONO;

    hopp21.sourceID = sourceID;
    hopp21.sinkID = gwSinkID1;
    hopp21.domainID = domainID1;
    hopp21.connectionFormat = CF_GENIVI_STEREO;

    hopp22.sourceID = gwSourceID1;
    hopp22.sinkID = gwSinkID2;
    hopp22.domainID = domainID1;
    hopp22.connectionFormat = CF_GENIVI_MONO;

    hopp13.sourceID = gwSourceID2;
    hopp13.sinkID = sinkID;
    hopp13.domainID = domainID1;
    hopp13.connectionFormat = CF_GENIVI_AUTO;

    listRoutingElements1.push_back(hopp11);
    listRoutingElements1.push_back(hopp12);
    listRoutingElements1.push_back(hopp13);

    listRoutingElements2.push_back(hopp21);
    listRoutingElements2.push_back(hopp22);
    listRoutingElements2.push_back(hopp13);

    am_Route_s compareRoute1;
    compareRoute1.route = listRoutingElements1;
    compareRoute1.sinkID = sinkID;
    compareRoute1.sourceID = sourceID;

    ASSERT_EQ(getRoute(false, false, source, sink, listRoutes, 0), E_OK);
    ASSERT_EQ(static_cast<uint>(2), listRoutes.size());
    ASSERT_TRUE(pCF.compareRoute(compareRoute1, listRoutes[0]) || pCF.compareRoute(compareRoute1, listRoutes[1]));

    am_Route_s compareRoute2;
    compareRoute2.route = listRoutingElements2;
    compareRoute2.sinkID = sinkID;
    compareRoute2.sourceID = sourceID;
    ASSERT_TRUE(pCF.compareRoute(compareRoute2, listRoutes[1]) || pCF.compareRoute(compareRoute2, listRoutes[0]));
}

TEST_F(CAmRouterMapTest,route2Domains1Source1Sink)
{
    EXPECT_CALL(pMockControlInterface,getConnectionFormatChoice(_,_,_,_,_)).WillRepeatedly(DoAll(returnConnectionFormat(), Return(E_OK)));

    am_SourceClass_s sourceclass;

    sourceclass.name = "sClass";
    sourceclass.sourceClassID = 5;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(sourceclass.sourceClassID, sourceclass));

    am_SinkClass_s sinkclass;
    sinkclass.sinkClassID = 5;
    sinkclass.name = "sname";

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkclass, sinkclass.sinkClassID));

    am_domainID_t domainID1, domainID2;
    enterDomainDB("domain1", domainID1);
    enterDomainDB("domain2", domainID2);

    am_sourceID_t sourceID;
    std::vector<am_CustomConnectionFormat_t> cf1;
    cf1.push_back(CF_GENIVI_STEREO);
    enterSourceDB("source1", domainID1, cf1, sourceID);

    am_sinkID_t sinkID;
    std::vector<am_CustomConnectionFormat_t> cf2;
    cf2.push_back(CF_GENIVI_ANALOG);
    enterSinkDB("sink1", domainID2, cf2, sinkID);

    am_sourceID_t gwSourceID;
    std::vector<am_CustomConnectionFormat_t> cf3;
    cf3.push_back(CF_GENIVI_ANALOG);
    enterSourceDB("gwSource1", domainID2, cf3, gwSourceID);

    am_sinkID_t gwSinkID;
    std::vector<am_CustomConnectionFormat_t> cf4;
    cf4.push_back(CF_GENIVI_STEREO);
    enterSinkDB("gwSink1", domainID1, cf4, gwSinkID);

    am_gatewayID_t gatewayID;
    std::vector<bool> matrix;
    matrix.push_back(true);
    enterGatewayDB("gateway", domainID2, domainID1, cf3, cf4, matrix, gwSourceID, gwSinkID, gatewayID);

    am::am_Source_s source;
    am::am_Sink_s sink;

    pDatabaseHandler.getSinkInfoDB(sinkID, sink);
    pDatabaseHandler.getSourceInfoDB(sourceID, source);
    std::vector<am_Route_s> listRoutes;

    ASSERT_EQ(getRoute(false, false, source, sink, listRoutes, 0), E_OK);
    ASSERT_EQ(static_cast<uint>(1), listRoutes.size());

    am_Route_s compareRoute1;
    compareRoute1.sinkID = sinkID;
    compareRoute1.sourceID = sourceID;
    compareRoute1.route.push_back(
    { sourceID, gwSinkID, domainID1, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gwSourceID, sinkID, domainID2, CF_GENIVI_ANALOG });
    ASSERT_TRUE(pCF.compareRoute(compareRoute1, listRoutes[0]));
}

TEST_F(CAmRouterMapTest,route3Domains1Source1Sink)
{
    EXPECT_CALL(pMockControlInterface,getConnectionFormatChoice(_,_,_,_,_)).WillRepeatedly(DoAll(returnConnectionFormat(), Return(E_OK)));

    am_SourceClass_s sourceclass;

    sourceclass.name = "sClass";
    sourceclass.sourceClassID = 5;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(sourceclass.sourceClassID, sourceclass));

    am_SinkClass_s sinkclass;
    sinkclass.sinkClassID = 5;
    sinkclass.name = "sname";

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkclass, sinkclass.sinkClassID));

    am_domainID_t domainID1, domainID2, domainID3;
    enterDomainDB("domain1", domainID1);
    enterDomainDB("domain2", domainID2);
    enterDomainDB("domain3", domainID3);

    std::vector<am_CustomConnectionFormat_t> cfStereo;
    cfStereo.push_back(CF_GENIVI_STEREO);
    std::vector<am_CustomConnectionFormat_t> cfAnalog;
    cfAnalog.push_back(CF_GENIVI_ANALOG);
    std::vector<am_CustomConnectionFormat_t> cfMono;
    cfMono.push_back(CF_GENIVI_MONO);

    am_sourceID_t sourceID;
    enterSourceDB("source1", domainID1, cfStereo, sourceID);

    am_sinkID_t gwSinkID1;
    enterSinkDB("gwSink1", domainID1, cfStereo, gwSinkID1);

    am_sourceID_t gwSourceID1;
    enterSourceDB("gwSource1", domainID2, cfMono, gwSourceID1);

    std::vector<bool> matrix;
    matrix.push_back(true);

    am_gatewayID_t gatewayID;
    enterGatewayDB("gateway", domainID2, domainID1, cfMono, cfStereo, matrix, gwSourceID1, gwSinkID1, gatewayID);

    am_sourceID_t gwSourceID2;
    enterSourceDB("gwSource2", domainID3, cfStereo, gwSourceID2);

    am_sinkID_t gwSinkID2;
    enterSinkDB("gwSink2", domainID2, cfMono, gwSinkID2);

    am_sinkID_t sinkID;
    enterSinkDB("sink1", domainID3, cfStereo, sinkID);

    am_gatewayID_t gatewayID1;
    enterGatewayDB("gateway", domainID3, domainID2, cfStereo, cfMono, matrix, gwSourceID2, gwSinkID2, gatewayID1);

    am::am_Source_s source;
    am::am_Sink_s sink;

    pDatabaseHandler.getSinkInfoDB(sinkID, sink);
    pDatabaseHandler.getSourceInfoDB(sourceID, source);

    std::vector<am_Route_s> listRoutes;

    ASSERT_EQ(getRoute(false, false, source, sink, listRoutes, 0), E_OK);
    ASSERT_EQ(static_cast<uint>(1), listRoutes.size());

    am_Route_s compareRoute1;
    compareRoute1.sinkID = sinkID;
    compareRoute1.sourceID = sourceID;
    compareRoute1.route.push_back(
    { sourceID, gwSinkID1, domainID1, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gwSourceID1, gwSinkID2, domainID2, CF_GENIVI_MONO });
    compareRoute1.route.push_back(
    { gwSourceID2, sinkID, domainID3, CF_GENIVI_STEREO });
    ASSERT_TRUE(pCF.compareRoute(compareRoute1, listRoutes[0]));
}

TEST_F(CAmRouterMapTest,routeSource1Sink2PathThroughConv1Gate1)
{
    EXPECT_CALL(pMockControlInterface,getConnectionFormatChoice(_,_,_,_,_)).WillRepeatedly(DoAll(returnConnectionFormat(), Return(E_OK)));

    am_SourceClass_s sourceclass;

    sourceclass.name = "sClass";
    sourceclass.sourceClassID = 5;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(sourceclass.sourceClassID, sourceclass));

    am_SinkClass_s sinkclass;
    sinkclass.sinkClassID = 5;
    sinkclass.name = "sname";

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkclass, sinkclass.sinkClassID));

    std::vector<bool> matrix;
    matrix.push_back(true);
    am_domainID_t domainID1, domainID2;
    enterDomainDB("domain1", domainID1);
    enterDomainDB("domain2", domainID2);

    std::vector<am_CustomConnectionFormat_t> cfStereo;
    cfStereo.push_back(CF_GENIVI_STEREO);
    std::vector<am_CustomConnectionFormat_t> cfAnalog;
    cfAnalog.push_back(CF_GENIVI_ANALOG);
    std::vector<am_CustomConnectionFormat_t> cfMono;
    cfMono.push_back(CF_GENIVI_MONO);
    std::vector<am_CustomConnectionFormat_t> cfAuto;
    cfAuto.push_back(CF_GENIVI_AUTO);

    am_sourceID_t sourceID;
    enterSourceDB("source1", domainID1, cfStereo, sourceID);

    am_sinkID_t gwSinkID1;
    enterSinkDB("gwSink1", domainID1, cfMono, gwSinkID1);

    am_sinkID_t coSinkID21;
    enterSinkDB("coSink21", domainID1, cfStereo, coSinkID21);

    am_sourceID_t coSourceID21;
    enterSourceDB("coSource21", domainID1, cfMono, coSourceID21);

    am_converterID_t converterID1;
    enterConverterDB("converter1", domainID1, cfMono, cfStereo, matrix, coSourceID21, coSinkID21, converterID1);

    am_sourceID_t gwSourceID1;
    enterSourceDB("gwSource21", domainID2, cfAuto, gwSourceID1);

    am_gatewayID_t gatewayID;
    enterGatewayDB("gateway1", domainID2, domainID1, cfAuto, cfMono, matrix, gwSourceID1, gwSinkID1, gatewayID);

    am_sinkID_t sinkID1;
    enterSinkDB("sink1", domainID2, cfAuto, sinkID1);

    am_sinkID_t sinkID2;
    enterSinkDB("sink2", domainID1, cfAuto, sinkID2);

    am::am_Source_s source;
    am::am_Sink_s sink1;
    pDatabaseHandler.getSinkInfoDB(sinkID1, sink1);
    am::am_Sink_s sink2;
    pDatabaseHandler.getSinkInfoDB(sinkID2, sink2);
    pDatabaseHandler.getSourceInfoDB(sourceID, source);

    std::vector<am_Route_s> listRoutes;

    ASSERT_EQ(getRoute(false, false, source, sink1, listRoutes, 0), E_OK);
    ASSERT_EQ(static_cast<uint>(1), listRoutes.size());

    am_Route_s compareRoute1;
    compareRoute1.sinkID = sinkID1;
    compareRoute1.sourceID = sourceID;
    compareRoute1.route.push_back(
    { sourceID, coSinkID21, domainID1, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { coSourceID21, gwSinkID1, domainID1, CF_GENIVI_MONO });
    compareRoute1.route.push_back(
    { gwSourceID1, sinkID1, domainID2, CF_GENIVI_AUTO });
    ASSERT_TRUE(pCF.compareRoute(compareRoute1, listRoutes[0]));

    listRoutes.clear();
    ASSERT_EQ(getRoute(false, false, source, sink2, listRoutes, 0), E_NOT_POSSIBLE);
    ASSERT_EQ(static_cast<uint>(0), listRoutes.size());
}

TEST_F(CAmRouterMapTest, routeSource1Sink1PathThroughDomain2)
{
    EXPECT_CALL(pMockControlInterface,getConnectionFormatChoice(_,_,_,_,_)).WillRepeatedly(DoAll(returnConnectionFormat(), Return(E_OK)));

    am_SourceClass_s sourceclass;

    sourceclass.name = "sClass";
    sourceclass.sourceClassID = 5;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(sourceclass.sourceClassID, sourceclass));

    am_SinkClass_s sinkclass;
    sinkclass.sinkClassID = 5;
    sinkclass.name = "sname";

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkclass, sinkclass.sinkClassID));

    std::vector<bool> matrix;
    matrix.push_back(true);
    am_domainID_t domainID1, domainID2;
    enterDomainDB("domain1", domainID1);
    enterDomainDB("domain2", domainID2);

    std::vector<am_CustomConnectionFormat_t> cfStereo;
    cfStereo.push_back(CF_GENIVI_STEREO);
    std::vector<am_CustomConnectionFormat_t> cfAnalog;
    cfAnalog.push_back(CF_GENIVI_ANALOG);
    std::vector<am_CustomConnectionFormat_t> cfMono;
    cfMono.push_back(CF_GENIVI_MONO);
    std::vector<am_CustomConnectionFormat_t> cfAuto;
    cfAuto.push_back(CF_GENIVI_AUTO);

    am_sourceID_t sourceID;
    enterSourceDB("source1", domainID1, cfStereo, sourceID);

    am_sinkID_t gwSinkID11;
    enterSinkDB("gwSink11", domainID1, cfStereo, gwSinkID11);
    am_sourceID_t gwSourceID11;
    enterSourceDB("gwSource11", domainID2, cfAnalog, gwSourceID11);
    am_converterID_t gatewayID1;
    enterGatewayDB("gateway1", domainID2, domainID1, cfAnalog, cfStereo, matrix, gwSourceID11, gwSinkID11, gatewayID1);

    am_sinkID_t gwSinkID21;
    enterSinkDB("gwSink21", domainID2, cfAnalog, gwSinkID21);
    am_sourceID_t gwSourceID12;
    enterSourceDB("gwSource12", domainID1, cfAuto, gwSourceID12);
    am_gatewayID_t gatewayID2;
    enterGatewayDB("gateway2", domainID1, domainID2, cfAuto, cfAnalog, matrix, gwSourceID12, gwSinkID21, gatewayID2);

    am_sinkID_t sink1ID;
    enterSinkDB("sink1", domainID1, cfAuto, sink1ID);
    am_sinkID_t sink2ID;
    enterSinkDB("sink2", domainID2, cfAnalog, sink2ID);

    std::vector<am_Route_s> listRoutes;

    am::am_Source_s source;
    am::am_Sink_s sink1;
    pDatabaseHandler.getSinkInfoDB(sink1ID, sink1);
    pDatabaseHandler.getSourceInfoDB(sourceID, source);

    ASSERT_EQ(getRoute(false, false, source, sink1, listRoutes, 0), E_NOT_POSSIBLE);
    ASSERT_EQ(static_cast<uint>(0), listRoutes.size());

    am::am_Sink_s sink2;
    pDatabaseHandler.getSinkInfoDB(sink2ID, sink2);

    ASSERT_EQ(getRoute(false, false, source, sink2, listRoutes, 0), E_OK);
    ASSERT_EQ(static_cast<uint>(1), listRoutes.size());

    am_Route_s compareRoute1;
    compareRoute1.sinkID = sink2ID;
    compareRoute1.sourceID = sourceID;
    compareRoute1.route.push_back(
    { sourceID, gwSinkID11, domainID1, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gwSourceID11, sink2ID, domainID2, CF_GENIVI_ANALOG });
    ASSERT_TRUE(pCF.compareRoute(compareRoute1, listRoutes[0]));
}

TEST_F(CAmRouterMapTest, routeSource1Sink1PathThroughGate1Conv2Gate2)
{
    EXPECT_CALL(pMockControlInterface,getConnectionFormatChoice(_,_,_,_,_)).WillRepeatedly(DoAll(returnConnectionFormat(), Return(E_OK)));

    am_SourceClass_s sourceclass;

    sourceclass.name = "sClass";
    sourceclass.sourceClassID = 5;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(sourceclass.sourceClassID, sourceclass));

    am_SinkClass_s sinkclass;
    sinkclass.sinkClassID = 5;
    sinkclass.name = "sname";

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkclass, sinkclass.sinkClassID));

    std::vector<bool> matrix;
    matrix.push_back(true);
    am_domainID_t domainID1, domainID2;
    enterDomainDB("domain1", domainID1);
    enterDomainDB("domain2", domainID2);

    std::vector<am_CustomConnectionFormat_t> cfStereo;
    cfStereo.push_back(CF_GENIVI_STEREO);
    std::vector<am_CustomConnectionFormat_t> cfAnalog;
    cfAnalog.push_back(CF_GENIVI_ANALOG);
    std::vector<am_CustomConnectionFormat_t> cfMono;
    cfMono.push_back(CF_GENIVI_MONO);
    std::vector<am_CustomConnectionFormat_t> cfAuto;
    cfAuto.push_back(CF_GENIVI_AUTO);

    am_sourceID_t sourceID;
    enterSourceDB("source1", domainID1, cfStereo, sourceID);

    am_sinkID_t gwSinkID11;
    enterSinkDB("gwSink11", domainID1, cfStereo, gwSinkID11);

    am_sourceID_t gwSourceID21;
    enterSourceDB("gwSource21", domainID2, cfAnalog, gwSourceID21);

    am_converterID_t gatewayID1;
    enterGatewayDB("gateway1", domainID2, domainID1, cfAnalog, cfStereo, matrix, gwSourceID21, gwSinkID11, gatewayID1);

    am_sinkID_t gwSinkID21;
    enterSinkDB("gwSink21", domainID2, cfStereo, gwSinkID21);

    am_sourceID_t gwSourceID12;
    enterSourceDB("gwSource12", domainID1, cfAuto, gwSourceID12);

    am_sinkID_t coSinkID21;
    enterSinkDB("coSink21", domainID2, cfAnalog, coSinkID21);

    am_sourceID_t coSourceID21;
    enterSourceDB("coSource21", domainID2, cfStereo, coSourceID21);

    am_converterID_t converterID2;
    enterConverterDB("converter2", domainID2, cfStereo, cfAnalog, matrix, coSourceID21, coSinkID21, converterID2);

    am_gatewayID_t gatewayID2;
    enterGatewayDB("gateway2", domainID1, domainID2, cfAuto, cfStereo, matrix, gwSourceID12, gwSinkID21, gatewayID2);

    am_sinkID_t sink1ID;
    enterSinkDB("sink1", domainID1, cfAuto, sink1ID);
    am_sinkID_t sink2ID;
    enterSinkDB("sink2", domainID2, cfStereo, sink2ID);

    am::am_Source_s source;
    am::am_Sink_s sink;

    pDatabaseHandler.getSinkInfoDB(sink1ID, sink);
    pDatabaseHandler.getSourceInfoDB(sourceID, source);

    std::vector<am_Route_s> listRoutes;

    ASSERT_EQ(getRoute(false, false, source, sink, listRoutes, 0), E_NOT_POSSIBLE);
    ASSERT_EQ(static_cast<uint>(0), listRoutes.size());

    am::am_Sink_s sink1;
    pDatabaseHandler.getSinkInfoDB(sink2ID, sink1);
    ASSERT_EQ(getRoute(false, false, source, sink1, listRoutes, 0), E_OK);

    ASSERT_EQ(static_cast<uint>(1), listRoutes.size());
    am_Route_s compareRoute1;
    compareRoute1.sinkID = sink2ID;
    compareRoute1.sourceID = sourceID;
    compareRoute1.route.push_back(
    { sourceID, gwSinkID11, domainID1, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gwSourceID21, coSinkID21, domainID2, CF_GENIVI_ANALOG });
    compareRoute1.route.push_back(
    { coSourceID21, sink2ID, domainID2, CF_GENIVI_STEREO });
    ASSERT_TRUE(pCF.compareRoute(compareRoute1, listRoutes[0]));
}

TEST_F(CAmRouterMapTest, routeSource1Sink1PathThroughConv1Gate1Conv2Gate2)
{
    EXPECT_CALL(pMockControlInterface,getConnectionFormatChoice(_,_,_,_,_)).WillRepeatedly(DoAll(returnConnectionFormat(), Return(E_OK)));

    am_SourceClass_s sourceclass;

    sourceclass.name = "sClass";
    sourceclass.sourceClassID = 5;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(sourceclass.sourceClassID, sourceclass));

    am_SinkClass_s sinkclass;
    sinkclass.sinkClassID = 5;
    sinkclass.name = "sname";

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkclass, sinkclass.sinkClassID));

    std::vector<bool> matrix;
    matrix.push_back(true);
    am_domainID_t domainID1, domainID2;
    enterDomainDB("domain1", domainID1);
    enterDomainDB("domain2", domainID2);

    std::vector<am_CustomConnectionFormat_t> cfStereo;
    cfStereo.push_back(CF_GENIVI_STEREO);
    std::vector<am_CustomConnectionFormat_t> cfAnalog;
    cfAnalog.push_back(CF_GENIVI_ANALOG);
    std::vector<am_CustomConnectionFormat_t> cfMono;
    cfMono.push_back(CF_GENIVI_MONO);
    std::vector<am_CustomConnectionFormat_t> cfAuto;
    cfAuto.push_back(CF_GENIVI_AUTO);
    std::vector<am_CustomConnectionFormat_t> cfFuture1;
    cfFuture1.push_back(5);
    std::vector<am_CustomConnectionFormat_t> cfFuture2;
    cfFuture2.push_back(6);

    am_sourceID_t sourceID;
    enterSourceDB("source1", domainID1, cfStereo, sourceID);

    am_sinkID_t coSinkID11;
    enterSinkDB("coSink11", domainID1, cfStereo, coSinkID11);
    am_sourceID_t coSourceID11;
    enterSourceDB("coSource11", domainID1, cfFuture1, coSourceID11);
    am_converterID_t converterID11;
    enterConverterDB("converter11", domainID1, cfFuture1, cfStereo, matrix, coSourceID11, coSinkID11, converterID11);

    am_sinkID_t coSinkID12;
    enterSinkDB("coSink12", domainID1, cfStereo, coSinkID12);
    am_sourceID_t coSourceID12;
    enterSourceDB("coSource12", domainID1, cfFuture2, coSourceID12);
    am_converterID_t converterID12;
    enterConverterDB("converter12", domainID1, cfFuture2, cfStereo, matrix, coSourceID12, coSinkID12, converterID12);

    am_sinkID_t coSinkID13;
    enterSinkDB("coSink13", domainID1, cfFuture2, coSinkID13);
    am_sourceID_t coSourceID13;
    enterSourceDB("coSource13", domainID1, cfFuture1, coSourceID13);
    am_converterID_t converterID13;
    enterConverterDB("converter13", domainID1, cfFuture1, cfFuture2, matrix, coSourceID13, coSinkID13, converterID13);

    am_sinkID_t gwSinkID11;
    enterSinkDB("gwSink11", domainID1, cfFuture1, gwSinkID11);
    am_sourceID_t gwSourceID21;
    enterSourceDB("gwSource21", domainID2, cfAnalog, gwSourceID21);
    am_converterID_t gatewayID1;
    enterGatewayDB("gateway1", domainID2, domainID1, cfAnalog, cfFuture1, matrix, gwSourceID21, gwSinkID11, gatewayID1);

    am_sinkID_t gwSinkID21;
    enterSinkDB("gwSink21", domainID2, cfStereo, gwSinkID21);

    am_sourceID_t gwSourceID12;
    enterSourceDB("gwSource12", domainID1, cfAuto, gwSourceID12);

    am_sinkID_t coSinkID21;
    enterSinkDB("coSink21", domainID2, cfAnalog, coSinkID21);

    am_sourceID_t coSourceID21;
    enterSourceDB("coSource21", domainID2, cfStereo, coSourceID21);

    am_converterID_t converterID2;
    enterConverterDB("converter2", domainID2, cfStereo, cfAnalog, matrix, coSourceID21, coSinkID21, converterID2);

    am_gatewayID_t gatewayID2;
    enterGatewayDB("gateway2", domainID1, domainID2, cfAuto, cfStereo, matrix, gwSourceID12, gwSinkID21, gatewayID2);

    am_sinkID_t sinkID;
    enterSinkDB("sink1", domainID1, cfAuto, sinkID);

    am::am_Source_s source;
    am::am_Sink_s sink;
    pDatabaseHandler.getSinkInfoDB(sinkID, sink);
    pDatabaseHandler.getSourceInfoDB(sourceID, source);

    std::vector<am_Route_s> listRoutes;

    ASSERT_EQ(getRoute(false, false, source, sink, listRoutes, 0), E_NOT_POSSIBLE);
    ASSERT_EQ(static_cast<uint>(0), listRoutes.size());

    am::am_Sink_s sink2;
    pDatabaseHandler.getSinkInfoDB(coSinkID21, sink2);
    ASSERT_EQ(getRoute(false, false, source, sink2, listRoutes, 0), E_OK);
    ASSERT_EQ(static_cast<uint>(2), listRoutes.size());

    am_Route_s compareRoute1;
    compareRoute1.sinkID = coSinkID21;
    compareRoute1.sourceID = sourceID;
    compareRoute1.route.push_back(
    { sourceID, coSinkID11, domainID1, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { coSourceID11, gwSinkID11, domainID1, 5 });
    compareRoute1.route.push_back(
    { gwSourceID21, coSinkID21, domainID2, CF_GENIVI_ANALOG });

    am_Route_s compareRoute2;
    compareRoute2.sinkID = coSinkID21;
    compareRoute2.sourceID = sourceID;
    compareRoute2.route.push_back(
    { sourceID, coSinkID12, domainID1, CF_GENIVI_STEREO });
    compareRoute2.route.push_back(
    { coSourceID12, coSinkID13, domainID1, 6 });
    compareRoute2.route.push_back(
    { coSourceID13, gwSinkID11, domainID1, 5 });
    compareRoute2.route.push_back(
    { gwSourceID21, coSinkID21, domainID2, CF_GENIVI_ANALOG });

    ASSERT_TRUE(pCF.compareRoute(compareRoute1, listRoutes[1]) || pCF.compareRoute(compareRoute1, listRoutes[0]));
    ASSERT_TRUE(pCF.compareRoute(compareRoute2, listRoutes[0]) || pCF.compareRoute(compareRoute2, listRoutes[1]));
}

TEST_F(CAmRouterMapTest,route3Domains1Source1SinkGwCycles)
{
    EXPECT_CALL(pMockControlInterface,getConnectionFormatChoice(_,_,_,_,_)).WillRepeatedly(DoAll(returnConnectionFormat(), Return(E_OK)));

    am_SourceClass_s sourceclass;

    sourceclass.name = "sClass";
    sourceclass.sourceClassID = 5;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(sourceclass.sourceClassID, sourceclass));

    am_SinkClass_s sinkclass;
    sinkclass.sinkClassID = 5;
    sinkclass.name = "sname";

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkclass, sinkclass.sinkClassID));

    am_domainID_t domain1ID, domain2ID, domain3ID;
    enterDomainDB("domain1", domain1ID);
    enterDomainDB("domain2", domain2ID);
    enterDomainDB("domain3", domain3ID);

    //just make so many cycles as possible
    std::vector<am_CustomConnectionFormat_t> cfStereo;
    cfStereo.push_back(CF_GENIVI_STEREO);
    std::vector<am_CustomConnectionFormat_t> cfAnalog = cfStereo;
    std::vector<am_CustomConnectionFormat_t> cfMono = cfStereo;
    std::vector<am_CustomConnectionFormat_t> cfAuto;
    cfAuto.push_back(CF_GENIVI_AUTO);

    am_sourceID_t source1ID;
    enterSourceDB("source1", domain1ID, cfStereo, source1ID);
    am_sinkID_t gw1SinkID;
    enterSinkDB("gw1Sink", domain1ID, cfStereo, gw1SinkID);
    am_sinkID_t gw2SinkID;
    enterSinkDB("gw2Sink", domain1ID, cfStereo, gw2SinkID);
    am_sourceID_t gw3SourceID;
    enterSourceDB("gw3Source", domain1ID, cfAnalog, gw3SourceID);
    am_sourceID_t gw4SourceID;
    enterSourceDB("gw4Source", domain1ID, cfAnalog, gw4SourceID);
    am_sinkID_t gw5SinkID;
    enterSinkDB("gw5Sink", domain1ID, cfAnalog, gw5SinkID);

    am_sourceID_t gw1SourceID;
    enterSourceDB("gw1Source", domain2ID, cfMono, gw1SourceID);
    am_sourceID_t gw2SourceID;
    enterSourceDB("gw2Source", domain2ID, cfMono, gw2SourceID);
    am_sinkID_t gw3SinkID;
    enterSinkDB("gw3Sink", domain2ID, cfMono, gw3SinkID);
    am_sinkID_t gw4SinkID;
    enterSinkDB("gw4Sink", domain2ID, cfMono, gw4SinkID);

    am_sourceID_t gw5SourceID;
    enterSourceDB("gw5Source", domain3ID, cfStereo, gw5SourceID);
    am_sinkID_t sink1ID;
    enterSinkDB("sink1", domain3ID, cfStereo, sink1ID);

    std::vector<bool> matrixT;
    matrixT.push_back(true);
    std::vector<bool> matrixF;
    matrixF.push_back(false);

    am_gatewayID_t gateway1ID;
    enterGatewayDB("gateway1", domain2ID, domain1ID, cfMono, cfStereo, matrixT, gw1SourceID, gw1SinkID, gateway1ID);
    am_gatewayID_t gateway2ID;
    enterGatewayDB("gateway2", domain2ID, domain1ID, cfMono, cfStereo, matrixT, gw2SourceID, gw2SinkID, gateway2ID);
    am_gatewayID_t gateway3ID;
    enterGatewayDB("gateway3", domain1ID, domain2ID, cfAnalog, cfMono, matrixT, gw3SourceID, gw3SinkID, gateway3ID);
    am_gatewayID_t gateway4ID;
    enterGatewayDB("gateway4", domain1ID, domain2ID, cfAnalog, cfMono, matrixT, gw4SourceID, gw4SinkID, gateway4ID);
    am_gatewayID_t gateway5ID;
    enterGatewayDB("gateway5", domain3ID, domain1ID, cfStereo, cfAnalog, matrixT, gw5SourceID, gw5SinkID, gateway5ID);

    std::vector<am_Route_s> listRoutes;

    am_Route_s compareRoute1;
    compareRoute1.sinkID = sink1ID;
    compareRoute1.sourceID = source1ID;

#define DO_ASSERT()	 \
		{\
	bool didMatch = false; \
	for(auto it = listRoutes.begin(); it!=listRoutes.end(); it++) \
	didMatch|=pCF.compareRoute(compareRoute1,*it); \
	ASSERT_TRUE(didMatch); \
		}

    ASSERT_EQ(getAllPaths(false, source1ID, sink1ID, listRoutes, UINT_MAX, 10), E_OK);
    ASSERT_EQ(static_cast<uint>(9), listRoutes.size());

    compareRoute1.route.clear();
    compareRoute1.route.push_back(
    { source1ID, gw2SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw2SourceID, gw4SinkID, domain2ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw4SourceID, gw5SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw5SourceID, sink1ID, domain3ID, CF_GENIVI_STEREO });
    DO_ASSERT()

    compareRoute1.route.clear();
    compareRoute1.route.push_back(
    { source1ID, gw1SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw1SourceID, gw3SinkID, domain2ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw3SourceID, gw5SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw5SourceID, sink1ID, domain3ID, CF_GENIVI_STEREO });
    DO_ASSERT()

    compareRoute1.route.clear();
    compareRoute1.route.push_back(
    { source1ID, gw2SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw2SourceID, gw3SinkID, domain2ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw3SourceID, gw5SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw5SourceID, sink1ID, domain3ID, CF_GENIVI_STEREO });
    DO_ASSERT()

    compareRoute1.route.clear();
    compareRoute1.route.push_back(
    { source1ID, gw1SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw1SourceID, gw4SinkID, domain2ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw4SourceID, gw5SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw5SourceID, sink1ID, domain3ID, CF_GENIVI_STEREO });
    DO_ASSERT()

    compareRoute1.route.clear();
    compareRoute1.route.push_back(
    { source1ID, gw2SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw2SourceID, gw4SinkID, domain2ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw4SourceID, gw1SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw1SourceID, gw3SinkID, domain2ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw3SourceID, gw5SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw5SourceID, sink1ID, domain3ID, CF_GENIVI_STEREO });
    DO_ASSERT()

    compareRoute1.route.clear();
    compareRoute1.route.push_back(
    { source1ID, gw2SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw2SourceID, gw3SinkID, domain2ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw3SourceID, gw1SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw1SourceID, gw4SinkID, domain2ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw4SourceID, gw5SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw5SourceID, sink1ID, domain3ID, CF_GENIVI_STEREO });
    DO_ASSERT()

    compareRoute1.route.clear();
    compareRoute1.route.push_back(
    { source1ID, gw1SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw1SourceID, gw3SinkID, domain2ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw3SourceID, gw2SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw2SourceID, gw4SinkID, domain2ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw4SourceID, gw5SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw5SourceID, sink1ID, domain3ID, CF_GENIVI_STEREO });
    DO_ASSERT()

    compareRoute1.route.clear();
    compareRoute1.route.push_back(
    { source1ID, gw1SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw1SourceID, gw4SinkID, domain2ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw4SourceID, gw2SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw2SourceID, gw3SinkID, domain2ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw3SourceID, gw5SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw5SourceID, sink1ID, domain3ID, CF_GENIVI_STEREO });
    DO_ASSERT()

    compareRoute1.route.clear();
    compareRoute1.route.push_back(
    { source1ID, gw5SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw5SourceID, sink1ID, domain3ID, CF_GENIVI_STEREO });
    DO_ASSERT()

    listRoutes.clear();

    ASSERT_EQ(getAllPaths(false, source1ID, sink1ID, listRoutes, 1, 10), E_OK);
    ASSERT_EQ(static_cast<uint>(5), listRoutes.size());

    compareRoute1.route.clear();
    compareRoute1.route.push_back(
    { source1ID, gw2SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw2SourceID, gw4SinkID, domain2ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw4SourceID, gw5SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw5SourceID, sink1ID, domain3ID, CF_GENIVI_STEREO });
    DO_ASSERT()

    compareRoute1.route.clear();
    compareRoute1.route.push_back(
    { source1ID, gw1SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw1SourceID, gw3SinkID, domain2ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw3SourceID, gw5SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw5SourceID, sink1ID, domain3ID, CF_GENIVI_STEREO });
    DO_ASSERT()

    compareRoute1.route.clear();
    compareRoute1.route.push_back(
    { source1ID, gw2SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw2SourceID, gw3SinkID, domain2ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw3SourceID, gw5SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw5SourceID, sink1ID, domain3ID, CF_GENIVI_STEREO });
    DO_ASSERT()

    compareRoute1.route.clear();
    compareRoute1.route.push_back(
    { source1ID, gw1SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw1SourceID, gw4SinkID, domain2ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw4SourceID, gw5SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw5SourceID, sink1ID, domain3ID, CF_GENIVI_STEREO });
    DO_ASSERT()

    compareRoute1.route.clear();
    compareRoute1.route.push_back(
    { source1ID, gw5SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw5SourceID, sink1ID, domain3ID, CF_GENIVI_STEREO });
    DO_ASSERT()

    listRoutes.clear();

    ASSERT_EQ(getAllPaths(false, source1ID, sink1ID, listRoutes), E_OK);
    ASSERT_EQ(static_cast<uint>(1), listRoutes.size());
    DO_ASSERT()
}

TEST_F(CAmRouterMapTest,route3Domains1Source1SinkGwCycles2)
{
    EXPECT_CALL(pMockControlInterface,getConnectionFormatChoice(_,_,_,_,_)).WillRepeatedly(DoAll(returnConnectionFormat(), Return(E_OK)));

    am_SourceClass_s sourceclass;

    sourceclass.name = "sClass";
    sourceclass.sourceClassID = 5;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(sourceclass.sourceClassID, sourceclass));

    am_SinkClass_s sinkclass;
    sinkclass.sinkClassID = 5;
    sinkclass.name = "sname";

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkclass, sinkclass.sinkClassID));

    am_domainID_t domain1ID, domain2ID, domain3ID;
    enterDomainDB("domain1", domain1ID);
    enterDomainDB("domain2", domain2ID);
    enterDomainDB("domain3", domain3ID);

    //just make so many cycles as possible
    std::vector<am_CustomConnectionFormat_t> cfStereo;
    cfStereo.push_back(CF_GENIVI_STEREO);
    std::vector<am_CustomConnectionFormat_t> cfOther;
    cfOther.push_back(CF_GENIVI_AUTO);
    std::vector<am_CustomConnectionFormat_t> cfMono;
    cfMono.push_back(CF_GENIVI_MONO);

    am_sourceID_t source1ID;
    enterSourceDB("source1", domain1ID, cfMono, source1ID);
    am_sinkID_t gw1SinkID;
    enterSinkDB("gw1Sink", domain1ID, cfStereo, gw1SinkID);
    am_sinkID_t gw2SinkID;
    enterSinkDB("gw2Sink", domain1ID, cfMono, gw2SinkID);
    am_sourceID_t gw3SourceID;
    enterSourceDB("gw3Source", domain1ID, cfStereo, gw3SourceID);
    am_sourceID_t gw4SourceID;
    enterSourceDB("gw4Source", domain1ID, cfStereo, gw4SourceID);
    am_sinkID_t gw5SinkID;
    enterSinkDB("gw5Sink", domain1ID, cfStereo, gw5SinkID);

    am_sourceID_t gw1SourceID;
    enterSourceDB("gw1Source", domain2ID, cfStereo, gw1SourceID);
    am_sourceID_t gw2SourceID;
    enterSourceDB("gw2Source", domain2ID, cfStereo, gw2SourceID);
    am_sinkID_t gw3SinkID;
    enterSinkDB("gw3Sink", domain2ID, cfStereo, gw3SinkID);
    am_sinkID_t gw4SinkID;
    enterSinkDB("gw4Sink", domain2ID, cfStereo, gw4SinkID);

    am_sourceID_t gw5SourceID;
    enterSourceDB("gw5Source", domain3ID, cfOther, gw5SourceID);
    am_sinkID_t sink1ID;
    enterSinkDB("sink1", domain3ID, cfOther, sink1ID);

    std::vector<bool> matrixT;
    matrixT.push_back(true);
    std::vector<bool> matrixF;
    matrixF.push_back(false);

    am_gatewayID_t gateway1ID;
    enterGatewayDB("gateway1", domain2ID, domain1ID, cfStereo, cfStereo, matrixT, gw1SourceID, gw1SinkID, gateway1ID);
    am_gatewayID_t gateway2ID;
    enterGatewayDB("gateway2", domain2ID, domain1ID, cfStereo, cfMono, matrixT, gw2SourceID, gw2SinkID, gateway2ID);
    am_gatewayID_t gateway3ID;
    enterGatewayDB("gateway3", domain1ID, domain2ID, cfStereo, cfStereo, matrixT, gw3SourceID, gw3SinkID, gateway3ID);
    am_gatewayID_t gateway4ID;
    enterGatewayDB("gateway4", domain1ID, domain2ID, cfStereo, cfStereo, matrixT, gw4SourceID, gw4SinkID, gateway4ID);
    am_gatewayID_t gateway5ID;
    enterGatewayDB("gateway5", domain3ID, domain1ID, cfOther, cfStereo, matrixT, gw5SourceID, gw5SinkID, gateway5ID);

    std::vector<am_Route_s> listRoutes;

    am_Route_s compareRoute1;
    compareRoute1.sinkID = sink1ID;
    compareRoute1.sourceID = source1ID;

#define DO_ASSERT()	 \
		{\
	bool didMatch = false; \
	for(auto it = listRoutes.begin(); it!=listRoutes.end(); it++) \
	didMatch|=pCF.compareRoute(compareRoute1,*it); \
	ASSERT_TRUE(didMatch); \
		}

    ASSERT_EQ(getRoute(false, false, source1ID, sink1ID, listRoutes, 0, 10), E_NOT_POSSIBLE);
    ASSERT_EQ(static_cast<uint>(0), listRoutes.size());

    ASSERT_EQ(getRoute(false, false, source1ID, sink1ID, listRoutes, 1, 10), E_OK);
    ASSERT_EQ(static_cast<uint>(2), listRoutes.size());

    compareRoute1.route.clear();
    compareRoute1.route.push_back(
    { source1ID, gw2SinkID, domain1ID, CF_GENIVI_MONO });
    compareRoute1.route.push_back(
    { gw2SourceID, gw4SinkID, domain2ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw4SourceID, gw5SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw5SourceID, sink1ID, domain3ID, CF_GENIVI_AUTO });
    DO_ASSERT()

    compareRoute1.route.clear();
    compareRoute1.route.push_back(
    { source1ID, gw2SinkID, domain1ID, CF_GENIVI_MONO });
    compareRoute1.route.push_back(
    { gw2SourceID, gw3SinkID, domain2ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw3SourceID, gw5SinkID, domain1ID, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gw5SourceID, sink1ID, domain3ID, CF_GENIVI_AUTO });
    DO_ASSERT()
}

TEST_F(CAmRouterMapTest,route3Domains1Source3Gateways3Convertres1Sink)
{
    EXPECT_CALL(pMockControlInterface,getConnectionFormatChoice(_,_,_,_,_)).WillRepeatedly(DoAll(returnConnectionFormat(), Return(E_OK)));

    am_SourceClass_s sourceclass;

    sourceclass.name = "sClass";
    sourceclass.sourceClassID = 5;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(sourceclass.sourceClassID, sourceclass));

    am_SinkClass_s sinkclass;
    sinkclass.sinkClassID = 5;
    sinkclass.name = "sname";

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkclass, sinkclass.sinkClassID));

    am_domainID_t domainID1, domainID2, domainID3;
    enterDomainDB("domain1", domainID1);
    enterDomainDB("domain2", domainID2);
    enterDomainDB("domain3", domainID3);

    std::vector<am_CustomConnectionFormat_t> cfStereo;
    cfStereo.push_back(CF_GENIVI_STEREO);
    std::vector<am_CustomConnectionFormat_t> cfAnalog;
    cfAnalog.push_back(CF_GENIVI_ANALOG);
    std::vector<am_CustomConnectionFormat_t> cfMono;
    cfMono.push_back(CF_GENIVI_MONO);
    std::vector<am_CustomConnectionFormat_t> cfAuto;
    cfAuto.push_back(CF_GENIVI_AUTO);

    am_sourceID_t sourceID;
    enterSourceDB("source1", domainID1, cfStereo, sourceID);
    am_sinkID_t gwSinkID1;
    enterSinkDB("gwSink1", domainID1, cfStereo, gwSinkID1);
    am_sinkID_t gwSinkID21;
    enterSinkDB("gwSink21", domainID1, cfStereo, gwSinkID21);

    am_sourceID_t gwSourceID1;
    enterSourceDB("gwSource1", domainID2, cfMono, gwSourceID1);
    am_sinkID_t gwSinkID22;
    enterSinkDB("gwSink22", domainID2, cfMono, gwSinkID22);

    am_sourceID_t gwSourceID21;
    enterSourceDB("gwSource21", domainID3, cfAuto, gwSourceID21);

    am_sourceID_t gwSourceID22;
    enterSourceDB("gwSource22", domainID3, cfAuto, gwSourceID22);
    am_sourceID_t cSourceID5;
    enterSourceDB("cSource5", domainID3, cfStereo, cSourceID5);
    am_sinkID_t cSinkID5;
    enterSinkDB("cSink5", domainID3, cfAnalog, cSinkID5);
    am_sourceID_t cSourceID3;
    enterSourceDB("cSource3", domainID3, cfAnalog, cSourceID3);
    am_sinkID_t cSinkID3;
    enterSinkDB("cSinkID3", domainID3, cfAuto, cSinkID3);
    am_sourceID_t cSourceID4;
    enterSourceDB("cSource4", domainID3, cfStereo, cSourceID4);
    am_sinkID_t cSinkID4;
    enterSinkDB("cSink4", domainID3, cfAnalog, cSinkID4);
    am_sinkID_t sinkID;
    enterSinkDB("sink1", domainID3, cfStereo, sinkID);

    std::vector<bool> matrix;
    matrix.push_back(true);
    am_gatewayID_t gatewayID;
    enterGatewayDB("gateway1", domainID2, domainID1, cfMono, cfStereo, matrix, gwSourceID1, gwSinkID1, gatewayID);
    am_gatewayID_t gatewayID22;
    enterGatewayDB("gateway22", domainID3, domainID2, cfAuto, cfMono, matrix, gwSourceID22, gwSinkID22, gatewayID22);
    am_gatewayID_t gatewayID21;
    enterGatewayDB("gateway21", domainID3, domainID1, cfAuto, cfStereo, matrix, gwSourceID21, gwSinkID21, gatewayID21);
    am_converterID_t converterID1;
    enterConverterDB("converter1", domainID3, cfAnalog, cfAuto, matrix, cSourceID3, cSinkID3, converterID1);
    am_converterID_t converterID2;
    enterConverterDB("converter2", domainID3, cfStereo, cfAnalog, matrix, cSourceID4, cSinkID4, converterID2);
    am_converterID_t converterID3;
    enterConverterDB("converter3", domainID3, cfStereo, cfAnalog, matrix, cSourceID5, cSinkID5, converterID3);

    am::am_Source_s source;
    am::am_Sink_s sink;

    pDatabaseHandler.getSinkInfoDB(sinkID, sink);
    pDatabaseHandler.getSourceInfoDB(sourceID, source);

    std::vector<am_Route_s> listRoutes;

    ASSERT_EQ(getRoute(false, false, source, sink, listRoutes), E_OK);
    ASSERT_EQ(static_cast<uint>(4), listRoutes.size());

    am_Route_s compareRoute1;
    compareRoute1.sinkID = sinkID;
    compareRoute1.sourceID = sourceID;
    compareRoute1.route.push_back(
    { sourceID, gwSinkID1, domainID1, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gwSourceID1, gwSinkID22, domainID2, CF_GENIVI_MONO });
    compareRoute1.route.push_back(
    { gwSourceID22, cSinkID3, domainID3, CF_GENIVI_AUTO });
    compareRoute1.route.push_back(
    { cSourceID3, cSinkID4, domainID3, CF_GENIVI_ANALOG });
    compareRoute1.route.push_back(
    { cSourceID4, sinkID, domainID3, CF_GENIVI_STEREO });

    am_Route_s compareRoute2;
    compareRoute2.sinkID = sinkID;
    compareRoute2.sourceID = sourceID;
    compareRoute2.route.push_back(
    { sourceID, gwSinkID1, domainID1, CF_GENIVI_STEREO });
    compareRoute2.route.push_back(
    { gwSourceID1, gwSinkID22, domainID2, CF_GENIVI_MONO });
    compareRoute2.route.push_back(
    { gwSourceID22, cSinkID3, domainID3, CF_GENIVI_AUTO });
    compareRoute2.route.push_back(
    { cSourceID3, cSinkID5, domainID3, CF_GENIVI_ANALOG });
    compareRoute2.route.push_back(
    { cSourceID5, sinkID, domainID3, CF_GENIVI_STEREO });

    am_Route_s compareRoute3;
    compareRoute3.sinkID = sinkID;
    compareRoute3.sourceID = sourceID;
    compareRoute3.route.push_back(
    { sourceID, gwSinkID21, domainID1, CF_GENIVI_STEREO });
    compareRoute3.route.push_back(
    { gwSourceID21, cSinkID3, domainID3, CF_GENIVI_AUTO });
    compareRoute3.route.push_back(
    { cSourceID3, cSinkID4, domainID3, CF_GENIVI_ANALOG });
    compareRoute3.route.push_back(
    { cSourceID4, sinkID, domainID3, CF_GENIVI_STEREO });

    am_Route_s compareRoute4;
    compareRoute4.sinkID = sinkID;
    compareRoute4.sourceID = sourceID;
    compareRoute4.route.push_back(
    { sourceID, gwSinkID21, domainID1, CF_GENIVI_STEREO });
    compareRoute4.route.push_back(
    { gwSourceID21, cSinkID3, domainID3, CF_GENIVI_AUTO });
    compareRoute4.route.push_back(
    { cSourceID3, cSinkID5, domainID3, CF_GENIVI_ANALOG });
    compareRoute4.route.push_back(
    { cSourceID5, sinkID, domainID3, CF_GENIVI_STEREO });

    ASSERT_TRUE(
            pCF.compareRoute(compareRoute1, listRoutes[0]) || pCF.compareRoute(compareRoute1, listRoutes[1]) || pCF.compareRoute(compareRoute1, listRoutes[2])
                    || pCF.compareRoute(compareRoute1, listRoutes[3]));

    ASSERT_TRUE(
            pCF.compareRoute(compareRoute2, listRoutes[0]) || pCF.compareRoute(compareRoute2, listRoutes[1]) || pCF.compareRoute(compareRoute2, listRoutes[2])
                    || pCF.compareRoute(compareRoute2, listRoutes[3]));

    ASSERT_TRUE(
            pCF.compareRoute(compareRoute3, listRoutes[0]) || pCF.compareRoute(compareRoute3, listRoutes[1]) || pCF.compareRoute(compareRoute3, listRoutes[2])
                    || pCF.compareRoute(compareRoute3, listRoutes[3]));

    ASSERT_TRUE(
            pCF.compareRoute(compareRoute4, listRoutes[0]) || pCF.compareRoute(compareRoute4, listRoutes[1]) || pCF.compareRoute(compareRoute4, listRoutes[2])
                    || pCF.compareRoute(compareRoute4, listRoutes[3]));
}

TEST_F(CAmRouterMapTest, routeTunerHeadphonePathThroughGWPlus2OtherSinks)
{
    EXPECT_CALL(pMockControlInterface,getConnectionFormatChoice(_,_,_,_,_)).WillRepeatedly(DoAll(returnConnectionFormat(), Return(E_OK)));

    am_SourceClass_s sourceclass;

    sourceclass.name = "sClass";
    sourceclass.sourceClassID = 5;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(sourceclass.sourceClassID, sourceclass));

    am_SinkClass_s sinkclass;
    sinkclass.sinkClassID = 5;
    sinkclass.name = "sname";

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkclass, sinkclass.sinkClassID));

    std::vector<bool> matrix;
    matrix.push_back(true);
    matrix.push_back(false);
    matrix.push_back(false);
    matrix.push_back(true);

    am_domainID_t domainID1, domainID2;
    enterDomainDB("domain1", domainID1);
    enterDomainDB("domain2", domainID2);

    std::vector<am_CustomConnectionFormat_t> cfStereo;
    cfStereo.push_back(CF_GENIVI_STEREO);

    std::vector<am_CustomConnectionFormat_t> cfMulti;
    cfMulti.push_back(CF_GENIVI_STEREO);
    cfMulti.push_back(CF_GENIVI_ANALOG);

    am_sourceID_t tunerID;
    enterSourceDB("Tuner", domainID1, cfStereo, tunerID);

    am_sinkID_t gwSinkID1;
    enterSinkDB("gwSink1", domainID1, cfStereo, gwSinkID1);
    am_sourceID_t gwSourceID1;
    enterSourceDB("gwSource1", domainID2, cfMulti, gwSourceID1);
    am_converterID_t gatewayID1;
    enterGatewayDB("gateway1", domainID2, domainID1, cfMulti, cfMulti, matrix, gwSourceID1, gwSinkID1, gatewayID1);

    am_sinkID_t rseLeftID;
    enterSinkDB("RSE Left", domainID2, cfMulti, rseLeftID);
    am_sinkID_t rseRightID;
    enterSinkDB("RSE Right", domainID2, cfMulti, rseRightID);
    am_sinkID_t rseHeadphoneID;
    enterSinkDB("Headphone", domainID2, cfMulti, rseHeadphoneID);

    am::am_Source_s source;
    am::am_Sink_s sink;
    pDatabaseHandler.getSinkInfoDB(rseLeftID, sink);
    pDatabaseHandler.getSourceInfoDB(tunerID, source);

    std::vector<am_Route_s> listRoutes;
    ASSERT_EQ(getRoute(false, false, source, sink, listRoutes), E_OK);
    ASSERT_EQ(listRoutes.size(), static_cast<uint>(1));

    am_Route_s compareRoute1;
    compareRoute1.sinkID = rseLeftID;
    compareRoute1.sourceID = tunerID;
    compareRoute1.route.push_back(
    { tunerID, gwSinkID1, domainID1, CF_GENIVI_STEREO });
    compareRoute1.route.push_back(
    { gwSourceID1, rseLeftID, domainID2, CF_GENIVI_STEREO });

    ASSERT_TRUE(pCF.compareRoute(compareRoute1, listRoutes[0]));

    listRoutes.clear();

    am::am_Source_s gwSource;
    am::am_Sink_s sink2;
    pDatabaseHandler.getSinkInfoDB(rseHeadphoneID, sink2);
    pDatabaseHandler.getSourceInfoDB(gwSourceID1, gwSource);
    ASSERT_EQ(getRoute(false, false, gwSource, sink2, listRoutes), E_OK);
    ASSERT_GT(listRoutes.size(), static_cast<uint>(0));

    am_Route_s compareRoute2;
    compareRoute2.sinkID = rseHeadphoneID;
    compareRoute2.sourceID = gwSourceID1;
    compareRoute2.route.push_back(
    { gwSourceID1, rseHeadphoneID, domainID2, CF_GENIVI_STEREO });
    am_Route_s compareRoute3;
    compareRoute3.sinkID = rseHeadphoneID;
    compareRoute3.sourceID = gwSourceID1;
    compareRoute3.route.push_back(
    { gwSourceID1, rseHeadphoneID, domainID2, CF_GENIVI_ANALOG });

    ASSERT_TRUE(pCF.compareRoute(compareRoute2, listRoutes[0]) || pCF.compareRoute(compareRoute2, listRoutes[1]));
    ASSERT_TRUE(pCF.compareRoute(compareRoute3, listRoutes[0]) || pCF.compareRoute(compareRoute3, listRoutes[1]));
}

int main(int argc, char **argv)
{
    try
    {
        TCLAP::CmdLine* cmd(CAmCommandLineSingleton::instanciateOnce("The team of the AudioManager wishes you a nice day!", ' ', DAEMONVERSION, true));
        cmd->add(enableDebug);
    } catch (TCLAP::ArgException &e)  // catch any exceptions
    {
        std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    }
    CAmCommandLineSingleton::instance()->preparse(argc, argv);
    CAmDltWrapper::instanctiateOnce("rTEST", "RouterMap Test", enableDebug.getValue(), CAmDltWrapper::logDestination::DAEMON);
    logInfo("Routing Test started ");
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

