/**
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
 * \author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
 *
 * \file CAmRoutingSender.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include "CAmRoutingSender.h"
#include <utility>
#include <dirent.h>
#include <dlfcn.h>
#include <cassert>
#include <iostream>
#include <sstream>
#include "CAmRoutingReceiver.h"
#include "TAmPluginTemplate.h"
#include "shared/CAmDltWrapper.h"

namespace am
{

#define REQUIRED_INTERFACE_VERSION_MAJOR 1  //!< major interface version. All versions smaller than this will be rejected
#define REQUIRED_INTERFACE_VERSION_MINOR 0 //!< minor interface version. All versions smaller than this will be rejected

CAmRoutingSender::CAmRoutingSender(const std::vector<std::string>& listOfPluginDirectories) :
        mHandleCount(0), //
        mlistActiveHandles(), //
        mListInterfaces(), //
        mMapConnectionInterface(), //
        mMapCrossfaderInterface(), //
        mMapDomainInterface(), //
        mMapSinkInterface(), //
        mMapSourceInterface(), //
        mMapHandleInterface(), //
        mpRoutingReceiver()
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

        IAmRoutingSend* (*createFunc)();
        void* tempLibHandle = NULL;
        createFunc = getCreateFunction<IAmRoutingSend*()>(*iter, tempLibHandle);

        if (!createFunc)
        {
            logError("RoutingSender::RoutingSender Entry point of RoutingPlugin not found");
            continue;
        }

        IAmRoutingSend* router = createFunc();

        if (!router)
        {
            logError("RoutingSender::RoutingSender RoutingPlugin initialization failed. Entry Function not callable");
            dlclose(tempLibHandle);
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

    //clean up heap if existent
    for (; it != mlistActiveHandles.end(); ++it)
    {
        if (it->first.handleType == H_SETSINKSOUNDPROPERTIES || it->first.handleType == H_SETSOURCESOUNDPROPERTIES)
        {
            delete it->second.soundProperties;
        }
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
    HandleInterfaceMap::iterator iter = mMapHandleInterface.begin();
    iter = mMapHandleInterface.find(handle.handle);
    if (iter != mMapHandleInterface.end())
    {
        return (iter->second->asyncAbort(handle));
    }

    return (E_NON_EXISTENT);
}

am_Error_e CAmRoutingSender::asyncConnect(am_Handle_s& handle, const am_connectionID_t connectionID, const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_ConnectionFormat_e connectionFormat)
{
    am_handleData_c handleData;
    SinkInterfaceMap::iterator iter = mMapSinkInterface.begin();
    iter = mMapSinkInterface.find(sinkID);
    if (iter != mMapSinkInterface.end())
    {
        handleData.connectionID = connectionID;
        handle = createHandle(handleData, H_CONNECT);
        mMapConnectionInterface.insert(std::make_pair(connectionID, iter->second));
        mMapHandleInterface.insert(std::make_pair(+ handle.handle, iter->second));
        return (iter->second->asyncConnect(handle, connectionID, sourceID, sinkID, connectionFormat));
    }

    return (E_NON_EXISTENT);
}

am_Error_e CAmRoutingSender::asyncDisconnect(am_Handle_s& handle, const am_connectionID_t connectionID)
{
    am_handleData_c handleData;
    ConnectionInterfaceMap::iterator iter = mMapConnectionInterface.begin();
    iter = mMapConnectionInterface.find(connectionID);
    if (iter != mMapConnectionInterface.end())
    {
        handleData.connectionID = connectionID;
        handle = createHandle(handleData, H_DISCONNECT);
        mMapHandleInterface.insert(std::make_pair(+ handle.handle, iter->second));
        am_Error_e returnVal = iter->second->asyncDisconnect(handle, connectionID);
        mMapConnectionInterface.erase(iter);
        return (returnVal);
    }

    return (E_NON_EXISTENT);
}

am_Error_e CAmRoutingSender::asyncSetSinkVolume(am_Handle_s& handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time)
{
    am_handleData_c handleData;
    SinkInterfaceMap::iterator iter = mMapSinkInterface.begin();
    iter = mMapSinkInterface.find(sinkID);
    if (iter != mMapSinkInterface.end())
    {
        handleData.sinkID = sinkID;
        handleData.volume = volume;
        handle = createHandle(handleData, H_SETSINKVOLUME);
        mMapHandleInterface.insert(std::make_pair(+ handle.handle, iter->second));
        return (iter->second->asyncSetSinkVolume(handle, sinkID, volume, ramp, time));
    }
    return (E_NON_EXISTENT);
}

am_Error_e CAmRoutingSender::asyncSetSourceVolume(am_Handle_s& handle, const am_sourceID_t sourceID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time)
{
    am_handleData_c handleData;
    SourceInterfaceMap::iterator iter = mMapSourceInterface.begin();
    iter = mMapSourceInterface.find(sourceID);
    if (iter != mMapSourceInterface.end())
    {
        handleData.sourceID = sourceID;
        handleData.volume = volume;
        handle = createHandle(handleData, H_SETSOURCEVOLUME);
        mMapHandleInterface.insert(std::make_pair(+ handle.handle, iter->second));
        return (iter->second->asyncSetSourceVolume(handle, sourceID, volume, ramp, time));
    }
    return (E_NON_EXISTENT);
}

am_Error_e CAmRoutingSender::asyncSetSourceState(am_Handle_s& handle, const am_sourceID_t sourceID, const am_SourceState_e state)
{
    am_handleData_c handleData;
    SourceInterfaceMap::iterator iter = mMapSourceInterface.begin();
    iter = mMapSourceInterface.find(sourceID);
    if (iter != mMapSourceInterface.end())
    {
        handleData.sourceID = sourceID;
        handleData.sourceState = state;
        handle = createHandle(handleData, H_SETSOURCESTATE);
        mMapHandleInterface.insert(std::make_pair(+ handle.handle, iter->second));
        return (iter->second->asyncSetSourceState(handle, sourceID, state));
    }
    return (E_NON_EXISTENT);
}

am_Error_e CAmRoutingSender::asyncSetSinkSoundProperty(am_Handle_s& handle, const am_sinkID_t sinkID, const am_SoundProperty_s & soundProperty)
{
    am_handleData_c handleData;
    SinkInterfaceMap::iterator iter = mMapSinkInterface.begin();
    iter = mMapSinkInterface.find(sinkID);
    if (iter != mMapSinkInterface.end())
    {
        handleData.sinkID = sinkID;
        handleData.soundPropery = soundProperty;
        handle = createHandle(handleData, H_SETSINKSOUNDPROPERTY);
        mMapHandleInterface.insert(std::make_pair(+ handle.handle, iter->second));
        return (iter->second->asyncSetSinkSoundProperty(handle, sinkID, soundProperty));
    }
    return (E_NON_EXISTENT);
}

am_Error_e CAmRoutingSender::asyncSetSourceSoundProperty(am_Handle_s& handle, const am_sourceID_t sourceID, const am_SoundProperty_s & soundProperty)
{
    am_handleData_c handleData;
    SourceInterfaceMap::iterator iter = mMapSourceInterface.begin();
    iter = mMapSourceInterface.find(sourceID);
    if (iter != mMapSourceInterface.end())
    {
        handleData.sourceID = sourceID;
        handleData.soundPropery = soundProperty;
        handle = createHandle(handleData, H_SETSOURCESOUNDPROPERTY);
        mMapHandleInterface.insert(std::make_pair(+ handle.handle, iter->second));
        return (iter->second->asyncSetSourceSoundProperty(handle, sourceID, soundProperty));
    }
    return (E_NON_EXISTENT);
}

am_Error_e CAmRoutingSender::asyncSetSourceSoundProperties(am_Handle_s& handle, const std::vector<am_SoundProperty_s> & listSoundProperties, const am_sourceID_t sourceID)
{
    am_handleData_c handleData;
    SourceInterfaceMap::iterator iter = mMapSourceInterface.begin();
    iter = mMapSourceInterface.find(sourceID);
    if (iter != mMapSourceInterface.end())
    {
        handleData.sourceID = sourceID;
        handleData.soundProperties = new std::vector<am_SoundProperty_s>(listSoundProperties);
        handle = createHandle(handleData, H_SETSOURCESOUNDPROPERTIES);
        mMapHandleInterface.insert(std::make_pair(+ handle.handle, iter->second));
        return (iter->second->asyncSetSourceSoundProperties(handle, sourceID, listSoundProperties));
    }
    return (E_NON_EXISTENT);
}

am_Error_e CAmRoutingSender::asyncSetSinkSoundProperties(am_Handle_s& handle, const std::vector<am_SoundProperty_s> & listSoundProperties, const am_sinkID_t sinkID)
{
    am_handleData_c handleData;
    SinkInterfaceMap::iterator iter = mMapSinkInterface.begin();
    iter = mMapSinkInterface.find(sinkID);
    if (iter != mMapSinkInterface.end())
    {
        handleData.sinkID = sinkID;
        handleData.soundProperties = new std::vector<am_SoundProperty_s>(listSoundProperties);
        handle = createHandle(handleData, H_SETSINKSOUNDPROPERTIES);
        mMapHandleInterface.insert(std::make_pair(+ handle.handle, iter->second));
        return (iter->second->asyncSetSinkSoundProperties(handle, sinkID, listSoundProperties));
    }
    return (E_NON_EXISTENT);

}

am_Error_e CAmRoutingSender::asyncCrossFade(am_Handle_s& handle, const am_crossfaderID_t crossfaderID, const am_HotSink_e hotSink, const am_RampType_e rampType, const am_time_t time)
{
    am_handleData_c handleData;
    CrossfaderInterfaceMap::iterator iter = mMapCrossfaderInterface.begin();
    iter = mMapCrossfaderInterface.find(crossfaderID);
    if (iter != mMapCrossfaderInterface.end())
    {
        handleData.crossfaderID = crossfaderID;
        handleData.hotSink = hotSink;
        handle = createHandle(handleData, H_CROSSFADE);
        mMapHandleInterface.insert(std::make_pair(+ handle.handle, iter->second));
        return (iter->second->asyncCrossFade(handle, crossfaderID, hotSink, rampType, time));
    }
    return (E_NON_EXISTENT);
}

am_Error_e CAmRoutingSender::setDomainState(const am_domainID_t domainID, const am_DomainState_e domainState)
{
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
        return (E_OK);
    return (E_UNKNOWN);
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
am_Handle_s CAmRoutingSender::createHandle(const am_handleData_c& handleData, const am_Handle_e type)
{
    am_Handle_s handle;
    handle.handle = ++mHandleCount; //todo: handle overflows here...
    handle.handleType = type;
    mlistActiveHandles.insert(std::make_pair(handle, handleData));
    return (handle);
}

/**
 * returns the data that belong to handles
 * @param handle the handle
 * @return a class holding the handle data
 */
CAmRoutingSender::am_handleData_c CAmRoutingSender::returnHandleData(const am_Handle_s handle) const
{
    HandlesMap::const_iterator it = mlistActiveHandles.begin();
    it = mlistActiveHandles.find(handle);
    return (it->second);
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
    version = RoutingSendVersion;
}
}
