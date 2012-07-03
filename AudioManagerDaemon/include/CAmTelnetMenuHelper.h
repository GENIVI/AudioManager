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
 * \file CAmTelnetMenuHelper.h
 * For further information see http://www.genivi.org/.
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
#include <sys/socket.h>
#include "audiomanagertypes.h"

namespace am
{

class CAmTelnetServer;
class CAmDatabaseHandler;
class CAmCommandSender;
class CAmRoutingSender;
class CAmControlSender;
class CAmCommandReceiver;
class CAmRoutingReceiver;
class CAmControlReceiver;

class CAmRouter;
class CAmSocketHandler;

/**
 * helper class for CAmTelnetServer
 */
class CAmTelnetMenuHelper
{
public:

    enum EMainState
    {
        eRootState = 0, eListState, eInfoState, eGetState, eSetState
    };

    CAmTelnetMenuHelper(CAmSocketHandler *iSocketHandler, CAmCommandSender *iCommandSender, CAmCommandReceiver *iCommandReceiver, CAmRoutingSender *iRoutingSender, CAmRoutingReceiver *iRoutingReceiver, CAmControlSender *iControlSender, CAmControlReceiver *iControlReceiver, CAmDatabaseHandler *iDatabasehandler, CAmRouter *iRouter, CAmTelnetServer *iTelnetServer);

    ~CAmTelnetMenuHelper();

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
    static void listMainConnectionsCommand(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void listMainConnectionsCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor);
    static void listMainSourcesCommand(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void listMainSourcesCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor);
    static void listMainSinksCommand(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void listMainSinksCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor);

    // SET commands
    static void setRoutingCommand(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void setRoutingCommandExec(std::queue<std::string> & CmdQueue, int & filedescriptor);
    static void setConnection(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void setConnectionExec(std::queue<std::string> & CmdQueue, int & filedescriptor);
    static void setDisconnectConnId(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void setDisconnectConnIdExec(std::queue<std::string> & CmdQueue, int & filedescriptor);
    static void setSourceSoundProperties(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void setSourceSoundPropertiesExec(std::queue<std::string> & CmdQueue, int & filedescriptor);
    static void setSinkSoundProperty(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void setSinkSoundPropertyExec(std::queue<std::string> & CmdQueue, int & filedescriptor);
    static void setSinkVolume(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void setSinkVolumeExec(std::queue<std::string> & CmdQueue, int & filedescriptor);
    static void setVolumeStep(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void setVolumeStepExec(std::queue<std::string> & CmdQueue, int & filedescriptor);
    static void setSinkMuteState(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void setSinkMuteStateExec(std::queue<std::string> & CmdQueue, int & filedescriptor);
    static void setSourceSoundProperty(std::queue<std::string> & CmdQueue, int & filedescriptor);
    void setSourceSoundPropertyExec(std::queue<std::string> & CmdQueue, int & filedescriptor);

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
    std::map<int, EMainState> mCurrentMainStateMap; //!< int filedescriptor of socket connection; EMainState state of current telnet session

    static CAmTelnetMenuHelper* instance;
    CAmTelnetServer *mpTelenetServer;
    CAmSocketHandler *mpSocketHandler;
    CAmCommandSender *mpCommandSender;
    CAmCommandReceiver *mpCommandReceiver;
    CAmRoutingSender *mpRoutingSender;
    CAmRoutingReceiver *mpRoutingReceiver;
    CAmControlSender *mpControlSender;
    CAmControlReceiver *mpControlReceiver;
    CAmDatabaseHandler *mpDatabasehandler;
    CAmRouter *mpRouter;

    tCommandMap mRootCommands;
    tCommandMap mListCommands;
    tCommandMap mGetCommands;
    tCommandMap mSetCommands;
    tCommandMap mInfoCommands;

};
// class CAmTelnetMenuHelper
}// namespace am

#endif // CAMTELNETMENUHELPER_H_
