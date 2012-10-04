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
 * \author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
 *
 * For further information see http://www.genivi.org/.
 *
 */

#ifndef MOCKCOMMANDINTERFACE_H_
#define MOCKCOMMANDINTERFACE_H_

#include "command/IAmCommandSend.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace am {

class MockIAmCommandSend : public IAmCommandSend {
 public:
  MOCK_METHOD1(startupInterface,
      am_Error_e(IAmCommandReceive* commandreceiveinterface));
  MOCK_METHOD1(setCommandReady,
      void(const uint16_t handle));
  MOCK_METHOD1(setCommandRundown,
      void(const uint16_t handle));
  MOCK_METHOD1(cbNewMainConnection,
      void(const am_MainConnectionType_s& mainConnection));
  MOCK_METHOD1(cbRemovedMainConnection,
      void(const am_mainConnectionID_t mainConnection));
  MOCK_METHOD1(cbNewSink,
      void(const am_SinkType_s& sink));
  MOCK_METHOD1(cbRemovedSink,
      void(const am_sinkID_t sinkID));
  MOCK_METHOD1(cbNewSource,
      void(const am_SourceType_s& source));
  MOCK_METHOD1(cbRemovedSource,
      void(const am_sourceID_t source));
  MOCK_METHOD0(cbNumberOfSinkClassesChanged,
      void());
  MOCK_METHOD0(cbNumberOfSourceClassesChanged,
      void());
  MOCK_METHOD2(cbMainConnectionStateChanged,
      void(const am_mainConnectionID_t connectionID, const am_ConnectionState_e connectionState));
  MOCK_METHOD2(cbMainSinkSoundPropertyChanged,
      void(const am_sinkID_t sinkID, const am_MainSoundProperty_s& soundProperty));
  MOCK_METHOD2(cbMainSourceSoundPropertyChanged,
      void(const am_sourceID_t sourceID, const am_MainSoundProperty_s& soundProperty));
  MOCK_METHOD2(cbSinkAvailabilityChanged,
      void(const am_sinkID_t sinkID, const am_Availability_s& availability));
  MOCK_METHOD2(cbSourceAvailabilityChanged,
      void(const am_sourceID_t sourceID, const am_Availability_s& availability));
  MOCK_METHOD2(cbVolumeChanged,
      void(const am_sinkID_t sinkID, const am_mainVolume_t volume));
  MOCK_METHOD2(cbSinkMuteStateChanged,
      void(const am_sinkID_t sinkID, const am_MuteState_e muteState));
  MOCK_METHOD1(cbSystemPropertyChanged,
      void(const am_SystemProperty_s& systemProperty));
  MOCK_METHOD2(cbTimingInformationChanged,
      void(const am_mainConnectionID_t mainConnectionID, const am_timeSync_t time));
  MOCK_CONST_METHOD1(getInterfaceVersion,
      void(std::string& version));
};

}  // namespace am
#endif /* MOCKCOMMANDINTERFACE_H_ */
