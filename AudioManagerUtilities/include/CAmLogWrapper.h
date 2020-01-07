/**
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2017, ADIT GmbH
 *
 * \author Mattia Guerra, mguerra@de.adit-jv.com ADIT 2017
 * \author Jens Lorenz, jlorenz@de.adit-jv.com ADIT 2017
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \file CAmLogWrapper.h
 * For further information see http://www.genivi.org/.
 */

#ifndef LOGWRAPPER_H_
#define LOGWRAPPER_H_

#include <audiomanagerconfig.h>
#include "pthread.h"
#include "IAmLogger.h"
#include "audiomanagertypes.h"

namespace am
{
/**
 * This class is instantiated as a singleton and offers logging to a default context (maincontext).
 * Logging under the default context can simply be done with the logDebug/logInfo/logError/logWarning/logVerbose
 * templates. For logging with a different context, you need to register a context with registerContext,
 * method provided by backends implementing the IAmLogger interface. registerContext returns a logging context.
 * To import this context into other classes use importContext, method provided by backends implementing the IAmLogger
 * interface. To access the IAmLogger interface, simply ask CAmLogWrapper::instance().
 */
class CAmLogWrapper
{
public:
    static IAmLogger *instantiateOnce(const char *appid, const char *description,
        const am_LogStatus_e logStatus = LS_ON, const am_LogService_e logService = DEFAULT_LOG_SERVICE,
        const std::string Filename = "", bool onlyError = false);
    static IAmLogger *instance(const am_LogService_e logservice = DEFAULT_LOG_SERVICE);

    inline static am_LogService_e getLogService()
    {
        return mLogService;
    }

    virtual ~CAmLogWrapper();

private:
    CAmLogWrapper(void);             //!< is private because of singleton pattern
    static IAmLogger      *mpLogger; //!< pointer to the logger instance
    static std::string     mAppId;
    static std::string     mDescription;
    static am_LogStatus_e  mLogStatus;
    static am_LogService_e mLogService;
    static std::string     mFilename;
    static bool            mOnlyError;
};

/**
 * returns the instance of instantiated IAmLogger
 * @return
 */
inline IAmLogger *getLogger()
{
    return (CAmLogWrapper::instance(CAmLogWrapper::getLogService()));
}

/**
 * logs given values with debuglevel with the default context
 * @param value
 * @param ...
 */
template<typename T, typename... TArgs>
void logDebug(const T &value, const TArgs & ... args)
{
    getLogger()->logToDefaultContext(LL_DEBUG, value, args...);
}

/**
 * logs given values with infolevel with the default context
 * @param value
 * @param ...
 */
template<typename T, typename... TArgs>
void logInfo(const T &value, const TArgs & ... args)
{
    getLogger()->logToDefaultContext(LL_INFO, value, args...);
}

/**
 * logs given values with errorlevel with the default context
 * @param value
 * @param ...
 */
template<typename T, typename... TArgs>
void logError(const T &value, const TArgs & ... args)
{
    getLogger()->logToDefaultContext(LL_ERROR, value, args...);
}

/**
 * logs given values with warninglevel with the default context
 * @param value
 * @param ...
 */
template<typename T, typename... TArgs>
void logWarning(const T &value, const TArgs & ... args)
{
    getLogger()->logToDefaultContext(LL_WARN, value, args...);
}

/**
 * logs given values with verboselevel with the default context
 * @param value
 * @param ...
 */
template<typename T, typename... TArgs>
void logVerbose(const T &value, const TArgs & ... args)
{
    getLogger()->logToDefaultContext(LL_VERBOSE, value, args...);
}

}

#endif /* LOGWRAPPER_H_ */
