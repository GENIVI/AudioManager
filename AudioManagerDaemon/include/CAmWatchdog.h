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
 * \file CAmWatchdog.h
 * For further information see http://www.genivi.org/.
 *
 */

#ifndef CAMWATCHDOG_H_
#define CAMWATCHDOG_H_

#include "shared/CAmSocketHandler.h"

namespace am
{

/**
 * Implements the watchdog of the AudioManager with the help of systemd
 */
class CAmWatchdog
{
public:
    CAmWatchdog(CAmSocketHandler* CAmSocketHandler);
    virtual ~CAmWatchdog();
    void startWatchdog(); //!< starts the watchdog by sending ready to systemD
    void timerCallback(sh_timerHandle_t handle, void * userData); //!< the watchdog timer callback
    TAmShTimerCallBack<CAmWatchdog> TimerCallback;

private:
    CAmSocketHandler* mpCAmSocketHandler; //!< pointer to the sockethandler
    sh_timerHandle_t mHandle; //!< handle of the timer
};

} /* namespace am */
#endif /* CAMWATCHDOG_H_ */
