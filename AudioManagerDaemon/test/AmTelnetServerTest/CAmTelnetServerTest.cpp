/**
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
 * \author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
 * \author Frank Herchet, frank.fh.herchet@bmw.de BMW 2012
 *
 * For further information see http://www.genivi.org/.
 *
 */

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string>

#include "CAmTelnetServerTest.h"
#include "CAmCommandReceiver.h"
#include "CAmRoutingReceiver.h"
#include "CAmControlReceiver.h"
#include "config.h"


using namespace testing;
using namespace am;
using namespace std;


static std::string controllerPlugin = std::string(CONTROLLER_PLUGIN);
static unsigned short servPort = 6060;
static int staticSocket = -1;
static CAmSocketHandler* mpSocketHandler = NULL;

void* startSocketHandler(void* data)
{
    CAmEnvironment* Env = static_cast<CAmEnvironment*>(data);
    CAmSocketHandler mySocketHandler;
    Env->setSocketHandler(&mySocketHandler);
    mySocketHandler.start_listenting();
    Env->setSocketHandler(NULL);
    return (NULL);
}

CAmEnvironment::CAmEnvironment()
: mlistRoutingPluginDirs()
, mlistCommandPluginDirs()
//, mpSocketHandler(NULL)
, mDatabasehandler(std::string(":memory:"))
, mRoutingSender(mlistRoutingPluginDirs)
, mCommandSender(mlistRoutingPluginDirs)
, mControlSender(controllerPlugin)
, mRouter(&mDatabasehandler,&mControlSender)
, mpCommandReceiver(NULL)
, mpRoutingReceiver(NULL)
, mpControlReceiver(NULL)
, mpTelnetServer(NULL)
, mSocketHandlerThread(0)
{
}

CAmEnvironment::~CAmEnvironment()
{
    usleep(500);
    if(NULL != mpTelnetServer)
       delete(mpTelnetServer);
    if(NULL != mpControlReceiver)
       delete(mpControlReceiver);
    if(NULL != mpRoutingReceiver)
       delete(mpRoutingReceiver);
    if(NULL != mpCommandReceiver)
       delete(mpCommandReceiver);
}

void CAmEnvironment::SetUp()
{
    pthread_create(&mSocketHandlerThread, NULL, startSocketHandler, this);
    sleep(1);
}

void CAmEnvironment::TearDown()
{
    pthread_cancel(mSocketHandlerThread);
}

void CAmEnvironment::setSocketHandler(CAmSocketHandler* pSocketHandler)
{
    mpSocketHandler = pSocketHandler;

    if(NULL != pSocketHandler)
    {
        mpCommandReceiver = new CAmCommandReceiver(&mDatabasehandler,&mControlSender,mpSocketHandler);
        mpRoutingReceiver = new CAmRoutingReceiver(&mDatabasehandler,&mRoutingSender,&mControlSender,mpSocketHandler);
        mpControlReceiver = new CAmControlReceiver(&mDatabasehandler,&mRoutingSender,&mCommandSender,mpSocketHandler,&mRouter);

        //startup all the Plugins and Interfaces
        mControlSender.startupController(mpControlReceiver);
        mCommandSender.startupInterfaces(mpCommandReceiver);
        mRoutingSender.startupInterfaces(mpRoutingReceiver);

        //when the routingInterface is done, all plugins are loaded:
        mControlSender.setControllerReady();

        // Starting TelnetServer
        mpTelnetServer = new CAmTelnetServer(mpSocketHandler,&mCommandSender,mpCommandReceiver,&mRoutingSender,mpRoutingReceiver,&mControlSender,mpControlReceiver,&mDatabasehandler,&mRouter,servPort,3);
    }
}

void CAmEnvironment::stopSocketHandler()
{
    mpSocketHandler->stop_listening();
}

CAmTelnetServerTest::CAmTelnetServerTest()
{

}

CAmTelnetServerTest::~CAmTelnetServerTest()
{

}

void CAmTelnetServerTest::SetUp()
{

}

void CAmTelnetServerTest::TearDown()
{

}

TEST_F(CAmTelnetServerTest,connectTelnetServer)
{
    struct sockaddr_in servAddr;

    staticSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    ASSERT_GE(staticSocket,0);

    struct hostent *host = (struct hostent*) gethostbyname("localhost");
    if (host == 0)
    {
        std::cout << " ERROR: gethostbyname() failed\n" << std::endl;
        return;
    }

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*) (host->h_addr_list[0])));
    servAddr.sin_port = htons(servPort);

    int return_connect = connect(staticSocket, (struct sockaddr *) &servAddr, sizeof(servAddr));
    ASSERT_GE(return_connect,0);

    char buffer[1000];
    int read=recv(staticSocket,buffer,sizeof(buffer),0);
    ASSERT_GT(read,1);
}

TEST_F(CAmTelnetServerTest,sendCmdTelnetServer)
{
    std::string string("help");

    ssize_t sizesent = send(staticSocket, string.c_str(), string.size(), 0);
    ASSERT_EQ(sizesent,string.size());

    char buffer[1000];
    memset(buffer,0,sizeof(buffer));
    int read=recv(staticSocket,buffer,sizeof(buffer),0);
    ASSERT_GT(read,1);
}

TEST_F(CAmTelnetServerTest,closeTelnetServerConnection)
{
    std::string string ("exit");

    mpSocketHandler->stop_listening();

    ssize_t sizesent = send(staticSocket, string.c_str(), string.size(), 0);
    ASSERT_EQ(sizesent,string.size());

    char buffer[1000];
    memset(buffer,0,sizeof(buffer));
    int read=recv(staticSocket,buffer,sizeof(buffer),0);
    ASSERT_GT(read,1);

    close(staticSocket);
    staticSocket = -1;
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::Environment* const env = ::testing::AddGlobalTestEnvironment(new CAmEnvironment);
    (void) env;
    return RUN_ALL_TESTS();
}
