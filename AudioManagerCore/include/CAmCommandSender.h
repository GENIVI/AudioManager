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
 * \file CAmCommandSender.h
 * For further information see http://www.genivi.org/.
 *
 */

#ifndef COMMANDSENDER_H_
#define COMMANDSENDER_H_

#ifdef UNIT_TEST
#include "../test/IAmCommandBackdoor.h" //we need this for the unit test
#endif

#include "IAmCommand.h"
#include "CAmDatabaseHandlerMap.h"
#include "CAmSerializer.h"

namespace am
{

class CAmCommandReceiver;
class CAmCommandSender;


/**
 * This class is used to send data to the CommandInterface.
 * All loaded plugins will be called when a callback is invoked.
 */
class CAmCommandSender: public CAmDatabaseHandlerMap::AmDatabaseObserverCallbacks
{
public:
    CAmCommandSender(const std::vector<std::string>& listOfPluginDirectories, CAmSocketHandler *iSocketHandler);
    ~CAmCommandSender();
    am_Error_e startupInterfaces(CAmCommandReceiver* iCommandReceiver);
    void setCommandReady();
    void setCommandRundown();
    void cbNewMainConnection(const am_MainConnectionType_s mainConnection);
    void cbRemovedMainConnection(const am_mainConnectionID_t mainConnection);
    void cbNewSink(am_SinkType_s sink);
    void cbRemovedSink(const am_sinkID_t sink);
    void cbNewSource(const am_SourceType_s source);
    void cbRemovedSource(const am_sourceID_t source);
    void cbNumberOfSinkClassesChanged();
    void cbNumberOfSourceClassesChanged();
    void cbMainConnectionStateChanged(const am_mainConnectionID_t connectionID, const am_ConnectionState_e connectionState);
    void cbMainSinkSoundPropertyChanged(const am_sinkID_t sinkID, const am_MainSoundProperty_s& soundProperty);
    void cbMainSourceSoundPropertyChanged(const am_sourceID_t sourceID, const am_MainSoundProperty_s& soundProperty);
    void cbSinkAvailabilityChanged(const am_sinkID_t sinkID, const am_Availability_s& availability);
    void cbSourceAvailabilityChanged(const am_sourceID_t sourceID, const am_Availability_s& availability);
    void cbVolumeChanged(const am_sinkID_t sinkID, const am_mainVolume_t volume);
    void cbSinkMuteStateChanged(const am_sinkID_t sinkID, const am_MuteState_e muteState);
    void cbSystemPropertyChanged(const am_SystemProperty_s& systemProperty);
    void cbTimingInformationChanged(const am_mainConnectionID_t mainConnectionID, const am_timeSync_t time);
    void getInterfaceVersion(std::string& version) const;
    am_Error_e getListPlugins(std::vector<std::string>& interfaces) const;
    void cbSinkUpdated(const am_sinkID_t sinkID, const am_sinkClass_t sinkClassID, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties);
    void cbSourceUpdated(const am_sourceID_t sourceID, const am_sourceClass_t sourceClassID, const std::vector<am_MainSoundProperty_s>& listMainSoundProperties);
    void cbSinkNotification(const am_sinkID_t sinkID, const am_NotificationPayload_s& notification);
    void cbSourceNotification(const am_sourceID_t sourceID, const am_NotificationPayload_s& notification);
    void cbSinkMainNotificationConfigurationChanged(const am_sinkID_t sinkID, const am_NotificationConfiguration_s& mainNotificationConfiguration);
    void cbSourceMainNotificationConfigurationChanged(const am_sourceID_t sourceID, const am_NotificationConfiguration_s& mainNotificationConfiguration);

#ifdef UNIT_TEST
    friend class IAmCommandBackdoor; //this is to get access to the loaded plugins and be able to exchange the interfaces
#endif
private:
    void loadPlugins(const std::vector<std::string>& listOfPluginDirectories);
    void unloadLibraries(void); //!< unload the shared libraries
    std::vector<IAmCommandSend*> mListInterfaces; //!< list of all interfaces
    std::vector<void*> mListLibraryHandles; //!< list of all library handles. This information is used to unload the plugins correctly.
    std::vector<std::string> mListLibraryNames; //!< list of all library names. This information is used for getListPlugins.
    
    CAmCommandReceiver *mCommandReceiver;
    V2::CAmSerializer mSerializer;
};



}

#endif /* COMMANDSENDER_H_ */
