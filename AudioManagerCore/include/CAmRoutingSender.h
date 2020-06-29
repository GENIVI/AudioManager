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
 * \file CAmRoutingSender.h
 * For further information see http://www.genivi.org/.
 *
 */

#ifndef ROUTINGSENDER_H_
#define ROUTINGSENDER_H_

#include "IAmRouting.h"
#include <map>
#include <memory>

#ifdef UNIT_TEST // this is needed to test RoutingSender
# include "../test/IAmRoutingBackdoor.h"
#endif

#include "CAmDatabaseHandlerMap.h"

namespace am
{

class CAmRoutingReceiver;

/**
 * Implements the RoutingSendInterface. Loads all plugins and dispatches calls to the plugins
 */
class CAmRoutingSender : public CAmDatabaseHandlerMap::AmDatabaseObserverCallbacks
{
public:
    CAmRoutingSender(const std::vector<std::string> &listOfPluginDirectories, IAmDatabaseHandler *databaseHandler);
    ~CAmRoutingSender();

    am_Error_e removeHandle(const am_Handle_s &handle);
    am_Error_e addDomainLookup(const am_Domain_s &domainData);
    am_Error_e addSourceLookup(const am_Source_s &sourceData);
    am_Error_e addSinkLookup(const am_Sink_s &sinkData);
    am_Error_e addCrossfaderLookup(const am_Crossfader_s &crossfaderData);
    am_Error_e removeDomainLookup(const am_domainID_t domainID);
    am_Error_e removeSourceLookup(const am_sourceID_t sourceID);
    am_Error_e removeSinkLookup(const am_sinkID_t sinkID);
    am_Error_e removeCrossfaderLookup(const am_crossfaderID_t crossfaderID);
    am_Error_e removeConnectionLookup(const am_connectionID_t connectionID);

    am_Error_e startupInterfaces(CAmRoutingReceiver *iRoutingReceiver);
    void setRoutingReady();
    void setRoutingRundown();
    am_Error_e asyncTransferConnection(am_Handle_s &handle, am_domainID_t domainID
        , const std::vector<std::pair<std::string, std::string>>  &route, am_ConnectionState_e state);
    am_Error_e asyncAbort(const am_Handle_s &handle);
    am_Error_e asyncConnect(am_Handle_s &handle, am_connectionID_t &connectionID, const am_sourceID_t sourceID, const am_sinkID_t sinkID, const am_CustomConnectionFormat_t connectionFormat);
    am_Error_e asyncDisconnect(am_Handle_s &handle, const am_connectionID_t connectionID);
    am_Error_e asyncSetSinkVolume(am_Handle_s &handle, const am_sinkID_t sinkID, const am_volume_t volume, const am_CustomRampType_t ramp, const am_time_t time);
    am_Error_e asyncSetSourceVolume(am_Handle_s &handle, const am_sourceID_t sourceID, const am_volume_t volume, const am_CustomRampType_t ramp, const am_time_t time);
    am_Error_e asyncSetSourceState(am_Handle_s &handle, const am_sourceID_t sourceID, const am_SourceState_e state);
    am_Error_e asyncSetSinkSoundProperty(am_Handle_s &handle, const am_sinkID_t sinkID, const am_SoundProperty_s &soundProperty);
    am_Error_e asyncSetSourceSoundProperties(am_Handle_s &handle, const std::vector<am_SoundProperty_s> &listSoundProperties, const am_sourceID_t sourceID);
    am_Error_e asyncSetSinkSoundProperties(am_Handle_s &handle, const std::vector<am_SoundProperty_s> &listSoundProperties, const am_sinkID_t sinkID);
    am_Error_e asyncSetSourceSoundProperty(am_Handle_s &handle, const am_sourceID_t sourceID, const am_SoundProperty_s &soundProperty);
    am_Error_e asyncCrossFade(am_Handle_s &handle, const am_crossfaderID_t crossfaderID, const am_HotSink_e hotSink, const am_CustomRampType_t rampType, const am_time_t time);
    am_Error_e setDomainState(const am_domainID_t domainID, const am_DomainState_e domainState);
    am_Error_e getListHandles(std::vector<am_Handle_s> &listHandles) const;
    am_Error_e getListPlugins(std::vector<std::string> &interfaces) const;
    void getInterfaceVersion(std::string &version) const;
    am_Error_e asyncSetVolumes(am_Handle_s &handle, const std::vector<am_Volumes_s> &listVolumes);
    am_Error_e asyncSetSinkNotificationConfiguration(am_Handle_s &handle, const am_sinkID_t sinkID, const am_NotificationConfiguration_s &notificationConfiguration);
    am_Error_e asyncSetSourceNotificationConfiguration(am_Handle_s &handle, const am_sourceID_t sourceID, const am_NotificationConfiguration_s &notificationConfiguration);
    am_Error_e resyncConnectionState(const am_domainID_t domainID, std::vector<am_Connection_s> &listOfExistingConnections);

    struct InterfaceNamePairs //!< is used to pair interfaces with busnames
    {
        IAmRoutingSend *routingInterface; //!< pointer to the routingInterface
        std::string busName;              //!< the busname
    };

    class handleDataBase
    {
    public:
        handleDataBase(IAmRoutingSend *interface, IAmDatabaseHandler *databaseHandler)
            : mInterface(interface)
            , mpDatabaseHandler(databaseHandler) {}
        virtual ~handleDataBase() {}
        virtual am_Error_e writeDataToDatabase() = 0;   //!< function to write the handle data to the database

        IAmRoutingSend *returnInterface() {return mInterface;}
    private:
        IAmRoutingSend     *mInterface;
    protected:
        IAmDatabaseHandler *mpDatabaseHandler;
    };

    class handleVolumeBase : public handleDataBase
    {
    public:
        handleVolumeBase(IAmRoutingSend *interface, IAmDatabaseHandler *databaseHandler, am_volume_t volume)
            : handleDataBase(interface, databaseHandler)
            , mVolume(volume) {}
        virtual ~handleVolumeBase(){}
        am_volume_t returnVolume() { return mVolume; }
    private:
        am_volume_t mVolume;
    };

    class handleSinkSoundProperty : public handleDataBase
    {
    public:
        handleSinkSoundProperty(IAmRoutingSend *interface, const am_sinkID_t sinkID, const am_SoundProperty_s &soundProperty, IAmDatabaseHandler *databaseHandler)
            : handleDataBase(interface, databaseHandler)
            , mSinkID(sinkID)
            , mSoundProperty(soundProperty) {}
        ~handleSinkSoundProperty() {}
        am_Error_e writeDataToDatabase();

    private:
        am_sinkID_t        mSinkID;
        am_SoundProperty_s mSoundProperty;
    };

    class handleSinkSoundProperties : public handleDataBase
    {
    public:
        handleSinkSoundProperties(IAmRoutingSend *interface, const am_sinkID_t sinkID, const std::vector<am_SoundProperty_s> &listSoundProperties, IAmDatabaseHandler *databaseHandler)
            : handleDataBase(interface, databaseHandler)
            , mSinkID(sinkID)
            , mlistSoundProperties(listSoundProperties) {}
        ~handleSinkSoundProperties() {}
        am_Error_e writeDataToDatabase();

    private:
        am_sinkID_t                     mSinkID;
        std::vector<am_SoundProperty_s> mlistSoundProperties;
    };

    class handleSourceSoundProperty : public handleDataBase
    {
    public:
        handleSourceSoundProperty(IAmRoutingSend *interface, const am_sourceID_t sourceID, const am_SoundProperty_s &soundProperty, IAmDatabaseHandler *databaseHandler)
            : handleDataBase(interface, databaseHandler)
            , mSourceID(sourceID)
            , mSoundProperty(soundProperty) {}
        ~handleSourceSoundProperty() {}
        am_Error_e writeDataToDatabase();

    private:
        am_sourceID_t      mSourceID;
        am_SoundProperty_s mSoundProperty;
    };

    class handleSourceSoundProperties : public handleDataBase
    {
    public:
        handleSourceSoundProperties(IAmRoutingSend *interface, const am_sourceID_t sourceID, const std::vector<am_SoundProperty_s> &listSoundProperties, IAmDatabaseHandler *databaseHandler)
            : handleDataBase(interface, databaseHandler)
            , mSourceID(sourceID)
            , mlistSoundProperties(listSoundProperties) {}
        ~handleSourceSoundProperties(){}
        am_Error_e writeDataToDatabase();

    private:
        am_sourceID_t                   mSourceID;
        std::vector<am_SoundProperty_s> mlistSoundProperties;
    };

    class handleSourceState : public handleDataBase
    {
    public:
        handleSourceState(IAmRoutingSend *interface, const am_sourceID_t sourceID, const am_SourceState_e &state, IAmDatabaseHandler *databaseHandler)
            : handleDataBase(interface, databaseHandler)
            , mSourceID(sourceID)
            , mSourceState(state) {}
        ~handleSourceState() {}
        am_Error_e writeDataToDatabase();

    private:
        am_sourceID_t    mSourceID;
        am_SourceState_e mSourceState;
    };

    class handleSourceVolume : public handleVolumeBase
    {
    public:
        handleSourceVolume(IAmRoutingSend *interface, const am_sourceID_t sourceID, IAmDatabaseHandler *databaseHandler, const am_volume_t &volume)
            : handleVolumeBase(interface, databaseHandler, volume)
            , mSourceID(sourceID) {}
        ~handleSourceVolume() {}
        am_Error_e writeDataToDatabase();

    private:
        am_sourceID_t mSourceID;
    };

    class handleSinkVolume : public handleVolumeBase
    {
    public:
        handleSinkVolume(IAmRoutingSend *interface, const am_sinkID_t sinkID, IAmDatabaseHandler *databaseHandler, const am_volume_t &volume)
            : handleVolumeBase(interface, databaseHandler, volume)
            , mSinkID(sinkID) {}
        ~handleSinkVolume() {}
        am_Error_e writeDataToDatabase();

    private:
        am_sinkID_t mSinkID;
    };

    class handleCrossFader : public handleDataBase
    {
    public:
        handleCrossFader(IAmRoutingSend *interface, const am_crossfaderID_t crossfaderID, const am_HotSink_e &hotSink, IAmDatabaseHandler *databaseHandler)
            : handleDataBase(interface, databaseHandler)
            , mCrossfaderID(crossfaderID)
            , mHotSink(hotSink) {}
        ~handleCrossFader() {}
        am_Error_e writeDataToDatabase();

    private:
        am_crossfaderID_t mCrossfaderID;
        am_HotSink_e      mHotSink;
    };

    class handleConnect : public handleDataBase
    {
    public:
        handleConnect(IAmRoutingSend *interface, const am_connectionID_t connectionID, IAmDatabaseHandler *databaseHandler)
            : handleDataBase(interface, databaseHandler)
            , mConnectionID(connectionID)
            , mConnectionPending(true) {}
        ~handleConnect();
        am_Error_e writeDataToDatabase();

    private:
        am_connectionID_t mConnectionID;
        bool              mConnectionPending;
    };

    class handleDisconnect : public handleDataBase
    {
    public:
        handleDisconnect(IAmRoutingSend *interface, const am_connectionID_t connectionID, IAmDatabaseHandler *databaseHandler, CAmRoutingSender *routingSender)
            : handleDataBase(interface, databaseHandler)
            , mConnectionID(connectionID)
            , mRoutingSender(routingSender){}
        ~handleDisconnect();
        am_Error_e writeDataToDatabase();

    private:
        am_connectionID_t mConnectionID;
        CAmRoutingSender *mRoutingSender;
    };

    class handleTransfer : public handleDataBase
    {
    public:
        handleTransfer(IAmRoutingSend *interface, const std::vector<std::pair<std::string, std::string>> &route
                , am_ConnectionState_e state, IAmDatabaseHandler *databaseHandler)
            : handleDataBase(interface, databaseHandler)
            , mRoute(route)
            , mState(state)
            , mTransferPending(true) {}
        ~handleTransfer()  { };
        am_Error_e writeDataToDatabase() { return E_OK; };

    private:
        const std::vector<std::pair<std::string, std::string>> mRoute;
        am_ConnectionState_e mState;
        bool                 mTransferPending;
    };

    class handleSetVolumes : public handleDataBase
    {
    public:
        handleSetVolumes(IAmRoutingSend *interface, const std::vector<am_Volumes_s> listVolumes, IAmDatabaseHandler *databaseHandler)
            : handleDataBase(interface, databaseHandler)
            , mlistVolumes(listVolumes) {}
        ~handleSetVolumes() {}
        am_Error_e writeDataToDatabase();

    private:
        std::vector<am_Volumes_s> mlistVolumes;
    };

    class handleSetSinkNotificationConfiguration : public handleDataBase
    {
    public:
        handleSetSinkNotificationConfiguration(IAmRoutingSend *interface, const am_sinkID_t sinkID, const am_NotificationConfiguration_s notificationConfiguration, IAmDatabaseHandler *databaseHandler)
            : handleDataBase(interface, databaseHandler)
            , mSinkID(sinkID)
            , mNotificationConfiguration(notificationConfiguration){}
        ~handleSetSinkNotificationConfiguration() {}
        am_Error_e writeDataToDatabase();

    private:
        am_sinkID_t                    mSinkID;
        am_NotificationConfiguration_s mNotificationConfiguration;
    };

    class handleSetSourceNotificationConfiguration : public handleDataBase
    {
    public:
        handleSetSourceNotificationConfiguration(IAmRoutingSend *interface, const am_sourceID_t sourceID, const am_NotificationConfiguration_s notificationConfiguration, IAmDatabaseHandler *databaseHandler)
            : handleDataBase(interface, databaseHandler)
            , mSourceID(sourceID)
            , mNotificationConfiguration(notificationConfiguration) {}
        ~handleSetSourceNotificationConfiguration() {}
        am_Error_e writeDataToDatabase();

    private:
        am_sourceID_t                  mSourceID;
        am_NotificationConfiguration_s mNotificationConfiguration;
    };

    am_Error_e writeToDatabaseAndRemove(const am_Handle_s handle); //!< write data to Database and remove handle
    void checkVolume(const am_Handle_s handle, const am_volume_t volume);
    bool handleExists(const am_Handle_s handle); //!< returns true if the handle exists

#ifdef UNIT_TEST // this is needed to test RoutingSender
    friend class IAmRoutingBackdoor;
#endif

private:
    struct comparator //!< is needed to sort the handles in the map
    {
        bool operator()(const am_Handle_s &a, const am_Handle_s &b) const
        {
            return (a.handle < b.handle || (a.handle == b.handle && a.handleType < b.handleType));
        }

    };

    void loadPlugins(const std::vector<std::string> &listOfPluginDirectories);
    am_Handle_s createHandle(std::shared_ptr<handleDataBase> handleData, const am_Handle_e type); //!< creates a handle
    void unloadLibraries(void);                                                                   //!< unloads all loaded plugins

    typedef std::map<am_domainID_t, IAmRoutingSend *>                          DomainInterfaceMap;     //!< maps domains to interfaces
    typedef std::map<am_sinkID_t, IAmRoutingSend *>                            SinkInterfaceMap;       //!< maps sinks to interfaces
    typedef std::map<am_sourceID_t, IAmRoutingSend *>                          SourceInterfaceMap;     //!< maps sources to interfaces
    typedef std::map<am_crossfaderID_t, IAmRoutingSend *>                      CrossfaderInterfaceMap; //!< maps crossfaders to interfaces
    typedef std::map<am_connectionID_t, IAmRoutingSend *>                      ConnectionInterfaceMap; //!< maps connections to interfaces
    typedef std::map<am_Handle_s, std::shared_ptr<handleDataBase>, comparator> HandlesMap;             //!< maps handleData to handles

    int16_t                         mHandleCount;            //!< is used to create handles
    HandlesMap                      mlistActiveHandles;      //!< list of all currently "running" handles.
    std::vector<void *>             mListLibraryHandles;     //!< list of all loaded pluginInterfaces
    std::vector<InterfaceNamePairs> mListInterfaces;         //!< list of busname/interface relation
    CrossfaderInterfaceMap          mMapCrossfaderInterface; //!< map of crossfaders to interface
    ConnectionInterfaceMap          mMapConnectionInterface; //!< map of connection to interfaces
    DomainInterfaceMap              mMapDomainInterface;     //!< map of domains to interfaces
    SinkInterfaceMap                mMapSinkInterface;       //!< map of sinks to interfaces
    SourceInterfaceMap              mMapSourceInterface;     //!< map of sources to interfaces
    CAmRoutingReceiver             *mpRoutingReceiver;       //!< pointer to routing receiver
    IAmDatabaseHandler             *mpDatabaseHandler;       //!< pointer to the databaseHandler
};

}

#endif /* ROUTINGSENDER_H_ */
