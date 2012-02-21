/*
 * CAmTelnetServerTest.h
 *
 *  Created on: Feb 7, 2012
 *      Author: Frank Herchet
 */

#ifndef CAMTELNETSERVERTEST_H_
#define CAMTELNETSERVERTEST_H_

#include "gtest/gtest.h"
#include "TelnetServer.h"
#include "DatabaseHandler.h"
#include "RoutingSender.h"
#include "CommandSender.h"
#include "ControlSender.h"
#include "Router.h"

namespace am
{

class SocketHandler;
class DatabaseHandler;
class RoutingSender;
class CommandSender;
class ControlSender;
class Router;
class CommandReceiver;
class RoutingReceiver;
class ControlReceiver;
class TelnetServer;


class MyEnvironment : public ::testing::Environment
{
 public:
  MyEnvironment();

  ~MyEnvironment();
  // Override this to define how to set up the environment.
  void SetUp();
  // Override this to define how to tear down the environment.
  void TearDown();

  void setSocketHandler(SocketHandler* pSocketHandler);

  void stopSocketHandler();

  std::vector<std::string> mlistRoutingPluginDirs;
  std::vector<std::string> mlistCommandPluginDirs;

  //SocketHandler*    mpSocketHandler;
  DatabaseHandler   mDatabasehandler;
  RoutingSender     mRoutingSender;
  CommandSender     mCommandSender;
  ControlSender     mControlSender;
  Router            mRouter;

  CommandReceiver*  mpCommandReceiver;
  RoutingReceiver*  mpRoutingReceiver;
  ControlReceiver*  mpControlReceiver;

  TelnetServer*     mpTelnetServer;

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
