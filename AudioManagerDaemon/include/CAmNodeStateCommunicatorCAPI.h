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
 * \author Aleksandar Donchev, aleksander.donchev@partner.bmw.de BMW 2013
 *
 * \file CAmNodeStateCommunicatorCAPI.h
 * For further information see http://www.genivi.org/.
 *
 */
#ifndef CAMNODESTATECOMMUNICATORCAPI_H_
#define CAMNODESTATECOMMUNICATORCAPI_H_

#include "CAmNodeStateCommunicator.h"
#include "CAmCommonAPIWrapper.h"
#include <v1_0/org/genivi/NodeStateManager/ConsumerProxy.hpp>
#include <v1_0/org/genivi/NodeStateManager/LifeCycleConsumerStubDefault.hpp>


namespace am
{

#define am_nodestatemanager v1_0::org::genivi::NodeStateManager


class CAmCommonAPIWrapper;
/** communicates with the NSM
 *  The CAmNodeStateCommunicator communicates with the NodeStateManager via Common-API wrapping mechanism. It works only, if CAmCommonAPIWrapper is enabled.
 */
class CAmNodeStateCommunicatorCAPI : public CAmNodeStateCommunicator
{
    static const char * DEFAULT_DOMAIN_STRING;
    static const char * CLIENT_INSTANCE_STRING;
    static const char * CLIENT_INTERFACE_STRING;
    static const char * LIFECYCLE_SERVICE_INSTANCE_STRING;
    static const char * LIFECYCLE_SERVICE_INTERFACE_STRING;
    static const char * OBJECT_NAME;
    static const char * BUS_NAME;

	/* A concrete implementation of the life cycle stub.
	 * An object from this class is instantiated from the common-api factory.
	 * It forwards the invocations to its delegate CAmNodeStateCommunicatorCAPI.
	*/
	class CAmNodeStateCommunicatorServiceImpl : public am_nodestatemanager::LifeCycleConsumerStubDefault
	{
		CAmNodeStateCommunicatorCAPI *mpDelegate;
	public:
		CAmNodeStateCommunicatorServiceImpl ():mpDelegate(NULL) {}
		CAmNodeStateCommunicatorServiceImpl (CAmNodeStateCommunicatorCAPI *aNSCommunicator):mpDelegate(aNSCommunicator) {}
		~CAmNodeStateCommunicatorServiceImpl() { mpDelegate = NULL; }

		CAmNodeStateCommunicatorCAPI *getDelegate() { return mpDelegate; };
		void setDelegate(CAmNodeStateCommunicatorCAPI *aDelegate) { mpDelegate = aDelegate; };

		void LifecycleRequest(uint32_t Request, uint32_t RequestId, int32_t& ErrorCode) {
		   if(mpDelegate)
			   mpDelegate->cbReceivedLifecycleRequest(Request, RequestId, ErrorCode);
		}

		void LifecycleRequest(const std::shared_ptr<CommonAPI::ClientId> client, uint32_t Request, uint32_t RequestId, LifecycleRequestReply_t reply){
			int32_t ErrorCode;
			LifecycleRequest(Request, RequestId, ErrorCode);
			reply(ErrorCode);
		}
	};

	CAmCommonAPIWrapper *mpCAPIWrapper;
	std::shared_ptr<am_nodestatemanager::ConsumerProxy<> > mNSMProxy;
	std::shared_ptr<CAmNodeStateCommunicatorCAPI::CAmNodeStateCommunicatorServiceImpl> mNSMStub;

    /* Client events */
    void onNodeStateEvent(const int32_t nodeState);
    void onNodeApplicationModeEvent(const int32_t nodeApplicationMode);
    void onSessionStateChangedEvent(const std::string & sessionName, const int32_t seatID, const int32_t sessionState);
    void onServiceStatusEvent(const CommonAPI::AvailabilityStatus& serviceStatus);
    /* Service callbacks */
    void cbReceivedLifecycleRequest(uint32_t Request, uint32_t RequestId, int32_t& ErrorCode);

protected:
    bool mIsServiceAvailable;

public:
	CAmNodeStateCommunicatorCAPI(CAmCommonAPIWrapper* iCAPIWrapper);
    virtual ~CAmNodeStateCommunicatorCAPI();

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
    bool isServiceAvailable();
};

}
/* namespace am */
#endif /* CAMNODESTATECOMMUNICATORCAPI_H_ */
