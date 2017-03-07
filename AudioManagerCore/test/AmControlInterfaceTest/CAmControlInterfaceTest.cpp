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

#include "CAmControlInterfaceTest.h"
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include "CAmDltWrapper.h"
#include "CAmCommandLineSingleton.h"

using namespace am;
using namespace testing;

DLT_DECLARE_CONTEXT(AudioManager)
TCLAP::SwitchArg enableDebug ("V","logDlt","print DLT logs to stdout or dlt-daemon default on",true);

ACTION(returnResyncConnection)
{
	std::vector<am_Connection_s> listConnections;
	am_Connection_s conn;
	conn.sinkID=1;
	conn.sourceID=1;
	conn.connectionFormat=CF_GENIVI_ANALOG;
	listConnections.push_back(conn);
	arg1=listConnections;
}

CAmControlInterfaceTest::CAmControlInterfaceTest() :
        pSocketHandler(), //
        plistCommandPluginDirs(), //
        plistRoutingPluginDirs(), //
        pDatabaseHandler(), //
        pRoutingSender(plistRoutingPluginDirs,dynamic_cast<IAmDatabaseHandler*>( &pDatabaseHandler )), //RoutingReceiver
        pCommandSender(plistCommandPluginDirs, &pSocketHandler), //
        pMockControlInterface(), //
        pMockRoutingInterface(), //
        pRoutingInterfaceBackdoor(), //
        pCommandInterfaceBackdoor(), //
        pControlInterfaceBackdoor(), //
        pControlSender(), //
        pRouter(&pDatabaseHandler,&pControlSender), //
        pControlReceiver(&pDatabaseHandler, &pRoutingSender, &pCommandSender, &pSocketHandler, &pRouter), //
        pRoutingReceiver(&pDatabaseHandler, &pRoutingSender, &pControlSender, &pSocketHandler)
{
    pDatabaseHandler.registerObserver(&pRoutingSender);
    pDatabaseHandler.registerObserver(&pCommandSender);
    pControlInterfaceBackdoor.replaceController(&pControlSender, &pMockControlInterface);
    pRoutingInterfaceBackdoor.injectInterface(&pRoutingSender, &pMockRoutingInterface, "mock");

}

CAmControlInterfaceTest::~CAmControlInterfaceTest()
{
	CAmDltWrapper::instance()->unregisterContext(AudioManager);
}

void CAmControlInterfaceTest::SetUp()
{

	::testing::FLAGS_gmock_verbose = "error";

	am_Domain_s domain;
	pCF.createDomain(domain);
    am_domainID_t forgetDomain;
    am_sinkClass_t forgetSinkClassID;
    am_SinkClass_s sinkClass;
    sinkClass.name="TestSinkClass";
    sinkClass.sinkClassID=1;
    am_sourceClass_t forgetSourceClassID;
    am_SourceClass_s sourceClass;
    sourceClass.name="TestSourceClass";
    sourceClass.sourceClassID=1;
    domain.domainID=4;
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,forgetDomain));
    ASSERT_EQ(E_OK,pDatabaseHandler.enterSinkClassDB(sinkClass,forgetSinkClassID));
    ASSERT_EQ(E_OK,pDatabaseHandler.enterSourceClassDB(forgetSourceClassID,sourceClass));

}

void CAmControlInterfaceTest::TearDown()
{
}

TEST_F(CAmControlInterfaceTest,registerDomain)
{

    am_Domain_s domain;
    am_domainID_t domainID;
    pCF.createDomain(domain);

    //When we run this test, we expect the call on the control interface
    EXPECT_CALL(pMockControlInterface,hookSystemRegisterDomain(_,_)).WillRepeatedly(DoAll(SetArgReferee<1>(2), Return(E_OK)));
    ASSERT_EQ(E_OK, pRoutingReceiver.registerDomain(domain,domainID));
    ASSERT_EQ(domainID, 2);
}

TEST_F(CAmControlInterfaceTest,deregisterDomain)
{
    am_domainID_t domainID = 34;

    //When we run this test, we expect the call on the control interface
    EXPECT_CALL(pMockControlInterface,hookSystemDeregisterDomain(34)).WillRepeatedly(Return(E_OK));
    ASSERT_EQ(E_OK, pRoutingReceiver.deregisterDomain(domainID));
}

TEST_F(CAmControlInterfaceTest,registerSink)
{
    am_Sink_s sink;
    am_sinkID_t sinkID;
    pCF.createSink(sink);

    //When we run this test, we expect the call on the control interface
    EXPECT_CALL(pMockControlInterface,hookSystemRegisterSink(_,_)).WillRepeatedly(DoAll(SetArgReferee<1>(2), Return(E_OK)));
    ASSERT_EQ(E_OK, pRoutingReceiver.registerSink(sink,sinkID));
    ASSERT_EQ(sinkID, 2);
}

TEST_F(CAmControlInterfaceTest,deregisterSink)
{
    am_sinkID_t sinkID = 12;

    //When we run this test, we expect the call on the control interface
    EXPECT_CALL(pMockControlInterface,hookSystemDeregisterSink(12)).WillRepeatedly(Return(E_OK));
    ASSERT_EQ(E_OK, pRoutingReceiver.deregisterSink(sinkID));
}

TEST_F(CAmControlInterfaceTest,registerSource)
{
    am_Source_s source;
    am_sourceID_t sourceID;
    pCF.createSource(source);

    //When we run this test, we expect the call on the control interface
    EXPECT_CALL(pMockControlInterface,hookSystemRegisterSource(_,_)).WillRepeatedly(DoAll(SetArgReferee<1>(2), Return(E_OK)));
    ASSERT_EQ(E_OK, pRoutingReceiver.registerSource(source,sourceID));
    ASSERT_EQ(sourceID, 2);
}

TEST_F(CAmControlInterfaceTest,deregisterSource)
{
    am_sourceID_t sourceID = 12;

    //When we run this test, we expect the call on the control interface
    EXPECT_CALL(pMockControlInterface,hookSystemDeregisterSource(12)).WillRepeatedly(Return(E_OK));
    ASSERT_EQ(E_OK, pRoutingReceiver.deregisterSource(sourceID));
}

TEST_F(CAmControlInterfaceTest,hookInterruptStatusChange)
{
    am_Source_s source;
    am_sourceID_t sourceID;
    pCF.createSource(source);
    source.sourceID=12;
    source.interruptState = IS_UNKNOWN;
    sourceID = source.sourceID;

    //prepare the stage
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));

    //When we run this test, we expect the call on the control interface
    EXPECT_CALL(pMockControlInterface,hookSystemInterruptStateChange(sourceID, IS_OFF));
    pRoutingReceiver.hookInterruptStatusChange(sourceID, IS_OFF);
}

TEST_F(CAmControlInterfaceTest,registerGateway)
{
    am_Gateway_s gateway;
    am_gatewayID_t gatewayID;
    pCF.createGateway(gateway);

    //When we run this test, we expect the call on the control interface
    EXPECT_CALL(pMockControlInterface,hookSystemRegisterGateway(_,_)).WillRepeatedly(DoAll(SetArgReferee<1>(2), Return(E_OK)));
    ASSERT_EQ(E_OK, pRoutingReceiver.registerGateway(gateway,gatewayID));
    ASSERT_EQ(gatewayID, 2);
}

TEST_F(CAmControlInterfaceTest,deregisterGateway)
{
    am_gatewayID_t gatewayID = 12;

    //When we run this test, we expect the call on the control interface
    EXPECT_CALL(pMockControlInterface,hookSystemDeregisterGateway(12)).WillRepeatedly(Return(E_OK));
    ASSERT_EQ(E_OK, pRoutingReceiver.deregisterGateway(gatewayID));
}

TEST_F(CAmControlInterfaceTest,ackConnect)
{
    am_connectionID_t connectionID;
    am_Sink_s sink;
    am_sinkID_t sinkID;
    am_Source_s source;
    am_sourceID_t sourceID;
    am_Domain_s domain;
    am_domainID_t domainID;
    std::vector<am_Connection_s> connectionList;
    std::vector<am_Handle_s> handlesList;
    am_Handle_s handle;
    pCF.createSink(sink);
    pCF.createDomain(domain);
    pCF.createSource(source);
    domain.name = "mock";
    domain.busname = "mock";
    sink.sinkID = 2;
    source.sourceID=2;

    //prepare the stage
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));

    //when asyncConnect is called, we expect a call on the routingInterface
    EXPECT_CALL(pMockRoutingInterface,asyncConnect(_,1,2,2,CF_GENIVI_STEREO)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK, pControlReceiver.connect(handle,connectionID,CF_GENIVI_STEREO,2,2));

    //The handle should have the correct type
    ASSERT_EQ(handle.handleType, H_CONNECT);
    ASSERT_EQ(handle.handle, 1);
    ASSERT_EQ(connectionID, 1);

    //The list of handles shall have the handle inside
    ASSERT_EQ(E_OK, pControlReceiver.getListHandles(handlesList));
    ASSERT_EQ(handlesList[0].handle, handle.handle);
    ASSERT_EQ(handlesList[0].handleType, handle.handleType);

    //we check the list of connections - but it must be empty because the ack did not arrive yet
    ASSERT_EQ(E_OK, pDatabaseHandler.getListConnections(connectionList));
    ASSERT_TRUE(connectionList.empty());

    //finally we answer via the RoutingInterface and expect a call on the controlInterface
    EXPECT_CALL(pMockControlInterface,cbAckConnect(_,E_OK)).Times(1);
    pRoutingReceiver.ackConnect(handle, connectionID, E_OK);

    //the list of handles must be empty now
    ASSERT_EQ(E_OK, pControlReceiver.getListHandles(handlesList));
    ASSERT_TRUE(handlesList.empty());

    //but the connection must be in the connectionlist
    ASSERT_EQ(E_OK, pDatabaseHandler.getListConnections(connectionList));
    ASSERT_TRUE(!connectionList.empty());

    //no we try the same, but do expect a no_change answer directly and no call because connection already exists
    //ASSERT_EQ(E_ALREADY_EXISTS, pControlReceiver.connect(handle,connectionID,CF_GENIVI_STEREO,2,2));
    //needed to be removed because logic changed here
}

TEST_F(CAmControlInterfaceTest,ackDisconnect)
{
    am_connectionID_t connectionID;
    am_Sink_s sink;
    am_sinkID_t sinkID;
    am_Domain_s domain;
    am_domainID_t domainID;
    am_Source_s source;
    am_sourceID_t sourceID;
    std::vector<am_Connection_s> connectionList;
    std::vector<am_Handle_s> handlesList;
    am_Handle_s handle;
    pCF.createSink(sink);
    pCF.createDomain(domain);
    pCF.createSource(source);
    domain.domainID=0;
    domain.name = "mock";
    domain.busname = "mock";
    sink.sinkID = 2;
    sink.domainID = DYNAMIC_ID_BOUNDARY;
    source.sourceID=2;

    //prepare the stage
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));

    //now we first need to connect, we expect a call on the routing interface
    EXPECT_CALL(pMockRoutingInterface,asyncConnect(_,1,2,2,CF_GENIVI_STEREO)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK, pControlReceiver.connect(handle,connectionID,CF_GENIVI_STEREO,2,2));

    //answer with an ack to insert the connection in the database
    EXPECT_CALL(pMockControlInterface,cbAckConnect(_,E_OK)).Times(1);
    pRoutingReceiver.ackConnect(handle, connectionID, E_OK);

    //now we can start to disconnect and expect a call on the routing interface
    EXPECT_CALL(pMockRoutingInterface,asyncDisconnect(_,1)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK, pControlReceiver.disconnect(handle,1));

    //during the disconnection, the connection is still in the list!
    ASSERT_EQ(E_OK, pDatabaseHandler.getListConnections(connectionList));
    ASSERT_TRUE(!connectionList.empty());

    //then we fire the ack and expect a call on the controlInterface
    EXPECT_CALL(pMockControlInterface,cbAckDisconnect(_,E_OK)).Times(1);
    pRoutingReceiver.ackDisconnect(handle, connectionID, E_OK);

    //make sure the handle is gone
    ASSERT_EQ(E_OK, pControlReceiver.getListHandles(handlesList));
    ASSERT_TRUE(handlesList.empty());

    //make sure the connection is gone
    ASSERT_EQ(E_OK, pDatabaseHandler.getListConnections(connectionList));
    ASSERT_TRUE(connectionList.empty());

    //Now let's try to disconnect what is not existing...
    ASSERT_EQ(E_NON_EXISTENT, pControlReceiver.disconnect(handle,2));
}

TEST_F(CAmControlInterfaceTest,setSourceState)
{

    am_Source_s source;
    am_sourceID_t sourceID;
    am_Domain_s domain;
    am_domainID_t domainID;
    std::vector<am_Handle_s> handlesList;
    am_Handle_s handle;
    am_SourceState_e state;
    pCF.createSource(source);
    pCF.createDomain(domain);
    domain.name = "mock";
    domain.busname = "mock";
    domain.domainID=0;
    source.sourceID = 2;
    source.domainID = DYNAMIC_ID_BOUNDARY;

    //prepare the stage
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));

    //we set the sourcestate and expect a call on the routingInterface
    EXPECT_CALL(pMockRoutingInterface,asyncSetSourceState(_,2,SS_PAUSED)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK, pControlReceiver.setSourceState(handle,source.sourceID,SS_PAUSED));

    //we want our handle in the list and let the type be the right one
    ASSERT_EQ(E_OK, pControlReceiver.getListHandles(handlesList));
    ASSERT_EQ(handlesList[0].handle, handle.handle);
    ASSERT_EQ(handlesList[0].handleType, H_SETSOURCESTATE);

    //the state must be unchanged because did not get the ack
    ASSERT_EQ(E_OK, pDatabaseHandler.getSoureState(source.sourceID,state));
    ASSERT_EQ(state, SS_ON);

    //now we sent out the ack and expect a call on the controlInterface
    EXPECT_CALL(pMockControlInterface,cbAckSetSourceState(_,E_OK)).Times(1);
    pRoutingReceiver.ackSetSourceState(handle, E_OK);

    //finally we need the sourcestate to be changed
    ASSERT_EQ(E_OK, pDatabaseHandler.getSoureState(source.sourceID,state));
    ASSERT_EQ(state, SS_PAUSED);

    //make sure the handle is gone
    ASSERT_EQ(E_OK, pControlReceiver.getListHandles(handlesList));
    ASSERT_TRUE(handlesList.empty());

    //we try again but expect a no change error
    //ASSERT_EQ(E_NO_CHANGE, pControlReceiver.setSourceState(handle,source.sourceID,SS_PAUSED));
    //needed to be removed because logic changed here
}

TEST_F(CAmControlInterfaceTest,SetSinkVolumeChange)
{
    am_Sink_s sink;
    am_sinkID_t sinkID;
    am_Domain_s domain;
    am_domainID_t domainID;
    am_volume_t volume;
    std::vector<am_Handle_s> handlesList;
    am_Handle_s handle;
    pCF.createSink(sink);
    pCF.createDomain(domain);
    domain.name = "mock";
    domain.busname = "mock";
    domain.domainID=0;
    sink.sinkID = 2;
    sink.domainID = DYNAMIC_ID_BOUNDARY;
    sink.volume = 10;

    //setup environment, we need a domain and a sink
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));

    //set the volume and expect a call on the routing interface
    EXPECT_CALL(pMockRoutingInterface,asyncSetSinkVolume(_,2,11,RAMP_GENIVI_DIRECT,23)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK, pControlReceiver.setSinkVolume(handle,sinkID,11,RAMP_GENIVI_DIRECT,23));

    //check the list of handles. The handle must be in there and have the right type
    ASSERT_EQ(E_OK, pControlReceiver.getListHandles(handlesList));
    ASSERT_EQ(handlesList[0].handle, handle.handle);
    ASSERT_EQ(handlesList[0].handleType, H_SETSINKVOLUME);

    //now we read out the volume, but we expect no change because the ack did not arrive yet
    ASSERT_EQ(E_OK, pDatabaseHandler.getSinkVolume(sinkID,volume));
    ASSERT_EQ(sink.volume, volume);

    //lets send the answer and expect a call on the controlInterface
    EXPECT_CALL(pMockControlInterface,cbAckSetSinkVolumeChange(_,11,E_OK)).Times(1);
    pRoutingReceiver.ackSetSinkVolumeChange(handle, 11, E_OK);

    //finally, the new value must be in the database
    ASSERT_EQ(E_OK, pDatabaseHandler.getSinkVolume(sinkID,volume));
    ASSERT_EQ(11, volume);

    //and the handle must be destroyed
    ASSERT_EQ(E_OK, pControlReceiver.getListHandles(handlesList));
    ASSERT_TRUE(handlesList.empty());

    //Now we try again, but the value is unchanged
    //ASSERT_EQ(E_NO_CHANGE, pControlReceiver.setSinkVolume(handle,sinkID,11,RAMP_GENIVI_DIRECT,23));
    //needed to be removed because logic changed here
}

TEST_F(CAmControlInterfaceTest,ackSetSourceVolumeChange)
{
    am_Source_s source;
    am_sourceID_t sourceID;
    am_Domain_s domain;
    am_domainID_t domainID;
    am_volume_t volume;
    std::vector<am_Handle_s> handlesList;
    am_Handle_s handle;
    pCF.createSource(source);
    pCF.createDomain(domain);
    domain.name = "mock";
    domain.busname = "mock";
    domain.domainID=0;
    source.sourceID = 2;
    source.domainID = DYNAMIC_ID_BOUNDARY;
    source.volume = 12;

    //prepare the scene
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));

    //change the sinkVolume, expect a call on the routingInterface
    EXPECT_CALL(pMockRoutingInterface,asyncSetSourceVolume(_,2,11,RAMP_GENIVI_DIRECT,23)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK, pControlReceiver.setSourceVolume(handle,source.sourceID,11,RAMP_GENIVI_DIRECT,23));

    //check the list of handles. The handle must be in there and have the right type
    ASSERT_EQ(E_OK, pControlReceiver.getListHandles(handlesList));
    ASSERT_EQ(handlesList[0].handle, handle.handle);
    ASSERT_EQ(handlesList[0].handleType, H_SETSOURCEVOLUME);

    //now we read out the volume, but we expect no change because the ack did not arrive yet
    ASSERT_EQ(E_OK, pDatabaseHandler.getSourceVolume(sourceID,volume));
    ASSERT_EQ(source.volume, volume);

    //lets send the answer and expect a call on the controlInterface
    EXPECT_CALL(pMockControlInterface,cbAckSetSourceVolumeChange(_,11,E_OK)).Times(1);
    pRoutingReceiver.ackSetSourceVolumeChange(handle, 11, E_OK);

    //finally, the new value must be in the database
    ASSERT_EQ(E_OK, pDatabaseHandler.getSourceVolume(sourceID,volume));
    ASSERT_EQ(11, volume);

    //and the handle must be destroyed
    ASSERT_EQ(E_OK, pControlReceiver.getListHandles(handlesList));
    ASSERT_TRUE(handlesList.empty());

    //Now we try again, but the value is unchanged
    //ASSERT_EQ(E_NO_CHANGE, pControlReceiver.setSourceVolume(handle,source.sourceID,11,RAMP_GENIVI_DIRECT,23));
    //needed to be removed because logic changed here
}

TEST_F(CAmControlInterfaceTest,ackSetSinkSoundProperty)
{
    am_Sink_s sink;
    am_sinkID_t sinkID;
    am_Domain_s domain;
    am_domainID_t domainID;
    std::vector<am_Handle_s> handlesList;
    am_Handle_s handle;
    am_SoundProperty_s soundProperty;
    int16_t oldvalue;
    pCF.createSink(sink);
    pCF.createDomain(domain);
    domain.name = "mock";
    domain.busname = "mock";
    domain.domainID=0;
    sink.sinkID = 2;
    sink.domainID = DYNAMIC_ID_BOUNDARY;
    soundProperty.type = SP_GENIVI_BASS;
    soundProperty.value = 244;

    //setup environment, we need a domain and a sink
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));

    //change the soundproperty, expect a call on the routinginterface
    EXPECT_CALL(pMockRoutingInterface,asyncSetSinkSoundProperty(_,2,_)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK, pControlReceiver.setSinkSoundProperty(handle,sink.sinkID,soundProperty));

    //check the list of handles. The handle must be in there and have the right type
    ASSERT_EQ(E_OK, pControlReceiver.getListHandles(handlesList));
    ASSERT_EQ(handlesList[0].handle, handle.handle);
    ASSERT_EQ(handlesList[0].handleType, H_SETSINKSOUNDPROPERTY);

    //read out this property. There is no change, because the ack did not arrive yet.
    ASSERT_EQ(E_OK, pDatabaseHandler.getSinkSoundPropertyValue(2,SP_GENIVI_BASS,oldvalue));
    ASSERT_EQ(sink.listSoundProperties[0].value, oldvalue);

    //lets send the answer and expect a call on the controlInterface
    EXPECT_CALL(pMockControlInterface,cbAckSetSinkSoundProperty(_,E_OK)).Times(1);
    pRoutingReceiver.ackSetSinkSoundProperty(handle, E_OK);

    //finally, the new value must be in the database
    ASSERT_EQ(E_OK, pDatabaseHandler.getSinkSoundPropertyValue(sinkID,SP_GENIVI_BASS,oldvalue));
    ASSERT_EQ(soundProperty.value, oldvalue);

    //and the handle must be destroyed
    ASSERT_EQ(E_OK, pControlReceiver.getListHandles(handlesList));
    ASSERT_TRUE(handlesList.empty());

    //Now we try again, but the value is unchanged
    //ASSERT_EQ(E_NO_CHANGE, pControlReceiver.setSinkSoundProperty(handle,sink.sinkID,soundProperty));
    //needed to be removed because logic changed here
}

TEST_F(CAmControlInterfaceTest,ackSetSourceSoundProperty)
{
    am_Source_s source;
    am_sourceID_t sourceID;
    am_Domain_s domain;
    am_domainID_t domainID;
    std::vector<am_Handle_s> handlesList;
    am_Handle_s handle;
    am_SoundProperty_s soundProperty;
    int16_t oldvalue;
    pCF.createSource(source);
    pCF.createDomain(domain);
    domain.name = "mock";
    domain.busname = "mock";
    domain.domainID=0;
    source.sourceID = 2;
    source.domainID = DYNAMIC_ID_BOUNDARY;
    soundProperty.type = SP_GENIVI_BASS;
    soundProperty.value = 244;

    //prepare the scene
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));

    //we trigger the change and wait for a call on the routinginterface
    EXPECT_CALL(pMockRoutingInterface,asyncSetSourceSoundProperty(_,2,_)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK, pControlReceiver.setSourceSoundProperty(handle,source.sourceID,soundProperty));

    //check the list of handles. The handle must be in there and have the right type
    ASSERT_EQ(E_OK, pControlReceiver.getListHandles(handlesList));
    ASSERT_EQ(handlesList[0].handle, handle.handle);
    ASSERT_EQ(handlesList[0].handleType, H_SETSOURCESOUNDPROPERTY);

    //read out this property. There is no change, because the ack did not arrive yet.
    ASSERT_EQ(E_OK, pDatabaseHandler.getSourceSoundPropertyValue(2,SP_GENIVI_BASS,oldvalue));
    ASSERT_EQ(source.listSoundProperties[0].value, oldvalue);

    //lets send the answer and expect a call on the controlInterface
    EXPECT_CALL(pMockControlInterface,cbAckSetSourceSoundProperty(_,E_OK)).Times(1);
    pRoutingReceiver.ackSetSourceSoundProperty(handle, E_OK);

    //finally, the new value must be in the database
    ASSERT_EQ(E_OK, pDatabaseHandler.getSourceSoundPropertyValue(sourceID,SP_GENIVI_BASS,oldvalue));
    ASSERT_EQ(soundProperty.value, oldvalue);

    //and the handle must be destroyed
    ASSERT_EQ(E_OK, pControlReceiver.getListHandles(handlesList));
    ASSERT_TRUE(handlesList.empty());

    //Now we try again, but the value is unchanged
    //ASSERT_EQ(E_NO_CHANGE, pControlReceiver.setSourceSoundProperty(handle,source.sourceID,soundProperty));
    //needed to be removed because logic changed here
}

TEST_F(CAmControlInterfaceTest,crossFading)
{
    //todo: implement crossfading test
}

TEST_F(CAmControlInterfaceTest,resyncConnectionsTest)
{
    am_Domain_s domain;
    am_domainID_t domainID;
    pCF.createDomain(domain);
    domain.name = "mock";
    domain.busname = "mock";

    //prepare the scene
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));

	std::vector<am_Connection_s> listConnections;

    EXPECT_CALL(pMockRoutingInterface,resyncConnectionState(domainID,_)).WillOnce(DoAll(returnResyncConnection(), Return(E_OK)));
    ASSERT_EQ(am_Error_e::E_OK,pControlReceiver.resyncConnectionState(domainID,listConnections));
    ASSERT_EQ(listConnections[0].sinkID,1);
    ASSERT_EQ(listConnections[0].sourceID,1);
    ASSERT_EQ(listConnections[0].connectionFormat,CF_GENIVI_ANALOG);
}

TEST_F(CAmControlInterfaceTest,ackConnectNotPossible)
{
    am_connectionID_t connectionID;
    am_Sink_s sink;
    am_sinkID_t sinkID;
    am_Domain_s domain;
    am_domainID_t domainID;
    am_Source_s source;
    am_sourceClass_t sourceID;
    std::vector<am_Connection_s> connectionList;
    std::vector<am_Handle_s> handlesList;
    am_Handle_s handle;
    pCF.createSink(sink);
    pCF.createDomain(domain);
    pCF.createSource(source);

    domain.name = "mock";
    domain.busname = "mock";
    domain.domainID=0;
    sink.sinkID = 2;
    sink.domainID = DYNAMIC_ID_BOUNDARY;
    source.sourceID=2;

    //prepare the stage
    ASSERT_EQ(E_OK, pDatabaseHandler.enterDomainDB(domain,domainID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSinkDB(sink,sinkID));
    ASSERT_EQ(E_OK, pDatabaseHandler.enterSourceDB(source,sourceID));

    //when asyncConnect is called, we expect a call on the routingInterface
    EXPECT_CALL(pMockRoutingInterface,asyncConnect(_,1,2,2,CF_GENIVI_STEREO)).WillOnce(Return(E_COMMUNICATION));
    ASSERT_EQ(E_COMMUNICATION, pControlReceiver.connect(handle,connectionID,CF_GENIVI_STEREO,2,2));

    //The list of handles shall be empty
    ASSERT_EQ(E_OK, pControlReceiver.getListHandles(handlesList));
    ASSERT_TRUE(handlesList.empty());


    ASSERT_EQ(E_OK, pDatabaseHandler.getListConnections(connectionList));
    ASSERT_TRUE(connectionList.empty());

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
	CAmDltWrapper::instanctiateOnce("ATEST","AudioManagerControlInterface Test",enableDebug.getValue(),CAmDltWrapper::logDestination::DAEMON);
	logInfo("RoutingSendInterface Test started");
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

