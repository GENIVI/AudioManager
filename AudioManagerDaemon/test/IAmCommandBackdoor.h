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
 *
 * For further information see http://www.genivi.org/.
 *
 */

#ifndef COMMANDINTERFACEBACKDOOR_H_
#define COMMANDINTERFACEBACKDOOR_H_

#include <command/IAmCommandSend.h>
#include "CAmCommandSender.h"

namespace am
{

class CAmCommandSender;

class IAmCommandBackdoor
{
public:
    IAmCommandBackdoor();
    virtual ~IAmCommandBackdoor();
    bool unloadPlugins(CAmCommandSender *CommandSender);
    bool injectInterface(CAmCommandSender* CommandSender, IAmCommandSend* CommandSendInterface);
};

}

//definitions are in CAmCommonFunctions.cpp!

#endif /* COMMANDINTERFACEBACKDOOR_H_ */
