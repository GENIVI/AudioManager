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
 * \file MockDatabaseObserver.h
 * For further information see http://www.genivi.org/.
 *
 */


#ifndef MOCKDATABASEOBSERVER_H_
#define MOCKDATABASEOBSERVER_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "CAmTestDatabaseObserver.h"

class CAmCommandSender;
class CAmRoutingSender;
class CAmSocketHandler;
class CAmTelnetServer;

namespace am {
using namespace testing;

class IAmDatabaseObserver
{
public:
	IAmDatabaseObserver() {};
    virtual ~IAmDatabaseObserver() {};

    virtual void numberOfSinkClassesChanged() = 0;
    virtual void numberOfSourceClassesChanged() = 0;
    virtual void newSink(const am_Sink_s& sink) = 0;
    virtual void newSource(const am_Source_s& source) = 0;
    virtual void newDomain(const am_Domain_s& domain) = 0;
    virtual void newGateway(const am_Gateway_s& gateway) = 0;
    virtual void newConverter(const am_Converter_s& coverter) = 0;
    virtual void newCrossfader(const am_Crossfader_s& crossfader) = 0;
    virtual void newMainConnection(const am_MainConnectionType_s& mainConnection) = 0;
    virtual void removedMainConnection(const am_mainConnectionID_t mainConnection) = 0;
    virtual void removedSink(const am_sinkID_t sinkID, const bool visible) = 0;
    virtual void removedSource(const am_sourceID_t sourceID, const bool visible) = 0;
    virtual void removeDomain(const am_domainID_t domainID) = 0;
    virtual void removeGateway(const am_gatewayID_t gatewayID) = 0;
    virtual void removeConverter(const am_converterID_t converterID) = 0;
    virtual void removeCrossfader(const am_crossfaderID_t crossfaderID) = 0;
    virtual void mainConnectionStateChanged(const am_mainConnectionID_t connectionID, const am_ConnectionState_e connectionState) = 0;
    virtual void mainSinkSoundPropertyChanged(const am_sinkID_t sinkID, const am_MainSoundProperty_s& SoundProperty) = 0;
    virtual void mainSourceSoundPropertyChanged(const am_sourceID_t sourceID, const am_MainSoundProperty_s& SoundProperty) = 0;
    virtual void sinkAvailabilityChanged(const am_sinkID_t sinkID, const am_Availability_s& availability) = 0;
    virtual void sourceAvailabilityChanged(const am_sourceID_t sourceID, const am_Availability_s& availability) = 0;
    virtual void volumeChanged(const am_sinkID_t sinkID, const am_mainVolume_t volume) = 0;
    virtual void sinkMuteStateChanged(const am_sinkID_t sinkID, const am_MuteState_e muteState) = 0;
    virtual void systemPropertyChanged(const am_SystemProperty_s& SystemProperty) = 0;
    virtual void timingInformationChanged(const am_mainConnectionID_t mainConnection, const am_timeSync_t time) = 0;
    virtual void sinkUpdated(const am_sinkID_t sinkID, const am_sinkClass_t sinkClassID, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties, const bool visible) = 0;
    virtual void sourceUpdated(const am_sourceID_t sourceID, const am_sourceClass_t sourceClassID, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties, const bool visible) = 0;
    virtual void sinkMainNotificationConfigurationChanged(const am_sinkID_t sinkID, const am_NotificationConfiguration_s mainNotificationConfiguration) = 0;
    virtual void sourceMainNotificationConfigurationChanged(const am_sourceID_t sourceID, const am_NotificationConfiguration_s mainNotificationConfiguration) = 0;

};

class MockDatabaseObserver : public IAmDatabaseObserver{
 public:
	MOCK_METHOD0(numberOfSinkClassesChanged, void());
	MOCK_METHOD0(numberOfSourceClassesChanged, void());
	MOCK_METHOD1(newSink, void(const am_Sink_s& sink));
	MOCK_METHOD1(newSource, void(const am_Source_s& source));
	MOCK_METHOD1(newDomain, void(const am_Domain_s& domain));
	MOCK_METHOD1(newGateway, void(const am_Gateway_s& gateway));
	MOCK_METHOD1(newConverter, void(const am_Converter_s& converter));
	MOCK_METHOD1(newCrossfader, void(const am_Crossfader_s& crossfader));
	MOCK_METHOD1(newMainConnection, void(const am_MainConnectionType_s & mainConnection));
	MOCK_METHOD1(removedMainConnection, void(const am_mainConnectionID_t mainConnection));
	MOCK_METHOD2(removedSink, void(const am_sinkID_t sinkID, const bool visible));
	MOCK_METHOD2(removedSource, void(const am_sourceID_t sourceID, const bool visible));
	MOCK_METHOD1(removeDomain, void(const am_domainID_t domainID));
	MOCK_METHOD1(removeGateway, void(const am_gatewayID_t gatewayID));
	MOCK_METHOD1(removeConverter, void(const am_converterID_t converterID));
	MOCK_METHOD1(removeCrossfader, void(const am_crossfaderID_t crossfaderID));
	MOCK_METHOD2(mainConnectionStateChanged, void(const am_mainConnectionID_t connectionID, const am_ConnectionState_e connectionState));
	MOCK_METHOD2(mainSinkSoundPropertyChanged, void(const am_sinkID_t sinkID, const am_MainSoundProperty_s& SoundProperty));
	MOCK_METHOD2(mainSourceSoundPropertyChanged, void(const am_sourceID_t sourceID, const am_MainSoundProperty_s& SoundProperty));
	MOCK_METHOD2(sinkAvailabilityChanged, void(const am_sinkID_t sinkID, const am_Availability_s& availability));
	MOCK_METHOD2(sourceAvailabilityChanged, void(const am_sourceID_t sourceID, const am_Availability_s& availability));
	MOCK_METHOD2(volumeChanged, void(const am_sinkID_t sinkID, const am_mainVolume_t volume));
	MOCK_METHOD2(sinkMuteStateChanged, void(const am_sinkID_t sinkID, const am_MuteState_e muteState));
	MOCK_METHOD1(systemPropertyChanged, void(const am_SystemProperty_s& SystemProperty));
	MOCK_METHOD2(timingInformationChanged, void(const am_mainConnectionID_t mainConnection, const am_timeSync_t time));
	MOCK_METHOD4(sinkUpdated, void(const am_sinkID_t sinkID, const am_sinkClass_t sinkClassID, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties, const bool visible));
	MOCK_METHOD4(sourceUpdated, void(const am_sourceID_t sourceID, const am_sourceClass_t sourceClassID, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties, const bool visible));
	MOCK_METHOD2(sinkMainNotificationConfigurationChanged, void(const am_sinkID_t sinkID, const am_NotificationConfiguration_s mainNotificationConfiguration));
	MOCK_METHOD2(sourceMainNotificationConfigurationChanged, void(const am_sourceID_t sourceID, const am_NotificationConfiguration_s mainNotificationConfiguration));

	static MockDatabaseObserver *getMockObserverObject()
	{
		static MockDatabaseObserver glMockObserverObject;
		return &glMockObserverObject;
	}
};



}  // namespace am
#endif /* MOCKDATABASEOBSERVER_H_ */
