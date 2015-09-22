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
 * \file CAmCommandReceiver.h
 * For further information see http://www.genivi.org/.
 *
 */

#ifndef COMMANDRECEIVER_H_
#define COMMANDRECEIVER_H_

#include "IAmCommand.h"

namespace am
{

class IAmDatabaseHandler;
class CAmControlSender;
class CAmDbusWrapper;
class CAmSocketHandler;

/**
 * This class realizes the command Interface
 */
class CAmCommandReceiver: public IAmCommandReceive
{
public:
    CAmCommandReceiver(IAmDatabaseHandler* iDatabaseHandler, CAmControlSender* iControlSender, CAmSocketHandler* iSocketHandler);
    CAmCommandReceiver(IAmDatabaseHandler* iDatabaseHandler, CAmControlSender* iControlSender, CAmSocketHandler* iSocketHandler, CAmDbusWrapper* iDBusWrapper);
    ~CAmCommandReceiver();
    am_Error_e connect(const am_sourceID_t sourceID, const am_sinkID_t sinkID, am_mainConnectionID_t& mainConnectionID);
    am_Error_e disconnect(const am_mainConnectionID_t mainConnectionID);
    am_Error_e setVolume(const am_sinkID_t sinkID, const am_mainVolume_t volume);
    am_Error_e volumeStep(const am_sinkID_t sinkID, const int16_t volumeStep);
    am_Error_e setSinkMuteState(const am_sinkID_t sinkID, const am_MuteState_e muteState);
    am_Error_e setMainSinkSoundProperty(const am_MainSoundProperty_s& soundProperty, const am_sinkID_t sinkID);
    am_Error_e setMainSourceSoundProperty(const am_MainSoundProperty_s& soundProperty, const am_sourceID_t sourceID);
    am_Error_e setSystemProperty(const am_SystemProperty_s& property);
    am_Error_e getVolume(const am_sinkID_t sinkID, am_mainVolume_t& mainVolume) const;
    am_Error_e getListMainConnections(std::vector<am_MainConnectionType_s>& listConnections) const;
    am_Error_e getListMainSinks(std::vector<am_SinkType_s>& listMainSinks) const;
    am_Error_e getListMainSources(std::vector<am_SourceType_s>& listMainSources) const;
    am_Error_e getListMainSinkSoundProperties(const am_sinkID_t sinkID, std::vector<am_MainSoundProperty_s>& listSoundProperties) const;
    am_Error_e getListMainSourceSoundProperties(const am_sourceID_t sourceID, std::vector<am_MainSoundProperty_s>& listSourceProperties) const;
    am_Error_e getListSourceClasses(std::vector<am_SourceClass_s>& listSourceClasses) const;
    am_Error_e getListSinkClasses(std::vector<am_SinkClass_s>& listSinkClasses) const;
    am_Error_e getListSystemProperties(std::vector<am_SystemProperty_s>& listSystemProperties) const;
    am_Error_e getTimingInformation(const am_mainConnectionID_t mainConnectionID, am_timeSync_t& delay) const;
    am_Error_e getDBusConnectionWrapper(CAmDbusWrapper*& dbusConnectionWrapper) const;
    am_Error_e getSocketHandler(CAmSocketHandler*& socketHandler) const;
    void confirmCommandReady(const uint16_t handle, const am_Error_e error);
    void confirmCommandRundown(const uint16_t handle, const am_Error_e error);
    void getInterfaceVersion(std::string& version) const;
    am_Error_e getListMainSinkNotificationConfigurations(const am_sinkID_t sinkID, std::vector<am_NotificationConfiguration_s>& listMainNotificationConfigurations) const ;
    am_Error_e getListMainSourceNotificationConfigurations(const am_sourceID_t sourceID, std::vector<am_NotificationConfiguration_s>& listMainNotificationConfigurations) const ;
    am_Error_e setMainSinkNotificationConfiguration(const am_sinkID_t sinkID, const am_NotificationConfiguration_s& mainNotificationConfiguration) ;
    am_Error_e setMainSourceNotificationConfiguration(const am_sourceID_t sourceID, const am_NotificationConfiguration_s& mainNotificationConfiguration) ;

    uint16_t getStartupHandle(); //!< returns a startup handle
    uint16_t getRundownHandle(); //!< returns a rundown handle

    void waitOnStartup(bool startup); //!< tells the ComandReceiver to start waiting for all handles to be confirmed
    void waitOnRundown(bool rundown); //!< tells the ComandReceiver to start waiting for all handles to be confirmed

private:
    IAmDatabaseHandler* mDatabaseHandler; //!< pointer to the databasehandler
    CAmControlSender* mControlSender; //!< pointer to the control sender
    CAmDbusWrapper* mDBusWrapper; //!< pointer to the dbuswrapper
    CAmSocketHandler* mSocketHandler; //!< pointer to the SocketHandler

    uint16_t handleCount; //!< counts all handles
    std::vector<uint16_t> mListStartupHandles; //!< list of handles that wait for a confirm
    std::vector<uint16_t> mListRundownHandles; //!< list of handles that wait for a confirm
    bool mWaitStartup; //!< if true confirmation will be sent if list of handles = 0
    bool mWaitRundown; //!< if true confirmation will be sent if list of handles = 0
    am_Error_e mLastErrorStartup;
    am_Error_e mLastErrorRundown;
};

}

#endif /* COMMANDRECEIVER_H_ */
