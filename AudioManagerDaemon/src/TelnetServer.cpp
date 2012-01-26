/**
 * Copyright (C) 2011, BMW AG
 *
 * GeniviAudioMananger AudioManagerDaemon
 *
 * \file TelnetServer.cpp
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

#include "TelnetServer.h"
#include <assert.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <string.h>
#include <netdb.h>
#include <dlt/dlt.h>
#include <config.h>
#include <errno.h>
#include <sstream>
#include <istream>
#include <iostream>
#include <iterator>
#include "DatabaseHandler.h"
#include "RoutingSender.h"

using namespace am;

DLT_IMPORT_CONTEXT(AudioManager)

TelnetServer* TelnetServer::instance = NULL;

#define PRINT_BOOL(var) var ? output+="true\t\t" : output+="false\t\t";

TelnetServer::TelnetServer(SocketHandler *iSocketHandler, CommandSender *iCommandSender, CommandReceiver *iCommandReceiver, RoutingSender *iRoutingSender, RoutingReceiver *iRoutingReceiver, ControlSender *iControlSender, ControlReceiver *iControlReceiver, DatabaseHandler *iDatabasehandler, unsigned int servPort, unsigned int maxConnections) :
        telnetConnectFiredCB(this, &TelnetServer::connectSocket), //
        telnetReceiveFiredCB(this, &TelnetServer::receiveData), //
        telnetDispatchCB(this, &TelnetServer::dispatchData), //
        telnetCheckCB(this, &TelnetServer::check), //
        mSocketHandler(iSocketHandler), //
        mCommandSender(iCommandSender), //
        mCommandReceiver(iCommandReceiver), //
        mRoutingSender(iRoutingSender), //
        mRoutingReceiver(iRoutingReceiver), //
        mControlSender(iControlSender), //
        mControlReceiver(iControlReceiver), //
        mDatabasehandler(iDatabasehandler), //
        mConnecthandle(), //
        msgList(), //
        mListConnections(), //
        mConnectFD(NULL), //
        mServerPort(servPort), //
        mMaxConnections(maxConnections), //
        mMapCommands(createCommandMap())
{
    assert(mSocketHandler!=NULL);
    assert(mCommandReceiver!=NULL);
    assert(mCommandSender!=NULL);
    assert(mControlSender!=NULL);
    assert(mControlReceiver!=NULL);
    assert(mRoutingSender!=NULL);
    assert(mRoutingReceiver!=NULL);
    assert(mDatabasehandler!=NULL);
    assert(servPort!=0);
    assert(mMaxConnections!=0);

    instance = this;

    int yes = 1;
    struct sockaddr_in servAddr;

    //setup the port Listener
    mConnectFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    setsockopt(mConnectFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;
    servAddr.sin_port = htons(servPort);
    bind(mConnectFD, (struct sockaddr *) &servAddr, sizeof(servAddr));

    if (listen(mConnectFD, mMaxConnections) < 0)
    {
        DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("TelnetServer::TelnetServerk cannot listen "), DLT_INT(errno));
    }DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("TelnetServer::TelnetServerk started listening on port"), DLT_INT(mServerPort));

    int a = 1;
    ioctl(mConnectFD, FIONBIO, (char *) &a);
    setsockopt(mConnectFD, SOL_SOCKET, SO_KEEPALIVE, (char *) &a, sizeof(a));

    short events = 0;
    events |= POLLIN;
    mSocketHandler->addFDPoll(mConnectFD, events, NULL, &telnetConnectFiredCB, NULL, NULL, NULL, mConnecthandle);
}

TelnetServer::~TelnetServer()
{
}

void TelnetServer::connectSocket(const pollfd pfd, const sh_pollHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    //first, accept the connection, create a new filedescriptor
    struct sockaddr answer;
    socklen_t len = sizeof(answer);
    connection_s connection;
    connection.filedescriptor = accept(pfd.fd, (struct sockaddr*) &answer, &len);

    mListConnections.push_back(connection);

    //set the correct event:
    short event = 0;
    event |= POLLIN;

    //aded the filedescriptor to the sockethandler and register the callbacks for receiving the data
    mSocketHandler->addFDPoll(mListConnections.back().filedescriptor, event, NULL, &telnetReceiveFiredCB, &telnetCheckCB, &telnetDispatchCB, NULL, mListConnections.back().handle);
}

void TelnetServer::receiveData(const pollfd pollfd, const sh_pollHandle_t handle, void *userData)
{
    (void)handle;
    (void)userData;
    //initialize buffer
    char buffer[100];
    //read until buffer is full or no more data is there
    int read = recv(pollfd.fd, buffer, 100, NULL);
    if (read > 1)
    {
        //read the message and store it in a queue - its a telnet connection so data will be sent on enter !
        std::string msg = std::string(buffer, read);
        msgList.push(msg);
    }
}

bool TelnetServer::dispatchData(const sh_pollHandle_t handle, void *userData)
{
    (void)handle;
    (void)userData;
    std::vector<connection_s>::iterator iterator = mListConnections.begin();
    for (; iterator != mListConnections.end(); ++iterator)
    {
        if (iterator->handle == handle) break;
    }
    if (iterator == mListConnections.end()) return false;

    std::string command;
    std::vector<std::string> msg;
    sliceCommand(msgList.front(), command, msg);
    msgList.pop();
    mMapCommand_t::iterator commandIter = mMapCommands.find(command);
    if (commandIter == mMapCommands.end())
    {
        send(iterator->filedescriptor, "Command not found!\n", 20, 0);
    }
    else
    {
        (*commandIter).second(msg, iterator->filedescriptor);
    }

    //remove the message from the queue and return false if there is no more message to read.
    if (msgList.size() != 0) return true;
    return false;
}

bool TelnetServer::check(const sh_pollHandle_t handle, void *userData)
{
    (void)handle;
    (void)userData;
    if (msgList.size() != 0) return true;
    return false;
}

void TelnetServer::listCommand(std::vector<std::string>& msg, int filedescriptor)
{
    instance->listCommandShadow(msg, filedescriptor);
}

void TelnetServer::listCommandShadow(std::vector<std::string> & msg, int filedescriptor)
{
    std::string output;
    if (msg.empty())
    {
        output += "No second parameter given after list, please enter\n";
    }
    else if (msg.front().compare("plugins") == 0)
    {
        std::vector<std::string> plugins;
        mRoutingSender->getListPlugins(plugins);
        std::vector<std::string>::iterator it = plugins.begin();
        output = "\n\nrouting plugins:\n-------------------------\n";
        for (; it != plugins.end(); ++it)
        {
            output += *it + "\n";
        }
    }
    else if (msg.front().compare("domains") == 0)
    {
        output = "domainID\t\tdomainName\t\tbusName\t\tnodeName\t\tearly\t\tstate\t\tcomplete\n";
        output += "-------------------------------------------------------------------------------------\n";
        std::vector<am_Domain_s> domainList;
        mDatabasehandler->getListDomains(domainList);
        std::vector<am_Domain_s>::iterator it = domainList.begin();
        for (; it != domainList.end(); ++it)
        {
            output += it->domainID + "\t\t";
            output += it->name + "\t\t";
            output += it->busname + "\t\t";
            output += it->nodename + "\t\t";
            PRINT_BOOL(it->early);
            switch (it->state)
            {
            case DS_CONTROLLED:
                output += "DS_CONTROLLED\t\t";
                break;
            case DS_INDEPENDENT_STARTUP:
                output += "DS_INDEPENDENT_STARTUP\t\t";
                break;
            case DS_INDEPENDENT_RUNDOWN:
                output += "DS_INDEPENDENT_RUNDOWN\t\t";
                break;
            default:
                output += "undefined\t\t";
                break;
            }PRINT_BOOL(it->complete);
        }
    }
    else
    {
        output = "did not recognize parameter: " + msg.front() + "\n";
    }
    send(filedescriptor, output.c_str(), output.size(), 0);
}

void am::TelnetServer::sliceCommand(const std::string & string, std::string & command, std::vector<std::string> & msg)
{
    std::stringstream stream(string);
    std::istream_iterator<std::string> begin(stream);
    std::istream_iterator<std::string> end;
    command = *begin++;
    msg = std::vector<std::string>(begin, end);
}

TelnetServer::mMapCommand_t TelnetServer::createCommandMap()
{
    mMapCommand_t commands;
    commands.insert(std::make_pair("list", &TelnetServer::listCommand));
    return commands;
}

