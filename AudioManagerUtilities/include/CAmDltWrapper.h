/**
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2012, BMW AG
 *
 * \author Christian Linke, christian.linke@bmw.de BMW 2011,2012
 * \author Jens Lorenz, jlorenz@de.adit-jv.com ADIT 2014
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \file CAmDltWrapper.h
 * For further information see http://www.genivi.org/.
 */

#ifndef DLTWRAPPER_H_
#define DLTWRAPPER_H_

#include <string>
#include <pthread.h>
#include <sstream>
#include <audiomanagerconfig.h>
#include "audiomanagertypes.h"

#ifdef WITH_DLT
#include <dlt/dlt.h>
namespace am
{
#else
#include <stdint.h>
#include <sstream>

#define DLT_USER_BUF_MAX_SIZE 2048

/**
 * This structure is used for every context used in an application.
 */
typedef struct
{
    char contextID[4]; /**< context id */
    int32_t log_level_pos; /**< offset in user-application context field */
    int32_t log_level_user; /** any message above this log level is not logged */
} DltContext;

/**
 * Definitions of DLT log level
 */
typedef enum
{
    DLT_LOG_DEFAULT = -1, /**< Default log level */
    DLT_LOG_OFF = 0x00, /**< Log level off */
    DLT_LOG_FATAL = 0x01, /**< fatal system error */
    DLT_LOG_ERROR = 0x02, /**< error with impact to correct functionality */
    DLT_LOG_WARN = 0x03, /**< warning, correct behaviour could not be ensured */
    DLT_LOG_INFO = 0x04, /**< informational */
    DLT_LOG_DEBUG = 0x05, /**< debug  */
    DLT_LOG_VERBOSE = 0x06 /**< highest grade of information */
} DltLogLevelType;

/**
 * Definition of DLT trace status
 */
typedef enum
{
    DLT_TRACE_STATUS_DEFAULT =   -1,    /**< Default trace status */
    DLT_TRACE_STATUS_OFF     = 0x00,    /**< Trace status: Off */
    DLT_TRACE_STATUS_ON      = 0x01     /**< Trace status: On */
} DltTraceStatusType;


/**
 * This structure is used for context data used in an application.
 */
typedef struct
{
    DltContext *handle; /**< pointer to DltContext */
    std::stringstream buffer; /**< buffer for building log message*/
    int32_t log_level; /**< log level */
    int32_t trace_status; /**< trace status */
    int32_t args_num; /**< number of arguments for extended header*/
    uint8_t mcnt; /**< message counter */
    char* context_description; /**< description of context */
} DltContextData;

#define DLT_DEFAULT_LOG_LEVEL DLT_LOG_INFO


#define DLT_DECLARE_CONTEXT(CONTEXT) \
DltContext CONTEXT;

#define DLT_IMPORT_CONTEXT(CONTEXT) \
extern DltContext CONTEXT;
namespace am
{
#endif // WITH_DLT

/**
 * Wraps around the dlt. This class is instantiated as a singleton and offers a default
 * context (maincontext) that is registered to log to.
 * Logging under the default context can simply be done with the logInfo/logError templates with up to 10 values at a time.
 * For logging with a different context, you can use the log template. First register a context with registerContext.
 */
class CAmDltWrapper
{
public:
    static CAmDltWrapper* instance(const bool enableNoDLTDebug = false);
    void registerApp(const char *appid, const char * description);
    void registerContext(DltContext& handle, const char *contextid, const char * description);
    void registerContext(DltContext& handle, const char *contextid, const char * description,
            const DltLogLevelType level, const DltTraceStatusType status);
    void unregisterContext(DltContext& handle);

    bool init(DltLogLevelType loglevel, DltContext* context = NULL);
    void deinit();
    void send();
    void append(const int8_t value);
    void append(const uint8_t value);
    void append(const int16_t value);
    void append(const uint16_t value);
    void append(const int32_t value);
    void append(const uint32_t value);
    void append(const uint64_t value);
    void append(const int64_t value);
    void append(const std::string& value);
    void append(const bool value);
    void append(const std::vector<uint8_t> & data);

    // specialization for const char*
    template<typename T = const char*> void append(const char* value)
    {
    #ifdef WITH_DLT
    	if(mEnableNoDLTDebug)
    	{
    		dlt_user_log_write_string(&mDltContextData, value);
    	}
    #else
        mDltContextData.buffer << value;
    #endif
    }

    // specialization for const am_Error_e
    template<typename T = const am_Error_e> void append(const am_Error_e value)
    {
        const char* str_error[E_MAX] = {
            "E_OK",
            "E_UNKNOWN",
            "E_OUT_OF_RANGE",
            "E_NOT_USED",
            "E_DATABASE_ERROR",
            "E_ALREADY_EXISTS",
            "E_NO_CHANGE",
            "E_NOT_POSSIBLE",
            "E_NON_EXISTENT",
            "E_ABORTED",
            "E_WRONG_FORMAT"
        };
        if(mEnableNoDLTDebug)
        	append(str_error[value]);
    }

    // Template to print unknown pointer types with their address
    template<typename T> void append(T* value)
    {
        std::ostringstream ss;
        ss << "0x" << std::hex << (uint64_t)value;
        append(ss.str().c_str());
    }

    // Template to print unknown types
    template<typename T> void append(T value)
    {
        std::ostringstream ss;
        ss << std::dec << value;
        append(ss.str().c_str());
    }

    // Template parameter pack to generate recursive code
    void append(void) {}
    template<typename T, typename... TArgs> void append(T value, TArgs... args)
    {
        this->append(value);
        this->append(args...);
    }

#ifndef WITH_DLT
    void enableNoDLTDebug(const bool enableNoDLTDebug = true);
#endif
    ~CAmDltWrapper();
private:
    CAmDltWrapper(const bool enableNoDLTDebug); //is private because of singleton pattern
#ifndef WITH_DLT
    template<class T> void appendNoDLT(T value);
#endif
    DltContext mDltContext; //!< the default context
    DltContextData mDltContextData; //!< contextdata
    bool mEnableNoDLTDebug;
    static CAmDltWrapper* mpDLTWrapper; //!< pointer to the wrapper instance
    static pthread_mutex_t mMutex;

};

/**
 * returns the instance of the CAmDltWrapper
 * @return
 */
inline CAmDltWrapper* getWrapper()
{
    return (CAmDltWrapper::instance());
}

/**
 * logs given values with a given context (register first!) and given loglevel
 * @param context
 * @param loglevel
 * @param value
 * @param ...
 */
template<typename T, typename... TArgs>
void log(DltContext* const context, DltLogLevelType loglevel, T value, TArgs... args)
{
    CAmDltWrapper* inst(getWrapper());

    if (!inst->init(loglevel, context))
        return;
    inst->append(value);
    inst->append(args...);
    inst->send();
}

/**
 * logs given values with debuglevel with the default context
 * @param value
 * @param ...
 */
template<typename T, typename... TArgs>
void logDebug(T value, TArgs... args)
{
    log(NULL, DLT_LOG_DEBUG, value, args...);
}

/**
 * logs given values with infolevel with the default context
 * @param value
 * @param ...
 */
template<typename T, typename... TArgs>
void logInfo(T value, TArgs... args)
{
    log(NULL, DLT_LOG_INFO, value, args...);
}

/**
 * logs given values with errorlevel with the default context
 * @param value
 * @param ...
 */
template<typename T, typename... TArgs>
void logError(T value, TArgs... args)
{
    log(NULL, DLT_LOG_ERROR,value,args...);
}

}

#endif /* DLTWRAPPER_H_ */
