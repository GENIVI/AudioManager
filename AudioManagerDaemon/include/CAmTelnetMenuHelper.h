/**
 * Copyright (C) 2012, BMW AG
 *
 * GeniviAudioMananger AudioManagerDaemon
 *
 * \file CAmTelnetMenuHelper.h
 *
 * \date 23-Jan-2012
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

// Local header
#ifndef CAMTELNETMENUHELPER_H_
#define CAMTELNETMENUHELPER_H_

// Standard header
#include <iostream>
#include <queue>
#include <map>
#include <string>
#include <sstream>
#include <vector>

// for sockets
#include <sys/socket.h>
#include <audiomanagertypes.h>

namespace am
{

class TelnetServer;
class DatabaseHandler;
class CommandSender;
class RoutingSender;
class ControlSender;
class CommandReceiver;
class RoutingReceiver;
class ControlReceiver;
class Router;
class SocketHandler;

class CAmTelnetMenuHelper
{
public:

    enum EMainState
    {
        eRootState = 0, eListState, eInfoState, eGetState, eSetState
    };

    CAmTelnetMenuHelper(SocketHandler *iSocketHandler, CommandSender *iCommandSender, CommandReceiver *iCommandReceiver, RoutingSender *iRoutingSender, RoutingReceiver *iRoutingReceiver, ControlSender *iControlSender, ControlReceiver *iControlReceiver, DatabaseHandler *iDatabasehandler, Router *iRouter);

    ~CAmTelnetMenuHelper();

    void setTelnetServer(TelnetServer* iTelnetServer);

    void newSocketConnection(int filedescriptor);

    void socketConnectionsClosed(int filedescriptor);

    void enterCmdQueue(std::queue<std::string> &CmdQueue, int &filedescriptor);

private:

    void createCommandMaps();
    void sendError(int & filedescriptor, std::string error_string);
    void sendTelnetLine(int & filedescriptor, std::stringstream &line);
    void sendCurrentCmdPrompt(int &filedescriptor);

    // COMMON commands
    static void oneStepBackCommand(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void oneStepBackCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor);
    static void exitCommand(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void exitCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor);
    static void helpCommand(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void helpCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor);

    // ROOT commands
    static void rootGetCommand(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void rootGetCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor);
    static void rootSetCommand(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void rootSetCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor);
    static void rootListCommand(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void rootListCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor);
    static void rootInfoCommand(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void rootInfoCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor);

    // LIST commands
    static void listConnectionsCommand(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void listConnectionsCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor);
    static void listSourcesCommand(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void listSourcesCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor);
    static void listSinksCommands(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void listSinksCommandsExec(std::queue<std::string> & CmdQueue, int & filedescriptor);
    static void listCrossfaders(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void listCrossfadersExec(std::queue<std::string> & CmdQueue, int & filedescriptor);
    static void listDomainsCommand(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void listDomainsCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor);
    static void listGatewaysCommand(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void listGatewaysCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor);
    static void listPluginsCommand(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void listPluginsCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor);

    // SET commands
    static void setRoutingCommand(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void setRoutingCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor);
    static void setConnection(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void setConnectionExec(std::queue<std::string> & CmdQueue, int & filedescriptor);
    static void setDisconnectConnId(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void setDisconnectConnIdExec(std::queue<std::string> & CmdQueue, int & filedescriptor);
    static void setSourceSoundProperties(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void setSourceSoundPropertiesExec(std::queue<std::string> & CmdQueue, int & filedescriptor);
    static void setSinkSoundProperties(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void setSinkSoundPropertiesExec(std::queue<std::string> & CmdQueue, int & filedescriptor);

    // GET commands
    static void getRoutingCommand(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void getRoutingCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor);
    static void getSenderversionCommand(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void getSenderversionCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor);
    static void getReceiverversionCommand(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void getReceiverversionCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor);

    // INFO commands
    static void infoSystempropertiesCommand(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void infoSystempropertiesCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor);

private:

    typedef void (*pCommandPrototype)(std::queue<std::string>& msg, int & filedescriptor);

    struct sCommandPrototypeInfo
    {
        std::string info;
        pCommandPrototype CommandPrototype;

        // default contructor to set NULL
        sCommandPrototypeInfo() :
                info(""), CommandPrototype(NULL)
        {
        }

        // a small contructor
        sCommandPrototypeInfo(std::string MyInfo, pCommandPrototype MyCommandPrototype) :
                info(MyInfo), CommandPrototype(MyCommandPrototype)
        {
        }
    };

    typedef std::map<std::string, sCommandPrototypeInfo> tCommandMap;

    // int filedescriptor of socket connection; EMainState state of current telnet session
    std::map<int, EMainState> mCurrentMainStateMap;

    static CAmTelnetMenuHelper* instance;
    TelnetServer *mTelenetServer;
    SocketHandler *mSocketHandler;
    CommandSender *mCommandSender;
    CommandReceiver *mCommandReceiver;
    RoutingSender *mRoutingSender;
    RoutingReceiver *mRoutingReceiver;
    ControlSender *mControlSender;
    ControlReceiver *mControlReceiver;
    DatabaseHandler *mDatabasehandler;
    Router *mRouter;

    tCommandMap mRootCommands;
    tCommandMap mListCommands;
    tCommandMap mGetCommands;
    tCommandMap mSetCommands;
    tCommandMap mInfoCommands;

};
// class CAmTelnetMenuHelper
}// namespace am

#endif // CAMTELNETMENUHELPER_H_
