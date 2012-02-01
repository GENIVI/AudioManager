/*
 * sockethandlerTest.cpp
 *
 *  Created on: Dec 19, 2011
 *      Author: christian
 */

#include "sockethandlerTest.h"
#include <stdio.h>
#include <sys/socket.h> /* for socket(), connect(), (), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <sys/ioctl.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/un.h>

//todo: expand test, implement more usecases
//todo: test removeFD

#define SOCK_PATH "/tmp/mysock"

using namespace testing;
using namespace am;

sockethandlerTest::sockethandlerTest()
{
}

sockethandlerTest::~sockethandlerTest()
{
}

timerCallBack::timerCallBack(SocketHandler *myHandler) :
        pTimer1Callback(this, &timerCallBack::timer1Callback), //
        pTimer2Callback(this, &timerCallBack::timer2Callback), //
        pTimer3Callback(this, &timerCallBack::timer3Callback), //
        pTimer4Callback(this, &timerCallBack::timer4Callback), //
        mSocketHandler(myHandler)

{
}

am::timerCallBack::~timerCallBack()
{
}

void am::timerCallBack::timer1Callback(sh_timerHandle_t handle, void* userData)
{
    (void) handle;
    (void) userData;
    std::cout << "callback1 called" << std::endl;
    timespec timeout;
    timeout.tv_nsec = 0;
    timeout.tv_sec = 1;
    shTimerCallBack *buf = &pTimer1Callback;
    sh_timerHandle_t handle_;
    mSocketHandler->addTimer(timeout, buf, handle_, NULL);
}

void am::timerCallBack::timer2Callback(sh_timerHandle_t handle, void* userData)
{
    (void) handle;
    (void) userData;
    std::cout << "callback2 called" << std::endl;
    timespec timeout;
    timeout.tv_nsec = 0;
    timeout.tv_sec = 1;
    shTimerCallBack *buf = &pTimer2Callback;
    sh_timerHandle_t handle_;
    mSocketHandler->addTimer(timeout, buf, handle_, NULL);
}

void am::timerCallBack::timer3Callback(sh_timerHandle_t, void* userData)
{
    (void) userData;
    std::cout << "callback3 called" << std::endl;
}

void am::timerCallBack::timer4Callback(sh_timerHandle_t, void* userData)
{
    (void) userData;
    std::cout << "callback4 called" << std::endl;
    mSocketHandler->stop_listening();
}

void* playWithSocketServer(void* data)
{
    (void) data;
    SocketHandler myHandler;
    SamplePlugin::sockType_e type = SamplePlugin::INET;
    SamplePlugin myplugin(&myHandler, type);
    myHandler.start_listenting();
    return (NULL);
}

void* playWithUnixSocketServer(void* data)
{
    (void) data;
    SocketHandler myHandler;
    SamplePlugin::sockType_e type = SamplePlugin::UNIX;
    SamplePlugin myplugin(&myHandler, type);
    myHandler.start_listenting();
    return (NULL);
}

TEST(sockethandlerTest,playWithUNIXSockets)
{
    pthread_t serverThread;
    struct sockaddr_un servAddr;
    int socket_;

    //creates a thread that handles the serverpart
    pthread_create(&serverThread, NULL, playWithUnixSocketServer, NULL);

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

    for (int i = 0; i <= 1000; i++)
    {
        std::string string("Got It?");
        send(socket_, string.c_str(), string.size(), 0);
    }
    std::string string("finish!");
    send(socket_, string.c_str(), string.size(), 0);

    pthread_join(serverThread, NULL);
}

TEST(sockethandlerTest,playWithSockets)
{
    pthread_t serverThread;
    struct sockaddr_in servAddr;
    unsigned short servPort = 6060;
    struct hostent *host;
    int socket_;

    //creates a thread that handles the serverpart
    pthread_create(&serverThread, NULL, playWithSocketServer, NULL);

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

    for (int i = 0; i <= 1000; i++)
    {
        std::string string("Got It?");
        send(socket_, string.c_str(), string.size(), 0);
    }
    std::string string("finish!");
    send(socket_, string.c_str(), string.size(), 0);

    pthread_join(serverThread, NULL);
}

TEST(sockethandlerTest,playWithTimers)
{
    gDispatchDone = 0;
    SocketHandler myHandler;
    timerCallBack testCallback(&myHandler);
    timespec timeoutTime, timeout2, timeout3, timeout4;
    timeoutTime.tv_sec = 3;
    timeoutTime.tv_nsec = 0;
    timeout2.tv_nsec = 0;
    timeout2.tv_sec = 1;
    timeout3.tv_nsec = 000000000;
    timeout3.tv_sec = 2;
    timeout4.tv_nsec = 0;
    timeout4.tv_sec = 13;
    shTimerCallBack* buf = &testCallback.pTimer1Callback;
    shTimerCallBack* buf2 = &testCallback.pTimer2Callback;
    shTimerCallBack* buf3 = &testCallback.pTimer3Callback;
    shTimerCallBack* buf4 = &testCallback.pTimer4Callback;
    sh_timerHandle_t handle;
    myHandler.addTimer(timeoutTime, buf, handle, NULL);
    myHandler.addTimer(timeout2, buf2, handle, NULL);
    myHandler.addTimer(timeout3, buf3, handle, NULL);
    myHandler.addTimer(timeout4, buf4, handle, NULL);
    myHandler.start_listenting();

}

void sockethandlerTest::SetUp()
{
}

void sockethandlerTest::TearDown()
{
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

am::SamplePlugin::SamplePlugin(SocketHandler *mySocketHandler, sockType_e socketType) :
        connectFiredCB(this, &SamplePlugin::connectSocket), //
        receiveFiredCB(this, &SamplePlugin::receiveData), //
        sampleDispatchCB(this, &SamplePlugin::dispatchData), //
        sampleCheckCB(this, &SamplePlugin::check), //
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
        std::cout << "listen ok" << std::endl;
    } /* if */

    int a = 1;
    ioctl(socketHandle, FIONBIO, (char *) &a);
    setsockopt(socketHandle, SOL_SOCKET, SO_KEEPALIVE, (char *) &a, sizeof(a));

    short events = 0;
    events |= POLLIN;
    mySocketHandler->addFDPoll(socketHandle, events, NULL, &connectFiredCB, NULL, NULL, NULL, mConnecthandle);
    std::cout << "setup server - listening" << std::endl;
}

void am::SamplePlugin::connectSocket(const pollfd pollfd1, const sh_pollHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    //first, accept the connection, create a new filedescriptor
    std::cout << "Got a connection request !" << std::endl;
    struct sockaddr answer;
    socklen_t len = sizeof(answer);
    int receiveFD = accept(pollfd1.fd, (struct sockaddr*) &answer, &len);

    //set the correct event:
    short event = 0;
    event |= POLLIN;

    //aded the filedescriptor to the sockethandler and register the callbacks for receiving the data
    mSocketHandler->addFDPoll(receiveFD, event, NULL, &receiveFiredCB, &sampleCheckCB, &sampleDispatchCB, NULL, mReceiveHandle);

}

void am::SamplePlugin::receiveData(const pollfd pollfd, const sh_pollHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    //initialize buffer
    char buffer[10];
    //read until buffer is full or no more data is there
    int read = recv(pollfd.fd, buffer, 7, NULL);
    if (read > 1)
    {
        //read the message and store it in a queue
        std::string msg = std::string(buffer, read);
        msgList.push(msg);
        std::cout << "Got a message !" << std::endl;
    }
}

bool am::SamplePlugin::dispatchData(const sh_pollHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    //read data from the queue
    std::cout << "Data:" << msgList.front() << std::endl;

    //if the message was our finish message, we quit the poll loop
    if (msgList.front().compare("finish!") == 0)
    {
        mSocketHandler->stop_listening();
    }
    //remove the message from the queue and return false if there is no more message to read.
    msgList.pop();
    if (msgList.size() != 0)
        return true;
    return false;
}

bool am::SamplePlugin::check(const sh_pollHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    //checks if there is data to dispatch
    std::cout << "check!:" << std::endl;
    if (msgList.size() != 0)
        return true;
    return false;
}

