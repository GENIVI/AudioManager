/*
 * CAmTelnetServerTest.cpp
 *
 *  Created on: Feb 7, 2012
 *      Author: demo
 */

#include <sys/socket.h> /* for socket(), connect(), (), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <netdb.h> /* for  struct hostent */

#include "CAmTelnetServerTest.h"
#include "CommandReceiver.h"
#include "RoutingReceiver.h"
#include "ControlReceiver.h"


using namespace testing;
using namespace am;
using namespace std;


static std::string controllerPlugin = std::string(CONTROLLER_PLUGIN);
static unsigned short servPort = 6060;
static int staticSocket = -1;
static SocketHandler* mpSocketHandler = NULL;

void* startSocketHandler(void* data)
{
    MyEnvironment* Env = static_cast<MyEnvironment*>(data);
    SocketHandler mySocketHandler;
    Env->setSocketHandler(&mySocketHandler);
    mySocketHandler.start_listenting();
    Env->setSocketHandler(NULL);
    return (NULL);
}

MyEnvironment::MyEnvironment()
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

MyEnvironment::~MyEnvironment()
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

void MyEnvironment::SetUp()
{
    pthread_create(&mSocketHandlerThread, NULL, startSocketHandler, this);
    sleep(1);
}

void MyEnvironment::TearDown()
{

}

void MyEnvironment::setSocketHandler(SocketHandler* pSocketHandler)
{
    mpSocketHandler = pSocketHandler;

    if(NULL != pSocketHandler)
    {
        mpCommandReceiver = new CommandReceiver(&mDatabasehandler,&mControlSender,mpSocketHandler);
        mpRoutingReceiver = new RoutingReceiver(&mDatabasehandler,&mRoutingSender,&mControlSender,mpSocketHandler);
        mpControlReceiver = new ControlReceiver(&mDatabasehandler,&mRoutingSender,&mCommandSender,mpSocketHandler,&mRouter);

        //startup all the Plugins and Interfaces
        mControlSender.startupController(mpControlReceiver);
        mCommandSender.startupInterface(mpCommandReceiver);
        mRoutingSender.startupRoutingInterface(mpRoutingReceiver);

        //when the routingInterface is done, all plugins are loaded:
        mControlSender.hookAllPluginsLoaded();

        // Starting TelnetServer
        mpTelnetServer = new TelnetServer(mpSocketHandler,&mCommandSender,mpCommandReceiver,&mRoutingSender,mpRoutingReceiver,&mControlSender,mpControlReceiver,&mDatabasehandler,&mRouter,servPort,3);
    }
}

void MyEnvironment::stopSocketHandler()
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
    ::testing::Environment* const env = ::testing::AddGlobalTestEnvironment(new MyEnvironment);
    (void) env;
    return RUN_ALL_TESTS();
}
