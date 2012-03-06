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

#include "CAmRouterTest.h"
#include <string.h>
#include "shared/CAmDltWrapper.h"

using namespace am;
using namespace testing;

CAmRouterTest::CAmRouterTest() :
        plistRoutingPluginDirs(), //
        plistCommandPluginDirs(), //
        pSocketHandler(), //
        pDatabaseHandler(std::string(":memory:")), //
        pControlSender(std::string("")), //
        pRouter(&pDatabaseHandler, &pControlSender), //
        pRoutingSender(plistRoutingPluginDirs), //
        pCommandSender(plistCommandPluginDirs), //
        pMockInterface(), //
        pMockControlInterface(), //
        pRoutingInterfaceBackdoor(), //
        pCommandInterfaceBackdoor(), //
        pControlInterfaceBackdoor(), //
        pControlReceiver(&pDatabaseHandler, &pRoutingSender, &pCommandSender,&pSocketHandler, &pRouter), //
        pObserver(&pCommandSender, &pRoutingSender, &pSocketHandler)
{
    pDatabaseHandler.registerObserver(&pObserver);
    pCommandInterfaceBackdoor.injectInterface(&pCommandSender, &pMockInterface);
    pControlInterfaceBackdoor.replaceController(&pControlSender, &pMockControlInterface);
}

CAmRouterTest::~CAmRouterTest()
{
}

void CAmRouterTest::SetUp()
{
    logInfo("Routing Test started ");
}

void CAmRouterTest::TearDown()
{
}

ACTION(returnConnectionFormat){
arg4=arg3;
}

//test that checks just sinks and source in a domain but connectionformats do not match
TEST_F(CAmRouterTest,simpleRoute2withDomainNoMatchFormats)
{
    EXPECT_CALL(pMockControlInterface,getConnectionFormatChoice(_,_,_,_,_)).WillRepeatedly(DoAll(returnConnectionFormat(), Return(E_OK)));

    //initialize 2 domains
    am_Domain_s domain1;
    am_domainID_t domainID1;

    domain1.domainID = 0;
    domain1.name = "domain1";
    domain1.busname = "domain1bus";
    domain1.state = DS_CONTROLLED;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain1,domainID1));

    am_Source_s source;
    am_sourceID_t sourceID;

    source.domainID = domainID1;
    source.name = "source1";
    source.sourceState = SS_ON;
    source.sourceID = 0;
    source.sourceClassID = 5;
    source.listConnectionFormats.push_back(CF_GENIVI_MONO);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));

    am_Sink_s sink;
    am_sinkID_t sinkID;

    sink.domainID = domainID1;
    sink.name = "sink1";
    sink.sinkID = 0;
    sink.sinkClassID = 5;
    sink.muteState = MS_MUTED;
    sink.listConnectionFormats.push_back(CF_GENIVI_ANALOG);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));

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

    ASSERT_EQ(E_OK, pRouter.getRoute(true,sourceID,sinkID,listRoutes));
    ASSERT_EQ(0, listRoutes.size());

}

//test that checks just sinks and source in a domain
TEST_F(CAmRouterTest,simpleRoute2withDomain)
{
    EXPECT_CALL(pMockControlInterface,getConnectionFormatChoice(_,_,_,_,_)).WillRepeatedly(DoAll(returnConnectionFormat(), Return(E_OK)));

    //initialize 2 domains
    am_Domain_s domain1;
    am_domainID_t domainID1;

    domain1.domainID = 0;
    domain1.name = "domain1";
    domain1.busname = "domain1bus";
    domain1.state = DS_CONTROLLED;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain1,domainID1));

    am_Source_s source;
    am_sourceID_t sourceID;

    source.domainID = domainID1;
    source.name = "source1";
    source.sourceState = SS_ON;
    source.sourceID = 0;
    source.sourceClassID = 5;
    source.listConnectionFormats.push_back(CF_GENIVI_ANALOG);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));

    am_Sink_s sink;
    am_sinkID_t sinkID;

    sink.domainID = domainID1;
    sink.name = "sink1";
    sink.sinkID = 0;
    sink.sinkClassID = 5;
    sink.muteState = MS_MUTED;
    sink.listConnectionFormats.push_back(CF_GENIVI_ANALOG);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));

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

    ASSERT_EQ(E_OK, pRouter.getRoute(true,sourceID,sinkID,listRoutes));
    ASSERT_EQ(1, listRoutes.size());
    ASSERT_TRUE(pCF.compareRoute(compareRoute,listRoutes[0]));

}

//test that checks just 2 domains, one sink one source with only one connection format each
TEST_F(CAmRouterTest,simpleRoute2DomainsOnlyFree)
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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain1,domainID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain2,domainID2));

    am_Source_s source, gwSource;
    am_sourceID_t sourceID, gwSourceID;

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource,gwSourceID));

    am_Sink_s sink, gwSink;
    am_sinkID_t sinkID, gwSinkID;

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink,gwSinkID));

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway,gatewayID));

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

    ASSERT_EQ(E_OK, pRouter.getRoute(true,sourceID,sinkID,listRoutes));
    ASSERT_EQ(1, listRoutes.size());
    ASSERT_TRUE(pCF.compareRoute(compareRoute,listRoutes[0]));

}

//test that checks just 2 domains, one sink one source with only one connection format each
TEST_F(CAmRouterTest,simpleRoute2DomainsOnlyFreeNotFree)
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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain1,domainID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain2,domainID2));

    am_Source_s source, gwSource;
    am_sourceID_t sourceID, gwSourceID;

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource,gwSourceID));

    am_Sink_s sink, gwSink;
    am_sinkID_t sinkID, gwSinkID;

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink,gwSinkID));

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway,gatewayID));

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

    am_Connection_s connection,connection1;
    am_connectionID_t id1,id2;
    connection.sourceID=sourceID;
    connection.sinkID=gwSinkID;
    connection.connectionFormat=CF_GENIVI_ANALOG;
    connection.connectionID=0;
    connection1.sourceID=gwSourceID;
    connection1.sinkID=sinkID;
    connection1.connectionFormat=CF_GENIVI_ANALOG;
    connection1.connectionID=0;

    ASSERT_EQ(E_OK,pDatabaseHandler.enterConnectionDB(connection,id1));
    ASSERT_EQ(E_OK,pDatabaseHandler.enterConnectionDB(connection1,id2));

    ASSERT_EQ(E_OK, pRouter.getRoute(true,sourceID,sinkID,listRoutes));
    ASSERT_EQ(0, listRoutes.size());

    ASSERT_EQ(E_OK, pRouter.getRoute(false,sourceID,sinkID,listRoutes));
    ASSERT_EQ(1, listRoutes.size());
    ASSERT_TRUE(pCF.compareRoute(compareRoute,listRoutes[0]));
}

//test that checks 3 domains, one sink one source, longer lists of connectionformats.
TEST_F(CAmRouterTest,simpleRoute3DomainsListConnectionFormats_2)
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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain1,domainID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain2,domainID2));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain3,domainID3));

    am_Source_s source, gwSource, gwSource1;
    am_sourceID_t sourceID, gwSourceID, gwSourceID1;

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource,gwSourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource1,gwSourceID1));

    am_Sink_s sink, gwSink, gwSink1;
    am_sinkID_t sinkID, gwSinkID, gwSinkID1;

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
    gwSink1.listConnectionFormats.push_back(CF_GENIVI_STEREO);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink,gwSinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink1,gwSinkID1));

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway,gatewayID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway1,gatewayID1));

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

    ASSERT_EQ(E_OK, pRouter.getRoute(false,sourceID,sinkID,listRoutes));
    ASSERT_EQ(1, listRoutes.size());
    ASSERT_TRUE(pCF.compareRoute(compareRoute,listRoutes[0]));
}

//test that checks 3 domains, one sink one source, longer lists of connectionformats.
TEST_F(CAmRouterTest,simpleRoute3DomainsListConnectionFormats_1)
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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain1,domainID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain2,domainID2));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain3,domainID3));

    am_Source_s source, gwSource, gwSource1;
    am_sourceID_t sourceID, gwSourceID, gwSourceID1;

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
    gwSource.listConnectionFormats.push_back(CF_GENIVI_STEREO);
    gwSource.listConnectionFormats.push_back(CF_GENIVI_MONO);
    gwSource.listConnectionFormats.push_back(CF_GENIVI_ANALOG);

    gwSource1.domainID = domainID3;
    gwSource1.name = "gwsource2";
    gwSource1.sourceState = SS_ON;
    gwSource1.sourceID = 0;
    gwSource1.sourceClassID = 5;
    gwSource1.listConnectionFormats.push_back(CF_GENIVI_MONO);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource,gwSourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource1,gwSourceID1));

    am_Sink_s sink, gwSink, gwSink1;
    am_sinkID_t sinkID, gwSinkID, gwSinkID1;

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink,gwSinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink1,gwSinkID1));

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway,gatewayID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway1,gatewayID1));

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

    ASSERT_EQ(E_OK, pRouter.getRoute(false,sourceID,sinkID,listRoutes));
    ASSERT_EQ(1, listRoutes.size());
    ASSERT_TRUE(pCF.compareRoute(compareRoute,listRoutes[0]));
}


//test that checks 3 domains, one sink one source, longer lists of connectionformats.
TEST_F(CAmRouterTest,simpleRoute3DomainsListConnectionFormats)
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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain1,domainID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain2,domainID2));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain3,domainID3));

    am_Source_s source, gwSource, gwSource1;
    am_sourceID_t sourceID, gwSourceID, gwSourceID1;

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource,gwSourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource1,gwSourceID1));

    am_Sink_s sink, gwSink, gwSink1;
    am_sinkID_t sinkID, gwSinkID, gwSinkID1;

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink,gwSinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink1,gwSinkID1));

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway,gatewayID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway1,gatewayID1));

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

    ASSERT_EQ(E_OK, pRouter.getRoute(false,sourceID,sinkID,listRoutes));
    ASSERT_EQ(1, listRoutes.size());
    ASSERT_TRUE(pCF.compareRoute(compareRoute,listRoutes[0]));
}


//test that checks 4 domains, one sink and one source but there are 2 routes because there are 2 gateways
TEST_F(CAmRouterTest,simpleRoute4Domains2Routes)
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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain1,domainID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain2,domainID2));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain3,domainID3));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain4,domainID4));

    am_Source_s source, gwSource, gwSource1, gwSource2, gwSource3;
    am_sourceID_t sourceID, gwSourceID, gwSourceID1, gwSourceID2, gwSourceID3;

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource,gwSourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource1,gwSourceID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource2,gwSourceID2));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource3,gwSourceID3));

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink,gwSinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink1,gwSinkID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink2,gwSinkID2));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink3,gwSinkID3));

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway,gatewayID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway1,gatewayID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway2,gatewayID2));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway3,gatewayID3));

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

    ASSERT_EQ(E_OK, pRouter.getRoute(false,sourceID,sinkID,listRoutes));
    ASSERT_EQ(2, listRoutes.size());
    ASSERT_TRUE(pCF.compareRoute(compareRoute,listRoutes[0]));
    ASSERT_TRUE(pCF.compareRoute(compareRoute1,listRoutes[1]));
}

//test that checks 3 domains, one sink one source but the connectionformat of third domains do not fit.
TEST_F(CAmRouterTest,simpleRoute3DomainsNoConnection)
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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain1,domainID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain2,domainID2));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain3,domainID3));

    am_Source_s source, gwSource, gwSource1;
    am_sourceID_t sourceID, gwSourceID, gwSourceID1;

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource,gwSourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource1,gwSourceID1));

    am_Sink_s sink, gwSink, gwSink1;
    am_sinkID_t sinkID, gwSinkID, gwSinkID1;

    sink.domainID = domainID3;
    sink.name = "sink1";
    sink.sinkID = 0;
    sink.sinkClassID = 5;
    sink.muteState = MS_MUTED;
    sink.listConnectionFormats.push_back(CF_GENIVI_STEREO);

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink,gwSinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink1,gwSinkID1));

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway,gatewayID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway1,gatewayID1));

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

    ASSERT_EQ(E_OK, pRouter.getRoute(false,sourceID,sinkID,listRoutes));
    ASSERT_EQ(0, listRoutes.size());
}
//test that checks just 2 domains, one sink one source with only one connection format each
TEST_F(CAmRouterTest,simpleRoute2Domains)
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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain1,domainID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain2,domainID2));

    am_Source_s source, gwSource;
    am_sourceID_t sourceID, gwSourceID;

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource,gwSourceID));

    am_Sink_s sink, gwSink;
    am_sinkID_t sinkID, gwSinkID;

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink,gwSinkID));

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway,gatewayID));

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

    ASSERT_EQ(E_OK, pRouter.getRoute(false,sourceID,sinkID,listRoutes));
    ASSERT_EQ(1, listRoutes.size());
    ASSERT_TRUE(pCF.compareRoute(compareRoute,listRoutes[0]));

}

//test that checks just 2 domains, one sink one source but the connectionformat of source
TEST_F(CAmRouterTest,simpleRoute2DomainsNoMatchConnectionFormats)
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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain1,domainID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain2,domainID2));

    am_Source_s source, gwSource;
    am_sourceID_t sourceID, gwSourceID;

    source.domainID = domainID1;
    source.name = "source1";
    source.sourceState = SS_ON;
    source.sourceID = 0;
    source.sourceClassID = 5;
    source.listConnectionFormats.push_back(CF_GENIVI_STEREO);

    gwSource.domainID = domainID2;
    gwSource.name = "gwsource1";
    gwSource.sourceState = SS_ON;
    gwSource.sourceID = 0;
    gwSource.sourceClassID = 5;
    gwSource.listConnectionFormats.push_back(CF_GENIVI_MONO);

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource,gwSourceID));

    am_Sink_s sink, gwSink;
    am_sinkID_t sinkID, gwSinkID;

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink,gwSinkID));

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway,gatewayID));

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

    ASSERT_EQ(E_OK, pRouter.getRoute(false,sourceID,sinkID,listRoutes));
    ASSERT_EQ(0, listRoutes.size());
}

//test that checks 3 domains, one sink one source.
TEST_F(CAmRouterTest,simpleRoute3Domains)
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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain1,domainID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain2,domainID2));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain3,domainID3));

    am_Source_s source, gwSource, gwSource1;
    am_sourceID_t sourceID, gwSourceID, gwSourceID1;

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource,gwSourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource1,gwSourceID1));

    am_Sink_s sink, gwSink, gwSink1;
    am_sinkID_t sinkID, gwSinkID, gwSinkID1;

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink,gwSinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink1,gwSinkID1));

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway,gatewayID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway1,gatewayID1));

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

    ASSERT_EQ(E_OK, pRouter.getRoute(false,sourceID,sinkID,listRoutes));
    ASSERT_EQ(1, listRoutes.size());
    ASSERT_TRUE(pCF.compareRoute(compareRoute,listRoutes[0]));
}

//test that checks 4 domains, one sink and one source.
TEST_F(CAmRouterTest,simpleRoute4Domains)
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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain1,domainID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain2,domainID2));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain3,domainID3));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain4,domainID4));

    am_Source_s source, gwSource, gwSource1, gwSource2;
    am_sourceID_t sourceID, gwSourceID, gwSourceID1, gwSourceID2;

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource,gwSourceID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource1,gwSourceID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(gwSource2,gwSourceID2));

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink,gwSinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink1,gwSinkID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(gwSink2,gwSinkID2));

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

    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway,gatewayID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway1,gatewayID1));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterGatewayDB(gateway2,gatewayID2));

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

    ASSERT_EQ(E_OK, pRouter.getRoute(false,sourceID,sinkID,listRoutes));
    ASSERT_EQ(1, listRoutes.size());
    ASSERT_TRUE(pCF.compareRoute(compareRoute,listRoutes[0]));
}

int main(int argc, char **argv)
{
    CAmDltWrapper::instance()->registerApp("routing", "CAmRouterTest");
    logInfo("Routing Test started ");
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

