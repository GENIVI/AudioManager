/*
 * mocklnterfaces.h
 *
 *  Created on: Dec 27, 2011
 *      Author: christian
 */

#ifndef MOCKLNTERFACES_H_
#define MOCKLNTERFACES_H_

#include <routing/RoutingReceiveInterface.h>

namespace am {

class MockRoutingReceiveInterface : public RoutingReceiveInterface {
 public:
  MOCK_METHOD3(ackConnect,
      void(const am_Handle_s handle, const am_connectionID_t connectionID, const am_Error_e error));
  MOCK_METHOD3(ackDisconnect,
      void(const am_Handle_s handle, const am_connectionID_t connectionID, const am_Error_e error));
  MOCK_METHOD3(ackSetSinkVolumeChange,
      void(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error));
  MOCK_METHOD3(ackSetSourceVolumeChange,
      void(const am_Handle_s handle, const am_volume_t volume, const am_Error_e error));
  MOCK_METHOD2(ackSetSourceState,
      void(const am_Handle_s handle, const am_Error_e error));
  MOCK_METHOD2(ackSetSinkSoundProperty,
      void(const am_Handle_s handle, const am_Error_e error));
  MOCK_METHOD2(ackSetSourceSoundProperty,
      void(const am_Handle_s handle, const am_Error_e error));
  MOCK_METHOD3(ackCrossFading,
      void(const am_Handle_s handle, const am_HotSink_e hotSink, const am_Error_e error));
  MOCK_METHOD3(ackSourceVolumeTick,
      void(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume));
  MOCK_METHOD3(ackSinkVolumeTick,
      void(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume));
  MOCK_METHOD2(peekDomain,
      am_Error_e(const std::string& name, am_domainID_t& domainID));
  MOCK_METHOD2(registerDomain,
      am_Error_e(const am_Domain_s& domainData, am_domainID_t& domainID));
  MOCK_METHOD1(deregisterDomain,
      am_Error_e(const am_domainID_t domainID));
  MOCK_METHOD2(registerGateway,
      am_Error_e(const am_Gateway_s& gatewayData, am_gatewayID_t& gatewayID));
  MOCK_METHOD1(deregisterGateway,
      am_Error_e(const am_gatewayID_t gatewayID));
  MOCK_METHOD2(peekSink,
      am_Error_e(const std::string& name, am_sinkID_t& sinkID));
  MOCK_METHOD2(registerSink,
      am_Error_e(const am_Sink_s& sinkData, am_sinkID_t& sinkID));
  MOCK_METHOD1(deregisterSink,
      am_Error_e(const am_sinkID_t sinkID));
  MOCK_METHOD2(peekSource,
      am_Error_e(const std::string& name, am_sourceID_t& sourceID));
  MOCK_METHOD2(registerSource,
      am_Error_e(const am_Source_s& sourceData, am_sourceID_t& sourceID));
  MOCK_METHOD1(deregisterSource,
      am_Error_e(const am_sourceID_t sourceID));
  MOCK_METHOD2(registerCrossfader,
      am_Error_e(const am_Crossfader_s& crossfaderData, am_crossfaderID_t& crossfaderID));
  MOCK_METHOD1(deregisterCrossfader,
      am_Error_e(const am_crossfaderID_t crossfaderID));
  MOCK_METHOD2(peekSourceClassID,
      am_Error_e(const std::string name, const am_sourceClass_t& sourceClassID));
  MOCK_METHOD2(peekSinkClassID,
      am_Error_e(const std::string name, const am_sinkClass_t& sinkClassID));
  MOCK_METHOD2(hookInterruptStatusChange,
      void(const am_sourceID_t sourceID, const am_InterruptState_e interruptState));
  MOCK_METHOD1(hookDomainRegistrationComplete,
      void(const am_domainID_t domainID));
  MOCK_METHOD2(hookSinkAvailablityStatusChange,
      void(const am_sinkID_t sinkID, const am_Availability_s& availability));
  MOCK_METHOD2(hookSourceAvailablityStatusChange,
      void(const am_sourceID_t sourceID, const am_Availability_s& availability));
  MOCK_METHOD2(hookDomainStateChange,
      void(const am_domainID_t domainID, const am_DomainState_e domainState));
  MOCK_METHOD2(hookTimingInformationChanged,
      void(const am_connectionID_t connectionID, const am_timeSync_t delay));
  MOCK_METHOD1(sendChangedData,
      am_Error_e(const std::vector<am_EarlyData_s>& earlyData));
  MOCK_CONST_METHOD1(getDBusConnectionWrapper,
      am_Error_e(DBusWrapper*& dbusConnectionWrapper));
  MOCK_CONST_METHOD1(getSocketHandler,
      am_Error_e(SocketHandler*& socketHandler));
};

}  // namespace am



#endif /* MOCKLNTERFACES_H_ */
