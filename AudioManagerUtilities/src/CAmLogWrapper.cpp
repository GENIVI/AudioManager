/**
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2017, ADIT GmbH
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
 * \author Mattia Guerra, mguerra@de.adit-jv.com ADIT 2017
 * \author Jens Lorenz, jlorenz@de.adit-jv.com ADIT 2017
 *
 * \file CAmLogWrapper.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include <iostream>
#include <unistd.h>
#include "CAmLogWrapper.h"
#include "CAmLoggerStdOut.h"
#include "CAmLoggerFile.h"
#ifdef WITH_DLT
# include "CAmLoggerDlt.h"
#endif


using namespace std;

namespace am
{

IAmLogger      *CAmLogWrapper::mpLogger     = NULL;
string          CAmLogWrapper::mAppId       = "";
string          CAmLogWrapper::mDescription = "";
am_LogStatus_e  CAmLogWrapper::mLogStatus   = LS_ON;
am_LogService_e CAmLogWrapper::mLogService  = DEFAULT_LOG_SERVICE;
string          CAmLogWrapper::mFilename    = "";
bool            CAmLogWrapper::mOnlyError   = false;

CAmLogWrapper::CAmLogWrapper(void)
{
}

IAmLogger *CAmLogWrapper::instantiateOnce(const char *appid, const char *description,
    const am_LogStatus_e logStatus, const am_LogService_e logService,
    const string fileName, bool onlyError)
{
    if (mpLogger)
    {
        return mpLogger;
    }

    mAppId       = string(appid);
    mDescription = string(description);
    mLogStatus   = logStatus;
    mLogService  = logService;
    mFilename    = fileName;
    mOnlyError   = onlyError;
    return instance(mLogService);
}

IAmLogger *CAmLogWrapper::instance(const am_LogService_e service)
{
    if (mpLogger)
    {
        return mpLogger;
    }

    switch (service)
    {
    case LOG_SERVICE_DLT:
#ifdef WITH_DLT
        mpLogger = new CAmLoggerDlt();
#else
        std::cerr << "Option WITH_DLT not enabled for CAmLogWrapper! "
                  << "Redirecting log output to stdout ..." << std::endl;
        mLogService = LOG_SERVICE_STDOUT;
        mpLogger = new CAmLoggerStdOut(mLogStatus, mOnlyError);
#endif
        break;
    case LOG_SERVICE_STDOUT:
        mpLogger = new CAmLoggerStdOut(mLogStatus, mOnlyError);
        break;
    case LOG_SERVICE_FILE:
        mpLogger = new CAmLoggerFile(mLogStatus, mOnlyError, mFilename);
        break;
    default:
        mpLogger = new CAmLoggerStdOut(LS_OFF);
        break;
    }

    // an application seems not to use our CAmLogWrapper class properly therefore create default context
    if ((mLogStatus == LS_ON) && mAppId.empty() && mDescription.empty())
    {
        mAppId = "AMDL";
        std::ostringstream description;
        description << "PID=" << getpid() << " _=" << getenv("_");
        mDescription = description.str().c_str();
        std::cerr << "Application doesn't call CAmLogWrapper::instantiateOnce!!!" << std::endl;
        std::cerr << "-> CAmLogWrapper::instance registers DLT application [ AMDL | " << description.str() << " ]" << std::endl;
    }

    mpLogger->registerApp(mAppId.c_str(), mDescription.c_str());

    return mpLogger;
}

CAmLogWrapper::~CAmLogWrapper()
{
    if (mpLogger)
    {
        mpLogger->unregisterApp();
        delete mpLogger;
    }
}

}
