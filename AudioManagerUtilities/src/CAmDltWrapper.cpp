/**
 *  SPDX license identifier: MPL-2.0
 *
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
 * \author Christian Linke, christian.linke@bmw.de BMW 2011,2012
 * \author Jens Lorenz, jlorenz@de.adit-jv.com ADIT 2014
 *
 * \file CAmDltWrapper.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include <string.h>
#include <unistd.h>
#include "CAmDltWrapper.h"

using namespace std;


namespace am
{
CAmDltWrapper  *CAmDltWrapper::mpDLTWrapper = NULL;


CAmDltWrapper *CAmDltWrapper::instanctiateOnce(const char *appid, const char *description, const bool debugEnabled, const logDestination logDest, const std::string Filename, bool onlyError)
{
    if (!mpDLTWrapper)
    {
        IAmLogger *pLogger = CAmLogWrapper::instantiateOnce(appid, description
                , (debugEnabled ? LS_ON : LS_OFF), static_cast<am_LogService_e>(logDest)
                , Filename, onlyError);
        mpDLTWrapper = new CAmDltWrapper(pLogger, debugEnabled, onlyError);
    }

    return (mpDLTWrapper);
}

CAmDltWrapper *CAmDltWrapper::instance()
{
    if (!mpDLTWrapper)
    {
        // an application seems not to use our CAmDltWrapper class therefore create default
        std::ostringstream description;
        description << "PID=" << getpid() << " _=" << getenv("_");
        mpDLTWrapper = instanctiateOnce("AMDL", description.str().c_str());
        std::cerr << "Application doesn't call CAmDltWrapper::instanctiateOnce!!!" << std::endl;
        std::cerr << "-> CAmDltWrapper::instance registers DLT application [ AMDL | " << description.str() << " ]" << std::endl;
    }

    return mpDLTWrapper;
}

bool CAmDltWrapper::getEnabled()
{
    return (mDebugEnabled);
}

CAmDltWrapper::CAmDltWrapper(IAmLogger *pLogger, bool debugEnabled, bool onlyError)
    : mpLogger(pLogger)
    , mpCurrentContext(NULL)
    , mDebugEnabled(debugEnabled)
    , mOnlyError(onlyError)
{

}

CAmDltWrapper::~CAmDltWrapper()
{
    for (auto context : mMapContext)
    {
        unregisterContext(*(context.first));
    }

    delete mpLogger;
    mpLogger = NULL;

    mpDLTWrapper = NULL;
}

void CAmDltWrapper::unregisterContext(DltContext &handle)
{
    if (mpLogger)
    {
        string ctxID(handle.contextID, DLT_ID_SIZE);
        mpLogger->unregisterContext(ctxID.c_str());
        mMapContext.erase(&handle);
    }
}

void CAmDltWrapper::deinit()
{
    if (mpCurrentContext)
    {
        mpCurrentContext->configure(mOnlyError ? LL_ERROR : LL_INFO);
    }
    mpCurrentContext = NULL;
}

void CAmDltWrapper::registerContext(DltContext &handle, const char *contextid, const char *description)
{
    if (mpLogger)
    {
        mpLogger->registerContext(contextid, description);
        size_t len = min(DLT_ID_SIZE, 1 + (int)strlen(contextid));
        strncpy(handle.contextID, contextid, len);
        mMapContext.emplace(&handle, std::string(contextid));
    }
}

void CAmDltWrapper::registerContext(DltContext &handle, const char *contextid, const char *description
        , DltLogLevelType level, DltTraceStatusType status)
{
    if (mpLogger)
    {
        if (level == DLT_LOG_DEFAULT)
        {
            logWarning("CAmDltWrapper::registerContext - understanding DLT_LOG_DEFAULT as DLT_LOG_INFO");
            level = DLT_LOG_INFO;
        }
        if (status == DLT_TRACE_STATUS_DEFAULT)
        {
            logError("CAmDltWrapper::registerContext - understanding DLT_TRACE_STATUS_DEFAULT as DLT_TRACE_STATUS_ON");
            status = DLT_TRACE_STATUS_ON;
        }

        size_t len = min(DLT_ID_SIZE, 1 + (int)strlen(contextid));
        mpLogger->registerContext(contextid, description
                , static_cast<am_LogLevel_e>(level), static_cast<am_LogStatus_e>(status));
        strncpy(handle.contextID, contextid, len);
        mMapContext.emplace(&handle, std::string(contextid));
    }
}


bool CAmDltWrapper::init(DltLogLevelType loglevel, DltContext *context)
{
    if (mpLogger)
    {
        if (context)
        {
            string mCurrentContextID(context->contextID, DLT_ID_SIZE);
            mpCurrentContext = &(mpLogger->importContext(mCurrentContextID.c_str()));
        }
        else
        {
            mpCurrentContext = &(mpLogger->importContext());
        }
        mpCurrentContext->configure(static_cast<am_LogLevel_e>(loglevel));
    }

    return true;
}

void CAmDltWrapper::send()
{
    if (mpCurrentContext)
    {
        mpCurrentContext->send();
    }
    mpCurrentContext = NULL;
}

}
