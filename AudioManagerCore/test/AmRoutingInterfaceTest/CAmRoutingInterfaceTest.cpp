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

#include "CAmRoutingInterfaceTest.h"
#include "CAmDltWrapper.h"
#include "CAmCommandLineSingleton.h"

using namespace am;
using namespace testing;

TCLAP::SwitchArg enableDebug ("V","logDlt","print DLT logs to stdout or dlt-daemon default off",false);

CAmRoutingInterfaceTest::CAmRoutingInterfaceTest() :
        plistRoutingPluginDirs(), //
        plistCommandPluginDirs(), //
        pSocketHandler(), //
        pDatabaseHandler(), //
        pRoutingSender(plistRoutingPluginDirs,dynamic_cast<IAmDatabaseHandler*>( &pDatabaseHandler )), //
        pCommandSender(plistCommandPluginDirs, &pSocketHandler), //
        pControlSender(), //
        pRouter(&pDatabaseHandler, &pControlSender), //
        pMockInterface(), //
        pRoutingInterfaceBackdoor(), //
        pCommandInterfaceBackdoor(), //
        pControlReceiver(&pDatabaseHandler, &pRoutingSender, &pCommandSender, &pSocketHandler, &pRouter), //
        pRoutingReceiver(&pDatabaseHandler,  &pRoutingSender, &pControlSender, &pSocketHandler)
{
    pDatabaseHandler.registerObserver(&pRoutingSender);
    pDatabaseHandler.registerObserver(&pCommandSender);
    pRoutingInterfaceBackdoor.unloadPlugins(&pRoutingSender);
    pRoutingInterfaceBackdoor.injectInterface(&pRoutingSender, &pMockInterface, "mock");
    pControlInterfaceBackdoor.replaceController(&pControlSender, &pMockControlInterface);
    pCommandInterfaceBackdoor.unloadPlugins(&pCommandSender);
}

CAmRoutingInterfaceTest::~CAmRoutingInterfaceTest()
{
}

void CAmRoutingInterfaceTest::SetUp()
{
    logInfo("RoutingSendInterface Test started ");
	am_Domain_s domain;
	pCF.createDomain(domain);
	domain.domainID=0;
	domain.busname="mock";
    am_domainID_t forgetDomain;
    am_sinkClass_t forgetSinkClassID;
    am_SinkClass_s sinkClass;
    sinkClass.name="TestSinkClass";
    sinkClass.sinkClassID=1;
    am_sourceClass_t forgetSourceClassID;
    am_SourceClass_s sourceClass;
    sourceClass.name="TestSourceClass";
    sourceClass.sourceClassID=1;
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,forgetDomain));
    ASSERT_EQ(E_OK,pDatabaseHandler.enterSinkClassDB(sinkClass,forgetSinkClassID));
    ASSERT_EQ(E_OK,pDatabaseHandler.enterSourceClassDB(forgetSourceClassID,sourceClass));
    ASSERT_EQ(E_OK,pRoutingSender.addDomainLookup(domain));


}

void CAmRoutingInterfaceTest::TearDown()
{
}


TEST_F(CAmRoutingInterfaceTest,connectRace)
{
    am_Sink_s sink;
    am_sinkID_t sinkID;
    am_Domain_s domain;
    am_domainID_t domainID;
    am_Handle_s handle;
    am_connectionID_t connectionID;
    std::vector<am_Handle_s> listHandles;
    am_Source_s source;
    pCF.createSource(source);
    source.sourceID=1;
    source.domainID=DYNAMIC_ID_BOUNDARY;
    source.name="sds";

    pCF.createSink(sink);
    pCF.createDomain(domain);
    domain.name = "mock";
    domain.busname = "mock";
    domain.domainID=0;
    sink.sinkID = 2;
    sink.domainID = DYNAMIC_ID_BOUNDARY;

    //prepare the stage
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,source.sourceID));

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
	
	//so we aborted but there was a race. We get the correct answer now. 
    EXPECT_CALL(pMockControlInterface,cbAckConnect(_,E_OK));
    pRoutingReceiver.ackConnect(handle,connectionID,E_OK);
    
    //the abort can be ignored or anwered with error by the routingadaper, should not matter

    EXPECT_CALL(pMockControlInterface,cbAckConnect(_,E_NON_EXISTENT));    
    pRoutingReceiver.ackConnect(handle,connectionID,E_NON_EXISTENT);
	
	std::vector<am_Connection_s> listconnections;
	//In the end, the database must be correct:
	pDatabaseHandler.getListConnections(listconnections);
	ASSERT_EQ(listconnections[0].connectionID,connectionID);
	
	//In the end, the database must be correct:
	pDatabaseHandler.getListConnectionsReserved(listconnections);
	ASSERT_EQ(listconnections[0].connectionID,connectionID);
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
    am_Source_s source;
    pCF.createSource(source);
    source.sourceID=1;
    source.domainID=DYNAMIC_ID_BOUNDARY;
    source.name="sds";

    pCF.createSink(sink);
    pCF.createDomain(domain);
    domain.name = "mock";
    domain.busname = "mock";
    domain.domainID=0;
    sink.sinkID = 2;
    sink.domainID = DYNAMIC_ID_BOUNDARY;

    //prepare the stage
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,source.sourceID));

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

    EXPECT_CALL(pMockControlInterface,cbAckConnect(_,E_ABORTED));
    pRoutingReceiver.ackConnect(handle,connectionID,E_ABORTED);

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
    sink.domainID = DYNAMIC_ID_BOUNDARY;


    am_SourceClass_s sourceclass;

    sourceclass.name="sClass";
    sourceclass.sourceClassID=5;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceClassDB(sourceclass.sourceClassID,sourceclass));

    am_SinkClass_s sinkclass;
    sinkclass.sinkClassID=5;
    sinkclass.name="sname";

    am_Source_s source;
    pCF.createSource(source);
    source.sourceID=1;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkClassDB(sinkclass,sinkclass.sinkClassID));

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,source.sourceID));
    EXPECT_CALL(pMockInterface,asyncConnect(_,_,1,sinkID,CF_GENIVI_ANALOG)).WillOnce(Return(E_OK));
    am_Handle_s handle;
    am_connectionID_t connectionID;
    ASSERT_EQ(E_OK, pControlReceiver.connect(handle,connectionID,CF_GENIVI_ANALOG,1,2));
    ASSERT_EQ(E_OK, pDatabaseHandler.changeConnectionFinal(connectionID));
    //ASSERT_EQ(E_ALREADY_EXISTS, pControlReceiver.connect(handle,connectionID,CF_GENIVI_ANALOG,1,2));
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
    soundProperty.type = SP_GENIVI_TREBLE;
    soundProperty.value = 23;
    std::vector<am_Handle_s> listHandles;
    pCF.createSink(sink);
    pCF.createDomain(domain);
    domain.name = "mock";
    domain.busname = "mock";
    sink.sinkID = 2;
    sink.domainID = DYNAMIC_ID_BOUNDARY;
    sink.listSoundProperties.push_back(soundProperty);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    EXPECT_CALL(pMockInterface,asyncSetSinkSoundProperty(_,sinkID,_)).Times(1).WillOnce(Return(E_NO_CHANGE));
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
    source.domainID = DYNAMIC_ID_BOUNDARY;
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
    source.domainID = DYNAMIC_ID_BOUNDARY;
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
    soundProperty.type=SP_GENIVI_MID;
    std::vector<am_Handle_s> listHandles;
    pCF.createSink(sink);
    pCF.createDomain(domain);
    domain.name = "mock";
    domain.busname = "mock";
    sink.sinkID = 2;
    sink.domainID = DYNAMIC_ID_BOUNDARY;
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



TEST_F(CAmRoutingInterfaceTest,setSourceVolume)
{
    am_Source_s source;
    am_sourceID_t sourceID;
    am_Domain_s domain;
    am_domainID_t domainID;
    am_Handle_s handle;
    am_volume_t volume = 34;
    am_CustomRampType_t rampType = RAMP_GENIVI_DIRECT;
    am_time_t rampTime = 300;
    std::vector<am_Handle_s> listHandles;
    pCF.createSource(source);
    pCF.createDomain(domain);
    domain.name = "mock";
    domain.busname = "mock";
    source.sourceID = 2;
    source.domainID = DYNAMIC_ID_BOUNDARY;
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

TEST_F(CAmRoutingInterfaceTest,setSinkVolume)
{
    am_Sink_s sink;
    am_sinkID_t sinkID;
    am_Domain_s domain;
    am_domainID_t domainID;
    am_Handle_s handle;
    am_volume_t volume = 34;
    am_CustomRampType_t rampType = RAMP_GENIVI_DIRECT;
    am_time_t rampTime = 300;
    std::vector<am_Handle_s> listHandles;
    pCF.createSink(sink);
    pCF.createDomain(domain);
    domain.name = "mock";
    domain.busname = "mock";
    sink.sinkID = 2;
    sink.domainID = DYNAMIC_ID_BOUNDARY;
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
    sink.domainID = DYNAMIC_ID_BOUNDARY;

    am_Source_s source;
    pCF.createSource(source);
    source.sourceID=1;
    source.domainID=DYNAMIC_ID_BOUNDARY;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,source.sourceID));
    EXPECT_CALL(pMockInterface,asyncConnect(_,_,1,sinkID,CF_GENIVI_ANALOG)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK, pControlReceiver.connect(handle,connectionID,CF_GENIVI_ANALOG,1,2));
    ASSERT_NE(handle.handle, 0);
    ASSERT_EQ(handle.handleType, H_CONNECT);
    ASSERT_EQ(E_OK, pControlReceiver.getListHandles(listHandles));
    ASSERT_TRUE(listHandles[0].handle==handle.handle);
    ASSERT_TRUE(listHandles[0].handleType==handle.handleType);
}

TEST_F(CAmRoutingInterfaceTest,connectError)
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
    sink.domainID = DYNAMIC_ID_BOUNDARY;

    am_Source_s source;
    pCF.createSource(source);
    source.sourceID=1;
    source.domainID=DYNAMIC_ID_BOUNDARY;

    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,source.sourceID));
    EXPECT_CALL(pMockInterface,asyncConnect(_,_,1,sinkID,CF_GENIVI_ANALOG)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK, pControlReceiver.connect(handle,connectionID,CF_GENIVI_ANALOG,1,2));
    ASSERT_NE(handle.handle, 0);
    ASSERT_EQ(handle.handleType, H_CONNECT);
    ASSERT_EQ(E_OK, pControlReceiver.getListHandles(listHandles));
    ASSERT_TRUE(listHandles[0].handle==handle.handle);
    ASSERT_TRUE(listHandles[0].handleType==handle.handleType);
 
 	//so we aborted but there was a race. We get the correct answer now. 
    EXPECT_CALL(pMockControlInterface,cbAckConnect(_,E_UNKNOWN));
    pRoutingReceiver.ackConnect(handle,connectionID,E_UNKNOWN);
       
 	std::vector<am_Connection_s> listconnections;
	//In the end, the database must be correct:
	pDatabaseHandler.getListConnections(listconnections);
	ASSERT_EQ(listconnections.size(),0);  
	
	//No more reservations
	pDatabaseHandler.getListConnectionsReserved(listconnections);
	ASSERT_EQ(listconnections.size(),0);	
	
	ASSERT_EQ(E_OK, pControlReceiver.getListHandles(listHandles));
	ASSERT_EQ(listHandles.size(),0);  
    
}

TEST_F(CAmRoutingInterfaceTest,disconnect)
{
    am_Sink_s sink;
    am_sinkID_t sinkID;
    am_Domain_s domain;
    am_domainID_t domainID;
    am_Handle_s handle,handle2;
    am_connectionID_t connectionID;
    std::vector<am_Handle_s> listHandles;
    pCF.createSink(sink);
    pCF.createDomain(domain);
    domain.name = "mock";
    domain.busname = "mock";
    sink.sinkID = 2;
    sink.domainID = DYNAMIC_ID_BOUNDARY;

    am_Source_s source;
     pCF.createSource(source);
     source.sourceID=1;
     source.domainID=DYNAMIC_ID_BOUNDARY;

     ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
     ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
     ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,source.sourceID));

    EXPECT_CALL(pMockInterface,asyncConnect(_,_,1,sinkID,CF_GENIVI_ANALOG)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK, pControlReceiver.connect(handle,connectionID,CF_GENIVI_ANALOG,1,2));
    ASSERT_EQ(E_OK, pDatabaseHandler.changeConnectionFinal(connectionID));
    EXPECT_CALL(pMockInterface,asyncDisconnect(_,connectionID)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK, pControlReceiver.disconnect(handle2,connectionID));
    ASSERT_NE(handle2.handle, 0);
    ASSERT_EQ(handle2.handleType, H_DISCONNECT);
    ASSERT_EQ(E_OK, pControlReceiver.getListHandles(listHandles));
    ASSERT_TRUE(listHandles[1].handle==handle2.handle);
    ASSERT_TRUE(listHandles[1].handleType==handle2.handleType);
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

TEST_F(CAmRoutingInterfaceTest,handleOverflow)
{
    am_Handle_s handle,handleOverflow1,handleOverflow2,handleOverflowCheck1,handleOverflowCheck2;	
    am_sinkID_t sinkID;
    am_Sink_s sink;
    am_Domain_s domain;
    am_domainID_t domainID;
   
    pCF.createSink(sink);
    pCF.createDomain(domain);
    domain.name = "mock";
    domain.busname = "mock";
    sink.sinkID = 2;
    sink.domainID = DYNAMIC_ID_BOUNDARY;
    am_SoundProperty_s soundProperty;
    soundProperty.type = SP_GENIVI_TREBLE;
    soundProperty.value = 23;

    sink.listSoundProperties.push_back(soundProperty);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));


    
    EXPECT_CALL(pMockInterface,asyncSetSinkSoundProperty(_,sinkID,_)).WillRepeatedly(Return(E_OK));
    
    //open handles till 50
    for(int i=0;i<50;i++) 
    {
		handle.handle=0;
		soundProperty.value = i;
		ASSERT_EQ(E_OK, pControlReceiver.setSinkSoundProperty(handle,sinkID,soundProperty));
	}
	//now we ack 2 handles
	EXPECT_CALL(pMockControlInterface,cbAckSetSinkSoundProperty(_,E_OK));
	ASSERT_EQ(E_OK, pControlReceiver.setSinkSoundProperty(handleOverflow1,sinkID,soundProperty));
	pRoutingReceiver.ackSetSinkSoundProperty(handleOverflow1,E_OK);
	
	EXPECT_CALL(pMockControlInterface,cbAckSetSinkSoundProperty(_,E_OK));
	ASSERT_EQ(E_OK, pControlReceiver.setSinkSoundProperty(handleOverflow2,sinkID,soundProperty));
	pRoutingReceiver.ackSetSinkSoundProperty(handleOverflow2,E_OK);
	    
	for(int i=52;i<1023;i++) //now we get into the overflow areay
    {
		handle.handle=0;
		soundProperty.value = i;
		ASSERT_EQ(E_OK, pControlReceiver.setSinkSoundProperty(handle,sinkID,soundProperty));
	}
	
	//the next two handles must be the one we already acked
	ASSERT_EQ(E_OK, pControlReceiver.setSinkSoundProperty(handleOverflowCheck1,sinkID,soundProperty));
	ASSERT_EQ(handleOverflow1.handle,handleOverflowCheck1.handle);
	
	ASSERT_EQ(E_OK, pControlReceiver.setSinkSoundProperty(handleOverflowCheck2,sinkID,soundProperty));
	ASSERT_EQ(handleOverflow2.handle,handleOverflowCheck2.handle);
	
}

TEST_F(CAmRoutingInterfaceTest,handleOverflowAbsolute)
{
    am_Handle_s handle,handleOverflow1,handleOverflow2,handleOverflowCheck1,handleOverflowCheck2;	
    am_sinkID_t sinkID;
    am_Sink_s sink;
    am_Domain_s domain;
    am_domainID_t domainID;
   
    pCF.createSink(sink);
    pCF.createDomain(domain);
    domain.name = "mock";
    domain.busname = "mock";
    sink.sinkID = 2;
    sink.domainID = DYNAMIC_ID_BOUNDARY;
    am_SoundProperty_s soundProperty;
    soundProperty.type = SP_GENIVI_TREBLE;
    soundProperty.value = 23;

    sink.listSoundProperties.push_back(soundProperty);
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));


    
    EXPECT_CALL(pMockInterface,asyncSetSinkSoundProperty(_,sinkID,_)).WillRepeatedly(Return(E_OK));
    
    
	for(int i=0;i<1023;i++) //we fill up the handles
    {
		handle.handle=0;
		soundProperty.value = i;
		ASSERT_EQ(E_OK, pControlReceiver.setSinkSoundProperty(handle,sinkID,soundProperty));
	}
	
	//the next handle must return 0!
	ASSERT_EQ(E_OK, pControlReceiver.setSinkSoundProperty(handleOverflowCheck1,sinkID,soundProperty));
	ASSERT_EQ(handleOverflowCheck1.handle,0);	
}



int main(int argc, char **argv)
{
	try
	{
		TCLAP::CmdLine* cmd(CAmCommandLineSingleton::instanciateOnce("The team of the AudioManager wishes you a nice day!",' ',DAEMONVERSION,true));
		cmd->add(enableDebug);
	}
	catch (TCLAP::ArgException &e)  // catch any exceptions
	{ std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; }
	CAmCommandLineSingleton::instance()->preparse(argc,argv);
		CAmDltWrapper::instanctiateOnce("RTEST","RoutingInterface Test",enableDebug.getValue(),CAmDltWrapper::logDestination::COMMAND_LINE);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

