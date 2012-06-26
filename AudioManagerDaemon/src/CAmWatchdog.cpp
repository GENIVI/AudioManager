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
 * \file CAmWatchdog.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include "CAmWatchdog.h"
#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include "config.h"
#include "shared/CAmDltWrapper.h"

#ifdef SYSTEMD_FOUND
    #include <systemd/sd-daemon.h>
#else
    #include "sd-daemon.h"
#endif

namespace am
{

CAmWatchdog::CAmWatchdog(CAmSocketHandler* CAmSocketHandler) :
        TimerCallback(this, &CAmWatchdog::timerCallback), //
        mpCAmSocketHandler(CAmSocketHandler), //
        mHandle(0) //
{
    assert(mpCAmSocketHandler);


    //first retrieve the timeout interval

    int watchdogTimeout = 0;

    char* wusec=getenv("WATCHDOG_USEC");
    if (wusec)
        watchdogTimeout=atoi(wusec);

    if (watchdogTimeout > 0)
    {
        timespec timeout;
        logInfo("CAmWatchdog::CAmWatchdog setting watchdog timeout to ", watchdogTimeout, " museconds");

        //calulate the half cycle as the right interval to trigger the watchdog.
        timeout.tv_sec = watchdogTimeout / 2000000;
        timeout.tv_nsec = (watchdogTimeout % 1000000) * 500;

        //add the timer here
        if (mpCAmSocketHandler->addTimer(timeout, &TimerCallback, mHandle, NULL))
        {
            logError("CAmWatchdog::CAmWatchdog failed to add timer");
            throw std::runtime_error("CAmWatchdog::CAmWatchdog failed to add timer");
        }
    }

    else
    {
        logInfo("CAmWatchdog::CAmWatchdog watchdog timeout was ", watchdogTimeout, " museconds, no watchdog active");
    }
}

CAmWatchdog::~CAmWatchdog()
{
    //remove the timer again.
    mpCAmSocketHandler->removeTimer(mHandle);
}

void CAmWatchdog::timerCallback(sh_timerHandle_t handle, void* userData)
{
    (void) userData;
    int error(sd_notify(0, "WATCHDOG=1"));
    if (error < 0)
    {
        logError("CAmWatchdog::timerCallback could not reset watchdog, error ", error);
        throw std::runtime_error("CAmWatchdog::timerCallback could not reset watchdog");
    }

    mpCAmSocketHandler->restartTimer(handle);
    logInfo("restarted watchdog ");
}

void CAmWatchdog::startWatchdog()
{
    int error(sd_notify(0, "READY=1"));
    if (error < 0)
    {
        logError("CAmWatchdog::startWatchdog could not start watchdog, error ", error);
        throw std::runtime_error("CAmWatchdog::startWatchdog could not start watchdog");
    }
    logInfo("READY=1 was sent to systemd");
}

}

/* namespace am */
