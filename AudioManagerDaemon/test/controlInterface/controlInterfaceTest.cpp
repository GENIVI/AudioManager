/*
 * routingInterfaceTest.cpp
 *
 *  Created on: Dec 10, 2011
 *      Author: christian
 */

#include "controlInterfaceTest.h"

using namespace am;

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
using ::testing::Assign;
using ::testing::SetArgReferee;
using ::testing::DoAll;

controlInterfaceTest::controlInterfaceTest()
	:pDatabaseHandler(),
	 pRoutingSender(),
	 pCommandSender(),
	 pMockInterface(),
	 pControlSender(),
	 pRoutingInterfaceBackdoor(),
	 pCommandInterfaceBackdoor(),
	 pControlInterfaceBackdoor(),
	 pObserver(&pCommandSender,&pRoutingSender),
	 pControlReceiver(&pDatabaseHandler,&pRoutingSender),
	 pRoutingReceiver(&pDatabaseHandler,&pRoutingSender,&pControlSender)
{
	pDatabaseHandler.registerObserver(&pObserver);
	pRoutingInterfaceBackdoor.unloadPlugins(&pRoutingSender);
	pCommandInterfaceBackdoor.unloadPlugins(&pCommandSender);
	pControlInterfaceBackdoor.replaceController(&pControlSender,&pMockInterface);

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
	EXPECT_CALL(pMockInterface,hookSystemRegisterDomain(_,_)).WillRepeatedly(DoAll(SetArgReferee<1>(2),Return(E_OK)));
	ASSERT_EQ(E_OK,pRoutingReceiver.registerDomain(domain,domainID));
}


int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

