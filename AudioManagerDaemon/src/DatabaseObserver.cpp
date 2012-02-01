/**
 * Copyright (C) 2011, BMW AG
 *
 * GeniviAudioMananger AudioManagerDaemon
 *
 * \file DatabaseObserver.cpp
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

#include "DatabaseObserver.h"
#include "CommandSender.h"
#include "RoutingSender.h"
#include "TelnetServer.h"
#include <cassert>

using namespace am;

DatabaseObserver::DatabaseObserver(CommandSender *iCommandSender, RoutingSender *iRoutingSender) :
        mCommandSender(iCommandSender), //
        mRoutingSender(iRoutingSender)
{
    assert(mCommandSender!=0);
    assert(mRoutingSender!=0);
}

DatabaseObserver::DatabaseObserver(CommandSender *iCommandSender, RoutingSender *iRoutingSender, TelnetServer *iTelnetServer) :
        mCommandSender(iCommandSender), //
        mRoutingSender(iRoutingSender), //
        mTelnetServer(iTelnetServer)
{
    assert(mCommandSender!=0);
    assert(mRoutingSender!=0);
    assert(mTelnetServer!=0);
}

DatabaseObserver::~DatabaseObserver()
{
}

void DatabaseObserver::newSink(am_Sink_s sink)
{
    mRoutingSender->addSinkLookup(sink);
    mCommandSender->cbNumberOfSinksChanged();
}

void DatabaseObserver::newSource(am_Source_s source)
{
    mRoutingSender->addSourceLookup(source);
    mCommandSender->cbNumberOfSourcesChanged();
}

void DatabaseObserver::newDomain(am_Domain_s domain)
{
    mRoutingSender->addDomainLookup(domain);
}

void DatabaseObserver::newGateway(am_Gateway_s gateway)
{
    (void) gateway;
    //todo: implement something
}

void DatabaseObserver::newCrossfader(am_Crossfader_s crossfader)
{
    mRoutingSender->addCrossfaderLookup(crossfader);
}

void DatabaseObserver::removedSink(am_sinkID_t sinkID)
{
    mRoutingSender->removeSinkLookup(sinkID);
    mCommandSender->cbNumberOfSinksChanged();
}

void DatabaseObserver::removedSource(am_sourceID_t sourceID)
{
    mRoutingSender->removeSourceLookup(sourceID);
    mCommandSender->cbNumberOfSourcesChanged();
}

void DatabaseObserver::removeDomain(am_domainID_t domainID)
{
    mRoutingSender->removeDomainLookup(domainID);
}

void DatabaseObserver::removeGateway(am_gatewayID_t gatewayID)
{
    (void) gatewayID;
    //todo: implement something
}

void DatabaseObserver::removeCrossfader(am_crossfaderID_t crossfaderID)
{
    mRoutingSender->removeCrossfaderLookup(crossfaderID);
}

void DatabaseObserver::numberOfMainConnectionsChanged()
{
    mCommandSender->cbNumberOfMainConnectionsChanged();
}

void DatabaseObserver::numberOfSinkClassesChanged()
{
    mCommandSender->cbNumberOfSinkClassesChanged();
}

void DatabaseObserver::numberOfSourceClassesChanged()
{
    mCommandSender->cbNumberOfSourceClassesChanged();
}

void DatabaseObserver::mainConnectionStateChanged(const am_mainConnectionID_t connectionID, const am_ConnectionState_e connectionState)
{
    mCommandSender->cbMainConnectionStateChanged(connectionID, connectionState);
}

void DatabaseObserver::mainSinkSoundPropertyChanged(const am_sinkID_t sinkID, const am_MainSoundProperty_s SoundProperty)
{
    mCommandSender->cbMainSinkSoundPropertyChanged(sinkID, SoundProperty);
}

void DatabaseObserver::mainSourceSoundPropertyChanged(const am_sourceID_t sourceID, const am_MainSoundProperty_s & SoundProperty)
{
    mCommandSender->cbMainSourceSoundPropertyChanged(sourceID, SoundProperty);
}

void DatabaseObserver::sinkAvailabilityChanged(const am_sinkID_t sinkID, const am_Availability_s & availability)
{
    mCommandSender->cbSinkAvailabilityChanged(sinkID, availability);
}

void DatabaseObserver::sourceAvailabilityChanged(const am_sourceID_t sourceID, const am_Availability_s & availability)
{
    mCommandSender->cbSourceAvailabilityChanged(sourceID, availability);
}

void DatabaseObserver::volumeChanged(const am_sinkID_t sinkID, const am_mainVolume_t volume)
{
    mCommandSender->cbVolumeChanged(sinkID, volume);
}

void DatabaseObserver::sinkMuteStateChanged(const am_sinkID_t sinkID, const am_MuteState_e muteState)
{
    mCommandSender->cbSinkMuteStateChanged(sinkID, muteState);
}

void DatabaseObserver::systemPropertyChanged(const am_SystemProperty_s & SystemProperty)
{
    mCommandSender->cbSystemPropertyChanged(SystemProperty);
}

void DatabaseObserver::timingInformationChanged(const am_mainConnectionID_t mainConnection, const am_timeSync_t time)
{
    mCommandSender->cbTimingInformationChanged(mainConnection, time);
    //todo:inform the controller via controlsender
}

