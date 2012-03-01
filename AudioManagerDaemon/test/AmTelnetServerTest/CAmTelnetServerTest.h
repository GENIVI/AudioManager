/*
 * CAmTelnetServerTest.h
 *
 *  Created on: Feb 7, 2012
 *      Author: Frank Herchet
 */

#ifndef CAMTELNETSERVERTEST_H_
#define CAMTELNETSERVERTEST_H_

#include "gtest/gtest.h"
#include "CAmTelnetServer.h"
#include "CAmDatabaseHandler.h"
#include "CAmRoutingSender.h"
#include "CAmCommandSender.h"
#include "CAmControlSender.h"
#include "CAmRouter.h"

namespace am
{

class CAmSocketHandler;
class CAmDatabaseHandler;
class CAmRoutingSender;
class CAmCommandSender;
class CAmControlSender;
class CAmRouter;
class CAmCommandReceiver;
class CAmRoutingReceiver;
class CAmControlReceiver;
class CAmTelnetServer;


class CAmEnvironment : public ::testing::Environment
{
 public:
  CAmEnvironment();

  ~CAmEnvironment();
  // Override this to define how to set up the environment.
  void SetUp();
  // Override this to define how to tear down the environment.
  void TearDown();

  void setSocketHandler(CAmSocketHandler* pSocketHandler);

  void stopSocketHandler();

  std::vector<std::string> mlistRoutingPluginDirs;
  std::vector<std::string> mlistCommandPluginDirs;

  //SocketHandler*    mpSocketHandler;
  CAmDatabaseHandler   mDatabasehandler;
  CAmRoutingSender     mRoutingSender;
  CAmCommandSender     mCommandSender;
  CAmControlSender     mControlSender;
  CAmRouter            mRouter;

  CAmCommandReceiver*  mpCommandReceiver;
  CAmRoutingReceiver*  mpRoutingReceiver;
  CAmControlReceiver*  mpControlReceiver;

  CAmTelnetServer*     mpTelnetServer;

  pthread_t         mSocketHandlerThread;
};

class CAmTelnetServerTest : public ::testing::Test
{
   public:
      CAmTelnetServerTest();
      ~CAmTelnetServerTest();


   void SetUp() ;

   void TearDown() ;

   //int              mSocket;
};

}




#endif /* CAMTELNETSERVERTEST_H_ */
