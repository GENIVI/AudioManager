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
 * \file CAmTelnetServer.h
 * For further information see http://www.genivi.org/.
 *
 */

#ifndef TELNETSERVER_H_
#define TELNETSERVER_H_

#include <queue>
#include <map>
#include "shared/CAmSocketHandler.h"
#include "CAmTelnetMenuHelper.h"

namespace am
{

class CAmDatabaseHandler;
class CAmCommandSender;
class CAmRoutingSender;
class CAmControlSender;
class CAmCommandReceiver;
class CAmRoutingReceiver;
class CAmControlReceiver;
class CAmRouter;
class CAmTelnetMenuHelper;

/**
 * Implements a telnetserver that can be used to connect to the audiomanager, retrieve some information and use it. For debugginp purposes.
 * For example, launch a telnet session on port 6060:
 * \code telnet localhost 6060 \endcode
 *  more details can be found at the README
 */
class CAmTelnetServer
{
public:
    CAmTelnetServer(CAmSocketHandler *iSocketHandler, CAmCommandSender *iCommandSender, CAmCommandReceiver *iCommandReceiver, CAmRoutingSender *iRoutingSender, CAmRoutingReceiver *iRoutingReceiver, CAmControlSender *iControlSender, CAmControlReceiver *iControlReceiver, CAmDatabaseHandler *iDatabasehandler, CAmRouter *iRouter, unsigned int servPort, unsigned int maxConnections);
    ~CAmTelnetServer();
    void connectSocket(const pollfd pfd, const sh_pollHandle_t handle, void* userData);
    void disconnectClient(int filedescriptor);
    void receiveData(const pollfd pfd, const sh_pollHandle_t handle, void* userData);
    bool dispatchData(const sh_pollHandle_t handle, void* userData);
    bool check(const sh_pollHandle_t handle, void* userData);
    TAmShPollFired<CAmTelnetServer> telnetConnectFiredCB;
    TAmShPollFired<CAmTelnetServer> telnetReceiveFiredCB;
    TAmShPollDispatch<CAmTelnetServer> telnetDispatchCB;
    TAmShPollCheck<CAmTelnetServer> telnetCheckCB;
private:

    typedef void (*CommandPrototype)(std::vector<std::string>& msg, int filedescriptor);
    typedef std::map<std::string, CommandPrototype> mMapCommand_t;

    void sliceCommand(const std::string& string, std::string& command, std::queue<std::string>& msg);
    mMapCommand_t createCommandMap();
    struct connection_s
    {
        int filedescriptor;
        sh_pollHandle_t handle;
    };

    static CAmTelnetServer* mpInstance;
    CAmSocketHandler *mpSocketHandler;
    CAmCommandSender *mpCommandSender;
    CAmCommandReceiver *mpCommandReceiver;
    CAmRoutingSender *mpRoutingSender;
    CAmRoutingReceiver *mpRoutingReceiver;
    CAmControlSender *mpControlSender;
    CAmControlReceiver *mpControlReceiver;
    CAmDatabaseHandler *mpDatabasehandler;
    CAmRouter *mpRouter;
    sh_pollHandle_t mConnecthandle;
    std::queue<std::string> mListMessages;
    std::vector<connection_s> mListConnections;
    int mConnectFD;
    unsigned int mServerPort;
    unsigned int mMaxConnections;
    CAmTelnetMenuHelper mTelnetMenuHelper;

};

} /* namespace am */
#endif /* TELNETSERVER_H_ */
