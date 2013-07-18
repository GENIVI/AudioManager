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
 * \author Christian Linke, christian.linke@bmw.de BMW 2012
 *
 * \file CAmNodeStateCommunicator.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include "CAmNodeStateCommunicator.h"
#include <assert.h>
#include <string>
#include <fstream>
#include <stdexcept>
#include "CAmControlSender.h"
#include "shared/CAmDltWrapper.h"
#include "config.h"
#include <sstream>

namespace am
{

static DBusObjectPathVTable gObjectPathVTable;

CAmNodeStateCommunicator::CAmNodeStateCommunicator(CAmDbusWrapper* iDbusWrapper) :
        mpDbusWrapper(iDbusWrapper), //
        mpControlSender(NULL), //
        mpDBusConnection(NULL)
{
    assert(mpDbusWrapper);
    logInfo("CAmNodeStateCommunicator::CAmNodeStateCommunicator started");

    //save the DBusConnection
    mpDbusWrapper->getDBusConnection(mpDBusConnection);
    assert(mpDBusConnection!=NULL);

    //register the path and the callback for receiving messages
    std::string path("LifeCycleConsumer");
    gObjectPathVTable.message_function=CAmNodeStateCommunicator::receiveCallback;
    mpDbusWrapper->registerCallback(&gObjectPathVTable, path, this);

    //now we need to make sure we catch the signals from the NSM:
    dbus_bus_add_match(mpDBusConnection, "type=\'signal\',path=\'/org/genivi/NodeStateManager\'", NULL);
    if (!dbus_connection_add_filter(mpDBusConnection, CAmNodeStateCommunicator::signalCallback, this, NULL))
    {
        logError("CAmNodeStateCommunicator::CAmNodeStateCommunicator not enought memory!");
        throw std::runtime_error("CAmNodeStateCommunicator::CAmNodeStateCommunicator not enought memory!");
    }
    dbus_connection_flush(mpDBusConnection);
}

CAmNodeStateCommunicator::~CAmNodeStateCommunicator()
{}

/** retrieves the actual restartReason
 *
 * @param restartReason
 * @return E_OK on success
 */
am_Error_e CAmNodeStateCommunicator::nsmGetRestartReasonProperty(NsmRestartReason_e& restartReason)
{
    int32_t answer(0);
    am_Error_e error=readIntegerProperty("RestartReason",answer);
    restartReason=static_cast<NsmRestartReason_e>(answer);
    return(error);
}

/** retrieves the actual shutdownreason
 *
 * @param ShutdownReason
 * @return E_OK on success
 */
am_Error_e CAmNodeStateCommunicator::nsmGetShutdownReasonProperty(NsmShutdownReason_e& ShutdownReason)
{
    int32_t answer(0);
    am_Error_e error=readIntegerProperty("ShutdownReason",answer);
    ShutdownReason=static_cast<NsmShutdownReason_e>(answer);
    return(error);
}

/** retrieves the actual runnuing reason
 *
 * @param nsmRunningReason
 * @return E_OK on success
 */
am_Error_e CAmNodeStateCommunicator::nsmGetRunningReasonProperty(NsmRunningReason_e& nsmRunningReason)
{
    int32_t answer(0);
    am_Error_e error=readIntegerProperty("WakeUpReason",answer);
    nsmRunningReason=static_cast<NsmRunningReason_e>(answer);
    return(error);
}

/** gets the node state
 *
 * @param nsmNodeState
 * @return NsmErrorStatus_Ok on success
 */
NsmErrorStatus_e CAmNodeStateCommunicator::nsmGetNodeState(NsmNodeState_e& nsmNodeState)
{
    DBusError error;
    dbus_error_init(&error);

    uint32_t nodeStateID;
    uint32_t returnedError;

    DBusMessage * message = dbus_message_new_method_call(NSM_BUS_INTERFACE, NSM_PATH, NSM_INTERFACE, "GetNodeState");

    if (!message)
    {
        logError("CAmNodeStateCommunicator::nsmGetNodeState dbus error:", error.message);
        return (NsmErrorStatus_Dbus);
    }

    DBusMessage* reply(dbus_connection_send_with_reply_and_block(mpDBusConnection, message, -1, &error));
    if (!reply)
    {
        logError("CAmNodeStateCommunicator::nsmGetNodeState failed, dbus error", error.message);
        return (NsmErrorStatus_Dbus);
    }

    if(!dbus_message_get_args(reply, &error, DBUS_TYPE_INT32, &nodeStateID, DBUS_TYPE_INT32, &returnedError, DBUS_TYPE_INVALID))
        return (NsmErrorStatus_Dbus);

    dbus_message_unref(reply);

    nsmNodeState=static_cast<NsmNodeState_e>(nodeStateID);
    return (static_cast<NsmErrorStatus_e>(returnedError));
}

/** gets the session state for a session and seatID
 *
 * @param sessionName the name of the session
 * @param seatID the seatID
 * @param sessionState
 * @return NsmErrorStatus_Ok on success
 */
NsmErrorStatus_e CAmNodeStateCommunicator::nsmGetSessionState(const std::string& sessionName, const NsmSeat_e& seatID, NsmSessionState_e& sessionState)
{
    DBusError error;
    dbus_error_init(&error);
    DBusMessageIter iter;

    uint32_t returnedError;
    int32_t BsessionState(0);

    DBusMessage * message = dbus_message_new_method_call(NSM_BUS_INTERFACE, NSM_PATH, NSM_INTERFACE, "GetSessionState");

    if (!message)
    {
        logError("CAmNodeStateCommunicator::nsmGetSessionState dbus error:", error.message);
        return (NsmErrorStatus_Dbus);
    }

    dbus_message_iter_init_append(message, &iter);

    if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &sessionName))
    {
        logError( "CAmNodeStateCommunicator::nsmGetSessionState no more memory");
        return (NsmErrorStatus_Dbus);
    }

    if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32, &seatID))
    {
        logError( "CAmNodeStateCommunicator::nsmGetSessionState no more memory");
        return (NsmErrorStatus_Dbus);
    }

    DBusMessage* reply(dbus_connection_send_with_reply_and_block(mpDBusConnection, message, -1, &error));
    if (!reply)
    {
        logError("CAmNodeStateCommunicator::nsmGetSessionState failed, dbus error", error.message);
        return (NsmErrorStatus_Dbus);
    }

    if(!dbus_message_get_args(reply, &error,
            DBUS_TYPE_INT32, &BsessionState,
            DBUS_TYPE_INT32, &returnedError,DBUS_TYPE_INVALID))
        return (NsmErrorStatus_Dbus);

    dbus_message_unref(reply);

    sessionState=static_cast<NsmSessionState_e>(BsessionState);
    return (static_cast<NsmErrorStatus_e>(returnedError));
}

/** gets the application mode
 *
 * @param applicationMode
 * @return NsmErrorStatus_Ok on success
 */
NsmErrorStatus_e CAmNodeStateCommunicator::nsmGetApplicationMode(NsmApplicationMode_e& applicationMode)
{
    DBusError error;
    dbus_error_init(&error);

    uint32_t BapplicationMode(0),returnedError(0);

    DBusMessage * message = dbus_message_new_method_call(NSM_BUS_INTERFACE, NSM_PATH, NSM_INTERFACE, "GetApplicationMode");

    if (!message)
    {
        logError("CAmNodeStateCommunicator::nsmGetApplicationMode dbus error:", error.message);
        return (NsmErrorStatus_Dbus);
    }

    DBusMessage* reply(dbus_connection_send_with_reply_and_block(mpDBusConnection, message, -1, &error));
    if (!reply)
    {
        logError("CAmNodeStateCommunicator::nsmGetApplicationMode failed, dbus error", error.message);
        return (NsmErrorStatus_Dbus);
    }

    if(!dbus_message_get_args(reply, &error, DBUS_TYPE_INT32, &BapplicationMode, DBUS_TYPE_INT32, &returnedError, DBUS_TYPE_INVALID))
        return (NsmErrorStatus_Dbus);

    dbus_message_unref(reply);

    applicationMode=static_cast<NsmApplicationMode_e>(BapplicationMode);
    return (static_cast<NsmErrorStatus_e>(returnedError));
}

/** this function registers the AudioManager as shutdown client at the NSM
 *  for more information check the Nodestatemanager
 * @param shutdownMode the shutdownmode you wish to set
 * @param timeoutMs the timeout you need to have
 * @return NsmErrorStatus_Ok on success
 */
NsmErrorStatus_e CAmNodeStateCommunicator::nsmRegisterShutdownClient(const uint32_t shutdownMode, const uint32_t timeoutMs)
{
    DBusError error;
    DBusMessageIter iter;
    dbus_error_init(&error);
    int32_t returnError(0);
    std::string path = std::string(DBUS_SERVICE_OBJECT_PATH) + "/LifeCycleConsumer";
    const char* charPath = path.c_str();
    const char* service =DBUS_SERVICE_PREFIX;
    DBusMessage * message = dbus_message_new_method_call(NSM_BUS_INTERFACE, NSM_PATH, NSM_INTERFACE, "RegisterShutdownClient");

    if (!message)
    {
        logError( "CAmNodeStateCommunicator::nsmRegisterShutdownClient dbus error:", error.message);
        return (NsmErrorStatus_Dbus);
    }
    dbus_message_iter_init_append(message, &iter);

    if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &service))
    {
        logError( "CAmNodeStateCommunicator::nsmRegisterShutdownClient no more memory");
        return (NsmErrorStatus_Dbus);
    }

    if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &charPath))
    {
        logError( "CAmNodeStateCommunicator::nsmRegisterShutdownClient no more memory");
        return (NsmErrorStatus_Dbus);
    }

    if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &shutdownMode))
    {
        logError( "CAmNodeStateCommunicator::nsmRegisterShutdownClient no more memory");
        return (NsmErrorStatus_Dbus);
    }

    if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &timeoutMs))
    {
        logError( "CAmNodeStateCommunicator::nsmRegisterShutdownClient no more memory");
        return (NsmErrorStatus_Dbus);
    }

    DBusMessage* reply(dbus_connection_send_with_reply_and_block(mpDBusConnection, message, -1, &error));
    dbus_message_unref(message);

    if (!reply)
    {
        logError( "CAmRoutingDbusSend::send failed, dbus error", error.message);
        return (NsmErrorStatus_Dbus);
    }

    if(!dbus_message_get_args(reply, &error, DBUS_TYPE_INT32, &returnError,DBUS_TYPE_INVALID))
    {
        logError( "CAmRoutingDbusSend::send failed, dbus error", error.message);
        return (NsmErrorStatus_Dbus);
    }
    dbus_message_unref(reply);

    return (static_cast<NsmErrorStatus_e>(returnError));

}

/** this function unregisters the AudioManager as shutdown client at the NSM
 *
 * @param shutdownMode
 * @return NsmErrorStatus_Ok on success
 */
NsmErrorStatus_e CAmNodeStateCommunicator::nsmUnRegisterShutdownClient(const uint32_t shutdownMode)
{
    DBusError error;
    DBusMessageIter iter;
    dbus_error_init(&error);
    int32_t returnError(0);
    std::string path = std::string(DBUS_SERVICE_OBJECT_PATH) + "/LifeCycleConsumer";
    const char* charPath = path.c_str();
    const char* service =DBUS_SERVICE_PREFIX;
    DBusMessage * message = dbus_message_new_method_call(NSM_BUS_INTERFACE, NSM_PATH, NSM_INTERFACE, "UnRegisterShutdownClient");

    if (!message)
    {
        logError( "CAmNodeStateCommunicator::nsmUnRegisterShutdownClient dbus error:", error.message);
        return (NsmErrorStatus_Dbus);
    }
    dbus_message_iter_init_append(message, &iter);

    if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &service))
    {
        logError( "CAmNodeStateCommunicator::nsmUnRegisterShutdownClient no more memory");
        return (NsmErrorStatus_Dbus);
    }

    if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &charPath))
    {
        logError( "CAmNodeStateCommunicator::nsmUnRegisterShutdownClient no more memory");
        return (NsmErrorStatus_Dbus);
    }

    if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &shutdownMode))
    {
        logError( "CAmNodeStateCommunicator::nsmUnRegisterShutdownClient no more memory");
        return (NsmErrorStatus_Dbus);
    }

    DBusMessage* reply(dbus_connection_send_with_reply_and_block(mpDBusConnection, message, -1, &error));
    dbus_message_unref(message);

    if (!reply)
    {
        logError( "CAmNodeStateCommunicator::nsmUnRegisterShutdownClient failed, dbus error", error.message);
        return (NsmErrorStatus_Dbus);
    }

    if(!dbus_message_get_args(reply, &error, DBUS_TYPE_INT32, &returnError, DBUS_TYPE_INVALID))
    {
        logError( "CAmNodeStateCommunicator::nsmUnRegisterShutdownClient failed, dbus error", error.message);
        return (NsmErrorStatus_Dbus);
    }
    dbus_message_unref(reply);

    return (static_cast<NsmErrorStatus_e>(returnError));
}

/** returns the interface version
 *
 * @param version
 * @return E_OK on success
 */
am_Error_e CAmNodeStateCommunicator::nsmGetInterfaceVersion(uint32_t& version)
{
    DBusError error;
    dbus_error_init(&error);

    DBusMessage * message = dbus_message_new_method_call(NSM_BUS_INTERFACE, NSM_PATH, NSM_INTERFACE, "GetInterfaceVersion");

    if (!message)
    {
        logError("CAmNodeStateCommunicator::nsmGetInterfaceVersion dbus error:", error.message);
        return (E_UNKNOWN);
    }

    DBusMessage* reply(dbus_connection_send_with_reply_and_block(mpDBusConnection, message, -1, &error));

    dbus_message_unref(message);

    if (!reply)
    {
        logError("CAmNodeStateCommunicator::nsmGetInterfaceVersion failed, dbus error", error.message);
        return (E_UNKNOWN);
    }

    if(!dbus_message_get_args(reply, &error, DBUS_TYPE_UINT32, &version, DBUS_TYPE_INVALID))
    {
        logError("CAmNodeStateCommunicator::nsmGetInterfaceVersion failed, dbus error", error.message);
        return (E_UNKNOWN);
    }

    dbus_message_unref(reply);

    return (E_OK);
}

/** sends out the Lifecycle request complete message
 *
 * @param RequestId
 * @param status
 * @return NsmErrorStatus_Ok on success
 */
NsmErrorStatus_e CAmNodeStateCommunicator::nsmSendLifecycleRequestComplete(const uint32_t RequestId, const NsmErrorStatus_e status)
{
    DBusError error;
    DBusMessageIter iter;
    dbus_error_init(&error);
    int32_t returnError(0);
    DBusMessage * message = dbus_message_new_method_call(NSM_BUS_INTERFACE, NSM_PATH, NSM_INTERFACE, "LifecycleRequestComplete");

    if (!message)
    {
        logError( "CAmNodeStateCommunicator::nsmSendLifecycleRequestComplete dbus error:", error.message);
        return (NsmErrorStatus_Dbus);
    }
    dbus_message_iter_init_append(message, &iter);

    if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_UINT32, &RequestId))
    {
        logError( "CAmNodeStateCommunicator::nsmSendLifecycleRequestComplete no more memory");
        return (NsmErrorStatus_Dbus);
    }

    if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_INT32,&status))
    {
        logError( "CAmNodeStateCommunicator::nsmSendLifecycleRequestComplete no more memory");
        return (NsmErrorStatus_Dbus);
    }

    DBusMessage* reply(dbus_connection_send_with_reply_and_block(mpDBusConnection, message, -1, &error));
    dbus_message_unref(message);

    if (!reply)
    {
        logError( "CAmNodeStateCommunicator::nsmSendLifecycleRequestComplete failed, dbus error", error.message);
        return (NsmErrorStatus_Dbus);
    }

    if(!dbus_message_get_args(reply, &error,DBUS_TYPE_INT32, &returnError, DBUS_TYPE_INVALID))
    {
        logError( "CAmNodeStateCommunicator::nsmSendLifecycleRequestComplete failed, dbus error", error.message);
        return (NsmErrorStatus_Dbus);
    }
    dbus_message_unref(reply);

    return (static_cast<NsmErrorStatus_e>(returnError));
}

void CAmNodeStateCommunicator::registerControlSender(CAmControlSender* iControlSender)
{
    assert(iControlSender);
    mpControlSender=iControlSender;
}

DBusHandlerResult CAmNodeStateCommunicator::receiveCallback(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    CAmNodeStateCommunicator* instance = static_cast<CAmNodeStateCommunicator*>(user_data);
    assert(instance);
    return (instance->receiveCallbackDelegate(conn,msg));
}

DBusHandlerResult CAmNodeStateCommunicator::receiveCallbackDelegate(DBusConnection* conn, DBusMessage* msg)
{
    if (dbus_message_is_method_call(msg, DBUS_INTERFACE_INTROSPECTABLE, "Introspect"))
    {
        sendIntrospection(conn, msg);
        return (DBUS_HANDLER_RESULT_HANDLED);
    }
    else
    {
        DBusMessage * returnMessage;
        dbus_uint32_t Request(0),RequestId(0);
        //no introspection - ok. So we are only interested in out LifecycleRequest message...
        std::string method(dbus_message_get_member(msg));
        if (method=="LifecycleRequest")
        {
            DBusMessageIter iter,replyIter;
            if (!dbus_message_iter_init(msg, &iter))
            {
                logError("CAmNodeStateCommunicator::receiveCallbackDelegate DBus Message has no arguments!");
                returnMessage = dbus_message_new_error(msg,DBUS_ERROR_INVALID_ARGS, "DBUS Message has no arguments!");
                sendMessage(returnMessage,msg);
                return (DBUS_HANDLER_RESULT_HANDLED);
            }

            if (dbus_message_iter_get_arg_type(&iter)!=DBUS_TYPE_UINT32)
            {
                logError("CAmNodeStateCommunicator::receiveCallbackDelegate DBus Message has invalid arguments!");
                returnMessage = dbus_message_new_error(msg,DBUS_ERROR_INVALID_ARGS,"DBus argument is not uint32_t!");
                sendMessage(returnMessage,msg);
                return (DBUS_HANDLER_RESULT_HANDLED);
            }

            dbus_message_iter_get_basic(&iter, &Request);
            dbus_message_iter_next(&iter);

            if (dbus_message_iter_get_arg_type(&iter)!=DBUS_TYPE_UINT32)
            {
                logError("CAmNodeStateCommunicator::receiveCallbackDelegate DBus Message has invalid arguments!");
                returnMessage = dbus_message_new_error(msg,DBUS_ERROR_INVALID_ARGS,"DBus argument is not uint32_t!");
                sendMessage(returnMessage,msg);
                return (DBUS_HANDLER_RESULT_HANDLED);
            }

            dbus_message_iter_get_basic(&iter, &RequestId);

            assert(mpControlSender);
            NsmErrorStatus_e returnError = mpControlSender->hookSystemLifecycleRequest(static_cast<uint32_t>(Request),static_cast<uint32_t>(RequestId));

            returnMessage = dbus_message_new_method_return(msg);

            if (returnMessage == NULL)
            {
                logError("CAmNodeStateCommunicator::receiveCallbackDelegate Cannot allocate DBus message!");
                returnMessage = dbus_message_new_error(msg,DBUS_ERROR_NO_MEMORY,"Cannot create reply!");
                sendMessage(returnMessage,msg);
                return (DBUS_HANDLER_RESULT_HANDLED);
            }

            dbus_message_iter_init_append(returnMessage, &replyIter);

            if (!dbus_message_iter_append_basic(&replyIter, DBUS_TYPE_INT32, &returnError))
            {
                logError("CAmNodeStateCommunicator::receiveCallbackDelegate Cannot allocate DBus message!");
                returnMessage = dbus_message_new_error(msg,DBUS_ERROR_NO_MEMORY,"Cannot create reply!");
            }
            sendMessage(returnMessage,msg);
            return (DBUS_HANDLER_RESULT_HANDLED);
        }
    }
    return (DBUS_HANDLER_RESULT_NOT_YET_HANDLED);
}

void CAmNodeStateCommunicator::sendIntrospection(DBusConnection* conn, DBusMessage* msg)
{
    assert(conn != NULL);
    assert(msg != NULL);
    DBusMessage* reply;
    DBusMessageIter args;
    dbus_uint32_t serial = 0;

    // create a reply from the message
    reply = dbus_message_new_method_return(msg);
    std::string fullpath(NSM_INTROSPECTION_FILE);
    std::ifstream in(fullpath.c_str(), std::ifstream::in);
    if (!in)
    {
       logError("IAmCommandReceiverShadow::sendIntrospection could not load xml file ",fullpath);
       throw std::runtime_error("IAmCommandReceiverShadow::sendIntrospection Could not load introspecton XML");
    }
    std::stringstream buffer;
    buffer << in.rdbuf();
    std::string introspect = buffer.str();
    const char* string = introspect.c_str();

    // add the arguments to the reply
    dbus_message_iter_init_append(reply, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &string))
    {
       logError( "CAmNodeStateCommunicator::sendIntrospection DBUS handler Out Of Memory!");
    }

    // send the reply && flush the connection
    if (!dbus_connection_send(conn, reply, &serial))
    {
        logError( "CAmNodeStateCommunicator::sendIntrospection DBUS handler Out Of Memory!");
    }
    dbus_connection_flush(conn);

    // free the reply
    dbus_message_unref(reply);
}

void CAmNodeStateCommunicator::sendMessage(DBusMessage* message, DBusMessage* origMessage)
{
    dbus_uint32_t serial = dbus_message_get_serial(origMessage);

    if(!dbus_connection_send(mpDBusConnection, message, &serial))
    {
    	logError( "CAmNodeStateCommunicator::sendMessage DBUS handler Out Of Memory!");
    }
    dbus_connection_flush(mpDBusConnection);
    dbus_message_unref(message);
}

DBusHandlerResult CAmNodeStateCommunicator::signalCallback(DBusConnection* conn, DBusMessage* msg, void* user_data)
{
    (void) conn;
    CAmNodeStateCommunicator* instance(static_cast<CAmNodeStateCommunicator*>(user_data));

    const char* iface = dbus_message_get_interface(msg);
    if (iface==NULL)
        return (DBUS_HANDLER_RESULT_NOT_YET_HANDLED);
    std::string interface(iface);
    std::string member = dbus_message_get_member(msg);

    if (interface=="org.genivi.NodeStateManager.Consumer")
    {
        if (member=="NodeState")
        {
            int32_t nodeState;
            DBusMessageIter iter;
            if (!dbus_message_iter_init(msg, &iter))
            {
                logError("CAmNodeStateCommunicator::signalCallback NodeState DBus Message has no arguments!");
                return (DBUS_HANDLER_RESULT_HANDLED);
            }

            if (dbus_message_iter_get_arg_type(&iter)!=DBUS_TYPE_INT32)
            {
                logError("CAmNodeStateCommunicator::signalCallback NodeState DBus Message has invalid arguments!");
                return (DBUS_HANDLER_RESULT_HANDLED);
            }

            dbus_message_iter_get_basic(&iter, &nodeState);

            logInfo("CAmNodeStateCommunicator::signalCallback got signal NodeState, with nodeState",nodeState);

            assert(instance->mpControlSender);
            instance->mpControlSender->hookSystemNodeStateChanged(static_cast<NsmNodeState_e>(nodeState));
            return (DBUS_HANDLER_RESULT_HANDLED);
        }

        else if (member=="NodeApplicationMode")
        {
            int32_t nodeApplicationMode;
            DBusMessageIter iter;
            if (!dbus_message_iter_init(msg, &iter))
            {
                logError("CAmNodeStateCommunicator::signalCallback nodeApplicationMode DBus Message has no arguments!");
                return (DBUS_HANDLER_RESULT_HANDLED);
            }

            if (dbus_message_iter_get_arg_type(&iter)!=DBUS_TYPE_INT32)
            {
                logError("CAmNodeStateCommunicator::signalCallback nodeApplicationMode DBus Message has invalid arguments!");
                return (DBUS_HANDLER_RESULT_HANDLED);
            }

            dbus_message_iter_get_basic(&iter, &nodeApplicationMode);

            logInfo("CAmNodeStateCommunicator::signalCallback got signal nodeApplicationMode, with applicationMode",nodeApplicationMode);

            assert(instance->mpControlSender);
            instance->mpControlSender->hookSystemNodeApplicationModeChanged(static_cast<NsmApplicationMode_e>(nodeApplicationMode));
            return (DBUS_HANDLER_RESULT_HANDLED);
        }

        else if (member=="SessionStateChanged")
        {
            std::string sessionName;
            NsmSeat_e seatID;
            NsmSessionState_e sessionState;
            DBusMessageIter iter;
            if (!dbus_message_iter_init(msg, &iter))
            {
                logError("CAmNodeStateCommunicator::signalCallback nodeApplicationMode DBus Message has no arguments!");
                return (DBUS_HANDLER_RESULT_HANDLED);
            }

            if (dbus_message_iter_get_arg_type(&iter)!=DBUS_TYPE_STRING)
            {
                logError("CAmNodeStateCommunicator::signalCallback nodeApplicationMode DBus Message has invalid arguments!");
                return (DBUS_HANDLER_RESULT_HANDLED);
            }

            char * sessionNameChar;
            dbus_message_iter_get_basic(&iter, &sessionNameChar);
            sessionName=std::string(sessionNameChar);
            dbus_message_iter_next(&iter);

            if (dbus_message_iter_get_arg_type(&iter)!=DBUS_TYPE_INT32)
            {
                logError("CAmNodeStateCommunicator::signalCallback nodeApplicationMode DBus Message has invalid arguments!");
                return (DBUS_HANDLER_RESULT_HANDLED);
            }

            dbus_message_iter_get_basic(&iter, &seatID);
            dbus_message_iter_next(&iter);

            if (dbus_message_iter_get_arg_type(&iter)!=DBUS_TYPE_INT32)
            {
                logError("CAmNodeStateCommunicator::signalCallback nodeApplicationMode DBus Message has invalid arguments!");
                return (DBUS_HANDLER_RESULT_HANDLED);
            }

            dbus_message_iter_get_basic(&iter, &sessionState);


            logInfo("CAmNodeStateCommunicator::signalCallback got signal sessionStateChanged, with session",sessionName,"seatID=",seatID,"sessionState",sessionState);

            assert(instance->mpControlSender);
            instance->mpControlSender->hookSystemSessionStateChanged(sessionName,seatID,sessionState);
            return (DBUS_HANDLER_RESULT_HANDLED);
        }

        else
        {
            return (DBUS_HANDLER_RESULT_NOT_YET_HANDLED);
        }
    }

    return (DBUS_HANDLER_RESULT_NOT_YET_HANDLED);
}

am_Error_e CAmNodeStateCommunicator::readIntegerProperty(const std::string property, int32_t& value)
{
    DBusError error;
    dbus_error_init(&error);
    DBusMessageIter iter,iterVariant;

    DBusMessage * message = dbus_message_new_method_call(NSM_BUS_INTERFACE, NSM_PATH, "org.freedesktop.DBus.Properties", "Get");

    if (!message)
    {
        logError("CAmNodeStateCommunicator::readIntegerProperty dbus error:", error.message);
        dbus_message_unref(message);
        return (E_UNKNOWN);
    }


    dbus_message_iter_init_append(message, &iter);
    const char *interface=NSM_INTERFACE;
    const char *propertyChar=property.c_str();
    if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &interface))
    {
        logError("CAmNodeStateCommunicator::readIntegerProperty append error");
        dbus_message_unref(message);
        return (E_UNKNOWN);
    }

    if (!dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING, &propertyChar))
    {
        logError("CAmNodeStateCommunicator::readIntegerProperty append error");
        dbus_message_unref(message);
        return (E_UNKNOWN);
    }

    DBusMessage* reply(dbus_connection_send_with_reply_and_block(mpDBusConnection, message, -1, &error));
    if (!reply)
    {
        logError("CAmNodeStateCommunicator::readIntegerProperty failed, dbus error", error.message);
        dbus_message_unref(message);
        return (E_UNKNOWN);
    }

    if(!dbus_message_iter_init(reply,&iterVariant))
    {
    	logError("CAmNodeStateCommunicator::readIntegerProperty failed, dbus error", error.message);
		dbus_message_unref(message);
		dbus_message_unref(reply);
		return (E_UNKNOWN);
    }
    if (dbus_message_iter_get_arg_type (&iterVariant)!= DBUS_TYPE_VARIANT)
    {
        logError("CAmNodeStateCommunicator::readIntegerProperty failed, dbus return type wrong");
        dbus_message_unref(reply);
        dbus_message_unref(message);
        return (E_UNKNOWN);
    }
    DBusMessageIter subiter;
    dbus_message_iter_recurse (&iterVariant, &subiter);
    if (dbus_message_iter_get_arg_type (&subiter)!= DBUS_TYPE_INT32)
    {
        logError("CAmNodeStateCommunicator::readIntegerProperty failed, dbus return type wrong");
        dbus_message_unref(reply);
        dbus_message_unref(message);
        return (E_UNKNOWN);
    }

   dbus_message_iter_get_basic(&subiter,&value);
   dbus_message_unref(reply);
   dbus_message_unref(message);

    return (E_OK);
}

} /* namespace am */
