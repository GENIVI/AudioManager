/**
 * SPDX license identifier: MPL-2.0
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
 * \author Christian Linke, christian.linke@bmw.de BMW 2012
 * \author Aleksandar Donchev, aleksander.donchev@partner.bmw.de BMW 2013
 *
 * \file CAmNodeStateCommunicator.h
 * For further information see http://www.genivi.org/.
 *
 */

#ifndef CAMNODESTATECOMMUNICATORBASE_H_
#define CAMNODESTATECOMMUNICATORBASE_H_

#include <assert.h>
#include "audiomanagerconfig.h"
#include "NodeStateManager.h"
#include "audiomanagertypes.h"



namespace am
{

class CAmControlSender;

/** communicates with the NSM
 *  The CAmNodeStateCommunicator communicates with the NodeStateManager.
 *  The CAmNodeStateCommunicator is triggered via CAmControlReceiver, so you can communicate from the ControllerPlugin with it.
 *  Most of the interfaces are passive, so you get the information you need via retrieving it. If you need to register the AudioManager
 *  as LifeCycleConsumer, you need to call CAmNodeStateCommunicator::nsmRegisterShutdownClient which can be undone with CAmNodeStateCommunicator::nsmUnRegisterShutdownClient.
 *  After you have registered, you will get hookSystemLifecycleRequest on the ControlSendInterface of the controller.
 *  You should answer this within your set timeout with CAmNodeStateCommunicator::nsmSendLifecycleRequestComplete.
 */

class CAmNodeStateCommunicator
{
protected:
    CAmControlSender* mpControlSender;
public:
	CAmNodeStateCommunicator():mpControlSender(NULL) {}
    virtual ~CAmNodeStateCommunicator() {}
    virtual am_Error_e nsmGetRestartReasonProperty(NsmRestartReason_e& restartReason) = 0;
    virtual am_Error_e nsmGetShutdownReasonProperty(NsmShutdownReason_e& ShutdownReason) = 0;
    virtual am_Error_e nsmGetRunningReasonProperty(NsmRunningReason_e& nsmRunningReason) = 0;
    virtual NsmErrorStatus_e nsmGetNodeState(NsmNodeState_e& nsmNodeState) = 0;
    virtual NsmErrorStatus_e nsmGetSessionState(const std::string& sessionName, const NsmSeat_e& seatID, NsmSessionState_e& sessionState) = 0;
    virtual NsmErrorStatus_e nsmGetApplicationMode(NsmApplicationMode_e& applicationMode) = 0;
    virtual NsmErrorStatus_e nsmRegisterShutdownClient(const uint32_t shutdownMode, const uint32_t timeoutMs) = 0;
    virtual NsmErrorStatus_e nsmUnRegisterShutdownClient(const uint32_t shutdownMode) = 0;
    virtual am_Error_e nsmGetInterfaceVersion(uint32_t& version) = 0;
    virtual NsmErrorStatus_e nsmSendLifecycleRequestComplete(const uint32_t RequestId, const NsmErrorStatus_e status) = 0;
    virtual void registerControlSender(CAmControlSender* iControlSender) {
    	assert(NULL!=iControlSender);
        mpControlSender = iControlSender;
    }
};

}

#endif /* CAMNODESTATECOMMUNICATORBASE_H_ */
