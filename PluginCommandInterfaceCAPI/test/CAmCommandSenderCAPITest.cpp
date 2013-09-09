/**
 *  Copyright (c) 2012 BMW
 *
 *  \author Aleksandar Donchev, aleksander.donchev@partner.bmw.de BMW 2013
 *
 *  \copyright
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction,
 *  including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 *  subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 *  THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *  For further information see http://www.genivi.org/.
 */

#include "CAmCommandSenderCAPITest.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include "TAmPluginTemplate.h"
#include "MockIAmCommandReceive.h"
#include "shared/CAmDltWrapper.h"
#include "../include/CAmCommandSenderCAPI.h"
#include "../include/CAmCommandSenderCommon.h"
#include "MockNotificationsClient.h"
#include <CommonAPI/CommonAPI.h>
#include <sys/time.h>



using namespace am;
using namespace testing;
using namespace CommonAPI;

static CAmTestsEnvironment* env;

pthread_cond_t      cond  		= PTHREAD_COND_INITIALIZER;
pthread_mutex_t     mutex		= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t      condPxy 	= PTHREAD_COND_INITIALIZER;
pthread_mutex_t     mutexPxy    = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t      condSer 	= PTHREAD_COND_INITIALIZER;
pthread_mutex_t     mutexSer    = PTHREAD_MUTEX_INITIALIZER;

void* run_client(void*)
{
	CAmSocketHandler socketHandler;
	CAmTestCAPIWrapper wrapper(&socketHandler);
	env->mSocketHandlerClient = &socketHandler;
	std::shared_ptr<CommonAPI::Factory> factory = wrapper.factory();
	env->mProxy = factory->buildProxy<org::genivi::am::CommandControlProxy>(CAmCommandSenderCAPI::COMMAND_SENDER_SERVICE);
	env->mProxy->getProxyStatusEvent().subscribe(std::bind(&CAmTestsEnvironment::onServiceStatusEvent,env,std::placeholders::_1));

	pthread_mutex_lock(&mutexSer);
	env->mIsProxyInitilized = true;
	pthread_mutex_unlock(&mutexSer);
	pthread_cond_signal(&condSer);

	socketHandler.start_listenting();

//Cleanup
    env->mProxy.reset();
    env->mSocketHandlerClient = NULL;

    return (NULL);
}

void* run_service(void*)
{
	CAmSocketHandler socketHandler;
	CAmTestCAPIWrapper wrapper(&socketHandler);
	CAmCommandSenderCAPI plugin(&wrapper);
	env->mpPlugin = &plugin;
	env->mSocketHandlerService = &socketHandler;
	MockIAmCommandReceive mock;
	env->mpCommandReceive = &mock;
    if(plugin.startupInterface(env->mpCommandReceive)!=E_OK)
	{
		logError("CommandSendInterface can't start!");
	}
    else
    {
    	EXPECT_CALL(*env->mpCommandReceive,confirmCommandReady(10,_));
    	plugin.setCommandReady(10);
    	socketHandler.start_listenting();

    	EXPECT_CALL(*env->mpCommandReceive,confirmCommandRundown(10,_));
    	plugin.setCommandRundown(10);
    	plugin.tearDownInterface(env->mpCommandReceive);
    }

//Cleanup
    env->mpPlugin = NULL;
    env->mpCommandReceive = NULL;
    env->mSocketHandlerClient = NULL;

    return (NULL);
}

void* run_listener(void*)
{
    pthread_mutex_lock(&mutexSer);
    while (env->mIsProxyInitilized==false)
    {
    	std::cout << "\n\r Intialize proxy..\n\r" ;
    	pthread_cond_wait(&condSer, &mutexSer);
    }
    pthread_mutex_unlock(&mutexSer);

    time_t start = time(0);
    time_t now = start;
    pthread_mutex_lock(&mutexPxy);
    while ( env->mIsServiceAvailable==false && now-start <= 15 )
    {
    	std::cout << " Waiting for proxy..\n\r" ;
        struct timespec ts = { 0, 0 };
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += 5;
        pthread_cond_timedwait(&condPxy, &mutexPxy, &ts);
        now = time(0);
    }
    pthread_mutex_unlock(&mutexPxy);
    pthread_cond_signal(&cond);

	return NULL;
}

CAmTestsEnvironment::CAmTestsEnvironment() :
		mListenerThread(0),
		mServicePThread(0),
		mClientPThread(0),
		mSocketHandlerService(NULL),
		mSocketHandlerClient(NULL),
		mIsProxyInitilized(false),
		mIsServiceAvailable(false),
		mpCommandReceive(NULL),
		mpPlugin(NULL)
{
    env=this;

	CAmDltWrapper::instance()->registerApp("capiTest", "capiTest");
	pthread_create(&mListenerThread, NULL, run_listener, NULL);
    pthread_create(&mServicePThread, NULL, run_service, NULL);
    pthread_create(&mClientPThread, NULL, run_client, NULL);
}

CAmTestsEnvironment::~CAmTestsEnvironment()
{

}

void CAmTestsEnvironment::SetUp()
{
	pthread_cond_wait(&cond, &mutex);
}

void CAmTestsEnvironment::TearDown()
{
//	mWrapperClient.factory().reset();

	mSocketHandlerClient->exit_mainloop();
    pthread_join(mClientPThread, NULL);
	mSocketHandlerService->exit_mainloop();
    pthread_join(mServicePThread, NULL);
    sleep(1);
}

void CAmTestsEnvironment::onServiceStatusEvent(const CommonAPI::AvailabilityStatus& serviceStatus)
{
    std::stringstream  avail;
    avail  << "(" << static_cast<int>(serviceStatus) << ")";

    logInfo("Service Status changed to ", avail.str());
    std::cout << std::endl << "Service Status changed to " << avail.str() << std::endl;
    pthread_mutex_lock(&mutexPxy);
    mIsServiceAvailable = (serviceStatus==CommonAPI::AvailabilityStatus::AVAILABLE);
    pthread_mutex_unlock(&mutexPxy);
    pthread_cond_signal(&condPxy);
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new CAmTestsEnvironment);
    return RUN_ALL_TESTS();
}

CAmCommandSenderCAPITest::CAmCommandSenderCAPITest()
{

}

CAmCommandSenderCAPITest::~CAmCommandSenderCAPITest()
{

}

void CAmCommandSenderCAPITest::SetUp()
{
	::testing::GTEST_FLAG(throw_on_failure) = false;
}

void CAmCommandSenderCAPITest::TearDown()
{
	::testing::GTEST_FLAG(throw_on_failure) = true;
}

TEST_F(CAmCommandSenderCAPITest, ClientStartupTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

ACTION(returnClientConnect)
{
arg2=101;
}

TEST_F(CAmCommandSenderCAPITest, ConnectTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		org::genivi::am::am_sourceID_t sourceID = 500;
		org::genivi::am::am_sinkID_t sinkID = 400;
		CommonAPI::CallStatus callStatus = CommonAPI::CallStatus::NOT_AVAILABLE;
		org::genivi::am::am_Error_e result = org::genivi::am::am_Error_e::E_OK;
		org::genivi::am::am_mainConnectionID_t mainConnectionID = 0;

		EXPECT_CALL(*env->mpCommandReceive, connect(_, _, _)).WillOnce(DoAll(returnClientConnect(), Return(E_OK)));
		env->mProxy->connect(sourceID, sinkID, callStatus, mainConnectionID, result);
		ASSERT_EQ(mainConnectionID, 101);
		ASSERT_EQ(result, org::genivi::am::am_Error_e::E_OK);
		EXPECT_CALL(*env->mpCommandReceive, disconnect(mainConnectionID)).WillOnce(Return(am_Error_e::E_OK));
		ASSERT_EQ(result, org::genivi::am::am_Error_e::E_OK);
		env->mProxy->disconnect(mainConnectionID, callStatus, result);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

TEST_F(CAmCommandSenderCAPITest, SetVolumeTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		org::genivi::am::am_mainVolume_t volume = 100;
		org::genivi::am::am_sinkID_t sinkID = 400;
		CommonAPI::CallStatus callStatus = CommonAPI::CallStatus::NOT_AVAILABLE;
		org::genivi::am::am_Error_e result = org::genivi::am::am_Error_e::E_OK;

		EXPECT_CALL(*env->mpCommandReceive, setVolume(sinkID,volume)).WillOnce(Return(E_OK));
		env->mProxy->setVolume(sinkID, volume, callStatus, result);
		ASSERT_EQ(result, org::genivi::am::am_Error_e::E_OK);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

TEST_F(CAmCommandSenderCAPITest, VolumeStepTest)
{

	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		org::genivi::am::am_mainVolume_t volume = 100;
		org::genivi::am::am_sinkID_t sinkID = 400;
		CommonAPI::CallStatus callStatus = CommonAPI::CallStatus::NOT_AVAILABLE;
		org::genivi::am::am_Error_e result = org::genivi::am::am_Error_e::E_OK;

		EXPECT_CALL(*env->mpCommandReceive, volumeStep(sinkID,volume)).WillOnce(Return(E_OK));
		env->mProxy->volumeStep(sinkID, volume, callStatus, result);
		ASSERT_EQ(result, org::genivi::am::am_Error_e::E_OK);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

TEST_F(CAmCommandSenderCAPITest, SetSinkMuteStateTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		org::genivi::am::am_MuteState_e value = org::genivi::am::am_MuteState_e::MS_UNKNOWN;
		org::genivi::am::am_sinkID_t sinkID = 400;
		CommonAPI::CallStatus callStatus = CommonAPI::CallStatus::NOT_AVAILABLE;
		org::genivi::am::am_Error_e result = org::genivi::am::am_Error_e::E_OK;

		EXPECT_CALL(*env->mpCommandReceive, setSinkMuteState(sinkID, am_MuteState_e::MS_UNKNOWN)).WillOnce(Return(E_OK));
		env->mProxy->setSinkMuteState(sinkID, value, callStatus, result);
		ASSERT_EQ(result, org::genivi::am::am_Error_e::E_OK);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

TEST_F(CAmCommandSenderCAPITest, SetMainSinkSoundPropertyTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		org::genivi::am::am_sinkID_t sinkID = 400;
		CommonAPI::CallStatus callStatus = CommonAPI::CallStatus::NOT_AVAILABLE;
		org::genivi::am::am_Error_e result = org::genivi::am::am_Error_e::E_OK;

		EXPECT_CALL(*env->mpCommandReceive, setMainSinkSoundProperty(AllOf(
				Field(&am_MainSoundProperty_s::value, 3),
				Field(&am_MainSoundProperty_s::type, MSP_UNKNOWN)), sinkID)).WillOnce(Return(E_OK));
		org::genivi::am::am_MainSoundProperty_s value(static_cast<org::genivi::am::am_MainSoundPropertyType_pe>(am_MainSoundPropertyType_e::MSP_UNKNOWN), (const int16_t)3);
		env->mProxy->setMainSinkSoundProperty(sinkID, value, callStatus, result);
		ASSERT_EQ(result, org::genivi::am::am_Error_e::E_OK);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

TEST_F(CAmCommandSenderCAPITest, SetMainSourceSoundPropertyTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		org::genivi::am::am_sourceID_t sID = 400;
		CommonAPI::CallStatus callStatus = CommonAPI::CallStatus::NOT_AVAILABLE;
		org::genivi::am::am_Error_e result = org::genivi::am::am_Error_e::E_OK;

		EXPECT_CALL(*env->mpCommandReceive, setMainSourceSoundProperty(AllOf(
				Field(&am_MainSoundProperty_s::value, 3),
				Field(&am_MainSoundProperty_s::type, MSP_UNKNOWN)), sID)).WillOnce(Return(E_OK));
		org::genivi::am::am_MainSoundProperty_s value(static_cast<org::genivi::am::am_MainSoundPropertyType_pe>(am_MainSoundPropertyType_e::MSP_UNKNOWN), (const int16_t)3);
		env->mProxy->setMainSourceSoundProperty(sID, value, callStatus, result);
		ASSERT_EQ(result, org::genivi::am::am_Error_e::E_OK);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

TEST_F(CAmCommandSenderCAPITest, SetSystemPropertyTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		CommonAPI::CallStatus callStatus = CommonAPI::CallStatus::NOT_AVAILABLE;
		org::genivi::am::am_Error_e result = org::genivi::am::am_Error_e::E_OK;

		EXPECT_CALL(*env->mpCommandReceive, setSystemProperty(Field(&am_SystemProperty_s::value, 2))).WillOnce(Return(E_OK));

		org::genivi::am::am_SystemProperty_s value(static_cast<org::genivi::am::am_SystemPropertyType_pe>(am_SystemPropertyType_e::SYP_UNKNOWN), (const int16_t)2);
		env->mProxy->setSystemProperty(value, callStatus, result);
		ASSERT_EQ(result, org::genivi::am::am_Error_e::E_OK);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

ACTION(returnListConnections){
	std::vector<am_MainConnectionType_s> list;
	am_MainConnectionType_s listItem;
	listItem.mainConnectionID=15;
	listItem.sinkID=4;
	listItem.sourceID=3;
	listItem.connectionState=CS_UNKNOWN;
	listItem.delay=34;
	list.push_back(listItem);
	arg0=list;
}

TEST_F(CAmCommandSenderCAPITest, GetListMainConnectionsTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		CommonAPI::CallStatus callStatus = CommonAPI::CallStatus::NOT_AVAILABLE;
		org::genivi::am::am_Error_e result = org::genivi::am::am_Error_e::E_OK;

		EXPECT_CALL(*env->mpCommandReceive, getListMainConnections(_)).WillOnce(DoAll(returnListConnections(), Return(E_OK)));
		org::genivi::am::am_MainConnection_L listConnections;
		env->mProxy->getListMainConnections(callStatus, result, listConnections);
		ASSERT_EQ(result, org::genivi::am::am_Error_e::E_OK);
		ASSERT_EQ(1, listConnections.size());
		ASSERT_EQ(15, listConnections.at(0).mainConnectionID);
		ASSERT_EQ(4, listConnections.at(0).sinkID);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

ACTION(returnListSinks){
	std::vector<am_SinkType_s> list;
	am_SinkType_s listItem;
	listItem.availability.availability=A_UNAVAILABLE;
	listItem.availability.availabilityReason=AR_GENIVI_NOMEDIA;
	listItem.muteState=MS_UNMUTED;
	listItem.name="mySink";
	listItem.sinkClassID=34;
	listItem.sinkID=24;
	listItem.volume=124;
	list.push_back(listItem);
	arg0=list;
}

TEST_F(CAmCommandSenderCAPITest, GetListMainSinksTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		CommonAPI::CallStatus callStatus = CommonAPI::CallStatus::NOT_AVAILABLE;
		org::genivi::am::am_Error_e result = org::genivi::am::am_Error_e::E_OK;

		EXPECT_CALL(*env->mpCommandReceive, getListMainSinks(_)).WillOnce(DoAll(returnListSinks(), Return(E_OK)));
		org::genivi::am::am_SinkType_L listMainSinks;
		env->mProxy->getListMainSinks(callStatus, listMainSinks,result);
		ASSERT_EQ(result, org::genivi::am::am_Error_e::E_OK);
		ASSERT_EQ(1, listMainSinks.size());
		ASSERT_EQ(34, listMainSinks.at(0).sinkClassID);
		ASSERT_EQ(24, listMainSinks.at(0).sinkID);
		ASSERT_EQ(org::genivi::am::am_Availability_e::A_UNAVAILABLE, listMainSinks.at(0).availability.availability);
		ASSERT_EQ(static_cast<org::genivi::am::am_AvailabilityReason_pe>(am_AvailabilityReason_e::AR_GENIVI_NOMEDIA), listMainSinks.at(0).availability.availabilityReason);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

ACTION(returnListSources){
	std::vector<am_SourceType_s> list;
	am_SourceType_s listItem;
	listItem.availability.availability=A_MAX;
	listItem.availability.availabilityReason=AR_GENIVI_SAMEMEDIA;
	listItem.name="MySource";
	listItem.sourceClassID=12;
	listItem.sourceID=224;
	list.push_back(listItem);
	listItem.name="NextSource";
	listItem.sourceID=22;
	list.push_back(listItem);
	arg0=list;
}
TEST_F(CAmCommandSenderCAPITest, GetListMainSourcesTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		CommonAPI::CallStatus callStatus = CommonAPI::CallStatus::NOT_AVAILABLE;
		org::genivi::am::am_Error_e result = org::genivi::am::am_Error_e::E_OK;

		EXPECT_CALL(*env->mpCommandReceive, getListMainSources(_)).WillOnce(DoAll(returnListSources(), Return(E_OK)));
		org::genivi::am::am_SourceType_L list;
		env->mProxy->getListMainSources(callStatus, list, result);
		ASSERT_EQ(result, org::genivi::am::am_Error_e::E_OK);
		ASSERT_EQ(2, list.size());
		ASSERT_EQ(12, list.at(0).sourceClassID);
		ASSERT_EQ(224, list.at(0).sourceID);
		ASSERT_EQ(org::genivi::am::am_Availability_e::A_MAX, list.at(0).availability.availability);
		ASSERT_EQ(static_cast<org::genivi::am::am_AvailabilityReason_pe>(am_AvailabilityReason_e::AR_GENIVI_SAMEMEDIA), list.at(0).availability.availabilityReason);
		ASSERT_EQ(22, list.at(1).sourceID);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

ACTION(returnListMainSinkSoundProperties){
	std::vector<am_MainSoundProperty_s> list;
	am_MainSoundProperty_s listItem;
	listItem.type=MSP_MAX;
	listItem.value=223;
	list.push_back(listItem);
	listItem.type=MSP_MAX;
	listItem.value=2;
	list.push_back(listItem);
	arg1=list;
}

TEST_F(CAmCommandSenderCAPITest, GetListMainSinkSoundPropertiesTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		org::genivi::am::am_sinkID_t sID = 400;
		CommonAPI::CallStatus callStatus = CommonAPI::CallStatus::NOT_AVAILABLE;
		org::genivi::am::am_Error_e result = org::genivi::am::am_Error_e::E_OK;

		EXPECT_CALL(*env->mpCommandReceive, getListMainSinkSoundProperties(sID,_)).WillOnce(DoAll(returnListMainSinkSoundProperties(), Return(E_OK)));
		org::genivi::am::am_MainSoundProperty_L list;
		env->mProxy->getListMainSinkSoundProperties(sID, callStatus, list, result);
		ASSERT_EQ(result, org::genivi::am::am_Error_e::E_OK);
		ASSERT_EQ(2, list.size());
		ASSERT_EQ(223, list.at(0).value);
		ASSERT_EQ(static_cast<org::genivi::am::am_MainSoundPropertyType_pe>(am_MainSoundPropertyType_e::MSP_MAX), list.at(0).type);
		ASSERT_EQ(2, list.at(1).value);
		ASSERT_EQ(static_cast<org::genivi::am::am_MainSoundPropertyType_pe>(am_MainSoundPropertyType_e::MSP_MAX), list.at(1).type);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

ACTION(returnListMainSourceSoundProperties){
	std::vector<am_MainSoundProperty_s> list;
	am_MainSoundProperty_s listItem;
	listItem.type=MSP_EXAMPLE_MID;
	listItem.value=223;
	list.push_back(listItem);
	listItem.type=MSP_MAX;
	listItem.value=2;
	list.push_back(listItem);
	arg1=list;
}

TEST_F(CAmCommandSenderCAPITest, GetListMainSourceSoundPropertiesTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		org::genivi::am::am_sourceID_t sID = 400;
		CommonAPI::CallStatus callStatus = CommonAPI::CallStatus::NOT_AVAILABLE;
		org::genivi::am::am_Error_e result = org::genivi::am::am_Error_e::E_OK;

		EXPECT_CALL(*env->mpCommandReceive, getListMainSourceSoundProperties(sID,_)).WillOnce(DoAll(returnListMainSourceSoundProperties(), Return(E_OK)));
		org::genivi::am::am_MainSoundProperty_L list;
		env->mProxy->getListMainSourceSoundProperties(sID, callStatus, list, result);
		ASSERT_EQ(result, org::genivi::am::am_Error_e::E_OK);
		ASSERT_EQ(2, list.size());
		ASSERT_EQ(223, list.at(0).value);
		ASSERT_EQ(static_cast<org::genivi::am::am_MainSoundPropertyType_pe>(am_MainSoundPropertyType_e::MSP_EXAMPLE_MID), list.at(0).type);
		ASSERT_EQ(2, list.at(1).value);
		ASSERT_EQ(static_cast<org::genivi::am::am_MainSoundPropertyType_pe>(am_MainSoundPropertyType_e::MSP_MAX), list.at(1).type);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

ACTION(returnListSourceClasses){
	std::vector<am_SourceClass_s> list;
	am_SourceClass_s listItem;
	am_ClassProperty_s property;
	property.classProperty=CP_MAX;
	property.value=12;
	listItem.name="FirstCLass";
	listItem.sourceClassID=23;
	listItem.listClassProperties.push_back(property);
	list.push_back(listItem);
	listItem.name="SecondCLass";
	listItem.sourceClassID=2;
	listItem.listClassProperties.push_back(property);
	list.push_back(listItem);
	arg0=list;
}

TEST_F(CAmCommandSenderCAPITest, GetListSourceClassesTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		CommonAPI::CallStatus callStatus = CommonAPI::CallStatus::NOT_AVAILABLE;
		org::genivi::am::am_Error_e result = org::genivi::am::am_Error_e::E_OK;

		EXPECT_CALL(*env->mpCommandReceive, getListSourceClasses(_)).WillOnce(DoAll(returnListSourceClasses(), Return(E_OK)));
		org::genivi::am::am_SourceClass_L list;
		env->mProxy->getListSourceClasses(callStatus, list, result);
		ASSERT_EQ(result, org::genivi::am::am_Error_e::E_OK);
		ASSERT_EQ(2, list.size());

		ASSERT_EQ(23, list.at(0).sourceClassID);
		ASSERT_EQ(1, list.at(0).listClassProperties.size());
		ASSERT_EQ(static_cast<org::genivi::am::am_ClassProperty_pe>(am_ClassProperty_e::CP_MAX), list.at(0).listClassProperties.at(0).classProperty);

		ASSERT_EQ(2, list.at(1).sourceClassID);
		ASSERT_EQ(2, list.at(1).listClassProperties.size());
		ASSERT_EQ(static_cast<org::genivi::am::am_ClassProperty_pe>(am_ClassProperty_e::CP_MAX), list.at(1).listClassProperties.at(0).classProperty);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

ACTION(returnListSinkClasses){
	std::vector<am_SinkClass_s> list;
	am_SinkClass_s listItem;
	am_ClassProperty_s property;
	property.classProperty=CP_MAX;
	property.value=122;
	listItem.name="FirstCLass";
	listItem.sinkClassID=23;
	listItem.listClassProperties.push_back(property);
	list.push_back(listItem);
	listItem.name="SecondCLass";
	listItem.sinkClassID=2;
	listItem.listClassProperties.push_back(property);
	list.push_back(listItem);
	arg0=list;
}

TEST_F(CAmCommandSenderCAPITest, GetListSinkClassesTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		CommonAPI::CallStatus callStatus = CommonAPI::CallStatus::NOT_AVAILABLE;
		org::genivi::am::am_Error_e result = org::genivi::am::am_Error_e::E_OK;

		EXPECT_CALL(*env->mpCommandReceive, getListSinkClasses(_)).WillOnce(DoAll(returnListSinkClasses(), Return(E_OK)));
		org::genivi::am::am_SinkClass_L list;
		env->mProxy->getListSinkClasses(callStatus, list,result);
		ASSERT_EQ(result, org::genivi::am::am_Error_e::E_OK);
		ASSERT_EQ(2, list.size());

		ASSERT_EQ(0, list.at(0).name.compare("FirstCLass"));
		ASSERT_EQ(23, list.at(0).sinkClassID);
		ASSERT_EQ(1, list.at(0).listClassProperties.size());
		ASSERT_EQ(static_cast<org::genivi::am::am_ClassProperty_pe>(am_ClassProperty_e::CP_MAX), list.at(0).listClassProperties.at(0).classProperty);

		ASSERT_EQ(0, list.at(1).name.compare("SecondCLass"));
		ASSERT_EQ(2, list.at(1).sinkClassID);
		ASSERT_EQ(2, list.at(1).listClassProperties.size());
		ASSERT_EQ(static_cast<org::genivi::am::am_ClassProperty_pe>(am_ClassProperty_e::CP_MAX), list.at(1).listClassProperties.at(0).classProperty);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

ACTION(returnListSystemProperties){
	std::vector<am_SystemProperty_s> list;
	am_SystemProperty_s listItem;
	listItem.type=SYP_MAX;
	listItem.value=-2245;
	list.push_back(listItem);
	arg0=list;
}

TEST_F(CAmCommandSenderCAPITest, GetListSystemPropertiesTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		CommonAPI::CallStatus callStatus = CommonAPI::CallStatus::NOT_AVAILABLE;
		org::genivi::am::am_Error_e result = org::genivi::am::am_Error_e::E_OK;

		EXPECT_CALL(*env->mpCommandReceive, getListSystemProperties(_)).WillOnce(DoAll(returnListSystemProperties(), Return(E_OK)));
		org::genivi::am::am_SystemProperty_L list;
		env->mProxy->getListSystemProperties(callStatus, list,result);
		ASSERT_EQ(result, org::genivi::am::am_Error_e::E_OK);
		ASSERT_EQ(1, list.size());

		ASSERT_EQ(-2245, list.at(0).value);
		ASSERT_EQ(static_cast<org::genivi::am::am_ClassProperty_pe>(am_SystemPropertyType_e::SYP_MAX), list.at(0).type);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}


/**
 * Signal tests
 */

#define SIMPLE_THREADS_SYNC_MICROSEC() usleep(50000)

TEST_F(CAmCommandSenderCAPITest, onNewMainConnection)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		MockNotificationsClient mock;
		auto subscription = env->mProxy->getNewMainConnectionEvent().subscribe(std::bind(&MockNotificationsClient::onNewMainConnection, std::ref(mock), std::placeholders::_1));
		am_MainConnectionType_s mainConnection;
		mainConnection.connectionState=am_ConnectionState_e::CS_CONNECTING;
		mainConnection.delay=400;
		mainConnection.mainConnectionID=3;
		mainConnection.sinkID=4;
		mainConnection.sourceID=5;
		org::genivi::am::am_MainConnectionType_s mainConnectionCAPI;
		mainConnectionCAPI.connectionState=CAmConvert2CAPIType(mainConnection.connectionState);
		mainConnectionCAPI.delay=mainConnection.delay;
		mainConnectionCAPI.mainConnectionID=mainConnection.mainConnectionID;
		mainConnectionCAPI.sinkID=mainConnection.sinkID;
		mainConnectionCAPI.sourceID=mainConnection.sourceID;
		EXPECT_CALL(mock, onNewMainConnection(mainConnectionCAPI));
		env->mpPlugin->cbNewMainConnection(mainConnection);
		SIMPLE_THREADS_SYNC_MICROSEC();
		env->mProxy->getNewMainConnectionEvent().unsubscribe(subscription);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

TEST_F(CAmCommandSenderCAPITest, removedMainConnection)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		MockNotificationsClient mock;
		auto subscription = env->mProxy->getRemovedMainConnectionEvent().subscribe(std::bind(&MockNotificationsClient::removedMainConnection, std::ref(mock), std::placeholders::_1));
		am_mainConnectionID_t mainConnectionID(3);
		org::genivi::am::am_mainConnectionID_t mainConnectionIDCAPI(mainConnectionID);
		EXPECT_CALL(mock, removedMainConnection(mainConnectionIDCAPI));
		env->mpPlugin->cbRemovedMainConnection(mainConnectionID);
		SIMPLE_THREADS_SYNC_MICROSEC();
		env->mProxy->getRemovedMainConnectionEvent().unsubscribe(subscription);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

TEST_F(CAmCommandSenderCAPITest, onNumberOfSourceClassesChangedEventTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		MockNotificationsClient mock;
		auto subscription = env->mProxy->getNumberOfSourceClassesChangedEvent().subscribe(
												std::bind(&MockNotificationsClient::onNumberOfSourceClassesChangedEvent, std::ref(mock)));
		EXPECT_CALL(mock, onNumberOfSourceClassesChangedEvent());
		env->mpPlugin->cbNumberOfSourceClassesChanged();
		SIMPLE_THREADS_SYNC_MICROSEC();
		env->mProxy->getNumberOfSourceClassesChangedEvent().unsubscribe(subscription);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

TEST_F(CAmCommandSenderCAPITest, onMainConnectionStateChangedEventTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		MockNotificationsClient mock;
		auto subscription = env->mProxy->getMainConnectionStateChangedEvent().subscribe(std::bind(&MockNotificationsClient::onMainConnectionStateChangedEvent, std::ref(mock),
													   std::placeholders::_1, std::placeholders::_2));
		EXPECT_CALL(mock, onMainConnectionStateChangedEvent(101, org::genivi::am::am_ConnectionState_e::CS_SUSPENDED));
		env->mpPlugin->cbMainConnectionStateChanged(101, CS_SUSPENDED);
		SIMPLE_THREADS_SYNC_MICROSEC();
		env->mProxy->getMainConnectionStateChangedEvent().unsubscribe(subscription);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

TEST_F(CAmCommandSenderCAPITest, onSourceAddedEventTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		MockNotificationsClient mock;
		auto subscription = env->mProxy->getNewSourceEvent().subscribe(std::bind(&MockNotificationsClient::onSourceAddedEvent, std::ref(mock),std::placeholders::_1));
		org::genivi::am::am_SourceType_s destination;
		destination.sourceID = 100;
		destination.name = "Name";
		destination.availability.availability = org::genivi::am::am_Availability_e::A_MAX;
		destination.availability.availabilityReason = static_cast<org::genivi::am::am_AvailabilityReason_pe>(AR_MAX);
		destination.sourceClassID = 200;

		am_SourceType_s origin;
		origin.sourceID = 100;
		origin.name = "Name";
		origin.availability.availability = A_MAX;
		origin.availability.availabilityReason = AR_MAX;
 		origin.sourceClassID = 200;
		EXPECT_CALL(mock, onSourceAddedEvent(destination));
		env->mpPlugin->cbNewSource(origin);
		SIMPLE_THREADS_SYNC_MICROSEC();
		env->mProxy->getNewSourceEvent().unsubscribe(subscription);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

TEST_F(CAmCommandSenderCAPITest, onSourceRemovedEventTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		MockNotificationsClient mock;
		auto subscription = env->mProxy->getRemovedSourceEvent().subscribe(std::bind(&MockNotificationsClient::onSourceRemovedEvent, std::ref(mock),
													   std::placeholders::_1));
		am_sourceID_t source = 101;
		EXPECT_CALL(mock, onSourceRemovedEvent(source));
		env->mpPlugin->cbRemovedSource(source);
		SIMPLE_THREADS_SYNC_MICROSEC();
		env->mProxy->getRemovedSourceEvent().unsubscribe(subscription);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

TEST_F(CAmCommandSenderCAPITest, onMainSourceSoundPropertyChangedEventTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		MockNotificationsClient mock;
		auto subscription = env->mProxy->getMainSourceSoundPropertyChangedEvent().subscribe(std::bind(&MockNotificationsClient::onMainSourceSoundPropertyChangedEvent, std::ref(mock),
													   std::placeholders::_1, std::placeholders::_2));

		am_MainSoundProperty_s soundProperty;
		soundProperty.value = 10;
		soundProperty.type = am_MainSoundPropertyType_e::MSP_MAX;

		org::genivi::am::am_MainSoundProperty_s destination(static_cast<org::genivi::am::am_MainSoundPropertyType_pe>(MSP_MAX), 10);

		EXPECT_CALL(mock, onMainSourceSoundPropertyChangedEvent(101, destination));
		env->mpPlugin->cbMainSourceSoundPropertyChanged(101, soundProperty);
		SIMPLE_THREADS_SYNC_MICROSEC();
		env->mProxy->getMainSourceSoundPropertyChangedEvent().unsubscribe(subscription);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

TEST_F(CAmCommandSenderCAPITest, onSourceAvailabilityChangedEventTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		MockNotificationsClient mock;
		auto subscription = env->mProxy->getSourceAvailabilityChangedEvent().subscribe(std::bind(&MockNotificationsClient::onSourceAvailabilityChangedEvent, std::ref(mock),
													   std::placeholders::_1, std::placeholders::_2));

		am_Availability_s availability;
		availability.availability = A_MAX;
		availability.availabilityReason = AR_MAX;

		org::genivi::am::am_Availability_s destination(org::genivi::am::am_Availability_e::A_MAX, static_cast<org::genivi::am::am_AvailabilityReason_pe>(AR_MAX));

		EXPECT_CALL(mock, onSourceAvailabilityChangedEvent(101, destination));
		env->mpPlugin->cbSourceAvailabilityChanged(101, availability);
		SIMPLE_THREADS_SYNC_MICROSEC();
		env->mProxy->getSourceAvailabilityChangedEvent().unsubscribe(subscription);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

TEST_F(CAmCommandSenderCAPITest, onNumberOfSinkClassesChangedEventTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		MockNotificationsClient mock;
		auto subscription = env->mProxy->getNumberOfSinkClassesChangedEvent().subscribe(std::bind(&MockNotificationsClient::onNumberOfSinkClassesChangedEvent, std::ref(mock)));
		EXPECT_CALL(mock, onNumberOfSinkClassesChangedEvent());
		env->mpPlugin->cbNumberOfSinkClassesChanged();
		SIMPLE_THREADS_SYNC_MICROSEC();
		env->mProxy->getNumberOfSinkClassesChangedEvent().unsubscribe(subscription);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

TEST_F(CAmCommandSenderCAPITest, onSinkAddedEventTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		MockNotificationsClient mock;
		auto subscription = env->mProxy->getNewSinkEvent().subscribe(std::bind(&MockNotificationsClient::onSinkAddedEvent, std::ref(mock),
													   std::placeholders::_1));
		org::genivi::am::am_SinkType_s destination;
		destination.sinkID = 100;
		destination.name = "Name";
		destination.availability.availability = org::genivi::am::am_Availability_e::A_MAX;
		destination.availability.availabilityReason = static_cast<org::genivi::am::am_AvailabilityReason_pe>(AR_MAX);
		destination.muteState = org::genivi::am::am_MuteState_e::MS_MAX;
		destination.volume = 1;
		destination.sinkClassID = 100;

		am_SinkType_s origin;
		origin.sinkID = 100;
		origin.name = "Name";
		origin.availability.availability = A_MAX;
		origin.availability.availabilityReason = AR_MAX;
 		origin.muteState = am_MuteState_e::MS_MAX;
 		origin.volume = 1;
 		origin.sinkClassID = 100;

		EXPECT_CALL(mock, onSinkAddedEvent(destination));
		env->mpPlugin->cbNewSink(origin);
		SIMPLE_THREADS_SYNC_MICROSEC();
		env->mProxy->getNewSinkEvent().unsubscribe(subscription);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

TEST_F(CAmCommandSenderCAPITest, onSinkRemovedEventTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		MockNotificationsClient mock;
		auto subscription = env->mProxy->getRemovedSinkEvent().subscribe(std::bind(&MockNotificationsClient::onSinkRemovedEvent, std::ref(mock),
													   std::placeholders::_1));
		EXPECT_CALL(mock, onSinkRemovedEvent(101));
		env->mpPlugin->cbRemovedSink(101);
		SIMPLE_THREADS_SYNC_MICROSEC();
		env->mProxy->getRemovedSinkEvent().unsubscribe(subscription);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

TEST_F(CAmCommandSenderCAPITest, onMainSinkSoundPropertyChangedEventTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		MockNotificationsClient mock;
		auto subscription = env->mProxy->getMainSinkSoundPropertyChangedEvent().subscribe(std::bind(&MockNotificationsClient::onMainSinkSoundPropertyChangedEvent, std::ref(mock),
													   std::placeholders::_1, std::placeholders::_2));

		am_MainSoundProperty_s soundProperty;
		soundProperty.value = 10;
		soundProperty.type = am_MainSoundPropertyType_e::MSP_MAX;

		org::genivi::am::am_MainSoundProperty_s destination(static_cast<org::genivi::am::am_MainSoundPropertyType_pe>(MSP_MAX), 10);

		EXPECT_CALL(mock, onMainSinkSoundPropertyChangedEvent(101, destination));
		env->mpPlugin->cbMainSinkSoundPropertyChanged(101, soundProperty);
		SIMPLE_THREADS_SYNC_MICROSEC();
		env->mProxy->getMainSinkSoundPropertyChangedEvent().unsubscribe(subscription);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

TEST_F(CAmCommandSenderCAPITest, onSinkAvailabilityChangedEventTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		MockNotificationsClient mock;
		auto subscription = env->mProxy->getSinkAvailabilityChangedEvent().subscribe(std::bind(&MockNotificationsClient::onSinkAvailabilityChangedEvent, std::ref(mock),
													   std::placeholders::_1, std::placeholders::_2));

		am_Availability_s availability;
		availability.availability = A_MAX;
		availability.availabilityReason = AR_MAX;

		org::genivi::am::am_Availability_s destination(org::genivi::am::am_Availability_e::A_MAX, static_cast<org::genivi::am::am_AvailabilityReason_pe>(AR_MAX));

		EXPECT_CALL(mock, onSinkAvailabilityChangedEvent(101, destination));
		env->mpPlugin->cbSinkAvailabilityChanged(101, availability);
		SIMPLE_THREADS_SYNC_MICROSEC();
		env->mProxy->getSinkAvailabilityChangedEvent().unsubscribe(subscription);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

TEST_F(CAmCommandSenderCAPITest, onVolumeChangedEventTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		MockNotificationsClient mock;
		auto subscription = env->mProxy->getVolumeChangedEvent().subscribe(std::bind(&MockNotificationsClient::onVolumeChangedEvent, std::ref(mock),
													   std::placeholders::_1, std::placeholders::_2));
		EXPECT_CALL(mock, onVolumeChangedEvent(101, 4));
		env->mpPlugin->cbVolumeChanged(101, 4);
		SIMPLE_THREADS_SYNC_MICROSEC();
		env->mProxy->getVolumeChangedEvent().unsubscribe(subscription);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

TEST_F(CAmCommandSenderCAPITest, onSinkMuteStateChangedEventTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		MockNotificationsClient mock;
		auto subscription = env->mProxy->getSinkMuteStateChangedEvent().subscribe(std::bind(&MockNotificationsClient::onSinkMuteStateChangedEvent, std::ref(mock),
													   std::placeholders::_1, std::placeholders::_2));
		EXPECT_CALL(mock, onSinkMuteStateChangedEvent(101, org::genivi::am::am_MuteState_e::MS_MAX));
		env->mpPlugin->cbSinkMuteStateChanged(101, am_MuteState_e::MS_MAX);
		SIMPLE_THREADS_SYNC_MICROSEC();
		env->mProxy->getSinkMuteStateChangedEvent().unsubscribe(subscription);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

TEST_F(CAmCommandSenderCAPITest, onSystemPropertyChangedEventTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		MockNotificationsClient mock;
		auto subscription = env->mProxy->getSystemPropertyChangedEvent().subscribe(std::bind(&MockNotificationsClient::onSystemPropertyChangedEvent, std::ref(mock),
													   std::placeholders::_1));

		org::genivi::am::am_SystemProperty_s value(static_cast<org::genivi::am::am_SystemPropertyType_pe>(SYP_UNKNOWN), (const int16_t)2);
		am_SystemProperty_s systemProperty;
		systemProperty.value = 2;
		systemProperty.type = am_SystemPropertyType_e::SYP_UNKNOWN;

		EXPECT_CALL(mock, onSystemPropertyChangedEvent(value));
		env->mpPlugin->cbSystemPropertyChanged(systemProperty);
		SIMPLE_THREADS_SYNC_MICROSEC();
		env->mProxy->getSystemPropertyChangedEvent().unsubscribe(subscription);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

TEST_F(CAmCommandSenderCAPITest, onTimingInformationChangedEventTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		MockNotificationsClient mock;
		auto subscription = env->mProxy->getTimingInformationChangedEvent().subscribe(std::bind(&MockNotificationsClient::onTimingInformationChangedEvent, std::ref(mock),
													   std::placeholders::_1, std::placeholders::_2));

		EXPECT_CALL(mock, onTimingInformationChangedEvent(1, 2));
		env->mpPlugin->cbTimingInformationChanged(1, 2);
		SIMPLE_THREADS_SYNC_MICROSEC();
		env->mProxy->getTimingInformationChangedEvent().unsubscribe(subscription);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

TEST_F(CAmCommandSenderCAPITest, onSinkUpdatedEventTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		MockNotificationsClient mock;
		auto subscription = env->mProxy->getSinkUpdatedEvent().subscribe(std::bind(&MockNotificationsClient::onSinkUpdatedEvent, std::ref(mock),
													   std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		std::vector<am_MainSoundProperty_s> listMainSoundProperties;
		am_MainSoundProperty_s prop;
		prop.value = 1;
		prop.type = am_MainSoundPropertyType_e::MSP_MAX;
		listMainSoundProperties.push_back(prop);
		EXPECT_CALL(mock, onSinkUpdatedEvent(1, 2, _));
		env->mpPlugin->cbSinkUpdated(1, 2, listMainSoundProperties);
		SIMPLE_THREADS_SYNC_MICROSEC();
		env->mProxy->getSinkUpdatedEvent().unsubscribe(subscription);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

TEST_F(CAmCommandSenderCAPITest, onSourceUpdatedTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		MockNotificationsClient mock;
		auto subscription = env->mProxy->getSourceUpdatedEvent().subscribe(std::bind(&MockNotificationsClient::onSourceUpdatedEvent, std::ref(mock),
													   std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		std::vector<am_MainSoundProperty_s> listMainSoundProperties;
		am_MainSoundProperty_s prop;
		prop.value = 1;
		prop.type = am_MainSoundPropertyType_e::MSP_MAX;
		listMainSoundProperties.push_back(prop);
		EXPECT_CALL(mock, onSourceUpdatedEvent(1, 2, _));
		env->mpPlugin->cbSourceUpdated(1, 2, listMainSoundProperties);
		SIMPLE_THREADS_SYNC_MICROSEC();
		env->mProxy->getSourceUpdatedEvent().unsubscribe(subscription);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

TEST_F(CAmCommandSenderCAPITest, onSinkNotificationEventTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		MockNotificationsClient mock;
		auto subscription = env->mProxy->getSinkNotificationEvent().subscribe(std::bind(&MockNotificationsClient::onSinkNotificationEvent, std::ref(mock),
													   std::placeholders::_1, std::placeholders::_2));
		am_NotificationPayload_s orig;
		orig.type = am_NotificationType_e::NT_MAX;
		orig.value = 1;
		org::genivi::am::am_NotificationPayload_s dest;
		dest.type = static_cast<org::genivi::am::am_NotificationType_pe>(NT_MAX);
		dest.value = 1;

		EXPECT_CALL(mock, onSinkNotificationEvent(1, dest));
		env->mpPlugin->cbSinkNotification(1, orig);
		SIMPLE_THREADS_SYNC_MICROSEC();
		env->mProxy->getSinkNotificationEvent().unsubscribe(subscription);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}


TEST_F(CAmCommandSenderCAPITest, onSourceNotificationEventTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		MockNotificationsClient mock;
		auto subscription = env->mProxy->getSourceNotificationEvent().subscribe(std::bind(&MockNotificationsClient::onSourceNotificationEvent, std::ref(mock),
													   std::placeholders::_1, std::placeholders::_2));
		am_NotificationPayload_s orig;
		orig.type = am_NotificationType_e::NT_MAX;
		orig.value = 1;
		org::genivi::am::am_NotificationPayload_s dest;
		dest.type = static_cast<org::genivi::am::am_NotificationType_pe>(NT_MAX);
		dest.value = 1;

		EXPECT_CALL(mock, onSourceNotificationEvent(1, dest));
		env->mpPlugin->cbSourceNotification(1, orig);
		SIMPLE_THREADS_SYNC_MICROSEC();
		env->mProxy->getSourceNotificationEvent().unsubscribe(subscription);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

TEST_F(CAmCommandSenderCAPITest, onMainSinkNotificationConfigurationChangedEventTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		MockNotificationsClient mock;
		auto subscription = env->mProxy->getMainSinkNotificationConfigurationChangedEvent().subscribe(std::bind(&MockNotificationsClient::onMainSinkNotificationConfigurationChangedEvent, std::ref(mock),
													   std::placeholders::_1, std::placeholders::_2));
		am_NotificationConfiguration_s orig;
		orig.type = am_NotificationType_e::NT_MAX;
		orig.parameter = 1;
		orig.status = am_NotificationStatus_e::NS_MAX;
		org::genivi::am::am_NotificationConfiguration_s dest;
		dest.type = static_cast<org::genivi::am::am_NotificationType_pe>(NT_MAX);
		dest.parameter = 1;
		dest.status = org::genivi::am::am_NotificationStatus_e::NS_MAX;

		EXPECT_CALL(mock, onMainSinkNotificationConfigurationChangedEvent(1, dest));
		env->mpPlugin->cbMainSinkNotificationConfigurationChanged(1, orig);
		SIMPLE_THREADS_SYNC_MICROSEC();
		env->mProxy->getMainSinkNotificationConfigurationChangedEvent().unsubscribe(subscription);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

TEST_F(CAmCommandSenderCAPITest, onMainSourceNotificationConfigurationChangedEventTest)
{
	ASSERT_TRUE(env->mIsServiceAvailable);
	if(env->mIsServiceAvailable)
	{
		MockNotificationsClient mock;
		auto subscription = env->mProxy->getMainSourceNotificationConfigurationChangedEvent().subscribe(std::bind(&MockNotificationsClient::onMainSourceNotificationConfigurationChangedEvent, std::ref(mock),
													   std::placeholders::_1, std::placeholders::_2));
		am_NotificationConfiguration_s orig;
		orig.type = am_NotificationType_e::NT_MAX;
		orig.parameter = 1;
		orig.status = am_NotificationStatus_e::NS_MAX;
		org::genivi::am::am_NotificationConfiguration_s dest;
		dest.type =static_cast<org::genivi::am::am_NotificationType_pe>(NT_MAX);
		dest.parameter = 1;
		dest.status = org::genivi::am::am_NotificationStatus_e::NS_MAX;

		EXPECT_CALL(mock, onMainSourceNotificationConfigurationChangedEvent(1, dest));
		env->mpPlugin->cbMainSourceNotificationConfigurationChanged(1, orig);
		SIMPLE_THREADS_SYNC_MICROSEC();
		env->mProxy->getMainSourceNotificationConfigurationChangedEvent().unsubscribe(subscription);
	}
	EXPECT_TRUE(Mock::VerifyAndClearExpectations(env->mpCommandReceive));
}

