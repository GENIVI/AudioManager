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
 * Linking AudioManager statiasyncCally or dynamiasyncCally with other modules is making a combined work based on AudioManager. You may license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to license your linked modules under the GNU Lesser General Public License, version 2.1, you may use the program under the following exception.
 * As a special exception, the copyright holders of AudioManager give you permission to combine AudioManager with software programs or libraries that are released under any license unless such a combination is not permitted by the license of such a software program or library. You may copy and distribute such a system following the terms of the GNU Lesser General Public License, version 2.1, including this special exception, for AudioManager and the licenses of the other code concerned.
 * Note that people who make modified versions of AudioManager are not obligated to grant this special exception for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, version 2.1, gives permission to release a modified version without this exception; this exception also makes it possible to release a modified version which carries forward this exception.
 *
 */

#include "DatabaseObserver.h"
#include <string.h>
#include <cassert>
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include "CommandSender.h"
#include "RoutingSender.h"
#include "TelnetServer.h"
#include "DLTWrapper.h"
//#include "CAmSerializer.h"

using namespace am;

DatabaseObserver::DatabaseObserver(CommandSender *iCommandSender, RoutingSender *iRoutingSender, SocketHandler *iSocketHandler) :
        mCommandSender(iCommandSender), //
        mRoutingSender(iRoutingSender), //
        mSerializer(iSocketHandler) //
{
    assert(mCommandSender!=0);
    assert(mRoutingSender!=0);
    assert(iSocketHandler!=0);
}

DatabaseObserver::DatabaseObserver(CommandSender *iCommandSender, RoutingSender *iRoutingSender, SocketHandler *iSocketHandler, TelnetServer *iTelnetServer) :
        mCommandSender(iCommandSender), //
        mRoutingSender(iRoutingSender), //
        mTelnetServer(iTelnetServer), //
        mSerializer(iSocketHandler) //
{
    assert(mTelnetServer!=0);
    assert(mCommandSender!=0);
    assert(mRoutingSender!=0);
    assert(iSocketHandler!=0);
}

DatabaseObserver::~DatabaseObserver()
{
}

void DatabaseObserver::newMainConnection(const am_MainConnectionType_s& mainConnection)
{
    mSerializer.asyncCall<CommandSender, const am_MainConnectionType_s>(mCommandSender, &CommandSender::cbNewMainConnection, mainConnection);
}

void DatabaseObserver::removedMainConnection(const am_mainConnectionID_t mainConnection)
{
    mSerializer.asyncCall<CommandSender, const am_mainConnectionID_t>(mCommandSender, &CommandSender::cbRemovedMainConnection, mainConnection);
}

void DatabaseObserver::newSink(const am_Sink_s& sink)
{
    mRoutingSender->addSinkLookup(sink);
    if (sink.visible)
    {
        am_SinkType_s s;
        s.availability = sink.available;
        s.muteState = sink.muteState;
        s.name = sink.name;
        s.sinkClassID = sink.sinkClassID;
        s.sinkID = sink.sinkID;
        s.volume = sink.mainVolume;
        mSerializer.asyncCall<CommandSender, const am_SinkType_s>(mCommandSender, &CommandSender::cbNewSink, s);
    }
}

void DatabaseObserver::newSource(const am_Source_s& source)
{
    mRoutingSender->addSourceLookup(source);
    if (source.visible)
    {
        am_SourceType_s s;
        s.availability = source.available;
        s.name = source.name;
        s.sourceClassID = source.sourceClassID;
        s.sourceID = source.sourceID;
        mSerializer.asyncCall<CommandSender, const am_SourceType_s>(mCommandSender, &CommandSender::cbNewSource, s);
    }
}

void DatabaseObserver::newDomain(const am_Domain_s& domain)
{
    mRoutingSender->addDomainLookup(domain);
}

void DatabaseObserver::newGateway(const am_Gateway_s& gateway)
{
    (void) gateway;
    //todo: implement something
}

void DatabaseObserver::newCrossfader(const am_Crossfader_s& crossfader)
{
    mRoutingSender->addCrossfaderLookup(crossfader);
}

void DatabaseObserver::removedSink(const am_sinkID_t sinkID, const bool visible)
{
    mRoutingSender->removeSinkLookup(sinkID);

    if (visible)
        mSerializer.asyncCall<CommandSender, const am_sinkID_t>(mCommandSender, &CommandSender::cbRemovedSink, sinkID);
}

void DatabaseObserver::removedSource(const am_sourceID_t sourceID, const bool visible)
{
    mRoutingSender->removeSourceLookup(sourceID);

    if (visible)
        mSerializer.asyncCall<CommandSender, const am_sourceID_t>(mCommandSender, &CommandSender::cbRemovedSource, sourceID);
}

void DatabaseObserver::removeDomain(const am_domainID_t domainID)
{
    mRoutingSender->removeDomainLookup(domainID);
}

void DatabaseObserver::removeGateway(const am_gatewayID_t gatewayID)
{
    (void) gatewayID;
    //todo: implement something?
}

void DatabaseObserver::removeCrossfader(const am_crossfaderID_t crossfaderID)
{
    mRoutingSender->removeCrossfaderLookup(crossfaderID);
}

void DatabaseObserver::numberOfSinkClassesChanged()
{
    mSerializer.asyncCall<CommandSender>(mCommandSender, &CommandSender::cbNumberOfSinkClassesChanged);
}

void DatabaseObserver::numberOfSourceClassesChanged()
{
    mSerializer.asyncCall<CommandSender>(mCommandSender, &CommandSender::cbNumberOfSourceClassesChanged);
}

void DatabaseObserver::mainConnectionStateChanged(const am_mainConnectionID_t connectionID, const am_ConnectionState_e connectionState)
{
    mSerializer.asyncCall<CommandSender, const am_connectionID_t, const am_ConnectionState_e>(mCommandSender, &CommandSender::cbMainConnectionStateChanged, connectionID, connectionState);
}

void DatabaseObserver::mainSinkSoundPropertyChanged(const am_sinkID_t sinkID, const am_MainSoundProperty_s& SoundProperty)
{
    mSerializer.asyncCall<CommandSender, const am_sinkID_t, const am_MainSoundProperty_s&>(mCommandSender, &CommandSender::cbMainSinkSoundPropertyChanged, sinkID, SoundProperty);
}

void DatabaseObserver::mainSourceSoundPropertyChanged(const am_sourceID_t sourceID, const am_MainSoundProperty_s & SoundProperty)
{
    mSerializer.asyncCall<CommandSender, const am_sourceID_t, const am_MainSoundProperty_s&>(mCommandSender, &CommandSender::cbMainSourceSoundPropertyChanged, sourceID, SoundProperty);
}

void DatabaseObserver::sinkAvailabilityChanged(const am_sinkID_t sinkID, const am_Availability_s & availability)
{
    mSerializer.asyncCall<CommandSender, const am_sinkID_t, const am_Availability_s&>(mCommandSender, &CommandSender::cbSinkAvailabilityChanged, sinkID, availability);
}

void DatabaseObserver::sourceAvailabilityChanged(const am_sourceID_t sourceID, const am_Availability_s & availability)
{
    mSerializer.asyncCall<CommandSender, const am_sourceID_t, const am_Availability_s&>(mCommandSender, &CommandSender::cbSourceAvailabilityChanged, sourceID, availability);
}

void DatabaseObserver::volumeChanged(const am_sinkID_t sinkID, const am_mainVolume_t volume)
{
    mSerializer.asyncCall<CommandSender, const am_sinkID_t, const am_mainVolume_t>(mCommandSender, &CommandSender::cbVolumeChanged, sinkID, volume);
}

void DatabaseObserver::sinkMuteStateChanged(const am_sinkID_t sinkID, const am_MuteState_e muteState)
{
    mSerializer.asyncCall<CommandSender, const am_sinkID_t, const am_MuteState_e>(mCommandSender, &CommandSender::cbSinkMuteStateChanged, sinkID, muteState);
}

void DatabaseObserver::systemPropertyChanged(const am_SystemProperty_s& SystemProperty)
{
    mSerializer.asyncCall<CommandSender, const am_SystemProperty_s&>(mCommandSender, &CommandSender::cbSystemPropertyChanged, SystemProperty);
}

void DatabaseObserver::timingInformationChanged(const am_mainConnectionID_t mainConnection, const am_timeSync_t time)
{
    mSerializer.asyncCall<CommandSender, const am_mainConnectionID_t, const am_timeSync_t>(mCommandSender, &CommandSender::cbTimingInformationChanged, mainConnection, time);
}
