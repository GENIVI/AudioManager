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

#include "CAmSocketHandlerTest.h"
#include <cstdio>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/un.h>
#include <sys/poll.h>

#include "CAmSocketHandler.h"

//todo: expand test, implement more usecases
//todo: test removeFD

#define SOCK_PATH "/tmp/mysock"

#define SOCKET_TEST_LOOPS_COUNT 1000

using namespace testing;
using namespace am;

static const char * TEST_SOCKET_DATA = "Got It?";
static const char * TEST_SOCKET_DATA_FINAL = "finish!";

static const std::chrono::time_point<std::chrono::high_resolution_clock> TP_ZERO;


MockIAmSignalHandler *pMockSignalHandler = NULL;
static void signalHandler(int sig, siginfo_t *siginfo, void *context)
{
    (void) sig;
    (void) siginfo;
    (void) context;

    if(pMockSignalHandler!=NULL)
      pMockSignalHandler->signalHandler(sig, siginfo, context);
    
#ifdef ENABLED_SOCKETHANDLER_TEST_OUTPUT
        std::cout<<"signal handler was called with signal " << sig << std::endl;
#endif
}

CAmSocketHandlerTest::CAmSocketHandlerTest()
{
}

CAmSocketHandlerTest::~CAmSocketHandlerTest()
{
}

void CAmSocketHandlerTest::SetUp()
{

}

void CAmSocketHandlerTest::TearDown()
{
}

CAmTimerSockethandlerController::CAmTimerSockethandlerController(CAmSocketHandler *myHandler, const timespec &timeout) :
        MockIAmTimerCb(), mpSocketHandler(myHandler), mUpdateTimeout(timeout), pTimerCallback(this, &CAmTimerSockethandlerController::timerCallback)
{
}

am::CAmTimerSockethandlerController::~CAmTimerSockethandlerController()
{
}

void am::CAmTimerSockethandlerController::timerCallback(sh_timerHandle_t handle, void* userData)
{
    MockIAmTimerCb::timerCallback(handle, userData);
    mpSocketHandler->stop_listening();
}

CAmTimerSignalHandler::CAmTimerSignalHandler(CAmSocketHandler *myHandler, const timespec &timeout, const std::set<unsigned> & signals) :
        MockIAmTimerCb(), mIndex(0), mSignals(signals), mpSocketHandler(myHandler), mUpdateTimeout(timeout), pTimerCallback(this, &CAmTimerSignalHandler::timerCallback)
{

}

am::CAmTimerSignalHandler::~CAmTimerSignalHandler()
{
}

void am::CAmTimerSignalHandler::timerCallback(sh_timerHandle_t handle, void* userData)
{
    MockIAmTimerCb::timerCallback(handle, userData);
    if(mIndex<mSignals.size())
    {
        std::set<unsigned>::iterator it = mSignals.begin();
        std::advance(it, mIndex);
        kill(getpid(), *it);
        mIndex++;
        
#ifndef WITH_TIMERFD
         mpSocketHandler->updateTimer( handle, mUpdateTimeout);
#endif
    }
    else
        mpSocketHandler->stop_listening();
    
}

CAmTimer::CAmTimer(CAmSocketHandler *myHandler, const timespec &timeout, const int32_t repeats) :
        MockIAmTimerCb(), mpSocketHandler(myHandler), mUpdateTimeout(timeout), pTimerCallback(this, &CAmTimer::timerCallback), mRepeats(repeats)
{
}

am::CAmTimer::~CAmTimer()
{
}

void am::CAmTimer::timerCallback(sh_timerHandle_t handle, void* userData)
{
    MockIAmTimerCb::timerCallback(handle, userData);
    if (--mRepeats > 0)
    {
#ifndef WITH_TIMERFD
        mpSocketHandler->updateTimer( handle, mUpdateTimeout);
#endif
    }
    else
    {
        mpSocketHandler->stopTimer(handle);
    }
}

CAmTimerMeasurment::CAmTimerMeasurment(CAmSocketHandler *myHandler, const timespec &timeout, const std::string & label, const int32_t repeats, void * userData) :
        MockIAmTimerCb(), pTimerCallback(this, &CAmTimerMeasurment::timerCallback), //
        mSocketHandler(myHandler), mUpdateTimeout(timeout), mUpdateTimePoint(std::chrono::seconds
        { mUpdateTimeout.tv_sec } + std::chrono::nanoseconds
        { mUpdateTimeout.tv_nsec }), mLastInvocationTime(), mExpected(mUpdateTimePoint - TP_ZERO), mRepeats(repeats), mpUserData(userData), mDebugText(label)
{
}

am::CAmTimerMeasurment::~CAmTimerMeasurment()
{
}

void am::CAmTimerMeasurment::timerCallback(sh_timerHandle_t handle, void* userData)
{
    MockIAmTimerCb::timerCallback(handle, userData);

    std::chrono::time_point<std::chrono::high_resolution_clock> t_end = std::chrono::high_resolution_clock::now();
    if (TP_ZERO != mLastInvocationTime)
    {
        auto durationLast = t_end - mLastInvocationTime;
        double diff = (std::chrono::duration<double, std::milli>(mExpected - durationLast).count());

#ifdef ENABLED_TIMERS_TEST_OUTPUT
        std::cout << mDebugText <<
        " [ expected:" <<std::chrono::duration<double, std::milli>(mExpected).count() << "ms"
        " , current:" << std::chrono::duration<double, std::milli>(durationLast).count() << "ms"
        ", diff:" << diff << "ms ] " <<
        std::endl;
#endif
        if (diff > TIMERS_CB_TOLERANCE)
            std::cout << mDebugText << " Warning [ expected:" << std::chrono::duration<double, std::milli>(mExpected).count() << "ms, current:" << std::chrono::duration<double, std::milli>(durationLast).count() << "ms ]" << std::endl;
        if (diff < -TIMERS_CB_TOLERANCE)
            std::cout << mDebugText << " Warning [ expected:" << std::chrono::duration<double, std::milli>(mExpected).count() << "ms, current:" << std::chrono::duration<double, std::milli>(durationLast).count() << "ms ]" << std::endl;

        mLastInvocationTime = t_end;
        if (--mRepeats > 0)
        {
#ifndef WITH_TIMERFD
            mSocketHandler->updateTimer( handle, mUpdateTimeout);
#endif
        }
        else
        {
            mSocketHandler->stopTimer(handle);
        }
    }
    else
    {
#ifdef ENABLED_TIMERS_TEST_OUTPUT
        std::cout << mDebugText << " Init measurment " << std::endl;
#endif
        mLastInvocationTime = t_end;
        mSocketHandler->updateTimer(handle, mUpdateTimeout);
    }

}

void* playWithSocketServer(void* data)
{
    CAmSocketHandler *pSockethandler = (CAmSocketHandler*) data;
    pSockethandler->start_listenting();
    return (NULL);
}

void* playWithUnixSocketServer(void* data)
{
    CAmSocketHandler *pSockethandler = (CAmSocketHandler*) data;
    pSockethandler->start_listenting();
    return (NULL);
}

TEST(CAmSocketHandlerTest, timersOneshot)
{
    CAmSocketHandler myHandler;
    ASSERT_FALSE(myHandler.fatalErrorOccurred());
    timespec timeoutTime;
    timeoutTime.tv_sec = 1;
    timeoutTime.tv_nsec = 0;
    CAmTimer testCallback1(&myHandler, timeoutTime);

    struct TestUserData
    {
        int i;
        float f;
    };
    TestUserData userData;
    userData.i = 1;
    userData.f = 1.f;

    sh_timerHandle_t handle;
    myHandler.addTimer(timeoutTime, &testCallback1.pTimerCallback, handle, &userData);
#ifndef WITH_TIMERFD
    ASSERT_EQ(handle, 1);
#else
    ASSERT_EQ(handle, 2);
#endif
    EXPECT_CALL(testCallback1,timerCallback(handle,&userData)).Times(1);

    timespec timeout4;
    timeout4.tv_nsec = 0;
    timeout4.tv_sec = 3;
    CAmTimerSockethandlerController testCallback4(&myHandler, timeout4);

    myHandler.addTimer(timeout4, &testCallback4.pTimerCallback, handle, NULL);
#ifndef WITH_TIMERFD
    ASSERT_EQ(handle, 2);
#else
    ASSERT_EQ(handle, 3);
#endif
    EXPECT_CALL(testCallback4,timerCallback(handle,NULL)).Times(1);
    myHandler.start_listenting();
}

TEST(CAmSocketHandlerTest, timersStop)
{
    CAmSocketHandler myHandler;
    ASSERT_FALSE(myHandler.fatalErrorOccurred());
    timespec timeoutTime;
    timeoutTime.tv_sec = 1;
    timeoutTime.tv_nsec = 0;
    CAmTimer testCallback1(&myHandler, timeoutTime, 4);

    struct TestUserData
    {
        int i;
        float f;
    };
    TestUserData userData;
    userData.i = 1;
    userData.f = 1.f;

    sh_timerHandle_t handle;
    myHandler.addTimer(timeoutTime, &testCallback1.pTimerCallback, handle, &userData, true);
#ifndef WITH_TIMERFD
    ASSERT_EQ(handle, 1);
#else
    ASSERT_EQ(handle, 2);
#endif
    EXPECT_CALL(testCallback1,timerCallback(handle,&userData)).Times(4);

    timespec timeout4;
    timeout4.tv_nsec = 0;
    timeout4.tv_sec = 6;
    CAmTimerSockethandlerController testCallback4(&myHandler, timeout4);

    myHandler.addTimer(timeout4, &testCallback4.pTimerCallback, handle, NULL);
#ifndef WITH_TIMERFD
    ASSERT_EQ(handle, 2);
#else
    ASSERT_EQ(handle, 3);
#endif
    EXPECT_CALL(testCallback4,timerCallback(handle,NULL)).Times(1);
    myHandler.start_listenting();
}

TEST(CAmSocketHandlerTest, timersGeneral)
{
    CAmSocketHandler myHandler;
    ASSERT_FALSE(myHandler.fatalErrorOccurred());

    timespec timeoutTime;
    timeoutTime.tv_sec = 1;
    timeoutTime.tv_nsec = 0;
    CAmTimer testCallback1(&myHandler, timeoutTime, 4);

    struct TestUserData
    {
        int i;
        float f;
    };
    TestUserData userData;
    userData.i = 1;
    userData.f = 1.f;

    sh_timerHandle_t handle;
    myHandler.addTimer(timeoutTime, &testCallback1.pTimerCallback, handle, &userData, true);
#ifndef WITH_TIMERFD
    ASSERT_EQ(handle, 1);
#else
    ASSERT_EQ(handle, 2);
#endif
    EXPECT_CALL(testCallback1,timerCallback(handle,&userData)).Times(4); //+1 because of measurment

    timespec timeout4;
    timeout4.tv_nsec = 0;
    timeout4.tv_sec = 5;
    CAmTimerSockethandlerController testCallback4(&myHandler, timeout4);

    myHandler.addTimer(timeout4, &testCallback4.pTimerCallback, handle, NULL);
#ifndef WITH_TIMERFD
    ASSERT_EQ(handle, 2);
#else
    ASSERT_EQ(handle, 3);
#endif
    EXPECT_CALL(testCallback4,timerCallback(handle,NULL)).Times(1);
    myHandler.start_listenting();
}

TEST(CAmSocketHandlerTest,playWithTimers)
{
    CAmSocketHandler myHandler;
    ASSERT_FALSE(myHandler.fatalErrorOccurred());
    timespec timeoutTime, timeout2, timeout3, timeout4;
    timeoutTime.tv_sec = 1;
    timeoutTime.tv_nsec = 34000000;
    CAmTimerMeasurment testCallback1(&myHandler, timeoutTime, "repeatedCallback 1", std::numeric_limits<int32_t>::max());

    timeout2.tv_nsec = 2000000;
    timeout2.tv_sec = 0;
    CAmTimerMeasurment testCallback2(&myHandler, timeout2, "repeatedCallback 2", std::numeric_limits<int32_t>::max());

    timeout3.tv_nsec = 333000000;
    timeout3.tv_sec = 3;
    CAmTimerMeasurment testCallback3(&myHandler, timeout3, "oneshotCallback 3");
    timeout4.tv_nsec = 0;
    timeout4.tv_sec = 8;
    CAmTimerSockethandlerController testCallback4(&myHandler, timeout4);

    sh_timerHandle_t handle;
    myHandler.addTimer(timeoutTime, &testCallback1.pTimerCallback, handle, NULL, true);
#ifndef WITH_TIMERFD
    ASSERT_EQ(handle, 1);
#else
    ASSERT_EQ(handle, 2);
#endif
    EXPECT_CALL(testCallback1,timerCallback(handle,NULL)).Times(AnyNumber());

    myHandler.addTimer(timeout2, &testCallback2.pTimerCallback, handle, NULL, true);
#ifndef WITH_TIMERFD
    ASSERT_EQ(handle, 2);
#else
    ASSERT_EQ(handle, 3);
#endif
    EXPECT_CALL(testCallback2,timerCallback(handle,NULL)).Times(AnyNumber());

    myHandler.addTimer(timeout3, &testCallback3.pTimerCallback, handle, NULL);
#ifndef WITH_TIMERFD
    ASSERT_EQ(handle, 3);
#else
    ASSERT_EQ(handle, 4);
#endif
    EXPECT_CALL(testCallback3,timerCallback(handle,NULL)).Times(2); //+1 because of measurment

    myHandler.addTimer(timeout4, &testCallback4.pTimerCallback, handle, NULL);
#ifndef WITH_TIMERFD
    ASSERT_EQ(handle, 4);
#else
    ASSERT_EQ(handle, 5);
#endif
    EXPECT_CALL(testCallback4,timerCallback(handle,NULL)).Times(1);

    myHandler.start_listenting();
}



TEST(CAmSocketHandlerTest, signalHandlerPrimaryPlusSecondary)
{
    pMockSignalHandler = new MockIAmSignalHandler;
    CAmSocketHandler myHandler;
    ASSERT_FALSE(myHandler.fatalErrorOccurred());
    ASSERT_TRUE(myHandler.listenToSignals({SIGHUP})==E_OK);
    ASSERT_TRUE(myHandler.listenToSignals({SIGHUP, SIGTERM, SIGCHLD})==E_OK);
    sh_pollHandle_t signalHandler1, signalHandler2;
 
    std::string userData = "User data";

//     critical signals are registered here:
    struct sigaction signalAction;
    memset(&signalAction, '\0', sizeof(signalAction));
    signalAction.sa_sigaction = &signalHandler;
    signalAction.sa_flags = SA_RESETHAND | SA_NODEFER| SA_SIGINFO;
    sigaction(SIGINT, &signalAction, NULL);
    sigaction(SIGQUIT, &signalAction, NULL);
    
   myHandler.addSignalHandler([&](const sh_pollHandle_t handle, const signalfd_siginfo & info, void* userData)
   {
       unsigned sig = info.ssi_signo;
       pMockSignalHandler->signalHandlerAction(handle, sig, userData);
#ifdef ENABLED_SOCKETHANDLER_TEST_OUTPUT
       unsigned user = info.ssi_uid;
       std::cout<<"signal handler was called from user "<< user << " with signal " << sig << std::endl;
#endif
   }, signalHandler1, &userData);
   ASSERT_EQ(signalHandler1, 1);
   myHandler.addSignalHandler([&](const sh_pollHandle_t handle, const signalfd_siginfo & info, void* userData)
       {
           unsigned sig = info.ssi_signo;
           pMockSignalHandler->signalHandlerAction(handle, sig, userData);
#ifdef ENABLED_SOCKETHANDLER_TEST_OUTPUT
           unsigned user = info.ssi_uid;
           std::cout<<"signal handler was called from user "<< user << " with signal " << sig << std::endl;
#endif
       }, signalHandler2, &userData);
   ASSERT_EQ(signalHandler2, 2);
    timespec timeout4;
    timeout4.tv_nsec = 200000000;
    timeout4.tv_sec = 0;
    std::set<unsigned> secondarySignals;
    secondarySignals.insert({SIGHUP,SIGTERM, SIGCHLD});
    std::set<unsigned> primarySignals({SIGQUIT,SIGINT});
    std::set<unsigned> signals(primarySignals);
    signals.insert(secondarySignals.begin(), secondarySignals.end());

    CAmTimerSignalHandler testCallback4(&myHandler, timeout4, signals);
    sh_timerHandle_t handle;

    myHandler.addTimer(timeout4, &testCallback4.pTimerCallback, handle, NULL, true);
#ifndef WITH_TIMERFD
   ASSERT_EQ(handle, 1);
#else
   ASSERT_EQ(handle, 3);
#endif
    EXPECT_CALL(testCallback4,timerCallback(handle,NULL)).Times(signals.size()+1);
   for(auto it: secondarySignals)
       EXPECT_CALL(*pMockSignalHandler,signalHandlerAction(signalHandler1,it,&userData)).Times(1);
   for(auto it: secondarySignals)
       EXPECT_CALL(*pMockSignalHandler,signalHandlerAction(signalHandler2,it,&userData)).Times(1);
   for(auto it: primarySignals)
       EXPECT_CALL(*pMockSignalHandler,signalHandler(it,_,_)).Times(1);
   
    myHandler.start_listenting();
   delete pMockSignalHandler;
}

TEST(CAmSocketHandlerTest,playWithUNIXSockets)
{
    pthread_t serverThread;
    struct sockaddr_un servAddr;
    int socket_;

    CAmSocketHandler myHandler;
    ASSERT_FALSE(myHandler.fatalErrorOccurred());
    CAmSamplePlugin::sockType_e type = CAmSamplePlugin::UNIX;
    CAmSamplePlugin myplugin(&myHandler, type);

    EXPECT_CALL(myplugin,receiveData(_,_,_)).Times(SOCKET_TEST_LOOPS_COUNT + 1);
    EXPECT_CALL(myplugin,dispatchData(_,_)).Times(SOCKET_TEST_LOOPS_COUNT + 1);
    EXPECT_CALL(myplugin,check(_,_)).Times(SOCKET_TEST_LOOPS_COUNT + 1);

    //creates a thread that handles the serverpart
    pthread_create(&serverThread, NULL, playWithUnixSocketServer, &myHandler);

    sleep(1); //we need that here because the port needs to be opened
    if ((socket_ = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        std::cout << "socket problem" << std::endl;

    }

    memset(&servAddr, 0, sizeof(servAddr));
    strcpy(servAddr.sun_path, SOCK_PATH);
    servAddr.sun_family = AF_UNIX;
    if (connect(socket_, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
    {
        std::cout << "ERROR: connect() failed\n" << std::endl;
    }

    for (int i = 1; i <= SOCKET_TEST_LOOPS_COUNT; i++)
    {
        std::string stringToSend(TEST_SOCKET_DATA);
        send(socket_, stringToSend.c_str(), stringToSend.size(), 0);
    }
    std::string stringToSend(TEST_SOCKET_DATA_FINAL);
    send(socket_, stringToSend.c_str(), stringToSend.size(), 0);

    pthread_join(serverThread, NULL);

}

TEST(CAmSocketHandlerTest,playWithSockets)
{
    pthread_t serverThread;
    struct sockaddr_in servAddr;
    unsigned short servPort = 6060;
    struct hostent *host;
    int socket_;

    CAmSocketHandler myHandler;
    ASSERT_FALSE(myHandler.fatalErrorOccurred());
    CAmSamplePlugin::sockType_e type = CAmSamplePlugin::INET;
    CAmSamplePlugin myplugin(&myHandler, type);

    EXPECT_CALL(myplugin,receiveData(_,_,_)).Times(SOCKET_TEST_LOOPS_COUNT + 1);
    EXPECT_CALL(myplugin,dispatchData(_,_)).Times(SOCKET_TEST_LOOPS_COUNT + 1);
    EXPECT_CALL(myplugin,check(_,_)).Times(SOCKET_TEST_LOOPS_COUNT + 1);

    //creates a thread that handles the serverpart
    pthread_create(&serverThread, NULL, playWithSocketServer, &myHandler);

    sleep(1); //we need that here because the port needs to be opened
    if ((socket_ = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        std::cout << "socket problem" << std::endl;

    }

    if ((host = (struct hostent*) gethostbyname("localhost")) == 0)
    {
        std::cout << "ERROR: gethostbyname() failed\n" << std::endl;
        exit(1);
    }

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*) (host->h_addr_list[0])));
    servAddr.sin_port = htons(servPort);

    if (connect(socket_, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
    {
        std::cout << "ERROR: connect() failed\n" << std::endl;
    }

    for (int i = 1; i <= SOCKET_TEST_LOOPS_COUNT; i++)
    {
        std::string string(TEST_SOCKET_DATA);
        send(socket_, string.c_str(), string.size(), 0);
    }
    std::string string(TEST_SOCKET_DATA_FINAL);
    send(socket_, string.c_str(), string.size(), 0);

    pthread_join(serverThread, NULL);

}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

am::CAmSamplePlugin::CAmSamplePlugin(CAmSocketHandler *mySocketHandler, sockType_e socketType) :
        MockSocketHandlerCb(), connectFiredCB(this, &CAmSamplePlugin::connectSocket), //
        receiveFiredCB(this, &CAmSamplePlugin::receiveData), //
        sampleDispatchCB(this, &CAmSamplePlugin::dispatchData), //
        sampleCheckCB(this, &CAmSamplePlugin::check), //
        mSocketHandler(mySocketHandler), //
        mConnecthandle(), //
                mReceiveHandle(), //
                msgList()
{
    int yes = 1;

    int socketHandle;
    struct sockaddr_in servAddr;
    struct sockaddr_un unixAddr;
    unsigned int servPort = 6060;

    switch (socketType)
    {
        case UNIX:
            socketHandle = socket(AF_UNIX, SOCK_STREAM, 0);
            unixAddr.sun_family = AF_UNIX;
            strcpy(unixAddr.sun_path, SOCK_PATH);
            unlink(unixAddr.sun_path);
            bind(socketHandle, (struct sockaddr *) &unixAddr, strlen(unixAddr.sun_path) + sizeof(unixAddr.sun_family));
            break;
        case INET:
            socketHandle = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
            setsockopt(socketHandle, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
            memset(&servAddr, 0, sizeof(servAddr));
            servAddr.sin_family = AF_INET;
            servAddr.sin_addr.s_addr = INADDR_ANY;
            servAddr.sin_port = htons(servPort);
            bind(socketHandle, (struct sockaddr *) &servAddr, sizeof(servAddr));
            break;
        default:
            break;
    }

    if (listen(socketHandle, 3) < 0)
    {
#ifdef ENABLED_SOCKETHANDLER_TEST_OUTPUT
        std::cout << "listen ok" << std::endl;
#endif
    } /* if */

    int a = 1;
    ioctl(socketHandle, FIONBIO, (char *) &a);
    setsockopt(socketHandle, SOL_SOCKET, SO_KEEPALIVE, (char *) &a, sizeof(a));

    short events = 0;
    events |= POLLIN;
    mySocketHandler->addFDPoll(socketHandle, events, NULL, &connectFiredCB, NULL, NULL, NULL, mConnecthandle);
#ifdef ENABLED_SOCKETHANDLER_TEST_OUTPUT
    std::cout << "setup server - listening" << std::endl;
#endif
}

void am::CAmSamplePlugin::connectSocket(const pollfd pollfd1, const sh_pollHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    //first, accept the connection, create a new filedescriptor
#ifdef ENABLED_SOCKETHANDLER_TEST_OUTPUT
    std::cout << "Got a connection request !" << std::endl;
#endif
    struct sockaddr answer;
    socklen_t len = sizeof(answer);
    int receiveFD = accept(pollfd1.fd, (struct sockaddr*) &answer, &len);

    //set the correct event:
    short event = 0;
    event |= POLLIN;

    //aded the filedescriptor to the sockethandler and register the callbacks for receiving the data
    mSocketHandler->addFDPoll(receiveFD, event, NULL, &receiveFiredCB, &sampleCheckCB, &sampleDispatchCB, NULL, mReceiveHandle);

}

void am::CAmSamplePlugin::receiveData(const pollfd pollfd, const sh_pollHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    MockSocketHandlerCb::receiveData(pollfd, handle, userData);
    //initialize buffer
    char buffer[10];
    //read until buffer is full or no more data is there
    int read = recv(pollfd.fd, buffer, 7, 0);
    if (read > 1)
    {
        //read the message and store it in a queue
        std::string msg = std::string(buffer, read);
        msgList.push(msg);
#ifdef ENABLED_SOCKETHANDLER_TEST_OUTPUT
        std::cout << "Got a message !" << std::endl;
#endif
    }
}

bool am::CAmSamplePlugin::dispatchData(const sh_pollHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    //read data from the queue
    MockSocketHandlerCb::dispatchData(handle, userData);
#ifdef ENABLED_SOCKETHANDLER_TEST_OUTPUT
    std::cout << "Data:" << msgList.front() << std::endl;
#endif
    //if the message was our finish message, we quit the poll loop
    if (!(msgList.front().compare(TEST_SOCKET_DATA_FINAL) == 0 || msgList.front().compare(TEST_SOCKET_DATA) == 0)) //error
    {
        mSocketHandler->stop_listening();
    }
    if (msgList.front().compare(TEST_SOCKET_DATA_FINAL) == 0) //ok
    {
        mSocketHandler->stop_listening();
    }

    //remove the message from the queue and return false if there is no more message to read.
    msgList.pop();
    if (msgList.size() != 0)
        return true;
    return false;
}

bool am::CAmSamplePlugin::check(const sh_pollHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    MockSocketHandlerCb::check(handle, userData);
    //checks if there is data to dispatch
#ifdef ENABLED_SOCKETHANDLER_TEST_OUTPUT
    std::cout << "check!:" << std::endl;
#endif
    if (msgList.size() != 0)
        return true;
    return false;
}

