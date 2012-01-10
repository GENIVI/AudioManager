/**
* Copyright (C) 2011, BMW AG
*
* GeniviAudioMananger AudioManagerDaemon
*
* \file mockInterfaces.h
*
* \date 20-Oct-2011 3:42:04 PM
* \author Christian Mueller (christian.ei.mueller@bmw.de)
*
* \section License
* GNU Lesser General Public License, version 2.1, with special exception (GENIVI clause)
* Copyright (C) 2011, BMW AG Christian Mueller  Christian.ei.mueller@bmw.de
*
* This program is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License, version 2.1, for more details.
* You should have received a copy of the GNU Lesser General Public License, version 2.1, along with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
* Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may also be applicable to programs even in cases in which the program is not a library in the technical sense.
* Linking AudioManager statically or dynamically with other modules is making a combined work based on AudioManager. You may license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to license your linked modules under the GNU Lesser General Public License, version 2.1, you may use the program under the following exception.
* As a special exception, the copyright holders of AudioManager give you permission to combine AudioManager with software programs or libraries that are released under any license unless such a combination is not permitted by the license of such a software program or library. You may copy and distribute such a system following the terms of the GNU Lesser General Public License, version 2.1, including this special exception, for AudioManager and the licenses of the other code concerned.
* Note that people who make modified versions of AudioManager are not obligated to grant this special exception for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, version 2.1, gives permission to release a modified version without this exception; this exception also makes it possible to release a modified version which carries forward this exception.
*
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
  MOCK_METHOD2(ackSetSinkSoundProperties,
      void(const am_Handle_s handle, const am_Error_e error));
  MOCK_METHOD2(ackSetSinkSoundProperty,
      void(const am_Handle_s handle, const am_Error_e error));
  MOCK_METHOD2(ackSetSourceSoundProperties,
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
  MOCK_CONST_METHOD0(getInterfaceVersion,
      uint16_t());
};

}  // namespace am



#endif /* MOCKLNTERFACES_H_ */
