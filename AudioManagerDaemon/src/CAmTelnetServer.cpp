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
 * \file CAmTelnetServer.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include "CAmTelnetServer.h"
#include <cassert>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <string.h>
#include <netdb.h>
#include <config.h>
#include <errno.h>
#include <sstream>
#include <istream>
#include <iostream>
#include <iterator>
#include <unistd.h>
#include "CAmDatabaseHandler.h"
#include "CAmRoutingSender.h"
#include "CAmTelnetMenuHelper.h"
#include "shared/CAmDltWrapper.h"

namespace am
{

CAmTelnetServer* CAmTelnetServer::mpInstance = NULL;

#define PRINT_BOOL(var) var ? output+="true\t\t" : output+="false\t\t";

CAmTelnetServer::CAmTelnetServer(CAmSocketHandler *iSocketHandler, CAmCommandSender *iCommandSender, CAmCommandReceiver *iCommandReceiver, CAmRoutingSender *iRoutingSender, CAmRoutingReceiver *iRoutingReceiver, CAmControlSender *iControlSender, CAmControlReceiver *iControlReceiver, CAmDatabaseHandler *iDatabasehandler, CAmRouter *iRouter, unsigned int servPort, unsigned int maxConnections) :
        telnetConnectFiredCB(this, &CAmTelnetServer::connectSocket), //
        telnetReceiveFiredCB(this, &CAmTelnetServer::receiveData), //
        telnetDispatchCB(this, &CAmTelnetServer::dispatchData), //
        telnetCheckCB(this, &CAmTelnetServer::check), //
        mpSocketHandler(iSocketHandler), //
        mpCommandSender(iCommandSender), //
        mpCommandReceiver(iCommandReceiver), //
        mpRoutingSender(iRoutingSender), //
        mpRoutingReceiver(iRoutingReceiver), //
        mpControlSender(iControlSender), //
        mpControlReceiver(iControlReceiver), //
        mpDatabasehandler(iDatabasehandler), //
        mpRouter(iRouter), //
        mConnecthandle(), //
        mListMessages(), //
        mListConnections(), //
        mConnectFD(0), //
        mServerPort(servPort), //
        mMaxConnections(maxConnections), //
        mTelnetMenuHelper(iSocketHandler, iCommandSender, iCommandReceiver, iRoutingSender, iRoutingReceiver, iControlSender, iControlReceiver, iDatabasehandler, iRouter, this)
{
    assert(mpSocketHandler!=NULL);
    assert(mpCommandReceiver!=NULL);
    assert(mpCommandSender!=NULL);
    assert(mpControlSender!=NULL);
    assert(mpControlReceiver!=NULL);
    assert(mpRoutingSender!=NULL);
    assert(mpRoutingReceiver!=NULL);
    assert(mpDatabasehandler!=NULL);
    assert(mpRouter!=NULL);
    assert(servPort!=0);
    assert(mMaxConnections!=0);

    mpInstance = this;
    //mTelnetMenuHelper.setTelnetServer(this);

    int yes = 1;
    struct sockaddr_in servAddr;

    //setup the port Listener
    mConnectFD = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    assert (mConnectFD>0);
    setsockopt(mConnectFD, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;
    servAddr.sin_port = htons(servPort);
    bind(mConnectFD, (struct sockaddr *) &servAddr, sizeof(servAddr));

    if (listen(mConnectFD, mMaxConnections) < 0)
    {
        logError("TelnetServer::TelnetServerk cannot listen ", errno);
    }
    else
        logInfo("TelnetServer::TelnetServer started listening on port", mServerPort);

    int a = 1;
    ioctl(mConnectFD, FIONBIO, (char *) &a);
    setsockopt(mConnectFD, SOL_SOCKET, SO_KEEPALIVE, (char *) &a, sizeof(a));

    short events = 0;
    events |= POLLIN;
    mpSocketHandler->addFDPoll(mConnectFD, events, NULL, &telnetConnectFiredCB, NULL, NULL, NULL, mConnecthandle);
}

CAmTelnetServer::~CAmTelnetServer()
{
}

void CAmTelnetServer::connectSocket(const pollfd pfd, const sh_pollHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    //first, accept the connection, create a new filedescriptor
    struct sockaddr answer;
    socklen_t len = sizeof(answer);
    connection_s connection;
    connection.handle = 0;
    connection.filedescriptor = accept(pfd.fd, (struct sockaddr*) &answer, &len);

    assert(connection.filedescriptor>0);

    // Notiy menuhelper
    mTelnetMenuHelper.newSocketConnection(connection.filedescriptor);

    //set the correct event:
    short event = 0;
    event |= POLLIN;

    //aded the filedescriptor to the sockethandler and register the callbacks for receiving the data
    mpSocketHandler->addFDPoll(connection.filedescriptor, event, NULL, &telnetReceiveFiredCB, &telnetCheckCB, &telnetDispatchCB, NULL, connection.handle);
    mListConnections.push_back(connection);
}

void CAmTelnetServer::disconnectClient(int filedescriptor)
{
    std::vector<connection_s>::iterator iter = mListConnections.begin();
    while (iter != mListConnections.end())
    {
        if (filedescriptor == iter->filedescriptor)
        {
            if (E_OK == mpSocketHandler->removeFDPoll(iter->handle))
            {
                mListConnections.erase(iter);
                close(filedescriptor);
            }
            else
            {
                // TODO: Handle error
            }

            break;
        }
        iter++;
    }
}

void CAmTelnetServer::receiveData(const pollfd pollfd, const sh_pollHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    //initialize buffer
    char buffer[100];
    //read until buffer is full or no more data is there
    int read = recv(pollfd.fd, buffer, 100, 0);
    if (read > 1)
    {
        //read the message and store it in a queue - its a telnet connection so data will be sent on enter !
        std::string msg = std::string(buffer, read);
        mListMessages.push(msg);
    }
}

bool CAmTelnetServer::dispatchData(const sh_pollHandle_t handle, void *userData)
{
    (void) userData;
    std::vector<connection_s>::iterator iterator = mListConnections.begin();
    for (; iterator != mListConnections.end(); ++iterator)
    {
        if (iterator->handle == handle)
            break;
    }
    if (iterator==mListConnections.end())
    {
        logError("CAmTelnetServer::dispatchData could not find handle !");
        return (false);
    }

    std::string command;
    std::queue<std::string> MsgQueue;
    if (!mListMessages.empty())
    {
        sliceCommand(mListMessages.front(), command, MsgQueue);
        mListMessages.pop();
        mTelnetMenuHelper.enterCmdQueue(MsgQueue, iterator->filedescriptor);
    }
    else
    {
        logError("CAmTelnetServer::dispatchData Message queue was empty!");
    }

    // must return false to stop endless polling
    return (false);
}

bool CAmTelnetServer::check(const sh_pollHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;
    if (mListMessages.size() != 0)
        return (true);
    return (false);
}

void am::CAmTelnetServer::sliceCommand(const std::string & string, std::string & command, std::queue<std::string> & MsgQueue)
{
    (void) command;
    std::stringstream stream(string);
    std::istream_iterator<std::string> begin(stream);
    std::istream_iterator<std::string> end;
    std::string cmd;
    bool endOfStream = false;

    int c = 0;

    while (!endOfStream)
    {
        cmd = *begin;
        MsgQueue.push(cmd);
        begin++;

        if (begin == end)
        {
            endOfStream = true;
        }
        c++;
    }
}
}

