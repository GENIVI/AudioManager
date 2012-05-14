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

#include "CAmRoutingInterfaceTest.h"
#include "shared/CAmDltWrapper.h"

using namespace am;
using namespace testing;

CAmRoutingInterfaceTest::CAmRoutingInterfaceTest() :
        plistRoutingPluginDirs(), //
        plistCommandPluginDirs(), //
        pSocketHandler(), //
        pDatabaseHandler(std::string(":memory:")), //
        pRoutingSender(plistRoutingPluginDirs), //
        pCommandSender(plistCommandPluginDirs), //
        pControlSender(""), //
        pRouter(&pDatabaseHandler, &pControlSender), //
        pMockInterface(), //
        pRoutingInterfaceBackdoor(), //
        pCommandInterfaceBackdoor(), //
        pControlReceiver(&pDatabaseHandler, &pRoutingSender, &pCommandSender, &pSocketHandler, &pRouter), //
        pObserver(&pCommandSender, &pRoutingSender, &pSocketHandler)
{
    pDatabaseHandler.registerObserver(&pObserver);
    pRoutingInterfaceBackdoor.unloadPlugins(&pRoutingSender);
    pRoutingInterfaceBackdoor.injectInterface(&pRoutingSender, &pMockInterface, "mock");
    pCommandInterfaceBackdoor.unloadPlugins(&pCommandSender);
}

CAmRoutingInterfaceTest::~CAmRoutingInterfaceTest()
{
}

void CAmRoutingInterfaceTest::SetUp()
{
    logInfo("RoutingSendInterface Test started ");

}

void CAmRoutingInterfaceTest::TearDown()
{
}

TEST_F(CAmRoutingInterfaceTest,abort)
{
    am_Sink_s sink;
    am_sinkID_t sinkID;
    am_Domain_s domain;
    am_domainID_t domainID;
    am_Handle_s handle;
    am_connectionID_t connectionID;
    std::vector<am_Handle_s> listHandles;
    pCF.createSink(sink);
    pCF.createDomain(domain);
    domain.name = "mock";
    domain.busname = "mock";
    sink.sinkID = 2;
    sink.domainID = 1;

    //prepare the stage
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));

    //start a connect, expect a call on the routingInterface
    EXPECT_CALL(pMockInterface,asyncConnect(_,_,1,sinkID,CF_GENIVI_ANALOG)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK, pControlReceiver.connect(handle,connectionID,CF_GENIVI_ANALOG,1,2));

    //check the correctness of the handle
    ASSERT_NE(handle.handle, 0);
    ASSERT_EQ(handle.handleType, H_CONNECT);

    //the handle must be inside the handlelist
    ASSERT_EQ(E_OK, pControlReceiver.getListHandles(listHandles));
    ASSERT_TRUE(listHandles[0].handle==handle.handle);
    ASSERT_TRUE(listHandles[0].handleType==handle.handleType);

    //send an abort expect a call on the routing interface
    EXPECT_CALL(pMockInterface,asyncAbort(_)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK, pControlReceiver.abortAction(handle));

    //the reaction on the abort is specific for every function

    //now we try to abort a handle that does not exist
    handle.handle = 24;
    ASSERT_EQ(E_NON_EXISTENT, pControlReceiver.abortAction(handle));
}

TEST_F(CAmRoutingInterfaceTest,abortNonExistent)
{
    EXPECT_CALL(pMockInterface,asyncAbort(_)).Times(0);
    am_Handle_s handle;
    ASSERT_EQ(E_NON_EXISTENT, pControlReceiver.abortAction(handle));
}

TEST_F(CAmRoutingInterfaceTest,alreadyConnected)
{
    am_Sink_s sink;
    am_sinkID_t sinkID;
    am_Domain_s domain;
    am_domainID_t domainID;
    pCF.createSink(sink);
    pCF.createDomain(domain);
    domain.name = "mock";
    domain.busname = "mock";
    sink.sinkID = 2;
    sink.domainID = 1;
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    EXPECT_CALL(pMockInterface,asyncConnect(_,_,1,sinkID,CF_GENIVI_ANALOG)).WillOnce(Return(E_OK));
    am_Handle_s handle;
    am_connectionID_t connectionID;
    ASSERT_EQ(E_OK, pControlReceiver.connect(handle,connectionID,CF_GENIVI_ANALOG,1,2));
    ASSERT_EQ(E_OK, pDatabaseHandler.changeConnectionFinal(connectionID));
    ASSERT_EQ(E_ALREADY_EXISTS, pControlReceiver.connect(handle,connectionID,CF_GENIVI_ANALOG,1,2));
    ASSERT_NE(handle.handle, 0);
    ASSERT_EQ(handle.handleType, H_CONNECT);
}

TEST_F(CAmRoutingInterfaceTest,setSinkSoundPropertyNoChange)
{
    am_Sink_s sink;
    am_sinkID_t sinkID;
    am_Domain_s domain;
    am_domainID_t domainID;
    am_Handle_s handle;
    am_SoundProperty_s soundProperty;
    soundProperty.type = SP_EXAMPLE_TREBLE;
    soundProperty.value = 23;
    std::vector<am_Handle_s> listHandles;
    pCF.createSink(sink);
    pCF.createDomain(domain);
    domain.name = "mock";
    domain.busname = "mock";
    sink.sinkID = 2;
    sink.domainID = 1;
    sink.listSoundProperties.push_back(soundProperty);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    EXPECT_CALL(pMockInterface,asyncSetSinkSoundProperty(_,sinkID,_)).Times(0);
    ASSERT_EQ(E_NO_CHANGE, pControlReceiver.setSinkSoundProperty(handle,sinkID,soundProperty));
}

TEST_F(CAmRoutingInterfaceTest,setSourceState)
{
    am_Source_s source;
    am_sourceID_t sourceID;
    am_Domain_s domain;
    am_domainID_t domainID;
    am_Handle_s handle;
    handle.handle = 0;
    am_SourceState_e state = SS_PAUSED;
    pCF.createSource(source);
    pCF.createDomain(domain);
    domain.name = "mock";
    domain.busname = "mock";
    source.domainID = 1;
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    EXPECT_CALL(pMockInterface,asyncSetSourceState(_,sourceID,state)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK, pControlReceiver.setSourceState(handle,sourceID,state));
    ASSERT_NE(handle.handle, 0);
    ASSERT_EQ(handle.handleType, H_SETSOURCESTATE);
}

TEST_F(CAmRoutingInterfaceTest,setSourceSoundProperty)
{
    am_Source_s source;
    am_sourceID_t sourceID;
    am_Domain_s domain;
    am_domainID_t domainID;
    am_Handle_s handle;
    am_SoundProperty_s soundProperty;
    std::vector<am_Handle_s> listHandles;
    pCF.createSource(source);
    pCF.createDomain(domain);
    domain.name = "mock";
    domain.busname = "mock";
    source.sourceID = 2;
    source.domainID = 1;
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    EXPECT_CALL(pMockInterface,asyncSetSourceSoundProperty(_,sourceID,_)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK, pControlReceiver.setSourceSoundProperty(handle,sourceID,soundProperty));
    ASSERT_NE(handle.handle, 0);
    ASSERT_EQ(handle.handleType, H_SETSOURCESOUNDPROPERTY);
    ASSERT_EQ(E_OK, pControlReceiver.getListHandles(listHandles));
    ASSERT_TRUE(listHandles[0].handle==handle.handle);
    ASSERT_TRUE(listHandles[0].handleType==handle.handleType);
}

TEST_F(CAmRoutingInterfaceTest,setSinkSoundProperty)
{
    am_Sink_s sink;
    am_sinkID_t sinkID;
    am_Domain_s domain;
    am_domainID_t domainID;
    am_Handle_s handle;
    am_SoundProperty_s soundProperty;
    soundProperty.value=5;
    soundProperty.type=SP_EXAMPLE_MID;
    std::vector<am_Handle_s> listHandles;
    pCF.createSink(sink);
    pCF.createDomain(domain);
    domain.name = "mock";
    domain.busname = "mock";
    sink.sinkID = 2;
    sink.domainID = 1;
    sink.listSoundProperties.push_back(soundProperty);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    EXPECT_CALL(pMockInterface,asyncSetSinkSoundProperty(_,sinkID,_)).WillOnce(Return(E_OK));
    soundProperty.value=10;
    ASSERT_EQ(E_OK, pControlReceiver.setSinkSoundProperty(handle,sinkID,soundProperty));
    ASSERT_NE(handle.handle, 0);
    ASSERT_EQ(handle.handleType, H_SETSINKSOUNDPROPERTY);
    ASSERT_EQ(E_OK, pControlReceiver.getListHandles(listHandles));
    ASSERT_TRUE(listHandles[0].handle==handle.handle);
    ASSERT_TRUE(listHandles[0].handleType==handle.handleType);
}

TEST_F(CAmRoutingInterfaceTest,setSourceVolumeNoChange)
{
    am_Source_s source;
    am_sourceID_t sourceID;
    am_Domain_s domain;
    am_domainID_t domainID;
    am_Handle_s handle;
    am_volume_t volume = 34;
    am_RampType_e rampType = RAMP_GENIVI_DIRECT;
    am_time_t rampTime = 300;
    std::vector<am_Handle_s> listHandles;
    pCF.createSource(source);
    pCF.createDomain(domain);
    domain.name = "mock";
    domain.busname = "mock";
    source.sourceID = 2;
    source.domainID = 1;
    source.volume = volume;
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    EXPECT_CALL(pMockInterface,asyncSetSourceVolume(_,2,volume,rampType,rampTime)).Times(0);
    ASSERT_EQ(E_NO_CHANGE, pControlReceiver.setSourceVolume(handle,2,volume,rampType,rampTime));
    ASSERT_EQ(E_OK, pControlReceiver.getListHandles(listHandles));
    ASSERT_TRUE(listHandles.empty());
}

TEST_F(CAmRoutingInterfaceTest,setSourceVolume)
{
    am_Source_s source;
    am_sourceID_t sourceID;
    am_Domain_s domain;
    am_domainID_t domainID;
    am_Handle_s handle;
    am_volume_t volume = 34;
    am_RampType_e rampType = RAMP_GENIVI_DIRECT;
    am_time_t rampTime = 300;
    std::vector<am_Handle_s> listHandles;
    pCF.createSource(source);
    pCF.createDomain(domain);
    domain.name = "mock";
    domain.busname = "mock";
    source.sourceID = 2;
    source.domainID = 1;
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    EXPECT_CALL(pMockInterface,asyncSetSourceVolume(_,2,volume,rampType,rampTime)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK, pControlReceiver.setSourceVolume(handle,2,volume,rampType,rampTime));
    ASSERT_NE(handle.handle, 0);
    ASSERT_EQ(handle.handleType, H_SETSOURCEVOLUME);
    ASSERT_EQ(E_OK, pControlReceiver.getListHandles(listHandles));
    ASSERT_TRUE(listHandles[0].handle==handle.handle);
    ASSERT_TRUE(listHandles[0].handleType==handle.handleType);
}

TEST_F(CAmRoutingInterfaceTest,setSinkVolumeNoChange)
{
    am_Sink_s sink;
    am_sinkID_t sinkID;
    am_Domain_s domain;
    am_domainID_t domainID;
    am_Handle_s handle;
    am_volume_t volume = 34;
    am_RampType_e rampType = RAMP_GENIVI_DIRECT;
    am_time_t rampTime = 300;
    std::vector<am_Handle_s> listHandles;
    pCF.createSink(sink);
    pCF.createDomain(domain);
    domain.name = "mock";
    domain.busname = "mock";
    sink.sinkID = 2;
    sink.domainID = 1;
    sink.volume = volume;
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    EXPECT_CALL(pMockInterface,asyncSetSinkVolume(_,2,volume,rampType,rampTime)).Times(0);
    ASSERT_EQ(E_NO_CHANGE, pControlReceiver.setSinkVolume(handle,2,volume,rampType,rampTime));
    ASSERT_EQ(E_OK, pControlReceiver.getListHandles(listHandles));
    ASSERT_TRUE(listHandles.empty());
}

TEST_F(CAmRoutingInterfaceTest,setSinkVolume)
{
    am_Sink_s sink;
    am_sinkID_t sinkID;
    am_Domain_s domain;
    am_domainID_t domainID;
    am_Handle_s handle;
    am_volume_t volume = 34;
    am_RampType_e rampType = RAMP_GENIVI_DIRECT;
    am_time_t rampTime = 300;
    std::vector<am_Handle_s> listHandles;
    pCF.createSink(sink);
    pCF.createDomain(domain);
    domain.name = "mock";
    domain.busname = "mock";
    sink.sinkID = 2;
    sink.domainID = 1;
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    EXPECT_CALL(pMockInterface,asyncSetSinkVolume(_,2,volume,rampType,rampTime)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK, pControlReceiver.setSinkVolume(handle,2,volume,rampType,rampTime));
    ASSERT_NE(handle.handle, 0);
    ASSERT_EQ(handle.handleType, H_SETSINKVOLUME);
    ASSERT_EQ(E_OK, pControlReceiver.getListHandles(listHandles));
    ASSERT_TRUE(listHandles[0].handle==handle.handle);
    ASSERT_TRUE(listHandles[0].handleType==handle.handleType);
}

TEST_F(CAmRoutingInterfaceTest,connect)
{
    am_Sink_s sink;
    am_sinkID_t sinkID;
    am_Domain_s domain;
    am_domainID_t domainID;
    am_Handle_s handle;
    am_connectionID_t connectionID;
    std::vector<am_Handle_s> listHandles;
    pCF.createSink(sink);
    pCF.createDomain(domain);
    domain.name = "mock";
    domain.busname = "mock";
    sink.sinkID = 2;
    sink.domainID = 1;
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    EXPECT_CALL(pMockInterface,asyncConnect(_,_,1,sinkID,CF_GENIVI_ANALOG)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK, pControlReceiver.connect(handle,connectionID,CF_GENIVI_ANALOG,1,2));
    ASSERT_NE(handle.handle, 0);
    ASSERT_EQ(handle.handleType, H_CONNECT);
    ASSERT_EQ(E_OK, pControlReceiver.getListHandles(listHandles));
    ASSERT_TRUE(listHandles[0].handle==handle.handle);
    ASSERT_TRUE(listHandles[0].handleType==handle.handleType);
}

TEST_F(CAmRoutingInterfaceTest,disconnect)
{
    am_Sink_s sink;
    am_sinkID_t sinkID;
    am_Domain_s domain;
    am_domainID_t domainID;
    am_Handle_s handle;
    am_connectionID_t connectionID;
    std::vector<am_Handle_s> listHandles;
    pCF.createSink(sink);
    pCF.createDomain(domain);
    domain.name = "mock";
    domain.busname = "mock";
    sink.sinkID = 2;
    sink.domainID = 1;
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    EXPECT_CALL(pMockInterface,asyncConnect(_,_,1,sinkID,CF_GENIVI_ANALOG)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK, pControlReceiver.connect(handle,connectionID,CF_GENIVI_ANALOG,1,2));
    ASSERT_EQ(E_OK, pDatabaseHandler.changeConnectionFinal(connectionID));
    EXPECT_CALL(pMockInterface,asyncDisconnect(_,connectionID)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK, pControlReceiver.disconnect(handle,connectionID));
    ASSERT_NE(handle.handle, 0);
    ASSERT_EQ(handle.handleType, H_DISCONNECT);
    ASSERT_EQ(E_OK, pControlReceiver.getListHandles(listHandles));
    ASSERT_TRUE(listHandles[1].handle==handle.handle);
    ASSERT_TRUE(listHandles[1].handleType==handle.handleType);
}

TEST_F(CAmRoutingInterfaceTest,nothingTodisconnect)
{
    am_Handle_s handle;
    am_connectionID_t connectionID = 4;
    std::vector<am_Handle_s> listHandles;
    ASSERT_EQ(E_NON_EXISTENT, pControlReceiver.disconnect(handle,connectionID));
    ASSERT_EQ(E_OK, pControlReceiver.getListHandles(listHandles));
    ASSERT_TRUE(listHandles.empty());
}

TEST_F(CAmRoutingInterfaceTest,setSourceStateNoChange)
{
    am_Source_s source;
    am_sourceID_t sourceID;
    am_Domain_s domain;
    am_domainID_t domainID;
    am_Handle_s handle;
    handle.handle = 0;
    am_SourceState_e state = SS_PAUSED;
    pCF.createSource(source);
    pCF.createDomain(domain);
    domain.name = "mock";
    domain.busname = "mock";
    source.domainID = 1;
    source.sourceState = state;
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));
    EXPECT_CALL(pMockInterface,asyncSetSourceState(_,sourceID,state)).Times(0);
    ASSERT_EQ(E_NO_CHANGE, pControlReceiver.setSourceState(handle,sourceID,state));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

