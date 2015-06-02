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
* \author Aleksandar Donchev, aleksander.donchev@partner.bmw.de BMW 2013
 *
 * \file CAmNodeStateCommunicatorTest.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include "CAmNodeStateCommunicatorTest.h"
#include "CAmDltWrapper.h"
#include "CAmSocketHandler.h"

#include <unistd.h>

using namespace testing;
using namespace am;

static CAmEnvironment* env;


CAmNodeStateCommunicatorTest::CAmNodeStateCommunicatorTest()
{
	std::cout<<"Exec path : " << EXECUTABLE_OUTPUT_PATH <<std::endl;
}

CAmNodeStateCommunicatorTest::~CAmNodeStateCommunicatorTest()
{
    // TODO Auto-generated destructor stub
}

/**This is the thread for the nsm python fake
 *
 * @param
 */
void* nsmThread (void*)
{
   if(-1 == system("python nsm.py"))
	   logError("Something went wrong with nsm.py!");
   return (NULL);
}

/**this is the thread the mainloop runs in
 *
 * @param importHandler
 */
void* mainLoop(void* importHandler)
{
    CAmSocketHandler* handler=static_cast<CAmSocketHandler*>(importHandler);
    handler->start_listenting();
    return (NULL);
}


TEST_F(CAmNodeStateCommunicatorTest, nsmChangeNodeState)
{
    assert(true == env->pControlInterfaceBackdoor.replaceController(&env->pControlSender,&pMockControlInterface));
    NsmNodeState_e newstate(NsmNodeState_BaseRunning) ;
    EXPECT_CALL(pMockControlInterface,hookSystemNodeStateChanged(newstate));
    std::ostringstream send;
    send<<"python send2nsm.py nodeState "<<static_cast<std::int32_t>(newstate);
    system(send.str().c_str());
}

TEST_F(CAmNodeStateCommunicatorTest, nsmChangeApplicationMode)
{
    env->pControlInterfaceBackdoor.replaceController(&env->pControlSender,&pMockControlInterface);
    NsmApplicationMode_e appmode(NsmApplicationMode_Swl) ;
    EXPECT_CALL(pMockControlInterface,hookSystemNodeApplicationModeChanged(appmode));
    std::ostringstream send;
    send<<"python send2nsm.py appMode "<<static_cast<std::int32_t>(appmode);
    system(send.str().c_str());
}

TEST_F(CAmNodeStateCommunicatorTest, nsmChangeSessionState)
{
    env->pControlInterfaceBackdoor.replaceController(&env->pControlSender,&pMockControlInterface);
    std::string sessionName("mySession");
    NsmSeat_e seatID(NsmSeat_CoDriver);
    NsmSessionState_e sessionState(NsmSessionState_Inactive) ;
    EXPECT_CALL(pMockControlInterface,hookSystemSessionStateChanged(sessionName,seatID,sessionState));
    std::ostringstream send;
    send<<"python send2nsm.py sessionState "<<sessionName<<" "<<static_cast<std::int32_t>(seatID)<<" "<<static_cast<int32_t>(sessionState);
    system(send.str().c_str());
}

TEST_F(CAmNodeStateCommunicatorTest, getRestartReason)
{
    env->pControlInterfaceBackdoor.replaceController(&env->pControlSender,&pMockControlInterface);
    NsmRestartReason_e restartReason;
    ASSERT_EQ(E_OK,env->nsmController.nsmGetRestartReasonProperty(restartReason));
    ASSERT_EQ(restartReason,1);
}

TEST_F(CAmNodeStateCommunicatorTest, getShutdownReason)
{
    env->pControlInterfaceBackdoor.replaceController(&env->pControlSender,&pMockControlInterface);
    NsmShutdownReason_e ShutdownReason;
    ASSERT_EQ(E_OK,env->nsmController.nsmGetShutdownReasonProperty(ShutdownReason));
    ASSERT_EQ(ShutdownReason,2);
}

TEST_F(CAmNodeStateCommunicatorTest, getWakeUpReason)
{
    env->pControlInterfaceBackdoor.replaceController(&env->pControlSender,&pMockControlInterface);
    NsmRunningReason_e WakeUpReason;
    ASSERT_EQ(E_OK,env->nsmController.nsmGetRunningReasonProperty(WakeUpReason));
    ASSERT_EQ(WakeUpReason,3);
}

TEST_F(CAmNodeStateCommunicatorTest, getNodeState)
{
    env->pControlInterfaceBackdoor.replaceController(&env->pControlSender,&pMockControlInterface);
    NsmNodeState_e nodeState;
    ASSERT_EQ(NsmErrorStatus_e::NsmErrorStatus_Ok,env->nsmController.nsmGetNodeState(nodeState));
    ASSERT_EQ(nodeState,1);
}

TEST_F(CAmNodeStateCommunicatorTest, getApplicationMode)
{
    env->pControlInterfaceBackdoor.replaceController(&env->pControlSender,&pMockControlInterface);
    NsmApplicationMode_e applicationMode;
    ASSERT_EQ(NsmErrorStatus_e::NsmErrorStatus_Error,env->nsmController.nsmGetApplicationMode(applicationMode));
    ASSERT_EQ(applicationMode,5);
}

TEST_F(CAmNodeStateCommunicatorTest, getSessionState)
{
    env->pControlInterfaceBackdoor.replaceController(&env->pControlSender,&pMockControlInterface);
    std::string sessionName("mySession");
    NsmSeat_e seatID(NsmSeat_Driver);
    NsmSessionState_e sessionState;
    ASSERT_EQ(NsmErrorStatus_e::NsmErrorStatus_Ok,env->nsmController.nsmGetSessionState(sessionName,seatID,sessionState));
    ASSERT_EQ(sessionState,5);
}

TEST_F(CAmNodeStateCommunicatorTest, RegisterShutdownClient)
{
    env->pControlInterfaceBackdoor.replaceController(&env->pControlSender,&pMockControlInterface);
    uint32_t shutdownmode(1), timeoutMs(100);
    ASSERT_EQ(NsmErrorStatus_e::NsmErrorStatus_Ok,env->nsmController.nsmRegisterShutdownClient(shutdownmode,timeoutMs));
}

TEST_F(CAmNodeStateCommunicatorTest, receiveLifecycleRequest)
{
    env->pControlInterfaceBackdoor.replaceController(&env->pControlSender,&pMockControlInterface);
    uint32_t shutdownmode(1);
    uint32_t timeoutMs(100);
    int32_t Request(1);
    int32_t RequestID(4);
    EXPECT_CALL(pMockControlInterface,hookSystemLifecycleRequest(_,RequestID)).WillOnce(Return(NsmErrorStatus_e::NsmErrorStatus_Ok));
    ASSERT_EQ(NsmErrorStatus_e::NsmErrorStatus_Ok,env->nsmController.nsmRegisterShutdownClient(shutdownmode,timeoutMs));
    std::ostringstream send;
    send << "python send2nsm.py LifecycleRequest "<<Request<<" "<<RequestID;
    system(send.str().c_str());
    sleep(2);
}

TEST_F(CAmNodeStateCommunicatorTest, UnRegisterShutdownClient)
{
    env->pControlInterfaceBackdoor.replaceController(&env->pControlSender,&pMockControlInterface);
    uint32_t shutdownmode(1),timeoutMs(100);
    ASSERT_EQ(NsmErrorStatus_e::NsmErrorStatus_Ok,env->nsmController.nsmRegisterShutdownClient(shutdownmode,timeoutMs));
    ASSERT_EQ(NsmErrorStatus_e::NsmErrorStatus_Ok,env->nsmController.nsmUnRegisterShutdownClient(shutdownmode));

}

TEST_F(CAmNodeStateCommunicatorTest, sendLifecycleRequestComplete)
{
    env->pControlInterfaceBackdoor.replaceController(&env->pControlSender,&pMockControlInterface);
    uint32_t RequestID(22);
    NsmErrorStatus_e errorStatus(NsmErrorStatus_Internal);
    ASSERT_EQ(NsmErrorStatus_e::NsmErrorStatus_Ok,env->nsmController.nsmSendLifecycleRequestComplete(RequestID,errorStatus));
}

TEST_F(CAmNodeStateCommunicatorTest, getInterfaceVersion)
{
    env->pControlInterfaceBackdoor.replaceController(&env->pControlSender,&pMockControlInterface);
    uint32_t version(0);
    ASSERT_EQ(E_OK,env->nsmController.nsmGetInterfaceVersion(version));
    ASSERT_EQ(version,static_cast<uint32_t>(23));
}

void CAmNodeStateCommunicatorTest::SetUp()
{
}

void CAmNodeStateCommunicatorTest::TearDown()
{
}

int main(int argc, char **argv)
{
    CAmDltWrapper::instance()->registerApp("nsm", "nsmtest");
    logInfo("nsmtest Test started ");
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::Environment* const env = ::testing::AddGlobalTestEnvironment(new CAmEnvironment);
    (void) env;
    return RUN_ALL_TESTS();
}

CAmEnvironment::CAmEnvironment() :
    pControlInterfaceBackdoor(),
    pControlSender(),
    iSocketHandler(),
    wrapper(CAmCommonAPIWrapper::instantiateOnce(&iSocketHandler)),
    nsmController(wrapper),
    pNsmThread(0),
    pMainLoopThread(0)
{
    env=this;
}

CAmEnvironment::~CAmEnvironment()
{
}

void CAmEnvironment::waitUntilAvailable(unsigned short seconds = 10)
{
	int countTries = 0;
	printf("\nWaiting for service");
	while( countTries++<seconds )
	{
		printf(".");
		if(nsmController.isServiceAvailable())
			break;
		else
			sleep(1);
	}
	printf("\n");
}

void CAmEnvironment::SetUp()
{
    //create the nsm thread
    pthread_create(&pNsmThread, NULL, nsmThread, NULL);
    nsmController.registerControlSender(&pControlSender);
    //create the mainloop thread
    pthread_create(&pMainLoopThread, NULL, mainLoop, (void*)&iSocketHandler);
    printf("[----------] Waiting for interface to be ready....\r\n");
    waitUntilAvailable(10);
}

void CAmEnvironment::TearDown()
{
    //end the nsm per dbus
    system("python send2nsm.py finish");
    pthread_join(pNsmThread, NULL);
    //end the mainloop
    iSocketHandler.exit_mainloop();
    pthread_join(pMainLoopThread, NULL);
}
