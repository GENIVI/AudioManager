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

void* startSocketHandler(void* data)
{
    CAmTelnetServerTest* Test = static_cast<CAmTelnetServerTest*>(data);
    SocketHandler mySocketHandler;
    Test->setSocketHandler(&mySocketHandler);
    std::cout << "pThread: startSocketHandler" << std::endl;
    mySocketHandler.start_listenting();
    Test->setSocketHandler(NULL);
    std::cout << "pThread: return" << std::endl;
    return (NULL);
}


CAmTelnetServerTest::CAmTelnetServerTest()
: mlistRoutingPluginDirs()
, mlistCommandPluginDirs()
, mpSocketHandler(NULL)
, mDatabasehandler(std::string(":memory:"))
, mRoutingSender(mlistRoutingPluginDirs)
, mCommandSender(mlistRoutingPluginDirs)
, mControlSender(std::string(""))
, mRouter(&mDatabasehandler,&mControlSender)
, mpCommandReceiver(NULL)
, mpRoutingReceiver(NULL)
, mpControlReceiver(NULL)
, mpTelnetServer(NULL)
, mSocketHandlerThread()
{
   // TODO Auto-generated constructor stub

}

CAmTelnetServerTest::~CAmTelnetServerTest()
{
   if(NULL != mpTelnetServer)
      delete(mpTelnetServer);
   if(NULL != mpControlReceiver)
      delete(mpControlReceiver);
   if(NULL != mpRoutingReceiver)
      delete(mpRoutingReceiver);
   if(NULL != mpCommandReceiver)
      delete(mpCommandReceiver);
}

void CAmTelnetServerTest::SetUp()
{
   std::cout << "CAmTelnetServerTest::SetUp" << std::endl;



   // Create a Thread for the SocketHandler loop
   std::cout << "creating mSocketHandlerThread" << std::endl;
   pthread_create(&mSocketHandlerThread, NULL, startSocketHandler, this);
}

void CAmTelnetServerTest::TearDown()
{
   pthread_join(mSocketHandlerThread, NULL);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

void CAmTelnetServerTest::setSocketHandler(SocketHandler* pSocketHandler)
{
   mpSocketHandler = pSocketHandler;

   if(NULL != pSocketHandler)
   {
      std::cout << "CommandReceiver" << std::endl;
      mpCommandReceiver = new CommandReceiver(&mDatabasehandler,&mControlSender,mpSocketHandler);
      std::cout << "RoutingReceiver" << std::endl;
      mpRoutingReceiver = new RoutingReceiver(&mDatabasehandler,&mRoutingSender,&mControlSender,mpSocketHandler);
      std::cout << "ControlReceiver" << std::endl;
      mpControlReceiver = new ControlReceiver(&mDatabasehandler,&mRoutingSender,&mCommandSender,mpSocketHandler,&mRouter);


      std::cout << "startup all the Plugins and Interfaces" << std::endl;
      //startup all the Plugins and Interfaces
      mControlSender.startupController(mpControlReceiver);
      mCommandSender.startupInterface(mpCommandReceiver);
      mRoutingSender.startupRoutingInterface(mpRoutingReceiver);

      //when the routingInterface is done, all plugins are loaded:
      mControlSender.hookAllPluginsLoaded();

      // Starting TelnetServer
      std::cout << "Starting TelnetServer" << std::endl;
      mpTelnetServer = new TelnetServer(mpSocketHandler,&mCommandSender,mpCommandReceiver,&mRoutingSender,mpRoutingReceiver,&mControlSender,mpControlReceiver,&mDatabasehandler,&mRouter,6060,3);
   }
}

TEST_F(CAmTelnetServerTest,playWithSockets)
{
    //pthread_t serverThread;
    std::cout << "start playWithSockets" << std::endl;
    struct sockaddr_in servAddr;
    unsigned short servPort = 6060;
    struct hostent *host;
    int socket_;

    //creates a thread that handles the serverpart
    //pthread_create(&serverThread, NULL, playWithSocketServer, NULL);

    sleep(1); //we need that here because the port needs to be opened
    if ((socket_ = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
    {
        std::cout << "socket problem" << std::endl;

    }
    else
    {
       std::cout << "socket open" << std::endl;
    }

    if ((host = (struct hostent*) gethostbyname("localhost")) == 0)
    {
        std::cout << " ERROR: gethostbyname() failed\n" << std::endl;
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


    std::string string("finish!");
    send(socket_, string.c_str(), string.size(), 0);

    //pthread_join(serverThread, NULL);
}

