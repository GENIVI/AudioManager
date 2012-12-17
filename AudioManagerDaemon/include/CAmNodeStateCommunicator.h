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
 * \file CAmNodeStateCommunicator.h
 * For further information see http://www.genivi.org/.
 *
 */
#ifndef CAMNODESTATECOMMUNICATOR_H_
#define CAMNODESTATECOMMUNICATOR_H_

#include "shared/CAmDbusWrapper.h"
#include "NodeStateManager.h"

namespace am
{

class CAmControlSender;

/** communicates with the NSM
 *  The CAmNodeStateCommunicator communicates with the NodeStateManager via Dbus. Only works, if CAmDBusWrapper is enabled.
 *  The CAmNodeStateCommunicator is triggered via CAmControlReceiver, so you can communicate from the ControllerPlugin with it.
 *  Most if the interfaces are passive, so you get the information you need via retrieving it. If you need to register the AudioManager
 *  as LifeCycleConsumer, you need to call CAmNodeStateCommunicator::nsmRegisterShutdownClient which can be undone with CAmNodeStateCommunicator::nsmUnRegisterShutdownClient.
 *  After you have registered, you will get hookSystemLifecycleRequest on the ControlSendInterface of the controller.
 *  You should answer this within your set timeout with CAmNodeStateCommunicator::nsmSendLifecycleRequestComplete.
 */
class CAmNodeStateCommunicator
{
public:
    CAmNodeStateCommunicator(CAmDbusWrapper* iDbusWrapper);
    virtual ~CAmNodeStateCommunicator();
    am_Error_e nsmGetRestartReasonProperty(NsmRestartReason_e& restartReason) ;
    am_Error_e nsmGetShutdownReasonProperty(NsmShutdownReason_e& ShutdownReason) ;
    am_Error_e nsmGetRunningReasonProperty(NsmRunningReason_e& nsmRunningReason) ;
    NsmErrorStatus_e nsmGetNodeState(NsmNodeState_e& nsmNodeState) ;
    NsmErrorStatus_e nsmGetSessionState(const std::string& sessionName, const NsmSeat_e& seatID, NsmSessionState_e& sessionState) ;
    NsmErrorStatus_e nsmGetApplicationMode(NsmApplicationMode_e& applicationMode) ;
    NsmErrorStatus_e nsmRegisterShutdownClient(const uint32_t shutdownMode, const uint32_t timeoutMs) ;
    NsmErrorStatus_e nsmUnRegisterShutdownClient(const uint32_t shutdownMode) ;
    am_Error_e nsmGetInterfaceVersion(uint32_t& version) ;
    NsmErrorStatus_e nsmSendLifecycleRequestComplete(const uint32_t RequestId, const NsmErrorStatus_e status) ;

    void registerControlSender(CAmControlSender* iControlSender);
    static DBusHandlerResult receiveCallback(DBusConnection *conn, DBusMessage *msg, void *user_data);
    static DBusHandlerResult signalCallback(DBusConnection *conn, DBusMessage *msg, void *user_data);


private:
    void sendIntrospection(DBusConnection* conn, DBusMessage* msg);
    void sendMessage(DBusMessage* message, DBusMessage* origMessage);
    DBusHandlerResult receiveCallbackDelegate(DBusConnection *conn, DBusMessage *msg);
    am_Error_e readIntegerProperty(const std::string property, int32_t &value);
    CAmDbusWrapper* mpDbusWrapper;
    CAmControlSender* mpControlSender;
    DBusConnection* mpDBusConnection;
};

} /* namespace am */
#endif /* CAMNODESTATECOMMUNICATOR_H_ */
