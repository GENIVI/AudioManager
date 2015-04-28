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
 * \file CAmDatabaseHandlerMap.h
 * For further information see http://www.genivi.org/.
 *
 */

#ifndef MAPHANDLER_H_
#define MAPHANDLER_H_

#include <stdint.h>
#include <limits.h>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <assert.h>
#include <vector>
#include "IAmDatabaseHandler.h"

namespace am
{
#ifndef AM_MAP_CAPACITY
	#define AM_MAP_CAPACITY 0
#endif

#ifndef AM_MAX_CONNECTIONS
	#define AM_MAX_CONNECTIONS 0x1000
#endif

#ifndef AM_MAX_MAIN_CONNECTIONS
	#define AM_MAX_MAIN_CONNECTIONS SHRT_MAX
#endif



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
class CAmDatabaseHandlerMap : public IAmDatabaseHandler
{
    bool mFirstStaticSink; //!< bool for dynamic range handling
    bool mFirstStaticSource; //!< bool for dynamic range handling
    bool mFirstStaticGateway; //!< bool for dynamic range handling
    bool mFirstStaticConverter; //!< bool for dynamic range handling
    bool mFirstStaticSinkClass; //!< bool for dynamic range handling
    bool mFirstStaticSourceClass; //!< bool for dynamic range handling
    bool mFirstStaticCrossfader; //!< bool for dynamic range handling

public:
	CAmDatabaseHandlerMap();
    virtual ~CAmDatabaseHandlerMap();
    am_Error_e enterDomainDB(const am_Domain_s& domainData, am_domainID_t& domainID);
    am_Error_e enterMainConnectionDB(const am_MainConnection_s& mainConnectionData, am_mainConnectionID_t& connectionID);
    am_Error_e enterSinkDB(const am_Sink_s& sinkData, am_sinkID_t& sinkID);
    am_Error_e enterCrossfaderDB(const am_Crossfader_s& crossfaderData, am_crossfaderID_t& crossfaderID);
    am_Error_e enterGatewayDB(const am_Gateway_s& gatewayData, am_gatewayID_t& gatewayID);
    am_Error_e enterConverterDB(const am_Converter_s & converterData, am_converterID_t & converterID);
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
    am_Error_e getSinkVolume(const am_sinkID_t sinkID, am_volume_t& volume) const;
    am_Error_e getSourceVolume(const am_sourceID_t sourceID, am_volume_t& volume) const;
    am_Error_e getSinkSoundPropertyValue(const am_sinkID_t sinkID, const am_CustomSoundPropertyType_t propertyType, int16_t& value) const;
    am_Error_e getSourceSoundPropertyValue(const am_sourceID_t sourceID, const am_CustomSoundPropertyType_t propertyType, int16_t& value) const;
    am_Error_e getMainSinkSoundPropertyValue(const am_sinkID_t sinkID, const am_CustomMainSoundPropertyType_t propertyType, int16_t& value) const;
    am_Error_e getMainSourceSoundPropertyValue(const am_sourceID_t sourceID, const am_CustomMainSoundPropertyType_t propertyType, int16_t& value) const;
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
    am_Error_e getTimingInformation(const am_mainConnectionID_t mainConnectionID, am_timeSync_t& delay) const;
    am_Error_e getDomainOfSource(const am_sourceID_t sourceID, am_domainID_t& domainID) const;
    am_Error_e getDomainOfSink(const am_sinkID_t sinkID, am_domainID_t& domainID) const;
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
    void dump( std::ostream & output ) const;
    am_Error_e enumerateSources(std::function<void(const am_Source_s & element)> cb) const;
    am_Error_e enumerateSinks(std::function<void(const am_Sink_s & element)> cb) const;
    am_Error_e enumerateGateways(std::function<void(const am_Gateway_s & element)> cb) const;
    am_Error_e enumerateConverters(std::function<void(const am_Converter_s & element)> cb) const;
    /**
     * The following structures extend the base structures with the field 'reserved'.
     */

#define AM_SUBCLASS_BEGIN(Subclass, Class) \
	    typedef struct Subclass : public Class\
	    {

#define AM_SUBCLASS_CONSTR(Subclass, Class) \
	        Subclass():Class()

#define AM_SUBCLASS_CONSTR_BODY()\
	        {};

#define AM_SUBCLASS_COPY_OP_START(Subclass, Class) \
	        Subclass & operator=(const Subclass & anObject)\
	        {\
	            if (this != &anObject)\
	            {\
	            	Class::operator=(anObject);

#define AM_SUBCLASS_COPY_OP_END()\
	            }\
	            return *this;\
	        };

#define AM_SUBCLASS_OP(Subclass, Class) \
    		Subclass & operator=(const Class & anObject)\
    		{\
    			if (this != &anObject)\
    				Class::operator=(anObject);\
    			return *this;\
    		};

#define AM_SUBCLASS_END(Typedef) \
		void getDescription (std::string & outString) const;\
	    } Typedef;

#define AM_TYPEDEF_SUBCLASS_RESERVED_FLAG_BEGIN(Subclass, Class)\
		AM_SUBCLASS_BEGIN(Subclass, Class)\
    	bool reserved;\
    	AM_SUBCLASS_CONSTR(Subclass, Class)\
    	,reserved(false)\
    	AM_SUBCLASS_CONSTR_BODY()\
    	AM_SUBCLASS_COPY_OP_START(Subclass, Class)\
    	reserved = anObject.reserved;\
    	AM_SUBCLASS_COPY_OP_END()\
    	AM_SUBCLASS_OP(Subclass, Class)\

#define AM_TYPEDEF_SUBCLASS_SOUND_PROPERTIES_BEGIN(Subclass, Class)\
		AM_SUBCLASS_BEGIN(Subclass, Class)\
    	bool reserved;\
    	std::unordered_map<am_CustomSoundPropertyType_t, int16_t> cacheSoundProperties;\
    	std::unordered_map<am_CustomMainSoundPropertyType_t, int16_t> cacheMainSoundProperties;\
    	AM_SUBCLASS_CONSTR(Subclass, Class)\
    	,reserved(false), cacheSoundProperties(), cacheMainSoundProperties()\
    	AM_SUBCLASS_CONSTR_BODY()\
    	AM_SUBCLASS_COPY_OP_START(Subclass, Class)\
    	reserved = anObject.reserved;\
    	cacheSoundProperties = anObject.cacheSoundProperties;\
    	cacheMainSoundProperties = anObject.cacheMainSoundProperties;\
    	AM_SUBCLASS_COPY_OP_END()\
    	AM_SUBCLASS_OP(Subclass, Class)\

#define AM_TYPEDEF_SUBCLASS_BEGIN(Subclass, Class)\
		AM_SUBCLASS_BEGIN(Subclass, Class)\
		AM_SUBCLASS_COPY_OP_START(Subclass, Class)\
		AM_SUBCLASS_COPY_OP_END()\
		AM_SUBCLASS_OP(Subclass, Class)\


    AM_TYPEDEF_SUBCLASS_RESERVED_FLAG_BEGIN(am_Domain_Database_s,am_Domain_s)
    AM_SUBCLASS_END(CAmDomain)

    AM_TYPEDEF_SUBCLASS_SOUND_PROPERTIES_BEGIN(am_Sink_Database_s,am_Sink_s)
    	void getSinkType(am_SinkType_s & sinkType) const;\
    AM_SUBCLASS_END(CAmSink)

    AM_TYPEDEF_SUBCLASS_SOUND_PROPERTIES_BEGIN(am_Source_Database_s,am_Source_s)
    	void getSourceType(am_SourceType_s & sourceType) const;\
    AM_SUBCLASS_END(CAmSource)

    AM_TYPEDEF_SUBCLASS_RESERVED_FLAG_BEGIN(am_Connection_Database_s,am_Connection_s)
    AM_SUBCLASS_END(CAmConnection)

    /**
      * The following structures extend the base structures with print capabilities.
    */
    AM_TYPEDEF_SUBCLASS_BEGIN(am_MainConnection_Database_s, am_MainConnection_s)
    	void getMainConnectionType(am_MainConnectionType_s & connectionType) const;\
    AM_SUBCLASS_END(CAmMainConnection)

	AM_TYPEDEF_SUBCLASS_BEGIN(am_SourceClass_Database_s, am_SourceClass_s)
	AM_SUBCLASS_END(CAmSourceClass)

	AM_TYPEDEF_SUBCLASS_BEGIN(am_SinkClass_Database_s, am_SinkClass_s)
	AM_SUBCLASS_END(CAmSinkClass)

	AM_TYPEDEF_SUBCLASS_BEGIN(am_Gateway_Database_s, am_Gateway_s)
	AM_SUBCLASS_END(CAmGateway)

	AM_TYPEDEF_SUBCLASS_BEGIN(am_Converter_Database_s, am_Converter_s)
	AM_SUBCLASS_END(CAmConverter)

	AM_TYPEDEF_SUBCLASS_BEGIN(am_Crossfader_Database_s, am_Crossfader_s)
	AM_SUBCLASS_END(CAmCrossfader)

    private:
    typedef std::unordered_map<am_domainID_t, CAmDomain>  		  			 	 CAmMapDomain;
    typedef std::unordered_map<am_sourceClass_t, CAmSourceClass> 				 CAmMapSourceClass;
    typedef std::unordered_map<am_sinkClass_t, CAmSinkClass> 		  	  	     CAmMapSinkClass;
    typedef std::unordered_map<am_sinkID_t, CAmSink> 			   				 CAmMapSink;
    typedef std::unordered_map<am_sourceID_t, CAmSource> 		 			 	 CAmMapSource;
    typedef std::unordered_map<am_gatewayID_t, CAmGateway> 			 	 		 CAmMapGateway;
    typedef std::unordered_map<am_converterID_t, CAmConverter> 			 	 	 CAmMapConverter;
    typedef std::unordered_map<am_crossfaderID_t, CAmCrossfader> 		    	 CAmMapCrossfader;
    typedef std::unordered_map<am_connectionID_t, CAmConnection>				 CAmMapConnection;
    typedef std::unordered_map<am_mainConnectionID_t, CAmMainConnection> 	 	 CAmMapMainConnection;
    typedef std::vector<am_SystemProperty_s> 	   				  			     CAmVectorSystemProperties;
    /**
     * The following structure groups the map objects needed for the implementation.
     * Every map object is coupled with an identifier, which hold the current value.
     * DYNAMIC_ID_BOUNDARY is used as initial value everywhere a dynamic id is considered .
     * The IDs can be increased through the method increaseID(...), which follows the AudioManager logic.
     * For more information about the static and dynamic IDs, please see the documentation.
     */
    typedef struct CAmMappedData
    {
        /**
         * The structure encapsulates the id boundary and the current id value.
         * It defines a range within the id can vary.
         */
    	struct am_Identifier_s
    	{
    		int16_t mMin;			//!< min possible value
    		int16_t mMax;			//!< max possible value
    		int16_t mCurrentValue;	//!< current value

    		am_Identifier_s():mMin(DYNAMIC_ID_BOUNDARY), mMax(SHRT_MAX), mCurrentValue(mMin){};
    		am_Identifier_s(const int16_t & min, const int16_t &  max):mMin(min), mMax(max), mCurrentValue(mMin){assert(min<max);};
    	};

    	am_Identifier_s mCurrentDomainID;			//!< domain ID
    	am_Identifier_s mCurrentSourceClassesID;	//!< source classes ID
    	am_Identifier_s mCurrentSinkClassesID;		//!< sink classes ID
    	am_Identifier_s mCurrentSinkID;				//!< sink ID
    	am_Identifier_s mCurrentSourceID;			//!< source ID
    	am_Identifier_s mCurrentGatewayID;			//!< gateway ID
    	am_Identifier_s mCurrentConverterID;		//!< converter ID
    	am_Identifier_s mCurrentCrossfaderID;		//!< crossfader ID
    	am_Identifier_s mCurrentConnectionID;		//!< connection ID
    	am_Identifier_s mCurrentMainConnectionID;	//!< mainconnection ID

    	CAmVectorSystemProperties mSystemProperties; //!< vector with system properties
    	CAmMapDomain mDomainMap;					 //!< map for domain structures
    	CAmMapSourceClass mSourceClassesMap;		 //!< map for source classes structures
    	CAmMapSinkClass mSinkClassesMap;			 //!< map for sink classes structures
    	CAmMapSink mSinkMap;						 //!< map for sink structures
    	CAmMapSource mSourceMap;					 //!< map for source structures
    	CAmMapGateway mGatewayMap;					 //!< map for gateway structures
    	CAmMapConverter mConverterMap;				 //!< map for converter structures
    	CAmMapCrossfader mCrossfaderMap;			 //!< map for crossfader structures
    	CAmMapConnection mConnectionMap;			 //!< map for connection structures
    	CAmMapMainConnection mMainConnectionMap;	 //!< map for main connection structures

    	CAmMappedData(): //For Domain, MainConnections, Connections we don't have static IDs.
    		mCurrentDomainID(1, SHRT_MAX),
    		mCurrentSourceClassesID(DYNAMIC_ID_BOUNDARY, SHRT_MAX),
    		mCurrentSinkClassesID(DYNAMIC_ID_BOUNDARY, SHRT_MAX),
    		mCurrentSinkID(DYNAMIC_ID_BOUNDARY, SHRT_MAX),
    		mCurrentSourceID(DYNAMIC_ID_BOUNDARY, SHRT_MAX),
    		mCurrentGatewayID(DYNAMIC_ID_BOUNDARY, SHRT_MAX),
    		mCurrentConverterID(DYNAMIC_ID_BOUNDARY, SHRT_MAX),
    		mCurrentCrossfaderID(DYNAMIC_ID_BOUNDARY, SHRT_MAX),
    		mCurrentConnectionID(1, AM_MAX_CONNECTIONS),
    		mCurrentMainConnectionID(1, AM_MAX_MAIN_CONNECTIONS),

    		mSystemProperties(),
			mDomainMap(),mSourceClassesMap(), mSinkClassesMap(), mSinkMap(AM_MAP_CAPACITY), mSourceMap(AM_MAP_CAPACITY),
			mGatewayMap(), mConverterMap(), mCrossfaderMap(), mConnectionMap(), mMainConnectionMap()
    	{};
    	/**
    	 * \brief Increases a given map ID.
    	 *
    	 * A common method implementing the logic for static and dynamic IDs except main connection ID.
    	 *
    	 * @param resultID Pointer to an output variable.
    	 * @param sourceID Pointer to ID, which will be manipulated.
    	 * @param desiredStaticID Not 0 for static IDs and 0 for dynamic IDs.
    	 * 						  Usually the static IDs are in interval [1 , DYNAMIC_ID_BOUNDARY]. Default is 0.
    	 * @param preferedStaticIDBoundary A limit for a given dynamic ID. Default is DYNAMIC_ID_BOUNDARY.
    	 * @return TRUE on successfully changed ID.
    	 */
    	bool increaseID(int16_t & resultID, am_Identifier_s & sourceID, int16_t const desiredStaticID);
      	/**
		 * \brief Increases the main connection ID.
		 *
		 * @param resultID Pointer to an output variable.
		 * @return TRUE on successfully changed ID.
		 */
    	bool increaseMainConnectionID(int16_t & resultID);

    	/**
		 * \brief Increases the connection ID.
		 *
		 * @param resultID Pointer to an output variable.
		 * @return TRUE on successfully changed ID.
		 */
    	bool increaseConnectionID(int16_t & resultID);

        template <class TPrintObject> static void print (const TPrintObject & t, std::ostream & output)
        {
        	std::string description;
        	t.getDescription( description );
        	output << description;
        }
        template <typename TPrintMapKey,class TPrintMapObject> static void printMap (const std::unordered_map<TPrintMapKey, TPrintMapObject> & t, std::ostream & output)
        {
        	typename std::unordered_map<TPrintMapKey, TPrintMapObject>::const_iterator iter = t.begin();
        	for(; iter!=t.end(); iter++)
        		CAmMappedData::print(iter->second, output);
        }
    private:
        template <typename TMapKey,class TMapObject> bool getNextConnectionID(int16_t & resultID, am_Identifier_s & sourceID,
        																			  const std::unordered_map<TMapKey, TMapObject> & map);
    } CAmMappedData;
    /*
     * Helper methods.
     */
    am_timeSync_t calculateMainConnectionDelay(const am_mainConnectionID_t mainConnectionID) const; //!< calculates a new main connection delay
    int16_t calculateDelayForRoute(const std::vector<am_connectionID_t>& listConnectionID);
    bool insertSinkDB(const am_Sink_s & sinkData, am_sinkID_t & sinkID);
    bool insertCrossfaderDB(const am_Crossfader_s & crossfaderData, am_crossfaderID_t & crossfaderID);
    bool insertGatewayDB(const am_Gateway_s & gatewayData, am_gatewayID_t & gatewayID);
    bool insertConverterDB(const am_Converter_s & converteData, am_converterID_t & converterID);
    bool insertSourceDB(const am_Source_s & sourceData, am_sourceID_t & sourceID);
    bool insertSinkClassDB(const am_SinkClass_s & sinkClass, am_sinkClass_t & sinkClassID);
    bool insertSourceClassDB(am_sourceClass_t & sourceClassID, const am_SourceClass_s & sourceClass);
    const am_Sink_Database_s * sinkWithNameOrID(const am_sinkID_t sinkID, const std::string & name) const;
    const am_Source_Database_s * sourceWithNameOrID(const am_sourceID_t sourceID, const std::string & name) const;
    template <class Component> bool isConnected(const Component & comp) const
    {
    	return std::find_if(mMappedData.mConnectionMap.begin(), mMappedData.mConnectionMap.end(),[&](const std::pair<am_connectionID_t, am_Connection_Database_s>& rConnection){
    						return (rConnection.second.sinkID == comp.sinkID ||rConnection.second.sourceID ==comp.sourceID);})!=mMappedData.mConnectionMap.end();
    }


    CAmDatabaseObserver *mpDatabaseObserver; //!< pointer to the Observer
    ListConnectionFormat mListConnectionFormat; //!< list of connection formats
    CAmMappedData mMappedData; //!< Internal structure encapsulating all the maps used in this class
#ifdef UNIT_TEST
    public:
    void setConnectionIDRange(const int16_t & min, const int16_t &  max)
	{
		mMappedData.mCurrentConnectionID.mMin = min;
		mMappedData.mCurrentConnectionID.mMax = max;
	}
	void setMainConnectionIDRange(const int16_t & min, const int16_t &  max)
	{
		mMappedData.mCurrentMainConnectionID.mMin = min;
		mMappedData.mCurrentMainConnectionID.mMax = max;
	}
	void setSinkIDRange(const int16_t & min, const int16_t &  max)
	{
		mMappedData.mCurrentSinkID.mMin = min;
		mMappedData.mCurrentSinkID.mMax = max;
	}
#endif
};

}

#endif /* MAPHANDLER_H_ */
