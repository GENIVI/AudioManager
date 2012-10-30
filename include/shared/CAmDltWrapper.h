/**
 *  Copyright (C) 2012, BMW AG
 *
 *  \author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
 *
 *  \copyright
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction,
 *  including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 *  subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 *  THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *  \file CAmDltWrapper.h
 *  For further information see http://www.genivi.org/.
 */

#ifndef DLTWRAPPER_H_
#define DLTWRAPPER_H_

#include "config.h"
#include <string>
#include <pthread.h>

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
    void unregisterContext(DltContext& handle);

    void init(DltLogLevelType loglevel, DltContext* context = NULL);
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
    void append(const char*& value);
    void append(const std::string& value);
    void append(const bool value);
#ifndef WITH_DLT
    void enableNoDLTDebug(const bool enableNoDLTDebug = true);
#endif
    ~CAmDltWrapper();
private:
    CAmDltWrapper(const bool enableNoDLTDebug); //is private because of singleton pattern
#ifndef WITH_DLT
    template<class T> void appendNoDLT(T value);
    bool mEnableNoDLTDebug;
#endif
    DltContext mDltContext; //!< the default context
    DltContextData mDltContextData; //!< contextdata
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
 * logs a given value with infolevel with the default context
 * @param value
 */
template<typename T> void logInfo(T value)
{
    CAmDltWrapper* inst(getWrapper());
    inst->init(DLT_LOG_INFO);
    inst->append(value);
    inst->send();
}

/**
 * logs a given value with infolevel with the default context
 * @param value
 * @param value1mDltContext
 */
template<typename T, typename T1> void logInfo(T value, T1 value1)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(DLT_LOG_INFO);
    inst->append(value);
    inst->append(value1);
    inst->send();

}

/**
 * logs a given value with infolevel with the default context
 * @param value
 * @param value1
 * @param value2
 */
template<typename T, typename T1, typename T2> void logInfo(T value, T1 value1, T2 value2)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(DLT_LOG_INFO);
    inst->append(value);
    inst->append(value1);
    inst->append(value2);
    inst->send();

}

/**
 * logs a given value with infolevel with the default context
 * @param value
 * @param value1
 * @param value2
 * @param value3
 */
template<typename T, typename T1, typename T2, typename T3> void logInfo(T value, T1 value1, T2 value2, T3 value3)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(DLT_LOG_INFO);
    inst->append(value);
    inst->append(value1);
    inst->append(value2);
    inst->append(value3);
    inst->send();

}

/**
 * logs a given value with infolevel with the default context
 * @param value
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 */
template<typename T, typename T1, typename T2, typename T3, typename T4> void logInfo(T value, T1 value1, T2 value2, T3 value3, T4 value4)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(DLT_LOG_INFO);
    inst->append(value);
    inst->append(value1);
    inst->append(value2);
    inst->append(value3);
    inst->append(value4);
    inst->send();

}

/**
 * logs a given value with infolevel with the default context
 * @param value
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 * @param value5
 */
template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5> void logInfo(T value, T1 value1, T2 value2, T3 value3, T4 value4, T5 value5)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(DLT_LOG_INFO);
    inst->append(value);
    inst->append(value1);
    inst->append(value2);
    inst->append(value3);
    inst->append(value4);
    inst->append(value5);
    inst->send();

}

/**
 * logs a given value with infolevel with the default context
 * @param value
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 * @param value5
 * @param value6
 */
template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> void logInfo(T value, T1 value1, T2 value2, T3 value3, T4 value4, T5 value5, T6 value6)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(DLT_LOG_INFO);
    inst->append(value);
    inst->append(value1);
    inst->append(value2);
    inst->append(value3);
    inst->append(value4);
    inst->append(value5);
    inst->append(value6);
    inst->send();

}

/**
 * logs a given value with infolevel with the default context
 * @param value
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 * @param value5
 * @param value6
 * @param value7
 */
template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> void logInfo(T value, T1 value1, T2 value2, T3 value3, T4 value4, T5 value5, T6 value6, T7 value7)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(DLT_LOG_INFO);
    inst->append(value);
    inst->append(value1);
    inst->append(value2);
    inst->append(value3);
    inst->append(value4);
    inst->append(value5);
    inst->append(value6);
    inst->append(value7);
    inst->send();

}

/**
 * logs a given value with infolevel with the default context
 * @param value
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 * @param value5
 * @param value6
 * @param value7
 * @param value8
 */
template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8> void logInfo(T value, T1 value1, T2 value2, T3 value3, T4 value4, T5 value5, T6 value6, T7 value7, T8 value8)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(DLT_LOG_INFO);
    inst->append(value);
    inst->append(value1);
    inst->append(value2);
    inst->append(value3);
    inst->append(value4);
    inst->append(value5);
    inst->append(value6);
    inst->append(value7);
    inst->append(value8);
    inst->send();

}

/**
 * logs a given value with infolevel with the default context
 * @param value
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 * @param value5
 * @param value6
 * @param value7
 * @param value8
 * @param value9
 */
template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9> void logInfo(T value, T1 value1, T2 value2, T3 value3, T4 value4, T5 value5, T6 value6, T7 value7, T8 value8, T9 value9)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(DLT_LOG_INFO);
    inst->append(value);
    inst->append(value1);
    inst->append(value2);
    inst->append(value3);
    inst->append(value4);
    inst->append(value5);
    inst->append(value6);
    inst->append(value7);
    inst->append(value8);
    inst->append(value9);
    inst->send();

}

/**
 * logs a given value with infolevel with the default context
 * @param value
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 * @param value5
 * @param value6
 * @param value7
 * @param value8
 * @param value9
 * @param value10
 */
template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10> void logInfo(T value, T1 value1, T2 value2, T3 value3, T4 value4, T5 value5, T6 value6, T7 value7, T8 value8, T9 value9, T10 value10)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(DLT_LOG_INFO);
    inst->append(value);
    inst->append(value1);
    inst->append(value2);
    inst->append(value3);
    inst->append(value4);
    inst->append(value5);
    inst->append(value6);
    inst->append(value7);
    inst->append(value8);
    inst->append(value9);
    inst->append(value10);
    inst->send();

}

/**
 * logs a given value with errorlevel with the default context
 * @param value
 */
template<typename T> void logError(T value)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(DLT_LOG_ERROR);
    inst->append(value);
    inst->send();

}

/**
 * logs a given value with errorlevel with the default context
 * @param value
 * @param value1
 */
template<typename T, typename T1> void logError(T value, T1 value1)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(DLT_LOG_ERROR);
    inst->append(value);
    inst->append(value1);
    inst->send();

}

/**
 * logs a given value with errorlevel with the default context
 * @param value
 * @param value1
 * @param value2
 */
template<typename T, typename T1, typename T2> void logError(T value, T1 value1, T2 value2)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(DLT_LOG_ERROR);
    inst->append(value);
    inst->append(value1);
    inst->append(value2);
    inst->send();

}

/**
 * logs a given value with errorlevel with the default context
 * @param value
 * @param value1
 * @param value2
 * @param value3
 */
template<typename T, typename T1, typename T2, typename T3> void logError(T value, T1 value1, T2 value2, T3 value3)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(DLT_LOG_ERROR);
    inst->append(value);
    inst->append(value1);
    inst->append(value2);
    inst->append(value3);
    inst->send();

}

/**
 * logs a given value with errorlevel with the default context
 * @param value
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 */
template<typename T, typename T1, typename T2, typename T3, typename T4> void logError(T value, T1 value1, T2 value2, T3 value3, T4 value4)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(DLT_LOG_ERROR);
    inst->append(value);
    inst->append(value1);
    inst->append(value2);
    inst->append(value3);
    inst->append(value4);
    inst->send();

}

/**
 * logs a given value with errorlevel with the default context
 * @param value
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 * @param value5
 */
template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5> void logError(T value, T1 value1, T2 value2, T3 value3, T4 value4, T5 value5)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(DLT_LOG_ERROR);
    inst->append(value);
    inst->append(value1);
    inst->append(value2);
    inst->append(value3);
    inst->append(value4);
    inst->append(value5);
    inst->send();

}

/**
 * logs a given value with errorlevel with the default context
 * @param value
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 * @param value5
 * @param value6
 */
template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> void logError(T value, T1 value1, T2 value2, T3 value3, T4 value4, T5 value5, T6 value6)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(DLT_LOG_ERROR);
    inst->append(value);
    inst->append(value1);
    inst->append(value2);
    inst->append(value3);
    inst->append(value4);
    inst->append(value5);
    inst->append(value6);
    inst->send();

}

/**
 * logs a given value with errorlevel with the default context
 * @param value
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 * @param value5
 * @param value6
 * @param value7
 */
template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> void logError(T value, T1 value1, T2 value2, T3 value3, T4 value4, T5 value5, T6 value6, T7 value7)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(DLT_LOG_ERROR);
    inst->append(value);
    inst->append(value1);
    inst->append(value2);
    inst->append(value3);
    inst->append(value4);
    inst->append(value5);
    inst->append(value6);
    inst->append(value7);
    inst->send();

}

/**
 * logs a given value with errorlevel with the default context
 * @param value
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 * @param value5
 * @param value6
 * @param value7
 * @param value8
 */
template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8> void logError(T value, T1 value1, T2 value2, T3 value3, T4 value4, T5 value5, T6 value6, T7 value7, T8 value8)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(DLT_LOG_ERROR);
    inst->append(value);
    inst->append(value1);
    inst->append(value2);
    inst->append(value3);
    inst->append(value4);
    inst->append(value5);
    inst->append(value6);
    inst->append(value7);
    inst->append(value8);
    inst->send();

}

/**
 * logs a given value with errorlevel with the default context
 * @param value
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 * @param value5
 * @param value6
 * @param value7
 * @param value8
 * @param value9
 */
template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9> void logError(T value, T1 value1, T2 value2, T3 value3, T4 value4, T5 value5, T6 value6, T7 value7, T8 value8, T9 value9)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(DLT_LOG_ERROR);
    inst->append(value);
    inst->append(value1);
    inst->append(value2);
    inst->append(value3);
    inst->append(value4);
    inst->append(value5);
    inst->append(value6);
    inst->append(value7);
    inst->append(value8);
    inst->append(value9);
    inst->send();

}

/**
 * logs a given value with errorlevel with the default context
 * @param value
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 * @param value5
 * @param value6
 * @param value7
 * @param value8
 * @param value9
 * @param value10
 */
template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10> void logError(T value, T1 value1, T2 value2, T3 value3, T4 value4, T5 value5, T6 value6, T7 value7, T8 value8, T9 value9, T10 value10)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(DLT_LOG_ERROR);
    inst->append(value);
    inst->append(value1);
    inst->append(value2);
    inst->append(value3);
    inst->append(value4);
    inst->append(value5);
    inst->append(value6);
    inst->append(value7);
    inst->append(value8);
    inst->append(value9);
    inst->append(value10);
    inst->send();

}

/**
 * logs a given value with a given context (register first!) and given loglevel
 * @param context
 * @param loglevel
 * @param value
 */
template<typename T> void log(DltContext* const context, DltLogLevelType loglevel, T value)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(loglevel, context);
    inst->append(value);
    inst->send();

}

/**
 * logs a given value with a given context (register first!) and given loglevel
 * @param context
 * @param loglevel
 * @param value
 * @param value1
 */
template<typename T, typename T1> void log(DltContext* const context, DltLogLevelType loglevel, T value, T1 value1)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(loglevel, context);
    inst->append(value);
    inst->append(value1);
    inst->send();

}

/**
 * logs a given value with a given context (register first!) and given loglevel
 * @param context
 * @param loglevel
 * @param value
 * @param value1
 * @param value2
 */
template<typename T, typename T1, typename T2> void log(DltContext* const context, DltLogLevelType loglevel, T value, T1 value1, T2 value2)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(loglevel, context);
    inst->append(value);
    inst->append(value1);
    inst->append(value2);
    inst->send();

}

/**
 * logs a given value with a given context (register first!) and given loglevel
 * @param context
 * @param loglevel
 * @param value
 * @param value1
 * @param value2
 * @param value3
 */
template<typename T, typename T1, typename T2, typename T3> void log(DltContext* const context, DltLogLevelType loglevel, T value, T1 value1, T2 value2, T3 value3)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(loglevel, context);
    inst->append(value);
    inst->append(value1);
    inst->append(value2);
    inst->append(value3);
    inst->send();

}

/**
 * logs a given value with a given context (register first!) and given loglevel
 * @param context
 * @param loglevel
 * @param value
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 */
template<typename T, typename T1, typename T2, typename T3, typename T4> void log(DltContext* const context, DltLogLevelType loglevel, T value, T1 value1, T2 value2, T3 value3, T4 value4)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(loglevel, context);
    inst->append(value);
    inst->append(value1);
    inst->append(value2);
    inst->append(value3);
    inst->append(value4);
    inst->send();

}

/**
 * logs a given value with a given context (register first!) and given loglevel
 * @param context
 * @param loglevel
 * @param value
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 * @param value5
 */
template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5> void log(DltContext* const context, DltLogLevelType loglevel, T value, T1 value1, T2 value2, T3 value3, T4 value4, T5 value5)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(loglevel, context);
    inst->append(value);
    inst->append(value1);
    inst->append(value2);
    inst->append(value3);
    inst->append(value4);
    inst->append(value5);
    inst->send();

}

/**
 * logs a given value with a given context (register first!) and given loglevel
 * @param context
 * @param loglevel
 * @param value
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 * @param value5
 * @param value6
 */
template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> void log(DltContext* const context, DltLogLevelType loglevel, T value, T1 value1, T2 value2, T3 value3, T4 value4, T5 value5, T6 value6)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(loglevel, context);
    inst->append(value);
    inst->append(value1);
    inst->append(value2);
    inst->append(value3);
    inst->append(value4);
    inst->append(value5);
    inst->append(value6);
    inst->send();

}

/**
 * logs a given value with a given context (register first!) and given loglevel
 * @param context
 * @param loglevel
 * @param value
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 * @param value5
 * @param value6
 * @param value7
 */
template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7> void log(DltContext* const context, DltLogLevelType loglevel, T value, T1 value1, T2 value2, T3 value3, T4 value4, T5 value5, T6 value6, T7 value7)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(loglevel, context);
    inst->append(value);
    inst->append(value1);
    inst->append(value2);
    inst->append(value3);
    inst->append(value4);
    inst->append(value5);
    inst->append(value6);
    inst->append(value7);
    inst->send();

}

/**
 * logs a given value with a given context (register first!) and given loglevel
 * @param context
 * @param loglevel
 * @param value
 * @param value1
 * @param value2
 * @param value3
 * @param value4
 * @param value5
 * @param value6
 * @param value7
 * @param value8
 */
template<typename T, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8> void log(DltContext* const context, DltLogLevelType loglevel, T value, T1 value1, T2 value2, T3 value3, T4 value4, T5 value5, T6 value6, T7 value7, T8 value8)
{
    CAmDltWrapper* inst(getWrapper());

    inst->init(loglevel, context);
    inst->append(value);
    inst->append(value1);
    inst->append(value2);
    inst->append(value3);
    inst->append(value4);
    inst->append(value5);
    inst->append(value6);
    inst->append(value7);
    inst->append(value8);
    inst->send();

}
}

#endif /* DLTWRAPPER_H_ */
