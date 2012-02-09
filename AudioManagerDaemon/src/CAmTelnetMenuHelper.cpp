/**
* Copyright (C) 2012, BMW AG
*
* GeniviAudioMananger AudioManagerDaemon
*
* \file CAmTelnetMenuHelper.cpp
*
* \date 24-Jan-2012
* \author Frank Herchet (frank.fh.herchet@bmw.de)
*
* \section License
* GNU Lesser General Public License, version 2.1, with special exception (GENIVI clause)
* Copyright (C) 2012, BMW AG Frank Herchet  frank.fh.herchet@bmw.de
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

#include "CAmTelnetMenuHelper.h"
#include <dlt/dlt.h>
#include <cassert>

#define DEBUG_ON false

using namespace am;

DLT_IMPORT_CONTEXT(AudioManager)

CAmTelnetMenuHelper* CAmTelnetMenuHelper::instance = NULL;

/****************************************************************************/
CAmTelnetMenuHelper::CAmTelnetMenuHelper(SocketHandler *iSocketHandler,
                                         CommandSender *iCommandSender,
                                         CommandReceiver *iCommandReceiver,
                                         RoutingSender *iRoutingSender,
                                         RoutingReceiver *iRoutingReceiver,
                                         ControlSender *iControlSender,
                                         ControlReceiver *iControlReceiver,
                                         DatabaseHandler *iDatabasehandler,
                                         Router *iRouter)
/****************************************************************************/
: mTelenetServer(NULL)
, mSocketHandler(iSocketHandler)
, mCommandSender(iCommandSender)
, mCommandReceiver(iCommandReceiver)
, mRoutingSender(iRoutingSender)
, mRoutingReceiver(iRoutingReceiver)
, mControlSender(iControlSender)
, mControlReceiver(iControlReceiver)
, mDatabasehandler(iDatabasehandler)
, mRouter(iRouter)
{
   instance = this;
   createCommandMaps();
}

/****************************************************************************/
CAmTelnetMenuHelper::~CAmTelnetMenuHelper()
/****************************************************************************/
{
}

/****************************************************************************/
void CAmTelnetMenuHelper::createCommandMaps()
/****************************************************************************/
{
   // ROOT commands

   mRootCommands.clear();

   mRootCommands.insert(std::make_pair("help",sCommandPrototypeInfo("show all possible commands",&CAmTelnetMenuHelper::helpCommand)));
   mRootCommands.insert(std::make_pair("list",sCommandPrototypeInfo("Go into 'list'-submenu",&CAmTelnetMenuHelper::rootListCommand)));
   mRootCommands.insert(std::make_pair("info",sCommandPrototypeInfo("Go into 'info'-submenu",&CAmTelnetMenuHelper::rootInfoCommand)));
   mRootCommands.insert(std::make_pair("set",sCommandPrototypeInfo("Go into 'set'-submenu",&CAmTelnetMenuHelper::rootSetCommand)));
   mRootCommands.insert(std::make_pair("get",sCommandPrototypeInfo("Go into 'get'-submenu",&CAmTelnetMenuHelper::rootGetCommand)));
   mRootCommands.insert(std::make_pair("exit",sCommandPrototypeInfo("quit telnet session",&CAmTelnetMenuHelper::exitCommand)));

   // List commands
   mListCommands.insert(std::make_pair("help",sCommandPrototypeInfo(std::string("show all possible commands"),&CAmTelnetMenuHelper::helpCommand)));
   mListCommands.insert(std::make_pair("conn",sCommandPrototypeInfo("list all connections",&CAmTelnetMenuHelper::listConnectionsCommand)));
   mListCommands.insert(std::make_pair("sources",sCommandPrototypeInfo("list all available sources",&CAmTelnetMenuHelper::listSourcesCommand)));
   mListCommands.insert(std::make_pair("sinks",sCommandPrototypeInfo("list all available sinks",&CAmTelnetMenuHelper::listSinksCommands)));
   mListCommands.insert(std::make_pair("crfaders",sCommandPrototypeInfo("list all crossfaders",&CAmTelnetMenuHelper::listCrossfaders)));
   mListCommands.insert(std::make_pair("domains",sCommandPrototypeInfo("list all domains",&CAmTelnetMenuHelper::listDomainsCommand)));
   mListCommands.insert(std::make_pair("gws",sCommandPrototypeInfo("list all gateways",&CAmTelnetMenuHelper::listGatewaysCommand)));
   mListCommands.insert(std::make_pair("..",sCommandPrototypeInfo("one step back in menu tree (back to root folder)",&CAmTelnetMenuHelper::oneStepBackCommand)));
   mListCommands.insert(std::make_pair("exit",sCommandPrototypeInfo("close telnet session",&CAmTelnetMenuHelper::exitCommand)));

   // Set commands
   mSetCommands.insert(std::make_pair("help",sCommandPrototypeInfo(std::string("show all possible commands"),&CAmTelnetMenuHelper::helpCommand)));
   mSetCommands.insert(std::make_pair("..",sCommandPrototypeInfo("one step back in menu tree (back to root folder)",&CAmTelnetMenuHelper::oneStepBackCommand)));
   mSetCommands.insert(std::make_pair("exit",sCommandPrototypeInfo("close telnet session",&CAmTelnetMenuHelper::exitCommand)));
   mSetCommands.insert(std::make_pair("conn",sCommandPrototypeInfo("use 'conn sourceId sinkId' to connect a source and a sink",&CAmTelnetMenuHelper::setConnection)));
   mSetCommands.insert(std::make_pair("routing",sCommandPrototypeInfo("use 'routing sourceId sinkId' to get all\n\t possible routes between a sourceID and a sinkID",&CAmTelnetMenuHelper::setRoutingCommand)));

   // Get commands
   mGetCommands.insert(std::make_pair("help",sCommandPrototypeInfo(std::string("show all possible commands"),&CAmTelnetMenuHelper::helpCommand)));
   mGetCommands.insert(std::make_pair("routing",sCommandPrototypeInfo("show current routing",&CAmTelnetMenuHelper::getRoutingCommand)));
   mGetCommands.insert(std::make_pair("sendv",sCommandPrototypeInfo("show senderversion",&CAmTelnetMenuHelper::getSenderversionCommand)));
   mGetCommands.insert(std::make_pair("recv",sCommandPrototypeInfo("show receiverversion ",&CAmTelnetMenuHelper::getReceiverversionCommand)));
   mGetCommands.insert(std::make_pair("..",sCommandPrototypeInfo("one step back in menu tree (back to root folder)",&CAmTelnetMenuHelper::oneStepBackCommand)));
   mGetCommands.insert(std::make_pair("exit",sCommandPrototypeInfo("close telnet session",&CAmTelnetMenuHelper::exitCommand)));

   // Info comands
   mInfoCommands.insert(std::make_pair("help",sCommandPrototypeInfo(std::string("show all possible commands"),&CAmTelnetMenuHelper::helpCommand)));
   mInfoCommands.insert(std::make_pair("sysprop",sCommandPrototypeInfo("show all systemproperties",&CAmTelnetMenuHelper::infoSystempropertiesCommand)));
   mInfoCommands.insert(std::make_pair("..",sCommandPrototypeInfo("one step back in menu tree (back to root folder)",&CAmTelnetMenuHelper::oneStepBackCommand)));
   mInfoCommands.insert(std::make_pair("exit",sCommandPrototypeInfo("close telnet session",&CAmTelnetMenuHelper::exitCommand)));
}

/****************************************************************************/
void CAmTelnetMenuHelper::setTelnetServer(TelnetServer* iTelnetServer)
/****************************************************************************/
{
   mTelenetServer = iTelnetServer;
}

/****************************************************************************/
void CAmTelnetMenuHelper::newSocketConnection(int filedescriptor)
/****************************************************************************/
{
   EMainState state = eRootState;
   std::map<int,EMainState>::iterator it;
   std::stringstream welcome;

   it = mCurrentMainStateMap.find(filedescriptor);
   if( it != mCurrentMainStateMap.end())
   {
      // socket connection already exists, delete entry and go back to root state
      mCurrentMainStateMap.erase(it);
   }

   it = mCurrentMainStateMap.begin();

   // insert new socket connection
   mCurrentMainStateMap.insert(it,std::make_pair<int,EMainState>(filedescriptor,state));

   // Send welcome message
   welcome << "Welcome to GENIVI AudioManager " << DAEMONVERSION << "\n>";
   send(filedescriptor,welcome.str().c_str(),welcome.str().size(),0);
}

/****************************************************************************/
void CAmTelnetMenuHelper::socketConnectionsClosed(int filedescriptor)
/****************************************************************************/
{
   std::map<int,EMainState>::iterator it;

   it = mCurrentMainStateMap.find(filedescriptor);
   if( it != mCurrentMainStateMap.end())
   {
      mCurrentMainStateMap.erase(it);
   }
   else
   {
      // connection not found
   }
}

/****************************************************************************/
void CAmTelnetMenuHelper::enterCmdQueue(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   std::map<int,EMainState>::iterator it;
   std::string cmd;
   tCommandMap::iterator cmditer;

   // find current filedescriptor to get the current state of the telnet session
   it = mCurrentMainStateMap.find(filedescriptor);
   while(!CmdQueue.empty())
   {
      cmd = CmdQueue.front();

      // Now remove the first command, it's stored in 'cmd'
      CmdQueue.pop();
      // telnet session found. depending on the current state, different commands are available
      switch(it->second)
      {
         case eRootState:
            cmditer = mRootCommands.find(cmd);
            if(mRootCommands.end() != cmditer)
               cmditer->second.CommandPrototype(CmdQueue,filedescriptor);
            else
               sendError(filedescriptor,"Command not found\n");
            break;
         case eListState:
            cmditer = mListCommands.find(cmd);
            if(mListCommands.end() != cmditer)
               cmditer->second.CommandPrototype(CmdQueue,filedescriptor);
            else
               sendError(filedescriptor,"Command not found\n");
            break;
         case eInfoState:
            cmditer = mInfoCommands.find(cmd);
            if(mInfoCommands.end() != cmditer)
               cmditer->second.CommandPrototype(CmdQueue,filedescriptor);
            else
               sendError(filedescriptor,"Command not found\n");
            break;
         case eGetState:
            cmditer = mGetCommands.find(cmd);
            if(mGetCommands.end() != cmditer)
               cmditer->second.CommandPrototype(CmdQueue,filedescriptor);
            else
               sendError(filedescriptor,"Command not found\n");
            break;
         case eSetState:
            cmditer = mSetCommands.find(cmd);
            if(mSetCommands.end() != cmditer)
               cmditer->second.CommandPrototype(CmdQueue,filedescriptor);
            else
               sendError(filedescriptor,"Command not found\n");
            break;
         default:
            break;
      }
   }

   sendCurrentCmdPrompt(filedescriptor);
}

/****************************************************************************/
void CAmTelnetMenuHelper::sendError(int & filedescriptor, std::string error_string)
/****************************************************************************/
{
   send(filedescriptor,error_string.c_str(),error_string.size(),0);
}

/****************************************************************************/
void CAmTelnetMenuHelper::sendTelnetLine(int & filedescriptor, std::stringstream &line)
/****************************************************************************/
{
   send(filedescriptor,line.str().c_str(),line.str().size(),0);
}

/****************************************************************************/
void CAmTelnetMenuHelper::sendCurrentCmdPrompt(int & filedescriptor)
/****************************************************************************/
{
   std::map<int,EMainState>::iterator it;
   std::stringstream outputstream;
   outputstream << std::endl;

   it = mCurrentMainStateMap.find(filedescriptor);
   if( it != mCurrentMainStateMap.end())
   {
      switch(it->second)
      {
         case eRootState:
            outputstream  << "\\>";
            break;
         case eListState:
            outputstream  << "\\List>";
            break;
         case eGetState:
            outputstream  << "\\Get>";
            break;
         case eSetState:
            outputstream  << "\\Set>";
            break;
         case eInfoState:
            outputstream  << "\\Info>";
            break;
         default:
            break;
      }

      send(filedescriptor,outputstream.str().c_str(),outputstream.str().size(),0);

   }
   else
   {
      // connection not found
   }
}

/****************************************************************************/
void CAmTelnetMenuHelper::exitCommand(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   instance->exitCommandExec(CmdQueue,filedescriptor);
}

/****************************************************************************/
void CAmTelnetMenuHelper::oneStepBackCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   std::map<int,EMainState>::iterator it;
   it = mCurrentMainStateMap.find(filedescriptor);
   if( it != mCurrentMainStateMap.end())
   {
      if(DEBUG_ON)std::cout << "old state: " << it->second;
      switch(it->second)
      {
         case eRootState:
            it->second = eRootState;
            break;
         case eListState:
            it->second = eRootState;;
            break;
         case eGetState:
            it->second = eRootState;;
            break;
         case eSetState:
            it->second = eRootState;;
            break;
         case eInfoState:
            it->second = eRootState;;
            break;
         default:
            it->second = eRootState;
            break;
      }
      if(DEBUG_ON)std::cout << "new state: " << it->second << std::endl;
      //enterCmdQueue(CmdQueue,filedescriptor);
   }
}

/****************************************************************************/
void CAmTelnetMenuHelper::oneStepBackCommand(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   instance->oneStepBackCommandExec(CmdQueue,filedescriptor);
}

/****************************************************************************/
void CAmTelnetMenuHelper::exitCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   std::map<int,EMainState>::iterator it;
   std::stringstream line;
   std::stringstream output;

   // Sending a last message to the client
   output << "Your wish is my command ... bye!" << std::endl;
   sendTelnetLine(filedescriptor,output);


   tCommandMap::iterator iter;
   it = mCurrentMainStateMap.find(filedescriptor);
   if( it != mCurrentMainStateMap.end())
   {
      if(DEBUG_ON)std::cout << "removing client connection " << filedescriptor << std::endl;

      if(NULL != mTelenetServer)
      {
         mTelenetServer->disconnectClient(filedescriptor);
         mCurrentMainStateMap.erase(it);
      }
      else
      {
         // ASSERT mTelenetServer == NULL
         if(DEBUG_ON)std::cout << "mTelenetServer";
      }
   }
}

/****************************************************************************/
void CAmTelnetMenuHelper::helpCommand(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   instance->helpCommandExec(CmdQueue,filedescriptor);
}

/****************************************************************************/
void CAmTelnetMenuHelper::helpCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   std::map<int,EMainState>::iterator it;
   std::stringstream line;
   tCommandMap::iterator cmdIter;
   it = mCurrentMainStateMap.find(filedescriptor);
   if( it != mCurrentMainStateMap.end())
   {
      line << "###################################################" << std::endl;
      line << "###### The following commands are supported: ######"<< std::endl;
      line << "###################################################" << std::endl << std::endl;
      switch(it->second)
      {
         case eRootState:

            cmdIter = mRootCommands.begin();
            while(cmdIter != mRootCommands.end())
            {
               line << cmdIter->first << "\t- " << cmdIter->second.info << std::endl;
               cmdIter++;
            }
            break;
         case eListState:
            cmdIter = mListCommands.begin();
            while(cmdIter != mListCommands.end())
            {
               line << cmdIter->first << "\t- " << cmdIter->second.info << std::endl;
               cmdIter++;
            }
            break;
         case eGetState:
            cmdIter = mGetCommands.begin();
            while(cmdIter != mGetCommands.end())
            {
               line << cmdIter->first << "\t- " << cmdIter->second.info << std::endl;
               cmdIter++;
            }
            break;
         case eSetState:
            cmdIter = mSetCommands.begin();
            while(cmdIter != mSetCommands.end())
            {
               line << cmdIter->first << "\t- " << cmdIter->second.info << std::endl;
               cmdIter++;
            }
            break;
         case eInfoState:
            cmdIter = mInfoCommands.begin();
            while(cmdIter != mInfoCommands.end())
            {
               line << cmdIter->first << "\t- " << cmdIter->second.info << std::endl;
               cmdIter++;
            }
            break;
         default:
            break;
      }
      sendTelnetLine(filedescriptor,line);
      //enterCmdQueue(CmdQueue,filedescriptor);
   }
}

/****************************************************************************/
void CAmTelnetMenuHelper::rootGetCommand(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   instance->rootGetCommandExec(CmdQueue,filedescriptor);
}

/****************************************************************************/
void CAmTelnetMenuHelper::rootGetCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   std::map<int,EMainState>::iterator it;
   it = mCurrentMainStateMap.find(filedescriptor);
   if( it != mCurrentMainStateMap.end())
   {
      it->second = eGetState;
      //enterCmdQueue(CmdQueue,filedescriptor);
   }
}

/****************************************************************************/
void CAmTelnetMenuHelper::rootSetCommand(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   instance->rootSetCommandExec(CmdQueue,filedescriptor);
}

/****************************************************************************/
void CAmTelnetMenuHelper::rootSetCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   std::map<int,EMainState>::iterator it;
   it = mCurrentMainStateMap.find(filedescriptor);
   if( it != mCurrentMainStateMap.end())
   {
      it->second = eSetState;
      //enterCmdQueue(CmdQueue,filedescriptor);
   }
}

/****************************************************************************/
void CAmTelnetMenuHelper::rootListCommand(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   instance->rootListCommandExec(CmdQueue,filedescriptor);
}

/****************************************************************************/
void CAmTelnetMenuHelper::rootListCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   std::map<int,EMainState>::iterator it;
   it = mCurrentMainStateMap.find(filedescriptor);
   if( it != mCurrentMainStateMap.end())
   {
      it->second = eListState;
      //enterCmdQueue(CmdQueue,filedescriptor);
   }
}

/****************************************************************************/
void CAmTelnetMenuHelper::rootInfoCommand(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   instance->rootInfoCommandExec(CmdQueue,filedescriptor);
}

/****************************************************************************/
void CAmTelnetMenuHelper::rootInfoCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   std::map<int,EMainState>::iterator it;
   it = mCurrentMainStateMap.find(filedescriptor);
   if( it != mCurrentMainStateMap.end())
   {
      it->second = eInfoState;
      //enterCmdQueue(CmdQueue,filedescriptor);
   }
}

/****************************************************************************/
void CAmTelnetMenuHelper::listConnectionsCommand(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   instance->listConnectionsCommandExec(CmdQueue,filedescriptor);
}

/****************************************************************************/
void CAmTelnetMenuHelper::listConnectionsCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   std::vector<am_Connection_s> listConnections;
   std::vector<am_Connection_s>::iterator it;
   std::stringstream line;

   mDatabasehandler->getListConnections(listConnections);

   line << "Current connections: " << listConnections.size();

   sendTelnetLine(filedescriptor,line);

   while(it != listConnections.end())
   {
      line.clear();
      line << "\tID: "  << it->connectionID
           << "\tSrcID: "  << it->sourceID
           << "\tSinkID: " << it->sinkID
           << "\tFormat: " << it->connectionFormat
           << "\tdelay: "  << it->delay;

      sendTelnetLine(filedescriptor,line);
      it++;
   }
}

/****************************************************************************/
void CAmTelnetMenuHelper::listSourcesCommand(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   instance->listSourcesCommandExec(CmdQueue,filedescriptor);
}

/****************************************************************************/
void CAmTelnetMenuHelper::listSourcesCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   std::vector<am_Source_s> listSources;
   std::vector<am_Source_s>::iterator it;
   std::stringstream line;

   mDatabasehandler->getListSources(listSources);

   line << "Current sources: " << listSources.size();
   sendTelnetLine(filedescriptor,line);

   while(it != listSources.end())
   {
      line.clear();
      line << "\tID: "  << it->sourceID
           << "\tDomainID: "  << it->domainID
           << "\tName: " << it->name
           << "\tState: " << it->sourceState
           << "\tVolume: "  << it->volume;

      sendTelnetLine(filedescriptor,line);
      it++;
   }
}

/****************************************************************************/
void CAmTelnetMenuHelper::listSinksCommands(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   instance->listSinksCommandsExec(CmdQueue,filedescriptor);
}

/****************************************************************************/
void CAmTelnetMenuHelper::listSinksCommandsExec(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   std::vector<am_Sink_s> listSinks;
   std::vector<am_Sink_s>::iterator it;
   std::stringstream line;

   mDatabasehandler->getListSinks(listSinks);

   line << "Current sinks: " << listSinks.size();
   sendTelnetLine(filedescriptor,line);

   while(it != listSinks.end())
   {
      line.clear();
      line << "\tID: "  << it->sinkID
           << "\tDomainID: "  << it->domainID
           << "\tName: " << it->name
           << "\tAvailable: " << it->available.availability
           << "\tVolume: "  << it->volume;

      sendTelnetLine(filedescriptor,line);
      it++;
   }
}

/****************************************************************************/
void CAmTelnetMenuHelper::listCrossfaders(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   instance->listCrossfadersExec(CmdQueue,filedescriptor);
}

/****************************************************************************/
void CAmTelnetMenuHelper::listCrossfadersExec(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   std::vector<am_Crossfader_s> listCrossfaders;
   std::vector<am_Crossfader_s>::iterator it;
   std::stringstream line;

   mDatabasehandler->getListCrossfaders(listCrossfaders);

   line << "Current crossfaders: " << listCrossfaders.size();
   sendTelnetLine(filedescriptor,line);

   while(it != listCrossfaders.end())
   {
      line.clear();
      line << "\tID: "  << it->crossfaderID
           << "\tName: "  << it->name
           << "\tSourceID: " << it->sourceID;

      sendTelnetLine(filedescriptor,line);
      it++;
   }
}

/****************************************************************************/
void CAmTelnetMenuHelper::listDomainsCommand(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   instance->listDomainsCommandExec(CmdQueue,filedescriptor);
}

/****************************************************************************/
void CAmTelnetMenuHelper::listDomainsCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   std::vector<am_Domain_s> listDomains;
   std::vector<am_Domain_s>::iterator it;
   std::stringstream line;

   mDatabasehandler->getListDomains(listDomains);

   line << "Current domains: " << listDomains.size();
   sendTelnetLine(filedescriptor,line);

   while(it != listDomains.end())
   {
      line.clear();
      line << "\tID: "  << it->domainID
           << "\tName: "  << it->name
           << "\tBusname: " << it->busname
           << "\tNodename: " << it->nodename
           << "\tState: " << it->state;

      sendTelnetLine(filedescriptor,line);
      it++;
   }
}

/****************************************************************************/
void CAmTelnetMenuHelper::listGatewaysCommand(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   instance->listGatewaysCommandExec(CmdQueue,filedescriptor);
}

/****************************************************************************/
void CAmTelnetMenuHelper::listGatewaysCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   std::vector<am_Gateway_s> listGateways;
   std::vector<am_Gateway_s>::iterator it;
   std::stringstream line;

   mDatabasehandler->getListGateways(listGateways);

   line << "Current gateways: " << listGateways.size();
   sendTelnetLine(filedescriptor,line);

   while(it != listGateways.end())
   {
      line.clear();
      line << "\tID: "  << it->gatewayID
           << "\tName: "  << it->name
           << "\tSourceID: " << it->sourceID
           << "\tSinkID: " << it->sinkID;

      sendTelnetLine(filedescriptor,line);
      it++;
   }
}

/****************************************************************************/
void CAmTelnetMenuHelper::getRoutingCommand(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   instance->getRoutingCommandExec(CmdQueue,filedescriptor);
}

/****************************************************************************/
void CAmTelnetMenuHelper::getRoutingCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   std::vector<am_Gateway_s> listGateways;
   std::vector<am_Gateway_s>::iterator it;
   std::stringstream line;

   /*
   mRouter->getRoute()

   line << "Current gateways: " << listGateways.size();
   sendTelnetLine(filedescriptor,line);

   while(it != listGateways.end())
   {
      line.clear();
      line << "\tID: "  << it->gatewayID
           << "\tName: "  << it->name
           << "\tSourceID: " << it->sourceID
           << "\tSinkID: " << it->sinkID;

      sendTelnetLine(filedescriptor,line);
      it++;
   }
   */
}

/****************************************************************************/
void CAmTelnetMenuHelper::getSenderversionCommand(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   instance->getSenderversionCommandExec(CmdQueue,filedescriptor);
}

/****************************************************************************/
void CAmTelnetMenuHelper::getSenderversionCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   std::stringstream line;

   line << "Sender versions:" << std::endl
        << "Ctrl: "     << mControlSender->getInterfaceVersion() << " | "
        << "Cmd: "      << mCommandSender->getInterfaceVersion() << " | "
        << "Routing: "  << mRoutingSender->getInterfaceVersion();

   sendTelnetLine(filedescriptor,line);
}

/****************************************************************************/
void CAmTelnetMenuHelper::getReceiverversionCommand(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   instance->getReceiverversionCommandExec(CmdQueue,filedescriptor);
}

/****************************************************************************/
void CAmTelnetMenuHelper::getReceiverversionCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   std::stringstream line;

   line << "Receiver versions:" << std::endl
        << "Ctrl: "     << mControlReceiver->getInterfaceVersion() << " | "
        << "Cmd: "      << mCommandReceiver->getInterfaceVersion() << " | "
        << "Routing: "  << mRoutingReceiver->getInterfaceVersion();

   sendTelnetLine(filedescriptor,line);

}

/****************************************************************************/
void CAmTelnetMenuHelper::infoSystempropertiesCommand(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   instance->infoSystempropertiesCommandExec(CmdQueue,filedescriptor);
}

/****************************************************************************/
void CAmTelnetMenuHelper::infoSystempropertiesCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   std::vector<am_SystemProperty_s> listSystemProperties;
   std::vector<am_SystemProperty_s>::iterator it;
   std::stringstream line;

   mDatabasehandler->getListSystemProperties(listSystemProperties);

   line << "Systemproperties: ";
   sendTelnetLine(filedescriptor,line);

   for(it = listSystemProperties.begin(); it < listSystemProperties.end(); it++ )
   {
      line.clear();
      line << "Type: " <<  it->type << " Value: " << it->value;
      sendTelnetLine(filedescriptor,line);
   }
}

/****************************************************************************/
void CAmTelnetMenuHelper::setRoutingCommand(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   instance->setRoutingCommandExec(CmdQueue,filedescriptor);
}

/****************************************************************************/
void CAmTelnetMenuHelper::setRoutingCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   std::stringstream                   output;
   std::vector<am_Route_s>             routingList;

   am_sourceID_t                       sourceID = 0;
   am_sinkID_t                         sinkID   = 0;

   bool                                error = false;
   am_Error_e                          rError = E_OK;


   if(CmdQueue.size() >= 2)
   {
      std::istringstream istream_sourceID(CmdQueue.front());
      CmdQueue.pop();
      std::istringstream istream_sinkID(CmdQueue.front());
      CmdQueue.pop();

      if(!(istream_sourceID >> sourceID))
         error = true;
      if(!(istream_sinkID >> sinkID))
         error = true;

      if(error)
      {
         sendError(filedescriptor,"Error parsing sourcID and sinkID");
         return;
      }

      if(DEBUG_ON)std::cout << "setRoutingCommandExec(sourceID: " << sourceID << ",sinkID: " << sinkID << ")" << std::endl;

      rError = mRouter->getRoute(true,sourceID,sinkID,routingList);

      if(E_OK == rError)
      {
         std::vector<am_Route_s>::iterator rlIter = routingList.begin();
         for(int rlCnt = 1;rlIter < routingList.end();rlIter++)
         {
            output << "#" << rlCnt << " ";

            std::vector<am_RoutingElement_s>::iterator reIter = rlIter->route.begin();
            for(;reIter < rlIter->route.end();reIter++)
            {
               reIter->connectionFormat;
               reIter->domainID;
               output << ">(" << reIter->sourceID << ")->--[D:"<< reIter->domainID <<"F:"<< reIter->connectionFormat <<"]-->-(" << reIter->sinkID<< ")" << std::endl;
            }

            rlCnt++;
         }

         sendTelnetLine(filedescriptor,output);
      }
      else
      {
         sendError(filedescriptor,"Error getting route");
      }

   }
   else
   {
      CmdQueue.pop();
      output << "Not enough arguments to set routing. Please enter sourceID and sinkID after command" << std::endl;
      return;
   }


}

/****************************************************************************/
void CAmTelnetMenuHelper::setConnection(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   instance->setConnectionExec(CmdQueue,filedescriptor);
}

/****************************************************************************/
void CAmTelnetMenuHelper::setConnectionExec(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   std::stringstream                   output;

   am_sourceID_t                       sourceID = 0;
   am_sinkID_t                         sinkID   = 0;
   am_mainConnectionID_t               connID   = 0;

   bool                                error = false;
   am_Error_e                          rError = E_OK;

   if(CmdQueue.size() >= 2)
   {
      std::istringstream istream_sourceID(CmdQueue.front());
      CmdQueue.pop();

      std::istringstream istream_sinkID(CmdQueue.front());
            CmdQueue.pop();

      if(!(istream_sourceID >> sourceID))
         error = true;

      if(!(istream_sinkID >> sinkID))
         error = true;

      if(error)
      {
         sendError(filedescriptor,"Error parsing sinkID and/or sourceID");
         return;
      }

      // Try to set up connection
      rError = mCommandReceiver->connect(sourceID,sinkID,connID);

      if(E_OK == rError)
      {
         output << "ConnID: " << connID << "Src: " << sourceID << " ---> Sink: " << sinkID << std::endl;
         sendTelnetLine(filedescriptor,output);
      }
      else
      {
         sendError(filedescriptor,"Error connecting sourceID and sinkID");
      }

   }
   else
   {
      CmdQueue.pop();
      sendError(filedescriptor,"Not enough arguments to set routing. Please enter sourceID and sinkID after command");
      return;
   }
}

/****************************************************************************/
void CAmTelnetMenuHelper::setDisconnectConnId(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   instance->setConnectionExec(CmdQueue,filedescriptor);
}

/****************************************************************************/
void CAmTelnetMenuHelper::setDisconnectConnIdExec(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   std::stringstream                   output;

   am_mainConnectionID_t               connID   = 0;

   bool                                error = false;
   am_Error_e                          rError = E_OK;

   if(CmdQueue.size() >= 1)
   {
      std::istringstream istream_connID(CmdQueue.front());
      CmdQueue.pop();

      if(!(istream_connID >> connID))
         error = true;

      if(error)
      {
         sendError(filedescriptor,"Error parsing connID");
         return;
      }

      // Try to disconnect connection id
      rError = mCommandReceiver->disconnect(connID);

      if(E_OK == rError)
      {
         output << "ConnID " << connID << " closed successfully! " << std::endl;
         sendTelnetLine(filedescriptor,output);
      }
      else
      {
         sendError(filedescriptor,"Error connecting sourceID and sinkID");
      }
   }
   else
   {
      sendError(filedescriptor,"Not enough arguments to disconnect a Main Connection, please enter 'connection ID' after command");
      return;
   }
}

/****************************************************************************/
void CAmTelnetMenuHelper::setSourceSoundProperties(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   instance->setConnectionExec(CmdQueue,filedescriptor);
}

/****************************************************************************/
void CAmTelnetMenuHelper::setSourceSoundPropertiesExec(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   std::stringstream    output;
   am_sinkID_t          sourceID;
   am_MainSoundProperty_s soundProperty;
   unsigned int tmpType = 0;
   bool error = false;

   if(CmdQueue.size() >= 3)
   {
      std::istringstream istream_sourceID(CmdQueue.front());
      CmdQueue.pop();

      std::istringstream istream_type(CmdQueue.front());
      CmdQueue.pop();

      std::istringstream istream_value(CmdQueue.front());
      CmdQueue.pop();

      if(!(istream_type >> tmpType))
         error = true;

      if(tmpType < MSP_MAX)
         soundProperty.type = static_cast<am_MainSoundPropertyType_e>(tmpType);
      else
         error = true;

      if(!(istream_value >> soundProperty.value))
         error = true;

      if(!(istream_sourceID >> sourceID))
         error = true;

      if(error)
      {
         sendError(filedescriptor,"Error parsing MainSinkSoundProperty 'type', 'value' or 'sourceID'");
         return;
      }

      if(E_OK == mCommandReceiver->setMainSourceSoundProperty(soundProperty,sourceID))
      {
         output << "MainSourceSoundProperty set: " << soundProperty.type << "->" << soundProperty.value << std::endl;
         sendTelnetLine(filedescriptor,output);
      }
      else
      {
         sendError(filedescriptor,"Error setMainSourceSoundProperty");
      }
   }
   else
   {
      sendError(filedescriptor,"Not enough arguments to set MainSourceSoundProperty, please enter 'sourceID', 'type' and 'value' after command");
      return;
   }
}

/****************************************************************************/
void CAmTelnetMenuHelper::setSinkSoundProperties(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   instance->setConnectionExec(CmdQueue,filedescriptor);
}

/****************************************************************************/
void CAmTelnetMenuHelper::setSinkSoundPropertiesExec(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   std::stringstream    output;
   am_sinkID_t          sinkID;
   am_MainSoundProperty_s soundProperty;
   unsigned int tmpType = 0;
   bool error = false;

   if(CmdQueue.size() >= 3)
   {
      std::istringstream istream_sinkID(CmdQueue.front());
      CmdQueue.pop();

      std::istringstream istream_type(CmdQueue.front());
      CmdQueue.pop();

      std::istringstream istream_value(CmdQueue.front());
      CmdQueue.pop();

      if(!(istream_type >> tmpType))
         error = true;

      if(tmpType < MSP_MAX)
         soundProperty.type = static_cast<am_MainSoundPropertyType_e>(tmpType);
      else
         error = true;

      if(!(istream_value >> soundProperty.value))
         error = true;

      if(!(istream_sinkID >> sinkID))
         error = true;

      if(error)
      {
         sendError(filedescriptor,"Error parsing MainSinkSoundProperty 'type', 'value' or 'sinkID'");
         return;
      }

      if(E_OK == mCommandReceiver->setMainSinkSoundProperty(soundProperty,sinkID))
      {
         output << "MainSinkSoundProperty set: " << soundProperty.type << "->" << soundProperty.value << std::endl;
         sendTelnetLine(filedescriptor,output);
      }
      else
      {
         sendError(filedescriptor,"Error setMainSinkSoundProperty");
      }
   }
   else
   {
      sendError(filedescriptor,"Not enough arguments to set MainSinkSoundProperty, please enter 'sinkID', 'type' and 'value' after command");
      return;
   }
}


/****************************************************************************/
void CAmTelnetMenuHelper::listPluginsCommand(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   instance->listPluginsCommandExec(CmdQueue,filedescriptor);
}

/****************************************************************************/
void CAmTelnetMenuHelper::listPluginsCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor)
/****************************************************************************/
{
   std::vector<std::string> PlugInNames;
   std::vector<std::string>::iterator iter;
   std::stringstream output;
   am_Error_e rError = E_OK;


   rError = mCommandSender->getListPlugins(PlugInNames);

   output << "CommandSender Plugins loaded: " << PlugInNames.size() << std::endl;

   for(iter = PlugInNames.begin(); iter < PlugInNames.end(); iter++ )
   {
      output << iter->c_str() << std::endl;
   }

   rError = mRoutingSender->getListPlugins(PlugInNames);

   output << std::endl << "RoutingSender Plugins loaded: " << PlugInNames.size() << std::endl;

   for(iter = PlugInNames.begin(); iter < PlugInNames.end(); iter++ )
   {
      output << iter->c_str() << std::endl;
   }

   sendTelnetLine(filedescriptor,output);
}











