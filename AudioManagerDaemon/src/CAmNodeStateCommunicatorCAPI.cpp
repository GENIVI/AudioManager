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
 * \file CAmNodeStateCommunicatorCAPI.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include <assert.h>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <functional>
#include <memory>
#include <CommonAPI/CommonAPI.h>
#include "config.h"
#include "CAmCommonAPIWrapper.h"
#include "CAmDltWrapper.h"
#include "CAmNodeStateCommunicatorCAPI.h"
#include "CAmControlSender.h"
#include <org/genivi/NodeStateManager/LifeCycleConsumerProxy.h>


namespace am
{

const char * CAmNodeStateCommunicatorCAPI::CLIENT_STRING = "local:org.genivi.NodeStateManager.Consumer:org.genivi.NodeStateManager";
const char * CAmNodeStateCommunicatorCAPI::SERVER_STRING = "local:org.genivi.NodeStateManager.LifeCycleConsumer:org.genivi.audiomanager";

const char * CAmNodeStateCommunicatorCAPI::OBJECT_NAME = "/org/genivi/audiomanager/LifeCycleConsumer";
const char * CAmNodeStateCommunicatorCAPI::BUS_NAME = "org.genivi.audiomanager";


#define IF_NOT_AVAILABLE_RETURN(error) \
if(!mIsServiceAvailable) { logError(__PRETTY_FUNCTION__, "Node State Manager not available yet"); return error; }

/**
 * Retrieves the value from given attribute wrapper.
 */
template <typename TValueReturnType, class TValueClass> am_Error_e getAttributeValue(Attribute<TValueClass>* attribute, TValueReturnType & resultValue)
{
	CallStatus status;
	typename Attribute<TValueClass>::ValueType value;
	attribute->getValue(status, value);
	std::cout << std::endl << "CallStatus : " << static_cast<int>(status) << std::endl;
	if( CallStatus::SUCCESS == status)
	{
		resultValue = static_cast<TValueReturnType>(value);
		return E_OK;
	}
	return E_UNKNOWN;
}


CAmNodeStateCommunicatorCAPI::CAmNodeStateCommunicatorCAPI(CAmCommonAPIWrapper* iCAPIWrapper) :
		CAmNodeStateCommunicator(),
		mpCAPIWrapper(iCAPIWrapper),
		mIsServiceAvailable(false)
{
    assert(mpCAPIWrapper);
    logInfo("CAmNodeStateCommunicatorCAPI::CAmNodeStateCommunicatorCAPI started");

    //Gets the factory pointer and build a proxy object
    std::shared_ptr<CommonAPI::Factory> factory = iCAPIWrapper->factory();
    mNSMProxy = factory->buildProxy<ConsumerProxy>(CAmNodeStateCommunicatorCAPI::CLIENT_STRING);

    //Makes subscriptions to the following 3 events
    mNSMProxy->getNodeStateEvent().subscribe(
    		std::bind(&CAmNodeStateCommunicatorCAPI::onNodeStateEvent, this, std::placeholders::_1)
    );
     mNSMProxy->getNodeApplicationModeEvent().subscribe(
    		std::bind(&CAmNodeStateCommunicatorCAPI::onNodeApplicationModeEvent, this, std::placeholders::_1)
    );
     mNSMProxy->getSessionStateChangedEvent().subscribe(
    		std::bind(&CAmNodeStateCommunicatorCAPI::onSessionStateChangedEvent, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
    );
    mNSMProxy->getProxyStatusEvent().subscribe(std::bind(&CAmNodeStateCommunicatorCAPI::onServiceStatusEvent,this,std::placeholders::_1));
    //Instantiates the concrete stub implementation
    mNSMStub = std::make_shared<CAmNodeStateCommunicatorCAPI::CAmNodeStateCommunicatorServiceImpl>(this);

    //Registers the service
    iCAPIWrapper->registerStub(mNSMStub, CAmNodeStateCommunicatorCAPI::SERVER_STRING);
}

CAmNodeStateCommunicatorCAPI::~CAmNodeStateCommunicatorCAPI()
{
	mNSMProxy.reset();
	mpCAPIWrapper->unregisterStub(CAmNodeStateCommunicatorCAPI::SERVER_STRING);
	mNSMStub->setDelegate(NULL);
	mNSMStub.reset();
	mpCAPIWrapper = NULL;
}

bool CAmNodeStateCommunicatorCAPI::isServiceAvailable()
{
    return mIsServiceAvailable;
}

/** retrieves the actual restart reason
 *
 * @param restartReason
 * @return E_OK on success
 */
am_Error_e CAmNodeStateCommunicatorCAPI::nsmGetRestartReasonProperty(NsmRestartReason_e& restartReason)
{
	//Check the service via the proxy object is available
	IF_NOT_AVAILABLE_RETURN(E_NOT_POSSIBLE)
	//Get the attribute
	int32_t value;
	CommonAPI::CallStatus status;
	mNSMProxy->getRestartReasonAttribute().getValue(status,value);
	if (status!=CommonAPI::CallStatus::SUCCESS)
		return (E_UNKNOWN);
	restartReason=static_cast<NsmRestartReason_e>(value);
	return (E_OK);
}

/** retrieves the actual shutdown reason
 *
 * @param ShutdownReason
 * @return E_OK on success
 */
am_Error_e CAmNodeStateCommunicatorCAPI::nsmGetShutdownReasonProperty(NsmShutdownReason_e& ShutdownReason)
{
	//Check the service via the proxy object is available
	IF_NOT_AVAILABLE_RETURN(E_NOT_POSSIBLE)
	//Get the attribute
	int32_t value;
	CommonAPI::CallStatus status;
	mNSMProxy->getShutdownReasonAttribute().getValue(status,value);
	if (status!=CommonAPI::CallStatus::SUCCESS)
		return (E_UNKNOWN);
	ShutdownReason=static_cast<NsmShutdownReason_e>(value);
	return (E_OK);
}

/** retrieves the actual running reason
 *
 * @param nsmRunningReason
 * @return E_OK on success
 */
am_Error_e CAmNodeStateCommunicatorCAPI::nsmGetRunningReasonProperty(NsmRunningReason_e& nsmRunningReason)
{
	//Check the service via the proxy object is available
	IF_NOT_AVAILABLE_RETURN(E_NOT_POSSIBLE)
	//Get the attribute
	int32_t value;
	CommonAPI::CallStatus status;
	mNSMProxy->getWakeUpReasonAttribute().getValue(status,value);
	if (status!=CommonAPI::CallStatus::SUCCESS)
		return (E_UNKNOWN);
	nsmRunningReason=static_cast<NsmRunningReason_e>(value);
	return (E_OK);
}

/** gets the node state
 *
 * @param nsmNodeState
 * @return NsmErrorStatus_Ok on success
 */
NsmErrorStatus_e CAmNodeStateCommunicatorCAPI::nsmGetNodeState(NsmNodeState_e& nsmNodeState)
{
	//Check the service via the proxy object is available
	IF_NOT_AVAILABLE_RETURN(NsmErrorStatus_Error)

	CallStatus callStatus;
	int32_t tmpNodeState = 0, errorCode = 0;
	mNSMProxy->GetNodeState(callStatus, tmpNodeState, errorCode);
	if( CallStatus::SUCCESS == callStatus )
	{
		nsmNodeState = static_cast<NsmNodeState_e>(tmpNodeState);
		return (static_cast<NsmErrorStatus_e>(errorCode));
	}
	return NsmErrorStatus_Error;
}

/** gets the session state for a session and seatID
 *
 * @param sessionName the name of the session
 * @param seatID the seatID
 * @param sessionState
 * @return NsmErrorStatus_Ok on success
 */
NsmErrorStatus_e CAmNodeStateCommunicatorCAPI::nsmGetSessionState(const std::string& sessionName, const NsmSeat_e& seatID, NsmSessionState_e& sessionState)
{
	//Check the service via the proxy object is available
	IF_NOT_AVAILABLE_RETURN(NsmErrorStatus_Error)

	CallStatus callStatus;
	int32_t tmpSessionState = 0 , errorCode = 0;
	mNSMProxy->GetSessionState(sessionName,seatID,callStatus, tmpSessionState, errorCode);

	if( CallStatus::SUCCESS == callStatus)
	{
		sessionState = static_cast<NsmSessionState_e>(tmpSessionState);
		return (static_cast<NsmErrorStatus_e>(errorCode));
	}
	return NsmErrorStatus_Error;
}

/** gets the application mode
 *
 * @param applicationMode
 * @return NsmErrorStatus_Ok on success
 */
NsmErrorStatus_e CAmNodeStateCommunicatorCAPI::nsmGetApplicationMode(NsmApplicationMode_e& applicationMode)
{
	//Check the service via the proxy object is available
	IF_NOT_AVAILABLE_RETURN(NsmErrorStatus_Error)

	CallStatus callStatus;
	int32_t tmpAppMode = 0 , errorCode = 0;
	mNSMProxy->GetApplicationMode(callStatus, tmpAppMode, errorCode);
	if( CallStatus::SUCCESS == callStatus)
	{
		applicationMode = static_cast<NsmApplicationMode_e>(tmpAppMode);
		return (static_cast<NsmErrorStatus_e>(errorCode));
	}
	return NsmErrorStatus_Dbus;
}

/** this function registers the AudioManager as shutdown client at the NSM
 *  for more information check the Nodestatemanager
 * @param shutdownMode the shutdownmode you wish to set
 * @param timeoutMs the timeout you need to have
 * @return NsmErrorStatus_Ok on success
 */
NsmErrorStatus_e CAmNodeStateCommunicatorCAPI::nsmRegisterShutdownClient(const uint32_t shutdownMode, const uint32_t timeoutMs)
{
	//Check the service via the proxy object is available
	IF_NOT_AVAILABLE_RETURN(NsmErrorStatus_Error)

	CallStatus callStatus;
	int32_t errorCode = 0;
	std::string objName = std::string(CAmNodeStateCommunicatorCAPI::OBJECT_NAME);
	std::string busName = std::string(CAmNodeStateCommunicatorCAPI::BUS_NAME);
	mNSMProxy->RegisterShutdownClient(busName, objName, shutdownMode, timeoutMs, callStatus, errorCode);
	if( CallStatus::SUCCESS == callStatus)
		return (static_cast<NsmErrorStatus_e>(errorCode));
	return NsmErrorStatus_Dbus;

}

/** this function unregisters the AudioManager as shutdown client at the NSM
 *
 * @param shutdownMode
 * @return NsmErrorStatus_Ok on success
 */
NsmErrorStatus_e CAmNodeStateCommunicatorCAPI::nsmUnRegisterShutdownClient(const uint32_t shutdownMode)
{
	//Check the service via the proxy object is available
	IF_NOT_AVAILABLE_RETURN(NsmErrorStatus_Error)

	CallStatus callStatus;
	int32_t errorCode = 0;
	std::string objName = std::string(CAmNodeStateCommunicatorCAPI::OBJECT_NAME);
	std::string busName = std::string(CAmNodeStateCommunicatorCAPI::BUS_NAME);
	mNSMProxy->UnRegisterShutdownClient(busName, objName, shutdownMode, callStatus, errorCode);
	if( CallStatus::SUCCESS == callStatus)
		return (static_cast<NsmErrorStatus_e>(errorCode));
	return NsmErrorStatus_Dbus;
}

/** returns the interface version
 *
 * @param version
 * @return E_OK on success
 */
am_Error_e CAmNodeStateCommunicatorCAPI::nsmGetInterfaceVersion(uint32_t& version)
{
	//Check the service via the proxy object is available
	IF_NOT_AVAILABLE_RETURN(E_NOT_POSSIBLE)

	CallStatus callStatus;
	mNSMProxy->GetInterfaceVersion(callStatus, version);
	if( CallStatus::SUCCESS == callStatus)
		return E_OK;
	return E_UNKNOWN;
}

/** sends out the Lifecycle request complete message
 *
 * @param RequestId
 * @param status
 * @return NsmErrorStatus_Ok on success
 */
NsmErrorStatus_e CAmNodeStateCommunicatorCAPI::nsmSendLifecycleRequestComplete(const uint32_t RequestId, const NsmErrorStatus_e status)
{
	//Check the service via the proxy object is available
	IF_NOT_AVAILABLE_RETURN(NsmErrorStatus_Error)

	CallStatus callStatus;
	int32_t errorCode = 0;
	mNSMProxy->LifecycleRequestComplete(RequestId, status, callStatus, errorCode);
	if( CallStatus::SUCCESS == callStatus)
	{
		return (static_cast<NsmErrorStatus_e>(errorCode));
	}
	return NsmErrorStatus_Dbus;
}

/** notification handler for changed node state
 *
 * @param nodeState
 * @return none
 */
void CAmNodeStateCommunicatorCAPI::onNodeStateEvent(const int32_t nodeState)
{
	logInfo(__PRETTY_FUNCTION__, " got signal NodeState, with nodeState",nodeState);
	assert(mpControlSender);
	mpControlSender->hookSystemNodeStateChanged(static_cast<NsmNodeState_e>(nodeState));
}

/** notification handler for changed node application mode
 *
 * @param nodeApplicationMode
 * @return none
 */
void CAmNodeStateCommunicatorCAPI::onNodeApplicationModeEvent(const int32_t nodeApplicationMode)
{
	logInfo(__PRETTY_FUNCTION__, " got signal nodeApplicationMode, with applicationMode",nodeApplicationMode);
    assert(mpControlSender);
    mpControlSender->hookSystemNodeApplicationModeChanged(static_cast<NsmApplicationMode_e>(nodeApplicationMode));
}

/** notification handler for changed session state
 *
 * @param sessionName
 * @param seatID
 * @param sessionState
 * @return none
 */
void CAmNodeStateCommunicatorCAPI::onSessionStateChangedEvent(const std::string & sessionName, const int32_t seatID, const int32_t sessionState)
{
    logInfo(__PRETTY_FUNCTION__, " got signal sessionStateChanged, with session",sessionName,"seatID=",seatID,"sessionState",sessionState);
    assert(mpControlSender);
    mpControlSender->hookSystemSessionStateChanged(sessionName, static_cast<NsmSeat_e>(seatID), static_cast<NsmSessionState_e>(sessionState));
}

void CAmNodeStateCommunicatorCAPI::onServiceStatusEvent(const CommonAPI::AvailabilityStatus& serviceStatus)
{
    std::stringstream  avail;
    avail  << "(" << static_cast<int>(serviceStatus) << ")";

    logInfo("Service Status of the NSM changed to ", avail.str());
    std::cout << std::endl << "Service Status of the NSM changed to " << avail.str();
    mIsServiceAvailable = (serviceStatus==CommonAPI::AvailabilityStatus::AVAILABLE);
}

/** implements the service part, which is invoked from the node state manager
 *
 * @param sessionName
 * @param seatID
 * @param sessionState
 * @return none
 */
void CAmNodeStateCommunicatorCAPI::cbReceivedLifecycleRequest(uint32_t Request, uint32_t RequestId, int32_t& ErrorCode)
{
    assert(mpControlSender);
    ErrorCode = mpControlSender->hookSystemLifecycleRequest(Request, RequestId);
}

} /* namespace am */
