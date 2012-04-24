/**
 *  Copyright (c) 2012 BMW
 *
 *  \author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
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

#include "CAmRoutingReceiverAsync.h"
#include "config.h"
#include "CAmRoutingReceiver.h"
#include "TAmPluginTemplate.h"
#include "MockIAmRoutingReceive.h"
#include "shared/CAmDltWrapper.h"
#include "routing/IAmRoutingSend.h"

using namespace am;
using namespace testing;

am_domainID_t CAmEnvironment::mDomainIDCount = 0;
static IAmRoutingSend* pRouter;
static CAmSocketHandler pSocketHandler;
static MockIAmRoutingReceive pReceiveInterface;

CAmEnvironment::CAmEnvironment() :
        ptimerCallback(this, &CAmEnvironment::timerCallback)
{
    DefaultValue<am_Error_e>::Set(E_OK); // Sets the default value to be returned.
}

CAmEnvironment::~CAmEnvironment()
{
}

void CAmEnvironment::SetUp()
{
    logInfo("RoutingSendInterface Test started ");

    std::vector<int> domainIDs;
    domainIDs.push_back(0);
    domainIDs.push_back(1);

    EXPECT_CALL(pReceiveInterface,getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(&pSocketHandler), Return(E_OK)));
    EXPECT_CALL(pReceiveInterface,registerDomain(_,_)).WillRepeatedly(Invoke(CAmEnvironment::handleDomainRegister));
    EXPECT_CALL(pReceiveInterface,registerSource(_,_)).WillRepeatedly(Invoke(CAmEnvironment::handleSourceRegister));
    EXPECT_CALL(pReceiveInterface,registerSink(_,_)).WillRepeatedly(Invoke(CAmEnvironment::handleSinkRegister));
    EXPECT_CALL(pReceiveInterface,confirmRoutingReady(_)).Times(1);

    IAmRoutingSend* (*createFunc)();
    void* tempLibHandle = NULL;
    std::string libname("../plugins/routing/libPluginRoutingInterfaceAsync.so");
    createFunc = getCreateFunction<IAmRoutingSend*()>(libname, tempLibHandle);

    if (!createFunc)
    {
        logError("RoutingSendInterface Test Entry point of RoutingPlugin not found");
        exit(1);
    }

    pRouter = createFunc();

    if (!pRouter)
    {
        logError("RoutingSendInterface Test RoutingPlugin initialization failed. Entry Function not callable");
        exit(1);
    }

    pRouter->startupInterface(&pReceiveInterface);
    pRouter->setRoutingReady(10);

    timespec t;
    t.tv_nsec = 500000000;
    t.tv_sec = 1;

    sh_timerHandle_t handle;

    //lets use a timeout so the test will finish
    pSocketHandler.addTimer(t, &ptimerCallback, handle, (void*) NULL);
    pSocketHandler.start_listenting();

}

void CAmEnvironment::TearDown()
{

}

CAmRoutingReceiverAsync::CAmRoutingReceiverAsync() :
        ptimerCallback(this, &CAmRoutingReceiverAsync::timerCallback)
{
}

CAmRoutingReceiverAsync::~CAmRoutingReceiverAsync()
{
}

void CAmRoutingReceiverAsync::timerCallback(sh_timerHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    pSocketHandler.stop_listening();
}

void CAmRoutingReceiverAsync::SetUp()
{
//    timespec t;
//    t.tv_nsec = 0;
//    t.tv_sec = 2;
//
//    sh_timerHandle_t handle;
//
//    shTimerCallBack *buf = &ptimerCallback;
//    //lets use a timeout so the test will finish
//    pSocketHandler.addTimer(t, buf, handle, (void*) NULL);
}

std::vector<std::string> CAmEnvironment::returnListPlugins()
{
    std::vector<std::string> list;
    list.push_back(std::string(DEFAULT_PLUGIN_ROUTING_DIR));
    return (list);
}

am_Error_e CAmEnvironment::handleSourceRegister(const am_Source_s & sourceData, am_sourceID_t & sourceID)
{
    sourceID = sourceData.sourceID;
    return (E_OK);
}

am_Error_e CAmEnvironment::handleSinkRegister(const am_Sink_s & sinkData, am_sinkID_t & sinkID)
{
    sinkID = sinkData.sinkID;
    return (E_OK);
}

am_Error_e CAmEnvironment::handleDomainRegister(const am_Domain_s & domainData, am_domainID_t & domainID)
{
    am_Domain_s domain = domainData;
    domainID = ++mDomainIDCount;
    domain.domainID = domainID;
    return (E_OK);
}

void CAmEnvironment::timerCallback(sh_timerHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    pSocketHandler.restartTimer(handle);
    pSocketHandler.stop_listening();
}

void CAmRoutingReceiverAsync::TearDown()
{
}

TEST_F(CAmRoutingReceiverAsync,setDomainState)
{
    am_domainID_t domainID = 1;
    am_DomainState_e state = DS_INDEPENDENT_RUNDOWN;

    EXPECT_CALL(pReceiveInterface,hookDomainStateChange(_,DS_INDEPENDENT_RUNDOWN)).Times(1);

    ASSERT_EQ(E_OK, pRouter->setDomainState(domainID,state));
    pSocketHandler.start_listenting();
}

TEST_F(CAmRoutingReceiverAsync,setSourceSoundProperty)
{

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_SETSOURCESOUNDPROPERTY;

    am_sourceID_t sourceID = 3;
    am_SoundProperty_s property;
    property.type = SP_EXAMPLE_MID;
    property.value = 24;

    EXPECT_CALL(pReceiveInterface,ackSetSourceSoundProperty(_,E_OK)).Times(1);

    ASSERT_EQ(E_OK, pRouter->asyncSetSourceSoundProperty(handle,sourceID,property));
    pSocketHandler.start_listenting();
}

TEST_F(CAmRoutingReceiverAsync,setSinkSoundProperty)
{

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_SETSINKSOUNDPROPERTY;

    am_sinkID_t sinkID = 1;
    am_SoundProperty_s property;
    property.type = SP_EXAMPLE_MID;
    property.value = 24;

    EXPECT_CALL(pReceiveInterface,ackSetSinkSoundProperty(_,E_OK)).Times(1);

    ASSERT_EQ(E_OK, pRouter->asyncSetSinkSoundProperty(handle,sinkID,property));
    pSocketHandler.start_listenting();
}

TEST_F(CAmRoutingReceiverAsync,setSourceState)
{

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_SETSOURCESTATE;

    am_sourceID_t sourceID = 1;
    am_SourceState_e state = SS_OFF;

    EXPECT_CALL(pReceiveInterface,ackSetSourceState(_,E_OK)).Times(1);

    ASSERT_EQ(E_OK, pRouter->asyncSetSourceState(handle,sourceID,state));
    pSocketHandler.start_listenting();
}

TEST_F(CAmRoutingReceiverAsync,setSourceVolume)
{

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_SETSOURCEVOLUME;

    am_sourceID_t sourceID = 3;
    am_volume_t volume = 3;
    am_RampType_e ramp = RAMP_GENIVI_DIRECT;
    am_time_t myTime = 25;

    EXPECT_CALL(pReceiveInterface,ackSourceVolumeTick(_,sourceID,_)).Times(AtLeast(1));
    EXPECT_CALL(pReceiveInterface,ackSetSourceVolumeChange(_,volume,E_OK)).Times(1);

    ASSERT_EQ(E_OK, pRouter->asyncSetSourceVolume(handle,sourceID,volume,ramp,myTime));
    pSocketHandler.start_listenting();
}

TEST_F(CAmRoutingReceiverAsync,setSinkVolume)
{

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_SETSINKVOLUME;

    am_sinkID_t sinkID = 1;
    am_volume_t volume = 9;
    am_RampType_e ramp = RAMP_GENIVI_DIRECT;
    am_time_t myTime = 25;

    EXPECT_CALL(pReceiveInterface,ackSinkVolumeTick(_,sinkID,_)).Times(AtLeast(2));
    EXPECT_CALL(pReceiveInterface,ackSetSinkVolumeChange(_,volume,E_OK)).Times(1);

    ASSERT_EQ(E_OK, pRouter->asyncSetSinkVolume(handle,sinkID,volume,ramp,myTime));
    pSocketHandler.start_listenting();
}

TEST_F(CAmRoutingReceiverAsync,setSinkVolumeAbort)
{

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_SETSINKVOLUME;

    am_sinkID_t sinkID = 2;
    am_volume_t volume = 25;
    am_RampType_e ramp = RAMP_GENIVI_DIRECT;
    am_time_t myTime = 25;

    EXPECT_CALL(pReceiveInterface, ackSinkVolumeTick(_,sinkID,_));
    EXPECT_CALL(pReceiveInterface,ackSetSinkVolumeChange(_,AllOf(Ne(volume),Ne(0)),E_ABORTED)).Times(1);

    ASSERT_EQ(E_OK, pRouter->asyncSetSinkVolume(handle,sinkID,volume,ramp,myTime));
    sleep(0.5);
    ASSERT_EQ(E_OK, pRouter->asyncAbort(handle));
    pSocketHandler.start_listenting();
}

TEST_F(CAmRoutingReceiverAsync,disconnectNonExisting)
{

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_DISCONNECT;

    am_connectionID_t connectionID = 4;

    EXPECT_CALL(pReceiveInterface,ackConnect(_,connectionID,E_OK)).Times(0);
    EXPECT_CALL(pReceiveInterface,ackDisconnect(_,connectionID,E_OK)).Times(0);
    ASSERT_EQ(E_NON_EXISTENT, pRouter->asyncDisconnect(handle,connectionID));
    pSocketHandler.start_listenting();
}

TEST_F(CAmRoutingReceiverAsync,disconnectTooEarly)
{

    am_Handle_s handle_c;
    handle_c.handle = 1;
    handle_c.handleType = H_CONNECT;

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_DISCONNECT;

    am_connectionID_t connectionID = 4;
    am_sourceID_t sourceID = 2;
    am_sinkID_t sinkID = 1;
    am_ConnectionFormat_e format = CF_GENIVI_ANALOG;

    EXPECT_CALL(pReceiveInterface, ackConnect(_,connectionID,E_OK));
    EXPECT_CALL(pReceiveInterface,ackDisconnect(_,connectionID,E_OK)).Times(0);
    ASSERT_EQ(E_OK, pRouter->asyncConnect(handle_c,connectionID,sourceID,sinkID,format));
    ASSERT_EQ(E_NON_EXISTENT, pRouter->asyncDisconnect(handle,connectionID));
    pSocketHandler.start_listenting();
}

TEST_F(CAmRoutingReceiverAsync,disconnectAbort)
{

    am_Handle_s handle_c;
    handle_c.handle = 1;
    handle_c.handleType = H_CONNECT;

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_DISCONNECT;

    am_connectionID_t connectionID = 5;
    am_sourceID_t sourceID = 2;
    am_sinkID_t sinkID = 1;
    am_ConnectionFormat_e format = CF_GENIVI_ANALOG;

    EXPECT_CALL(pReceiveInterface, ackConnect(_,connectionID,E_OK));
    EXPECT_CALL(pReceiveInterface, ackDisconnect(_,connectionID,E_ABORTED));
    ASSERT_EQ(E_OK, pRouter->asyncConnect(handle_c,connectionID,sourceID,sinkID,format));
    sleep(2);
    ASSERT_EQ(E_OK, pRouter->asyncDisconnect(handle,connectionID));
    ASSERT_EQ(E_OK, pRouter->asyncAbort(handle));
    pSocketHandler.start_listenting();
}

TEST_F(CAmRoutingReceiverAsync,disconnect)
{

    am_Handle_s handle_c;
    handle_c.handle = 1;
    handle_c.handleType = H_CONNECT;

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_DISCONNECT;

    am_connectionID_t connectionID = 4;
    am_sourceID_t sourceID = 2;
    am_sinkID_t sinkID = 1;
    am_ConnectionFormat_e format = CF_GENIVI_ANALOG;

    EXPECT_CALL(pReceiveInterface, ackConnect(_,connectionID,E_OK));
    EXPECT_CALL(pReceiveInterface, ackDisconnect(_,connectionID,E_OK));
    ASSERT_EQ(E_OK, pRouter->asyncConnect(handle_c,connectionID,sourceID,sinkID,format));
    sleep(2);
    ASSERT_EQ(E_OK, pRouter->asyncDisconnect(handle,connectionID));
    pSocketHandler.start_listenting();
}

TEST_F(CAmRoutingReceiverAsync,connectAbortTooLate)
{

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_CONNECT;

    am_connectionID_t connectionID = 4;
    am_sourceID_t sourceID = 2;
    am_sinkID_t sinkID = 1;
    am_ConnectionFormat_e format = CF_GENIVI_ANALOG;

    EXPECT_CALL(pReceiveInterface,ackConnect(_,connectionID,E_OK)).Times(1);
    ASSERT_EQ(E_OK, pRouter->asyncConnect(handle,connectionID,sourceID,sinkID,format));
    sleep(3);
    ASSERT_EQ(E_NON_EXISTENT, pRouter->asyncAbort(handle));
    pSocketHandler.start_listenting();
}

TEST_F(CAmRoutingReceiverAsync,connectAbort)
{

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_CONNECT;

    am_connectionID_t connectionID = 4;
    am_sourceID_t sourceID = 2;
    am_sinkID_t sinkID = 1;
    am_ConnectionFormat_e format = CF_GENIVI_ANALOG;

    EXPECT_CALL(pReceiveInterface,ackConnect(_,connectionID,E_ABORTED)).Times(1);
    ASSERT_EQ(E_OK, pRouter->asyncConnect(handle,connectionID,sourceID,sinkID,format));
    sleep(0.5);
    ASSERT_EQ(E_OK, pRouter->asyncAbort(handle));
    pSocketHandler.start_listenting();
}

TEST_F(CAmRoutingReceiverAsync,connectWrongFormat)
{

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_CONNECT;

    am_connectionID_t connectionID = 4;
    am_sourceID_t sourceID = 2;
    am_sinkID_t sinkID = 1;
    am_ConnectionFormat_e format = CF_GENIVI_MONO;

    EXPECT_CALL(pReceiveInterface,ackConnect(_,connectionID,E_OK)).Times(0);
    ASSERT_EQ(E_WRONG_FORMAT, pRouter->asyncConnect(handle,connectionID,sourceID,sinkID,format));
    pSocketHandler.start_listenting();
}

TEST_F(CAmRoutingReceiverAsync,connectWrongSink)
{

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_CONNECT;

    am_connectionID_t connectionID = 4;
    am_sourceID_t sourceID = 2;
    am_sinkID_t sinkID = 122;
    am_ConnectionFormat_e format = CF_GENIVI_ANALOG;

    EXPECT_CALL(pReceiveInterface,ackConnect(_,connectionID,E_OK)).Times(0);
    ASSERT_EQ(E_NON_EXISTENT, pRouter->asyncConnect(handle,connectionID,sourceID,sinkID,format));
    pSocketHandler.start_listenting();
}

TEST_F(CAmRoutingReceiverAsync,connectWrongSource)
{
    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_CONNECT;

    am_connectionID_t connectionID = 4;
    am_sourceID_t sourceID = 25;
    am_sinkID_t sinkID = 1;
    am_ConnectionFormat_e format = CF_GENIVI_ANALOG;

    EXPECT_CALL(pReceiveInterface,ackConnect(_,connectionID,E_OK)).Times(0);
    ASSERT_EQ(E_NON_EXISTENT, pRouter->asyncConnect(handle,connectionID,sourceID,sinkID,format));
    pSocketHandler.start_listenting();
}

TEST_F(CAmRoutingReceiverAsync,connect)
{

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_CONNECT;

    am_connectionID_t connectionID = 4;
    am_sourceID_t sourceID = 2;
    am_sinkID_t sinkID = 1;
    am_ConnectionFormat_e format = CF_GENIVI_ANALOG;

    EXPECT_CALL(pReceiveInterface, ackConnect(_,connectionID,E_OK));
    ASSERT_EQ(E_OK, pRouter->asyncConnect(handle,connectionID,sourceID,sinkID,format));
    pSocketHandler.start_listenting();
}

TEST_F(CAmRoutingReceiverAsync,connectNoMoreThreads)
{

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_CONNECT;

    am_connectionID_t connectionID = 1;
    am_sourceID_t sourceID = 2;
    am_sinkID_t sinkID = 1;
    am_ConnectionFormat_e format = CF_GENIVI_ANALOG;

    EXPECT_CALL(pReceiveInterface,ackConnect(_,_,E_OK)).Times(10);
    for (int i = 0; i < 10; i++)
    {
        handle.handle++;
        connectionID++;
        ASSERT_EQ(E_OK, pRouter->asyncConnect(handle,connectionID,sourceID,sinkID,format));
    }
    ASSERT_EQ(E_NOT_POSSIBLE, pRouter->asyncConnect(handle,connectionID,sourceID,sinkID,format));
    pSocketHandler.start_listenting();
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::Environment* const env = ::testing::AddGlobalTestEnvironment(new CAmEnvironment);
    (void) env;
    return RUN_ALL_TESTS();
}

