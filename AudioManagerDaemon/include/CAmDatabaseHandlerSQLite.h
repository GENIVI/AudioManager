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
 * \file CAmDatabaseHandlerSQLite.h
 * For further information see http://www.genivi.org/.
 *
 */

#ifndef DATABASEHANDLER_H_
#define DATABASEHANDLER_H_

#include "IAmDatabaseHandler.h"
#include <map>
#include <sqlite3.h>

namespace am
{

class CAmDatabaseObserver;
class CAmRoutingTree;
class CAmRoutingTreeItem;

//todo: check the enum values before entering & changing in the database.
//todo: change asserts for dynamic boundary checks into failure answers.#
//todo: check autoincrement boundary and set to 16bit limits
//todo: If the sink is part of a gateway, the listconnectionFormats is copied to the gatewayInformation. Check this statement for sinks & sources
//todo: exchange last_insert_row id to be more safe
//todo: create test to ensure uniqueness of names throughout the database
//todo: enforce the uniqueness of names

/**
 * This class handles and abstracts the database
 */
class CAmDatabaseHandlerSQLite : public IAmDatabaseHandler
{
private:
    am_timeSync_t calculateMainConnectionDelay(const am_mainConnectionID_t mainConnectionID) const; //!< calculates a new main connection delay

    CAmDatabaseObserver *mpDatabaseObserver; //!< pointer to the Observer
    bool mFirstStaticSink; //!< bool for dynamic range handling
    bool mFirstStaticSource; //!< bool for dynamic range handling
    bool mFirstStaticGateway; //!< bool for dynamic range handling
    bool mFirstStaticConverter; //!< bool for dynamic range handling
    bool mFirstStaticSinkClass; //!< bool for dynamic range handling
    bool mFirstStaticSourceClass; //!< bool for dynamic range handling
    bool mFirstStaticCrossfader; //!< bool for dynamic range handling
    ListConnectionFormat mListConnectionFormat; //!< list of connection formats
    sqlite3 *mpDatabase; //!< pointer to the database
    std::string mPath; //!< path to the database
    bool sqQuery(const std::string& query); //!< queries the database
    bool openDatabase(); //!< opens the database
    void createTables(); //!< creates all tables from the static table
public:
	CAmDatabaseHandlerSQLite();
    CAmDatabaseHandlerSQLite(std::string databasePath);
    virtual ~CAmDatabaseHandlerSQLite();
    am_Error_e enterDomainDB(const am_Domain_s& domainData, am_domainID_t& domainID);
    am_Error_e enterMainConnectionDB(const am_MainConnection_s& mainConnectionData, am_mainConnectionID_t& connectionID);
    am_Error_e enterSinkDB(const am_Sink_s& sinkData, am_sinkID_t& sinkID);
    am_Error_e enterCrossfaderDB(const am_Crossfader_s& crossfaderData, am_crossfaderID_t& crossfaderID);
    am_Error_e enterGatewayDB(const am_Gateway_s& gatewayData, am_gatewayID_t& gatewayID);
    am_Error_e enterConverterDB(const am_Converter_s & converteData, am_converterID_t & converterID);
    am_Error_e enterSourceDB(const am_Source_s& sourceData, am_sourceID_t& sourceID);
    am_Error_e enterConnectionDB(const am_Connection_s& connection, am_connectionID_t& connectionID);
    am_Error_e enterSinkClassDB(const am_SinkClass_s& sinkClass, am_sinkClass_t& sinkClassID);
    am_Error_e enterSourceClassDB(am_sourceClass_t& sourceClassID, const am_SourceClass_s& sourceClass);
    am_Error_e enterSystemProperties(const std::vector<am_SystemProperty_s>& listSystemProperties);
    am_Error_e changeMainConnectionRouteDB(const am_mainConnectionID_t mainconnectionID, const std::vector<am_connectionID_t>& listConnectionID);
    am_Error_e changeMainConnectionStateDB(const am_mainConnectionID_t mainconnectionID, const am_ConnectionState_e connectionState);
    am_Error_e changeSinkMainVolumeDB(const am_mainVolume_t mainVolume, const am_sinkID_t sinkID);
    am_Error_e changeSinkAvailabilityDB(const am_Availability_s& availability, const am_sinkID_t sinkID);
    am_Error_e changDomainStateDB(const am_DomainState_e domainState, const am_domainID_t domainID);
    am_Error_e changeSinkMuteStateDB(const am_MuteState_e muteState, const am_sinkID_t sinkID);
    am_Error_e changeMainSinkSoundPropertyDB(const am_MainSoundProperty_s& soundProperty, const am_sinkID_t sinkID);
    am_Error_e changeMainSourceSoundPropertyDB(const am_MainSoundProperty_s& soundProperty, const am_sourceID_t sourceID);
    am_Error_e changeSourceSoundPropertyDB(const am_SoundProperty_s& soundProperty, const am_sourceID_t sourceID);
    am_Error_e changeSinkSoundPropertyDB(const am_SoundProperty_s& soundProperty, const am_sinkID_t sinkID);
    am_Error_e changeSourceAvailabilityDB(const am_Availability_s& availability, const am_sourceID_t sourceID);
    am_Error_e changeSystemPropertyDB(const am_SystemProperty_s& property);
    am_Error_e changeDelayMainConnection(const am_timeSync_t & delay, const am_mainConnectionID_t & connectionID);
    am_Error_e changeSinkClassInfoDB(const am_SinkClass_s& sinkClass);
    am_Error_e changeSourceClassInfoDB(const am_SourceClass_s& sourceClass);
    am_Error_e changeConnectionTimingInformation(const am_connectionID_t connectionID, const am_timeSync_t delay);
    am_Error_e changeConnectionFinal(const am_connectionID_t connectionID);
    am_Error_e changeSourceState(const am_sourceID_t sourceID, const am_SourceState_e sourceState);
    am_Error_e changeSinkVolume(const am_sinkID_t sinkID, const am_volume_t volume);
    am_Error_e changeSourceVolume(const am_sourceID_t sourceID, const am_volume_t volume);
    am_Error_e changeCrossFaderHotSink(const am_crossfaderID_t crossfaderID, const am_HotSink_e hotsink);
    am_Error_e removeMainConnectionDB(const am_mainConnectionID_t mainConnectionID);
    am_Error_e removeSinkDB(const am_sinkID_t sinkID);
    am_Error_e removeSourceDB(const am_sourceID_t sourceID);
    am_Error_e removeGatewayDB(const am_gatewayID_t gatewayID);
    am_Error_e removeConverterDB(const am_converterID_t converterID);
    am_Error_e removeCrossfaderDB(const am_crossfaderID_t crossfaderID);
    am_Error_e removeDomainDB(const am_domainID_t domainID);
    am_Error_e removeSinkClassDB(const am_sinkClass_t sinkClassID);
    am_Error_e removeSourceClassDB(const am_sourceClass_t sourceClassID);
    am_Error_e removeConnection(const am_connectionID_t connectionID);
    am_Error_e getSourceClassInfoDB(const am_sourceID_t sourceID, am_SourceClass_s& classInfo) const;
    am_Error_e getSinkClassInfoDB(const am_sinkID_t sinkID, am_SinkClass_s& sinkClass) const;
    am_Error_e getGatewayInfoDB(const am_gatewayID_t gatewayID, am_Gateway_s& gatewayData) const;
    am_Error_e getConverterInfoDB(const am_converterID_t converterID, am_Converter_s& converterData) const;
    am_Error_e getSinkInfoDB(const am_sinkID_t sinkID, am_Sink_s& sinkData) const;
    am_Error_e getSourceInfoDB(const am_sourceID_t sourceID, am_Source_s& sourceData) const;
    am_Error_e getCrossfaderInfoDB(const am_crossfaderID_t crossfaderID, am_Crossfader_s& crossfaderData) const;
    am_Error_e getMainConnectionInfoDB(const am_mainConnectionID_t mainConnectionID, am_MainConnection_s& mainConnectionData) const;
    am_Error_e getSinkMainVolume(const am_sinkID_t sinkID, am_mainVolume_t& mainVolume) const;
    am_Error_e getSinkVolume(const am_sinkID_t sinkID, am_volume_t& volume) const;
    am_Error_e getSourceVolume(const am_sourceID_t sourceID, am_volume_t& volume) const;
    am_Error_e getMainSinkSoundPropertyValue(const am_sinkID_t sinkID, const am_CustomMainSoundPropertyType_t propertyType, int16_t& value) const;
    am_Error_e getMainSourceSoundPropertyValue(const am_sourceID_t sourceID, const am_CustomMainSoundPropertyType_t propertyType, int16_t& value) const;
    am_Error_e getSinkSoundPropertyValue(const am_sinkID_t sinkID, const am_CustomSoundPropertyType_t propertyType, int16_t& value) const;
    am_Error_e getSourceSoundPropertyValue(const am_sourceID_t sourceID, const am_CustomSoundPropertyType_t propertyType, int16_t& value) const;
	am_Error_e getListSinkSoundProperties(const am_sinkID_t sinkID, std::vector<am_SoundProperty_s>& listSoundproperties) const;
    am_Error_e getListSourceSoundProperties(const am_sourceID_t sourceID, std::vector<am_SoundProperty_s>& listSoundproperties) const;
    am_Error_e getListSinksOfDomain(const am_domainID_t domainID, std::vector<am_sinkID_t>& listSinkID) const;
    am_Error_e getListSourcesOfDomain(const am_domainID_t domainID, std::vector<am_sourceID_t>& listSourceID) const;
    am_Error_e getListCrossfadersOfDomain(const am_domainID_t domainID, std::vector<am_crossfaderID_t>& listGatewaysID) const;
    am_Error_e getListGatewaysOfDomain(const am_domainID_t domainID, std::vector<am_gatewayID_t>& listGatewaysID) const;
    am_Error_e getListConvertersOfDomain(const am_domainID_t domainID, std::vector<am_converterID_t>& listConvertersID) const;
    am_Error_e getListMainConnections(std::vector<am_MainConnection_s>& listMainConnections) const;
    am_Error_e getListDomains(std::vector<am_Domain_s>& listDomains) const;
    am_Error_e getListConnections(std::vector<am_Connection_s>& listConnections) const;
    am_Error_e getListSinks(std::vector<am_Sink_s>& listSinks) const;
    am_Error_e getListSources(std::vector<am_Source_s>& lisSources) const;
    am_Error_e getListSourceClasses(std::vector<am_SourceClass_s>& listSourceClasses) const;
    am_Error_e getListCrossfaders(std::vector<am_Crossfader_s>& listCrossfaders) const;
    am_Error_e getListGateways(std::vector<am_Gateway_s>& listGateways) const;
    am_Error_e getListConverters(std::vector<am_Converter_s> & listConverters) const;
    am_Error_e getListSinkClasses(std::vector<am_SinkClass_s>& listSinkClasses) const;
    am_Error_e getListVisibleMainConnections(std::vector<am_MainConnectionType_s>& listConnections) const;
    am_Error_e getListMainSinks(std::vector<am_SinkType_s>& listMainSinks) const;
    am_Error_e getListMainSources(std::vector<am_SourceType_s>& listMainSources) const;
    am_Error_e getListMainSinkSoundProperties(const am_sinkID_t sinkID, std::vector<am_MainSoundProperty_s>& listSoundProperties) const;
    am_Error_e getListMainSourceSoundProperties(const am_sourceID_t sourceID, std::vector<am_MainSoundProperty_s>& listSourceProperties) const;
    am_Error_e getListSystemProperties(std::vector<am_SystemProperty_s>& listSystemProperties) const;
    am_Error_e getListSinkConnectionFormats(const am_sinkID_t sinkID, std::vector<am_CustomAvailabilityReason_t> & listConnectionFormats) const;
    am_Error_e getListSourceConnectionFormats(const am_sourceID_t sourceID, std::vector<am_CustomAvailabilityReason_t> & listConnectionFormats) const;
    am_Error_e getListGatewayConnectionFormats(const am_gatewayID_t gatewayID, std::vector<bool> & listConnectionFormat) const;
    am_Error_e getListConverterConnectionFormats(const am_converterID_t converterID, std::vector<bool> & listConnectionFormat) const;
    am_Error_e getTimingInformation(const am_mainConnectionID_t mainConnectionID, am_timeSync_t& delay) const;
    am_Error_e getDomainOfSource(const am_sourceID_t sourceID, am_domainID_t& domainID) const;
    am_Error_e getDomainOfSink(const am_sinkID_t sinkID, am_domainID_t& domainID) const;
    am_Error_e getDomainOfCrossfader(const am_crossfaderID_t crossfader, am_domainID_t& domainID) const;
    am_Error_e getSoureState(const am_sourceID_t sourceID, am_SourceState_e& sourceState) const;
    am_Error_e getDomainState(const am_domainID_t domainID, am_DomainState_e& state) const;
    am_Error_e peekDomain(const std::string& name, am_domainID_t& domainID);
    am_Error_e peekSink(const std::string& name, am_sinkID_t& sinkID);
    am_Error_e peekSource(const std::string& name, am_sourceID_t& sourceID);
    am_Error_e peekSinkClassID(const std::string& name, am_sinkClass_t& sinkClassID);
    am_Error_e peekSourceClassID(const std::string& name, am_sourceClass_t& sourceClassID);
    am_Error_e changeSourceDB(const am_sourceID_t sourceID, const am_sourceClass_t sourceClassID, const std::vector<am_SoundProperty_s>& listSoundProperties, const std::vector<am_CustomAvailabilityReason_t>& listConnectionFormats, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties);
    am_Error_e changeSinkDB(const am_sinkID_t sinkID, const am_sinkClass_t sinkClassID, const std::vector<am_SoundProperty_s>& listSoundProperties, const std::vector<am_CustomAvailabilityReason_t>& listConnectionFormats, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties);
    am_Error_e getListMainSinkNotificationConfigurations(const am_sinkID_t sinkID, std::vector<am_NotificationConfiguration_s>& listMainNotificationConfigurations);
    am_Error_e getListMainSourceNotificationConfigurations(const am_sourceID_t sourceID, std::vector<am_NotificationConfiguration_s>& listMainNotificationConfigurations);
    am_Error_e changeMainSinkNotificationConfigurationDB(const am_sinkID_t sinkID, const am_NotificationConfiguration_s mainNotificationConfiguration);
    am_Error_e changeMainSourceNotificationConfigurationDB(const am_sourceID_t sourceID, const am_NotificationConfiguration_s mainNotificationConfiguration);
    am_Error_e changeGatewayDB(const am_gatewayID_t gatewayID, const std::vector<am_CustomAvailabilityReason_t>& listSourceConnectionFormats, const std::vector<am_CustomAvailabilityReason_t>& listSinkConnectionFormats, const std::vector<bool>& convertionMatrix);
    am_Error_e changeConverterDB(const am_converterID_t converterID, const std::vector<am_CustomConnectionFormat_t>& listSourceConnectionFormats, const std::vector<am_CustomConnectionFormat_t>& listSinkConnectionFormats, const std::vector<bool>& convertionMatrix);
    am_Error_e changeSinkNotificationConfigurationDB(const am_sinkID_t sinkID,const am_NotificationConfiguration_s notificationConfiguration);
    am_Error_e changeSourceNotificationConfigurationDB(const am_sourceID_t sourceID,const am_NotificationConfiguration_s notificationConfiguration);

    bool existMainConnection(const am_mainConnectionID_t mainConnectionID) const;
    bool existCrossFader(const am_crossfaderID_t crossfaderID) const;
    bool existConnection(const am_Connection_s & connection) const;
    bool existConnectionID(const am_connectionID_t connectionID) const;
    bool existSource(const am_sourceID_t sourceID) const;
    bool existSourceNameOrID(const am_sourceID_t sourceID, const std::string& name) const;
    bool existSourceName(const std::string& name) const;
    bool existSink(const am_sinkID_t sinkID) const;
    bool existSinkNameOrID(const am_sinkID_t sinkID, const std::string& name) const;
    bool existSinkName(const std::string& name) const;
    bool existDomain(const am_domainID_t domainID) const;
    bool existGateway(const am_gatewayID_t gatewayID) const;
    bool existConverter(const am_converterID_t converterID) const;
    bool existSinkClass(const am_sinkClass_t sinkClassID) const;
    bool existSourceClass(const am_sourceClass_t sourceClassID) const;
    void registerObserver(CAmDatabaseObserver *iObserver);
    bool sourceVisible(const am_sourceID_t sourceID) const;
    bool sinkVisible(const am_sinkID_t sinkID) const;
    bool isComponentConnected(const am_Gateway_s & gateway) const;
    bool isComponentConnected(const am_Converter_s & converter) const;
    //todo: Implement dump for SQLite database handler
    void dump( std::ostream & output) const { output << __FUNCTION__ << " not implemented!"; };
    am_Error_e enumerateSources(std::function<void(const am_Source_s & element)> cb) const;
    am_Error_e enumerateSinks(std::function<void(const am_Sink_s & element)> cb) const;
    am_Error_e enumerateGateways(std::function<void(const am_Gateway_s & element)> cb) const;
    am_Error_e enumerateConverters(std::function<void(const am_Converter_s & element)> cb) const;
};

}

#endif /* DATABASEHANDLER_H_ */
