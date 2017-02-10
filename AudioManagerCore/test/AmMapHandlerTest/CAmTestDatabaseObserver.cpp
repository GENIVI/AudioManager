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
 * \file CAmTestDatabaseObserver.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include "CAmTestDatabaseObserver.h"
#include "MockDatabaseObserver.h"

namespace am {

CAmDatabaseObserver::CAmDatabaseObserver()
{
		dboNumberOfSinkClassesChanged = [&]()
		{ MockDatabaseObserver::getMockObserverObject()->numberOfSinkClassesChanged(); };
		dboNumberOfSourceClassesChanged = [&]()
		{ MockDatabaseObserver::getMockObserverObject()->numberOfSourceClassesChanged(); };
		dboNewSink = [&](const am_Sink_s& sink)
		{ MockDatabaseObserver::getMockObserverObject()->newSink(sink); };
		dboNewSource = [&](const am_Source_s& source)
		{ MockDatabaseObserver::getMockObserverObject()->newSource(source); };
		dboNewDomain = [&](const am_Domain_s& domain)
		{ MockDatabaseObserver::getMockObserverObject()->newDomain(domain); };
		dboNewGateway = [&](const am_Gateway_s& gateway)
		{ MockDatabaseObserver::getMockObserverObject()->newGateway(gateway); };
		dboNewConverter = [&](const am_Converter_s& coverter)
		{ MockDatabaseObserver::getMockObserverObject()->newConverter(coverter); };
		dboNewCrossfader = [&](const am_Crossfader_s& crossfader)
		{ MockDatabaseObserver::getMockObserverObject()->newCrossfader(crossfader); };
		dboNewMainConnection = [&](const am_MainConnectionType_s& mainConnection)
		{ MockDatabaseObserver::getMockObserverObject()->newMainConnection(mainConnection); };
		dboRemovedMainConnection = [&](const am_mainConnectionID_t mainConnection)
		{ MockDatabaseObserver::getMockObserverObject()->removedMainConnection(mainConnection); };
		dboRemovedSink = [&](const am_sinkID_t sinkID, const bool visible)
		{ MockDatabaseObserver::getMockObserverObject()->removedSink(sinkID, visible); };
		dboRemovedSource = [&](const am_sourceID_t sourceID, const bool visible)
		{ MockDatabaseObserver::getMockObserverObject()->removedSource(sourceID, visible); };
		dboRemoveDomain = [&](const am_domainID_t domainID)
		{ MockDatabaseObserver::getMockObserverObject()->removeDomain(domainID); };
		dboRemoveGateway = [&](const am_gatewayID_t gatewayID)
		{ MockDatabaseObserver::getMockObserverObject()->removeGateway(gatewayID); };
		dboRemoveConverter = [&](const am_converterID_t converterID)
		{ MockDatabaseObserver::getMockObserverObject()->removeConverter(converterID); };
		dboRemoveCrossfader = [&](const am_crossfaderID_t crossfaderID)
		{ MockDatabaseObserver::getMockObserverObject()->removeCrossfader(crossfaderID); };
		dboMainConnectionStateChanged = [&](const am_mainConnectionID_t connectionID, const am_ConnectionState_e connectionState)
		{ MockDatabaseObserver::getMockObserverObject()->mainConnectionStateChanged(connectionID, connectionState); };
		dboMainSinkSoundPropertyChanged = [&](const am_sinkID_t sinkID, const am_MainSoundProperty_s& SoundProperty)
		{ MockDatabaseObserver::getMockObserverObject()->mainSinkSoundPropertyChanged(sinkID, SoundProperty); };
		dboMainSourceSoundPropertyChanged = [&](const am_sourceID_t sourceID, const am_MainSoundProperty_s& SoundProperty)
		{ MockDatabaseObserver::getMockObserverObject()->mainSourceSoundPropertyChanged(sourceID, SoundProperty); };
		dboSinkAvailabilityChanged = [&](const am_sinkID_t sinkID, const am_Availability_s& availability)
		{ MockDatabaseObserver::getMockObserverObject()->sinkAvailabilityChanged(sinkID, availability); };
		dboSourceAvailabilityChanged = [&](const am_sourceID_t sourceID, const am_Availability_s& availability)
		{ MockDatabaseObserver::getMockObserverObject()->sourceAvailabilityChanged(sourceID, availability); };
		dboVolumeChanged = [&](const am_sinkID_t sinkID, const am_mainVolume_t volume)
		{ MockDatabaseObserver::getMockObserverObject()->volumeChanged(sinkID, volume); };
		dboSinkMuteStateChanged = [&](const am_sinkID_t sinkID, const am_MuteState_e muteState)
		{ MockDatabaseObserver::getMockObserverObject()->sinkMuteStateChanged(sinkID, muteState); };
		dboSystemPropertyChanged = [&](const am_SystemProperty_s& SystemProperty)
		{ MockDatabaseObserver::getMockObserverObject()->systemPropertyChanged(SystemProperty); };
		dboTimingInformationChanged = [&](const am_mainConnectionID_t mainConnection, const am_timeSync_t time)
		{ MockDatabaseObserver::getMockObserverObject()->timingInformationChanged(mainConnection,time); };
		dboSinkUpdated = [&](const am_sinkID_t sinkID, const am_sinkClass_t sinkClassID, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties, const bool visible)
		{ MockDatabaseObserver::getMockObserverObject()->sinkUpdated(sinkID,sinkClassID,listMainSoundProperties, visible); };
		dboSourceUpdated = [&](const am_sourceID_t sourceID, const am_sourceClass_t sourceClassID, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties, const bool visible)
		{ MockDatabaseObserver::getMockObserverObject()->sourceUpdated(sourceID,sourceClassID,listMainSoundProperties, visible); };
		dboSinkMainNotificationConfigurationChanged = [&](const am_sinkID_t sinkID, const am_NotificationConfiguration_s mainNotificationConfiguration)
		{ MockDatabaseObserver::getMockObserverObject()->sinkMainNotificationConfigurationChanged(sinkID,mainNotificationConfiguration); };
		dboSourceMainNotificationConfigurationChanged = [&](const am_sourceID_t sourceID, const am_NotificationConfiguration_s mainNotificationConfiguration)
		{ MockDatabaseObserver::getMockObserverObject()->sourceMainNotificationConfigurationChanged(sourceID,mainNotificationConfiguration); };

}

CAmDatabaseObserver::~CAmDatabaseObserver() {}

}

