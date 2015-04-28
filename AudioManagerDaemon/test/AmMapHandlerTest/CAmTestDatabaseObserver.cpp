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

#include "CAmDatabaseObserver.h"
#include "MockDatabaseObserver.h"

namespace am {

CAmDatabaseObserver::CAmDatabaseObserver(CAmCommandSender *iCommandSender, CAmRoutingSender *iRoutingSender, CAmSocketHandler *iSocketHandler) :
mCommandSender(iCommandSender), //
mRoutingSender(iRoutingSender), //
mTelnetServer(NULL), //
mSerializer(iSocketHandler) //
{}

CAmDatabaseObserver::~CAmDatabaseObserver() {}


void CAmDatabaseObserver::numberOfSinkClassesChanged()
{ MockDatabaseObserver::getMockObserverObject()->numberOfSinkClassesChanged(); }
void CAmDatabaseObserver::numberOfSourceClassesChanged()
{ MockDatabaseObserver::getMockObserverObject()->numberOfSourceClassesChanged(); }
void CAmDatabaseObserver::newSink(const am_Sink_s& sink)
{ MockDatabaseObserver::getMockObserverObject()->newSink(sink); }
void CAmDatabaseObserver::newSource(const am_Source_s& source)
{ MockDatabaseObserver::getMockObserverObject()->newSource(source); }
void CAmDatabaseObserver::newDomain(const am_Domain_s& domain)
{ MockDatabaseObserver::getMockObserverObject()->newDomain(domain); }
void CAmDatabaseObserver::newGateway(const am_Gateway_s& gateway)
{ MockDatabaseObserver::getMockObserverObject()->newGateway(gateway); }
void CAmDatabaseObserver::newConverter(const am_Converter_s& coverter)
{ MockDatabaseObserver::getMockObserverObject()->newConverter(coverter); }
void CAmDatabaseObserver::newCrossfader(const am_Crossfader_s& crossfader)
{ MockDatabaseObserver::getMockObserverObject()->newCrossfader(crossfader); }
void CAmDatabaseObserver::newMainConnection(const am_MainConnectionType_s& mainConnection)
{ MockDatabaseObserver::getMockObserverObject()->newMainConnection(mainConnection); }
void CAmDatabaseObserver::removedMainConnection(const am_mainConnectionID_t mainConnection)
{ MockDatabaseObserver::getMockObserverObject()->removedMainConnection(mainConnection); }
void CAmDatabaseObserver::removedSink(const am_sinkID_t sinkID, const bool visible)
{ MockDatabaseObserver::getMockObserverObject()->removedSink(sinkID, visible); }
void CAmDatabaseObserver::removedSource(const am_sourceID_t sourceID, const bool visible)
{ MockDatabaseObserver::getMockObserverObject()->removedSource(sourceID, visible); }
void CAmDatabaseObserver::removeDomain(const am_domainID_t domainID)
{ MockDatabaseObserver::getMockObserverObject()->removeDomain(domainID); }
void CAmDatabaseObserver::removeGateway(const am_gatewayID_t gatewayID)
{ MockDatabaseObserver::getMockObserverObject()->removeGateway(gatewayID); }
void CAmDatabaseObserver::removeConverter(const am_converterID_t converterID)
{ MockDatabaseObserver::getMockObserverObject()->removeConverter(converterID); }
void CAmDatabaseObserver::removeCrossfader(const am_crossfaderID_t crossfaderID)
{ MockDatabaseObserver::getMockObserverObject()->removeCrossfader(crossfaderID); }
void CAmDatabaseObserver::mainConnectionStateChanged(const am_mainConnectionID_t connectionID, const am_ConnectionState_e connectionState)
{ MockDatabaseObserver::getMockObserverObject()->mainConnectionStateChanged(connectionID, connectionState); }
void CAmDatabaseObserver::mainSinkSoundPropertyChanged(const am_sinkID_t sinkID, const am_MainSoundProperty_s& SoundProperty)
{ MockDatabaseObserver::getMockObserverObject()->mainSinkSoundPropertyChanged(sinkID, SoundProperty); }
void CAmDatabaseObserver::mainSourceSoundPropertyChanged(const am_sourceID_t sourceID, const am_MainSoundProperty_s& SoundProperty)
{ MockDatabaseObserver::getMockObserverObject()->mainSourceSoundPropertyChanged(sourceID, SoundProperty); }
void CAmDatabaseObserver::sinkAvailabilityChanged(const am_sinkID_t sinkID, const am_Availability_s& availability)
{ MockDatabaseObserver::getMockObserverObject()->sinkAvailabilityChanged(sinkID, availability); }
void CAmDatabaseObserver::sourceAvailabilityChanged(const am_sourceID_t sourceID, const am_Availability_s& availability)
{ MockDatabaseObserver::getMockObserverObject()->sourceAvailabilityChanged(sourceID, availability); }
void CAmDatabaseObserver::volumeChanged(const am_sinkID_t sinkID, const am_mainVolume_t volume)
{ MockDatabaseObserver::getMockObserverObject()->volumeChanged(sinkID, volume); }
void CAmDatabaseObserver::sinkMuteStateChanged(const am_sinkID_t sinkID, const am_MuteState_e muteState)
{ MockDatabaseObserver::getMockObserverObject()->sinkMuteStateChanged(sinkID, muteState); }
void CAmDatabaseObserver::systemPropertyChanged(const am_SystemProperty_s& SystemProperty)
{ MockDatabaseObserver::getMockObserverObject()->systemPropertyChanged(SystemProperty); }
void CAmDatabaseObserver::timingInformationChanged(const am_mainConnectionID_t mainConnection, const am_timeSync_t time)
{ MockDatabaseObserver::getMockObserverObject()->timingInformationChanged(mainConnection,time); }
void CAmDatabaseObserver::sinkUpdated(const am_sinkID_t sinkID, const am_sinkClass_t sinkClassID, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties, const bool visible)
{ MockDatabaseObserver::getMockObserverObject()->sinkUpdated(sinkID,sinkClassID,listMainSoundProperties, visible); }
void CAmDatabaseObserver::sourceUpdated(const am_sourceID_t sourceID, const am_sourceClass_t sourceClassID, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties, const bool visible)
{ MockDatabaseObserver::getMockObserverObject()->sourceUpdated(sourceID,sourceClassID,listMainSoundProperties, visible); }
void CAmDatabaseObserver::sinkMainNotificationConfigurationChanged(const am_sinkID_t sinkID, const am_NotificationConfiguration_s mainNotificationConfiguration)
{ MockDatabaseObserver::getMockObserverObject()->sinkMainNotificationConfigurationChanged(sinkID,mainNotificationConfiguration); }
void CAmDatabaseObserver::sourceMainNotificationConfigurationChanged(const am_sourceID_t sourceID, const am_NotificationConfiguration_s mainNotificationConfiguration)
{ MockDatabaseObserver::getMockObserverObject()->sourceMainNotificationConfigurationChanged(sourceID,mainNotificationConfiguration); }
}

