/**
* Copyright (C) 2011, BMW AG
*
* GeniviAudioMananger AudioManagerDaemon
*
* \file RoutingSender.h
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

#include "RoutingSender.h"
#include "PluginTemplate.h"
#include <utility>
#include <dirent.h>
#include <dlfcn.h>

using namespace am;

#define CALL_ALL_INTERFACES(...) 														 \
		std::vector<InterfaceNamePairs>::iterator iter = mListInterfaces.begin();	 	 \
		std::vector<InterfaceNamePairs>::iterator iterEnd = mListInterfaces.end();	 	 \
		for (; iter<iterEnd;++iter)													 \
		{																				 \
			(*iter).routingInterface->__VA_ARGS__;										 \
		}

RoutingSender::RoutingSender(const std::vector<std::string>& listOfPluginDirectories)

	:mHandleCount(0),
	 mlistActiveHandles(),
	 mListInterfaces(),
	 mMapConnectionInterface(),
	 mMapCrossfaderInterface(),
	 mMapDomainInterface(),
	 mMapSinkInterface(),
	 mMapSourceInterface(),
	 mMapHandleInterface()
{
	std::vector<std::string> sharedLibraryNameList;
    std::vector<std::string>::const_iterator dirIter = listOfPluginDirectories.begin();
    std::vector<std::string>::const_iterator dirIterEnd = listOfPluginDirectories.end();

    // search communicator plugins in configured directories
    for (; dirIter < dirIterEnd; ++dirIter)
    {
		const char* directoryName = dirIter->c_str();
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
    	void* tempLibHandle=NULL;
        createFunc = getCreateFunction<RoutingSendInterface*()>(*iter,tempLibHandle);

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

        //here, the busname is saved together with the interface. Later The domains will register with the name and sinks, sources etc with the domain....
        router->returnBusName(routerInterface.busName);
        mListInterfaces.push_back(routerInterface);
        mListLibraryHandles.push_back(tempLibHandle);
    }
    //now all plugins are loaded, so the interface is ready
}

RoutingSender::~RoutingSender()
{
	unloadLibraries();
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

am_Error_e RoutingSender::asyncAbort(const am_Handle_s& handle)
{
	HandleInterfaceMap::iterator iter = mMapHandleInterface.begin();
	iter=mMapHandleInterface.find(handle.handle);
    if (iter != mMapHandleInterface.end())
    {
    	return iter->second->asyncAbort(handle);
    }

    return E_NON_EXISTENT;
}



am_Error_e RoutingSender::asyncConnect(am_Handle_s& handle, const am_connectionID_t connectionID, const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_ConnectionFormat_e connectionFormat)
{
	am_handleData_c handleData;
	SinkInterfaceMap::iterator iter = mMapSinkInterface.begin();
	iter=mMapSinkInterface.find(sinkID);
    if (iter != mMapSinkInterface.end())
    {
    	handleData.connectionID=connectionID;
    	handle=createHandle(handleData,H_CONNECT);
    	mMapConnectionInterface.insert(std::make_pair(connectionID,iter->second));
    	mMapHandleInterface.insert(std::make_pair(handle.handle,iter->second));
    	return iter->second->asyncConnect(handle,connectionID,sourceID,sinkID,connectionFormat);
    }

    return E_NON_EXISTENT;
}



am_Error_e RoutingSender::asyncDisconnect(am_Handle_s& handle, const am_connectionID_t connectionID)
{
	am_handleData_c handleData;
	ConnectionInterfaceMap::iterator iter = mMapConnectionInterface.begin();
	mMapConnectionInterface.find(connectionID);
    if (iter != mMapConnectionInterface.end())
    {
    	handleData.connectionID=connectionID;
    	handle=createHandle(handleData,H_DISCONNECT);
    	mMapHandleInterface.insert(std::make_pair(handle.handle,iter->second));
    	am_Error_e returnVal=iter->second->asyncDisconnect(handle,connectionID);
    	mMapConnectionInterface.erase(iter);
    	return returnVal;
    }

    return E_NON_EXISTENT;
}



am_Error_e RoutingSender::asyncSetSinkVolume(am_Handle_s& handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time)
{
	am_handleData_c handleData;
	SinkInterfaceMap::iterator iter = mMapSinkInterface.begin();
	iter=mMapSinkInterface.find(sinkID);
    if (iter != mMapSinkInterface.end())
    	handleData.sinkID=sinkID;
    	handleData.volume=volume;
    	handle=createHandle(handleData,H_SETSINKVOLUME);
		mMapHandleInterface.insert(std::make_pair(handle.handle,iter->second));
    	return iter->second->asyncSetSinkVolume(handle,sinkID,volume,ramp,time);
    return E_NON_EXISTENT;
}



am_Error_e RoutingSender::asyncSetSourceVolume(am_Handle_s& handle, const am_sourceID_t sourceID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time)
{
	am_handleData_c handleData;
	SourceInterfaceMap::iterator iter = mMapSourceInterface.begin();
	iter=mMapSourceInterface.find(sourceID);
    if (iter != mMapSourceInterface.end())
    	handleData.sourceID=sourceID;
    	handleData.volume=volume;
    	handle=createHandle(handleData,H_SETSOURCEVOLUME);
		mMapHandleInterface.insert(std::make_pair(handle.handle,iter->second));
    	return iter->second->asyncSetSourceVolume(handle,sourceID,volume,ramp,time);
    return E_NON_EXISTENT;
}



am_Error_e RoutingSender::asyncSetSourceState(am_Handle_s& handle, const am_sourceID_t sourceID, const am_SourceState_e state)
{
	am_handleData_c handleData;
	SourceInterfaceMap::iterator iter = mMapSourceInterface.begin();
	iter=mMapSourceInterface.find(sourceID);
    if (iter != mMapSourceInterface.end())
    	handleData.sourceID=sourceID;
    	handleData.sourceState=state;
    	handle=createHandle(handleData,H_SETSOURCESTATE);
		mMapHandleInterface.insert(std::make_pair(handle.handle,iter->second));
    	return iter->second->asyncSetSourceState(handle,sourceID,state);
    return E_NON_EXISTENT;
}



am_Error_e RoutingSender::asyncSetSinkSoundProperty(am_Handle_s& handle, const am_sinkID_t sinkID, const am_SoundProperty_s & soundProperty)
{
	am_handleData_c handleData;
	SinkInterfaceMap::iterator iter = mMapSinkInterface.begin();
	iter=mMapSinkInterface.find(sinkID);
    if (iter != mMapSinkInterface.end())
    	handleData.sinkID=sinkID;
    	handleData.soundPropery=soundProperty;
    	handle=createHandle(handleData,H_SETSINKSOUNDPROPERTY);
		mMapHandleInterface.insert(std::make_pair(handle.handle,iter->second));
    	return iter->second->asyncSetSinkSoundProperty(handle,soundProperty,sinkID);
    return E_NON_EXISTENT;
}



am_Error_e RoutingSender::asyncSetSourceSoundProperty(am_Handle_s& handle, const am_sourceID_t sourceID, const am_SoundProperty_s & soundProperty)
{
	am_handleData_c handleData;
	SourceInterfaceMap::iterator iter = mMapSourceInterface.begin();
	iter=mMapSourceInterface.find(sourceID);
    if (iter != mMapSourceInterface.end())
    	handleData.sourceID=sourceID;
    	handleData.soundPropery=soundProperty;
    	handle=createHandle(handleData,H_SETSOURCESOUNDPROPERTY);
		mMapHandleInterface.insert(std::make_pair(handle.handle,iter->second));
    	return iter->second->asyncSetSourceSoundProperty(handle,soundProperty,sourceID);
    return E_NON_EXISTENT;
}



am_Error_e RoutingSender::asyncCrossFade(am_Handle_s& handle, const am_crossfaderID_t crossfaderID, const am_HotSink_e hotSink, const am_RampType_e rampType, const am_time_t time)
{
	am_handleData_c handleData;
	CrossfaderInterfaceMap::iterator iter = mMapCrossfaderInterface.begin();
	iter=mMapCrossfaderInterface.find(crossfaderID);
    if (iter != mMapCrossfaderInterface.end())
    	handleData.crossfaderID=crossfaderID;
    	handleData.hotSink=hotSink;
    	handle=createHandle(handleData,H_CROSSFADE);
		mMapHandleInterface.insert(std::make_pair(handle.handle,iter->second));
    	return iter->second->asyncCrossFade(handle,crossfaderID,hotSink,rampType,time);
    return E_NON_EXISTENT;
}



am_Error_e RoutingSender::setDomainState(const am_domainID_t domainID, const am_DomainState_e domainState)
{
	DomainInterfaceMap::iterator iter = mMapDomainInterface.begin();
	iter=mMapDomainInterface.find(domainID);
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
	iter=mMapDomainInterface.find(sourceData.domainID);
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
	iter=mMapDomainInterface.find(sinkData.domainID);
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
	iter=mMapSourceInterface.find(crossfaderData.sourceID);
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
	iter=mMapDomainInterface.find(domainID);
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
	iter=mMapSourceInterface.find(sourceID);
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
	iter=mMapSinkInterface.find(sinkID);
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
	iter=mMapCrossfaderInterface.find(crossfaderID);
    if (iter != mMapCrossfaderInterface.end())
    {
    	mMapCrossfaderInterface.erase(iter);
    	return E_OK;
    }

    return E_NON_EXISTENT;
}


am_Error_e RoutingSender::removeHandle(const am_Handle_s& handle)
{
	if(mlistActiveHandles.erase(handle)) return E_OK;
	return E_UNKNOWN;
}

am_Error_e RoutingSender::getListHandles(std::vector<am_Handle_s> & listHandles) const
{
	listHandles.clear();
	HandlesMap::const_iterator it=mlistActiveHandles.begin();
	for(;it!=mlistActiveHandles.end();++it)
	{
		listHandles.push_back(it->first);
	}
	return E_OK;
}

am_Handle_s RoutingSender::createHandle(const am_handleData_c& handleData, const am_Handle_e type)
{
	am_Handle_s handle;
	handle.handle=++mHandleCount; //todo: handle overflows here...
	handle.handleType=type;
	mlistActiveHandles.insert(std::make_pair(handle,handleData));
	return handle;
}

RoutingSender::am_handleData_c RoutingSender::returnHandleData(am_Handle_s handle)
{
	HandlesMap::iterator it=mlistActiveHandles.begin();
	it=mlistActiveHandles.find(handle);
	return (it->second);
}

void RoutingSender::unloadLibraries(void)
{
	std::vector<void*>::iterator iterator=mListLibraryHandles.begin();
	for(;iterator<mListLibraryHandles.end();++iterator)
	{
		dlclose(*iterator);
	}
	mListLibraryHandles.clear();
}











