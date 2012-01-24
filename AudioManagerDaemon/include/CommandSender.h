/**
 * Copyright (C) 2011, BMW AG
 *
 * GeniviAudioMananger AudioManagerDaemon
 *
 * \file CommandSender.h
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

#ifndef COMMANDSENDER_H_
#define COMMANDSENDER_H_

#ifdef UNIT_TEST
#include "../test/CommandInterfaceBackdoor.h" //we need this for the unit test
#endif

#include "command/CommandSendInterface.h"

namespace am
{

/**
 * This class is used to send data to the CommandInterface.
 * All loaded plugins will be called when a callback is invoked.
 */
class CommandSender
{
public:
    CommandSender(const std::vector<std::string>& listOfPluginDirectories);
    virtual ~CommandSender();
    am_Error_e startupInterface(CommandReceiveInterface* commandreceiveinterface);
    am_Error_e stopInterface();
    void cbCommunicationReady();
    void cbCommunicationRundown();
    void cbNumberOfMainConnectionsChanged();
    void cbNumberOfSinksChanged();
    void cbNumberOfSourcesChanged();
    void cbNumberOfSinkClassesChanged();
    void cbNumberOfSourceClassesChanged();
    void cbMainConnectionStateChanged(const am_mainConnectionID_t connectionID, const am_ConnectionState_e connectionState);
    void cbMainSinkSoundPropertyChanged(const am_sinkID_t sinkID, const am_MainSoundProperty_s SoundProperty);
    void cbMainSourceSoundPropertyChanged(const am_sourceID_t sourceID, const am_MainSoundProperty_s& SoundProperty);
    void cbSinkAvailabilityChanged(const am_sinkID_t sinkID, const am_Availability_s& availability);
    void cbSourceAvailabilityChanged(const am_sourceID_t sourceID, const am_Availability_s& availability);
    void cbVolumeChanged(const am_sinkID_t sinkID, const am_mainVolume_t volume);
    void cbSinkMuteStateChanged(const am_sinkID_t sinkID, const am_MuteState_e muteState);
    void cbSystemPropertyChanged(const am_SystemProperty_s& SystemProperty);
    void cbTimingInformationChanged(const am_mainConnectionID_t mainConnection, const am_timeSync_t time);
    uint16_t getInterfaceVersion() const;
#ifdef UNIT_TEST
    friend class CommandInterfaceBackdoor; //this is to get access to the loaded plugins and be able to exchange the interfaces
#endif
private:
    void unloadLibraries(void); //!< unload the shared libraries
    std::vector<CommandSendInterface*> mListInterfaces; //!< list of all interfaces
    std::vector<void*> mListLibraryHandles; //!< list of all library handles. This information is used to unload the plugins correctly.
};

}

#endif /* COMMANDSENDER_H_ */
