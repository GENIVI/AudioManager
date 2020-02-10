/**
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2012, BMW AG
 *
 * \author Christian Linke, christian.linke@bmw.de BMW 2011,2012
 * \author Jens Lorenz, jlorenz@de.adit-jv.com ADIT 2014
 * \author Martin Koch, mkoch@de.adit-jv.com ADIT 2020
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \file CAmDltWrapper.h
 * This file is reduced to a legacy wrapper around the new logging architecture
 * to establish compatibility with plugins and applications developed for audio-manager
 * versions before 7.7.0.\n
 * For new development use CAmLogWrapper instead.
 *
 * For further information see http://www.genivi.org/.
 */

#ifndef DLTWRAPPER_H_
#define DLTWRAPPER_H_

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <map>

#include "CAmLogWrapper.h"

#ifdef WITH_DLT
# include <dlt.h>
#else
# include <stdint.h>
# include <sstream>

# define DLT_ID_SIZE 4
# define DLT_USER_BUF_MAX_SIZE 2048

/**
 * This structure is used for every context used in an application.
 */
typedef struct
{
    char contextID[4];      /**< context id */
    int32_t log_level_pos;  /**< offset in user-application context field */
    int32_t log_level_user; /** any message above this log level is not logged */
} DltContext;

/**
 * Definition of DLT trace status
 */
typedef enum
{
    DLT_TRACE_STATUS_DEFAULT = -1,          /**< Default trace status */
    DLT_TRACE_STATUS_OFF     = 0x00,        /**< Trace status: Off */
    DLT_TRACE_STATUS_ON      = 0x01         /**< Trace status: On */
} DltTraceStatusType;

/**
 * Definitions of DLT log level
 */
typedef enum
{
    DLT_LOG_DEFAULT = -1,   /**< Default log level */
    DLT_LOG_OFF     = 0x00, /**< Log level off */
    DLT_LOG_FATAL   = 0x01, /**< fatal system error */
    DLT_LOG_ERROR   = 0x02, /**< error with impact to correct functionality */
    DLT_LOG_WARN    = 0x03, /**< warning, correct behaviour could not be ensured */
    DLT_LOG_INFO    = 0x04, /**< informational */
    DLT_LOG_DEBUG   = 0x05, /**< debug  */
    DLT_LOG_VERBOSE = 0x06  /**< highest grade of information */
} DltLogLevelType;

# define DLT_DEFAULT_LOG_LEVEL DLT_LOG_INFO
# define DLT_DECLARE_CONTEXT(CONTEXT) \
    DltContext CONTEXT;

# define DLT_IMPORT_CONTEXT(CONTEXT) \
    extern DltContext CONTEXT;

#endif // WITH_DLT

namespace am
{

/**
 * Wraps around the dlt. This class is instantiated as a singleton and offers a default
 * context (maincontext) that is registered to log to.
 * Logging under the default context can simply be done with the logInfo/logError templates with up to 10 values at a time.
 * For logging with a different context, you can use the log template. First register a context with registerContext.
 */
class CAmDltWrapper
{
public:
    /*
     * The eunum gives the logtype
     */
    enum logDestination
    {
        DAEMON       = 0, //!< logging with the DLT daemon
        COMMAND_LINE = 1, //!< logging with commandline
        FILE_OUT     = 2  //!< logging into a file
    };

    /**
     * Instanciate the Dlt Wrapper
     * @param appid The AppID
     * @param description A description of the Application
     * @param debugEnabled if set to true, debug outputs will be generated, default = true
     * @param logDest the destination, the log should be written
     * @param Filename the filename with absolute path where the log shall be written. only needed if logDest==FILE_OUT
     * @param onlyError if set to true, only errors will be logged. just valid for commandline and file logs, default value = false
     */
    static CAmDltWrapper *instanctiateOnce(const char *appid, const char *description, const bool debugEnabled = true, const logDestination logDest = logDestination::DAEMON, const std::string Filename = "", bool onlyError = false);

    /**
     * get the Wrapper Instance
     */
    static CAmDltWrapper *instance();

    /**
     * register a context
     */
    void registerContext(DltContext &handle, const char *contextid, const char *description);
    void registerContext(DltContext &handle, const char *contextid, const char *description, DltLogLevelType level, DltTraceStatusType status);
    void unregisterContext(DltContext &handle);
    bool getEnabled();

    ~CAmDltWrapper();

    bool init(DltLogLevelType loglevel, DltContext *context = NULL);

    bool checkLogLevel(DltLogLevelType logLevel)
    {
        if (mpCurrentContext)
        {
            return mpCurrentContext->checkLogLevel(static_cast<am_LogLevel_e>(logLevel));
        }
        else
        {
            return getLogger()->importContext().checkLogLevel(static_cast<am_LogLevel_e>(logLevel));
        }
    }

    void deinit();
    void send();

    template<typename... TArgs>
    void append(TArgs... args)
    {
        if (mpCurrentContext)
        {
            mpCurrentContext->append(args...);
        }
    }


private:
    /**
     * private contructor
     */
    CAmDltWrapper(IAmLogger *pLogger, bool debugEnabled, bool onlyError = false); // is private because of singleton pattern

    IAmLogger                          *mpLogger;          //!< pointer to underlying logger instance
    IAmLogContext                      *mpCurrentContext;  //!< context for direct init(), append(...) and send() operations
    std::map<DltContext *, std::string> mMapContext;       //!< a Map for all registered context
    bool mDebugEnabled;                                    //!< debug Enabled or not
    bool                   mOnlyError;                     //!< Only if Log Level is above Error
    static CAmDltWrapper  *mpDLTWrapper;                   //!< pointer to the wrapper instance
};

/**
 * logs given values with a given context (register first!) and given loglevel
 * @param context
 * @param loglevel
 * @param ... args
 */
template<typename... TArgs>
void log(DltContext *const context, DltLogLevelType loglevel, TArgs... args)
{
    std::string contextID;
    if (context)
    {
        contextID = std::string(context->contextID, DLT_ID_SIZE);
    }

    // delegate to dedicated logging context
    getLogger()->importContext(contextID.c_str())
            .log(static_cast<am_LogLevel_e>(loglevel), args...);
}


}

#endif /* DLTWRAPPER_H_ */
