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
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <audiomanagerconfig.h>
#include "audiomanagertypes.h"

#ifdef WITH_DLT
	#include <dlt/dlt.h>
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

	#define DLT_DEFAULT_LOG_LEVEL DLT_LOG_INFO
	#define DLT_DECLARE_CONTEXT(CONTEXT)                               \
	DltContext CONTEXT;

	#define DLT_IMPORT_CONTEXT(CONTEXT)                                \
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
	} NoDltContextData;
	
	/*
	 * The eunum gives the logtype
	 */
	enum logDestination
	{
		DAEMON=0, //!< logging with the DLT daemon
		COMMAND_LINE=1, //!< logging with commandline 
		FILE_OUT =2 //!< logging into a file
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
	static CAmDltWrapper* instanctiateOnce(const char *appid, const char * description, const bool debugEnabled = true, const logDestination logDest = logDestination::DAEMON, const std::string Filename="",bool onlyError=false);
    
    /**
     * get the Wrapper Instance
     */
    static CAmDltWrapper* instance();
	
	/**
	 * register a context
	 */
    void registerContext(DltContext& handle, const char *contextid, const char * description);
    void registerContext(DltContext& handle, const char *contextid, const char * description, const DltLogLevelType level, const DltTraceStatusType status);
    void unregisterContext(DltContext& handle);
    bool getEnabled();
    ~CAmDltWrapper();
    
     
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
        
    template<class T> void appendNoDLT(T value)
	{
		mNoDltContextData.buffer << value <<" ";
	}

    // specialization for const char*
    template<typename T = const char*> void append(const char* value)
    {
		#ifdef WITH_DLT
			if (mlogDestination == logDestination::DAEMON)
			{
				dlt_user_log_write_string(&mDltContextData, value);
			}
			else
			{
				mNoDltContextData.buffer << std::string(value);
			}
		#else
			mNoDltContextData.buffer << std::string(value);
    	#endif //WITH_DLT

	}

    // specialization for const am_Error_e
    template<typename T = const am_Error_e> void append(const am_Error_e value)
    {
        const std::vector<const char*> str_error = {
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
        		"E_WRONG_FORMAT",
        		"E_COMMUNICATION",
        		"E_MAX"
        };
        if ((int)value >=str_error.size())
        {
        	append("value for am_Error_e out of bounds!");
        	append(value);
        	return;
        }
       	append(str_error[value]);
    }

    // specialization for const am_Error_e
    template<typename T = const am_SourceState_e> void append(const am_SourceState_e value)
    {
    	const std::vector<const char*> str_sourceState = {
        		"SS_UNKNNOWN",
        		"SS_ON",
        		"SS_OFF",
        		"SS_PAUSED",
        		"SS_MAX"
        };
        if ((int)value >=(str_sourceState.size()))
        {
        	append("value for am_SourceState_e out of bounds!");
        	append(value);
        	return;
        }
       	append(str_sourceState[value]);
    }


    template<typename T = const am_MuteState_e> void append(const am_MuteState_e value)
    {
    	const std::vector<const char*> str_MuteState= {
            	"MS_UNKNOWN" ,
            	"MS_MUTED" ,
            	"MS_UNMUTED" ,
            	"MS_MAX"
        };
        if ((int)value >=str_MuteState.size())
        {
        	append("value for am_MuteState_e out of bounds!");
        	append(value);
        	return;
        }
       	append(str_MuteState[value]);
    }

    template<typename T = const am_DomainState_e> void append(const am_DomainState_e value)
    {
    	const std::vector<const char*>  str_DomainState = {
        		"DS_UNKNOWN",
        		"DS_CONTROLLED",
        		"DS_INDEPENDENT_STARTUP",
        		"DS_INDEPENDENT_RUNDOWN",
        		"DS_MAX"
        };
        if ((int)value >= str_DomainState.size())
        {
        	append("value for am_DomainState_e out of bounds!");
        	append(value);
        	return;
        }
       	append(str_DomainState[value]);
    }

    template<typename T = const am_ConnectionState_e> void append(const am_ConnectionState_e value)
    {
    	const std::vector<const char*>  str_ConnectionState = {
        		"CS_UNKNOWN",
        		"CS_CONNECTING",
        		"CS_CONNECTED",
        		"CS_DISCONNECTING",
        		"CS_DISCONNECTED",
        		"CS_SUSPENDED",
        		"CS_MAX"
        };
        if ((int)value >=str_ConnectionState.size())
        {
        	append("value for am_ConnectionState_e out of bounds!");
        	append(value);
        	return;
        }
       	append(str_ConnectionState[value]);
    }

    template<typename T = const am_Availability_e> void append(const am_Availability_e value)
    {
    	const std::vector<const char*> str_Availability = {
        		"A_UNKNOWN",
        		"A_AVAILABLE",
				"A_UNAVAILABLE",
        		"A_MAX"
        };
        if ((int)value >= str_Availability.size())
        {
        	append("value for am_Availability_e out of bounds!");
        	append(value);
        	return;
        }
       	append(str_Availability[value]);
    }

    template<typename T = const am_InterruptState_e> void append(const am_InterruptState_e value)
    {
    	const std::vector<const char*> str_Interrupt = {
        		"IS_UNKNOWN",
        		"IS_OFF",
        		"IS_INTERRUPTED",
        		"IS_MAX"
        };
        if ((int)value >=str_Interrupt.size())
        {
        	append("value for am_InterruptState_e out of bounds!");
        	append(value);
        	return;
        }
       	append(str_Interrupt[value]);
    }

    template<typename T = const am_Handle_e> void append(const am_Handle_e value)
    {
    	const std::vector<const char*> str_Handle = {
    			"H_UNKNOWN",
				"H_CONNECT",
				"H_DISCONNECT",
				"H_SETSOURCESTATE",
				"H_SETSINKVOLUME",
				"H_SETSOURCEVOLUME",
				"H_SETSINKSOUNDPROPERTY",
				"H_SETSOURCESOUNDPROPERTY",
				"H_SETSINKSOUNDPROPERTIES",
				"H_SETSOURCESOUNDPROPERTIES",
				"H_CROSSFADE",
				"H_SETVOLUMES",
				"H_SETSINKNOTIFICATION",
				"H_SETSOURCENOTIFICATION",
				"H_MAX"
        };
        if ((int)value >=str_Handle.size())
        {
        	append("value for am_Handle_e out of bounds!");
        	append(value);
        	return;
        }
       	append(str_Handle[value]);
    }

    template<typename T = const am_NotificationStatus_e> void append(const am_NotificationStatus_e value)
    {
    	const std::vector<const char*> str_NotificationStatus = {
    			"NS_UNKNOWN",
				"NS_OFF",
				"NS_PERIODIC",
				"NS_MINIMUM",
				"NS_MAXIMUM",
				"NS_CHANGE",
				"NS_MAX"
        };
        if ((int)value >=str_NotificationStatus.size())
        {
        	append("value for am_NotificationStatus_e out of bounds!");
        	append(value);
        	return;
        }
       	append(str_NotificationStatus[value]);
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

private:
	/**
	 * private contructor
	 */
    CAmDltWrapper(const char *appid, const char * description, const bool debugEnabled = true, const logDestination logDest = logDestination::DAEMON, const std::string Filename="",bool onlyError=false); //is private because of singleton pattern
    bool initNoDlt(DltLogLevelType loglevel, DltContext* context);
    std::string now();
    DltContext mDltContext; //!< the default context
    DltContextData mDltContextData; //!< contextdata
    NoDltContextData mNoDltContextData; //!<contextdata for std out logging
    std::map<DltContext*,std::string> mMapContext; //!< a Map for all registered context
    bool mDebugEnabled;	//!< debug Enabled or not
    logDestination mlogDestination; //!< The log destination
    std::ofstream mFilename; //!< Filename for logging
    bool mOnlyError; //!< Only if Log Level is above Error
    bool mLogOn; //!< Used to keep track if currently logging is on
    static CAmDltWrapper* mpDLTWrapper; //!< pointer to the wrapper instance
    static pthread_mutex_t mMutex;

};

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
    CAmDltWrapper* inst(CAmDltWrapper::instance());
	if (!inst->getEnabled())
    {
		return;
	}
    if (!inst->init(loglevel, context))
    {
		return;
	}
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

/**
 * logs given values with warninglevel with the default context
 * @param value
 * @param ...
 */
template<typename T, typename... TArgs>
void logWarning(T value, TArgs... args)
{
    log(NULL, DLT_LOG_WARN,value,args...);
}

}

#endif /* DLTWRAPPER_H_ */
