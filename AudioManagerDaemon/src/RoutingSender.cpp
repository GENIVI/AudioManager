/*
 * RoutingSender.cpp
 *
 *  Created on: Oct 26, 2011
 *      Author: christian
 */

#include "RoutingSender.h"
#include <utility>
using namespace am;

#define CALL_ALL_INTERFACES(...) 														 \
		std::vector<InterfaceNamePairs>::iterator iter = mListInterfaces.begin();	 	 \
		std::vector<InterfaceNamePairs>::iterator iterEnd = mListInterfaces.end();	 	 \
		for (; iter<iterEnd;++iter)													 \
		{																				 \
			(*iter).routingInterface->__VA_ARGS__;										 \
		}

const char* routingPluginDirectories[] = { "/home/christian/workspace/gitserver/build/plugins/routing"};
uint16_t routingPluginDirectoriesCount = sizeof(routingPluginDirectories) / sizeof(routingPluginDirectories[0]);

RoutingSender::RoutingSender()
{
	std::vector<std::string> sharedLibraryNameList;

    // search communicator plugins in configured directories
    for (uint16_t dirIndex = 0; dirIndex < routingPluginDirectoriesCount; dirIndex++)
    {
		const char* directoryName = routingPluginDirectories[dirIndex];
		//DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Searching for HookPlugins in"),DLT_STRING(directoryName));
		DIR *directory = opendir(directoryName);

		if (!directory)
		{
			//DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Error opening directory "),DLT_STRING(dirName.c_str()));
		}

        // iterate content of directory
        struct dirent *itemInDirectory = 0;
        while ((itemInDirectory = readdir(directory)))
        {
			unsigned char entryType = itemInDirectory->d_type;
			std::string entryName = itemInDirectory->d_name;

			bool regularFile = (entryType == DT_REG);
			bool sharedLibExtension = ("so" == entryName.substr(entryName.find_last_of(".") + 1));

			if (regularFile && sharedLibExtension)
			{
			//	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("PluginSearch adding file "),DLT_STRING(entryName.c_str()));
			  std::string name(directoryName);
			  sharedLibraryNameList.push_back(name + "/" + entryName);
			}
			else
			{
			//DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("PluginSearch ignoring file "),DLT_STRING(entryName.c_str()));
			}
        }

          closedir(directory);
    }

    // iterate all communicator plugins and start them
    std::vector<std::string>::iterator iter = sharedLibraryNameList.begin();
    std::vector<std::string>::iterator iterEnd = sharedLibraryNameList.end();

    for (; iter != iterEnd; ++iter)
    {
    	//DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Loading Hook plugin"),DLT_STRING(iter->c_str()));

    	RoutingSendInterface* (*createFunc)();
        createFunc = getCreateFunction<RoutingSendInterface*()>(*iter);

        if (!createFunc)
        {
           // DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Entry point of Communicator not found"));
            continue;
        }

        RoutingSendInterface* router = createFunc();

        if (!router)
        {
        	//DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("HookPlugin initialization failed. Entry Function not callable"));
            continue;
        }

        InterfaceNamePairs routerInterface;
        routerInterface.routingInterface = router;
        router->returnBusName(routerInterface.busName);
        mListInterfaces.push_back(routerInterface);
    }
}

RoutingSender::~RoutingSender()
{
}

void RoutingSender::routingInterfacesReady()
{
	CALL_ALL_INTERFACES(routingInterfacesReady())
}

void RoutingSender::routingInterfacesRundown()
{
	CALL_ALL_INTERFACES(routingInterfacesRundown())
}

void RoutingSender::startupRoutingInterface(RoutingReceiveInterface *routingreceiveinterface)
{
	CALL_ALL_INTERFACES(startupRoutingInterface(routingreceiveinterface))
}

am_Error_e RoutingSender::asyncAbort(const am_Handle_s handle)
{
	//Todo: map handle to RoutingInterface via Database.
	return E_UNKNOWN;
}



am_Error_e RoutingSender::asyncConnect(const am_Handle_s handle, const am_connectionID_t connectionID, const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_ConnectionFormat_e connectionFormat)
{
	SinkInterfaceMap::iterator iter = mMapSinkInterface.begin();
	mMapSinkInterface.find(sinkID);
    if (iter != mMapSinkInterface.end())
    {
    	return iter->second->asyncConnect(handle,connectionID,sourceID,sinkID,connectionFormat);
    	mMapConnectionInterface.insert(std::make_pair(connectionID,iter->second));
    }

    return E_NON_EXISTENT;
}



am_Error_e RoutingSender::asyncDisconnect(const am_Handle_s handle, const am_connectionID_t connectionID)
{
	ConnectionInterfaceMap::iterator iter = mMapConnectionInterface.begin();
	mMapConnectionInterface.find(connectionID);
    if (iter != mMapConnectionInterface.end())
    {
    	return iter->second->asyncDisconnect(handle,connectionID);
    	mMapConnectionInterface.erase(iter);
    }

    return E_NON_EXISTENT;
}



am_Error_e RoutingSender::asyncSetSinkVolume(const am_Handle_s handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time)
{
	SinkInterfaceMap::iterator iter = mMapSinkInterface.begin();
	mMapSinkInterface.find(sinkID);
    if (iter != mMapSinkInterface.end())
    	return iter->second->asyncSetSinkVolume(handle,sinkID,volume,ramp,time);
    return E_NON_EXISTENT;
}



am_Error_e RoutingSender::asyncSetSourceVolume(const am_Handle_s handle, const am_sourceID_t sourceID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time)
{
	SourceInterfaceMap::iterator iter = mMapSourceInterface.begin();
	mMapSourceInterface.find(sourceID);
    if (iter != mMapSourceInterface.end())
    	return iter->second->asyncSetSourceVolume(handle,sourceID,volume,ramp,time);
    return E_NON_EXISTENT;
}



am_Error_e RoutingSender::asyncSetSourceState(const am_Handle_s handle, const am_sourceID_t sourceID, const am_SourceState_e state)
{
	SourceInterfaceMap::iterator iter = mMapSourceInterface.begin();
	mMapSourceInterface.find(sourceID);
    if (iter != mMapSourceInterface.end())
    	return iter->second->asyncSetSourceState(handle,sourceID,state);
    return E_NON_EXISTENT;
}



am_Error_e RoutingSender::asyncSetSinkSoundProperty(const am_Handle_s handle, const am_sinkID_t sinkID, const am_SoundProperty_s & soundProperty)
{
	SinkInterfaceMap::iterator iter = mMapSinkInterface.begin();
	mMapSinkInterface.find(sinkID);
    if (iter != mMapSinkInterface.end())
    	return iter->second->asyncSetSinkSoundProperty(handle,soundProperty,sinkID);
    return E_NON_EXISTENT;
}



am_Error_e RoutingSender::asyncSetSourceSoundProperty(const am_Handle_s handle, const am_sourceID_t sourceID, const am_SoundProperty_s & soundProperty)
{
	SourceInterfaceMap::iterator iter = mMapSourceInterface.begin();
	mMapSourceInterface.find(sourceID);
    if (iter != mMapSourceInterface.end())
    	return iter->second->asyncSetSourceSoundProperty(handle,soundProperty,sourceID);
    return E_NON_EXISTENT;
}



am_Error_e RoutingSender::asyncCrossFade(const am_Handle_s handle, const am_crossfaderID_t crossfaderID, const am_HotSink_e hotSink, const am_RampType_e rampType, const am_time_t time)
{
	CrossfaderInterfaceMap::iterator iter = mMapCrossfaderInterface.begin();
	mMapCrossfaderInterface.find(crossfaderID);
    if (iter != mMapCrossfaderInterface.end())
    	return iter->second->asyncCrossFade(handle,crossfaderID,hotSink,rampType,time);
    return E_NON_EXISTENT;
}



am_Error_e RoutingSender::setDomainState(const am_domainID_t domainID, const am_DomainState_e domainState)
{
	DomainInterfaceMap::iterator iter = mMapDomainInterface.begin();
	mMapDomainInterface.find(domainID);
    if (iter != mMapDomainInterface.end())
    	return iter->second->setDomainState(domainID,domainState);
    return E_NON_EXISTENT;
}

am_Error_e RoutingSender::addDomainLookup(const am_Domain_s& domainData)
{
	std::vector<InterfaceNamePairs>::iterator iter = mListInterfaces.begin();
	std::vector<InterfaceNamePairs>::iterator iterEnd = mListInterfaces.end();
	for (; iter<iterEnd;++iter)
	{
		if((*iter).busName.compare(domainData.busname) == 0)
		{
			mMapDomainInterface.insert(std::make_pair(domainData.domainID,(*iter).routingInterface));
			return E_OK;
		}
	}

	return E_UNKNOWN;
}



am_Error_e RoutingSender::addSourceLookup(const am_Source_s& sourceData)
{
	DomainInterfaceMap::iterator iter = mMapDomainInterface.begin();
	mMapDomainInterface.find(sourceData.domainID);
    if (iter != mMapDomainInterface.end())
    {
    	mMapSourceInterface.insert(std::make_pair(sourceData.sourceID,iter->second));
    	return E_OK;
    }

    return E_UNKNOWN;
}



am_Error_e RoutingSender::addSinkLookup(const am_Sink_s& sinkData)
{
	DomainInterfaceMap::iterator iter = mMapDomainInterface.begin();
	mMapDomainInterface.find(sinkData.domainID);
    if (iter != mMapDomainInterface.end())
    {
    	mMapSinkInterface.insert(std::make_pair(sinkData.sinkID,iter->second));
    	return E_OK;
    }

    return E_UNKNOWN;
}



am_Error_e RoutingSender::addCrossfaderLookup(const am_Crossfader_s& crossfaderData)
{
	DomainInterfaceMap::iterator iter = mMapSourceInterface.begin();
	mMapSourceInterface.find(crossfaderData.sourceID);
    if (iter != mMapSourceInterface.end())
    {
    	mMapSourceInterface.insert(std::make_pair(crossfaderData.crossfaderID,iter->second));
    	return E_OK;
    }

    return E_UNKNOWN;
}

am_Error_e RoutingSender::removeDomainLookup(const am_domainID_t domainID)
{
	DomainInterfaceMap::iterator iter = mMapDomainInterface.begin();
	mMapDomainInterface.find(domainID);
    if (iter != mMapDomainInterface.end())
    {
    	mMapDomainInterface.erase(iter);
    	return E_OK;
    }

    return E_NON_EXISTENT;
}



am_Error_e RoutingSender::removeSourceLookup(const am_sourceID_t sourceID)
{
	SourceInterfaceMap::iterator iter = mMapSourceInterface.begin();
	mMapSourceInterface.find(sourceID);
    if (iter != mMapSourceInterface.end())
    {
    	mMapSourceInterface.erase(iter);
    	return E_OK;
    }

    return E_NON_EXISTENT;
}



am_Error_e RoutingSender::removeSinkLookup(const am_sinkID_t sinkID)
{
	SinkInterfaceMap::iterator iter = mMapSinkInterface.begin();
	mMapSinkInterface.find(sinkID);
    if (iter != mMapSinkInterface.end())
    {
    	mMapSinkInterface.erase(iter);
    	return E_OK;
    }

    return E_NON_EXISTENT;
}



am_Error_e RoutingSender::removeCrossfaderLookup(const am_crossfaderID_t crossfaderID)
{
	CrossfaderInterfaceMap::iterator iter = mMapCrossfaderInterface.begin();
	mMapCrossfaderInterface.find(crossfaderID);
    if (iter != mMapCrossfaderInterface.end())
    {
    	mMapCrossfaderInterface.erase(iter);
    	return E_OK;
    }

    return E_NON_EXISTENT;
}







