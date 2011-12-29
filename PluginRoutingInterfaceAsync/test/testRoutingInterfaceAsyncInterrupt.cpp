/*
 * testRoutingInterfaceAsync.cpp
 *
 *  Created on: Dec 27, 2011
 *      Author: christian
 */

#define INTERRUPT_TEST 1

#include "testRoutingInterfaceAsync.h"
#include "config.h"


using namespace am;
using namespace testing;


DLT_DECLARE_CONTEXT(DLT_CONTEXT)


std::vector<std::string> testRoutingInterfaceAsync::pListRoutingPluginDirs=returnListPlugins();
am_domainID_t testRoutingInterfaceAsync::mDomainIDCount=0;
RoutingSender testRoutingInterfaceAsync::pRoutingSender=RoutingSender(pListRoutingPluginDirs);

testRoutingInterfaceAsync::testRoutingInterfaceAsync()
	:pSocketHandler(),
	 pReceiveInterface(),
	 ptimerCallback(this, &testRoutingInterfaceAsync::timerCallback)
{
}

testRoutingInterfaceAsync::~testRoutingInterfaceAsync()
{
}

void testRoutingInterfaceAsync::SetUp()
{
	DLT_REGISTER_APP("DPtest","RoutingInterfacetest");
	DLT_REGISTER_CONTEXT(DLT_CONTEXT,"Main","Main Context");
	DLT_LOG(DLT_CONTEXT,DLT_LOG_INFO, DLT_STRING("RoutingSendInterface Test started "));

	std::vector<int> domainIDs;
	domainIDs.push_back(0);
	domainIDs.push_back(1);

	EXPECT_CALL(pReceiveInterface,getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(&pSocketHandler),Return(E_OK)));
	EXPECT_CALL(pReceiveInterface,registerDomain(_,_)).WillRepeatedly(Invoke(testRoutingInterfaceAsync::handleDomainRegister));
	EXPECT_CALL(pReceiveInterface,registerSource(_,_)).WillRepeatedly(Invoke(testRoutingInterfaceAsync::handleSourceRegister));
	EXPECT_CALL(pReceiveInterface,registerSink(_,_)).WillRepeatedly(Invoke(testRoutingInterfaceAsync::handleSinkRegister));

	pRoutingSender.startupRoutingInterface(&pReceiveInterface);
	pRoutingSender.routingInterfacesReady();

	timespec t;
	t.tv_nsec=0;
	t.tv_sec=4;

	sh_timerHandle_t handle;

	shTimerCallBack *buf=&ptimerCallback;
	//lets use a timeout so the test will finish
	pSocketHandler.addTimer(t,buf,handle,(void*)NULL);
}

std::vector<std::string> am::testRoutingInterfaceAsync::returnListPlugins()
{
	std::vector<std::string> list;
	list.push_back(std::string(DEFAULT_PLUGIN_ROUTING_DIR));
	return (list);
}

am_Error_e am::testRoutingInterfaceAsync::handleSourceRegister(const am_Source_s & sourceData, am_sourceID_t & sourceID)
{
	sourceID=sourceData.sourceID;
	pRoutingSender.addSourceLookup(sourceData);
	return (E_OK);
}

am_Error_e am::testRoutingInterfaceAsync::handleSinkRegister(const am_Sink_s & sinkData, am_sinkID_t & sinkID)
{
	sinkID=sinkData.sinkID;
	pRoutingSender.addSinkLookup(sinkData);
	return(E_OK);
}

am_Error_e am::testRoutingInterfaceAsync::handleDomainRegister(const am_Domain_s & domainData, am_domainID_t & domainID)
{
	am_Domain_s domain=domainData;
	domainID=mDomainIDCount++;
	domain.domainID=domainID;
	pRoutingSender.addDomainLookup(domain);
	return (E_OK);
}

void am::testRoutingInterfaceAsync::timerCallback(sh_timerHandle_t handle, void *userData)
{
	pSocketHandler.stop_listening();
}

void testRoutingInterfaceAsync::TearDown()
{
	DLT_UNREGISTER_CONTEXT(DLT_CONTEXT);
}

std::string DBUSCOMMAND = "dbus-send --session --print-reply --dest=org.genivi.test /org/genivi/test org.genivi.test.";



TEST_F(testRoutingInterfaceAsync,hookInterruptStatusChange)
{
	am_sourceID_t sourceID=2;
	EXPECT_CALL(pReceiveInterface,hookInterruptStatusChange(sourceID,_)).Times(1);
	system((DBUSCOMMAND + std::string("InterruptStatusChange int16:2")).c_str());
	pSocketHandler.start_listenting();
}

TEST_F(testRoutingInterfaceAsync,hookSourceAvailablityStatusChange)
{
	am_sourceID_t sourceID=2;
	EXPECT_CALL(pReceiveInterface,hookSourceAvailablityStatusChange(sourceID,_)).Times(1);
	system((DBUSCOMMAND + std::string("SourceAvailablityStatusChange int16:2")).c_str());
	pSocketHandler.start_listenting();
}

TEST_F(testRoutingInterfaceAsync,hookSinkAvailablityStatusChange)
{
	am_sinkID_t sinkID=2;
	EXPECT_CALL(pReceiveInterface,hookSinkAvailablityStatusChange(sinkID,_)).Times(1);
	system((DBUSCOMMAND + std::string("SinkAvailablityStatusChange int16:2")).c_str());
	pSocketHandler.start_listenting();
}

TEST_F(testRoutingInterfaceAsync,hookTimingInformationChanged)
{
	am_connectionID_t connectionID=4;
	am_timeSync_t delay=35;
	EXPECT_CALL(pReceiveInterface,hookTimingInformationChanged(connectionID,delay)).Times(1);
	system((DBUSCOMMAND + std::string("timingChanged int16:4 int16:35")).c_str());
	pSocketHandler.start_listenting();
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}


