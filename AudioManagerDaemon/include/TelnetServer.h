/**
 * Copyright (C) 2011, BMW AG
 *
 * GeniviAudioMananger AudioManagerDaemon
 *
 * \file TelnetServer.h
 *
 * \date 20-Oct-2011 3:42:04 PM
 * \author Christian Mueller (christian.ei.mueller@bmw.de)
 *
 * \section License
 * GNU Lesser General Public License, version 2.1, with special exception (GENIVI clause)
 * Copyright (C) 2011, BMW AG Christian Mueller  Christian.ei.mueller@bmw.de
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

#ifndef TELNETSERVER_H_
#define TELNETSERVER_H_

#include "SocketHandler.h"
#include <queue>
#include <map>

namespace am
{

class DatabaseHandler;
class CommandSender;
class RoutingSender;
class ControlSender;
class CommandReceiver;
class RoutingReceiver;
class ControlReceiver;

class TelnetServer
{
public:
    TelnetServer(SocketHandler *iSocketHandler, CommandSender *iCommandSender, CommandReceiver *iCommandReceiver, RoutingSender *iRoutingSender, RoutingReceiver *iRoutingReceiver, ControlSender *iControlSender, ControlReceiver *iControlReceiver, DatabaseHandler *iDatabasehandler, unsigned int servPort, unsigned int maxConnections);
    virtual ~TelnetServer();
    void connectSocket(const pollfd pfd, const sh_pollHandle_t handle, void* userData);
    void receiveData(const pollfd pfd, const sh_pollHandle_t handle, void* userData);
    bool dispatchData(const sh_pollHandle_t handle, void* userData);
    bool check(const sh_pollHandle_t handle, void* userData);
    shPollFired_T<TelnetServer> telnetConnectFiredCB;
    shPollFired_T<TelnetServer> telnetReceiveFiredCB;
    shPollDispatch_T<TelnetServer> telnetDispatchCB;
    shPollCheck_T<TelnetServer> telnetCheckCB;
private:
    typedef void (*CommandPrototype)(std::vector<std::string>& msg, int filedescriptor);
    typedef std::map<std::string, CommandPrototype> mMapCommand_t;
    static void listCommand(std::vector<std::string>& msg, int filedescriptor);
    void listCommandShadow(std::vector<std::string>& msg, int filedescriptor);
    void sliceCommand(const std::string& string, std::string& command, std::vector<std::string>& msg);
    mMapCommand_t createCommandMap();
    struct connection_s
    {
        int filedescriptor;
        sh_pollHandle_t handle;
    };

    static TelnetServer* instance;
    SocketHandler *mSocketHandler;
    CommandSender *mCommandSender;
    CommandReceiver *mCommandReceiver;
    RoutingSender *mRoutingSender;
    RoutingReceiver *mRoutingReceiver;
    ControlSender *mControlSender;
    ControlReceiver *mControlReceiver;
    DatabaseHandler *mDatabasehandler;
    sh_pollHandle_t mConnecthandle;
    std::queue<std::string> msgList;
    std::vector<connection_s> mListConnections;
    int mConnectFD;
    unsigned int mServerPort;
    unsigned int mMaxConnections;
    mMapCommand_t mMapCommands;

};

} /* namespace am */
#endif /* TELNETSERVER_H_ */
