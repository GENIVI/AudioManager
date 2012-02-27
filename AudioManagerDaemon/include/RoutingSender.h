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

#ifndef ROUTINGSENDER_H_
#define ROUTINGSENDER_H_

#include "routing/RoutingSendInterface.h"
#include <map>

#ifdef UNIT_TEST //this is needed to test RoutingSender
#include "../test/RoutingInterfaceBackdoor.h"
#endif

namespace am
{

class RoutingReceiver;

/**
 * Implements the RoutingSendInterface. Loads all plugins and dispatches calls to the plugins
 */
class RoutingSender
{
public:
    RoutingSender(const std::vector<std::string>& listOfPluginDirectories);
    virtual ~RoutingSender();

    /**
     * removes a handle from the list
     * @param handle to be removed
     * @return E_OK in case of success
     */
    am_Error_e removeHandle(const am_Handle_s& handle);

    /**
     * @author Christian
     * this adds the domain to the lookup table of the Router. The data is used to have a quick lookup of the correct pluginInterface.
     * This must be done whenever a domain is registered.
     */
    am_Error_e addDomainLookup(const am_Domain_s& domainData);
    /**
     * @author Christian
     * this adds the Source to the lookup table of the Router. The data is used to have a quick lookup of the correct pluginInterface.
     * This must be done whenever a Source is registered.
     */
    am_Error_e addSourceLookup(const am_Source_s& sourceData);
    /**
     * @author Christian
     * this adds the Sink to the lookup table of the Router. The data is used to have a quick lookup of the correct pluginInterface.
     * This must be done whenever a Sink is registered.
     */
    am_Error_e addSinkLookup(const am_Sink_s& sinkData);
    /**
     * @author Christian
     * this adds the Crossfader to the lookup table of the Router. The data is used to have a quick lookup of the correct pluginInterface.
     * This must be done whenever a Crossfader is registered.
     */
    am_Error_e addCrossfaderLookup(const am_Crossfader_s& crossfaderData);
    /**
     * @author Christian
     * this removes the Domain to the lookup table of the Router. This must be done everytime a domain is deregistered.
     */
    am_Error_e removeDomainLookup(const am_domainID_t domainID);
    /**
     * @author Christian
     * this removes the Source to the lookup table of the Router. This must be done everytime a source is deregistered.
     */
    am_Error_e removeSourceLookup(const am_sourceID_t sourceID);
    /**
     * @author Christian
     * this removes the Sink to the lookup table of the Router. This must be done everytime a sink is deregistered.
     */
    am_Error_e removeSinkLookup(const am_sinkID_t sinkID);
    /**
     * @author Christian
     * this removes the Crossfader to the lookup table of the Router. This must be done everytime a crossfader is deregistered.
     */
    am_Error_e removeCrossfaderLookup(const am_crossfaderID_t crossfaderID);
    am_Error_e startupInterfaces(RoutingReceiver* iRoutingReceiver);
    void setRoutingReady();
    void setRoutingRundown();
    am_Error_e asyncAbort(const am_Handle_s& handle);
    am_Error_e asyncConnect(am_Handle_s& handle, const am_connectionID_t connectionID, const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_ConnectionFormat_e connectionFormat);
    am_Error_e asyncDisconnect(am_Handle_s& handle, const am_connectionID_t connectionID);
    am_Error_e asyncSetSinkVolume(am_Handle_s& handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time);
    am_Error_e asyncSetSourceVolume(am_Handle_s& handle, const am_sourceID_t sourceID, const am_volume_t volume, const am_RampType_e ramp, const am_time_t time);
    am_Error_e asyncSetSourceState(am_Handle_s& handle, const am_sourceID_t sourceID, const am_SourceState_e state);
    am_Error_e asyncSetSinkSoundProperty(am_Handle_s& handle, const am_sinkID_t sinkID, const am_SoundProperty_s& soundProperty);
    am_Error_e asyncSetSourceSoundProperties(am_Handle_s& handle, const std::vector<am_SoundProperty_s>& listSoundProperties, const am_sourceID_t sourceID);
    am_Error_e asyncSetSinkSoundProperties(am_Handle_s& handle, const std::vector<am_SoundProperty_s>& listSoundProperties, const am_sinkID_t sinkID);
    am_Error_e asyncSetSourceSoundProperty(am_Handle_s& handle, const am_sourceID_t sourceID, const am_SoundProperty_s& soundProperty);
    am_Error_e asyncCrossFade(am_Handle_s& handle, const am_crossfaderID_t crossfaderID, const am_HotSink_e hotSink, const am_RampType_e rampType, const am_time_t time);
    am_Error_e setDomainState(const am_domainID_t domainID, const am_DomainState_e domainState);
    am_Error_e getListHandles(std::vector<am_Handle_s> & listHandles) const;
    am_Error_e getListPlugins(std::vector<std::string>& interfaces) const;
    void getInterfaceVersion(std::string& version) const;

    //!< is used to pair interfaces with busnames
    struct InterfaceNamePairs
    {
        RoutingSendInterface* routingInterface;
        std::string busName;
    };

    //!< is used to store data related to handles
    class am_handleData_c
    {
    public:
        union
        {
            am_sinkID_t sinkID;
            am_sourceID_t sourceID;
            am_crossfaderID_t crossfaderID;
            am_connectionID_t connectionID;
        };
        union
        {
            am_SoundProperty_s soundPropery;
            am_SourceState_e sourceState;
            am_volume_t volume;
            am_HotSink_e hotSink;
            std::vector<am_SoundProperty_s>* soundProperties;
        };

    };

#ifdef UNIT_TEST //this is needed to test RoutingSender
    friend class RoutingInterfaceBackdoor;
#endif

    /**
     * returns the data that belong to handles
     * @param handle the handle
     * @return a class holding the handle data
     */
    am_handleData_c returnHandleData(const am_Handle_s handle) const;

private:

    //!< is needed to sort the handles in the map
    struct comparator
    {
        bool operator()(const am_Handle_s& a, const am_Handle_s& b) const
        {
            return (a.handle < b.handle);
        }
    };

    /**
     * creates a handle and adds it to the list of handles
     * @param handleData the data that should be saves together with the handle
     * @param type the type of handle to be created
     * @return the handle
     */
    am_Handle_s createHandle(const am_handleData_c& handleData, const am_Handle_e type);
    void unloadLibraries(void); //!< unloads all loaded plugins

    typedef std::map<am_domainID_t, RoutingSendInterface*> DomainInterfaceMap; //!< maps domains to interfaces
    typedef std::map<am_sinkID_t, RoutingSendInterface*> SinkInterfaceMap; //!< maps sinks to interfaces
    typedef std::map<am_sourceID_t, RoutingSendInterface*> SourceInterfaceMap; //!< maps sources to interfaces
    typedef std::map<am_crossfaderID_t, RoutingSendInterface*> CrossfaderInterfaceMap; //!< maps crossfaders to interfaces
    typedef std::map<am_connectionID_t, RoutingSendInterface*> ConnectionInterfaceMap; //!< maps connections to interfaces
    typedef std::map<uint16_t, RoutingSendInterface*> HandleInterfaceMap; //!< maps handles to interfaces
    typedef std::map<am_Handle_s, am_handleData_c, comparator> HandlesMap; //!< maps handleData to handles

    int16_t mHandleCount; //!< is used to create handles
    HandlesMap mlistActiveHandles; //!< list of all currently "running" handles.
    std::vector<void*> mListLibraryHandles; //!< list of all loaded pluginInterfaces
    std::vector<InterfaceNamePairs> mListInterfaces; //!< list of busname/interface relation
    ConnectionInterfaceMap mMapConnectionInterface; //!< map of connection to interfaces
    CrossfaderInterfaceMap mMapCrossfaderInterface; //!< map of crossfaders to interface
    DomainInterfaceMap mMapDomainInterface; //!< map of domains to interfaces
    SinkInterfaceMap mMapSinkInterface; //!< map of sinks to interfaces
    SourceInterfaceMap mMapSourceInterface; //!< map of sources to interfaces
    HandleInterfaceMap mMapHandleInterface; //!< map of handles to interfaces
    RoutingReceiver *mRoutingReceiver;
};

}

#endif /* ROUTINGSENDER_H_ */
