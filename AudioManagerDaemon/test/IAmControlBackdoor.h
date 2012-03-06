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

#ifndef CONTROLINTERFACEBACKDOOR_H_
#define CONTROLINTERFACEBACKDOOR_H_

#include "control/IAmControlSend.h"
#include "CAmControlSender.h"

namespace am
{

class CAmControlSender;

class IAmControlBackdoor
{
public:
    IAmControlBackdoor();
    virtual ~IAmControlBackdoor();
    bool replaceController(CAmControlSender *controlSender, IAmControlSend *newController);
};

}

#endif /* CONTROLINTERFACEBACKDOOR_H_ */
