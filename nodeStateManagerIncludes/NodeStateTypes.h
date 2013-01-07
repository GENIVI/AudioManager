#ifndef NODESTATETYPES_H
#define NODESTATETYPES_H

/**********************************************************************************************************************
*
* Copyright (C) 2012 Continental Automotive Systems, Inc.
*
* Author: Jean-Pierre.Bogler@continental-corporation.com
*
* Type and constant definitions to communicate with the NSM.
*
* The file defines types and constants to be able to communicate with the NSM.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*
* Date       Author             Reason
* 2012.09.27 uidu5846  1.0.0.0  CSP_WZ#1194: Introduced 'NodeStateTypes.h' to avoid circle includes
*                                            and encapsulate type definitions.
* 2012.10.24 uidu5846  1.0.0.1  CSP_WZ#1322: Removed "ssw_types" redefinition from header.
*                                            Since the same native types are used, no interface change.
*
**********************************************************************************************************************/

/** \ingroup SSW_LCS */
/** \defgroup SSW_NSM_TEMPLATE Node State Manager
 *  \{
 */
/** \defgroup SSW_NSM_INTERFACE API document
 *  \{
 */

#ifdef __cplusplus
extern "C"
{
#endif

/**********************************************************************************************************************
*
*  CONSTANTS
*
**********************************************************************************************************************/

/* Definitions of D-Bus names and addresses */
#define NSM_BUS_TYPE              1                                               /**< Defines bus type according to GBusType */
#define NSM_BUS_NAME              "org.genivi.NodeStateManager"                   /**< The bus name of the NSM                */
#define NSM_LIFECYCLE_OBJECT      "/org/genivi/NodeStateManager/LifecycleControl" /**< Address of the lifecycle object        */
#define NSM_CONSUMER_OBJECT       "/org/genivi/NodeStateManager/Consumer"         /**< Address of the consumer  object        */

/* Defines for session handling */
#define NSM_DEFAULT_SESSION_OWNER "NodeStateManager"           /**< "Owner" of the default sessions                  */

/* Defines for internal settings like max. string lenghts */
#define NSM_MAX_SESSION_NAME_LENGTH  256                       /**< Max. number of chars a session name can have     */
#define NSM_MAX_SESSION_OWNER_LENGTH 256                       /**< Max. number of chars for name of session owner   */

/*
 * Defines for shutdown handling as bit masks. Used to register for multiple shutdown types and as parameter to
 * inform clients about the shutdown type via the LifecycleConsumer interface.
 */
#define NSM_SHUTDOWNTYPE_NOT      0x00000000U                  /**< Client not registered for any shutdown           */
#define NSM_SHUTDOWNTYPE_NORMAL   0x00000001U                  /**< Client registered for normal shutdown            */
#define NSM_SHUTDOWNTYPE_FAST     0x00000002U                  /**< Client registered for fast shutdown              */
#define NSM_SHUTDOWNTYPE_RUNUP    0x80000000U                  /**< The shutdown type "run up" can not be used for
                                                                    registration. Clients which are registered and
                                                                    have been shut down, will automatically be
                                                                    informed about the "run up", when the shut down
                                                                    is canceled.                                    */

/**********************************************************************************************************************
*
*  TYPE
*
**********************************************************************************************************************/

/**
 * The enumeration defines the different types of data that can be exchanged between the NodeStateManager (NSM)
 * and the NodeStateMachine (NSMC). Based on this value, the setter and getter functions of the NSM and NSMC will
 * interpret data behind the passed byte pointer.
 */
typedef enum _NsmDataType_e
{
  NsmDataType_AppMode,                 /**< An ApplicationMode should be set or get */
  NsmDataType_NodeState,               /**< A NodeState should be set or get        */
  NsmDataType_RestartReason,           /**< A RestartReason should be set or get    */
  NsmDataType_SessionState,            /**< A SessionState should be set or get     */
  NsmDataType_ShutdownReason,          /**< A ShutdownReason should be set or get   */
  NsmDataType_BootMode,                /**< A BootMode should be set or get         */
  NsmDataType_RunningReason            /**< A RunningReason should be set or get    */
} NsmDataType_e;


/**
 * The enumeration defines the different wake up reasons.
 */
typedef enum _NsmErrorStatus_e
{
  NsmErrorStatus_NotSet,               /**< Initial value when error type is not set             */
  NsmErrorStatus_Ok,                   /**< Value when no error occurred                         */
  NsmErrorStatus_Error,                /**< A general, non-specific error occurred               */
  NsmErrorStatus_Dbus,                 /**< Error in D-Bus communication                         */
  NsmErrorStatus_Internal,             /**< Internal error (memory alloc. failed, etc.)          */
  NsmErrorStatus_Parameter,            /**< A passed parameter was incorrect                     */
  NsmErrorStatus_WrongSession,         /**< The requested session is unknown.                    */
  NsmErrorStatus_ResponsePending,      /**< Command accepted, return value delivered asynch.     */
  NsmErrorStatus_Last                  /**< Last error value to identify valid errors            */
} NsmErrorStatus_e;


/**
 * Possible application modes of the node.
 */
typedef enum _NsmApplicationMode_e
{
  NsmApplicationMode_NotSet,           /**< Initial state                                        */
  NsmApplicationMode_Parking,          /**< Parking          mode                                */
  NsmApplicationMode_Factory,          /**< Factory          mode                                */
  NsmApplicationMode_Transport,        /**< Transport        mode                                */
  NsmApplicationMode_Normal,           /**< Normal           mode                                */
  NsmApplicationMode_Swl,              /**< Software loading mode                                */
  NsmApplicationMode_Last              /**< Last value to identify valid values                  */
}NsmApplicationMode_e;


/**
 * The enumeration defines the different restart reasons.
 */
typedef enum _NsmRestartReason_e
{
  NsmRestartReason_NotSet,             /**< Initial value when reset reason is not set           */
  NsmRestartReason_ApplicationFailure, /**< Reset was requested by System Health Mon.            */
  NsmRestartReason_Diagnosis,          /**< Reset was requested by diagnosis                     */
  NsmRestartReason_Swl,                /**< Reset was requested by the SWL application           */
  NsmRestartReason_User,               /**< Reset was requested by an user application           */
  NsmRestartReason_Last                /**< Last value to identify valid reset reasons           */
} NsmRestartReason_e;


/**
 * Session can be enabled seat depended.
 */
typedef enum _NsmSeat_e
{
  NsmSeat_NotSet,                      /**< Initial state                                        */
  NsmSeat_Driver,                      /**< Driver seat                                          */
  NsmSeat_CoDriver,                    /**< CoDriver seat                                        */
  NsmSeat_Rear1,                       /**< Rear 1                                               */
  NsmSeat_Rear2,                       /**< Rear 2                                               */
  NsmSeat_Rear3,                       /**< Rear 3                                               */
  NsmSeat_Last                         /**< Last valid state                                     */
}NsmSeat_e;


/**
 * The enumeration defines the different wake up reasons.
 */
typedef enum _NsmSessionState_e
{
  NsmSessionState_Unregistered,        /**< Initial state, equals "not set"                      */
  NsmSessionState_Inactive,            /**< Session is inactive                                  */
  NsmSessionState_Active               /**< Session is active                                    */
} NsmSessionState_e;

/**
 * The enumeration defines the different shutdown reasons.
 */
typedef enum _NsmShutdownReason_e
{
  NsmShutdownReason_NotSet,            /**< Initial value when ShutdownReason not set            */
  NsmShutdownReason_Normal,            /**< A normal shutdown has been performed                 */
  NsmShutdownReason_SupplyBad,         /**< Shutdown because of bad supply                       */
  NsmShutdownReason_SupplyPoor,        /**< Shutdown because of poor supply                      */
  NsmShutdownReason_ThermalBad,        /**< Shutdown because of bad thermal state                */
  NsmShutdownReason_ThermalPoor,       /**< Shutdown because of poor thermal state               */
  NsmShutdownReason_SwlNotActive,      /**< Shutdown after software loading                      */
  NsmShutdownReason_Last               /**< Last value. Identify valid ShutdownReasons           */
} NsmShutdownReason_e;

/**
 * The enumeration defines the different start or wake up reasons.
 */
typedef enum _NsmRunningReason_e
{
  NsmRunningReason_NotSet,                       /**< Initial value when reason is not set.                          */
  NsmRunningReason_WakeupCan,                    /**< Wake up because of CAN activity                                */
  NsmRunningReason_WakeupMediaEject,             /**< Wake up because of 'Eject' button                              */
  NsmRunningReason_WakeupMediaInsertion,         /**< Wake up because of media insertion                             */
  NsmRunningReason_WakeupHevac,                  /**< Wake up because of user uses the HEVAC unit in the car.
                                                      Even if the HEVAC actually causes activity on the CAN bus a
                                                      different wakeup reason is required as it could result in a
                                                      different level of functionality being started                 */
  NsmRunningReason_WakeupPhone,                  /**< Wake up because of a phone call being received.
                                                      Even if this is passed as a CAN event a different wakeup reason
                                                      is required as it could result in a different level of
                                                      functionality being started                                    */
  NsmRunningReason_WakeupPowerOnButton,          /**< Startup because user presses the "Power ON" button in the car.
                                                      Even if this is passed as a CAN event a different wakeup reason
                                                      is required as it could result in a different level of
                                                      functionality being started                                    */
  NsmRunningReason_StartupFstp,                  /**< System was started due to a first switch to power              */
  NsmRunningReason_StartupSwitchToPower,         /**< System was switched to power                                   */
  NsmRunningReason_RestartSwRequest,             /**< System was restarted due to an internal SW Request
                                                      (i.e. SWL or Diagnosis)                                        */
  NsmRunningReason_RestartInternalHealth,        /**< System was restarted due to an internal health problem         */
  NsmRunningReason_RestartExternalHealth,        /**< System was restarted due to an external health problem
                                                      (i.e. external wdog believed node was in failure)              */
  NsmRunningReason_RestartUnexpected,            /**< System was restarted due to an unexpected kernel restart.
                                                      This will be the default catch when no other reason is known   */
  NsmRunningReason_RestartUser,                  /**< Target was reset due to user action (i.e user 3 finger press)  */
  NsmRunningReason_PlatformEnd = 0x7F,           /**< Last value (127) to identify where the platform defines end
                                                      (product will start from here on)                              */
  NsmRunningReason_ProductOffset = NsmRunningReason_PlatformEnd  + 1 /**< product will start from here with index 0  */
} NsmRunningReason_e;


/**
 * The enumeration defines the different node states
 */
typedef enum _NsmNodeState_e
{
  NsmNodeState_NotSet,                 /**< Initial state when node state is not set             */
  NsmNodeState_StartUp,                /**< Basic system is starting up                          */
  NsmNodeState_BaseRunning,            /**< Basic system components have been started            */
  NsmNodeState_LucRunning,             /**< All 'Last user context' components have been started */
  NsmNodeState_FullyRunning,           /**< All 'foreground' components have been started        */
  NsmNodeState_FullyOperational,       /**< All components have been started                     */
  NsmNodeState_ShuttingDown,           /**< The system is shutting down                          */
  NsmNodeState_ShutdownDelay,          /**< Shutdown request active. System will shutdown soon   */
  NsmNodeState_FastShutdown,           /**< Fast shutdown active                                 */
  NsmNodeState_DegradedPower,          /**< Node is in degraded power state                      */
  NsmNodeState_Shutdown,               /**< Node is completely shut down                         */
  NsmNodeState_Last                    /**< Last valid entry to identify valid node states       */
} NsmNodeState_e;


/** The type defines the structure for a session.                                                */
typedef struct _NsmSession_s
{
  char               sName[NSM_MAX_SESSION_NAME_LENGTH];   /**< Name  of the session             */
  char               sOwner[NSM_MAX_SESSION_OWNER_LENGTH]; /**< Owner of the session             */
  NsmSeat_e          enSeat;                               /**< Seat  of the session             */
  NsmSessionState_e  enState;                              /**< State of the session             */
} NsmSession_s, *pNsmSession_s;


#ifdef __cplusplus
}
#endif
/** \} */ /* End of SSW_NSM_INTERFACE */
/** \} */ /* End of SSW_NSM_TEMPLATE  */
#endif /* NODESTATETYPES_H */
