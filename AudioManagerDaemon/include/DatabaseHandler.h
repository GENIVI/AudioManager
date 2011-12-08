/*
 * DatabaseHandler.h
 *
 *  Created on: Oct 24, 2011
 *      Author: christian
 */

#ifndef DATABASEHANDLER_H_
#define DATABASEHANDLER_H_

#include "audiomanagertypes.h"
#include <map>
#include <string>
#include <vector>
#include <sqlite3.h>

using namespace am;

#define DYNAMIC_ID_BOUNDARY 100

//todo: we do not have to create MainSoundProperty tables if visible = false.
//todo: check the enum values before entering & changing in the database.
//todo: change asserts for dynamic boundary checks into failure answers.#
//todo: check autoincrement boundary and set to 16bit limits
//todo: If the sink is part of a gateway, the listconnectionFormats is copied to the gatewayInformation. Check this statement for sinks & sources

class DatabaseHandler {
public:
	DatabaseHandler();
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
	am_Error_e changeSourceAvailabilityDB(const am_Availability_s& availability, const am_sourceID_t sourceID) ;
	am_Error_e changeSystemPropertyDB(const am_SystemProperty_s& property) ;
	am_Error_e changeDelayMainConnection(const am_timeSync_t & delay, const am_mainConnectionID_t & connectionID) ;
	am_Error_e changeSinkClassInfoDB(const am_SinkClass_s& sinkClass) ;
	am_Error_e changeSourceClassInfoDB(const am_SourceClass_s& sourceClass) ;
	am_Error_e changeConnectionTimingInformation(const am_connectionID_t connectionID, const am_timeSync_t delay) ;
	am_Error_e removeMainConnectionDB(const am_mainConnectionID_t mainConnectionID) ;
	am_Error_e removeSinkDB(const am_sinkID_t sinkID) ;
	am_Error_e removeSourceDB(const am_sourceID_t sourceID) ;
	am_Error_e removeGatewayDB(const am_gatewayID_t gatewayID) ;
	am_Error_e removeCrossfaderDB(const am_crossfaderID_t crossfaderID) ;
	am_Error_e removeDomainDB(const am_domainID_t domainID) ;
	am_Error_e removeSinkClassDB(const am_sinkClass_t sinkClassID) ;
	am_Error_e removeSourceClassDB(const am_sourceClass_t sourceClassID) ;
	am_Error_e getSourceClassInfoDB(const am_sourceID_t sourceID, am_SourceClass_s& classInfo) const ;
	am_Error_e getSinkClassInfoDB(const am_sinkID_t sinkID, am_SinkClass_s& sinkClass) const ;
	am_Error_e getGatewayInfoDB(const am_gatewayID_t gatewayID, am_Gateway_s& gatewayData) const ;
	am_Error_e getCrossfaderInfoDB(const am_crossfaderID_t crossfaderID, am_Crossfader_s& crossfaderData) const ;
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


	/**
	 * checks for a certain mainConnection
	 * @param mainConnectionID to be checked for
	 * @return true if it exists
	 */
	bool existMainConnection(const am_mainConnectionID_t mainConnectionID) const;

	/**
	 * checks for a certain Source
	 * @param sourceID to be checked for
	 * @return true if it exists
	 */
	bool existSource(const am_sourceID_t sourceID) const;

	/**
	 * checks for a certain Sink
	 * @param sinkID to be checked for
	 * @return true if it exists
	 */
	bool existSink(const am_sinkID_t sinkID) const;

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

private:
	bool sqQuery(const std::string& query);
	bool openDatabase();
	void createTables();
	sqlite3 *mDatabase; //!< pointer to database
	std::string mPath;
	bool mFirstStaticSink;
	bool mFirstStaticSource;
	bool mFirstStaticGateway;
	bool mFirstStaticSinkClass;
	bool mFirstStaticSourceClass;
	typedef std::map<am_gatewayID_t,std::vector<bool> > ListConnectionFormat;
	ListConnectionFormat mListConnectionFormat;
};

#endif /* DATABASEHANDLER_H_ */
