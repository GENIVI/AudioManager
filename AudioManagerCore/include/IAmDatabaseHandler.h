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
 * \file IAmDatabaseHandler.h
 * For further information see http://www.genivi.org/.
 *
 */

#ifndef DATABASEHANDLERINTERFACE_H_
#define DATABASEHANDLERINTERFACE_H_

#include "audiomanagertypes.h"
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <functional>
#include <memory>
#include <assert.h>
#include <algorithm>
#include "audiomanagerconfig.h"

namespace am
{

//todo: check the enum values before entering & changing in the database.
//todo: change asserts for dynamic boundary checks into failure answers.#
//todo: check autoincrement boundary and set to 16bit limits
//todo: If the sink is part of a gateway, the listconnectionFormats is copied to the gatewayInformation. Check this statement for sinks & sources
//todo: exchange last_insert_row id to be more safe
//todo: create test to ensure uniqueness of names throughout the database
//todo: enforce the uniqueness of names

typedef std::map<am_gatewayID_t, std::vector<bool> > ListConnectionFormat; //!< type for list of connection formats

/**
 * This class handles and abstracts the database
 */

class IAmDatabaseHandler
{
public:
    IAmDatabaseHandler (){}
    virtual ~IAmDatabaseHandler (){}

    /**
      * The following interface methods must be implemented by the subclass.
      */
    virtual am_Error_e enterDomainDB(const am_Domain_s& domainData, am_domainID_t& domainID) = 0;
    virtual am_Error_e enterMainConnectionDB(const am_MainConnection_s& mainConnectionData, am_mainConnectionID_t& connectionID) = 0;
    virtual am_Error_e enterSinkDB(const am_Sink_s& sinkData, am_sinkID_t& sinkID) = 0;
    virtual am_Error_e enterCrossfaderDB(const am_Crossfader_s& crossfaderData, am_crossfaderID_t& crossfaderID) = 0;
    virtual am_Error_e enterGatewayDB(const am_Gateway_s& gatewayData, am_gatewayID_t& gatewayID) = 0;
    virtual am_Error_e enterConverterDB(const am_Converter_s & converteData, am_converterID_t & converterID) = 0;
    virtual am_Error_e enterSourceDB(const am_Source_s& sourceData, am_sourceID_t& sourceID) = 0;
    virtual am_Error_e enterConnectionDB(const am_Connection_s& connection, am_connectionID_t& connectionID) = 0;
    virtual am_Error_e enterSinkClassDB(const am_SinkClass_s& sinkClass, am_sinkClass_t& sinkClassID) = 0;
    virtual am_Error_e enterSourceClassDB(am_sourceClass_t& sourceClassID, const am_SourceClass_s& sourceClass) = 0;
    virtual am_Error_e enterSystemProperties(const std::vector<am_SystemProperty_s>& listSystemProperties) = 0;
    virtual am_Error_e changeMainConnectionRouteDB(const am_mainConnectionID_t mainconnectionID, const std::vector<am_connectionID_t>& listConnectionID) = 0;
    virtual am_Error_e changeMainConnectionStateDB(const am_mainConnectionID_t mainconnectionID, const am_ConnectionState_e connectionState) = 0;
    virtual am_Error_e changeSinkMainVolumeDB(const am_mainVolume_t mainVolume, const am_sinkID_t sinkID) = 0;
    virtual am_Error_e changeSinkAvailabilityDB(const am_Availability_s& availability, const am_sinkID_t sinkID) = 0;
    virtual am_Error_e changeDomainStateDB(const am_DomainState_e domainState, const am_domainID_t domainID) = 0;
    virtual am_Error_e changeSinkMuteStateDB(const am_MuteState_e muteState, const am_sinkID_t sinkID) = 0;
    virtual am_Error_e changeMainSinkSoundPropertyDB(const am_MainSoundProperty_s& soundProperty, const am_sinkID_t sinkID) = 0;
    virtual am_Error_e changeMainSourceSoundPropertyDB(const am_MainSoundProperty_s& soundProperty, const am_sourceID_t sourceID) = 0;
    virtual am_Error_e changeSourceSoundPropertyDB(const am_SoundProperty_s& soundProperty, const am_sourceID_t sourceID) = 0;
    virtual am_Error_e changeSinkSoundPropertyDB(const am_SoundProperty_s& soundProperty, const am_sinkID_t sinkID) = 0;
    virtual am_Error_e changeSourceAvailabilityDB(const am_Availability_s& availability, const am_sourceID_t sourceID) = 0;
    virtual am_Error_e changeSystemPropertyDB(const am_SystemProperty_s& property) = 0;
    virtual am_Error_e changeDelayMainConnection(const am_timeSync_t & delay, const am_mainConnectionID_t & connectionID) = 0;
    virtual am_Error_e changeSinkClassInfoDB(const am_SinkClass_s& sinkClass) = 0;
    virtual am_Error_e changeSourceClassInfoDB(const am_SourceClass_s& sourceClass) = 0;
    virtual am_Error_e changeConnectionTimingInformation(const am_connectionID_t connectionID, const am_timeSync_t delay) = 0;
    virtual am_Error_e changeConnectionFinal(const am_connectionID_t connectionID) = 0;
    virtual am_Error_e changeSourceState(const am_sourceID_t sourceID, const am_SourceState_e sourceState) = 0;
    virtual am_Error_e changeSourceInterruptState(const am_sourceID_t sourceID, const am_InterruptState_e interruptState) = 0;
    virtual am_Error_e changeSinkVolume(const am_sinkID_t sinkID, const am_volume_t volume) = 0;
    virtual am_Error_e changeSourceVolume(const am_sourceID_t sourceID, const am_volume_t volume) = 0;
    virtual am_Error_e changeCrossFaderHotSink(const am_crossfaderID_t crossfaderID, const am_HotSink_e hotsink) = 0;
    virtual am_Error_e removeMainConnectionDB(const am_mainConnectionID_t mainConnectionID) = 0;
    virtual am_Error_e removeSinkDB(const am_sinkID_t sinkID) = 0;
    virtual am_Error_e removeSourceDB(const am_sourceID_t sourceID) = 0;
    virtual am_Error_e removeGatewayDB(const am_gatewayID_t gatewayID) = 0;
    virtual am_Error_e removeConverterDB(const am_converterID_t converterID) = 0;
    virtual am_Error_e removeCrossfaderDB(const am_crossfaderID_t crossfaderID) = 0;
    virtual am_Error_e removeDomainDB(const am_domainID_t domainID) = 0;
    virtual am_Error_e removeSinkClassDB(const am_sinkClass_t sinkClassID) = 0;
    virtual am_Error_e removeSourceClassDB(const am_sourceClass_t sourceClassID) = 0;
    virtual am_Error_e removeConnection(const am_connectionID_t connectionID) = 0;
    virtual am_Error_e getSourceClassInfoDB(const am_sourceID_t sourceID, am_SourceClass_s& classInfo) const  = 0;
    virtual am_Error_e getSinkClassInfoDB(const am_sinkID_t sinkID, am_SinkClass_s& sinkClass) const  = 0;
    virtual am_Error_e getGatewayInfoDB(const am_gatewayID_t gatewayID, am_Gateway_s& gatewayData) const  = 0;
    virtual am_Error_e getConverterInfoDB(const am_converterID_t converterID, am_Converter_s& converterData) const  = 0;
    virtual am_Error_e getSinkInfoDB(const am_sinkID_t sinkID, am_Sink_s& sinkData) const  = 0;
    virtual am_Error_e getSourceInfoDB(const am_sourceID_t sourceID, am_Source_s& sourceData) const  = 0;
    virtual am_Error_e getCrossfaderInfoDB(const am_crossfaderID_t crossfaderID, am_Crossfader_s& crossfaderData) const = 0;
    virtual am_Error_e getMainConnectionInfoDB(const am_mainConnectionID_t mainConnectionID, am_MainConnection_s& mainConnectionData) const = 0;
    virtual am_Error_e getSinkMainVolume(const am_sinkID_t sinkID, am_mainVolume_t& mainVolume) const = 0;
    virtual am_Error_e getSinkVolume(const am_sinkID_t sinkID, am_volume_t& volume) const = 0;
    virtual am_Error_e getSourceVolume(const am_sourceID_t sourceID, am_volume_t& volume) const = 0;
    virtual am_Error_e getSinkSoundPropertyValue(const am_sinkID_t sinkID, const am_CustomSoundPropertyType_t propertyType, int16_t& value) const = 0;
    virtual am_Error_e getSourceSoundPropertyValue(const am_sourceID_t sourceID, const am_CustomSoundPropertyType_t propertyType, int16_t& value) const = 0;
    virtual am_Error_e getListSinkSoundProperties(const am_sinkID_t sinkID, std::vector<am_SoundProperty_s>& listSoundproperties) const =0;
    virtual am_Error_e getListSourceSoundProperties(const am_sourceID_t sourceID, std::vector<am_SoundProperty_s>& listSoundproperties) const =0;
    virtual am_Error_e getMainSinkSoundPropertyValue(const am_sinkID_t sinkID, const am_CustomMainSoundPropertyType_t propertyType, int16_t& value) const = 0;
    virtual am_Error_e getMainSourceSoundPropertyValue(const am_sourceID_t sourceID, const am_CustomMainSoundPropertyType_t propertyType, int16_t& value) const = 0;
    virtual am_Error_e getListSinksOfDomain(const am_domainID_t domainID, std::vector<am_sinkID_t>& listSinkID) const = 0;
    virtual am_Error_e getListSourcesOfDomain(const am_domainID_t domainID, std::vector<am_sourceID_t>& listSourceID) const = 0;
    virtual am_Error_e getListCrossfadersOfDomain(const am_domainID_t domainID, std::vector<am_crossfaderID_t>& listGatewaysID) const = 0;
    virtual am_Error_e getListGatewaysOfDomain(const am_domainID_t domainID, std::vector<am_gatewayID_t>& listGatewaysID) const = 0;
    virtual am_Error_e getListConvertersOfDomain(const am_domainID_t domainID, std::vector<am_converterID_t>& listConvertersID) const = 0;
    virtual am_Error_e getListMainConnections(std::vector<am_MainConnection_s>& listMainConnections) const = 0;
    virtual am_Error_e getListDomains(std::vector<am_Domain_s>& listDomains) const = 0;
    virtual am_Error_e getListConnections(std::vector<am_Connection_s>& listConnections) const = 0;
    virtual am_Error_e getListSinks(std::vector<am_Sink_s>& listSinks) const = 0;
    virtual am_Error_e getListSources(std::vector<am_Source_s>& lisSources) const = 0;
    virtual am_Error_e getListSourceClasses(std::vector<am_SourceClass_s>& listSourceClasses) const = 0;
    virtual am_Error_e getListCrossfaders(std::vector<am_Crossfader_s>& listCrossfaders) const = 0;
    virtual am_Error_e getListGateways(std::vector<am_Gateway_s>& listGateways) const = 0;
    virtual am_Error_e getListConverters(std::vector<am_Converter_s> & listConverters) const = 0;
    virtual am_Error_e getListSinkClasses(std::vector<am_SinkClass_s>& listSinkClasses) const = 0;
    virtual am_Error_e getListVisibleMainConnections(std::vector<am_MainConnectionType_s>& listConnections) const = 0;
    virtual am_Error_e getListMainSinks(std::vector<am_SinkType_s>& listMainSinks) const = 0;
    virtual am_Error_e getListMainSources(std::vector<am_SourceType_s>& listMainSources) const = 0;
    virtual am_Error_e getListMainSinkSoundProperties(const am_sinkID_t sinkID, std::vector<am_MainSoundProperty_s>& listSoundProperties) const = 0;
    virtual am_Error_e getListMainSourceSoundProperties(const am_sourceID_t sourceID, std::vector<am_MainSoundProperty_s>& listSourceProperties) const = 0;
    virtual am_Error_e getListSystemProperties(std::vector<am_SystemProperty_s>& listSystemProperties) const = 0;
    virtual am_Error_e getListSinkConnectionFormats(const am_sinkID_t sinkID, std::vector<am_CustomConnectionFormat_t> & listConnectionFormats) const = 0;
    virtual am_Error_e getListSourceConnectionFormats(const am_sourceID_t sourceID, std::vector<am_CustomConnectionFormat_t> & listConnectionFormats) const = 0;
    virtual am_Error_e getListGatewayConnectionFormats(const am_gatewayID_t gatewayID, std::vector<bool> & listConnectionFormat) const = 0;
    virtual am_Error_e getTimingInformation(const am_mainConnectionID_t mainConnectionID, am_timeSync_t& delay) const = 0;
    virtual am_Error_e getDomainOfSource(const am_sourceID_t sourceID, am_domainID_t& domainID) const = 0;
    virtual am_Error_e getDomainOfSink(const am_sinkID_t sinkID, am_domainID_t& domainID) const = 0;
    virtual am_Error_e getDomainOfCrossfader(const am_crossfaderID_t crossfader, am_domainID_t& domainID) const = 0;
    virtual am_Error_e getSoureState(const am_sourceID_t sourceID, am_SourceState_e& sourceState) const = 0;
    virtual am_Error_e getDomainState(const am_domainID_t domainID, am_DomainState_e& state) const = 0;
    virtual am_Error_e peekDomain(const std::string& name, am_domainID_t& domainID) = 0;
    virtual am_Error_e peekSink(const std::string& name, am_sinkID_t& sinkID) = 0;
    virtual am_Error_e peekSource(const std::string& name, am_sourceID_t& sourceID) = 0;
    virtual am_Error_e peekSinkClassID(const std::string& name, am_sinkClass_t& sinkClassID) = 0;
    virtual am_Error_e peekSourceClassID(const std::string& name, am_sourceClass_t& sourceClassID) = 0;
    virtual am_Error_e changeSourceDB(const am_sourceID_t sourceID, const am_sourceClass_t sourceClassID, const std::vector<am_SoundProperty_s>& listSoundProperties, const std::vector<am_CustomConnectionFormat_t>& listConnectionFormats, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties) = 0;
    virtual am_Error_e changeSinkDB(const am_sinkID_t sinkID, const am_sinkClass_t sinkClassID, const std::vector<am_SoundProperty_s>& listSoundProperties, const std::vector<am_CustomConnectionFormat_t>& listConnectionFormats, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties) = 0;
    virtual am_Error_e getListMainSinkNotificationConfigurations(const am_sinkID_t sinkID, std::vector<am_NotificationConfiguration_s>& listMainNotificationConfigurations) = 0;
    virtual am_Error_e getListMainSourceNotificationConfigurations(const am_sourceID_t sourceID, std::vector<am_NotificationConfiguration_s>& listMainNotificationConfigurations) = 0;
    virtual am_Error_e changeMainSinkNotificationConfigurationDB(const am_sinkID_t sinkID, const am_NotificationConfiguration_s mainNotificationConfiguration) = 0;
    virtual am_Error_e changeMainSourceNotificationConfigurationDB(const am_sourceID_t sourceID, const am_NotificationConfiguration_s mainNotificationConfiguration) = 0;
    virtual am_Error_e changeGatewayDB(const am_gatewayID_t gatewayID, const std::vector<am_CustomConnectionFormat_t>& listSourceConnectionFormats, const std::vector<am_CustomConnectionFormat_t>& listSinkConnectionFormats, const std::vector<bool>& convertionMatrix) = 0;
    virtual am_Error_e changeConverterDB(const am_converterID_t converterID, const std::vector<am_CustomConnectionFormat_t>& listSourceConnectionFormats, const std::vector<am_CustomConnectionFormat_t>& listSinkConnectionFormats, const std::vector<bool>& convertionMatrix) = 0;
    virtual am_Error_e changeSinkNotificationConfigurationDB(const am_sinkID_t sinkID,const am_NotificationConfiguration_s notificationConfiguration) = 0;
    virtual am_Error_e changeSourceNotificationConfigurationDB(const am_sourceID_t sourceID,const am_NotificationConfiguration_s notificationConfiguration) = 0;
    virtual bool existMainConnection(const am_mainConnectionID_t mainConnectionID) const = 0;
    virtual bool existCrossFader(const am_crossfaderID_t crossfaderID) const = 0;
    virtual bool existConnection(const am_Connection_s & connection) const = 0;
    virtual bool existConnectionID(const am_connectionID_t connectionID) const = 0;
    virtual bool existSource(const am_sourceID_t sourceID) const = 0;
    virtual bool existSourceNameOrID(const am_sourceID_t sourceID, const std::string& name) const = 0;
    virtual bool existSourceName(const std::string& name) const = 0;
    virtual bool existSink(const am_sinkID_t sinkID) const = 0;
    virtual bool existSinkNameOrID(const am_sinkID_t sinkID, const std::string& name) const = 0;
    virtual bool existSinkName(const std::string& name) const = 0;
    virtual bool existDomain(const am_domainID_t domainID) const = 0;
    virtual bool existGateway(const am_gatewayID_t gatewayID) const = 0;
    virtual bool existSinkClass(const am_sinkClass_t sinkClassID) const = 0;
    virtual bool existSourceClass(const am_sourceClass_t sourceClassID) const = 0;
    virtual bool sourceVisible(const am_sourceID_t sourceID) const = 0;
    virtual bool sinkVisible(const am_sinkID_t sinkID) const = 0;
    virtual bool isComponentConnected(const am_Gateway_s & gateway) const = 0;
    virtual bool isComponentConnected(const am_Converter_s & converter) const = 0;
    virtual am_timeSync_t calculateMainConnectionDelay(const am_mainConnectionID_t mainConnectionID) const = 0; //!< calculates a new main connection delay
    virtual void dump( std::ostream & output) const = 0 ;
    virtual am_Error_e enumerateSources(std::function<void(const am_Source_s & element)> cb) const = 0 ;
    virtual am_Error_e enumerateSinks(std::function<void(const am_Sink_s & element)> cb) const = 0 ;
    virtual am_Error_e enumerateGateways(std::function<void(const am_Gateway_s & element)> cb) const = 0 ;
    virtual am_Error_e enumerateConverters(std::function<void(const am_Converter_s & element)> cb) const = 0 ;
    /**
      * Database observer protocol
      */
    class IAmDatabaseObserver
    {
    public:
      IAmDatabaseObserver() {}
      virtual ~IAmDatabaseObserver(){}
    };

    virtual bool registerObserver(IAmDatabaseObserver * iObserver) = 0;
    virtual bool unregisterObserver(IAmDatabaseObserver * iObserver) = 0;
    virtual unsigned countObservers()  = 0;
};


}

#endif /* DATABASEHANDLERINTERFACE_H_ */
