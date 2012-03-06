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

#ifndef ROUTINGINTERFACEBACKDOOR_H_
#define ROUTINGINTERFACEBACKDOOR_H_

#include "CAmRoutingSender.h"

namespace am
{

class CAmRoutingSender;

class IAmRoutingBackdoor
{
public:
    IAmRoutingBackdoor();
    virtual ~IAmRoutingBackdoor();
    bool unloadPlugins(CAmRoutingSender *RoutingSender);
    bool injectInterface(CAmRoutingSender *RoutingSender, IAmRoutingSend *newInterface, const std::string& busname);
};

}

//definitions are in CAmCommonFunctions.cpp!

#endif /* ROUTINGINTERFACEBACKDOOR_H_ */
