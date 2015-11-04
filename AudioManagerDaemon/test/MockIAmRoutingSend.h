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
 * \author Christian Linke, christian.linke@bmw.de BMW 2011,2012
 *
 * For further information see http://www.genivi.org/.
 *
 */

#ifndef MOCKROUTINGINTERFACE_H_
#define MOCKROUTINGINTERFACE_H_

#include "IAmRouting.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"

namespace am {

class MockIAmRoutingSend : public IAmRoutingSend {
 public:
    MOCK_CONST_METHOD1(getInterfaceVersion,
        void(std::string& version));
    MOCK_METHOD1(startupInterface,
        am_Error_e(IAmRoutingReceive* routingreceiveinterface));
    MOCK_METHOD1(setRoutingReady,
        void(const uint16_t handle));
    MOCK_METHOD1(setRoutingRundown,
        void(const uint16_t handle));
    MOCK_METHOD1(asyncAbort,
        am_Error_e(const am_Handle_s handle));
    MOCK_METHOD5(asyncConnect,
        am_Error_e(const am_Handle_s handle, const am_connectionID_t connectionID, const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_CustomAvailabilityReason_t connectionFormat));
    MOCK_METHOD2(asyncDisconnect,
        am_Error_e(const am_Handle_s handle, const am_connectionID_t connectionID));
    MOCK_METHOD5(asyncSetSinkVolume,
        am_Error_e(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_CustomRampType_t ramp, const am_time_t time));
    MOCK_METHOD5(asyncSetSourceVolume,
        am_Error_e(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume, const am_CustomRampType_t ramp, const am_time_t time));
    MOCK_METHOD3(asyncSetSourceState,
        am_Error_e(const am_Handle_s handle, const am_sourceID_t sourceID, const am_SourceState_e state));
    MOCK_METHOD3(asyncSetSinkSoundProperties,
        am_Error_e(const am_Handle_s handle, const am_sinkID_t sinkID, const std::vector<am_SoundProperty_s>& listSoundProperties));
    MOCK_METHOD3(asyncSetSinkSoundProperty,
        am_Error_e(const am_Handle_s handle, const am_sinkID_t sinkID, const am_SoundProperty_s& soundProperty));
    MOCK_METHOD3(asyncSetSourceSoundProperties,
        am_Error_e(const am_Handle_s handle, const am_sourceID_t sourceID, const std::vector<am_SoundProperty_s>& listSoundProperties));
    MOCK_METHOD3(asyncSetSourceSoundProperty,
        am_Error_e(const am_Handle_s handle, const am_sourceID_t sourceID, const am_SoundProperty_s& soundProperty));
    MOCK_METHOD5(asyncCrossFade,
        am_Error_e(const am_Handle_s handle, const am_crossfaderID_t crossfaderID, const am_HotSink_e hotSink, const am_CustomRampType_t rampType, const am_time_t time));
    MOCK_METHOD2(setDomainState,
        am_Error_e(const am_domainID_t domainID, const am_DomainState_e domainState));
    MOCK_CONST_METHOD1(returnBusName,
        am_Error_e(std::string& BusName));
    MOCK_METHOD2(asyncSetVolumes,
        am_Error_e(const am_Handle_s handle, const std::vector<am_Volumes_s>& listVolumes));
    MOCK_METHOD3(asyncSetSinkNotificationConfiguration,
        am_Error_e(const am_Handle_s handle, const am_sinkID_t sinkID, const am_NotificationConfiguration_s& notificationConfiguration));
    MOCK_METHOD3(asyncSetSourceNotificationConfiguration,
        am_Error_e(const am_Handle_s handle, const am_sourceID_t sourceID, const am_NotificationConfiguration_s& notificationConfiguration));
    MOCK_METHOD2(resyncConnectionState,
        am_Error_e(const am_domainID_t domainID, std::vector<am_Connection_s>& listOfExistingConnections));


};

}  // namespace am


#endif /* MOCKROUTINGINTERFACE_H_ */
