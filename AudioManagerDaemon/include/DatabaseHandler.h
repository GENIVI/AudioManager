/**
* Copyright (C) 2011, BMW AG
*
* GeniviAudioMananger AudioManagerDaemon
*
* \file Databasehandler.h
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

#ifndef DATABASEHANDLER_H_
#define DATABASEHANDLER_H_

#include "audiomanagertypes.h"
#include "DatabaseObserver.h"
#include <sqlite3.h>

namespace am {

#define DYNAMIC_ID_BOUNDARY 100 //!< the value below is reserved for staticIDs, the value above will be assigned to dynamically registered items

//todo: we do not have to create MainSoundProperty tables if visible = false.
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
class DatabaseHandler {
public:
	DatabaseHandler(std::string databasePath);
	virtual ~DatabaseHandler();
	am_Error_e enterDomainDB(const am_Domain_s& domainData, am_domainID_t& domainID) ;
	am_Error_e enterMainConnectionDB(const am_MainConnection_s& mainConnectionData, am_mainConnectionID_t& connectionID) ;
	am_Error_e enterSinkDB(const am_Sink_s& sinkData, am_sinkID_t& sinkID) ;
	am_Error_e enterCrossfaderDB(const am_Crossfader_s& crossfaderData, am_crossfaderID_t& crossfaderID) ;
	am_Error_e enterGatewayDB(const am_Gateway_s& gatewayData, am_gatewayID_t& gatewayID) ;
	am_Error_e enterSourceDB(const am_Source_s& sourceData, am_sourceID_t& sourceID) ;
	am_Error_e enterConnectionDB(const am_Connection_s& connection, am_connectionID_t& connectionID);
	am_Error_e enterSinkClassDB(const am_SinkClass_s& sinkClass, am_sinkClass_t& sinkClassID) ;
	am_Error_e enterSourceClassDB(am_sourceClass_t& sourceClassID, const am_SourceClass_s& sourceClass) ;
	am_Error_e enterSystemProperties(const std::vector<am_SystemProperty_s>& listSystemProperties) ;
	am_Error_e changeMainConnectionRouteDB(const am_mainConnectionID_t mainconnectionID, const am_Route_s& route) ;
	am_Error_e changeMainConnectionStateDB(const am_mainConnectionID_t mainconnectionID, const am_ConnectionState_e connectionState) ;
	am_Error_e changeSinkMainVolumeDB(const am_mainVolume_t mainVolume, const am_sinkID_t sinkID) ;
	am_Error_e changeSinkAvailabilityDB(const am_Availability_s& availability, const am_sinkID_t sinkID) ;
	am_Error_e changDomainStateDB(const am_DomainState_e domainState, const am_domainID_t domainID) ;
	am_Error_e changeSinkMuteStateDB(const am_MuteState_e muteState, const am_sinkID_t sinkID) ;
	am_Error_e changeMainSinkSoundPropertyDB(const am_MainSoundProperty_s& soundProperty, const am_sinkID_t sinkID) ;
	am_Error_e changeMainSourceSoundPropertyDB(const am_MainSoundProperty_s& soundProperty, const am_sourceID_t sourceID) ;
	am_Error_e changeSourceSoundPropertyDB(const am_SoundProperty_s& soundProperty, const am_sourceID_t sourceID) ;
	am_Error_e changeSinkSoundPropertyDB(const am_SoundProperty_s& soundProperty, const am_sinkID_t sinkID) ;
	am_Error_e changeSourceAvailabilityDB(const am_Availability_s& availability, const am_sourceID_t sourceID) ;
	am_Error_e changeSystemPropertyDB(const am_SystemProperty_s& property) ;
	am_Error_e changeDelayMainConnection(const am_timeSync_t & delay, const am_mainConnectionID_t & connectionID) ;
	am_Error_e changeSinkClassInfoDB(const am_SinkClass_s& sinkClass) ;
	am_Error_e changeSourceClassInfoDB(const am_SourceClass_s& sourceClass) ;
	am_Error_e changeConnectionTimingInformation(const am_connectionID_t connectionID, const am_timeSync_t delay) ;
	am_Error_e changeConnectionFinal(const am_connectionID_t connectionID) ;
	am_Error_e changeSourceState(const am_sourceID_t sourceID, const am_SourceState_e sourceState);
	am_Error_e changeSinkVolume(const am_sinkID_t sinkID, const am_volume_t volume);
	am_Error_e changeSourceVolume(const am_sourceID_t sourceID, const am_volume_t volume);
	am_Error_e changeCrossFaderHotSink(const am_crossfaderID_t crossfaderID,const am_HotSink_e hotsink);
	am_Error_e removeMainConnectionDB(const am_mainConnectionID_t mainConnectionID) ;
	am_Error_e removeSinkDB(const am_sinkID_t sinkID) ;
	am_Error_e removeSourceDB(const am_sourceID_t sourceID) ;
	am_Error_e removeGatewayDB(const am_gatewayID_t gatewayID) ;
	am_Error_e removeCrossfaderDB(const am_crossfaderID_t crossfaderID) ;
	am_Error_e removeDomainDB(const am_domainID_t domainID) ;
	am_Error_e removeSinkClassDB(const am_sinkClass_t sinkClassID) ;
	am_Error_e removeSourceClassDB(const am_sourceClass_t sourceClassID) ;
	am_Error_e removeConnection(const am_connectionID_t connectionID) ;
	am_Error_e getSourceClassInfoDB(const am_sourceID_t sourceID, am_SourceClass_s& classInfo) const ;
	am_Error_e getSinkClassInfoDB(const am_sinkID_t sinkID, am_SinkClass_s& sinkClass) const ;
	am_Error_e getGatewayInfoDB(const am_gatewayID_t gatewayID, am_Gateway_s& gatewayData) const ;
	am_Error_e getCrossfaderInfoDB(const am_crossfaderID_t crossfaderID, am_Crossfader_s& crossfaderData) const ;
	am_Error_e getSinkVolume(const am_sinkID_t sinkID, am_volume_t& volume) const;
	am_Error_e getSourceVolume(const am_sourceID_t sourceID, am_volume_t& volume) const;
	am_Error_e getSinkSoundPropertyValue(const am_sinkID_t sinkID, const am_SoundPropertyType_e propertyType, uint16_t& value) const ;
	am_Error_e getSourceSoundPropertyValue(const am_sourceID_t sourceID, const am_SoundPropertyType_e propertyType, uint16_t& value) const ;
	am_Error_e getListSinksOfDomain(const am_domainID_t domainID, std::vector<am_sinkID_t>& listSinkID) const ;
	am_Error_e getListSourcesOfDomain(const am_domainID_t domainID, std::vector<am_sourceID_t>& listSourceID) const ;
	am_Error_e getListCrossfadersOfDomain(const am_domainID_t domainID, std::vector<am_crossfaderID_t>& listGatewaysID) const ;
	am_Error_e getListGatewaysOfDomain(const am_domainID_t domainID, std::vector<am_gatewayID_t>& listGatewaysID) const ;
	am_Error_e getListMainConnections(std::vector<am_MainConnection_s>& listMainConnections) const ;
	am_Error_e getListDomains(std::vector<am_Domain_s>& listDomains) const ;
	am_Error_e getListConnections(std::vector<am_Connection_s>& listConnections) const ;
	am_Error_e getListSinks(std::vector<am_Sink_s>& listSinks) const ;
	am_Error_e getListSources(std::vector<am_Source_s>& lisSources) const ;
	am_Error_e getListSourceClasses(std::vector<am_SourceClass_s>& listSourceClasses) const ;
	am_Error_e getListCrossfaders(std::vector<am_Crossfader_s>& listCrossfaders) const ;
	am_Error_e getListGateways(std::vector<am_Gateway_s>& listGateways) const ;
	am_Error_e getListSinkClasses(std::vector<am_SinkClass_s>& listSinkClasses) const ;
	am_Error_e getListVisibleMainConnections(std::vector<am_MainConnectionType_s>& listConnections) const ;
	am_Error_e getListMainSinks(std::vector<am_SinkType_s>& listMainSinks) const ;
	am_Error_e getListMainSources(std::vector<am_SourceType_s>& listMainSources) const ;
	am_Error_e getListMainSinkSoundProperties(const am_sinkID_t sinkID, std::vector<am_MainSoundProperty_s>& listSoundProperties) const ;
	am_Error_e getListMainSourceSoundProperties(const am_sourceID_t sourceID, std::vector<am_MainSoundProperty_s>& listSourceProperties) const ;
	am_Error_e getListSystemProperties(std::vector<am_SystemProperty_s>& listSystemProperties) const ;
	am_Error_e getTimingInformation(const am_mainConnectionID_t mainConnectionID, am_timeSync_t& delay) const ;
	am_Error_e getDomainOfSource(const am_sourceID_t sourceID, am_domainID_t& domainID) const;
	am_Error_e getSoureState(const am_sourceID_t sourceID, am_SourceState_e& sourceState) const;
	am_Error_e getDomainState(const am_domainID_t domainID, am_DomainState_e state) const;
	am_Error_e peekDomain(const std::string& name, am_domainID_t& domainID);
	am_Error_e peekSink(const std::string& name, am_sinkID_t& sinkID);
	am_Error_e peekSource(const std::string& name, am_sourceID_t& sourceID);
	/**
	 * checks for a certain mainConnection
	 * @param mainConnectionID to be checked for
	 * @return true if it exists
	 */
	bool existMainConnection(const am_mainConnectionID_t mainConnectionID) const;

	/**
	 * checks if a CrossFader exists
	 * @param crossfaderID the ID of the crossfader to be checked
	 * @return true if exists
	 */
	bool existcrossFader(const am_crossfaderID_t crossfaderID) const;

	/**
	 * checks if a connection already exists.
	 * Only takes sink, source and format information for search!
	 * @param connection the connection to be checked
	 * @return true if connections exists
	 */
	bool existConnection(const am_Connection_s connection);

	/**
	 * checks if a connection with the given ID exists
	 * @param connectionID
	 * @return true if connection exits
	 */
	bool existConnectionID(const am_connectionID_t connectionID);
	/**
	 * checks for a certain Source
	 * @param sourceID to be checked for
	 * @return true if it exists
	 */
	bool existSource(const am_sourceID_t sourceID) const;

	/**
	 * checks if a source name or ID exists
	 * @param sourceID the sourceID
	 * @param name the name
	 * @return true if it exits
	 */
	bool existSourceNameOrID(const am_sourceID_t sourceID, const std::string& name) const;

	/**
	 * checks if a name exits
	 * @param name the name
	 * @return true if it exits
	 */
	bool existSourceName(const std::string& name) const;
	/**
	 * checks for a certain Sink
	 * @param sinkID to be checked for
	 * @return true if it exists
	 */
	bool existSink(const am_sinkID_t sinkID) const;

	/**
	 * checks if a sink with the ID or the name exists
	 * @param sinkID the ID
	 * @param name the name
	 * @return true if it exists.
	 */
	bool existSinkNameOrID(const am_sinkID_t sinkID, const std::string& name) const;

	/**
	 * checks if a sink with the name exists
	 * @param name the name
	 * @return true if it exists
	 */
	bool existSinkName(const std::string& name) const;

	/**
	 * checks for a certain domain
	 * @param domainID to be checked for
	 * @return true if it exists
	 */
	bool existDomain(const am_domainID_t domainID) const;

	/**
	 * checks for certain gateway
	 * @param gatewayID to be checked for
	 * @return true if it exists
	 */
	bool existGateway(const am_gatewayID_t gatewayID) const;

	/**
	 * checks for certain SinkClass
	 * @param sinkClassID
	 * @return true if it exists
	 */
	bool existSinkClass(const am_sinkClass_t sinkClassID) const;

	/**
	 * checks for certain sourceClass
	 * @param sourceClassID
	 * @return true if it exists
	 */
	bool existSourceClass(const am_sourceClass_t sourceClassID) const;

	/**
	 * registers the Observer at the Database
	 * @param iObserver pointer to the observer
	 */
	void registerObserver(DatabaseObserver *iObserver);

	/**
	 * gives information about the visibility of a source
	 * @param sourceID the sourceID
	 * @return true if source is visible
	 */
	bool sourceVisible(const am_sourceID_t sourceID) const;

	/**
	 * gives information about the visibility of a sink
	 * @param sinkID the sinkID
	 * @return true if source is visible
	 */
	bool sinkVisible(const am_sinkID_t sinkID) const;

private:
	am_timeSync_t calculateMainConnectionDelay(const am_mainConnectionID_t mainConnectionID) const;
	bool sqQuery(const std::string& query);
	bool openDatabase(); //!< opens the database
	void createTables(); //!< creates all tables from the static table
	sqlite3 *mDatabase; //!< pointer to the database
	std::string mPath;  //!< path to the database
	DatabaseObserver *mDatabaseObserver; //!< pointer to the Observer
	bool mFirstStaticSink;
	bool mFirstStaticSource;
	bool mFirstStaticGateway;
	bool mFirstStaticSinkClass;
	bool mFirstStaticSourceClass;
	typedef std::map<am_gatewayID_t,std::vector<bool> > ListConnectionFormat; //!< type for list of connection formats
	ListConnectionFormat mListConnectionFormat; //!< list of connection formats
};

}

#endif /* DATABASEHANDLER_H_ */
