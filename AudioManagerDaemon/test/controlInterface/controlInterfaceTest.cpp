/*
 * routingInterfaceTest.cpp
 *
 *  Created on: Dec 10, 2011
 *      Author: christian
 */

#include "controlInterfaceTest.h"



controlInterfaceTest::controlInterfaceTest()
	:plistCommandPluginDirs(),
	 plistRoutingPluginDirs(),
	 pDatabaseHandler(std::string(":memory:")),
	 pRoutingSender(plistRoutingPluginDirs),
	 pCommandSender(plistCommandPluginDirs),
	 pMockControlInterface(),
	 pMockRoutingInterface(),
	 pControlSender(std::string("")),
	 pRoutingInterfaceBackdoor(),
	 pCommandInterfaceBackdoor(),
	 pControlInterfaceBackdoor(),
	 pDatabaseObserver(&pCommandSender,&pRoutingSender),
	 pControlReceiver(&pDatabaseHandler,&pRoutingSender,&pCommandSender),
	 pRoutingReceiver(&pDatabaseHandler,&pRoutingSender,&pControlSender)
{
	pDatabaseHandler.registerObserver(&pDatabaseObserver);
	pRoutingInterfaceBackdoor.unloadPlugins(&pRoutingSender);
	pCommandInterfaceBackdoor.unloadPlugins(&pCommandSender);
	pControlInterfaceBackdoor.replaceController(&pControlSender,&pMockControlInterface);
	pRoutingInterfaceBackdoor.injectInterface(&pRoutingSender,&pMockRoutingInterface,"mock");

}

controlInterfaceTest::~controlInterfaceTest()
{
}

void controlInterfaceTest::SetUp()
{
	DLT_REGISTER_APP("Rtest","RoutingInterfacetest");
	DLT_REGISTER_CONTEXT(AudioManager,"Main","Main Context");
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("RoutingSendInterface Test started "));

}

void controlInterfaceTest::TearDown()
{
	DLT_UNREGISTER_CONTEXT(AudioManager);
}

TEST_F(controlInterfaceTest,registerDomain)
{

	am_Domain_s domain;
	am_domainID_t domainID;
	pCF.createDomain(domain);
	EXPECT_CALL(pMockControlInterface,hookSystemRegisterDomain(_,_)).WillRepeatedly(DoAll(SetArgReferee<1>(2),Return(E_OK)));
	ASSERT_EQ(E_OK,pRoutingReceiver.registerDomain(domain,domainID));
	ASSERT_EQ(domainID,2);
}

TEST_F(controlInterfaceTest,deregisterDomain)
{
	am_domainID_t domainID=34;
	EXPECT_CALL(pMockControlInterface,hookSystemDeregisterDomain(34)).WillRepeatedly(Return(E_OK));
	ASSERT_EQ(E_OK,pRoutingReceiver.deregisterDomain(domainID));
}

TEST_F(controlInterfaceTest,registerSink)
{
	am_Sink_s sink;
	am_sinkID_t sinkID;
	pCF.createSink(sink);
	EXPECT_CALL(pMockControlInterface,hookSystemRegisterSink(_,_)).WillRepeatedly(DoAll(SetArgReferee<1>(2),Return(E_OK)));
	ASSERT_EQ(E_OK,pRoutingReceiver.registerSink(sink,sinkID));
	ASSERT_EQ(sinkID,2);
}

TEST_F(controlInterfaceTest,deregisterSink)
{
	am_sinkID_t sinkID=12;
	EXPECT_CALL(pMockControlInterface,hookSystemDeregisterSink(12)).WillRepeatedly(Return(E_OK));
	ASSERT_EQ(E_OK,pRoutingReceiver.deregisterSink(sinkID));
}

TEST_F(controlInterfaceTest,registerSource)
{
	am_Source_s source;
	am_sourceID_t sourceID;
	pCF.createSource(source);
	EXPECT_CALL(pMockControlInterface,hookSystemRegisterSource(_,_)).WillRepeatedly(DoAll(SetArgReferee<1>(2),Return(E_OK)));
	ASSERT_EQ(E_OK,pRoutingReceiver.registerSource(source,sourceID));
	ASSERT_EQ(sourceID,2);
}

TEST_F(controlInterfaceTest,deregisterSource)
{
	am_sourceID_t sourceID=12;
	EXPECT_CALL(pMockControlInterface,hookSystemDeregisterSource(12)).WillRepeatedly(Return(E_OK));
	ASSERT_EQ(E_OK,pRoutingReceiver.deregisterSource(sourceID));
}

TEST_F(controlInterfaceTest,registerGateway)
{
	am_Gateway_s gateway;
	am_gatewayID_t gatewayID;
	pCF.createGateway(gateway);
	EXPECT_CALL(pMockControlInterface,hookSystemRegisterGateway(_,_)).WillRepeatedly(DoAll(SetArgReferee<1>(2),Return(E_OK)));
	ASSERT_EQ(E_OK,pRoutingReceiver.registerGateway(gateway,gatewayID));
	ASSERT_EQ(gatewayID,2);
}

TEST_F(controlInterfaceTest,deregisterGateway)
{
	am_gatewayID_t gatewayID=12;
	EXPECT_CALL(pMockControlInterface,hookSystemDeregisterGateway(12)).WillRepeatedly(Return(E_OK));
	ASSERT_EQ(E_OK,pRoutingReceiver.deregisterGateway(gatewayID));
}


TEST_F(controlInterfaceTest,ackConnect)
{
	am_Connection_s connection;
	am_connectionID_t connectionID;
	am_Sink_s sink;
	am_sinkID_t sinkID;
	am_Domain_s domain;
	am_domainID_t domainID;
	std::vector<am_Connection_s> connectionList;
	std::vector<am_Handle_s> handlesList;
	am_Handle_s handle;
	pCF.createSink(sink);
	pCF.createDomain(domain);
	domain.name="mock";
	domain.busname="mock";
	sink.sinkID=2;
	sink.domainID=1;
	ASSERT_EQ(E_OK,pDatabaseHandler.enterDomainDB(domain,domainID));
	ASSERT_EQ(E_OK,pDatabaseHandler.enterSinkDB(sink,sinkID));
	EXPECT_CALL(pMockRoutingInterface,asyncConnect(_,1,2,2,CF_STEREO)).WillOnce(Return(E_OK));
	ASSERT_EQ(E_OK,pControlReceiver.connect(handle,connectionID,CF_STEREO,2,2));
	ASSERT_EQ(handle.handleType,H_CONNECT);
	ASSERT_EQ(handle.handle,1);
	ASSERT_EQ(connectionID,1);
	ASSERT_EQ(E_OK,pControlReceiver.getListHandles(handlesList));
	ASSERT_EQ(handlesList[0].handle,handle.handle);
	ASSERT_EQ(handlesList[0].handleType,handle.handleType);
	ASSERT_EQ(E_OK,pDatabaseHandler.getListConnections(connectionList));
	ASSERT_TRUE(connectionList.empty()); //ok, because Ack did not come
	EXPECT_CALL(pMockControlInterface,cbAckConnect(_,E_OK)).Times(1);
	pRoutingReceiver.ackConnect(handle,connectionID,E_OK);
	ASSERT_EQ(E_OK,pControlReceiver.getListHandles(handlesList));
	ASSERT_TRUE(handlesList.empty());
	ASSERT_EQ(E_OK,pDatabaseHandler.getListConnections(connectionList));
	ASSERT_TRUE(!connectionList.empty()); //ok, because Ack did not come

}

TEST_F(controlInterfaceTest,ackDisconnect)
{
	am_Connection_s connection;
	am_connectionID_t connectionID;
	am_Sink_s sink;
	am_sinkID_t sinkID;
	am_Domain_s domain;
	am_domainID_t domainID;
	std::vector<am_Connection_s> connectionList;
	std::vector<am_Handle_s> handlesList;
	am_Handle_s handle;
	pCF.createSink(sink);
	pCF.createDomain(domain);
	domain.name="mock";
	domain.busname="mock";
	sink.sinkID=2;
	sink.domainID=1;
	ASSERT_EQ(E_OK,pDatabaseHandler.enterDomainDB(domain,domainID));
	ASSERT_EQ(E_OK,pDatabaseHandler.enterSinkDB(sink,sinkID));
	EXPECT_CALL(pMockRoutingInterface,asyncConnect(_,1,2,2,CF_STEREO)).WillOnce(Return(E_OK));
	ASSERT_EQ(E_OK,pControlReceiver.connect(handle,connectionID,CF_STEREO,2,2));
	EXPECT_CALL(pMockControlInterface,cbAckConnect(_,E_OK)).Times(1);
	pRoutingReceiver.ackConnect(handle,connectionID,E_OK);
	EXPECT_CALL(pMockRoutingInterface,asyncDisconnect(_,1)).WillOnce(Return(E_OK));
	ASSERT_EQ(E_OK,pControlReceiver.disconnect(handle,1));
	EXPECT_CALL(pMockControlInterface,cbAckDisconnect(_,E_OK)).Times(1);
	pRoutingReceiver.ackDisconnect(handle,connectionID,E_OK);
	ASSERT_EQ(E_OK,pControlReceiver.getListHandles(handlesList));
	ASSERT_TRUE(handlesList.empty());
	ASSERT_EQ(E_OK,pDatabaseHandler.getListConnections(connectionList));
	ASSERT_TRUE(connectionList.empty()); //ok, because Ack did not come
}

TEST_F(controlInterfaceTest,setSourceState)
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
	domain.name="mock";
	domain.busname="mock";
	source.sourceID=2;
	source.domainID=1;
	ASSERT_EQ(E_OK,pDatabaseHandler.enterDomainDB(domain,domainID));
	ASSERT_EQ(E_OK,pDatabaseHandler.enterSourceDB(source,sourceID));
	EXPECT_CALL(pMockRoutingInterface,asyncSetSourceState(_,2,SS_PAUSED)).WillOnce(Return(E_OK));
	ASSERT_EQ(E_OK,pControlReceiver.setSourceState(handle,source.sourceID,SS_PAUSED));
	ASSERT_EQ(E_OK,pControlReceiver.getListHandles(handlesList));
	ASSERT_EQ(handlesList[0].handle,handle.handle);
	ASSERT_EQ(handlesList[0].handleType,handle.handleType);
	ASSERT_EQ(E_OK,pDatabaseHandler.getSoureState(source.sourceID,state));
	ASSERT_EQ(state,SS_ON); //ok, since value will be added after the ack!
	EXPECT_CALL(pMockControlInterface,cbAckSetSourceState(_,E_OK)).Times(1);
	pRoutingReceiver.ackSetSourceState(handle,E_OK);
	ASSERT_EQ(E_OK,pDatabaseHandler.getSoureState(source.sourceID,state));
	ASSERT_EQ(state,SS_PAUSED);

}


int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

