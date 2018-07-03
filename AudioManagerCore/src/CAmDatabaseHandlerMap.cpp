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
 * \file CAmDatabaseHandlerMap.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include <iostream>
#include <cassert>
#include <stdexcept>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <limits>
#include "CAmDatabaseHandlerMap.h"
#include "CAmRouter.h"
#include "CAmDltWrapper.h"

#define __METHOD_NAME__ std::string (std::string("CAmDatabaseHandlerMap::") + __func__)


#ifdef WITH_DATABASE_CHANGE_CHECK
#	define DB_COND_UPDATE_RIE(x,y) \
		if (isDataEqual(x,y)) return (E_NO_CHANGE); else x = y
#	define DB_COND_UPDATE_INIT \
		bool modified = false
#	define DB_COND_UPDATE(x,y) \
		if (!isDataEqual(x,y)) { x = y; modified = true; }
#	define DB_COND_ISMODIFIED \
		(modified == true)
#else
#	define DB_COND_UPDATE_RIE(x,y) \
		x = y
#	define DB_COND_UPDATE_INIT
#	define DB_COND_UPDATE(x,y) \
		x = y
#	define DB_COND_ISMODIFIED \
		(true)
#endif


#define NOTIFY_OBSERVERS(CALL)\
    for(AmDatabaseObserverCallbacks * nextObserver: mDatabaseObservers)\
            	if(nextObserver->CALL)\
    				nextObserver->CALL();

#define NOTIFY_OBSERVERS1(CALL, ARG1)\
    for(AmDatabaseObserverCallbacks * nextObserver: mDatabaseObservers)\
            	if(nextObserver->CALL)\
    				nextObserver->CALL(ARG1);

#define NOTIFY_OBSERVERS2(CALL, ARG1, ARG2)\
    for(AmDatabaseObserverCallbacks * nextObserver: mDatabaseObservers)\
            	if(nextObserver->CALL)\
    				nextObserver->CALL(ARG1, ARG2);

#define NOTIFY_OBSERVERS3(CALL, ARG1, ARG2, ARG3)\
    for(AmDatabaseObserverCallbacks * nextObserver: mDatabaseObservers)\
            	if(nextObserver->CALL)\
    				nextObserver->CALL(ARG1, ARG2, ARG3);

#define NOTIFY_OBSERVERS4(CALL, ARG1, ARG2, ARG3, ARG4)\
    for(AmDatabaseObserverCallbacks * nextObserver: mDatabaseObservers)\
            	if(nextObserver->CALL)\
    				nextObserver->CALL(ARG1, ARG2, ARG3, ARG4);

namespace am
{

/*
 * Checks if content of data is equal
 */
template <typename T> bool isDataEqual(const T & left, const T & right)
{
	return static_cast<bool>(!std::memcmp(&left, &right, sizeof(T)));
}

template <typename T, typename L = std::vector<T> > bool isDataEqual(const L & left, const L & right)
{
	return std::equal(left.begin(), left.end(), right.begin(), isDataEqual);
}


/*
 * Returns an object for given key
 */
template <typename TMapKeyType, class TMapObjectType> TMapObjectType const * objectForKeyIfExistsInMap(const TMapKeyType & key, const std::unordered_map<TMapKeyType,TMapObjectType> & map)
{
	typename std::unordered_map<TMapKeyType,TMapObjectType>::const_iterator iter = map.find(key);
	if( iter!=map.end() )
		return &iter->second;
	return NULL;
}

/*
 * Checks whether any object with key exists in a given map
 */
template <typename TMapKeyType, class TMapObjectType> bool existsObjectWithKeyInMap(const TMapKeyType & key, const std::unordered_map<TMapKeyType,TMapObjectType> & map)
{
	return objectForKeyIfExistsInMap(key, map)!=NULL;
}

/**
 * \brief Returns an object matching predicate.
 *
 * Convenient method for searching in a given map.
 *
 * @param map Map reference.
 * @param comparator Search predicate.
 * @return NULL or pointer to the found object.
 */
template <class TReturn, typename TIdentifier> const TReturn *  objectMatchingPredicate(const std::unordered_map<TIdentifier, TReturn> & map,
																								  std::function<bool(const TReturn & refObject)> comparator)
{
	typename std::unordered_map<TIdentifier, TReturn>::const_iterator elementIterator = map.begin();
	for (;elementIterator != map.end(); ++elementIterator)
	{
		if( comparator(elementIterator->second) )
			return &elementIterator->second;
	}
    return NULL;
}


/* Domain */

void CAmDatabaseHandlerMap::AmDomain::getDescription (std::string & outString) const
{
	std::ostringstream fmt;
	fmt << "Domain(" << name.c_str() << ") id(" << domainID << ")" << std::endl <<
			"bus name(" << busname.c_str() <<
			") node name(" << nodename.c_str() <<
			") early(" << early <<
			") domainID(" << domainID <<
			") complete(" << complete <<
			") state(" << state <<
			") reserved(" << reserved << ")" << std::endl;
	outString = fmt.str();
}

/* Source */

void CAmDatabaseHandlerMap::AmSource::getSourceType(am_SourceType_s & sourceType) const
{
	sourceType.name = name;
	sourceType.sourceClassID = sourceClassID;
	sourceType.availability = available;
	sourceType.sourceID = sourceID;
}

void CAmDatabaseHandlerMap::AmSource::getDescription (std::string & outString) const
{
	std::ostringstream fmt;
	fmt << "Source(" << name.c_str() << ") id(" << sourceID << ")" << std::endl <<
			"sourceClassID(" << sourceClassID <<
			") domainID(" << domainID <<
			") visible(" << visible <<
			") volume(" << volume <<
			") interruptState(" << interruptState <<
			") sourceState(" << sourceState <<
			") reserved(" << reserved << ")" <<
			") available([availability:" << available.availability << " availabilityReason:" << available.availabilityReason << "]"
			") listSoundProperties (";
			std::for_each(listSoundProperties.begin(), listSoundProperties.end(), [&](const am_SoundProperty_s & ref) {
				fmt << "[type:" << ref.type << " value:" << ref.value <<"]";
			});
			fmt << ") listConnectionFormats (";
			std::for_each(listConnectionFormats.begin(), listConnectionFormats.end(), [&](const am_CustomConnectionFormat_t & ref) {
				fmt << "[" << ref << "]";
			});
			fmt << ") listMainSoundProperties (";
			std::for_each(listMainSoundProperties.begin(), listMainSoundProperties.end(), [&](const am_MainSoundProperty_s & ref) {
				fmt << "[type:" << ref.type << " value:" << ref.value <<"]";
			});
			fmt << ") listMainNotificationConfigurations (";
			std::for_each(listMainNotificationConfigurations.begin(), listMainNotificationConfigurations.end(), [&](const am_NotificationConfiguration_s & ref) {
				fmt << "[type:" << ref.type << " status:" << ref.status << " parameter:" << ref.parameter <<"]";
			});
			fmt << ") listNotificationConfigurations (";
			std::for_each(listNotificationConfigurations.begin(), listNotificationConfigurations.end(), [&](const am_NotificationConfiguration_s & ref) {
				fmt << "[type:" << ref.type << " status:" << ref.status << " parameter:" << ref.parameter <<"]";
			});
			fmt <<  ")" << std::endl;
	outString = fmt.str();
}

/* Sink */

void CAmDatabaseHandlerMap::AmSink::getDescription (std::string & outString) const
{
	std::ostringstream fmt;
	fmt << "Sink(" << name.c_str() << ") id(" << sinkID << ")" << std::endl <<
			"sinkClassID(" << sinkClassID <<
			") domainID(" << domainID <<
			") visible(" << visible <<
			") volume(" << volume <<
			") muteState(" << muteState <<
			") mainVolume(" << mainVolume <<
			") reserved(" << reserved << ")" <<
			") available([availability:" << available.availability << " availabilityReason:" << available.availabilityReason << "]"
			") listSoundProperties (";
			std::for_each(listSoundProperties.begin(), listSoundProperties.end(), [&](const am_SoundProperty_s & ref) {
				fmt << "[type:" << ref.type << " value:" << ref.value <<"]";
			});
			fmt << ") listConnectionFormats (";
			std::for_each(listConnectionFormats.begin(), listConnectionFormats.end(), [&](const am_CustomConnectionFormat_t & ref) {
				fmt << "[" << ref << "]";
			});
			fmt << ") listMainSoundProperties (";
			std::for_each(listMainSoundProperties.begin(), listMainSoundProperties.end(), [&](const am_MainSoundProperty_s & ref) {
				fmt << "[type:" << ref.type << " value:" << ref.value <<"]";
			});
			fmt << ") listMainNotificationConfigurations (";
			std::for_each(listMainNotificationConfigurations.begin(), listMainNotificationConfigurations.end(), [&](const am_NotificationConfiguration_s & ref) {
				fmt << "[type:" << ref.type << " status:" << ref.status << " parameter:" << ref.parameter <<"]";
			});
			fmt << ") listNotificationConfigurations (";
			std::for_each(listNotificationConfigurations.begin(), listNotificationConfigurations.end(), [&](const am_NotificationConfiguration_s & ref) {
				fmt << "[type:" << ref.type << " status:" << ref.status << " parameter:" << ref.parameter <<"]";
			});
			fmt <<  ")" << std::endl;
	outString = fmt.str();
}

void CAmDatabaseHandlerMap::AmSink::getSinkType(am_SinkType_s & sinkType) const
{
	sinkType.name = name;
	sinkType.sinkID = sinkID;
	sinkType.availability = available;
	sinkType.muteState = muteState;
	sinkType.volume = mainVolume;
	sinkType.sinkClassID = sinkClassID;
}

/* Connection */

void CAmDatabaseHandlerMap::AmConnection::getDescription (std::string & outString) const
{
	std::ostringstream fmt;
	fmt << "Connection id(" << connectionID << ") " << std::endl <<
			"sourceID(" << sourceID <<
			") sinkID(" << sinkID <<
			") delay(" << delay <<
			") connectionFormat(" << connectionFormat <<
			") reserved(" << reserved << ")" << std::endl;
	outString = fmt.str();
}

/* Main Connection */

void CAmDatabaseHandlerMap::AmMainConnection::getDescription (std::string & outString) const
{
	std::ostringstream fmt;
	fmt << "MainConnection id(" << mainConnectionID << ") " << std::endl <<
			"connectionState(" << connectionState <<
			") sinkID(" << sinkID <<
			") sourceID(" << sourceID <<
			") delay(" << delay <<
			") listConnectionID (";
			std::for_each(listConnectionID.begin(), listConnectionID.end(), [&](const am_connectionID_t & connID) {
				fmt << "["<< connID << "]";
			});
			fmt << ")" << std::endl;
	outString = fmt.str();
}

void CAmDatabaseHandlerMap::am_MainConnection_Database_s::getMainConnectionType(am_MainConnectionType_s & connectionType) const
{
	connectionType.mainConnectionID = mainConnectionID;
	connectionType.sourceID = sourceID;
	connectionType.sinkID = sinkID;
	connectionType.connectionState = connectionState;
	connectionType.delay = delay;
}

/* Source Class */

void CAmDatabaseHandlerMap::AmSourceClass::getDescription (std::string & outString) const
{
	std::ostringstream fmt;
	fmt << "Source class(" << name.c_str() << ") id(" << sourceClassID << ")\n" <<
			") listClassProperties (";
			std::for_each(listClassProperties.begin(), listClassProperties.end(), [&](const am_ClassProperty_s & ref) {
				fmt << "[classProperty:" << ref.classProperty << " value:" << ref.value << "]";
			});
			fmt << ")" << std::endl;
	outString = fmt.str();
}

/* Sink Class */

void CAmDatabaseHandlerMap::AmSinkClass::getDescription (std::string & outString) const
{
	std::ostringstream fmt;
	fmt << "Sink class(" << name.c_str() << ") id(" << sinkClassID << ")\n" <<
			") listClassProperties (";
			std::for_each(listClassProperties.begin(), listClassProperties.end(), [&](const am_ClassProperty_s & ref) {
				fmt << "[classProperty:" << ref.classProperty << " value:" << ref.value << "]";
			});
			fmt << ")" << std::endl;
	outString = fmt.str();
}


/* Gateway */

void CAmDatabaseHandlerMap::AmGateway::getDescription (std::string & outString) const
{
	std::ostringstream fmt;
	fmt << "Gateway(" << name.c_str() << ") id(" << gatewayID << ")\n" <<
			"sinkID(" << sinkID <<
			") sourceID(" << sourceID <<
			") domainSinkID(" << domainSinkID <<
			") domainSourceID(" << domainSourceID <<
			") controlDomainID(" << controlDomainID <<
			") listSourceFormats (";
			std::for_each(listSourceFormats.begin(), listSourceFormats.end(), [&](const am_CustomConnectionFormat_t & ref) {
				fmt << "[" << ref << "]";
			});
			fmt << ") listSinkFormats (";
			std::for_each(listSinkFormats.begin(), listSinkFormats.end(), [&](const am_CustomConnectionFormat_t & ref) {
				fmt << "[" << ref << "]";
			});
			fmt << ") convertionMatrix (";
			std::for_each(convertionMatrix.begin(), convertionMatrix.end(), [&](const bool & ref) {
				fmt << "[" << ref << "]";
			});
			fmt << ")" << std::endl;
	outString = fmt.str();
}

/* Converter */

void CAmDatabaseHandlerMap::AmConverter::getDescription (std::string & outString) const
{
	std::ostringstream fmt;
	fmt << "Converter(" << name.c_str() << ") id(" << converterID << ")\n" <<
			"sinkID(" << sinkID <<
			") sourceID(" << sourceID <<
			") domainSinkID(" << domainID <<
			") listSourceFormats (";
			std::for_each(listSourceFormats.begin(), listSourceFormats.end(), [&](const am_CustomConnectionFormat_t & ref) {
				fmt << "[" << ref << "]";
			});
			fmt << ") listSinkFormats (";
			std::for_each(listSinkFormats.begin(), listSinkFormats.end(), [&](const am_CustomConnectionFormat_t & ref) {
				fmt << "[" << ref << "]";
			});
			fmt << ") convertionMatrix (";
			std::for_each(convertionMatrix.begin(), convertionMatrix.end(), [&](const bool & ref) {
				fmt << "[" << ref << "]";
			});
			fmt << ")" << std::endl;
	outString = fmt.str();
}

/* Crossfader */

void CAmDatabaseHandlerMap::AmCrossfader::getDescription (std::string & outString) const
{
	std::ostringstream fmt;
	fmt << "Crossfader(" << name.c_str() << ") id(" << crossfaderID << ")\n" <<
			"sinkID_A(" << sinkID_A <<
			") sinkID_B(" << sinkID_B <<
			") sourceID(" << sourceID <<
			") hotSink(" << hotSink <<
			")" << std::endl;
	outString = fmt.str();
}

bool CAmDatabaseHandlerMap::AmMappedData::increaseID(int16_t & resultID, AmIdentifier & elementID,
															 int16_t const desiredStaticID = 0)
{
	if( desiredStaticID > 0 && desiredStaticID < elementID.mMin )
	{
		resultID = desiredStaticID;
		return true;
	}
	else if( elementID.mCurrentValue < elementID.mMax ) //The last used value is 'limit' - 1. e.g. SHRT_MAX - 1, SHRT_MAX is reserved.
	{
		resultID = elementID.mCurrentValue++;
		return true;
	}
	else
	{
		resultID = -1;
		return false;
	}
}

template <typename TMapKey,class TMapObject> bool CAmDatabaseHandlerMap::AmMappedData::getNextConnectionID(int16_t & resultID, AmIdentifier & connID,
																			  	  	  	  	  	  	  	  	  	  	  	  const std::unordered_map<TMapKey, TMapObject> & map)
{
	TMapKey nextID;
	int16_t const lastID = connID.mCurrentValue;
	if( connID.mCurrentValue < connID.mMax )
		nextID = connID.mCurrentValue++;
	else
		nextID = connID.mCurrentValue = connID.mMin;

	bool notFreeIDs = false;
	while( existsObjectWithKeyInMap(nextID, map) )
	{

		if( connID.mCurrentValue < connID.mMax )
			nextID = connID.mCurrentValue++;
		else
		{
			connID.mCurrentValue = connID.mMin;
			nextID = connID.mCurrentValue;
		}

		if( connID.mCurrentValue == lastID )
		{
			notFreeIDs = true;
			break;
		}
	}
	if(notFreeIDs)
	{
		resultID = -1;
		return false;
	}
	resultID = nextID;
	return true;
}

bool CAmDatabaseHandlerMap::AmMappedData::increaseMainConnectionID(int16_t & resultID)
{
	return getNextConnectionID(resultID, mCurrentMainConnectionID, mMainConnectionMap);
}

bool CAmDatabaseHandlerMap::AmMappedData::increaseConnectionID(int16_t & resultID)
{
	return getNextConnectionID(resultID, mCurrentConnectionID, mConnectionMap);
}


CAmDatabaseHandlerMap::CAmDatabaseHandlerMap():	IAmDatabaseHandler(),
		mFirstStaticSink(true),
		mFirstStaticSource(true),
		mFirstStaticGateway(true),
		mFirstStaticConverter(true),
		mFirstStaticSinkClass(true),
		mFirstStaticSourceClass(true),
		mFirstStaticCrossfader(true),
		mListConnectionFormat(),
		mMappedData(),
		mDatabaseObservers()
{
	logVerbose(__METHOD_NAME__,"Init ");
}

CAmDatabaseHandlerMap::~CAmDatabaseHandlerMap()
{
    logVerbose(__METHOD_NAME__,"Destroy");
	for(AmDatabaseObserverCallbacks * ptr: mDatabaseObservers)
		ptr->mpDatabaseHandler=nullptr;
}

am_Error_e CAmDatabaseHandlerMap::enterDomainDB(const am_Domain_s & domainData, am_domainID_t & domainID)
{
    if(domainData.name.empty())
    {
    	logError(__METHOD_NAME__,"DomainName must not be emtpy!");
    	return (E_NOT_POSSIBLE);
    }
    if(domainData.busname.empty())
    {
    	logError(__METHOD_NAME__,"Busname must not be emtpy!");
    	return (E_NOT_POSSIBLE);
    }
    if(!(domainData.state>=DS_UNKNOWN && domainData.state<=DS_MAX))
    {
    	logError(__METHOD_NAME__,"State must not be valid!");
    	return (E_NOT_POSSIBLE);
    }
    //first check for a reserved domain
    am_Domain_s const *reservedDomain = objectMatchingPredicate<AmDomain, am_domainID_t>(mMappedData.mDomainMap, [&](const AmDomain & obj){
    	return domainData.name.compare(obj.name)==0;
    });

    int16_t nextID = 0;

    if( NULL != reservedDomain )
    {
    	nextID = reservedDomain->domainID;
    	domainID = nextID;
    	mMappedData.mDomainMap[nextID] = domainData;
    	mMappedData.mDomainMap[nextID].domainID = nextID;
    	mMappedData.mDomainMap[nextID].reserved = 0;
    	logVerbose("DatabaseHandler::enterDomainDB entered reserved domain with name=", domainData.name, "busname=", domainData.busname, "nodename=", domainData.nodename, "reserved ID:", domainID);

    	NOTIFY_OBSERVERS1(dboNewDomain, mMappedData.mDomainMap[nextID])

    	return (E_OK);
    }
    else
    {
		if(mMappedData.increaseID(nextID, mMappedData.mCurrentDomainID,  domainData.domainID))
		{
			domainID = nextID;
			mMappedData.mDomainMap[nextID] = domainData;
			mMappedData.mDomainMap[nextID].domainID = nextID;
			logVerbose("DatabaseHandler::enterDomainDB entered new domain with name=", domainData.name, "busname=", domainData.busname, "nodename=", domainData.nodename, "assigned ID:", domainID);

			NOTIFY_OBSERVERS1(dboNewDomain, mMappedData.mDomainMap[nextID])

			return (E_OK);
		}
		else
		{
			domainID = 0;
			logVerbose(__METHOD_NAME__,"Max limit reached.");
			return (E_UNKNOWN);
		}
    }
}

int16_t CAmDatabaseHandlerMap::calculateDelayForRoute(const std::vector<am_connectionID_t>& listConnectionID)
{
	int16_t delay = 0;
	std::vector<am_connectionID_t>::const_iterator elementIterator = listConnectionID.begin();
	for (; elementIterator < listConnectionID.end(); ++elementIterator)
	{
		am_connectionID_t key = *elementIterator;
		std::unordered_map<am_connectionID_t, am_Connection_Database_s>::const_iterator it = mMappedData.mConnectionMap.find(key);
		if (it!=mMappedData.mConnectionMap.end())
		{
			int16_t temp_delay = it->second.delay;
			if (temp_delay != -1 && delay != -1)
				delay += temp_delay;
			else
				delay = -1;
		}
	}
	return delay;
}

am_Error_e CAmDatabaseHandlerMap::enterMainConnectionDB(const am_MainConnection_s & mainConnectionData, am_mainConnectionID_t & connectionID)
{
    if(mainConnectionData.mainConnectionID!=0)
    {
    	logError(__METHOD_NAME__,"mainConnectionID must be 0!");
    	return (E_NOT_POSSIBLE);
    }
    if(!(mainConnectionData.connectionState>=CS_UNKNOWN && mainConnectionData.connectionState<=CS_MAX))
    {
    	logError(__METHOD_NAME__,"connectionState must be valid!");
    	return (E_NOT_POSSIBLE);
    }
    if(!existSink(mainConnectionData.sinkID))
    {
    	logError(__METHOD_NAME__,"sinkID must be valid!");
    	return (E_NOT_POSSIBLE);
    }
    if(!existSource(mainConnectionData.sourceID))
    {
    	logError(__METHOD_NAME__,"sourceID must be valid!");
    	return (E_NOT_POSSIBLE);
    }

    int16_t delay = 0;
    int16_t nextID = 0;
	if(mMappedData.increaseMainConnectionID(nextID))
	{
		connectionID = nextID;
		mMappedData.mMainConnectionMap[nextID] = mainConnectionData;
		mMappedData.mMainConnectionMap[nextID].mainConnectionID = nextID;
	}
	else
	{
		connectionID = 0;
		logVerbose(__METHOD_NAME__,"Max limit reached.");
		return (E_UNKNOWN);
	}

    //now check the connectionTable for all connections in the route. IF connectionID exist
    delay = calculateDelayForRoute(mainConnectionData.listConnectionID);
    logVerbose("DatabaseHandler::enterMainConnectionDB entered new mainConnection with sourceID", mainConnectionData.sourceID, "sinkID:", mainConnectionData.sinkID, "delay:", delay, "assigned ID:", connectionID);


    if (mDatabaseObservers.size())
    {
    	am_MainConnectionType_s mainConnection;
		mMappedData.mMainConnectionMap[nextID].getMainConnectionType(mainConnection);

        NOTIFY_OBSERVERS1(dboNewMainConnection, mainConnection)
        NOTIFY_OBSERVERS2(dboMainConnectionStateChanged, connectionID, mMappedData.mMainConnectionMap[nextID].connectionState)
    }

    //finally, we update the delay value for the maintable
    if (delay == 0)
        delay = -1;
    (void)changeDelayMainConnection(delay, connectionID);

    return (E_OK);
}

/**
 * Helper method, that inserts a new struct in the map and copies the given into it.
 * This method uses the increaseID function to secure the global id is properly increased.
 **/
bool CAmDatabaseHandlerMap::insertSinkDB(const am_Sink_s & sinkData, am_sinkID_t & sinkID)
{
    int16_t nextID = 0;
	if(  mMappedData.increaseID(nextID, mMappedData.mCurrentSinkID, sinkData.sinkID) )
	{
		sinkID = nextID;
		mMappedData.mSinkMap[nextID] = sinkData;
		mMappedData.mSinkMap[nextID].sinkID = nextID;
		filterDuplicateNotificationConfigurationTypes(mMappedData.mSinkMap[nextID].listNotificationConfigurations);
		filterDuplicateNotificationConfigurationTypes(mMappedData.mSinkMap[nextID].listMainNotificationConfigurations);
		return (true);
	}
	else
	{
		sinkID = 0;
		logVerbose(__METHOD_NAME__,"Max limit reached!");
		return (false);
	}
}

am_Error_e CAmDatabaseHandlerMap::enterSinkDB(const am_Sink_s & sinkData, am_sinkID_t & sinkID)
{
    if(sinkData.sinkID>=DYNAMIC_ID_BOUNDARY)
    {
    	logError(__METHOD_NAME__,"sinkID must be below:",DYNAMIC_ID_BOUNDARY);
    	return (E_NOT_POSSIBLE);
    }
    if(!existDomain(sinkData.domainID))
    {
    	logError(__METHOD_NAME__,"domainID must be valid");
    	return (E_NOT_POSSIBLE);
    }
    if(sinkData.name.empty())
    {
    	logError(__METHOD_NAME__,"sinkName must not be zero");
    	return (E_NOT_POSSIBLE);
    }
    if(!existSinkClass(sinkData.sinkClassID))
    {
    	logError(__METHOD_NAME__,"sinkClass must be valid");
    	return (E_NOT_POSSIBLE);
    }

    if(!(sinkData.muteState>=MS_UNKNOWN && sinkData.muteState<=MS_MAX))
    {
    	logError(__METHOD_NAME__,"muteState must be valid");
    	return (E_NOT_POSSIBLE);
    }

    am_sinkID_t temp_SinkID = 0;
    am_sinkID_t temp_SinkIndex = 0;
	//if sinkID is zero and the first Static Sink was already entered, the ID is created
    am_Sink_s const *reservedDomain = objectMatchingPredicate<AmSink, am_sinkID_t>(mMappedData.mSinkMap, [&](const AmSink & obj){
		return true==obj.reserved && obj.name.compare(sinkData.name)==0;
    });
    if( NULL!=reservedDomain )
    {
    	am_sinkID_t oldSinkID = reservedDomain->sinkID;
    	mMappedData.mSinkMap[oldSinkID] = sinkData;
    	mMappedData.mSinkMap[oldSinkID].reserved = 0;
    	temp_SinkID = oldSinkID;
    	temp_SinkIndex = oldSinkID;
    }
    else
    {
		bool result;
		if ( sinkData.sinkID != 0 || mFirstStaticSink )
		{
			//check if the ID already exists
			if (existSinkNameOrID(sinkData.sinkID, sinkData.name))
			{
				sinkID = sinkData.sinkID;
				return (E_ALREADY_EXISTS);
			}
		}
		result = insertSinkDB(sinkData, temp_SinkID);
		if( false == result )
			return (E_UNKNOWN);
		temp_SinkIndex = temp_SinkID;
    }
    //if the first static sink is entered, we need to set it onto the boundary
    if (sinkData.sinkID == 0 && mFirstStaticSink)
    {
        mFirstStaticSink = false;
    }
    mMappedData.mSinkMap[temp_SinkIndex].sinkID = temp_SinkID;
    sinkID = temp_SinkID;

    am_Sink_s & sink = mMappedData.mSinkMap[temp_SinkID];
    logVerbose("DatabaseHandler::enterSinkDB entered new sink with name", sink.name, "domainID:", sink.domainID, "classID:", sink.sinkClassID, "volume:", sink.volume, "assigned ID:", sink.sinkID);

	sink.sinkID=sinkID;
	NOTIFY_OBSERVERS1(dboNewSink, sink)

    return (E_OK);
}

bool CAmDatabaseHandlerMap::insertCrossfaderDB(const am_Crossfader_s & crossfaderData, am_crossfaderID_t & crossfaderID)
{
    int16_t nextID = 0;
	if(mMappedData.increaseID(nextID, mMappedData.mCurrentCrossfaderID, crossfaderData.crossfaderID))
	{
		crossfaderID = nextID;
		mMappedData.mCrossfaderMap[nextID] = crossfaderData;
		mMappedData.mCrossfaderMap[nextID].crossfaderID = nextID;
		return (true);
	}
	else
	{
		crossfaderID = 0;
		logVerbose(__METHOD_NAME__,"Max limit reached.");
		return (false);
	}
}

am_Error_e CAmDatabaseHandlerMap::enterCrossfaderDB(const am_Crossfader_s & crossfaderData, am_crossfaderID_t & crossfaderID)
{
    if(crossfaderData.crossfaderID>=DYNAMIC_ID_BOUNDARY)
    {
    	logError(__METHOD_NAME__,"crossfaderID must be below:",DYNAMIC_ID_BOUNDARY);
    	return (E_NOT_POSSIBLE);
    }

    if(!(crossfaderData.hotSink>=HS_UNKNOWN && crossfaderData.hotSink<=HS_MAX))
    {
    	logError(__METHOD_NAME__,"hotSink must be valid");
    	return (E_NOT_POSSIBLE);
    }
    if(crossfaderData.name.empty())
    {
    	logError(__METHOD_NAME__,"crossfaderName must not be zero");
    	return (E_NOT_POSSIBLE);
    }

    if(!existSink(crossfaderData.sinkID_A))
    {
    	logError(__METHOD_NAME__,"sinkID_A must exist");
    	return (E_NOT_POSSIBLE);
    }
    if(!existSink(crossfaderData.sinkID_B))
    {
    	logError(__METHOD_NAME__,"sinkID_B must exist");
    	return (E_NOT_POSSIBLE);
    }
    if(!existSource(crossfaderData.sourceID))
    {
    	logError(__METHOD_NAME__,"sourceID must exist");
    	return (E_NOT_POSSIBLE);
    }

    am_crossfaderID_t temp_CrossfaderID = 0;
    am_crossfaderID_t temp_CrossfaderIndex = 0;

    bool result;
    //if gatewayData is zero and the first Static Sink was already entered, the ID is created
    if (crossfaderData.crossfaderID != 0 || mFirstStaticCrossfader)
    {
        //check if the ID already exists
        if (existCrossFader(crossfaderData.crossfaderID))
        {
        	crossfaderID = crossfaderData.crossfaderID;
            return (E_ALREADY_EXISTS);
        }
    }
    result = insertCrossfaderDB(crossfaderData, temp_CrossfaderID);
	if( false == result )
		return (E_UNKNOWN);
	temp_CrossfaderIndex = temp_CrossfaderID;

    //if the first static sink is entered, we need to set it onto the boundary
    if ( 0==crossfaderData.crossfaderID && mFirstStaticCrossfader)
    {
        mFirstStaticCrossfader = false;
    }

   mMappedData.mCrossfaderMap[temp_CrossfaderIndex].crossfaderID = temp_CrossfaderID;
   crossfaderID = temp_CrossfaderID;
   logVerbose("DatabaseHandler::enterCrossfaderDB entered new crossfader with name=", crossfaderData.name, "sinkA= ", crossfaderData.sinkID_A, "sinkB=", crossfaderData.sinkID_B, "source=", crossfaderData.sourceID, "assigned ID:", crossfaderID);

   NOTIFY_OBSERVERS1(dboNewCrossfader, mMappedData.mCrossfaderMap[temp_CrossfaderIndex])

    return (E_OK);
}

bool CAmDatabaseHandlerMap::insertGatewayDB(const am_Gateway_s & gatewayData, am_gatewayID_t & gatewayID)
{
    int16_t nextID = 0;
	if(mMappedData.increaseID(nextID, mMappedData.mCurrentGatewayID, gatewayData.gatewayID))
	{
		gatewayID = nextID;
		mMappedData.mGatewayMap[nextID] = gatewayData;
		mMappedData.mGatewayMap[nextID].gatewayID = nextID;
		return (true);
	}
	else
	{
		gatewayID = 0;
		logVerbose(__METHOD_NAME__,"Max limit reached.");
		return (false);
	}
}

am_Error_e CAmDatabaseHandlerMap::enterGatewayDB(const am_Gateway_s & gatewayData, am_gatewayID_t & gatewayID)
{

    if(gatewayData.gatewayID>=DYNAMIC_ID_BOUNDARY)
    {
    	logError(__METHOD_NAME__,"gatewayID must be below:",DYNAMIC_ID_BOUNDARY);
    	return (E_NOT_POSSIBLE);
    }

    if(!existDomain(gatewayData.controlDomainID))
    {
    	logError(__METHOD_NAME__,"controlDomainID must be exist");
    	return (E_NOT_POSSIBLE);
    }

    if(gatewayData.name.empty())
    {
    	logError(__METHOD_NAME__,"gatewayName must not be empty");
    	return (E_NOT_POSSIBLE);
    }

    //might be that the sinks and sources are not there during registration time
    //assert(existSink(gatewayData.sinkID));
    //assert(existSource(gatewayData.sourceID));

    am_gatewayID_t temp_GatewayID = 0;
    am_gatewayID_t temp_GatewayIndex = 0;
    //if gatewayData is zero and the first Static Sink was already entered, the ID is created
    bool result;
    if (gatewayData.gatewayID != 0 || mFirstStaticGateway)
    {
        //check if the ID already exists
        if (existGateway(gatewayData.gatewayID))
        {
        	gatewayID = gatewayData.gatewayID;
            return (E_ALREADY_EXISTS);
        }
    }
    result = insertGatewayDB(gatewayData, temp_GatewayID);
	if( false == result )
		return (E_UNKNOWN);

	temp_GatewayIndex = temp_GatewayID;
    //if the ID is not created, we add it to the query
    if (gatewayData.gatewayID == 0 && mFirstStaticGateway)
    {
        mFirstStaticGateway = false;
    }
    mMappedData.mGatewayMap[temp_GatewayIndex].gatewayID = temp_GatewayID;
    gatewayID = temp_GatewayID;

    logVerbose("DatabaseHandler::enterGatewayDB entered new gateway with name", gatewayData.name, "sourceID:", gatewayData.sourceID, "sinkID:", gatewayData.sinkID, "assigned ID:", gatewayID);

    NOTIFY_OBSERVERS1(dboNewGateway, mMappedData.mGatewayMap[temp_GatewayIndex])
    return (E_OK);
}

bool CAmDatabaseHandlerMap::insertConverterDB(const am_Converter_s & converteData, am_converterID_t & converterID)
{
    int16_t nextID = 0;
	if(mMappedData.increaseID(nextID, mMappedData.mCurrentConverterID, converteData.converterID))
	{
		converterID = nextID;
		mMappedData.mConverterMap[nextID] = converteData;
		mMappedData.mConverterMap[nextID].converterID = nextID;
		return (true);
	}
	else
	{
		converterID = 0;
		logVerbose(__METHOD_NAME__,"Max limit reached.");
		return (false);
	}
}

am_Error_e CAmDatabaseHandlerMap::enterConverterDB(const am_Converter_s & converterData, am_converterID_t & converterID)
{
    if(converterData.converterID>=DYNAMIC_ID_BOUNDARY)
    {
    	logError(__METHOD_NAME__,"converterID must be below:",DYNAMIC_ID_BOUNDARY);
    	return (E_NOT_POSSIBLE);
    }

    if(!existSink(converterData.sinkID))
    {
    	logError(__METHOD_NAME__,"sinkID must exists");
    	return (E_NOT_POSSIBLE);
    }

    if(!existSource(converterData.sourceID))
    {
    	logError(__METHOD_NAME__,"sourceID must exists");
    	return (E_NOT_POSSIBLE);
    }

    if(!existDomain(converterData.domainID))
    {
    	logError(__METHOD_NAME__,"domainID must exists");
    	return (E_NOT_POSSIBLE);
    }

    if(converterData.name.empty())
    {
    	logError(__METHOD_NAME__,"converterName must not be empty");
    	return (E_NOT_POSSIBLE);
    }

    //might be that the sinks and sources are not there during registration time
    //assert(existSink(gatewayData.sinkID));
    //assert(existSource(gatewayData.sourceID));

    am_converterID_t tempID = 0;
    am_converterID_t tempIndex = 0;
    //if gatewayData is zero and the first Static Sink was already entered, the ID is created
    bool result;
    if (converterData.converterID != 0 || mFirstStaticConverter)
    {
        //check if the ID already exists
        if (existConverter(converterData.converterID))
        {
        	converterID = converterData.converterID;
            return (E_ALREADY_EXISTS);
        }
    }
    result = insertConverterDB(converterData, tempID);
	if( false == result )
		return (E_UNKNOWN);

	tempIndex = tempID;
    //if the ID is not created, we add it to the query
    if (converterData.converterID == 0 && mFirstStaticConverter)
    {
    	mFirstStaticConverter = false;
    }
    mMappedData.mConverterMap[tempIndex].converterID = tempID;
    converterID = tempID;

    logVerbose("DatabaseHandler::enterConverterDB entered new converter with name", converterData.name, "sourceID:", converterData.sourceID, "sinkID:", converterData.sinkID, "assigned ID:", converterID);
    NOTIFY_OBSERVERS1(dboNewConverter, mMappedData.mConverterMap[tempIndex])

    return (E_OK);
}

void CAmDatabaseHandlerMap::dump( std::ostream & output ) const
{
	output << std::endl << "****************** DUMP START ******************" << std::endl;
	AmMappedData::printMap(mMappedData.mDomainMap, output);
	AmMappedData::printMap(mMappedData.mSourceMap, output);
	AmMappedData::printMap(mMappedData.mSinkMap, output);
	AmMappedData::printMap(mMappedData.mSourceClassesMap, output);
	AmMappedData::printMap(mMappedData.mSinkClassesMap, output);
	AmMappedData::printMap(mMappedData.mConnectionMap, output);
	AmMappedData::printMap(mMappedData.mMainConnectionMap, output);
	AmMappedData::printMap(mMappedData.mCrossfaderMap, output);
	AmMappedData::printMap(mMappedData.mGatewayMap, output);
	AmVectorSystemProperties::const_iterator iter = mMappedData.mSystemProperties.begin();
	output << "System properties" << "\n";
	for(; iter!=mMappedData.mSystemProperties.end(); iter++)
		output << "[type:" << iter->type << " value:" << iter->value << "]";
	output << std::endl << "****************** DUMP END ******************" << std::endl;
}

bool CAmDatabaseHandlerMap::insertSourceDB(const am_Source_s & sourceData, am_sourceID_t & sourceID)
{
    int16_t nextID = 0;
	if(mMappedData.increaseID(nextID, mMappedData.mCurrentSourceID, sourceData.sourceID))
	{
		sourceID = nextID;
		mMappedData.mSourceMap[nextID] = sourceData;
		mMappedData.mSourceMap[nextID].sourceID = nextID;
		filterDuplicateNotificationConfigurationTypes(mMappedData.mSourceMap[nextID].listNotificationConfigurations);
		filterDuplicateNotificationConfigurationTypes(mMappedData.mSourceMap[nextID].listMainNotificationConfigurations);
		return (true);
	}
	else
	{
		sourceID = 0;
		logVerbose(__METHOD_NAME__,"Max limit reached.");
		return (false);
	}
}

am_Error_e CAmDatabaseHandlerMap::enterSourceDB(const am_Source_s & sourceData, am_sourceID_t & sourceID)
{
    if(sourceData.sourceID>=DYNAMIC_ID_BOUNDARY)
    {
    	logError(__METHOD_NAME__,"sourceID must be below:",DYNAMIC_ID_BOUNDARY);
    	return (E_NOT_POSSIBLE);
    }
    if(!existDomain(sourceData.domainID))
    {
    	logError(__METHOD_NAME__,"domainID must be valid");
    	return (E_NOT_POSSIBLE);
    }
    if(sourceData.name.empty())
    {
    	logError(__METHOD_NAME__,"sourceName must not be zero");
    	return (E_NOT_POSSIBLE);
    }
    if(!existSourceClass(sourceData.sourceClassID))
    {
    	logError(__METHOD_NAME__,"sourceClassID must be valid");
    	return (E_NOT_POSSIBLE);
    }

    if(!(sourceData.sourceState>=SS_UNKNNOWN && sourceData.sourceState<=SS_MAX))
    {
    	logError(__METHOD_NAME__,"sourceState must be valid");
    	return (E_NOT_POSSIBLE);
    }

    bool isFirstStatic = sourceData.sourceID == 0 && mFirstStaticSource;
    am_sourceID_t temp_SourceID = 0;
    am_sourceID_t temp_SourceIndex = 0;
    AmSource const *reservedSource = objectMatchingPredicate<AmSource, am_sourceID_t>(mMappedData.mSourceMap, [&](const AmSource & obj){
		return true==obj.reserved && obj.name.compare(sourceData.name)==0;
    });
	if( NULL != reservedSource )
	{
		am_sourceID_t oldSourceID = reservedSource->sourceID;
		mMappedData.mSourceMap[oldSourceID] = sourceData;
		mMappedData.mSourceMap[oldSourceID].reserved = 0;
		temp_SourceID = oldSourceID;
		temp_SourceIndex = oldSourceID;
	}
	else
	{
	    bool result;
	    if ( !isFirstStatic )
	    {
	        //check if the ID already exists
	    	 if (existSourceNameOrID(sourceData.sourceID, sourceData.name))
	    	 {
	    		sourceID = sourceData.sourceID;
	            return (E_ALREADY_EXISTS);
	    	 }
	    }
	    result = insertSourceDB(sourceData, temp_SourceID);
		if( false == result )
			return (E_UNKNOWN);
		temp_SourceIndex = temp_SourceID;
    }

    if ( isFirstStatic )
    {
      //if the first static sink is entered, we need to set it onto the boundary if needed
        mFirstStaticSource = false;
    }
    mMappedData.mSourceMap[temp_SourceIndex].sourceID = temp_SourceID;
    sourceID = temp_SourceID;

    logVerbose("DatabaseHandler::enterSourceDB entered new source with name", sourceData.name, "domainID:", sourceData.domainID, "classID:", sourceData.sourceClassID, "visible:", sourceData.visible, "assigned ID:", sourceID);

    NOTIFY_OBSERVERS1(dboNewSource, mMappedData.mSourceMap[temp_SourceIndex])

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::enterConnectionDB(const am_Connection_s& connection, am_connectionID_t& connectionID)
{
    if(connection.connectionID!=0)
    {
    	logError(__METHOD_NAME__,"connectionID must be 0!");
    	return (E_NOT_POSSIBLE);
    }

    if(!existSink(connection.sinkID))
    {
    	logError(__METHOD_NAME__,"sinkID must exist!");
    	return (E_NOT_POSSIBLE);
    }

    if(!existSource(connection.sourceID))
    {
    	logError(__METHOD_NAME__,"sourceID must exist!");
    	return (E_NOT_POSSIBLE);
    }
    //connection format is not checked, because it's project specific
    int16_t nextID = 0;
    if(mMappedData.increaseConnectionID(nextID))
	{
		connectionID = nextID;
		mMappedData.mConnectionMap[nextID] = connection;
		mMappedData.mConnectionMap[nextID].connectionID = nextID;
		mMappedData.mConnectionMap[nextID].reserved = true;
	}
	else
	{
		connectionID = 0;
		logVerbose(__METHOD_NAME__,"Max limit reached.");
		return (E_UNKNOWN);
	}

    logVerbose("DatabaseHandler::enterConnectionDB entered new connection sinkID=", connection.sinkID, "sourceID=", connection.sourceID, "connectionFormat=", connection.connectionFormat, "assigned ID=", connectionID);
    return (E_OK);
}

bool CAmDatabaseHandlerMap::insertSinkClassDB(const am_SinkClass_s & sinkClass, am_sinkClass_t & sinkClassID)
{
    int16_t nextID = 0;
	if(mMappedData.increaseID(nextID, mMappedData.mCurrentSinkClassesID, sinkClass.sinkClassID))
	{
		sinkClassID = nextID;
		mMappedData.mSinkClassesMap[nextID] = sinkClass;
		mMappedData.mSinkClassesMap[nextID].sinkClassID = nextID;
		return (true);
	}
	else
	{
		sinkClassID = 0;
		logVerbose(__METHOD_NAME__,"Max limit reached.");
		return (false);
	}
}

am_Error_e CAmDatabaseHandlerMap::enterSinkClassDB(const am_SinkClass_s & sinkClass, am_sinkClass_t & sinkClassID)
{
    if(sinkClass.sinkClassID>=DYNAMIC_ID_BOUNDARY)
    {
    	logError(__METHOD_NAME__,"sinkClassID must be <",DYNAMIC_ID_BOUNDARY);
    	return (E_NOT_POSSIBLE);
    }

    if(sinkClass.name.empty())
    {
    	logError(__METHOD_NAME__,"name must not be empty");
    	return (E_NOT_POSSIBLE);
    }

    am_sinkClass_t temp_SinkClassID = 0;
    am_sinkClass_t temp_SinkClassIndex = 0;

	bool result;
	if (sinkClass.sinkClassID != 0 || mFirstStaticSinkClass)
	{
		//check if the ID already exists
		 if (existSinkClass(sinkClass.sinkClassID))
		 {
			 sinkClassID = sinkClass.sinkClassID;
			return (E_ALREADY_EXISTS);
		 }
	}
	result = insertSinkClassDB(sinkClass, temp_SinkClassID);
	if( false == result )
		return (E_UNKNOWN);

	temp_SinkClassIndex = temp_SinkClassID;
	//if the ID is not created, we add it to the query
	if (sinkClass.sinkClassID == 0 && mFirstStaticSinkClass)
	{
		mFirstStaticSinkClass = false;
	}
	mMappedData.mSinkClassesMap[temp_SinkClassIndex].sinkClassID = temp_SinkClassID;
	sinkClassID = temp_SinkClassID;

    //todo:change last_insert implementations for multithreaded usage...
    logVerbose("DatabaseHandler::enterSinkClassDB entered new sinkClass");
    NOTIFY_OBSERVERS(dboNumberOfSinkClassesChanged)
    return (E_OK);
}

bool CAmDatabaseHandlerMap::insertSourceClassDB(am_sourceClass_t & sourceClassID, const am_SourceClass_s & sourceClass)
{
    int16_t nextID = 0;
	if(mMappedData.increaseID(nextID, mMappedData.mCurrentSourceClassesID, sourceClass.sourceClassID))
	{
		sourceClassID = nextID;
		mMappedData.mSourceClassesMap[nextID] = sourceClass;
		mMappedData.mSourceClassesMap[nextID].sourceClassID = nextID;
		return (true);
	}
	else
	{
		sourceClassID = 0;
		logVerbose(__METHOD_NAME__,"Max limit reached.");
		return (false);
	}
}

am_Error_e CAmDatabaseHandlerMap::enterSourceClassDB(am_sourceClass_t & sourceClassID, const am_SourceClass_s & sourceClass)
{
    if(sourceClass.sourceClassID>=DYNAMIC_ID_BOUNDARY)
    {
    	logError(__METHOD_NAME__,"sourceClassID must be <",DYNAMIC_ID_BOUNDARY);
    	return (E_NOT_POSSIBLE);
    }

    if(sourceClass.name.empty())
    {
    	logError(__METHOD_NAME__,"name must not be empty");
    	return (E_NOT_POSSIBLE);
    }


    am_sourceClass_t temp_SourceClassID = 0;
    am_sourceClass_t temp_SourceClassIndex = 0;

	bool result;
	if (sourceClass.sourceClassID != 0 || mFirstStaticSourceClass)
	{
		//check if the ID already exists
		if (existSourceClass(sourceClass.sourceClassID))
		{
			sourceClassID = sourceClass.sourceClassID;
			return (E_ALREADY_EXISTS);
		}
	}
	result = insertSourceClassDB(temp_SourceClassID, sourceClass);
	if( false == result )
		return (E_UNKNOWN);

	temp_SourceClassIndex = temp_SourceClassID;
	//if the ID is not created, we add it to the query
	if (sourceClass.sourceClassID == 0 && mFirstStaticSourceClass)
	{
		mFirstStaticSinkClass = false;
	}
	mMappedData.mSourceClassesMap[temp_SourceClassIndex].sourceClassID = temp_SourceClassID;
	sourceClassID = temp_SourceClassID;

	//todo:change last_insert implementations for multithread usage...

    logVerbose("DatabaseHandler::enterSourceClassDB entered new sourceClass");

    NOTIFY_OBSERVERS(dboNumberOfSourceClassesChanged)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::enterSystemProperties(const std::vector<am_SystemProperty_s> & listSystemProperties)
{
    if(listSystemProperties.empty())
	{
    	logError(__METHOD_NAME__,"listSystemProperties must not be empty");
    	return (E_NOT_POSSIBLE);
	}

    mMappedData.mSystemProperties = listSystemProperties;

    logVerbose("DatabaseHandler::enterSystemProperties entered system properties");
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeMainConnectionRouteDB(const am_mainConnectionID_t mainconnectionID, const std::vector<am_connectionID_t>& listConnectionID)
{
    if(mainconnectionID==0)
    {
    	logError(__METHOD_NAME__,"mainconnectionID must not be 0");
    	return (E_NOT_POSSIBLE);
    }

    if (!existMainConnection(mainconnectionID))
    {
    	logError(__METHOD_NAME__,"existMainConnection must exist");
        return (E_NON_EXISTENT);
    }

    int16_t delay = calculateDelayForRoute(listConnectionID);

    //now we replace the data in the main connection object with the new one
    mMappedData.mMainConnectionMap[mainconnectionID].listConnectionID = listConnectionID;

    if (changeDelayMainConnection(delay,mainconnectionID) == E_NO_CHANGE)
        logError("DatabaseHandler::changeMainConnectionRouteDB error while changing mainConnectionDelay to ", delay);

    logVerbose("DatabaseHandler::changeMainConnectionRouteDB entered new route:", mainconnectionID);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeMainConnectionStateDB(const am_mainConnectionID_t mainconnectionID, const am_ConnectionState_e connectionState)
{
    if(mainconnectionID==0)
    {
    	logError(__METHOD_NAME__,"mainconnectionID must not be 0");
    	return (E_NOT_POSSIBLE);
    }

    if(!(connectionState>=CS_UNKNOWN && connectionState<=CS_MAX))
    {
    	logError(__METHOD_NAME__,"connectionState must be valid");
    	return (E_NOT_POSSIBLE);
    }

    if (!existMainConnection(mainconnectionID))
    {
    	logError(__METHOD_NAME__,"existMainConnection must exist");
        return (E_NON_EXISTENT);
    }

    DB_COND_UPDATE_RIE(mMappedData.mMainConnectionMap[mainconnectionID].connectionState, connectionState);

    logVerbose("DatabaseHandler::changeMainConnectionStateDB changed mainConnectionState of MainConnection:", mainconnectionID, "to:", connectionState);
    NOTIFY_OBSERVERS2(dboMainConnectionStateChanged, mainconnectionID, connectionState)
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeSinkMainVolumeDB(const am_mainVolume_t mainVolume, const am_sinkID_t sinkID)
{
    if (!existSink(sinkID))
    {
    	logError(__METHOD_NAME__,"sinkID must exist");
        return (E_NON_EXISTENT);
    }

    DB_COND_UPDATE_RIE(mMappedData.mSinkMap[sinkID].mainVolume, mainVolume);

    logVerbose("DatabaseHandler::changeSinkMainVolumeDB changed mainVolume of sink:", sinkID, "to:", mainVolume);

    NOTIFY_OBSERVERS2(dboVolumeChanged, sinkID, mainVolume)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeSinkAvailabilityDB(const am_Availability_s & availability, const am_sinkID_t sinkID)
{
    if (!(availability.availability>=A_UNKNOWN && availability.availability<=A_MAX))
    {
    	logError(__METHOD_NAME__,"availability must be valid");
    	return (E_NOT_POSSIBLE);
    }

    if (!existSink(sinkID))
    {
    	logError(__METHOD_NAME__,"sinkID must exist");
        return (E_NON_EXISTENT);
    }

    DB_COND_UPDATE_RIE(mMappedData.mSinkMap[sinkID].available, availability);

    logVerbose("DatabaseHandler::changeSinkAvailabilityDB changed sinkAvailability of sink:", sinkID, "to:", availability.availability, "Reason:", availability.availabilityReason);

    if (sinkVisible(sinkID))
    {
    	NOTIFY_OBSERVERS2(dboSinkAvailabilityChanged,sinkID, availability)
    }
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeDomainStateDB(const am_DomainState_e domainState, const am_domainID_t domainID)
{

    if(!(domainState>=DS_UNKNOWN && domainState<=DS_MAX))
    {
    	logError(__METHOD_NAME__,"domainState must be valid");
    	return (E_NOT_POSSIBLE);
    }

    if (!existDomain(domainID))
    {
    	logError(__METHOD_NAME__,"domainID must exist");
        return (E_NON_EXISTENT);
    }

    DB_COND_UPDATE_RIE(mMappedData.mDomainMap[domainID].state, domainState);

    logVerbose("DatabaseHandler::changDomainStateDB changed domainState of domain:", domainID, "to:", domainState);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeSinkMuteStateDB(const am_MuteState_e muteState, const am_sinkID_t sinkID)
{

    if(!(muteState>=MS_UNKNOWN && muteState<=MS_MAX))
    {
    	logError(__METHOD_NAME__,"muteState must be valid");
    	return (E_NOT_POSSIBLE);
    }

    if (!existSink(sinkID))
    {
    	logError(__METHOD_NAME__,"sinkID must exist");
        return (E_NON_EXISTENT);
    }

    DB_COND_UPDATE_RIE(mMappedData.mSinkMap[sinkID].muteState, muteState);

    logVerbose("DatabaseHandler::changeSinkMuteStateDB changed sinkMuteState of sink:", sinkID, "to:", muteState);

    NOTIFY_OBSERVERS2(dboSinkMuteStateChanged, sinkID, muteState)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeMainSinkSoundPropertyDB(const am_MainSoundProperty_s & soundProperty, const am_sinkID_t sinkID)
{

    if (!existSink(sinkID))
    {
    	logError(__METHOD_NAME__,"sinkID must exist");
        return (E_NON_EXISTENT);
    }
    am_Sink_Database_s & sink = mMappedData.mSinkMap[sinkID];
    std::vector<am_MainSoundProperty_s>::iterator elementIterator = sink.listMainSoundProperties.begin();
	for (;elementIterator != sink.listMainSoundProperties.end(); ++elementIterator)
	{
		if (elementIterator->type == soundProperty.type)
		{
			DB_COND_UPDATE_RIE(elementIterator->value, soundProperty.value);
			if(sink.cacheMainSoundProperties.size())
				sink.cacheMainSoundProperties[soundProperty.type] = soundProperty.value;
			break;
		}
	}

    logVerbose("DatabaseHandler::changeMainSinkSoundPropertyDB changed MainSinkSoundProperty of sink:", sinkID, "type:", soundProperty.type, "to:", soundProperty.value);
    NOTIFY_OBSERVERS2(dboMainSinkSoundPropertyChanged, sinkID, soundProperty)
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeMainSourceSoundPropertyDB(const am_MainSoundProperty_s & soundProperty, const am_sourceID_t sourceID)
{

    if (!existSource(sourceID))
    {
    	logError(__METHOD_NAME__,"sourceID must exist");
        return (E_NON_EXISTENT);
    }
    am_Source_Database_s & source = mMappedData.mSourceMap.at(sourceID);
    std::vector<am_MainSoundProperty_s>::iterator elementIterator = source.listMainSoundProperties.begin();
	for (;elementIterator != source.listMainSoundProperties.end(); ++elementIterator)
	{
		if (elementIterator->type == soundProperty.type)
		{
			DB_COND_UPDATE_RIE(elementIterator->value, soundProperty.value);
			if(source.cacheMainSoundProperties.size())
				source.cacheMainSoundProperties[soundProperty.type] = soundProperty.value;
			break;
		}
	}

    logVerbose("DatabaseHandler::changeMainSourceSoundPropertyDB changed MainSinkSoundProperty of source:", sourceID, "type:", soundProperty.type, "to:", soundProperty.value);

    NOTIFY_OBSERVERS2(dboMainSourceSoundPropertyChanged, sourceID, soundProperty)
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeSourceAvailabilityDB(const am_Availability_s & availability, const am_sourceID_t sourceID)
{
    if(!(availability.availability>=A_UNKNOWN && availability.availability<=A_MAX))
    {
    	logError(__METHOD_NAME__,"availability must be valid");
    	return (E_NOT_POSSIBLE);
    }

    if (!existSource(sourceID))
    {
    	logError(__METHOD_NAME__,"sourceID must exist");
        return (E_NON_EXISTENT);
    }

    DB_COND_UPDATE_RIE(mMappedData.mSourceMap[sourceID].available, availability);

    logVerbose("DatabaseHandler::changeSourceAvailabilityDB changed changeSourceAvailabilityDB of source:", sourceID, "to:", availability.availability, "Reason:", availability.availabilityReason);

    if (sourceVisible(sourceID))
    {
    	NOTIFY_OBSERVERS2(dboSourceAvailabilityChanged, sourceID, availability)
    }
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeSystemPropertyDB(const am_SystemProperty_s & property)
{
    std::vector<am_SystemProperty_s>::iterator elementIterator = mMappedData.mSystemProperties.begin();
	for (;elementIterator != mMappedData.mSystemProperties.end(); ++elementIterator)
	{
		if (elementIterator->type == property.type)
			DB_COND_UPDATE_RIE(elementIterator->value, property.value);
	}

    logVerbose("DatabaseHandler::changeSystemPropertyDB changed system property");

    NOTIFY_OBSERVERS1(dboSystemPropertyChanged, property)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::removeMainConnectionDB(const am_mainConnectionID_t mainConnectionID)
{

    if (!existMainConnection(mainConnectionID))
    {
    	logError(__METHOD_NAME__,"mainConnectionID must exist");
        return (E_NON_EXISTENT);
    }

    DB_COND_UPDATE_INIT;
    DB_COND_UPDATE(mMappedData.mMainConnectionMap[mainConnectionID].mainConnectionID, CS_DISCONNECTED);
    if (DB_COND_ISMODIFIED)
        NOTIFY_OBSERVERS2(dboMainConnectionStateChanged, mainConnectionID, CS_DISCONNECTED)

    mMappedData.mMainConnectionMap.erase(mainConnectionID);
    logVerbose("DatabaseHandler::removeMainConnectionDB removed:", mainConnectionID);
    NOTIFY_OBSERVERS1(dboRemovedMainConnection, mainConnectionID)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::removeSinkDB(const am_sinkID_t sinkID)
{

    if (!existSink(sinkID))
    {
    	logError(__METHOD_NAME__,"sinkID must exist");
        return (E_NON_EXISTENT);
    }

    bool visible = sinkVisible(sinkID);

    mMappedData.mSinkMap.erase(sinkID);
    // todo: Check the tables SinkMainSoundProperty and SinkMainNotificationConfiguration with 'visible' set to true
    //if visible is true then delete SinkMainSoundProperty and SinkMainNotificationConfiguration ????
    logVerbose("DatabaseHandler::removeSinkDB removed:", sinkID);

    NOTIFY_OBSERVERS2(dboRemovedSink, sinkID, visible)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::removeSourceDB(const am_sourceID_t sourceID)
{

    if (!existSource(sourceID))
    {
    	logError(__METHOD_NAME__,"sourceID must exist");
        return (E_NON_EXISTENT);
    }

    bool visible = sourceVisible(sourceID);

    mMappedData.mSourceMap.erase(sourceID);

    // todo: Check the tables SourceMainSoundProperty and SourceMainNotificationConfiguration with 'visible' set to true
    //if visible is true then delete SourceMainSoundProperty and SourceMainNotificationConfiguration ????

    logVerbose("DatabaseHandler::removeSourceDB removed:", sourceID);
    NOTIFY_OBSERVERS2(dboRemovedSource, sourceID, visible)
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::removeGatewayDB(const am_gatewayID_t gatewayID)
{

    if (!existGateway(gatewayID))
    {
    	logError(__METHOD_NAME__,"gatewayID must exist");
        return (E_NON_EXISTENT);
    }

    mMappedData.mGatewayMap.erase(gatewayID);

    logVerbose("DatabaseHandler::removeGatewayDB removed:", gatewayID);
    NOTIFY_OBSERVERS1(dboRemoveGateway, gatewayID)
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::removeConverterDB(const am_converterID_t converterID)
{

    if (!existConverter(converterID))
    {
    	logError(__METHOD_NAME__,"converterID must exist");
        return (E_NON_EXISTENT);
    }

    mMappedData.mConverterMap.erase(converterID);

    logVerbose("DatabaseHandler::removeConverterDB removed:", converterID);
    NOTIFY_OBSERVERS1(dboRemoveConverter, converterID)
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::removeCrossfaderDB(const am_crossfaderID_t crossfaderID)
{

    if (!existCrossFader(crossfaderID))
    {
    	logError(__METHOD_NAME__,"crossfaderID must exist");
        return (E_NON_EXISTENT);
    }
    mMappedData.mCrossfaderMap.erase(crossfaderID);

    logVerbose("DatabaseHandler::removeCrossfaderDB removed:", crossfaderID);
    NOTIFY_OBSERVERS1(dboRemoveCrossfader, crossfaderID)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::removeDomainDB(const am_domainID_t domainID)
{

    if (!existDomain(domainID))
    {
    	logError(__METHOD_NAME__,"domainID must exist");
        return (E_NON_EXISTENT);
    }
    mMappedData.mDomainMap.erase(domainID);

    logVerbose("DatabaseHandler::removeDomainDB removed:", domainID);
    NOTIFY_OBSERVERS1(dboRemoveDomain, domainID)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::removeSinkClassDB(const am_sinkClass_t sinkClassID)
{

    if (!existSinkClass(sinkClassID))
    {
    	logError(__METHOD_NAME__,"sinkClassID must exist");
        return (E_NON_EXISTENT);
    }

    mMappedData.mSinkClassesMap.erase(sinkClassID);

    logVerbose("DatabaseHandler::removeSinkClassDB removed:", sinkClassID);
    NOTIFY_OBSERVERS(dboNumberOfSinkClassesChanged)
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::removeSourceClassDB(const am_sourceClass_t sourceClassID)
{

    if (!existSourceClass(sourceClassID))
    {
    	logError(__METHOD_NAME__,"sourceClassID must exist");
        return (E_NON_EXISTENT);
    }

    mMappedData.mSourceClassesMap.erase(sourceClassID);
    logVerbose("DatabaseHandler::removeSourceClassDB removed:", sourceClassID);
    NOTIFY_OBSERVERS(dboNumberOfSourceClassesChanged)
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::removeConnection(const am_connectionID_t connectionID)
{
    if (!existConnectionID(connectionID))
    {
    	logError(__METHOD_NAME__,"connectionID must exist",connectionID);
        return (E_NON_EXISTENT);
    }

    mMappedData.mConnectionMap.erase(connectionID);

    logVerbose("DatabaseHandler::removeConnection removed:", connectionID);
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getSourceClassInfoDB(const am_sourceID_t sourceID, am_SourceClass_s & classInfo) const
{

    if (!existSource(sourceID))
    {
    	logWarning(__METHOD_NAME__,"sourceID must exist");
        return (E_NON_EXISTENT);
    }
    am_Source_Database_s source = mMappedData.mSourceMap.at(sourceID);
    classInfo.sourceClassID  = source.sourceClassID;

    if (!existSourceClass(classInfo.sourceClassID))
    {
        return (E_NON_EXISTENT);
    }
    am_SourceClass_s tmpClass = mMappedData.mSourceClassesMap.at(classInfo.sourceClassID);
    classInfo = tmpClass;

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getSinkInfoDB(const am_sinkID_t sinkID, am_Sink_s & sinkData) const
{

    if (!existSink(sinkID))
    {
    	logWarning(__METHOD_NAME__,"sinkID",sinkID,"does not exist");
        return (E_NON_EXISTENT);
    }

    am_Sink_Database_s mappedSink = mMappedData.mSinkMap.at(sinkID);
    if( true == mappedSink.reserved )
    	return (E_NON_EXISTENT);
	sinkData = mappedSink;

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getSourceInfoDB(const am_sourceID_t sourceID, am_Source_s & sourceData) const
{

    if (!existSource(sourceID))
    {
    	logWarning(__METHOD_NAME__,"sourceID",sourceID,"does not exist");
        return (E_NON_EXISTENT);
    }

    am_Source_Database_s mappedSource = mMappedData.mSourceMap.at(sourceID);
    if( true == mappedSource.reserved )
    	return (E_NON_EXISTENT);

    sourceData = mappedSource;

    return (E_OK);
}

am_Error_e am::CAmDatabaseHandlerMap::getMainConnectionInfoDB(const am_mainConnectionID_t mainConnectionID, am_MainConnection_s & mainConnectionData) const
{
    if (!existMainConnection(mainConnectionID))
    {
    	logError(__METHOD_NAME__,"mainConnectionID must exist");
        return (E_NON_EXISTENT);
    }
    am_MainConnection_s temp = mMappedData.mMainConnectionMap.at(mainConnectionID);
    mainConnectionData = temp;

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeSinkClassInfoDB(const am_SinkClass_s& sinkClass)
{
    if(sinkClass.listClassProperties.empty())
    {
    	logError(__METHOD_NAME__,"listClassProperties must not be empty");
    	return (E_NOT_POSSIBLE);
    }

    //check if the ID already exists
    if (!existSinkClass(sinkClass.sinkClassID))
    {
    	logError(__METHOD_NAME__,"sinkClassID must exist");
        return (E_NON_EXISTENT);
    }

    DB_COND_UPDATE_RIE(mMappedData.mSinkClassesMap[sinkClass.sinkClassID].listClassProperties, sinkClass.listClassProperties);

    logVerbose("DatabaseHandler::setSinkClassInfoDB set setSinkClassInfo");
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeSourceClassInfoDB(const am_SourceClass_s& sourceClass)
{
    if(sourceClass.listClassProperties.empty())
    {
		logError(__METHOD_NAME__,"listClassProperties must not be empty");
		return (E_NOT_POSSIBLE);
    }

    //check if the ID already exists
    if (!existSourceClass(sourceClass.sourceClassID))
    {
    	logError(__METHOD_NAME__,"sourceClassID must exist");
        return (E_NON_EXISTENT);
    }

    DB_COND_UPDATE_RIE(mMappedData.mSourceClassesMap[sourceClass.sourceClassID].listClassProperties, sourceClass.listClassProperties);

    logVerbose("DatabaseHandler::setSinkClassInfoDB set setSinkClassInfo");
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getSinkClassInfoDB(const am_sinkID_t sinkID, am_SinkClass_s & sinkClass) const
{

    if (!existSink(sinkID))
    {
    	logWarning(__METHOD_NAME__,"sinkID must exist");
        return (E_NON_EXISTENT);
    }
    am_Sink_Database_s sink = mMappedData.mSinkMap.at(sinkID);
    sinkClass.sinkClassID  = sink.sinkClassID;

    if (!existSinkClass(sinkClass.sinkClassID))
    {
    	logWarning(__METHOD_NAME__,"sinkClassID must exist");
        return (E_NON_EXISTENT);
    }
    am_SinkClass_s tmpSinkClass = mMappedData.mSinkClassesMap.at(sinkClass.sinkClassID);
    sinkClass = tmpSinkClass;

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getGatewayInfoDB(const am_gatewayID_t gatewayID, am_Gateway_s & gatewayData) const
{
    if (!existGateway(gatewayID))
    {
    	logWarning(__METHOD_NAME__,"gatewayID must exist");
        return (E_NON_EXISTENT);
    }

    gatewayData = mMappedData.mGatewayMap.at(gatewayID);

    return (E_OK);

}

am_Error_e CAmDatabaseHandlerMap::getConverterInfoDB(const am_converterID_t converterID, am_Converter_s& converterData) const
{
    if (!existConverter(converterID))
    {
    	logWarning(__METHOD_NAME__,"converterID must exist");
        return (E_NON_EXISTENT);
    }

    converterData = mMappedData.mConverterMap.at(converterID);

    return (E_OK);

}

am_Error_e CAmDatabaseHandlerMap::getCrossfaderInfoDB(const am_crossfaderID_t crossfaderID, am_Crossfader_s & crossfaderData) const
{
    if (!existCrossFader(crossfaderID))
    {
    	logWarning(__METHOD_NAME__,"crossfaderID must exist");
        return (E_NON_EXISTENT);
    }

    crossfaderData = mMappedData.mCrossfaderMap.at(crossfaderID);

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListSinksOfDomain(const am_domainID_t domainID, std::vector<am_sinkID_t> & listSinkID) const
{
    listSinkID.clear();
    if (!existDomain(domainID))
    {
    	logWarning(__METHOD_NAME__,"domainID must exist");
        return (E_NON_EXISTENT);
    }

    std::unordered_map<am_sinkID_t, am_Sink_Database_s>::const_iterator elementIterator = mMappedData.mSinkMap.begin();
	for (;elementIterator != mMappedData.mSinkMap.end(); ++elementIterator)
	{
		if (0==elementIterator->second.reserved && domainID==elementIterator->second.domainID)
			listSinkID.push_back(elementIterator->second.sinkID);
	}
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListSourcesOfDomain(const am_domainID_t domainID, std::vector<am_sourceID_t> & listSourceID) const
{
    listSourceID.clear();
    if (!existDomain(domainID))
    {
    	logWarning(__METHOD_NAME__,"domainID must exist");
        return (E_NON_EXISTENT);
    }
     AmMapSource::const_iterator elementIterator = mMappedData.mSourceMap.begin();
	for (;elementIterator != mMappedData.mSourceMap.end(); ++elementIterator)
	{
		if (0==elementIterator->second.reserved && domainID==elementIterator->second.domainID)
			listSourceID.push_back(elementIterator->second.sourceID);
	}

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListCrossfadersOfDomain(const am_domainID_t domainID, std::vector<am_crossfaderID_t> & listCrossfader) const
{
    listCrossfader.clear();
    if (!existDomain(domainID))
    {
    	logWarning(__METHOD_NAME__,"domainID must exist");
        return (E_NON_EXISTENT);
    }

    AmMapSource::const_iterator sourceIterator = mMappedData.mSourceMap.begin();
	for (;sourceIterator != mMappedData.mSourceMap.end(); ++sourceIterator)
	{
		if (domainID==sourceIterator->second.domainID)
		{
			AmMapCrossfader::const_iterator elementIterator = mMappedData.mCrossfaderMap.begin();
			for (;elementIterator != mMappedData.mCrossfaderMap.end(); ++elementIterator)
			{
				if ( sourceIterator->second.sourceID==elementIterator->second.sourceID )
					listCrossfader.push_back(elementIterator->second.crossfaderID);
			}
		}
	}

    return (E_OK);

}

am_Error_e CAmDatabaseHandlerMap::getListGatewaysOfDomain(const am_domainID_t domainID, std::vector<am_gatewayID_t> & listGatewaysID) const
{
    listGatewaysID.clear();
    if (!existDomain(domainID))
    {
    	logWarning(__METHOD_NAME__,"domainID must exist");
        return (E_NON_EXISTENT);
    }

    AmMapGateway::const_iterator elementIterator = mMappedData.mGatewayMap.begin();
 	for (;elementIterator != mMappedData.mGatewayMap.end(); ++elementIterator)
 	{
 		if (domainID==elementIterator->second.controlDomainID)
 			listGatewaysID.push_back(elementIterator->second.gatewayID);
 	}
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListConvertersOfDomain(const am_domainID_t domainID, std::vector<am_converterID_t>& listConvertersID) const
{
    listConvertersID.clear();
    if (!existDomain(domainID))
    {
    	logWarning(__METHOD_NAME__,"domainID must exist");
        return (E_NON_EXISTENT);
    }

    AmMapConverter::const_iterator elementIterator = mMappedData.mConverterMap.begin();
 	for (;elementIterator != mMappedData.mConverterMap.end(); ++elementIterator)
 	{
 		if (domainID==elementIterator->second.domainID)
 			listConvertersID.push_back(elementIterator->second.converterID);
 	}
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListMainConnections(std::vector<am_MainConnection_s> & listMainConnections) const
{
    listMainConnections.clear();

    AmMapMainConnection::const_iterator elementIterator = mMappedData.mMainConnectionMap.begin();
    for (;elementIterator != mMappedData.mMainConnectionMap.end(); ++elementIterator)
    {
    	listMainConnections.push_back(elementIterator->second);
    }

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListDomains(std::vector<am_Domain_s> & listDomains) const
{
    listDomains.clear();

    AmMapDomain::const_iterator elementIterator = mMappedData.mDomainMap.begin();
     for (;elementIterator != mMappedData.mDomainMap.end(); ++elementIterator)
     {
    	 if( 0==elementIterator->second.reserved )
    		 listDomains.push_back(elementIterator->second);
     }

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListConnections(std::vector<am_Connection_s> & listConnections) const
{
    listConnections.clear();

    AmMapConnection::const_iterator elementIterator = mMappedData.mConnectionMap.begin();
	for (;elementIterator != mMappedData.mConnectionMap.end(); ++elementIterator)
	{
		if( 0==elementIterator->second.reserved )
			listConnections.push_back(elementIterator->second);
	}

      return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListConnectionsReserved(std::vector<am_Connection_s> & listConnections) const
{
    listConnections.clear();

    AmMapConnection::const_iterator elementIterator = mMappedData.mConnectionMap.begin();
	for (;elementIterator != mMappedData.mConnectionMap.end(); ++elementIterator)
	{
		if( elementIterator->second.reserved )
			listConnections.push_back(elementIterator->second);
	}

      return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListSinks(std::vector<am_Sink_s> & listSinks) const
{
    listSinks.clear();

	std::for_each(mMappedData.mSinkMap.begin(), mMappedData.mSinkMap.end(), [&](const std::pair<am_sinkID_t, am_Sink_Database_s>& ref) {
		if( 0==ref.second.reserved )
			listSinks.push_back(ref.second);
	});

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListSources(std::vector<am_Source_s> & listSources) const
{
    listSources.clear();

    std::for_each(mMappedData.mSourceMap.begin(), mMappedData.mSourceMap.end(), [&](const std::pair<am_sourceID_t, am_Source_Database_s>& ref) {
    		if( 0==ref.second.reserved )
    		{
    			listSources.push_back(ref.second);
    		}
    	});
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListSourceClasses(std::vector<am_SourceClass_s> & listSourceClasses) const
{
    listSourceClasses.clear();

    std::for_each(mMappedData.mSourceClassesMap.begin(), mMappedData.mSourceClassesMap.end(), [&](const std::pair<am_sourceClass_t, am_SourceClass_s>& ref) {
    	listSourceClasses.push_back(ref.second);
     });

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListCrossfaders(std::vector<am_Crossfader_s> & listCrossfaders) const
{
    listCrossfaders.clear();

    std::for_each(mMappedData.mCrossfaderMap.begin(), mMappedData.mCrossfaderMap.end(), [&](const std::pair<am_crossfaderID_t, am_Crossfader_s>& ref) {
    	listCrossfaders.push_back(ref.second);
       });

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListGateways(std::vector<am_Gateway_s> & listGateways) const
{
    listGateways.clear();

    std::for_each(mMappedData.mGatewayMap.begin(), mMappedData.mGatewayMap.end(), [&](const std::pair<am_gatewayID_t, am_Gateway_s>& ref) {
    	listGateways.push_back(ref.second);
       });

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListConverters(std::vector<am_Converter_s> & listConverters) const
{
	listConverters.clear();

    std::for_each(mMappedData.mConverterMap.begin(), mMappedData.mConverterMap.end(), [&](const std::pair<am_converterID_t, am_Converter_s>& ref) {
    	listConverters.push_back(ref.second);
    });

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListSinkClasses(std::vector<am_SinkClass_s> & listSinkClasses) const
{
    listSinkClasses.clear();

    std::for_each(mMappedData.mSinkClassesMap.begin(), mMappedData.mSinkClassesMap.end(), [&](const std::pair<am_gatewayID_t, am_SinkClass_s>& ref) {
    	   listSinkClasses.push_back(ref.second);
       });

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListVisibleMainConnections(std::vector<am_MainConnectionType_s> & listConnections) const
{
    listConnections.clear();
    std::for_each(mMappedData.mMainConnectionMap.begin(), mMappedData.mMainConnectionMap.end(), [&](const std::pair<am_mainConnectionID_t, am_MainConnection_Database_s>& ref) {
    	listConnections.emplace_back();
		ref.second.getMainConnectionType(listConnections.back());
    });

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListMainSinks(std::vector<am_SinkType_s> & listMainSinks) const
{
    listMainSinks.clear();
    std::for_each(mMappedData.mSinkMap.begin(), mMappedData.mSinkMap.end(), [&](const std::pair<am_sinkID_t, am_Sink_Database_s>& ref) {
    	if( 0==ref.second.reserved && 1==ref.second.visible )
    	{
    		listMainSinks.emplace_back();
    		ref.second.getSinkType(listMainSinks.back());
    	}
    });

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListMainSources(std::vector<am_SourceType_s> & listMainSources) const
{
    listMainSources.clear();
    std::for_each(mMappedData.mSourceMap.begin(), mMappedData.mSourceMap.end(), [&](const std::pair<am_sourceID_t, am_Source_Database_s>& ref) {
    	if( 0==ref.second.reserved && 1==ref.second.visible )
    	{
			listMainSources.emplace_back();
    		ref.second.getSourceType(listMainSources.back());
    	}
    });

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListMainSinkSoundProperties(const am_sinkID_t sinkID, std::vector<am_MainSoundProperty_s> & listSoundProperties) const
{
    if (!existSink(sinkID))
    {
    	logWarning(__METHOD_NAME__,"sinkID must exist");
    	return E_NON_EXISTENT;
    }

    const am_Sink_s & sink = mMappedData.mSinkMap.at(sinkID);
    listSoundProperties = sink.listMainSoundProperties;

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListMainSourceSoundProperties(const am_sourceID_t sourceID, std::vector<am_MainSoundProperty_s> & listSourceProperties) const
{
    if (!existSource(sourceID))
    {
    	logWarning(__METHOD_NAME__,"sourceID must exist");
    	return E_NON_EXISTENT;
    }
    const am_Source_s & source = mMappedData.mSourceMap.at(sourceID);
    listSourceProperties = source.listMainSoundProperties;

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListSinkSoundProperties(const am_sinkID_t sinkID, std::vector<am_SoundProperty_s>& listSoundproperties) const
{
    if (!existSink(sinkID))
    {
    	logWarning(__METHOD_NAME__,"sinkID must exist");
    	return E_NON_EXISTENT;
    }

    const am_Sink_Database_s & sink = mMappedData.mSinkMap.at(sinkID);
    listSoundproperties = sink.listSoundProperties;

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListSourceSoundProperties(const am_sourceID_t sourceID, std::vector<am_SoundProperty_s>& listSoundproperties) const
{
	if (!existSource(sourceID))
    {
		logWarning(__METHOD_NAME__,"sourceID must exist");
    	return E_NON_EXISTENT;
    }

	const am_Source_Database_s & source = mMappedData.mSourceMap.at(sourceID);
	listSoundproperties = source.listSoundProperties;

	return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListSystemProperties(std::vector<am_SystemProperty_s> & listSystemProperties) const
{
     listSystemProperties = mMappedData.mSystemProperties;
    return (E_OK);
}

am_Error_e am::CAmDatabaseHandlerMap::getListSinkConnectionFormats(const am_sinkID_t sinkID, std::vector<am_CustomConnectionFormat_t> & listConnectionFormats) const
{
   if (!existSink(sinkID))
   {
	   logWarning(__METHOD_NAME__,"sinkID must exist");
		return E_NON_EXISTENT;
   }
	const am_Sink_s & sink = mMappedData.mSinkMap.at(sinkID);
	listConnectionFormats = sink.listConnectionFormats;

    return (E_OK);
}

am_Error_e am::CAmDatabaseHandlerMap::getListSourceConnectionFormats(const am_sourceID_t sourceID, std::vector<am_CustomConnectionFormat_t> & listConnectionFormats) const
{
   if (!existSource(sourceID))
   {
	   logWarning(__METHOD_NAME__,"sourceID must exist");
		return E_NON_EXISTENT;
   }
    const am_Source_s & source = mMappedData.mSourceMap.at(sourceID);
    listConnectionFormats = source.listConnectionFormats;

    return (E_OK);
}

am_Error_e am::CAmDatabaseHandlerMap::getListGatewayConnectionFormats(const am_gatewayID_t gatewayID, std::vector<bool> & listConnectionFormat) const
{
   if (!existGateway(gatewayID))
   {
	   logWarning(__METHOD_NAME__,"gatewayID must exist");
		return E_NON_EXISTENT;
   }
    ListConnectionFormat::const_iterator iter = mListConnectionFormat.begin();
    iter = mListConnectionFormat.find(gatewayID);
    if (iter == mListConnectionFormat.end())
    {
    	logWarning("DatabaseHandler::getListGatewayConnectionFormats database error with convertionFormat");

        return E_NON_EXISTENT;
    }
    listConnectionFormat = iter->second;

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getTimingInformation(const am_mainConnectionID_t mainConnectionID, am_timeSync_t & delay) const
{
    if (!existMainConnection(mainConnectionID))
    {
    	logWarning(__METHOD_NAME__,"mainConnectionID must exist");
 		return E_NON_EXISTENT;
    }
    delay = -1;

    const am_MainConnection_s & mainConnection = mMappedData.mMainConnectionMap.at(mainConnectionID);
    delay = mainConnection.delay;

    if (delay == -1)
        return (E_NOT_POSSIBLE);

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeDelayMainConnection(const am_timeSync_t & delay, const am_mainConnectionID_t & connectionID)
{
    if (!existMainConnection(connectionID))
    {
 		logError(__METHOD_NAME__,"connectionID must exist");
 		return E_NON_EXISTENT;
    }
    DB_COND_UPDATE_RIE(mMappedData.mMainConnectionMap[connectionID].delay, delay);
    NOTIFY_OBSERVERS2(dboTimingInformationChanged, connectionID, delay)
    return (E_OK);
}

/**
 * checks for a certain mainConnection
 * @param mainConnectionID to be checked for
 * @return true if it exists
 */
bool CAmDatabaseHandlerMap::existMainConnection(const am_mainConnectionID_t mainConnectionID) const
{
	return existsObjectWithKeyInMap(mainConnectionID, mMappedData.mMainConnectionMap);
}

/**
 * checks for a certain Source
 * @param sourceID to be checked for
 * @return true if it exists
 */
bool CAmDatabaseHandlerMap::existSource(const am_sourceID_t sourceID) const
{
	am_Source_Database_s const * source = objectForKeyIfExistsInMap(sourceID, mMappedData.mSourceMap);
	if( NULL!=source )
		return (0==source->reserved);

	return false;
}

/**
 * checks if a source name or ID exists
 * @param sourceID the sourceID
 * @param name the name
 * @return true if it exits
 */
bool CAmDatabaseHandlerMap::existSourceNameOrID(const am_sourceID_t sourceID, const std::string & name) const
{
    return sourceWithNameOrID(sourceID, name);
}

/**
 * checks if a name exits
 * @param name the name
 * @return true if it exits
 */
bool CAmDatabaseHandlerMap::existSourceName(const std::string & name) const
{
    return existSourceNameOrID(mMappedData.mCurrentSourceID.mMax, name);
}

/**
 * checks for a certain Sink
 * @param sinkID to be checked for
 * @return true if it exists
 */
bool CAmDatabaseHandlerMap::existSink(const am_sinkID_t sinkID) const
{
	bool returnVal = false;
	AmMapSink::const_iterator elementIterator = mMappedData.mSinkMap.begin();
	for (;elementIterator != mMappedData.mSinkMap.end(); ++elementIterator)
	{
		if( 0==elementIterator->second.reserved &&
			sinkID==elementIterator->second.sinkID)
		{
			returnVal = true;
			break;
		}
	}
    return (returnVal);
}

/**
 * returns source with given ID or the name if exists
 * @param sourceID the ID
 * @param name the name
 * @return source structure if exists.
 */
const CAmDatabaseHandlerMap::am_Source_Database_s *  CAmDatabaseHandlerMap::sourceWithNameOrID(const am_sourceID_t sourceID, const std::string & name) const
{
	std::function<bool(const CAmDatabaseHandlerMap::am_Source_Database_s & refObject)> comparator = [&](const CAmDatabaseHandlerMap::am_Source_Database_s & source)->bool{
			return ( 0==source.reserved &&
					(sourceID==source.sourceID || name.compare(source.name)==0));
	};
	return objectMatchingPredicate(mMappedData.mSourceMap, comparator);
}

/**
 * returns sink with given ID or the name if exists
 * @param sinkID the ID
 * @param name the name
 * @return sink structure if exists.
 */
const CAmDatabaseHandlerMap::am_Sink_Database_s * CAmDatabaseHandlerMap::sinkWithNameOrID(const am_sinkID_t sinkID, const std::string & name) const
{
	std::function<bool(const CAmDatabaseHandlerMap::am_Sink_Database_s & refObject)> comparator = [&](const CAmDatabaseHandlerMap::am_Sink_Database_s & sink)->bool{
			return ( 0==sink.reserved &&
					(sinkID==sink.sinkID || name.compare(sink.name)==0));
	};
	return objectMatchingPredicate(mMappedData.mSinkMap, comparator);
}

/**
 * checks if a sink with the ID or the name exists
 * @param sinkID the ID
 * @param name the name
 * @return true if it exists.
 */
bool CAmDatabaseHandlerMap::existSinkNameOrID(const am_sinkID_t sinkID, const std::string & name) const
{
    return sinkWithNameOrID( sinkID,  name)!=NULL;
}

/**
 * checks if a sink with the name exists
 * @param name the name
 * @return true if it exists
 */
bool CAmDatabaseHandlerMap::existSinkName(const std::string & name) const
{
    return existSinkNameOrID(mMappedData.mCurrentSinkID.mMax, name);
}

/**
 * checks for a certain domain
 * @param domainID to be checked for
 * @return true if it exists
 */
bool CAmDatabaseHandlerMap::existDomain(const am_domainID_t domainID) const
{
	am_Domain_Database_s const * source = objectForKeyIfExistsInMap(domainID, mMappedData.mDomainMap);
	if( NULL!=source )
		return (0==source->reserved);

	return false;
}

/**
 * checks for certain gateway
 * @param gatewayID to be checked for
 * @return true if it exists
 */
bool CAmDatabaseHandlerMap::existGateway(const am_gatewayID_t gatewayID) const
{
	return existsObjectWithKeyInMap(gatewayID, mMappedData.mGatewayMap);
}

bool CAmDatabaseHandlerMap::existConverter(const am_converterID_t converterID) const
{
	return existsObjectWithKeyInMap(converterID, mMappedData.mConverterMap);
}

am_Error_e CAmDatabaseHandlerMap::getDomainOfSource(const am_sourceID_t sourceID, am_domainID_t & domainID) const
{
    domainID=0;

    am_Source_Database_s const * source = objectForKeyIfExistsInMap(sourceID, mMappedData.mSourceMap);
    if( NULL!=source )
    {
    	domainID = source->domainID;
    	return E_OK;
    }
    return E_NON_EXISTENT;
}

am_Error_e am::CAmDatabaseHandlerMap::getDomainOfSink(const am_sinkID_t sinkID, am_domainID_t & domainID) const
{
    domainID=0;

    am_Sink_Database_s const * source = objectForKeyIfExistsInMap(sinkID, mMappedData.mSinkMap);
	if( NULL!=source )
	{
		domainID = source->domainID;
		return E_OK;
	}
	return E_NON_EXISTENT;
}

am_Error_e am::CAmDatabaseHandlerMap::getDomainOfCrossfader(const am_converterID_t crossfader, am_domainID_t & domainID) const
{
    domainID=0;

    am_Crossfader_Database_s const * cross = objectForKeyIfExistsInMap(crossfader, mMappedData.mCrossfaderMap);
	if( NULL!=cross )
	{
		getDomainOfSource(cross->sinkID_A,domainID);
		return E_OK;
	}
	return E_NON_EXISTENT;
}

/**
 * checks for certain SinkClass
 * @param sinkClassID
 * @return true if it exists
 */
bool CAmDatabaseHandlerMap::existSinkClass(const am_sinkClass_t sinkClassID) const
{
	return existsObjectWithKeyInMap(sinkClassID, mMappedData.mSinkClassesMap);
}

/**
 * checks for certain sourceClass
 * @param sourceClassID
 * @return true if it exists
 */
bool CAmDatabaseHandlerMap::existSourceClass(const am_sourceClass_t sourceClassID) const
{
	return existsObjectWithKeyInMap(sourceClassID, mMappedData.mSourceClassesMap);
}

am_Error_e CAmDatabaseHandlerMap::changeConnectionTimingInformation(const am_connectionID_t connectionID, const am_timeSync_t delay)
{
     if(!existConnectionID(connectionID))
     {
     	logError(__METHOD_NAME__,"connectionID must exist");
         return (E_NON_EXISTENT);
     }

    mMappedData.mConnectionMap[connectionID].delay = delay;

    //now we need to find all mainConnections that use the changed connection and update their timing

    //first get all route tables for all mainconnections
    am_Error_e error = E_OK;
    AmMapMainConnection::const_iterator iter = mMappedData.mMainConnectionMap.begin();
    for(; iter != mMappedData.mMainConnectionMap.end(); ++iter)
    {
        const am_MainConnection_s & mainConnection = iter->second;
        if (std::find(mainConnection.listConnectionID.begin(), mainConnection.listConnectionID.end(), connectionID) != mainConnection.listConnectionID.end())
        {
            // Got it.
            error = changeDelayMainConnection(calculateMainConnectionDelay(mainConnection.mainConnectionID), mainConnection.mainConnectionID);
        }
    }

    return error;
}

am_Error_e CAmDatabaseHandlerMap::changeConnectionFinal(const am_connectionID_t connectionID)
{
    am_Connection_Database_s const * connection = objectForKeyIfExistsInMap(connectionID, mMappedData.mConnectionMap);
    if( NULL!=connection )
    {
    	mMappedData.mConnectionMap.at(connectionID).reserved = false;
    	return E_OK;
    }
	logError(__METHOD_NAME__,"connectionID must exist");
    return (E_NON_EXISTENT);
}

am_timeSync_t CAmDatabaseHandlerMap::calculateMainConnectionDelay(const am_mainConnectionID_t mainConnectionID) const
{
    if (!existMainConnection(mainConnectionID))
  		return -1;
    const am_MainConnection_s & mainConnection = mMappedData.mMainConnectionMap.at(mainConnectionID);
    am_timeSync_t delay = 0;
    std::vector<am_connectionID_t>::const_iterator iter = mainConnection.listConnectionID.begin();
	for(;iter<mainConnection.listConnectionID.end(); ++iter)
	{
		am_Connection_Database_s const * source = objectForKeyIfExistsInMap(*iter, mMappedData.mConnectionMap);
		if( NULL!=source )
		{
			delay += std::max(source->delay, static_cast<am_timeSync_t>(0));
		}
	}
    return (delay == 0 ? -1 : std::min(delay, static_cast<am_timeSync_t>(SHRT_MAX)));
}

/**
 * registers the Observer at the Database
 * @param iObserver pointer to the observer
 */

/**
 * gives information about the visibility of a source
 * @param sourceID the sourceID
 * @return true if source is visible
 */
bool CAmDatabaseHandlerMap::sourceVisible(const am_sourceID_t sourceID) const
{
    if (!existSource(sourceID))
    {
    	logError(__METHOD_NAME__,"sourceID must exist");
    	return false;
    }
    am_Source_Database_s source = mMappedData.mSourceMap.at(sourceID);
    return source.visible;
}

/**
 * gives information about the visibility of a sink
 * @param sinkID the sinkID
 * @return true if source is visible
 */
bool CAmDatabaseHandlerMap::sinkVisible(const am_sinkID_t sinkID) const
{
	am_Sink_Database_s const * source = objectForKeyIfExistsInMap(sinkID, mMappedData.mSinkMap);
	if( NULL!=source )
	{
		if(0==source->reserved)
			return source->visible;
	}
	return false;
}

/**
 * checks if a connection already exists.
 * Only takes sink, source and format information for search!
 * @param connection the connection to be checked
 * @return true if connections exists
 */
bool CAmDatabaseHandlerMap::existConnection(const am_Connection_s & connection) const
{
	am_Connection_Database_s const * connectionObject = objectMatchingPredicate<am_Connection_Database_s, am_connectionID_t>(mMappedData.mConnectionMap, [&](const am_Connection_Database_s & obj){
		return false==obj.reserved &&
				connection.sinkID == obj.sinkID &&
				connection.sourceID == obj.sourceID &&
				connection.connectionFormat == obj.connectionFormat;
    });
	return ( NULL!=connectionObject );
}

/**
 * checks if a connection with the given ID exists
 * @param connectionID
 * @return true if connection exits
 */
bool CAmDatabaseHandlerMap::existConnectionID(const am_connectionID_t connectionID) const
{
	am_Connection_Database_s const * connection = objectForKeyIfExistsInMap(connectionID, mMappedData.mConnectionMap);
	if( NULL!=connection )
	{
		return (true);
	}
	return false;
}

/**
 * checks if a CrossFader exists
 * @param crossfaderID the ID of the crossfader to be checked
 * @return true if exists
 */
bool CAmDatabaseHandlerMap::existCrossFader(const am_crossfaderID_t crossfaderID) const
{
     return existsObjectWithKeyInMap(crossfaderID, mMappedData.mCrossfaderMap);
}

am_Error_e CAmDatabaseHandlerMap::getSoureState(const am_sourceID_t sourceID, am_SourceState_e & sourceState) const
{
	am_Source_Database_s const * source = objectForKeyIfExistsInMap(sourceID, mMappedData.mSourceMap);
	if( NULL!=source )
	{
		sourceState = source->sourceState;
		return (E_OK);
	}
	else
	{
		sourceState =  SS_UNKNNOWN;
		return (E_NON_EXISTENT);
	}
}

am_Error_e CAmDatabaseHandlerMap::changeSourceState(const am_sourceID_t sourceID, const am_SourceState_e sourceState)
{
    if(!(sourceState>=SS_UNKNNOWN && sourceState<=SS_MAX))
	{
		logError(__METHOD_NAME__,"sourceState must be valid");
		return (E_NOT_POSSIBLE);
	}

    if(existSource(sourceID))
    {
    	mMappedData.mSourceMap.at(sourceID).sourceState = sourceState;
		return (E_OK);
	}
    logError(__METHOD_NAME__,"sourceID must exist");
	return (E_NON_EXISTENT);
}

am_Error_e CAmDatabaseHandlerMap::changeSourceInterruptState(const am_sourceID_t sourceID, const am_InterruptState_e interruptState)
{
    assert(sourceID!=0);
    assert(interruptState>=IS_UNKNOWN && interruptState<=IS_MAX);
    if(existSource(sourceID))
    {
        mMappedData.mSourceMap.at(sourceID).interruptState = interruptState;
        return (E_OK);
    }
    return (E_NON_EXISTENT);
}


am_Error_e CAmDatabaseHandlerMap::getSinkMainVolume(const am_sinkID_t sinkID, am_mainVolume_t& mainVolume) const {


	am_Sink_Database_s const * source = objectForKeyIfExistsInMap(sinkID, mMappedData.mSinkMap);
	if( NULL!=source )
	{
		mainVolume = source->mainVolume;
		return (E_OK);
	}
	mainVolume = -1;
	logWarning(__METHOD_NAME__,"sinkID must be valid");
	return (E_NON_EXISTENT);
}

am_Error_e CAmDatabaseHandlerMap::getSinkVolume(const am_sinkID_t sinkID, am_volume_t & volume) const
{

	am_Sink_Database_s const * source = objectForKeyIfExistsInMap(sinkID, mMappedData.mSinkMap);
	if( NULL!=source )
	{
		volume = source->volume;
		return (E_OK);
	}
	volume = -1;
	logWarning(__METHOD_NAME__,"sinkID must be valid");
	return (E_NON_EXISTENT);
}

am_Error_e CAmDatabaseHandlerMap::getSourceVolume(const am_sourceID_t sourceID, am_volume_t & volume) const
{
	am_Source_Database_s const * source = objectForKeyIfExistsInMap(sourceID, mMappedData.mSourceMap);
	if( NULL!=source )
	{
		volume = source->volume;
		return (E_OK);
	}
	volume = -1;
	logWarning(__METHOD_NAME__,"sourceID must be valid");
	return (E_NON_EXISTENT);
}

am_Error_e CAmDatabaseHandlerMap::getSinkSoundPropertyValue(const am_sinkID_t sinkID, const am_CustomSoundPropertyType_t propertyType, int16_t & value) const
{

	am_Sink_Database_s * pObject = (am_Sink_Database_s *)objectForKeyIfExistsInMap(sinkID, mMappedData.mSinkMap);
	if( NULL!=pObject )
	{
		if(pObject->listSoundProperties.size()>0 && 0==pObject->cacheSoundProperties.size())
		{
			std::vector<am_SoundProperty_s>::const_iterator iter = pObject->listSoundProperties.begin();
			for(; iter<pObject->listSoundProperties.end(); ++iter)
				pObject->cacheSoundProperties[iter->type] = iter->value;
		}
		auto it = pObject->cacheSoundProperties.find(propertyType);
		if(it!=pObject->cacheSoundProperties.end())
		{
			value = it->second;
			return (E_OK);
		}
	}
	value = -1;
	logWarning(__METHOD_NAME__,"sinkID must be valid");
	return (E_NON_EXISTENT);
}

am_Error_e CAmDatabaseHandlerMap::getSourceSoundPropertyValue(const am_sourceID_t sourceID, const am_CustomSoundPropertyType_t propertyType, int16_t & value) const
{
	am_Source_Database_s * pObject = (am_Source_Database_s *)objectForKeyIfExistsInMap(sourceID, mMappedData.mSourceMap);
	if( NULL!=pObject )
	{
		if(pObject->listSoundProperties.size()>0 && 0==pObject->cacheSoundProperties.size())
		{
			std::vector<am_SoundProperty_s>::const_iterator iter = pObject->listSoundProperties.begin();
			for(; iter<pObject->listSoundProperties.end(); ++iter)
				pObject->cacheSoundProperties[iter->type] = iter->value;
		}
		auto it = pObject->cacheSoundProperties.find(propertyType);
		if(it!=pObject->cacheSoundProperties.end())
		{
			value = it->second;
			return (E_OK);
		}
	}
	value = -1;
	logWarning(__METHOD_NAME__,"sourceID must be valid");
	return (E_NON_EXISTENT);
}

am_Error_e CAmDatabaseHandlerMap::getMainSinkSoundPropertyValue(const am_sinkID_t sinkID, const am_CustomMainSoundPropertyType_t propertyType, int16_t& value) const
{
	am_Sink_Database_s * pObject = (am_Sink_Database_s *)objectForKeyIfExistsInMap(sinkID, mMappedData.mSinkMap);
	if( NULL!=pObject )
	{
		if(pObject->listMainSoundProperties.size()>0 && 0==pObject->cacheMainSoundProperties.size())
		{
			std::vector<am_MainSoundProperty_s>::const_iterator iter = pObject->listMainSoundProperties.begin();
			for(; iter<pObject->listMainSoundProperties.end(); ++iter)
				pObject->cacheMainSoundProperties[iter->type] = iter->value;
		}
		auto it = pObject->cacheMainSoundProperties.find(propertyType);
		if(it!=pObject->cacheMainSoundProperties.end())
		{
			value = it->second;
			return (E_OK);
		}
	}
	value = -1;
	logWarning(__METHOD_NAME__,"sinkID must be valid");
	return (E_NON_EXISTENT);
}

am_Error_e CAmDatabaseHandlerMap::getMainSourceSoundPropertyValue(const am_sourceID_t sourceID, const am_CustomMainSoundPropertyType_t propertyType, int16_t& value) const
{

	am_Source_Database_s * pObject = (am_Source_Database_s *)objectForKeyIfExistsInMap(sourceID, mMappedData.mSourceMap);
	if( NULL!=pObject )
	{
		if(pObject->listMainSoundProperties.size()>0 && 0==pObject->cacheMainSoundProperties.size())
		{
			std::vector<am_MainSoundProperty_s>::const_iterator iter = pObject->listMainSoundProperties.begin();
			for(; iter<pObject->listMainSoundProperties.end(); ++iter)
				pObject->cacheMainSoundProperties[iter->type] = iter->value;
		}
		auto it = pObject->cacheMainSoundProperties.find(propertyType);
		if(it!=pObject->cacheMainSoundProperties.end())
		{
			value = it->second;
			return (E_OK);
		}
	}

	value = -1;
	logWarning(__METHOD_NAME__,"sourceID must be valid");
	return (E_NON_EXISTENT);
}

am_Error_e CAmDatabaseHandlerMap::getDomainState(const am_domainID_t domainID, am_DomainState_e& state) const
{

    am_Domain_Database_s const * source = objectForKeyIfExistsInMap(domainID, mMappedData.mDomainMap);
	if( NULL!=source )
	{
		state = source->state;
		return (E_OK);
	}
	state = DS_UNKNOWN;
	logWarning(__METHOD_NAME__,"domainID must be valid");
	return (E_NON_EXISTENT);
}

am_Error_e CAmDatabaseHandlerMap::peekDomain(const std::string & name, am_domainID_t & domainID)
{
    domainID=0;

    am_Domain_Database_s const *reservedDomain = objectMatchingPredicate<am_Domain_Database_s, am_domainID_t>(mMappedData.mDomainMap, [&](const am_Domain_Database_s & obj){
		return name.compare(obj.name)==0;
    });

     if( NULL != reservedDomain )
    {
    	domainID = reservedDomain->domainID;
    	return E_OK;
    }
    else
    {
    	int16_t nextID = 0;
    	if( mMappedData.increaseID( nextID, mMappedData.mCurrentDomainID) )
    	{
    		domainID = nextID;
    		am_Domain_Database_s domain;
    		domain.domainID = nextID;
    		domain.name = name;
    		domain.reserved = 1;
    		mMappedData.mDomainMap[nextID] = domain;
    		return E_OK;
    	}
    	return E_UNKNOWN;
    }
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::peekSink(const std::string & name, am_sinkID_t & sinkID)
{
	am_Sink_Database_s const *reservedSink = objectMatchingPredicate<am_Sink_Database_s, am_sinkID_t>(mMappedData.mSinkMap, [&](const am_Sink_Database_s & obj){
		return name.compare(obj.name)==0;
    });
	if( NULL!=reservedSink )
    {
		sinkID = reservedSink->sinkID;
    	return E_OK;
    }
    else
    {
    	int16_t nextID = 0;
    	if(mMappedData.increaseID(nextID, mMappedData.mCurrentSinkID))
    	{
    		if(mFirstStaticSink)
    		{
    			nextID = DYNAMIC_ID_BOUNDARY;
    			mFirstStaticSink = false;
    		}
    		sinkID = nextID;
    		am_Sink_Database_s object;
    		object.sinkID = nextID;
    		object.name = name;
    		object.reserved = 1;
    		mMappedData.mSinkMap[nextID] = object;
    		return E_OK;
    	}
   		return E_UNKNOWN;
    }
}

am_Error_e CAmDatabaseHandlerMap::peekSource(const std::string & name, am_sourceID_t & sourceID)
{
	am_Source_Database_s const *reservedSrc = objectMatchingPredicate<am_Source_Database_s, am_sourceID_t>(mMappedData.mSourceMap, [&](const am_Source_Database_s & obj){
		return name.compare(obj.name)==0;
    });
	if( NULL!=reservedSrc )
    {
		sourceID = reservedSrc->sourceID;
    	return E_OK;
    }
    else
    {
    	int16_t nextID = 0;
    	if(mMappedData.increaseID(nextID, mMappedData.mCurrentSourceID))
    	{
    		if(mFirstStaticSource)
    		{
//    			nextID = DYNAMIC_ID_BOUNDARY;
    			mFirstStaticSource = false;
    		}
    		sourceID = nextID;
    		am_Source_Database_s object;
    		object.sourceID = nextID;
    		object.name = name;
    		object.reserved = 1;
    		mMappedData.mSourceMap[nextID] = object;
    		return E_OK;
    	}
    	else
    		return E_UNKNOWN;
    }
}

am_Error_e CAmDatabaseHandlerMap::changeSinkVolume(const am_sinkID_t sinkID, const am_volume_t volume)
{
    if (!existSink(sinkID))
    {
    	logError(__METHOD_NAME__,"sinkID must be valid");
        return (E_NON_EXISTENT);
    }

    mMappedData.mSinkMap[sinkID].volume = volume;
    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeSourceVolume(const am_sourceID_t sourceID, const am_volume_t volume)
{
    if (!existSource(sourceID))
    {
    	logError(__METHOD_NAME__,"sourceID must be valid");
        return (E_NON_EXISTENT);
    }
    mMappedData.mSourceMap[sourceID].volume = volume;

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeSourceSoundPropertyDB(const am_SoundProperty_s & soundProperty, const am_sourceID_t sourceID)
{
    if (!existSource(sourceID))
    {
    	logError(__METHOD_NAME__,"sourceID must be valid");
        return (E_NON_EXISTENT);
    }

    am_Source_Database_s & source = mMappedData.mSourceMap[sourceID];
    std::vector<am_SoundProperty_s>::iterator iter = source.listSoundProperties.begin();
	for(; iter<source.listSoundProperties.end(); ++iter)
	{
		if( soundProperty.type == iter->type )
		{
			iter->value = soundProperty.value;
			if(source.cacheSoundProperties.size())
				source.cacheSoundProperties[soundProperty.type] = soundProperty.value;
			return (E_OK);
		}
	}
	logError(__METHOD_NAME__,"soundproperty type must be valid source:",sourceID,"type",soundProperty.type);
	return (E_NON_EXISTENT);
}

am_Error_e CAmDatabaseHandlerMap::changeSinkSoundPropertyDB(const am_SoundProperty_s & soundProperty, const am_sinkID_t sinkID)
{

    if (!existSink(sinkID))
    {
    	logError(__METHOD_NAME__,"sinkID must be valid");
        return (E_NON_EXISTENT);
    }
    am_Sink_Database_s & sink = mMappedData.mSinkMap[sinkID];
    std::vector<am_SoundProperty_s>::iterator iter = sink.listSoundProperties.begin();
 	for(; iter<sink.listSoundProperties.end(); ++iter)
 	{
 		if( soundProperty.type == iter->type )
 		{
 			iter->value = soundProperty.value;
 			if(sink.cacheSoundProperties.size())
 				sink.cacheSoundProperties[soundProperty.type] = soundProperty.value;
 			return (E_OK);
 		}
 	}
	logError(__METHOD_NAME__,"soundproperty type must be valid sinkID:",sinkID,"type",soundProperty.type);
 	return (E_NON_EXISTENT);
}

am_Error_e CAmDatabaseHandlerMap::changeCrossFaderHotSink(const am_crossfaderID_t crossfaderID, const am_HotSink_e hotsink)
{

    if (!existCrossFader(crossfaderID))
    {
    	logError(__METHOD_NAME__,"crossfaderID must be valid");
        return (E_NON_EXISTENT);
    }

    mMappedData.mCrossfaderMap[crossfaderID].hotSink = hotsink;
    return (E_OK);
}

bool CAmDatabaseHandlerMap::isComponentConnected(const am_Gateway_s & gateway) const
{
	bool ret = isConnected(gateway);
	return ret;
}

bool CAmDatabaseHandlerMap::isComponentConnected(const am_Converter_s & converter) const
{
	bool ret = isConnected(converter);
	return ret;
}

am_Error_e am::CAmDatabaseHandlerMap::peekSinkClassID(const std::string & name, am_sinkClass_t & sinkClassID)
{
    if (name.empty())
        return (E_NON_EXISTENT);
    am_SinkClass_Database_s const *reserved = objectMatchingPredicate<am_SinkClass_Database_s, am_sinkClass_t>(mMappedData.mSinkClassesMap, [&](const am_SinkClass_Database_s & obj){
		return name.compare(obj.name)==0;
    });
	if( NULL!=reserved )
	{
		sinkClassID = reserved->sinkClassID;
		return E_OK;
	}
	return (E_NON_EXISTENT);
}

am_Error_e am::CAmDatabaseHandlerMap::peekSourceClassID(const std::string & name, am_sourceClass_t & sourceClassID)
{
    if (name.empty())
        return (E_NON_EXISTENT);
    am_SourceClass_Database_s const *ptrSource = objectMatchingPredicate<am_SourceClass_Database_s, am_sourceClass_t>(mMappedData.mSourceClassesMap, [&](const am_SourceClass_Database_s & obj){
		return name.compare(obj.name)==0;
    });
  	if( NULL!=ptrSource )
  	{
  		sourceClassID = ptrSource->sourceClassID;
  		return E_OK;
  	}
  	return (E_NON_EXISTENT);
}


am_Error_e CAmDatabaseHandlerMap::changeSourceDB(const am_sourceID_t sourceID, const am_sourceClass_t sourceClassID, const std::vector<am_SoundProperty_s>& listSoundProperties, const std::vector<am_CustomConnectionFormat_t>& listConnectionFormats, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties)
{

    if (!existSource(sourceID))
    {
    	logError(__METHOD_NAME__,"sourceID must be valid");
        return (E_NON_EXISTENT);
    }

    DB_COND_UPDATE_INIT;
    am_sourceClass_t sourceClassOut(sourceClassID);
    std::vector<am_MainSoundProperty_s> listMainSoundPropertiesOut(listMainSoundProperties);
    //check if sinkClass needs to be changed

    std::unordered_map<am_sourceID_t, am_Source_Database_s>::iterator iter = mMappedData.mSourceMap.begin();
    for(; iter!=mMappedData.mSourceMap.end(); ++iter)
    {
        if( iter->second.sourceID == sourceID )
        {
            if (sourceClassID != 0)
            {
                DB_COND_UPDATE(iter->second.sourceClassID, sourceClassID);
            }
            else if (0 == iter->second.reserved)
            {
                sourceClassOut = iter->second.sourceClassID;
            }
            break;
        }
    }

    //check if soundProperties need to be updated
    if (!listSoundProperties.empty())
    {
        mMappedData.mSourceMap.at(sourceID).listSoundProperties = listSoundProperties;
        mMappedData.mSourceMap.at(sourceID).cacheSoundProperties.clear();
    }

    //check if we have to update the list of connectionformats
    if (!listConnectionFormats.empty())
    {
        mMappedData.mSourceMap.at(sourceID).listConnectionFormats = listConnectionFormats;
    }

    //then we need to check if we need to update the listMainSoundProperties
    if (sourceVisible(sourceID))
    {
        if (!listMainSoundProperties.empty())
        {
            DB_COND_UPDATE(mMappedData.mSourceMap.at(sourceID).listMainSoundProperties, listMainSoundProperties);
            mMappedData.mSourceMap.at(sourceID).cacheMainSoundProperties.clear();
        }
        else
        {
            getListMainSourceSoundProperties(sourceID,listMainSoundPropertiesOut);
        }
    }

    if (DB_COND_ISMODIFIED)
    {
        logVerbose("DatabaseHandler::changeSource changed changeSource of source:", sourceID);

        NOTIFY_OBSERVERS4(dboSourceUpdated, sourceID,sourceClassOut,listMainSoundPropertiesOut,sourceVisible(sourceID))

    }

    return (E_OK);

}

am_Error_e CAmDatabaseHandlerMap::changeSinkDB(const am_sinkID_t sinkID, const am_sinkClass_t sinkClassID, const std::vector<am_SoundProperty_s>& listSoundProperties, const std::vector<am_CustomConnectionFormat_t>& listConnectionFormats, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties)
{

    DB_COND_UPDATE_INIT;
    am_sinkClass_t sinkClassOut(sinkClassID);
    std::vector<am_MainSoundProperty_s> listMainSoundPropertiesOut(listMainSoundProperties);

    if (!existSink(sinkID))
    {
    	logError(__METHOD_NAME__,"sinkID must be valid");
        return (E_NON_EXISTENT);
    }

    std::unordered_map<am_sinkID_t, am_Sink_Database_s>::iterator iter = mMappedData.mSinkMap.begin();
    for(; iter!=mMappedData.mSinkMap.end(); ++iter)
    {
        if (iter->second.sinkID == sinkID)
        {
            if (sinkClassID != 0)
            {
                DB_COND_UPDATE(iter->second.sinkClassID, sinkClassID);
            }
            else if (0 == iter->second.reserved)
            {
                sinkClassOut = iter->second.sinkClassID;
            }
            break;
        }
    }

    //check if soundProperties need to be updated
    if (!listSoundProperties.empty())
    {
        mMappedData.mSinkMap.at(sinkID).listSoundProperties = listSoundProperties;
        mMappedData.mSinkMap.at(sinkID).cacheSoundProperties.clear();
    }

    //check if we have to update the list of connectionformats
    if (!listConnectionFormats.empty())
    {
        mMappedData.mSinkMap.at(sinkID).listConnectionFormats = listConnectionFormats;
    }

    //then we need to check if we need to update the listMainSoundProperties
    if (sinkVisible(sinkID))
    {
        if (!listMainSoundProperties.empty())
        {
            DB_COND_UPDATE(mMappedData.mSinkMap.at(sinkID).listMainSoundProperties, listMainSoundProperties);
            mMappedData.mSinkMap.at(sinkID).cacheMainSoundProperties.clear();
        }
        else //read out the properties
        {
            getListMainSinkSoundProperties(sinkID,listMainSoundPropertiesOut);
        }
    }

    if (DB_COND_ISMODIFIED)
    {
        logVerbose("DatabaseHandler::changeSink changed changeSink of sink:", sinkID);

        NOTIFY_OBSERVERS4(dboSinkUpdated, sinkID,sinkClassOut,listMainSoundPropertiesOut,sinkVisible(sinkID))
    }

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListMainSinkNotificationConfigurations(const am_sinkID_t sinkID, std::vector<am_NotificationConfiguration_s>& listMainNotificationConfigurations)
{

    if (!existSink(sinkID))
    {
    	logWarning(__METHOD_NAME__,"sinkID must be valid");
        return (E_DATABASE_ERROR);
    }
    listMainNotificationConfigurations.clear();

    listMainNotificationConfigurations = mMappedData.mSinkMap.at(sinkID).listMainNotificationConfigurations;

     return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::getListMainSourceNotificationConfigurations(const am_sourceID_t sourceID, std::vector<am_NotificationConfiguration_s>& listMainNotificationConfigurations)
{

    if (!existSource(sourceID))
    {
    	logWarning(__METHOD_NAME__,"sourceID must be valid");
        return (E_DATABASE_ERROR);
    }

    listMainNotificationConfigurations = mMappedData.mSourceMap.at(sourceID).listMainNotificationConfigurations;

    return (E_OK);
}

bool changeMainNotificationConfiguration(std::vector<am_NotificationConfiguration_s> & listMainNotificationConfigurations,
											  const am_NotificationConfiguration_s & mainNotificationConfiguration)
{
    std::vector<am_NotificationConfiguration_s>::iterator iter = listMainNotificationConfigurations.begin();
    for(; iter<listMainNotificationConfigurations.end(); ++iter)
    {
        if( mainNotificationConfiguration.type == iter->type )
        {
#ifdef WITH_DATABASE_CHANGE_CHECK
            if( iter->status == mainNotificationConfiguration.status && iter->parameter == mainNotificationConfiguration.parameter )
                return false;
#endif
            *iter = mainNotificationConfiguration;
            return true;
        }
    }
    return false;
}

am_Error_e CAmDatabaseHandlerMap::changeMainSinkNotificationConfigurationDB(const am_sinkID_t sinkID, const am_NotificationConfiguration_s mainNotificationConfiguration)
{

    if (!existSink(sinkID))
    {
    	logError(__METHOD_NAME__,"sinkID must be valid");
        return (E_NON_EXISTENT);
    }
    if(!changeMainNotificationConfiguration(mMappedData.mSinkMap.at(sinkID).listMainNotificationConfigurations, mainNotificationConfiguration))
    	return (E_NO_CHANGE);

    logVerbose("DatabaseHandler::changeMainSinkNotificationConfigurationDB changed MainNotificationConfiguration of source:", sinkID, "type:", mainNotificationConfiguration.type, "to status=", mainNotificationConfiguration.status, "and parameter=",mainNotificationConfiguration.parameter);

    NOTIFY_OBSERVERS2(dboSinkMainNotificationConfigurationChanged, sinkID, mainNotificationConfiguration)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeMainSourceNotificationConfigurationDB(const am_sourceID_t sourceID, const am_NotificationConfiguration_s mainNotificationConfiguration)
{

    if (!existSource(sourceID))
    {
    	logError(__METHOD_NAME__,"sourceID must be valid");
        return (E_NON_EXISTENT);
    }

    if(!changeMainNotificationConfiguration(mMappedData.mSourceMap.at(sourceID).listMainNotificationConfigurations, mainNotificationConfiguration))
    	return (E_NO_CHANGE);

    logVerbose("DatabaseHandler::changeMainSourceNotificationConfigurationDB changed MainNotificationConfiguration of source:", sourceID, "type:", mainNotificationConfiguration.type, "to status=", mainNotificationConfiguration.status, "and parameter=",mainNotificationConfiguration.parameter);

    NOTIFY_OBSERVERS2(dboSourceMainNotificationConfigurationChanged, sourceID, mainNotificationConfiguration)

    return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeGatewayDB(const am_gatewayID_t gatewayID, const std::vector<am_CustomConnectionFormat_t>& listSourceConnectionFormats, const std::vector<am_CustomConnectionFormat_t>& listSinkConnectionFormats, const std::vector<bool>& convertionMatrix)
{

   if (!existGateway(gatewayID))
   {
	   logError(__METHOD_NAME__,"gatewayID must be valid");
       return (E_NON_EXISTENT);
   }

   if (!listSourceConnectionFormats.empty())
   {
	   mMappedData.mGatewayMap.at(gatewayID).listSourceFormats = listSourceConnectionFormats;
   }

   if (!listSinkConnectionFormats.empty())
   {
	   mMappedData.mGatewayMap.at(gatewayID).listSinkFormats = listSinkConnectionFormats;
   }

   if (!convertionMatrix.empty())
   {
       mListConnectionFormat.clear();
       mListConnectionFormat.insert(std::make_pair(gatewayID, convertionMatrix));
   }

   logVerbose("DatabaseHandler::changeGatewayDB changed Gateway with ID", gatewayID);

   //todo: check if observer needs to be adopted.
   return (E_OK);
}

am_Error_e CAmDatabaseHandlerMap::changeConverterDB(const am_converterID_t converterID, const std::vector<am_CustomConnectionFormat_t>& listSourceConnectionFormats, const std::vector<am_CustomConnectionFormat_t>& listSinkConnectionFormats, const std::vector<bool>& convertionMatrix)
{

   if (!existConverter(converterID))
   {
	   logError(__METHOD_NAME__,"converterID must be valid");
       return (E_NON_EXISTENT);
   }

   if (!listSourceConnectionFormats.empty())
   {
	   mMappedData.mConverterMap.at(converterID).listSourceFormats = listSourceConnectionFormats;
   }

   if (!listSinkConnectionFormats.empty())
   {
	   mMappedData.mConverterMap.at(converterID).listSinkFormats = listSinkConnectionFormats;
   }

   if (!convertionMatrix.empty())
   {
       mListConnectionFormat.clear();
       mListConnectionFormat.insert(std::make_pair(converterID, convertionMatrix));
   }

   logVerbose("DatabaseHandler::changeConverterDB changed Gateway with ID", converterID);

   //todo: check if observer needs to be adopted.
   return (E_OK);
}

bool changeNotificationConfiguration(std::vector<am_NotificationConfiguration_s> & listNotificationConfigurations, const am_NotificationConfiguration_s & notificationConfiguration)
{
	bool changed = false;
    std::vector<am_NotificationConfiguration_s>::iterator iter = listNotificationConfigurations.begin();
 	for(; iter<listNotificationConfigurations.end(); ++iter)
 	{
 		if( notificationConfiguration.type == iter->type )
 		{
 			iter->status = notificationConfiguration.status;
 			iter->parameter = notificationConfiguration.parameter;
 			changed |= true;
 		}
 	}
 	return changed;
}

am_Error_e CAmDatabaseHandlerMap::changeSinkNotificationConfigurationDB(const am_sinkID_t sinkID, const am_NotificationConfiguration_s notificationConfiguration)
{

    if (!existSink(sinkID))
    {
 	   logError(__METHOD_NAME__,"sinkID must be valid");
        return (E_NON_EXISTENT);
    }
    if(!changeNotificationConfiguration(mMappedData.mSinkMap.at(sinkID).listNotificationConfigurations, notificationConfiguration))
    	return (E_NO_CHANGE);

    logVerbose("DatabaseHandler::changeMainSinkNotificationConfigurationDB changed MainNotificationConfiguration of source:", sinkID, "type:", notificationConfiguration.type, "to status=", notificationConfiguration.status, "and parameter=",notificationConfiguration.parameter);

    //todo:: inform obsever here...
    return (E_NON_EXISTENT);
}

am_Error_e CAmDatabaseHandlerMap::changeSourceNotificationConfigurationDB(const am_sourceID_t sourceID, const am_NotificationConfiguration_s notificationConfiguration)
{

    if (!existSource(sourceID))
    {
  	   logError(__METHOD_NAME__,"sourceID must be valid");
        return (E_NON_EXISTENT);
    }

    if(!changeNotificationConfiguration(mMappedData.mSourceMap.at(sourceID).listNotificationConfigurations, notificationConfiguration))
    	return (E_NO_CHANGE);

    logVerbose("DatabaseHandler::changeSourceNotificationConfigurationDB changed MainNotificationConfiguration of source:", sourceID, "type:", notificationConfiguration.type, "to status=", notificationConfiguration.status, "and parameter=",notificationConfiguration.parameter);

    //todo:: implement observer function
    return (E_NON_EXISTENT);
}

am_Error_e CAmDatabaseHandlerMap::enumerateSources(std::function<void(const am_Source_s & element)> cb) const
{
	for(auto it = mMappedData.mSourceMap.begin(); it!=mMappedData.mSourceMap.end(); it++)
	{
		const am_Source_Database_s *pObject = &it->second;
		if( 0==pObject->reserved )
			cb(*pObject);
	}
	return E_OK;
}

am_Error_e CAmDatabaseHandlerMap::enumerateSinks(std::function<void(const am_Sink_s & element)> cb) const
{
	for(auto it = mMappedData.mSinkMap.begin(); it!=mMappedData.mSinkMap.end(); it++)
	{
		const am_Sink_Database_s *pObject = &it->second;
		if( 0==pObject->reserved )
			cb(*pObject);
	}
	return E_OK;
}

am_Error_e CAmDatabaseHandlerMap::enumerateGateways(std::function<void(const am_Gateway_s & element)> cb) const
{
	for(auto it = mMappedData.mGatewayMap.begin(); it!=mMappedData.mGatewayMap.end(); it++)
	{
		const am_Gateway_s *pObject = &it->second;
		cb(*pObject);
	}
	return E_OK;
}

am_Error_e CAmDatabaseHandlerMap::enumerateConverters(std::function<void(const am_Converter_s & element)> cb) const
{
	for(auto it = mMappedData.mConverterMap.begin(); it!=mMappedData.mConverterMap.end(); it++)
	{
		const am_Converter_s *pObject =  &it->second;
		cb(*pObject);
	}
	return E_OK;
}

bool CAmDatabaseHandlerMap::registerObserver(IAmDatabaseObserver * iObserver) {
	assert(iObserver!=NULL);
	if (std::find(mDatabaseObservers.begin(), mDatabaseObservers.end(), iObserver) == mDatabaseObservers.end()) 
        {
		mDatabaseObservers.push_back(static_cast<AmDatabaseObserverCallbacks*>(iObserver));
                static_cast<AmDatabaseObserverCallbacks*>(iObserver)->mpDatabaseHandler = this;
		return true;
	}
	return false;
}
bool CAmDatabaseHandlerMap::unregisterObserver(IAmDatabaseObserver * iObserver) {
	assert(iObserver!=NULL);
	auto it = std::find(mDatabaseObservers.begin(), mDatabaseObservers.end(), iObserver);
	if (it != mDatabaseObservers.end()) {
		mDatabaseObservers.erase(it);
                static_cast<AmDatabaseObserverCallbacks*>(iObserver)->mpDatabaseHandler = nullptr;
		return true;
	}
	return false;
}

unsigned CAmDatabaseHandlerMap::countObservers() {
	return mDatabaseObservers.size();
}

}
