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
#include <utility>
#include <dirent.h>
#include <dlfcn.h>
#include <cassert>
#include <iostream>
#include <sstream>

#include "RoutingReceiver.h"
#include "PluginTemplate.h"
#include "DLTWrapper.h"

using namespace am;

#define REQUIRED_INTERFACE_VERSION_MAJOR 1
#define REQUIRED_INTERFACE_VERSION_MINOR 0

#define CALL_ALL_INTERFACES(...) 														 \
		std::vector<InterfaceNamePairs>::iterator iter = mListInterfaces.begin();	 	 \
		std::vector<InterfaceNamePairs>::iterator iterEnd = mListInterfaces.end();	 	 \
		for (; iter<iterEnd;++iter)													     \
		{																				 \
			(*iter).routingInterface->__VA_ARGS__;										 \
		}

RoutingSender::RoutingSender(const std::vector<std::string>& listOfPluginDirectories) :
        mHandleCount(0), //
        mlistActiveHandles(), //
        mListInterfaces(), //
        mMapConnectionInterface(), //
        mMapCrossfaderInterface(), //
        mMapDomainInterface(), //
        mMapSinkInterface(), //
        mMapSourceInterface(), //
        mMapHandleInterface(), //
        mRoutingReceiver()
{
    std::vector<std::string> sharedLibraryNameList;
    std::vector<std::string>::const_iterator dirIter = listOfPluginDirectories.begin();
    std::vector<std::string>::const_iterator dirIterEnd = listOfPluginDirectories.end();

    // search communicator plugins in configured directories
    for (; dirIter < dirIterEnd; ++dirIter)
    {
        const char* directoryName = dirIter->c_str();
        logInfo("Searching for HookPlugins in", directoryName);
        DIR *directory = opendir(directoryName);

        if (!directory)
        {
            logError("RoutingSender::RoutingSender Error opening directory: ", directoryName);
            continue;
        }

        // iterate content of directory
        struct dirent *itemInDirectory = 0;
        while ((itemInDirectory = readdir(directory)))
        {
            unsigned char entryType = itemInDirectory->d_type;
            std::string entryName = itemInDirectory->d_name;

            bool regularFile = (entryType == DT_REG || entryType == DT_LNK);
            bool sharedLibExtension = ("so" == entryName.substr(entryName.find_last_of(".") + 1));

            if (regularFile && sharedLibExtension)
            {
                logInfo("RoutingSender::RoutingSender adding file: ", entryName);
                std::string name(directoryName);
                sharedLibraryNameList.push_back(name + "/" + entryName);
            }
            else
            {
                logInfo("RoutingSender::RoutingSender PluginSearch ignoring file :", entryName);
            }
        }

        closedir(directory);
    }

    // iterate all communicator plugins and start them
    std::vector<std::string>::iterator iter = sharedLibraryNameList.begin();
    std::vector<std::string>::iterator iterEnd = sharedLibraryNameList.end();

    for (; iter != iterEnd; ++iter)
    {
        logInfo("RoutingSender::RoutingSender try loading: ", *iter);

        RoutingSendInterface* (*createFunc)();
        void* tempLibHandle = NULL;
        createFunc = getCreateFunction<RoutingSendInterface*()>(*iter, tempLibHandle);

        if (!createFunc)
        {
            logError("RoutingSender::RoutingSender Entry point of RoutingPlugin not found");
            continue;
        }

        RoutingSendInterface* router = createFunc();

        if (!router)
        {
            logError("RoutingSender::RoutingSender RoutingPlugin initialization failed. Entry Function not callable");
            continue;
        }

        InterfaceNamePairs routerInterface;
        routerInterface.routingInterface = router;

        //check libversion
        std::string version;
        router->getInterfaceVersion(version);
        uint16_t minorVersion, majorVersion;
        std::istringstream(version.substr(0, 1)) >> majorVersion;
        std::istringstream(version.substr(2, 1)) >> minorVersion;
        if (majorVersion < REQUIRED_INTERFACE_VERSION_MAJOR || ((majorVersion == REQUIRED_INTERFACE_VERSION_MAJOR) && (minorVersion > REQUIRED_INTERFACE_VERSION_MINOR)))
        {
            logInfo("RoutingPlugin initialization failed. Version of Interface to old");
            continue;
        }

        //here, the busname is saved together with the interface. Later The domains will register with the name and sinks, sources etc with the domain....
        router->returnBusName(routerInterface.busName);
        assert(!routerInterface.busName.empty());
        mListInterfaces.push_back(routerInterface);
        mListLibraryHandles.push_back(tempLibHandle);
    }
}

RoutingSender::~RoutingSender()
{
    unloadLibraries();
    HandlesMap::iterator it = mlistActiveHandles.begin();

    //clean up heap if existent
    for (; it != mlistActiveHandles.end(); ++it)
    {
        if (it->first.handleType == H_SETSINKSOUNDPROPERTIES || it->first.handleType == H_SETSOURCESOUNDPROPERTIES)
        {
            delete it->second.soundProperties;
        }
    }
}

am_Error_e RoutingSender::startupInterfaces(RoutingReceiver *iRoutingReceiver)
{
    mRoutingReceiver = iRoutingReceiver;
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
    return returnError;
}

am_Error_e RoutingSender::asyncAbort(const am_Handle_s& handle)
{
    HandleInterfaceMap::iterator iter = mMapHandleInterface.begin();
    iter = mMapHandleInterface.find(handle.handle);
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
    iter = mMapSinkInterface.find(sinkID);
    if (iter != mMapSinkInterface.end())
    {
        handleData.connectionID = connectionID;
        handle = createHandle(handleData, H_CONNECT);
        mMapConnectionInterface.insert(std::make_pair(connectionID, iter->second));
        mMapHandleInterface.insert(std::make_pair(handle.handle, iter->second));
        return iter->second->asyncConnect(handle, connectionID, sourceID, sinkID, connectionFormat);
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
        handleData.connectionID = connectionID;
        handle = createHandle(handleData, H_DISCONNECT);
        mMapHandleInterface.insert(std::make_pair(handle.handle, iter->second));
        am_Error_e returnVal = iter->second->asyncDisconnect(handle, connectionID);
        mMapConnectionInterface.erase(iter);
        return returnVal;
    }

    return E_NON_EXISTENT;
}

am_Error_e RoutingSender::asyncSetSinkVolume(am_Handle_s& handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time)
{
    am_handleData_c handleData;
    SinkInterfaceMap::iterator iter = mMapSinkInterface.begin();
    iter = mMapSinkInterface.find(sinkID);
    if (iter != mMapSinkInterface.end())
        handleData.sinkID = sinkID;
    handleData.volume = volume;
    handle = createHandle(handleData, H_SETSINKVOLUME);
    mMapHandleInterface.insert(std::make_pair(handle.handle, iter->second));
    return iter->second->asyncSetSinkVolume(handle, sinkID, volume, ramp, time);
    return E_NON_EXISTENT;
}

am_Error_e RoutingSender::asyncSetSourceVolume(am_Handle_s& handle, const am_sourceID_t sourceID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time)
{
    am_handleData_c handleData;
    SourceInterfaceMap::iterator iter = mMapSourceInterface.begin();
    iter = mMapSourceInterface.find(sourceID);
    if (iter != mMapSourceInterface.end())
        handleData.sourceID = sourceID;
    handleData.volume = volume;
    handle = createHandle(handleData, H_SETSOURCEVOLUME);
    mMapHandleInterface.insert(std::make_pair(handle.handle, iter->second));
    return iter->second->asyncSetSourceVolume(handle, sourceID, volume, ramp, time);
    return E_NON_EXISTENT;
}

am_Error_e RoutingSender::asyncSetSourceState(am_Handle_s& handle, const am_sourceID_t sourceID, const am_SourceState_e state)
{
    am_handleData_c handleData;
    SourceInterfaceMap::iterator iter = mMapSourceInterface.begin();
    iter = mMapSourceInterface.find(sourceID);
    if (iter != mMapSourceInterface.end())
        handleData.sourceID = sourceID;
    handleData.sourceState = state;
    handle = createHandle(handleData, H_SETSOURCESTATE);
    mMapHandleInterface.insert(std::make_pair(handle.handle, iter->second));
    return iter->second->asyncSetSourceState(handle, sourceID, state);
    return E_NON_EXISTENT;
}

am_Error_e RoutingSender::asyncSetSinkSoundProperty(am_Handle_s& handle, const am_sinkID_t sinkID, const am_SoundProperty_s & soundProperty)
{
    am_handleData_c handleData;
    SinkInterfaceMap::iterator iter = mMapSinkInterface.begin();
    iter = mMapSinkInterface.find(sinkID);
    if (iter != mMapSinkInterface.end())
        handleData.sinkID = sinkID;
    handleData.soundPropery = soundProperty;
    handle = createHandle(handleData, H_SETSINKSOUNDPROPERTY);
    mMapHandleInterface.insert(std::make_pair(handle.handle, iter->second));
    return iter->second->asyncSetSinkSoundProperty(handle, sinkID, soundProperty);
    return (E_NON_EXISTENT);
}

am_Error_e RoutingSender::asyncSetSourceSoundProperty(am_Handle_s& handle, const am_sourceID_t sourceID, const am_SoundProperty_s & soundProperty)
{
    am_handleData_c handleData;
    SourceInterfaceMap::iterator iter = mMapSourceInterface.begin();
    iter = mMapSourceInterface.find(sourceID);
    if (iter != mMapSourceInterface.end())
        handleData.sourceID = sourceID;
    handleData.soundPropery = soundProperty;
    handle = createHandle(handleData, H_SETSOURCESOUNDPROPERTY);
    mMapHandleInterface.insert(std::make_pair(handle.handle, iter->second));
    return iter->second->asyncSetSourceSoundProperty(handle, sourceID, soundProperty);
    return (E_NON_EXISTENT);
}

am_Error_e am::RoutingSender::asyncSetSourceSoundProperties(am_Handle_s& handle, const std::vector<am_SoundProperty_s> & listSoundProperties, const am_sourceID_t sourceID)
{
    am_handleData_c handleData;
    SourceInterfaceMap::iterator iter = mMapSourceInterface.begin();
    iter = mMapSourceInterface.find(sourceID);
    if (iter != mMapSourceInterface.end())
        handleData.sourceID = sourceID;
    handleData.soundProperties = new std::vector<am_SoundProperty_s>(listSoundProperties);
    handle = createHandle(handleData, H_SETSOURCESOUNDPROPERTIES);
    mMapHandleInterface.insert(std::make_pair(handle.handle, iter->second));
    return iter->second->asyncSetSourceSoundProperties(handle, sourceID, listSoundProperties);
    return (E_NON_EXISTENT);
}

am_Error_e am::RoutingSender::asyncSetSinkSoundProperties(am_Handle_s& handle, const std::vector<am_SoundProperty_s> & listSoundProperties, const am_sinkID_t sinkID)
{
    am_handleData_c handleData;
    SinkInterfaceMap::iterator iter = mMapSinkInterface.begin();
    iter = mMapSinkInterface.find(sinkID);
    if (iter != mMapSinkInterface.end())
        handleData.sinkID = sinkID;
    handleData.soundProperties = new std::vector<am_SoundProperty_s>(listSoundProperties);
    handle = createHandle(handleData, H_SETSINKSOUNDPROPERTIES);
    mMapHandleInterface.insert(std::make_pair(handle.handle, iter->second));
    return iter->second->asyncSetSinkSoundProperties(handle, sinkID, listSoundProperties);
    return (E_NON_EXISTENT);

}

am_Error_e RoutingSender::asyncCrossFade(am_Handle_s& handle, const am_crossfaderID_t crossfaderID, const am_HotSink_e hotSink, const am_RampType_e rampType, const am_time_t time)
{
    am_handleData_c handleData;
    CrossfaderInterfaceMap::iterator iter = mMapCrossfaderInterface.begin();
    iter = mMapCrossfaderInterface.find(crossfaderID);
    if (iter != mMapCrossfaderInterface.end())
        handleData.crossfaderID = crossfaderID;
    handleData.hotSink = hotSink;
    handle = createHandle(handleData, H_CROSSFADE);
    mMapHandleInterface.insert(std::make_pair(handle.handle, iter->second));
    return iter->second->asyncCrossFade(handle, crossfaderID, hotSink, rampType, time);
    return E_NON_EXISTENT;
}

am_Error_e RoutingSender::setDomainState(const am_domainID_t domainID, const am_DomainState_e domainState)
{
    DomainInterfaceMap::iterator iter = mMapDomainInterface.begin();
    iter = mMapDomainInterface.find(domainID);
    if (iter != mMapDomainInterface.end())
        return iter->second->setDomainState(domainID, domainState);
    return E_NON_EXISTENT;
}

am_Error_e RoutingSender::addDomainLookup(const am_Domain_s& domainData)
{
    std::vector<InterfaceNamePairs>::iterator iter = mListInterfaces.begin();
    std::vector<InterfaceNamePairs>::iterator iterEnd = mListInterfaces.end();
    for (; iter < iterEnd; ++iter)
    {
        if ((*iter).busName.compare(domainData.busname) == 0)
        {
            mMapDomainInterface.insert(std::make_pair(domainData.domainID, (*iter).routingInterface));
            return E_OK;
        }
    }

    return E_UNKNOWN;
}

am_Error_e RoutingSender::addSourceLookup(const am_Source_s& sourceData)
{
    DomainInterfaceMap::iterator iter = mMapDomainInterface.begin();
    iter = mMapDomainInterface.find(sourceData.domainID);
    if (iter != mMapDomainInterface.end())
    {
        mMapSourceInterface.insert(std::make_pair(sourceData.sourceID, iter->second));
        return E_OK;
    }

    return E_UNKNOWN;
}

am_Error_e RoutingSender::addSinkLookup(const am_Sink_s& sinkData)
{
    DomainInterfaceMap::iterator iter = mMapDomainInterface.begin();
    iter = mMapDomainInterface.find(sinkData.domainID);
    if (iter != mMapDomainInterface.end())
    {
        mMapSinkInterface.insert(std::make_pair(sinkData.sinkID, iter->second));
        return E_OK;
    }

    return E_UNKNOWN;
}

am_Error_e RoutingSender::addCrossfaderLookup(const am_Crossfader_s& crossfaderData)
{
    DomainInterfaceMap::iterator iter = mMapSourceInterface.begin();
    iter = mMapSourceInterface.find(crossfaderData.sourceID);
    if (iter != mMapSourceInterface.end())
    {
        mMapSourceInterface.insert(std::make_pair(crossfaderData.crossfaderID, iter->second));
        return E_OK;
    }

    return E_UNKNOWN;
}

am_Error_e RoutingSender::removeDomainLookup(const am_domainID_t domainID)
{
    DomainInterfaceMap::iterator iter = mMapDomainInterface.begin();
    iter = mMapDomainInterface.find(domainID);
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
    iter = mMapSourceInterface.find(sourceID);
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
    iter = mMapSinkInterface.find(sinkID);
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
    iter = mMapCrossfaderInterface.find(crossfaderID);
    if (iter != mMapCrossfaderInterface.end())
    {
        mMapCrossfaderInterface.erase(iter);
        return E_OK;
    }

    return E_NON_EXISTENT;
}

am_Error_e RoutingSender::removeHandle(const am_Handle_s& handle)
{
    if (mlistActiveHandles.erase(handle))
        return E_OK;
    return E_UNKNOWN;
}

am_Error_e RoutingSender::getListHandles(std::vector<am_Handle_s> & listHandles) const
{
    listHandles.clear();
    HandlesMap::const_iterator it = mlistActiveHandles.begin();
    for (; it != mlistActiveHandles.end(); ++it)
    {
        listHandles.push_back(it->first);
    }
    return E_OK;
}

am_Handle_s RoutingSender::createHandle(const am_handleData_c& handleData, const am_Handle_e type)
{
    am_Handle_s handle;
    handle.handle = ++mHandleCount; //todo: handle overflows here...
    handle.handleType = type;
    mlistActiveHandles.insert(std::make_pair(handle, handleData));
    return handle;
}

RoutingSender::am_handleData_c RoutingSender::returnHandleData(const am_Handle_s handle) const
{
    HandlesMap::const_iterator it = mlistActiveHandles.begin();
    it = mlistActiveHandles.find(handle);
    return (it->second);
}

void am::RoutingSender::setRoutingReady()
{
    mRoutingReceiver->waitOnStartup(false);
    std::vector<InterfaceNamePairs>::iterator iter = mListInterfaces.begin();
    std::vector<InterfaceNamePairs>::iterator iterEnd = mListInterfaces.end();
    for (; iter < iterEnd; ++iter)
    {
        (*iter).routingInterface->setRoutingReady(mRoutingReceiver->getStartupHandle());
    }
    mRoutingReceiver->waitOnStartup(true);
}

void am::RoutingSender::setRoutingRundown()
{
    mRoutingReceiver->waitOnRundown(false);
    std::vector<InterfaceNamePairs>::iterator iter = mListInterfaces.begin();
    std::vector<InterfaceNamePairs>::iterator iterEnd = mListInterfaces.end();
    for (; iter < iterEnd; ++iter)
    {
        (*iter).routingInterface->setRoutingRundown(mRoutingReceiver->getStartupHandle());
    }
    mRoutingReceiver->waitOnRundown(true);
}

void RoutingSender::unloadLibraries(void)
{
    std::vector<void*>::iterator iterator = mListLibraryHandles.begin();
    for (; iterator < mListLibraryHandles.end(); ++iterator)
    {
        dlclose(*iterator);
    }
    mListLibraryHandles.clear();
}

am_Error_e RoutingSender::getListPlugins(std::vector<std::string>& interfaces) const
{
    std::vector<InterfaceNamePairs>::const_iterator it = mListInterfaces.begin();
    for (; it != mListInterfaces.end(); ++it)
    {
        interfaces.push_back(it->busName);
    }
    return E_OK;
}

void RoutingSender::getInterfaceVersion(std::string & version) const
{
    version = RoutingSendVersion;
}

