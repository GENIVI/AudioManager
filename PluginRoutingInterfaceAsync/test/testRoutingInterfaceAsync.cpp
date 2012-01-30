/**
 * Copyright (C) 2011, BMW AG
 *
 * GeniviAudioMananger AudioManagerDaemon
 *
 * \file testRoutingItnerfaceAsync.cpp
 *
 * \date 20-Oct-2011 3:42:04 PM
 * \author Christian Mueller (christian.ei.mueller@bmw.de)
 *
 * \section License
 * GNU Lesser General Public License, version 2.1, with special exception (GENIVI clause)
 * Copyright (C) 2011, BMW AG Christian Mueller  Christian.ei.mueller@bmw.de
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License, version 2.1, for more details.
 * You should have received a copy of the GNU Lesser General Public License, version 2.1, along with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 * Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may also be applicable to programs even in cases in which the program is not a library in the technical sense.
 * Linking AudioManager statically or dynamically with other modules is making a combined work based on AudioManager. You may license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to license your linked modules under the GNU Lesser General Public License, version 2.1, you may use the program under the following exception.
 * As a special exception, the copyright holders of AudioManager give you permission to combine AudioManager with software programs or libraries that are released under any license unless such a combination is not permitted by the license of such a software program or library. You may copy and distribute such a system following the terms of the GNU Lesser General Public License, version 2.1, including this special exception, for AudioManager and the licenses of the other code concerned.
 * Note that people who make modified versions of AudioManager are not obligated to grant this special exception for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, version 2.1, gives permission to release a modified version without this exception; this exception also makes it possible to release a modified version which carries forward this exception.
 *
 */

#include "testRoutingInterfaceAsync.h"
#include "config.h"
#include "DLTWrapper.h"

using namespace am;
using namespace testing;

std::vector<std::string> testRoutingInterfaceAsync::pListRoutingPluginDirs = returnListPlugins();
am_domainID_t testRoutingInterfaceAsync::mDomainIDCount = 0;
RoutingSender testRoutingInterfaceAsync::pRoutingSender = RoutingSender(pListRoutingPluginDirs);

testRoutingInterfaceAsync::testRoutingInterfaceAsync() :
        pSocketHandler(), //
        pReceiveInterface(), //
        ptimerCallback(this, &testRoutingInterfaceAsync::timerCallback)
{
}

testRoutingInterfaceAsync::~testRoutingInterfaceAsync()
{
}

void testRoutingInterfaceAsync::SetUp()
{
    logInfo("RoutingSendInterface Test started ");

    std::vector<int> domainIDs;
    domainIDs.push_back(0);
    domainIDs.push_back(1);

    EXPECT_CALL(pReceiveInterface,getSocketHandler(_)).WillOnce(DoAll(SetArgReferee<0>(&pSocketHandler), Return(E_OK)));
    EXPECT_CALL(pReceiveInterface,registerDomain(_,_)).WillRepeatedly(Invoke(testRoutingInterfaceAsync::handleDomainRegister));
    EXPECT_CALL(pReceiveInterface,registerSource(_,_)).WillRepeatedly(Invoke(testRoutingInterfaceAsync::handleSourceRegister));
    EXPECT_CALL(pReceiveInterface,registerSink(_,_)).WillRepeatedly(Invoke(testRoutingInterfaceAsync::handleSinkRegister));

    pRoutingSender.startupRoutingInterface(&pReceiveInterface);
    pRoutingSender.routingInterfacesReady();

    timespec t;
    t.tv_nsec = 0;
    t.tv_sec = 4;

    sh_timerHandle_t handle;

    shTimerCallBack *buf = &ptimerCallback;
    //lets use a timeout so the test will finish
    pSocketHandler.addTimer(t, buf, handle, (void*) NULL);
}

std::vector<std::string> am::testRoutingInterfaceAsync::returnListPlugins()
{
    std::vector<std::string> list;
    list.push_back(std::string(DEFAULT_PLUGIN_ROUTING_DIR));
    return (list);
}

am_Error_e am::testRoutingInterfaceAsync::handleSourceRegister(const am_Source_s & sourceData, am_sourceID_t & sourceID)
{
    sourceID = sourceData.sourceID;
    pRoutingSender.addSourceLookup(sourceData);
    return (E_OK);
}

am_Error_e am::testRoutingInterfaceAsync::handleSinkRegister(const am_Sink_s & sinkData, am_sinkID_t & sinkID)
{
    sinkID = sinkData.sinkID;
    pRoutingSender.addSinkLookup(sinkData);
    return (E_OK);
}

am_Error_e am::testRoutingInterfaceAsync::handleDomainRegister(const am_Domain_s & domainData, am_domainID_t & domainID)
{
    am_Domain_s domain = domainData;
    domainID = mDomainIDCount++;
    domain.domainID = domainID;
    pRoutingSender.addDomainLookup(domain);
    return (E_OK);
}

void am::testRoutingInterfaceAsync::timerCallback(sh_timerHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    pSocketHandler.stop_listening();
}

void testRoutingInterfaceAsync::TearDown()
{
}

TEST_F(testRoutingInterfaceAsync,setDomainState)
{
    am_domainID_t domainID = 1;
    am_DomainState_e state = DS_INDEPENDENT_RUNDOWN;

    EXPECT_CALL(pReceiveInterface,hookDomainStateChange(_,DS_INDEPENDENT_RUNDOWN)).Times(1);

    ASSERT_EQ(E_OK, pRoutingSender.setDomainState(domainID,state));
    pSocketHandler.start_listenting();
}

TEST_F(testRoutingInterfaceAsync,setSourceSoundProperty)
{

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_SETSOURCESOUNDPROPERTY;

    am_sourceID_t sourceID = 3;
    am_SoundProperty_s property;
    property.type = SP_MID;
    property.value = 24;

    EXPECT_CALL(pReceiveInterface,ackSetSourceSoundProperty(_,E_OK)).Times(1);

    ASSERT_EQ(E_OK, pRoutingSender.asyncSetSourceSoundProperty(handle,sourceID,property));
    pSocketHandler.start_listenting();
}

TEST_F(testRoutingInterfaceAsync,setSinkSoundProperty)
{

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_SETSINKSOUNDPROPERTY;

    am_sinkID_t sinkID = 1;
    am_SoundProperty_s property;
    property.type = SP_MID;
    property.value = 24;

    EXPECT_CALL(pReceiveInterface,ackSetSinkSoundProperty(_,E_OK)).Times(1);

    ASSERT_EQ(E_OK, pRoutingSender.asyncSetSinkSoundProperty(handle,sinkID,property));
    pSocketHandler.start_listenting();
}

TEST_F(testRoutingInterfaceAsync,setSourceState)
{

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_SETSOURCEVOLUME;

    am_sourceID_t sourceID = 1;
    am_SourceState_e state = SS_OFF;

    EXPECT_CALL(pReceiveInterface,ackSetSourceState(_,E_OK)).Times(1);

    ASSERT_EQ(E_OK, pRoutingSender.asyncSetSourceState(handle,sourceID,state));
    pSocketHandler.start_listenting();
}

TEST_F(testRoutingInterfaceAsync,setSourceVolume)
{

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_SETSOURCEVOLUME;

    am_sourceID_t sourceID = 3;
    am_volume_t volume = 3;
    am_RampType_e ramp = RAMP_DIRECT;
    am_time_t myTime = 25;

    EXPECT_CALL(pReceiveInterface,ackSourceVolumeTick(_,sourceID,_)).Times(3);
    EXPECT_CALL(pReceiveInterface,ackSetSourceVolumeChange(_,volume,E_OK)).Times(1);

    ASSERT_EQ(E_OK, pRoutingSender.asyncSetSourceVolume(handle,sourceID,volume,ramp,myTime));
    pSocketHandler.start_listenting();
}

TEST_F(testRoutingInterfaceAsync,setSinkVolume)
{

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_SETSINKVOLUME;

    am_sinkID_t sinkID = 1;
    am_volume_t volume = 9;
    am_RampType_e ramp = RAMP_DIRECT;
    am_time_t myTime = 25;

    EXPECT_CALL(pReceiveInterface,ackSinkVolumeTick(_,sinkID,_)).Times(9);
    EXPECT_CALL(pReceiveInterface,ackSetSinkVolumeChange(_,volume,E_OK)).Times(1);

    ASSERT_EQ(E_OK, pRoutingSender.asyncSetSinkVolume(handle,sinkID,volume,ramp,myTime));
    pSocketHandler.start_listenting();
}

TEST_F(testRoutingInterfaceAsync,setSinkVolumeAbort)
{

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_SETSINKVOLUME;

    am_sinkID_t sinkID = 2;
    am_volume_t volume = 25;
    am_RampType_e ramp = RAMP_DIRECT;
    am_time_t myTime = 25;

    EXPECT_CALL(pReceiveInterface, ackSinkVolumeTick(_,sinkID,_));
    EXPECT_CALL(pReceiveInterface,ackSetSinkVolumeChange(_,AllOf(Ne(volume),Ne(0)),E_ABORTED)).Times(1);

    ASSERT_EQ(E_OK, pRoutingSender.asyncSetSinkVolume(handle,sinkID,volume,ramp,myTime));
    sleep(0.5);
    ASSERT_EQ(E_OK, pRoutingSender.asyncAbort(handle));
    pSocketHandler.start_listenting();
}

TEST_F(testRoutingInterfaceAsync,disconnectTooEarly)
{

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_CONNECT;

    am_connectionID_t connectionID = 4;
    am_sourceID_t sourceID = 2;
    am_sinkID_t sinkID = 1;
    am_ConnectionFormat_e format = CF_ANALOG;

    EXPECT_CALL(pReceiveInterface, ackConnect(_,connectionID,E_OK));
    EXPECT_CALL(pReceiveInterface,ackDisconnect(_,connectionID,E_OK)).Times(0);
    ASSERT_EQ(E_OK, pRoutingSender.asyncConnect(handle,connectionID,sourceID,sinkID,format));
    ASSERT_EQ(E_NON_EXISTENT, pRoutingSender.asyncDisconnect(handle,connectionID));
    pSocketHandler.start_listenting();
}

TEST_F(testRoutingInterfaceAsync,disconnectAbort)
{

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_CONNECT;

    am_connectionID_t connectionID = 4;
    am_sourceID_t sourceID = 2;
    am_sinkID_t sinkID = 1;
    am_ConnectionFormat_e format = CF_ANALOG;

    EXPECT_CALL(pReceiveInterface, ackConnect(_,connectionID,E_OK));
    EXPECT_CALL(pReceiveInterface, ackDisconnect(_,connectionID,E_ABORTED));
    ASSERT_EQ(E_OK, pRoutingSender.asyncConnect(handle,connectionID,sourceID,sinkID,format));
    sleep(2);
    ASSERT_EQ(E_OK, pRoutingSender.asyncDisconnect(handle,connectionID));
    ASSERT_EQ(E_OK, pRoutingSender.asyncAbort(handle));
    pSocketHandler.start_listenting();
}

TEST_F(testRoutingInterfaceAsync,disconnectNonExisting)
{

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_CONNECT;

    am_connectionID_t connectionID = 4;

    EXPECT_CALL(pReceiveInterface,ackConnect(_,connectionID,E_OK)).Times(0);
    EXPECT_CALL(pReceiveInterface,ackDisconnect(_,connectionID,E_OK)).Times(0);
    ASSERT_EQ(E_NON_EXISTENT, pRoutingSender.asyncDisconnect(handle,connectionID));
    pSocketHandler.start_listenting();
}

TEST_F(testRoutingInterfaceAsync,disconnect)
{

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_CONNECT;

    am_connectionID_t connectionID = 4;
    am_sourceID_t sourceID = 2;
    am_sinkID_t sinkID = 1;
    am_ConnectionFormat_e format = CF_ANALOG;

    EXPECT_CALL(pReceiveInterface, ackConnect(_,connectionID,E_OK));
    EXPECT_CALL(pReceiveInterface, ackDisconnect(_,connectionID,E_OK));
    ASSERT_EQ(E_OK, pRoutingSender.asyncConnect(handle,connectionID,sourceID,sinkID,format));
    sleep(2);
    ASSERT_EQ(E_OK, pRoutingSender.asyncDisconnect(handle,connectionID));
    pSocketHandler.start_listenting();
}

TEST_F(testRoutingInterfaceAsync,connectNoMoreThreads)
{

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_CONNECT;

    am_connectionID_t connectionID = 1;
    am_sourceID_t sourceID = 2;
    am_sinkID_t sinkID = 1;
    am_ConnectionFormat_e format = CF_ANALOG;

    EXPECT_CALL(pReceiveInterface,ackConnect(_,_,E_OK)).Times(10);
    for (int i = 0; i < 10; i++)
    {
        handle.handle++;
        connectionID++;
        ASSERT_EQ(E_OK, pRoutingSender.asyncConnect(handle,connectionID,sourceID,sinkID,format));
    }ASSERT_EQ(E_NOT_POSSIBLE, pRoutingSender.asyncConnect(handle,connectionID,sourceID,sinkID,format));
    pSocketHandler.start_listenting();
}

TEST_F(testRoutingInterfaceAsync,connectAbortTooLate)
{

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_CONNECT;

    am_connectionID_t connectionID = 4;
    am_sourceID_t sourceID = 2;
    am_sinkID_t sinkID = 1;
    am_ConnectionFormat_e format = CF_ANALOG;

    EXPECT_CALL(pReceiveInterface,ackConnect(_,connectionID,E_OK)).Times(1);
    ASSERT_EQ(E_OK, pRoutingSender.asyncConnect(handle,connectionID,sourceID,sinkID,format));
    sleep(3);
    ASSERT_EQ(E_NON_EXISTENT, pRoutingSender.asyncAbort(handle));
    pSocketHandler.start_listenting();
}

TEST_F(testRoutingInterfaceAsync,connectAbort)
{

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_CONNECT;

    am_connectionID_t connectionID = 4;
    am_sourceID_t sourceID = 2;
    am_sinkID_t sinkID = 1;
    am_ConnectionFormat_e format = CF_ANALOG;

    EXPECT_CALL(pReceiveInterface,ackConnect(_,connectionID,E_ABORTED)).Times(1);
    ASSERT_EQ(E_OK, pRoutingSender.asyncConnect(handle,connectionID,sourceID,sinkID,format));
    sleep(0.5);
    ASSERT_EQ(E_OK, pRoutingSender.asyncAbort(handle));
    pSocketHandler.start_listenting();
}

TEST_F(testRoutingInterfaceAsync,connectWrongFormat)
{

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_CONNECT;

    am_connectionID_t connectionID = 4;
    am_sourceID_t sourceID = 2;
    am_sinkID_t sinkID = 1;
    am_ConnectionFormat_e format = CF_MONO;

    EXPECT_CALL(pReceiveInterface,ackConnect(_,connectionID,E_OK)).Times(0);
    ASSERT_EQ(E_WRONG_FORMAT, pRoutingSender.asyncConnect(handle,connectionID,sourceID,sinkID,format));
    pSocketHandler.start_listenting();
}

TEST_F(testRoutingInterfaceAsync,connectWrongSink)
{

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_CONNECT;

    am_connectionID_t connectionID = 4;
    am_sourceID_t sourceID = 2;
    am_sinkID_t sinkID = 122;
    am_ConnectionFormat_e format = CF_ANALOG;

    EXPECT_CALL(pReceiveInterface,ackConnect(_,connectionID,E_OK)).Times(0);
    ASSERT_EQ(E_NON_EXISTENT, pRoutingSender.asyncConnect(handle,connectionID,sourceID,sinkID,format));
    pSocketHandler.start_listenting();
}

TEST_F(testRoutingInterfaceAsync,connectWrongSource)
{
    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_CONNECT;

    am_connectionID_t connectionID = 4;
    am_sourceID_t sourceID = 25;
    am_sinkID_t sinkID = 1;
    am_ConnectionFormat_e format = CF_ANALOG;

    EXPECT_CALL(pReceiveInterface,ackConnect(_,connectionID,E_OK)).Times(0);
    ASSERT_EQ(E_NON_EXISTENT, pRoutingSender.asyncConnect(handle,connectionID,sourceID,sinkID,format));
    pSocketHandler.start_listenting();
}

TEST_F(testRoutingInterfaceAsync,connect)
{

    am_Handle_s handle;
    handle.handle = 1;
    handle.handleType = H_CONNECT;

    am_connectionID_t connectionID = 4;
    am_sourceID_t sourceID = 2;
    am_sinkID_t sinkID = 1;
    am_ConnectionFormat_e format = CF_ANALOG;

    EXPECT_CALL(pReceiveInterface, ackConnect(_,connectionID,E_OK));
    ASSERT_EQ(E_OK, pRoutingSender.asyncConnect(handle,connectionID,sourceID,sinkID,format));
    pSocketHandler.start_listenting();
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

