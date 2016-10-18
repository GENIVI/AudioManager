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
 * \file CAmRoutingSender.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include "CAmRoutingSender.h"
#include <utility>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include "CAmRoutingReceiver.h"
#include "TAmPluginTemplate.h"
#include "CAmDltWrapper.h"
#include "IAmDatabaseHandler.h"

namespace am
{

#define REQUIRED_INTERFACE_VERSION_MAJOR 1  //!< major interface version. All versions smaller than this will be rejected
#define REQUIRED_INTERFACE_VERSION_MINOR 0 //!< minor interface version. All versions smaller than this will be rejected

#define __METHOD_NAME__ std::string (std::string("CAmRoutingSender::") + __func__)

CAmRoutingSender::CAmRoutingSender(const std::vector<std::string>& listOfPluginDirectories, IAmDatabaseHandler* databaseHandler) :
        mHandleCount(0), //
        mlistActiveHandles(), //
        mListInterfaces(), //
        mMapConnectionInterface(), //
        mMapCrossfaderInterface(), //
        mMapDomainInterface(), //
        mMapSinkInterface(), //
        mMapSourceInterface(), //
        mpRoutingReceiver(), //
        mpDatabaseHandler(databaseHandler)
{

    if (listOfPluginDirectories.empty())
    {
        logError(__METHOD_NAME__,"List of routingplugins is empty");
    }

    std::vector<std::string> sharedLibraryNameList;
    std::vector<std::string>::const_iterator dirIter = listOfPluginDirectories.begin();
    std::vector<std::string>::const_iterator dirIterEnd = listOfPluginDirectories.end();

    // search communicator plugins in configured directories
    for (; dirIter < dirIterEnd; ++dirIter)
    {
        const char* directoryName = dirIter->c_str();
        logInfo(__METHOD_NAME__,"Searching for HookPlugins in", directoryName);
        DIR *directory = opendir(directoryName);

        if (!directory)
        {
            logError(__METHOD_NAME__,"Error opening directory: ", directoryName);
            continue;
        }

        // iterate content of directory
        struct dirent *itemInDirectory = 0;
        while ((itemInDirectory = readdir(directory)))
        {
            unsigned char entryType = itemInDirectory->d_type;
            std::string entryName = itemInDirectory->d_name;
            std::string fullName = *dirIter + "/" + entryName;

            bool regularFile = (entryType == DT_REG || entryType == DT_LNK);
            bool sharedLibExtension = ("so" == entryName.substr(entryName.find_last_of(".") + 1));

            // Handle cases where readdir() could not determine the file type
	        if (entryType == DT_UNKNOWN) {
	            struct stat buf;

	            if (stat(fullName.c_str(), &buf)) {
	                logInfo(__METHOD_NAME__,"Failed to stat file: ", entryName, errno);
	                continue;
	            }

	            regularFile = S_ISREG(buf.st_mode);
	        }

            if (regularFile && sharedLibExtension)
            {
                logInfo(__METHOD_NAME__,"adding file: ", entryName);
                std::string name(directoryName);
                sharedLibraryNameList.push_back(name + "/" + entryName);
            }
            else
            {
                logInfo(__METHOD_NAME__, "plugin search ignoring file :", entryName);
            }
        }

        closedir(directory);
    }

    // iterate all communicator plugins and start them
    std::vector<std::string>::iterator iter = sharedLibraryNameList.begin();
    std::vector<std::string>::iterator iterEnd = sharedLibraryNameList.end();

    for (; iter != iterEnd; ++iter)
    {
        logInfo(__METHOD_NAME__,"try loading: ", *iter);

        IAmRoutingSend* (*createFunc)();
        void* tempLibHandle = NULL;
        createFunc = getCreateFunction<IAmRoutingSend*()>(*iter, tempLibHandle);

        if (!createFunc)
        {
            logError(__METHOD_NAME__,"Entry point of RoutingPlugin not found");
            continue;
        }

        IAmRoutingSend* router = createFunc();

        if (!router)
        {
            logError(__METHOD_NAME__,"initialization of plugin ",*iter,"failed. Entry Function not callable");
            dlclose(tempLibHandle);
            continue;
        }

        InterfaceNamePairs routerInterface;
        routerInterface.routingInterface = router;

        //check libversion
        std::string version, cVersion(RoutingVersion);
        router->getInterfaceVersion(version);
        uint16_t minorVersion, majorVersion, cMinorVersion, cMajorVersion;
        std::istringstream(version.substr(0, 1)) >> majorVersion;
        std::istringstream(version.substr(2, 1)) >> minorVersion;
        std::istringstream(cVersion.substr(0, 1)) >> cMajorVersion;
        std::istringstream(cVersion.substr(2, 1)) >> cMinorVersion;
        
        

        if (majorVersion < cMajorVersion || ((majorVersion == cMajorVersion) && (minorVersion > cMinorVersion)))
        {
            logError(__METHOD_NAME__,"Routing initialization failed. Version of Interface to old");
            dlclose(tempLibHandle);
            continue;
        }

        //here, the busname is saved together with the interface. Later The domains will register with the name and sinks, sources etc with the domain....
        router->returnBusName(routerInterface.busName);
        assert(!routerInterface.busName.empty());
        mListInterfaces.push_back(routerInterface);
        mListLibraryHandles.push_back(tempLibHandle);
    }
}

CAmRoutingSender::~CAmRoutingSender()
{
    //unloadLibraries();
    HandlesMap::iterator it = mlistActiveHandles.begin();

    //every open handle is assumed to be an error...
    for (; it != mlistActiveHandles.end(); ++it)
    {
        logError(__METHOD_NAME__,"The action for the handle",it->first,"is still open");
    }
}

am_Error_e CAmRoutingSender::startupInterfaces(CAmRoutingReceiver *iRoutingReceiver)
{
    mpRoutingReceiver = iRoutingReceiver;
    am_Error_e returnError = E_OK;

    std::vector<InterfaceNamePairs>::iterator iter = mListInterfaces.begin();
    std::vector<InterfaceNamePairs>::iterator iterEnd = mListInterfaces.end();
    for (; iter < iterEnd; ++iter)
    {
        am_Error_e error = (*iter).routingInterface->startupInterface(iRoutingReceiver);
        if (error != E_OK)
        {
            returnError = error;
        }
    }
    return (returnError);
}

am_Error_e CAmRoutingSender::asyncAbort(const am_Handle_s& handle)
{
    auto iter (mlistActiveHandles.find(handle));
    if (iter == mlistActiveHandles.end())
    {
		logError(__METHOD_NAME__,"Could not find handle",handle);
		return (E_NON_EXISTENT);
    }
    logInfo(__METHOD_NAME__," handle", handle);
	return (iter->second->returnInterface()->asyncAbort(handle));
}

am_Error_e CAmRoutingSender::asyncConnect(am_Handle_s& handle, am_connectionID_t& connectionID, const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_CustomConnectionFormat_t connectionFormat)
{
	auto iter (mMapSinkInterface.find(sinkID));
	if (iter == mMapSinkInterface.end())
	{
		logError(__METHOD_NAME__,"Could not find sink",sinkID);
		return (E_NON_EXISTENT);
	}
	
	if(handleExists(handle))
	{
		if (handle.handleType==am_Handle_e::H_CONNECT)
		{
			logInfo(__METHOD_NAME__,"Resending for handle",handle);
		}
		else
		{
			logError(__METHOD_NAME__,"Handle exists but wrong type",handle);
			return(E_UNKNOWN);	
		}
	}
	else
	{
		
		am_Connection_s tempConnection;
		tempConnection.sinkID = sinkID;
		tempConnection.sourceID = sourceID;
		tempConnection.connectionFormat = connectionFormat;
		tempConnection.connectionID = 0;
		tempConnection.delay=-1;

		am_Error_e connError(mpDatabaseHandler->enterConnectionDB(tempConnection, connectionID));
		if (connError)
		{
			return(connError);
		}
		mMapConnectionInterface.insert(std::make_pair(connectionID, iter->second));
		auto handleData = std::make_shared<handleConnect>(iter->second,connectionID,mpDatabaseHandler);
		handle = createHandle(handleData, am_Handle_e::H_CONNECT);
	}

    logInfo(__METHOD_NAME__,"connectionID=",connectionID,"connectionFormat=", connectionFormat, "sourceID=", sourceID, "sinkID=", sinkID,"handle=",handle);
	am_Error_e syncError(iter->second->asyncConnect(handle, connectionID, sourceID, sinkID, connectionFormat));
	if (syncError)
	{
		removeHandle(handle);
		logError(__METHOD_NAME__,"Error while calling connect connectionID:",connectionID,"sourceID:",sourceID,"sinkID:",sinkID,"connectionFormat:",connectionFormat,"handle",handle);
		mpDatabaseHandler->removeConnection(connectionID);
	}
	return(syncError); 
}

am_Error_e CAmRoutingSender::asyncDisconnect(am_Handle_s& handle, const am_connectionID_t connectionID)
{
	auto iter(mMapConnectionInterface.find(connectionID));
	if (iter == mMapConnectionInterface.end())
	{
		logError(__METHOD_NAME__,"Could not find connection",connectionID);
		return (E_NON_EXISTENT);	
	}
	
	if(handleExists(handle))
	{
		if (handle.handleType==am_Handle_e::H_DISCONNECT)
		{
			logInfo(__METHOD_NAME__,"Resending for handle",handle);
		}
		else
		{
			logError(__METHOD_NAME__,"Handle exists but wrong type",handle);
			return(E_UNKNOWN);	
		}
    }
    else
    {
		auto handleData = std::make_shared<handleDisconnect>(iter->second,connectionID,mpDatabaseHandler,this);
		handle = createHandle(handleData, am_Handle_e::H_DISCONNECT);
	}

    logInfo(__METHOD_NAME__,"connectionID=", connectionID, "handle=",handle);
	am_Error_e syncError(iter->second->asyncDisconnect(handle, connectionID));
	if (syncError)
	{
		removeHandle(handle);
		logError(__METHOD_NAME__,"Error while calling disconnect connectionID:",connectionID,"handle",handle);
	}
	return(syncError);
}

am_Error_e CAmRoutingSender::asyncSetSinkVolume(am_Handle_s& handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_CustomRampType_t ramp, const am_time_t time)
{
	auto iter (mMapSinkInterface.find(sinkID));
	if (iter == mMapSinkInterface.end())
	{
		logError(__METHOD_NAME__,"Could not find sink",sinkID);
		return (E_NON_EXISTENT);
	}
	
	if(handleExists(handle))
	{
		if (handle.handleType==am_Handle_e::H_SETSINKVOLUME)
		{
			logInfo(__METHOD_NAME__,"Resending for handle",handle);
		}
		else
		{
			logError(__METHOD_NAME__,"Handle exists but wrong type",handle);
			return(E_UNKNOWN);	
		}
    }
	else
	{
        auto handleData = std::make_shared<handleSinkVolume>(iter->second,sinkID,mpDatabaseHandler,volume);
        handle = createHandle(handleData, H_SETSINKVOLUME);
    }
    
    logInfo(__METHOD_NAME__,"sinkID=", sinkID, "volume=", volume, "ramp=", ramp, "time=", time,"handle=",handle);
	am_Error_e syncError(iter->second->asyncSetSinkVolume(handle, sinkID, volume, ramp, time));
	if (syncError)
	{
		removeHandle(handle);
		logError(__METHOD_NAME__,"Error while calling asyncSetSinkVolume sinkID:",sinkID,"handle:",handle,"volume:",volume,"ramp:",ramp,"time:",time);
	}
	return(syncError);
}

am_Error_e CAmRoutingSender::asyncSetSourceVolume(am_Handle_s& handle, const am_sourceID_t sourceID, const am_volume_t volume, const am_CustomRampType_t ramp, const am_time_t time)
{
	auto iter (mMapSourceInterface.find(sourceID));
	if (iter == mMapSourceInterface.end())
	{
		logError(__METHOD_NAME__,"Could not find sourceID",sourceID);
		return (E_NON_EXISTENT);
	}
	
	if(handleExists(handle))
	{
		if (handle.handleType==am_Handle_e::H_SETSOURCEVOLUME)
		{
			logInfo(__METHOD_NAME__,"Resending for handle",handle);
		}
		else
		{
			logError(__METHOD_NAME__,"Handle exists but wrong type",handle);
			return(E_UNKNOWN);	
		}
    }
	else
	{
		auto handleData = std::make_shared<handleSourceVolume>(iter->second,sourceID,mpDatabaseHandler,volume);
        handle = createHandle(handleData, H_SETSOURCEVOLUME);
    }
     
    logInfo(__METHOD_NAME__,"sourceID=", sourceID,"volume=", volume, "ramp=", ramp, "time=", time,"handle=",handle);
	am_Error_e syncError(iter->second->asyncSetSourceVolume(handle, sourceID, volume, ramp, time));
	if (syncError)
	{
		removeHandle(handle);
		logError(__METHOD_NAME__,"Error while calling asyncSetSourceVolume sourceID:",sourceID,"handle:",handle,"volume:",volume,"ramp:",ramp,"time:",time);
	}
	return(syncError);
}

am_Error_e CAmRoutingSender::asyncSetSourceState(am_Handle_s& handle, const am_sourceID_t sourceID, const am_SourceState_e state)
{
	auto iter (mMapSourceInterface.find(sourceID));
	if (iter == mMapSourceInterface.end())
	{
		logError(__METHOD_NAME__,"Could not find sourceID",sourceID);
		return (E_NON_EXISTENT);
	}
	
	if(handleExists(handle))
	{
		if (handle.handleType==am_Handle_e::H_SETSOURCESTATE)
		{
			logInfo(__METHOD_NAME__,"Resending for handle",handle);
		}
		else
		{
			logError(__METHOD_NAME__,"Handle exists but wrong type",handle);
			return(E_UNKNOWN);	
		}
    }
	else
	{	
		auto handleData = std::make_shared<handleSourceState>(iter->second,sourceID,state,mpDatabaseHandler);
		handle = createHandle(handleData, H_SETSOURCESTATE);
	}
    logInfo(__METHOD_NAME__,"sourceID=", sourceID, "state=", state,"handle=",handle);
	am_Error_e syncError(iter->second->asyncSetSourceState(handle, sourceID, state));
	if (syncError)
	{
		removeHandle(handle);
		logError(__METHOD_NAME__,"Error while calling asyncSetSourceState sourceID:",sourceID,"handle:",handle,"state:",state);
	}
	return(syncError);
}

am_Error_e CAmRoutingSender::asyncSetSinkSoundProperty(am_Handle_s& handle, const am_sinkID_t sinkID, const am_SoundProperty_s & soundProperty)
{
	auto iter (mMapSinkInterface.find(sinkID));
	if (iter == mMapSinkInterface.end())
	{
		logError(__METHOD_NAME__,"Could not find sink",sinkID);
		return (E_NON_EXISTENT);
	}
	
	if(handleExists(handle))
	{
		if (handle.handleType==am_Handle_e::H_SETSINKSOUNDPROPERTY)
		{
			logInfo(__METHOD_NAME__,"Resending for handle",handle);
		}
		else
		{
			logError(__METHOD_NAME__,"Handle exists but wrong type",handle);
			return(E_UNKNOWN);	
		}
    }
	else
	{
		auto handleData = std::make_shared<handleSinkSoundProperty>(iter->second,sinkID,soundProperty,mpDatabaseHandler);
        handle = createHandle(handleData, H_SETSINKSOUNDPROPERTY);
     }
     
    logInfo(__METHOD_NAME__,"sinkID=", sinkID, "soundProperty.Type=", soundProperty.type, "soundProperty.value=", soundProperty.value,"handle=",handle);
	am_Error_e syncError(iter->second->asyncSetSinkSoundProperty(handle, sinkID, soundProperty));
	if (syncError)
	{
		removeHandle(handle);
		logError(__METHOD_NAME__,"Error while calling asyncSetSinkSoundProperty sinkID:",sinkID,"handle:",handle,"soundProperty:",soundProperty.type,soundProperty.value);
	}
	return(syncError);
}

am_Error_e CAmRoutingSender::asyncSetSourceSoundProperty(am_Handle_s& handle, const am_sourceID_t sourceID, const am_SoundProperty_s & soundProperty)
{
	auto iter (mMapSourceInterface.find(sourceID));
	if (iter == mMapSourceInterface.end())
	{
		logError(__METHOD_NAME__,"Could not find sourceID",sourceID);
		return (E_NON_EXISTENT);
	}
	
	if(handleExists(handle))
	{
		if (handle.handleType==am_Handle_e::H_SETSOURCESOUNDPROPERTY)
		{
			logInfo(__METHOD_NAME__,"Resending for handle",handle);
		}
		else
		{
			logError(__METHOD_NAME__,"Handle exists but wrong type",handle);
			return(E_UNKNOWN);	
		}
    }
	else
	{		
		auto handleData = std::make_shared<handleSourceSoundProperty>(iter->second,sourceID,soundProperty,mpDatabaseHandler);
        handle = createHandle(handleData, H_SETSOURCESOUNDPROPERTY);
    }
    logInfo(__METHOD_NAME__,"sourceID=", sourceID, "soundProperty.Type=", soundProperty.type, "soundProperty.value=", soundProperty.value,"handle=",handle);
	am_Error_e syncError(iter->second->asyncSetSourceSoundProperty(handle, sourceID, soundProperty));
	if (syncError)
	{
		removeHandle(handle);
		logError(__METHOD_NAME__,"Error while calling asyncSetSourceSoundProperty sourceID:",sourceID,"handle:",handle,"soundProperty:",soundProperty.type,soundProperty.value);
	}
	return(syncError);
}

am_Error_e CAmRoutingSender::asyncSetSourceSoundProperties(am_Handle_s& handle, const std::vector<am_SoundProperty_s> & listSoundProperties, const am_sourceID_t sourceID)
{
	auto iter (mMapSourceInterface.find(sourceID));
	if (iter == mMapSourceInterface.end())
	{
		logError(__METHOD_NAME__,"Could not find sourceID",sourceID);
		return (E_NON_EXISTENT);
	}
	
	if(handleExists(handle))
	{
		if (handle.handleType==am_Handle_e::H_SETSOURCESOUNDPROPERTIES)
		{
			logInfo(__METHOD_NAME__,"Resending for handle",handle);
		}
		else
		{
			logError(__METHOD_NAME__,"Handle exists but wrong type",handle);
			return(E_UNKNOWN);	
		}
    }
	else
	{		
		auto handleData = std::make_shared<handleSourceSoundProperties>(iter->second,sourceID,listSoundProperties,mpDatabaseHandler);
        handle = createHandle(handleData, H_SETSOURCESOUNDPROPERTIES);
    }
     
    logInfo(__METHOD_NAME__,"sourceID=", sourceID);
	am_Error_e syncError(iter->second->asyncSetSourceSoundProperties(handle, sourceID, listSoundProperties));
	if (syncError)
	{
		removeHandle(handle);
		logError(__METHOD_NAME__,"Error while calling asyncSetSourceSoundProperties sourceID:",sourceID,"handle:",handle);
	}
	return(syncError);
}

am_Error_e CAmRoutingSender::asyncSetSinkSoundProperties(am_Handle_s& handle, const std::vector<am_SoundProperty_s> & listSoundProperties, const am_sinkID_t sinkID)
{
	auto iter (mMapSinkInterface.find(sinkID));
	if (iter == mMapSinkInterface.end())
	{
		logError(__METHOD_NAME__,"Could not find sink",sinkID);
		return (E_NON_EXISTENT);
	}
	
	if(handleExists(handle))
	{
		if (handle.handleType==am_Handle_e::H_SETSINKSOUNDPROPERTIES)
		{
			logInfo(__METHOD_NAME__,"Resending for handle",handle);
		}
		else
		{
			logError(__METHOD_NAME__,"Handle exists but wrong type",handle);
			return(E_UNKNOWN);	
		}
    }
	else
	{	
		auto handleData = std::make_shared<handleSinkSoundProperties>(iter->second,sinkID,listSoundProperties,mpDatabaseHandler);
        handle = createHandle(handleData, H_SETSINKSOUNDPROPERTIES);
    }
    
    logInfo(__METHOD_NAME__,"sinkID=", sinkID,"handle=",handle);
	am_Error_e syncError(iter->second->asyncSetSinkSoundProperties(handle, sinkID, listSoundProperties));
	if (syncError)
	{
		removeHandle(handle);
		logError(__METHOD_NAME__,"Error while calling asyncSetSinkSoundProperties sinkID:",sinkID,"handle:",handle);
	}
	return(syncError);
}

am_Error_e CAmRoutingSender::asyncCrossFade(am_Handle_s& handle, const am_crossfaderID_t crossfaderID, const am_HotSink_e hotSink, const am_CustomRampType_t rampType, const am_time_t time)
{
	auto iter (mMapCrossfaderInterface.find(crossfaderID));
	if (iter == mMapCrossfaderInterface.end())
	{
		logError(__METHOD_NAME__,"Could not find crossfaderID",crossfaderID);
		return (E_NON_EXISTENT);
	}
	
	if(handleExists(handle))
	{
		if (handle.handleType==am_Handle_e::H_CROSSFADE)
		{
			logInfo(__METHOD_NAME__,"Resending for handle",handle);
		}
		else
		{
			logError(__METHOD_NAME__,"Handle exists but wrong type",handle);
			return(E_UNKNOWN);	
		}
    }
	else
	{	
		auto handleData = std::make_shared<handleCrossFader>(iter->second,crossfaderID,hotSink,mpDatabaseHandler);
        handle = createHandle(handleData, H_CROSSFADE);
	}
	
	logInfo(__METHOD_NAME__,"hotSource=", hotSink, "crossfaderID=", crossfaderID, "rampType=", rampType, "rampTime=", time,"handle=",handle);
	am_Error_e syncError(iter->second->asyncCrossFade(handle, crossfaderID, hotSink, rampType, time));
	if (syncError)
	{
		removeHandle(handle);
	}
	return(syncError);
}

am_Error_e CAmRoutingSender::setDomainState(const am_domainID_t domainID, const am_DomainState_e domainState)
{
	logInfo(__METHOD_NAME__,"domainID=", domainID, "domainState=", domainState);
    DomainInterfaceMap::iterator iter = mMapDomainInterface.begin();
    iter = mMapDomainInterface.find(domainID);
    if (iter != mMapDomainInterface.end())
        return (iter->second->setDomainState(domainID, domainState));
    return (E_NON_EXISTENT);
}

/**
 * @author Christian
 * this adds the domain to the lookup table of the Router. The data is used to have a quick lookup of the correct pluginInterface.
 * This must be done whenever a domain is registered.
 */
am_Error_e CAmRoutingSender::addDomainLookup(const am_Domain_s& domainData)
{
    std::vector<InterfaceNamePairs>::iterator iter = mListInterfaces.begin();
    std::vector<InterfaceNamePairs>::iterator iterEnd = mListInterfaces.end();
    for (; iter < iterEnd; ++iter)
    {
        if ((*iter).busName.compare(domainData.busname) == 0)
        {
            mMapDomainInterface.insert(std::make_pair(domainData.domainID, (*iter).routingInterface));
            return (E_OK);
        }
    }
    logError(__PRETTY_FUNCTION__," Could not find busname for bus",domainData.busname);
    return (E_UNKNOWN);
}

/**
 * @author Christian
 * this adds the Source to the lookup table of the Router. The data is used to have a quick lookup of the correct pluginInterface.
 * This must be done whenever a Source is registered.
 */
am_Error_e CAmRoutingSender::addSourceLookup(const am_Source_s& sourceData)
{
    DomainInterfaceMap::iterator iter = mMapDomainInterface.begin();
    iter = mMapDomainInterface.find(sourceData.domainID);
    if (iter != mMapDomainInterface.end())
    {
        mMapSourceInterface.insert(std::make_pair(sourceData.sourceID, iter->second));
        return (E_OK);
    }
    logError(__PRETTY_FUNCTION__," Could not find domainInterface for domainID",sourceData.domainID);
    return (E_UNKNOWN);
}

/**
 * @author Christian
 * this adds the Sink to the lookup table of the Router. The data is used to have a quick lookup of the correct pluginInterface.
 * This must be done whenever a Sink is registered.
 */
am_Error_e CAmRoutingSender::addSinkLookup(const am_Sink_s& sinkData)
{
    DomainInterfaceMap::iterator iter = mMapDomainInterface.begin();
    iter = mMapDomainInterface.find(sinkData.domainID);
    if (iter != mMapDomainInterface.end())
    {
        mMapSinkInterface.insert(std::make_pair(sinkData.sinkID, iter->second));
        return (E_OK);
    }
    logError(__PRETTY_FUNCTION__,"Could not find domainInterface for domainID",sinkData.domainID);
    return (E_UNKNOWN);
}

/**
 * @author Christian
 * this adds the Crossfader to the lookup table of the Router. The data is used to have a quick lookup of the correct pluginInterface.
 * This must be done whenever a Crossfader is registered.
 */
am_Error_e CAmRoutingSender::addCrossfaderLookup(const am_Crossfader_s& crossfaderData)
{
    DomainInterfaceMap::iterator iter = mMapSourceInterface.begin();
    iter = mMapSourceInterface.find(crossfaderData.sourceID);
    if (iter != mMapSourceInterface.end())
    {
        mMapSourceInterface.insert(std::make_pair(crossfaderData.crossfaderID, iter->second));
        return (E_OK);
    }
    logError(__PRETTY_FUNCTION__," Could not find sourceInterface for source",crossfaderData.sourceID);
    return (E_UNKNOWN);
}

/**
 * @author Christian
 * this removes the Domain to the lookup table of the Router. This must be done everytime a domain is deregistered.
 */
am_Error_e CAmRoutingSender::removeDomainLookup(const am_domainID_t domainID)
{
    DomainInterfaceMap::iterator iter = mMapDomainInterface.begin();
    iter = mMapDomainInterface.find(domainID);
    if (iter != mMapDomainInterface.end())
    {
        mMapDomainInterface.erase(iter);
        return (E_OK);
    }

    return (E_NON_EXISTENT);
}

/**
 * @author Christian
 * this removes the Source to the lookup table of the Router. This must be done everytime a source is deregistered.
 */
am_Error_e CAmRoutingSender::removeSourceLookup(const am_sourceID_t sourceID)
{
    SourceInterfaceMap::iterator iter = mMapSourceInterface.begin();
    iter = mMapSourceInterface.find(sourceID);
    if (iter != mMapSourceInterface.end())
    {
        mMapSourceInterface.erase(iter);
        return (E_OK);
    }

    return (E_NON_EXISTENT);
}

/**
 * @author Christian
 * this removes the Sink to the lookup table of the Router. This must be done everytime a sink is deregistered.
 */
am_Error_e CAmRoutingSender::removeSinkLookup(const am_sinkID_t sinkID)
{
    SinkInterfaceMap::iterator iter = mMapSinkInterface.begin();
    iter = mMapSinkInterface.find(sinkID);
    if (iter != mMapSinkInterface.end())
    {
        mMapSinkInterface.erase(iter);
        return (E_OK);
    }

    return (E_NON_EXISTENT);
}

/**
 * @author Christian
 * this removes the Crossfader to the lookup table of the Router. This must be done everytime a crossfader is deregistered.
 */
am_Error_e CAmRoutingSender::removeCrossfaderLookup(const am_crossfaderID_t crossfaderID)
{
    CrossfaderInterfaceMap::iterator iter = mMapCrossfaderInterface.begin();
    iter = mMapCrossfaderInterface.find(crossfaderID);
    if (iter != mMapCrossfaderInterface.end())
    {
        mMapCrossfaderInterface.erase(iter);
        return (E_OK);
    }

    return (E_NON_EXISTENT);
}

/**
 * removes a handle from the list
 * @param handle to be removed
 * @return E_OK in case of success
 */
am_Error_e CAmRoutingSender::removeHandle(const am_Handle_s& handle)
{
    if (mlistActiveHandles.erase(handle))
    {
        return (E_OK);
    }
    logError(__METHOD_NAME__,"Could not remove handle",handle.handle);
    return (E_NON_EXISTENT);
}

am_Error_e CAmRoutingSender::getListHandles(std::vector<am_Handle_s> & listHandles) const
{
    listHandles.clear();
    HandlesMap::const_iterator it = mlistActiveHandles.begin();
    for (; it != mlistActiveHandles.end(); ++it)
    {
        listHandles.push_back(it->first);
    }
    return (E_OK);
}

/**
 * creates a handle and adds it to the list of handles
 * @param handleData the data that should be saves together with the handle
 * @param type the type of handle to be created
 * @return the handle
 */
am_Handle_s CAmRoutingSender::createHandle(std::shared_ptr<handleDataBase> handleData, const am_Handle_e type)
{
    am_Handle_s handle;
    if (++mHandleCount>=1024) //defined by 10 bit (out if structure!)
        mHandleCount=1;
    handle.handle = mHandleCount;
    handle.handleType = type;
    mlistActiveHandles.insert(std::make_pair(handle, handleData));
    if ((mlistActiveHandles.size()%100) == 0)
    {
        logInfo("CAmRoutingSender::createHandle warning: too many open handles, number of handles: ", mlistActiveHandles.size());
    }
    logInfo(__METHOD_NAME__,handle.handle, handle.handleType);
    return (handle);
}

void CAmRoutingSender::setRoutingReady()
{
    mpRoutingReceiver->waitOnStartup(false);

    //create a list of handles
    std::vector<uint16_t> listStartupHandles;
    for (size_t i = 0; i < mListInterfaces.size(); i++)
    {
        listStartupHandles.push_back(mpRoutingReceiver->getStartupHandle());
    }

    //set the receiver ready to wait for replies
    mpRoutingReceiver->waitOnStartup(true);

    std::vector<InterfaceNamePairs>::iterator iter = mListInterfaces.begin();
    std::vector<InterfaceNamePairs>::iterator iterEnd = mListInterfaces.end();
    std::vector<uint16_t>::const_iterator handleIter(listStartupHandles.begin());
    for (; iter < iterEnd; ++iter)
    {
        (*iter).routingInterface->setRoutingReady(*(handleIter++));
    }
}

void CAmRoutingSender::setRoutingRundown()
{
    mpRoutingReceiver->waitOnRundown(false);
    //create a list of handles
    std::vector<uint16_t> listStartupHandles;
    for (size_t i = 0; i < mListInterfaces.size(); i++)
    {
        listStartupHandles.push_back(mpRoutingReceiver->getRundownHandle());
    }

    //set the receiver ready to wait for replies
    mpRoutingReceiver->waitOnRundown(true);

    std::vector<InterfaceNamePairs>::iterator iter = mListInterfaces.begin();
    std::vector<InterfaceNamePairs>::iterator iterEnd = mListInterfaces.end();
    std::vector<uint16_t>::const_iterator handleIter(listStartupHandles.begin());
    for (; iter < iterEnd; ++iter)
    {
        (*iter).routingInterface->setRoutingRundown(*(handleIter++));
    }
}

am_Error_e CAmRoutingSender::asyncSetVolumes(am_Handle_s& handle, const std::vector<am_Volumes_s>& listVolumes)
{
    IAmRoutingSend* pRoutingInterface(NULL);
    if (listVolumes.empty())
        return (E_NOT_POSSIBLE);

    //we need an interface so lets get either the sink or source ID from the first entry in the listVolumes
    if (listVolumes[0].volumeType==VT_SINK)
    {
        am_sinkID_t sinkID=listVolumes[0].volumeID.sink;
        SinkInterfaceMap::iterator iter = mMapSinkInterface.begin();
        iter = mMapSinkInterface.find(sinkID);
        if(iter!=mMapSinkInterface.end())
            pRoutingInterface=iter->second;
        else
            return(E_NON_EXISTENT);
    }

    else if (listVolumes[0].volumeType==VT_SOURCE)
    {
        am_sourceID_t sourceID=listVolumes[0].volumeID.source;
        SourceInterfaceMap::iterator iter = mMapSourceInterface.begin();
        iter = mMapSourceInterface.find(sourceID);
        if (iter!=mMapSourceInterface.end())
            pRoutingInterface=iter->second;
        else
            return(E_NON_EXISTENT);
    }
    else
        return (E_NON_EXISTENT);

	auto handleData = std::make_shared<handleSetVolumes>(pRoutingInterface,listVolumes,mpDatabaseHandler);
    handle = createHandle(handleData, H_SETVOLUMES);

    logInfo(__METHOD_NAME__, "handle=", handle);
    am_Error_e syncError(pRoutingInterface->asyncSetVolumes(handle, listVolumes));
    if (syncError)
	{
		removeHandle(handle);
	}
	return(syncError);

}

am_Error_e CAmRoutingSender::asyncSetSinkNotificationConfiguration(am_Handle_s& handle, const am_sinkID_t sinkID, const am_NotificationConfiguration_s& notificationConfiguration)
{
	auto iter (mMapSinkInterface.find(sinkID));
	if (iter == mMapSinkInterface.end())
	{
		logError(__METHOD_NAME__,"Could not find sink",sinkID);
		return (E_NON_EXISTENT);
	}
	
	if(handleExists(handle))
	{
		if (handle.handleType==am_Handle_e::H_SETSINKNOTIFICATION)
		{
			logInfo(__METHOD_NAME__,"Resending for handle",handle);
		}
		else
		{
			logError(__METHOD_NAME__,"Handle exists but wrong type",handle);
			return(E_UNKNOWN);	
		}
    }
	else
	{	
		auto handleData = std::make_shared<handleSetSinkNotificationConfiguration>(iter->second,sinkID,notificationConfiguration,mpDatabaseHandler);
        handle = createHandle(handleData, H_SETSINKNOTIFICATION);
    }

    logInfo(__METHOD_NAME__,"sinkID=",sinkID,"notificationConfiguration.type=",notificationConfiguration.type,"notificationConfiguration.status",notificationConfiguration.status,"notificationConfiguration.parameter",notificationConfiguration.parameter);
	am_Error_e syncError(iter->second->asyncSetSinkNotificationConfiguration(handle, sinkID, notificationConfiguration));
	if (syncError)
	{
		removeHandle(handle);
		logError(__METHOD_NAME__,"Error while calling asyncSetSinkNotificationConfiguration sinkID:",sinkID,"handle:",handle);
	}
	return(syncError);
}

am_Error_e CAmRoutingSender::asyncSetSourceNotificationConfiguration(am_Handle_s& handle, const am_sourceID_t sourceID, const am_NotificationConfiguration_s& notificationConfiguration)
{
	auto iter (mMapSourceInterface.find(sourceID));
	if (iter == mMapSourceInterface.end())
	{
		logError(__METHOD_NAME__,"Could not find sourceID",sourceID);
		return (E_NON_EXISTENT);
	}
	
	if(handleExists(handle))
	{
		if (handle.handleType==am_Handle_e::H_SETSOURCENOTIFICATION)
		{
			logInfo(__METHOD_NAME__,"Resending for handle",handle);
		}
		else
		{
			logError(__METHOD_NAME__,"Handle exists but wrong type",handle);
			return(E_UNKNOWN);	
		}
    }
	else
	{	
		auto handleData = std::make_shared<handleSetSourceNotificationConfiguration>(iter->second,sourceID,notificationConfiguration,mpDatabaseHandler);
        handle = createHandle(handleData, H_SETSOURCENOTIFICATION);
    }

    logInfo(__METHOD_NAME__,"sourceID=",sourceID,"notificationConfiguration.type=",notificationConfiguration.type,"notificationConfiguration.status",notificationConfiguration.status,"notificationConfiguration.parameter",notificationConfiguration.parameter);
	am_Error_e syncError(iter->second->asyncSetSourceNotificationConfiguration(handle, sourceID, notificationConfiguration));
	if (syncError)
	{
		removeHandle(handle);
		logError(__METHOD_NAME__,"Error while calling asyncSetSourceNotificationConfiguration sourceID:",sourceID,"handle:",handle);
	}
	return(syncError);
}

void CAmRoutingSender::unloadLibraries(void)
{
    std::vector<void*>::iterator iterator = mListLibraryHandles.begin();
    for (; iterator < mListLibraryHandles.end(); ++iterator)
    {
        dlclose(*iterator);
    }
    mListLibraryHandles.clear();
}

am_Error_e CAmRoutingSender::getListPlugins(std::vector<std::string>& interfaces) const
{
    std::vector<InterfaceNamePairs>::const_iterator it = mListInterfaces.begin();
    for (; it != mListInterfaces.end(); ++it)
    {
        interfaces.push_back(it->busName);
    }
    return (E_OK);
}

void CAmRoutingSender::getInterfaceVersion(std::string & version) const
{
    version = RoutingVersion;
}
am_Error_e CAmRoutingSender::resyncConnectionState(const am_domainID_t domainID,std::vector<am_Connection_s>& listOfExistingConnections)
{
    DomainInterfaceMap::iterator iter = mMapDomainInterface.begin();
    iter = mMapDomainInterface.find(domainID);
    if (iter != mMapDomainInterface.end())
        return (iter->second->resyncConnectionState(domainID, listOfExistingConnections));
    return (E_NON_EXISTENT);
}

am_Error_e CAmRoutingSender::writeToDatabaseAndRemove(const am_Handle_s handle)
{
    auto it(mlistActiveHandles.find(handle));
    if (it!=mlistActiveHandles.end())
    {
    	am_Error_e error(it->second->writeDataToDatabase());
    	mlistActiveHandles.erase(handle);
        return (error);
    }
    logError(__METHOD_NAME__,"could not find handle data for handle",handle);
    return (am_Error_e::E_NON_EXISTENT);	
}

void CAmRoutingSender::checkVolume(const am_Handle_s handle, const am_volume_t volume)
{
    auto it(mlistActiveHandles.find(handle));
    if (it!=mlistActiveHandles.end())
    {
		handleVolumeBase* basePtr = static_cast<handleVolumeBase*>(it->second.get());
    	if (basePtr->returnVolume()!=volume)
    	{	
			logError(__METHOD_NAME__,"volume returned for handle does not match: ",volume,"expected:",basePtr->returnVolume());
		}	
    	return;
    }
    logError(__METHOD_NAME__,"could not find handle data for handle",handle);
}

bool CAmRoutingSender::handleExists(const am_Handle_s handle)
{
    auto iter(mlistActiveHandles.find(handle));
    if (iter!=mlistActiveHandles.end())
    {
		return (true);
	}
	return (false);
}

am_Error_e CAmRoutingSender::handleSinkSoundProperty::writeDataToDatabase()
{
	return (mpDatabaseHandler->changeSinkSoundPropertyDB(mSoundProperty,mSinkID));
}

am_Error_e CAmRoutingSender::handleSinkSoundProperties::writeDataToDatabase()
{
	std::vector<am_SoundProperty_s>::const_iterator it = mlistSoundProperties.begin();
	for (; it != mlistSoundProperties.end(); ++it)
	{
		mpDatabaseHandler->changeSinkSoundPropertyDB(*it, mSinkID);
	}
	return (am_Error_e::E_OK);
}

am_Error_e CAmRoutingSender::handleSourceSoundProperty::writeDataToDatabase()
{
	return (mpDatabaseHandler->changeSourceSoundPropertyDB(mSoundProperty,mSourceID));
}

am_Error_e CAmRoutingSender::handleSourceSoundProperties::writeDataToDatabase()
{
	std::vector<am_SoundProperty_s>::const_iterator it = mlistSoundProperties.begin();
	for (; it != mlistSoundProperties.end(); ++it)
	{
		mpDatabaseHandler->changeSourceSoundPropertyDB(*it, mSourceID);
	}
	return (am_Error_e::E_OK);
}

am_Error_e CAmRoutingSender::handleSourceState::writeDataToDatabase()
{
	return (mpDatabaseHandler->changeSourceState(mSourceID,mSourceState));
}

am_Error_e CAmRoutingSender::handleSourceVolume::writeDataToDatabase()
{
	return (mpDatabaseHandler->changeSourceVolume(mSourceID,returnVolume()));
}

am_Error_e CAmRoutingSender::handleSinkVolume::writeDataToDatabase()
{
	return (mpDatabaseHandler->changeSinkVolume(mSinkID,returnVolume()));
}

am_Error_e CAmRoutingSender::handleCrossFader::writeDataToDatabase()
{
	return (mpDatabaseHandler->changeCrossFaderHotSink(mCrossfaderID, mHotSink));
}

am_Error_e CAmRoutingSender::handleConnect::writeDataToDatabase()
{
	mConnectionPending = false;
	return (mpDatabaseHandler->changeConnectionFinal(mConnectionID));
}

am_Error_e CAmRoutingSender::handleDisconnect::writeDataToDatabase()
{
	return E_OK;
}

am_Error_e CAmRoutingSender::handleSetVolumes::writeDataToDatabase()
{
	std::vector<am_Volumes_s>::const_iterator iterator (mlistVolumes.begin());

	for (;iterator!=mlistVolumes.end();++iterator)
	{
		if (iterator->volumeType==VT_SINK)
		{
			return (mpDatabaseHandler->changeSinkVolume(iterator->volumeID.sink,iterator->volume));
		}
		else if (iterator->volumeType==VT_SOURCE)
		{
			return (mpDatabaseHandler->changeSourceVolume(iterator->volumeID.source,iterator->volume));
		}
	}
	return (am_Error_e::E_WRONG_FORMAT);
}

am_Error_e CAmRoutingSender::handleSetSinkNotificationConfiguration::writeDataToDatabase()
{
	return (mpDatabaseHandler->changeSinkNotificationConfigurationDB(mSinkID,mNotificationConfiguration));
}

am_Error_e CAmRoutingSender::handleSetSourceNotificationConfiguration::writeDataToDatabase()
{
	return (mpDatabaseHandler->changeSourceNotificationConfigurationDB(mSourceID,mNotificationConfiguration));
}

am_Error_e CAmRoutingSender::removeConnectionLookup(const am_connectionID_t connectionID)
{
    ConnectionInterfaceMap::iterator iter = mMapConnectionInterface.begin();
    iter = mMapConnectionInterface.find(connectionID);
    if (iter != mMapConnectionInterface.end())
    {
        mMapConnectionInterface.erase(iter);
        return (E_OK);
    }
    return (E_UNKNOWN);
}

CAmRoutingSender::handleConnect::~handleConnect()
{
	if (mConnectionPending)
	{
		mpDatabaseHandler->removeConnection(mConnectionID);
	}
}

CAmRoutingSender::handleDisconnect::~handleDisconnect()
{
	mpDatabaseHandler->removeConnection(mConnectionID);
}

}

