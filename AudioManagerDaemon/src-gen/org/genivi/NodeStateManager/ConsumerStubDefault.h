/*
* This file was generated by the CommonAPI Generators. 
* Used org.genivi.commonapi.core 2.1.2.201309301424.
* Used org.franca.core 0.8.9.201308271211.
*
* Copyright (C) 2012, BMW AG
* 
* This file is part of GENIVI Project AudioManager.
* 
* Contributions are licensed to the GENIVI Alliance under one or more
* Contribution License Agreements.
* 
* \copyright
*  This Source Code Form is subject to the terms of the
* Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
*  this file, You can obtain one at http://mozilla.org/MPL/2.0/.
* 
* 
* \author Aleksandar Donchev, aleksander.donchev@partner.bmw.de BMW 2013
* 
* For further information see http://www.genivi.org/.
*/
/**
 * @author Christian Linke
 */
#ifndef ORG_GENIVI_NODESTATEMANAGER_Consumer_STUB_DEFAULT_H_
#define ORG_GENIVI_NODESTATEMANAGER_Consumer_STUB_DEFAULT_H_

#include <org/genivi/NodeStateManager/ConsumerStub.h>
#include <sstream>

namespace org {
namespace genivi {
namespace NodeStateManager {

/**
 * Provides a default implementation for ConsumerStubRemoteEvent and
 * ConsumerStub. Method callbacks have an empty implementation,
 * remote set calls on attributes will always change the value of the attribute
 * to the one received.
 *
 * Override this stub if you only want to provide a subset of the functionality
 * that would be defined for this service, and/or if you do not need any non-default
 * behaviour.
 */
class ConsumerStubDefault : public ConsumerStub {
 public:
    ConsumerStubDefault();

    ConsumerStubRemoteEvent* initStubAdapter(const std::shared_ptr<ConsumerStubAdapter>& stubAdapter);

    virtual const int32_t& getBootModeAttribute();
    virtual const int32_t& getBootModeAttribute(const std::shared_ptr<CommonAPI::ClientId> clientId);
    virtual void setBootModeAttribute(int32_t value);
    virtual const int32_t& getRestartReasonAttribute();
    virtual const int32_t& getRestartReasonAttribute(const std::shared_ptr<CommonAPI::ClientId> clientId);
    virtual void setRestartReasonAttribute(int32_t value);
    virtual const int32_t& getShutdownReasonAttribute();
    virtual const int32_t& getShutdownReasonAttribute(const std::shared_ptr<CommonAPI::ClientId> clientId);
    virtual void setShutdownReasonAttribute(int32_t value);
    virtual const int32_t& getWakeUpReasonAttribute();
    virtual const int32_t& getWakeUpReasonAttribute(const std::shared_ptr<CommonAPI::ClientId> clientId);
    virtual void setWakeUpReasonAttribute(int32_t value);

    virtual void GetAppHealthCount(const std::shared_ptr<CommonAPI::ClientId> clientId, uint32_t& Count);
    virtual void GetAppHealthCount(uint32_t& Count);

    virtual void LifecycleRequestComplete(const std::shared_ptr<CommonAPI::ClientId> clientId, uint32_t RequestId, int32_t Status, int32_t& ErrorCode);
    virtual void LifecycleRequestComplete(uint32_t RequestId, int32_t Status, int32_t& ErrorCode);

    virtual void GetInterfaceVersion(const std::shared_ptr<CommonAPI::ClientId> clientId, uint32_t& Version);
    virtual void GetInterfaceVersion(uint32_t& Version);

    virtual void GetApplicationMode(const std::shared_ptr<CommonAPI::ClientId> clientId, int32_t& ErrorCode, int32_t& ApplicationModeId);
    virtual void GetApplicationMode(int32_t& ErrorCode, int32_t& ApplicationModeId);

    virtual void UnRegisterSession(const std::shared_ptr<CommonAPI::ClientId> clientId, std::string SessionName, std::string SessionOwner, int32_t SeatID, int32_t& ErrorCode);
    virtual void UnRegisterSession(std::string SessionName, std::string SessionOwner, int32_t SeatID, int32_t& ErrorCode);

    virtual void RegisterSession(const std::shared_ptr<CommonAPI::ClientId> clientId, std::string SessionName, std::string SessionOwner, int32_t SeatID, int32_t SessionState, int32_t& ErrorCode);
    virtual void RegisterSession(std::string SessionName, std::string SessionOwner, int32_t SeatID, int32_t SessionState, int32_t& ErrorCode);

    virtual void UnRegisterShutdownClient(const std::shared_ptr<CommonAPI::ClientId> clientId, std::string BusName, std::string ObjName, uint32_t ShutdownMode, int32_t& ErrorCode);
    virtual void UnRegisterShutdownClient(std::string BusName, std::string ObjName, uint32_t ShutdownMode, int32_t& ErrorCode);

    virtual void RegisterShutdownClient(const std::shared_ptr<CommonAPI::ClientId> clientId, std::string BusName, std::string ObjName, uint32_t ShutdownMode, uint32_t TimeoutMs, int32_t& ErrorCode);
    virtual void RegisterShutdownClient(std::string BusName, std::string ObjName, uint32_t ShutdownMode, uint32_t TimeoutMs, int32_t& ErrorCode);

    virtual void GetNodeState(const std::shared_ptr<CommonAPI::ClientId> clientId, int32_t& ErrorCode, int32_t& NodeStateId);
    virtual void GetNodeState(int32_t& ErrorCode, int32_t& NodeStateId);

    virtual void GetSessionState(const std::shared_ptr<CommonAPI::ClientId> clientId, std::string SessionName, int32_t SeatID, int32_t& SessionState, int32_t& ErrorCode);
    virtual void GetSessionState(std::string SessionName, int32_t SeatID, int32_t& SessionState, int32_t& ErrorCode);

    virtual void SetSessionState(const std::shared_ptr<CommonAPI::ClientId> clientId, std::string SessionName, std::string SessionOwner, int32_t SessionState, int32_t SeatID, int32_t& ErrorCode);
    virtual void SetSessionState(std::string SessionName, std::string SessionOwner, int32_t SessionState, int32_t SeatID, int32_t& ErrorCode);


    virtual void fireNodeApplicationModeEvent(const int32_t& ApplicationModeId);
    virtual void fireSessionStateChangedEvent(const std::string& SessionStateName, const int32_t& SeatID, const int32_t& SessionState);
    virtual void fireNodeStateEvent(const int32_t& NodeState);
    

 protected:
    virtual bool trySetBootModeAttribute(int32_t value);
    virtual bool validateBootModeAttributeRequestedValue(const int32_t& value);
    virtual bool trySetRestartReasonAttribute(int32_t value);
    virtual bool validateRestartReasonAttributeRequestedValue(const int32_t& value);
    virtual bool trySetShutdownReasonAttribute(int32_t value);
    virtual bool validateShutdownReasonAttributeRequestedValue(const int32_t& value);
    virtual bool trySetWakeUpReasonAttribute(int32_t value);
    virtual bool validateWakeUpReasonAttributeRequestedValue(const int32_t& value);
    std::shared_ptr<ConsumerStubAdapter> stubAdapter_;
 private:
    class RemoteEventHandler: public ConsumerStubRemoteEvent {
     public:
        RemoteEventHandler(ConsumerStubDefault* defaultStub);






     private:
        ConsumerStubDefault* defaultStub_;
    };

    RemoteEventHandler remoteEventHandler_;

    int32_t bootModeAttributeValue_;
    int32_t restartReasonAttributeValue_;
    int32_t shutdownReasonAttributeValue_;
    int32_t wakeUpReasonAttributeValue_;
};

} // namespace NodeStateManager
} // namespace genivi
} // namespace org

#endif // ORG_GENIVI_NODESTATEMANAGER_Consumer_STUB_DEFAULT_H_
