/*
 * CAmTelnetServerTest.cpp
 *
 *  Created on: Feb 7, 2012
 *      Author: demo
 */

#include "CAmTelnetServerTest.h"

namespace am {

CAmTelnetServerTest::CAmTelnetServerTest()
: mlistRoutingPluginDirs()
, mlistCommandPluginDirs()
, mSocketHandler()
, mDatabasehandler(std::string(":memory:"))
, mRoutingSender(mlistRoutingPluginDirs)
, mCommandSender(mlistRoutingPluginDirs)
, mControlSender(std::string(""))
, mRouter(&mDatabasehandler,&mControlSender)
, mCommandReceiver(&mDatabasehandler,&mControlSender,&mSocketHandler)
, mRoutingReceiver(&mDatabasehandler,&mRoutingSender,&mControlSender,&mSocketHandler)
, mControlReceiver(&mDatabasehandler,&mRoutingSender,&mCommandSender,&mSocketHandler,&mRouter)
, mTelnetServer(&mSocketHandler,&mCommandSender,&mCommandReceiver,&mRoutingSender,&mRoutingReceiver,&mControlSender,&mControlReceiver,&mDatabasehandler,&mRouter)
{
   // TODO Auto-generated constructor stub

}

CAmTelnetServerTest::~CAmTelnetServerTest()
{
   // TODO Auto-generated destructor stub
}

CAmTelnetServerTest::SetUp()
{
   //startup all the Plugins and Interfaces
   mControlSender.startupController(&iControlReceiver);
   mCommandSender.startupInterface(&iCommandReceiver);
   mRoutingSender.startupRoutingInterface(&iRoutingReceiver);

   //when the routingInterface is done, all plugins are loaded:
   mControlSender.hookAllPluginsLoaded();

   mSocketHandler.start_listenting();

}

}
