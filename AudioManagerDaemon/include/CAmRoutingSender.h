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
 * \file CAmRoutingSender.h
 * For further information see http://www.genivi.org/.
 *
 */

#ifndef ROUTINGSENDER_H_
#define ROUTINGSENDER_H_

#include "routing/IAmRoutingSend.h"
#include <map>

#ifdef UNIT_TEST //this is needed to test RoutingSender
#include "../test/IAmRoutingBackdoor.h"
#endif

namespace am
{

class CAmRoutingReceiver;

/**
 * Implements the RoutingSendInterface. Loads all plugins and dispatches calls to the plugins
 */
class CAmRoutingSender
{
public:
    CAmRoutingSender(const std::vector<std::string>& listOfPluginDirectories);
    ~CAmRoutingSender();

    am_Error_e removeHandle(const am_Handle_s& handle);
    am_Error_e addDomainLookup(const am_Domain_s& domainData);
    am_Error_e addSourceLookup(const am_Source_s& sourceData);
    am_Error_e addSinkLookup(const am_Sink_s& sinkData);
    am_Error_e addCrossfaderLookup(const am_Crossfader_s& crossfaderData);
    am_Error_e removeDomainLookup(const am_domainID_t domainID);
    am_Error_e removeSourceLookup(const am_sourceID_t sourceID);
    am_Error_e removeSinkLookup(const am_sinkID_t sinkID);
    am_Error_e removeCrossfaderLookup(const am_crossfaderID_t crossfaderID);

    am_Error_e startupInterfaces(CAmRoutingReceiver* iRoutingReceiver);
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

    struct InterfaceNamePairs //!< is used to pair interfaces with busnames
    {
        IAmRoutingSend* routingInterface; //!< pointer to the routingInterface
        std::string busName; //!< the busname
    };

    class am_handleData_c //!< is used to store data related to handles
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

    am_handleData_c returnHandleData(const am_Handle_s handle) const; //!< returns the handle data associated with a handle

#ifdef UNIT_TEST //this is needed to test RoutingSender
    friend class IAmRoutingBackdoor;
#endif

private:
    struct comparator //!< is needed to sort the handles in the map
    {
        bool operator()(const am_Handle_s& a, const am_Handle_s& b) const
        {
            return (a.handle < b.handle);
        }
    };

    am_Handle_s createHandle(const am_handleData_c& handleData, const am_Handle_e type); //!< creates a handle
    void unloadLibraries(void); //!< unloads all loaded plugins

    typedef std::map<am_domainID_t, IAmRoutingSend*> DomainInterfaceMap; //!< maps domains to interfaces
    typedef std::map<am_sinkID_t, IAmRoutingSend*> SinkInterfaceMap; //!< maps sinks to interfaces
    typedef std::map<am_sourceID_t, IAmRoutingSend*> SourceInterfaceMap; //!< maps sources to interfaces
    typedef std::map<am_crossfaderID_t, IAmRoutingSend*> CrossfaderInterfaceMap; //!< maps crossfaders to interfaces
    typedef std::map<am_connectionID_t, IAmRoutingSend*> ConnectionInterfaceMap; //!< maps connections to interfaces
    typedef std::map<uint16_t, IAmRoutingSend*> HandleInterfaceMap; //!< maps handles to interfaces
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
    CAmRoutingReceiver *mpRoutingReceiver; //!< pointer to routing receiver
};

}

#endif /* ROUTINGSENDER_H_ */
