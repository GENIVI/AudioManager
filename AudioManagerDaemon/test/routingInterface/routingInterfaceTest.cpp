/*
 * routingInterfaceTest.cpp
 *
 *  Created on: Dec 10, 2011
 *      Author: christian
 */

#include "routingInterfaceTest.h"

using ::testing::Field;
using ::testing::Property;
using ::testing::Matcher;
using ::testing::Pointee;
using ::testing::AllOf;
using ::testing::SafeMatcherCast;
using ::testing::MatcherCast;
using ::testing::DefaultValue;
using ::testing::Eq;
using ::testing::An;
using ::testing::ElementsAreArray;
using ::testing::ElementsAre;
using ::testing::NotNull;

routingInterfaceTest::routingInterfaceTest()
	:plistCommandPluginDirs(),
	 pDatabaseHandler(std::string(":memory:")),
	 pRoutingSender(plistRoutingPluginDirs),
	 pCommandSender(plistCommandPluginDirs),
	 pMockInterface(),
	 pRoutingInterfaceBackdoor(),
	 pCommandInterfaceBackdoor(),
	 pControlReceiver(&pDatabaseHandler,&pRoutingSender,&pCommandSender),
	 pObserver(&pCommandSender,&pRoutingSender)
{
	pDatabaseHandler.registerObserver(&pObserver);
	pRoutingInterfaceBackdoor.unloadPlugins(&pRoutingSender);
	pRoutingInterfaceBackdoor.injectInterface(&pRoutingSender,&pMockInterface,"mock");
	pCommandInterfaceBackdoor.unloadPlugins(&pCommandSender);
}

routingInterfaceTest::~routingInterfaceTest()
{
}

void routingInterfaceTest::SetUp()
{
	DLT_REGISTER_APP("Rtest","RoutingInterfacetest");
	DLT_REGISTER_CONTEXT(AudioManager,"Main","Main Context");
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("RoutingSendInterface Test started "));

}

void routingInterfaceTest::TearDown()
{
	DLT_UNREGISTER_CONTEXT(AudioManager);
}

TEST_F(routingInterfaceTest,abort)
{
	am_Sink_s sink;
	am_sinkID_t sinkID;
	am_Domain_s domain;
	am_domainID_t domainID;
    am_Handle_s handle;
    am_connectionID_t connectionID;
    std::vector<am_Handle_s>listHandles;
	pCF.createSink(sink);
	pCF.createDomain(domain);
	domain.name="mock";
	domain.busname="mock";
	sink.sinkID=2;
	sink.domainID=1;
	ASSERT_EQ(E_OK,pDatabaseHandler.enterDomainDB(domain,domainID));
	ASSERT_EQ(E_OK,pDatabaseHandler.enterSinkDB(sink,sinkID));
    EXPECT_CALL(pMockInterface,asyncConnect(_,_,1,sinkID,CF_ANALOG)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK,pControlReceiver.connect(handle,connectionID,CF_ANALOG,1,2));
    ASSERT_NE(handle.handle,0);
    ASSERT_EQ(handle.handleType,H_CONNECT);
    ASSERT_EQ(E_OK,pControlReceiver.getListHandles(listHandles));
    ASSERT_TRUE(listHandles[0].handle==handle.handle);
    ASSERT_TRUE(listHandles[0].handleType==handle.handleType);
    EXPECT_CALL(pMockInterface,asyncAbort(_)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK,pControlReceiver.abortAction(handle));
}

TEST_F(routingInterfaceTest,abortNonExistent)
{
    EXPECT_CALL(pMockInterface,asyncAbort(_)).Times(0);
    am_Handle_s handle;
    ASSERT_EQ(E_NON_EXISTENT,pControlReceiver.abortAction(handle));
}

TEST_F(routingInterfaceTest,alreadyConnected)
{
	am_Sink_s sink;
	am_sinkID_t sinkID;
	am_Domain_s domain;
	am_domainID_t domainID;
	pCF.createSink(sink);
	pCF.createDomain(domain);
	domain.name="mock";
	domain.busname="mock";
	sink.sinkID=2;
	sink.domainID=1;
	ASSERT_EQ(E_OK,pDatabaseHandler.enterDomainDB(domain,domainID));
	ASSERT_EQ(E_OK,pDatabaseHandler.enterSinkDB(sink,sinkID));
    EXPECT_CALL(pMockInterface,asyncConnect(_,_,1,sinkID,CF_ANALOG)).WillOnce(Return(E_OK));
    am_Handle_s handle;
    am_connectionID_t connectionID;
    ASSERT_EQ(E_OK,pControlReceiver.connect(handle,connectionID,CF_ANALOG,1,2));
    ASSERT_EQ(E_OK,pDatabaseHandler.changeConnectionFinal(connectionID));
    ASSERT_EQ(E_ALREADY_EXISTS,pControlReceiver.connect(handle,connectionID,CF_ANALOG,1,2));
    ASSERT_NE(handle.handle,0);
    ASSERT_EQ(handle.handleType,H_CONNECT);
}

TEST_F(routingInterfaceTest,setSinkSoundPropertyNoChange)
{
	am_Sink_s sink;
	am_sinkID_t sinkID;
	am_Domain_s domain;
	am_domainID_t domainID;
    am_Handle_s handle;
    am_SoundProperty_s soundProperty;
    soundProperty.type=SP_TREBLE;
    soundProperty.value=23;
    std::vector<am_Handle_s>listHandles;
	pCF.createSink(sink);
	pCF.createDomain(domain);
	domain.name="mock";
	domain.busname="mock";
	sink.sinkID=2;
	sink.domainID=1;
	sink.listSoundProperties.push_back(soundProperty);
	ASSERT_EQ(E_OK,pDatabaseHandler.enterDomainDB(domain,domainID));
	ASSERT_EQ(E_OK,pDatabaseHandler.enterSinkDB(sink,sinkID));
    EXPECT_CALL(pMockInterface,asyncSetSinkSoundProperty(_,_,sinkID)).Times(0);
    ASSERT_EQ(E_NO_CHANGE,pControlReceiver.setSinkSoundProperty(handle,sinkID,soundProperty));
}

TEST_F(routingInterfaceTest,setSourceState)
{
	am_Source_s source;
	am_sourceID_t sourceID;
	am_Domain_s domain;
	am_domainID_t domainID;
	am_Handle_s handle;
	handle.handle=0;
	am_SourceState_e state=SS_PAUSED;
	pCF.createSource(source);
	pCF.createDomain(domain);
	domain.name="mock";
	domain.busname="mock";
	source.domainID=1;
	ASSERT_EQ(E_OK,pDatabaseHandler.enterDomainDB(domain,domainID));
	ASSERT_EQ(E_OK,pDatabaseHandler.enterSourceDB(source,sourceID));
	EXPECT_CALL(pMockInterface,asyncSetSourceState(_,sourceID,state)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK,pControlReceiver.setSourceState(handle,sourceID,state));
    ASSERT_NE(handle.handle,0);
    ASSERT_EQ(handle.handleType,H_SETSOURCESTATE);
}

TEST_F(routingInterfaceTest,setSourceSoundProperty)
{
	am_Source_s source;
	am_sourceID_t sourceID;
	am_Domain_s domain;
	am_domainID_t domainID;
    am_Handle_s handle;
    am_SoundProperty_s soundProperty;
    std::vector<am_Handle_s>listHandles;
	pCF.createSource(source);
	pCF.createDomain(domain);
	domain.name="mock";
	domain.busname="mock";
	source.sourceID=2;
	source.domainID=1;
	ASSERT_EQ(E_OK,pDatabaseHandler.enterDomainDB(domain,domainID));
	ASSERT_EQ(E_OK,pDatabaseHandler.enterSourceDB(source,sourceID));
    EXPECT_CALL(pMockInterface,asyncSetSourceSoundProperty(_,_,sourceID)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK,pControlReceiver.setSourceSoundProperty(handle,sourceID,soundProperty));
    ASSERT_NE(handle.handle,0);
    ASSERT_EQ(handle.handleType,H_SETSOURCESOUNDPROPERTY);
    ASSERT_EQ(E_OK,pControlReceiver.getListHandles(listHandles));
    ASSERT_TRUE(listHandles[0].handle==handle.handle);
    ASSERT_TRUE(listHandles[0].handleType==handle.handleType);
}

TEST_F(routingInterfaceTest,setSinkSoundProperty)
{
	am_Sink_s sink;
	am_sinkID_t sinkID;
	am_Domain_s domain;
	am_domainID_t domainID;
    am_Handle_s handle;
    am_SoundProperty_s soundProperty;
    std::vector<am_Handle_s>listHandles;
	pCF.createSink(sink);
	pCF.createDomain(domain);
	domain.name="mock";
	domain.busname="mock";
	sink.sinkID=2;
	sink.domainID=1;
	ASSERT_EQ(E_OK,pDatabaseHandler.enterDomainDB(domain,domainID));
	ASSERT_EQ(E_OK,pDatabaseHandler.enterSinkDB(sink,sinkID));
    EXPECT_CALL(pMockInterface,asyncSetSinkSoundProperty(_,_,sinkID)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK,pControlReceiver.setSinkSoundProperty(handle,sinkID,soundProperty));
    ASSERT_NE(handle.handle,0);
    ASSERT_EQ(handle.handleType,H_SETSINKSOUNDPROPERTY);
    ASSERT_EQ(E_OK,pControlReceiver.getListHandles(listHandles));
    ASSERT_TRUE(listHandles[0].handle==handle.handle);
    ASSERT_TRUE(listHandles[0].handleType==handle.handleType);
}

TEST_F(routingInterfaceTest,setSourceVolumeNoChange)
{
	am_Source_s source;
	am_sourceID_t sourceID;
	am_Domain_s domain;
	am_domainID_t domainID;
    am_Handle_s handle;
    am_volume_t volume=34;
    am_RampType_e rampType=RAMP_DIRECT;
    am_time_t rampTime=300;
    std::vector<am_Handle_s>listHandles;
	pCF.createSource(source);
	pCF.createDomain(domain);
	domain.name="mock";
	domain.busname="mock";
	source.sourceID=2;
	source.domainID=1;
	source.volume=volume;
	ASSERT_EQ(E_OK,pDatabaseHandler.enterDomainDB(domain,domainID));
	ASSERT_EQ(E_OK,pDatabaseHandler.enterSourceDB(source,sourceID));
    EXPECT_CALL(pMockInterface,asyncSetSourceVolume(_,2,volume,rampType,rampTime)).Times(0);
    ASSERT_EQ(E_NO_CHANGE,pControlReceiver.setSourceVolume(handle,2,volume,rampType,rampTime));
    ASSERT_EQ(E_OK,pControlReceiver.getListHandles(listHandles));
    ASSERT_TRUE(listHandles.empty());
}

TEST_F(routingInterfaceTest,setSourceVolume)
{
	am_Source_s source;
	am_sourceID_t sourceID;
	am_Domain_s domain;
	am_domainID_t domainID;
    am_Handle_s handle;
    am_volume_t volume=34;
    am_RampType_e rampType=RAMP_DIRECT;
    am_time_t rampTime=300;
    std::vector<am_Handle_s>listHandles;
	pCF.createSource(source);
	pCF.createDomain(domain);
	domain.name="mock";
	domain.busname="mock";
	source.sourceID=2;
	source.domainID=1;
	ASSERT_EQ(E_OK,pDatabaseHandler.enterDomainDB(domain,domainID));
	ASSERT_EQ(E_OK,pDatabaseHandler.enterSourceDB(source,sourceID));
    EXPECT_CALL(pMockInterface,asyncSetSourceVolume(_,2,volume,rampType,rampTime)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK,pControlReceiver.setSourceVolume(handle,2,volume,rampType,rampTime));
    ASSERT_NE(handle.handle,0);
    ASSERT_EQ(handle.handleType,H_SETSOURCEVOLUME);
    ASSERT_EQ(E_OK,pControlReceiver.getListHandles(listHandles));
    ASSERT_TRUE(listHandles[0].handle==handle.handle);
    ASSERT_TRUE(listHandles[0].handleType==handle.handleType);
}

TEST_F(routingInterfaceTest,setSinkVolumeNoChange)
{
	am_Sink_s sink;
	am_sinkID_t sinkID;
	am_Domain_s domain;
	am_domainID_t domainID;
    am_Handle_s handle;
    am_volume_t volume=34;
    am_RampType_e rampType=RAMP_DIRECT;
    am_time_t rampTime=300;
    std::vector<am_Handle_s>listHandles;
	pCF.createSink(sink);
	pCF.createDomain(domain);
	domain.name="mock";
	domain.busname="mock";
	sink.sinkID=2;
	sink.domainID=1;
	sink.volume=volume;
	ASSERT_EQ(E_OK,pDatabaseHandler.enterDomainDB(domain,domainID));
	ASSERT_EQ(E_OK,pDatabaseHandler.enterSinkDB(sink,sinkID));
    EXPECT_CALL(pMockInterface,asyncSetSinkVolume(_,2,volume,rampType,rampTime)).Times(0);
    ASSERT_EQ(E_NO_CHANGE,pControlReceiver.setSinkVolume(handle,2,volume,rampType,rampTime));
    ASSERT_EQ(E_OK,pControlReceiver.getListHandles(listHandles));
    ASSERT_TRUE(listHandles.empty());
}

TEST_F(routingInterfaceTest,setSinkVolume)
{
	am_Sink_s sink;
	am_sinkID_t sinkID;
	am_Domain_s domain;
	am_domainID_t domainID;
    am_Handle_s handle;
    am_volume_t volume=34;
    am_RampType_e rampType=RAMP_DIRECT;
    am_time_t rampTime=300;
    std::vector<am_Handle_s>listHandles;
	pCF.createSink(sink);
	pCF.createDomain(domain);
	domain.name="mock";
	domain.busname="mock";
	sink.sinkID=2;
	sink.domainID=1;
	ASSERT_EQ(E_OK,pDatabaseHandler.enterDomainDB(domain,domainID));
	ASSERT_EQ(E_OK,pDatabaseHandler.enterSinkDB(sink,sinkID));
    EXPECT_CALL(pMockInterface,asyncSetSinkVolume(_,2,volume,rampType,rampTime)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK,pControlReceiver.setSinkVolume(handle,2,volume,rampType,rampTime));
    ASSERT_NE(handle.handle,0);
    ASSERT_EQ(handle.handleType,H_SETSINKVOLUME);
    ASSERT_EQ(E_OK,pControlReceiver.getListHandles(listHandles));
    ASSERT_TRUE(listHandles[0].handle==handle.handle);
    ASSERT_TRUE(listHandles[0].handleType==handle.handleType);
}

TEST_F(routingInterfaceTest,connect)
{
	am_Sink_s sink;
	am_sinkID_t sinkID;
	am_Domain_s domain;
	am_domainID_t domainID;
    am_Handle_s handle;
    am_connectionID_t connectionID;
    std::vector<am_Handle_s>listHandles;
	pCF.createSink(sink);
	pCF.createDomain(domain);
	domain.name="mock";
	domain.busname="mock";
	sink.sinkID=2;
	sink.domainID=1;
	ASSERT_EQ(E_OK,pDatabaseHandler.enterDomainDB(domain,domainID));
	ASSERT_EQ(E_OK,pDatabaseHandler.enterSinkDB(sink,sinkID));
    EXPECT_CALL(pMockInterface,asyncConnect(_,_,1,sinkID,CF_ANALOG)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK,pControlReceiver.connect(handle,connectionID,CF_ANALOG,1,2));
    ASSERT_NE(handle.handle,0);
    ASSERT_EQ(handle.handleType,H_CONNECT);
    ASSERT_EQ(E_OK,pControlReceiver.getListHandles(listHandles));
    ASSERT_TRUE(listHandles[0].handle==handle.handle);
    ASSERT_TRUE(listHandles[0].handleType==handle.handleType);
}

TEST_F(routingInterfaceTest,disconnect)
{
	am_Sink_s sink;
	am_sinkID_t sinkID;
	am_Domain_s domain;
	am_domainID_t domainID;
    am_Handle_s handle;
    am_connectionID_t connectionID;
    std::vector<am_Handle_s>listHandles;
	pCF.createSink(sink);
	pCF.createDomain(domain);
	domain.name="mock";
	domain.busname="mock";
	sink.sinkID=2;
	sink.domainID=1;
	ASSERT_EQ(E_OK,pDatabaseHandler.enterDomainDB(domain,domainID));
	ASSERT_EQ(E_OK,pDatabaseHandler.enterSinkDB(sink,sinkID));
    EXPECT_CALL(pMockInterface,asyncConnect(_,_,1,sinkID,CF_ANALOG)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK,pControlReceiver.connect(handle,connectionID,CF_ANALOG,1,2));
    ASSERT_EQ(E_OK,pDatabaseHandler.changeConnectionFinal(connectionID));
    EXPECT_CALL(pMockInterface,asyncDisconnect(_,connectionID)).WillOnce(Return(E_OK));
    ASSERT_EQ(E_OK,pControlReceiver.disconnect(handle,connectionID));
    ASSERT_NE(handle.handle,0);
    ASSERT_EQ(handle.handleType,H_DISCONNECT);
    ASSERT_EQ(E_OK,pControlReceiver.getListHandles(listHandles));
    ASSERT_TRUE(listHandles[1].handle==handle.handle);
    ASSERT_TRUE(listHandles[1].handleType==handle.handleType);
}

TEST_F(routingInterfaceTest,nothingTodisconnect)
{
	am_Handle_s handle;
	am_connectionID_t connectionID=4;
    std::vector<am_Handle_s>listHandles;
    ASSERT_EQ(E_NON_EXISTENT,pControlReceiver.disconnect(handle,connectionID));
    ASSERT_EQ(E_OK,pControlReceiver.getListHandles(listHandles));
    ASSERT_TRUE(listHandles.empty());
}


TEST_F(routingInterfaceTest,setSourceStateNoChange)
{
	am_Source_s source;
	am_sourceID_t sourceID;
	am_Domain_s domain;
	am_domainID_t domainID;
	am_Handle_s handle;
	handle.handle=0;
	am_SourceState_e state=SS_PAUSED;
	pCF.createSource(source);
	pCF.createDomain(domain);
	domain.name="mock";
	domain.busname="mock";
	source.domainID=1;
	source.sourceState=state;
	ASSERT_EQ(E_OK,pDatabaseHandler.enterDomainDB(domain,domainID));
	ASSERT_EQ(E_OK,pDatabaseHandler.enterSourceDB(source,sourceID));
	EXPECT_CALL(pMockInterface,asyncSetSourceState(_,sourceID,state)).Times(0);
    ASSERT_EQ(E_NO_CHANGE,pControlReceiver.setSourceState(handle,sourceID,state));
}


int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

