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
 * \file CAmDatabaseObserver.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include "CAmDatabaseObserver.h"
#include <string.h>
#include <cassert>
#include <errno.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include "CAmCommandSender.h"
#include "CAmRoutingSender.h"
#include "CAmTelnetServer.h"
#include "shared/CAmDltWrapper.h"
#include "shared/CAmSerializer.h"

namespace am {

CAmDatabaseObserver::CAmDatabaseObserver(CAmCommandSender *iCommandSender, CAmRoutingSender *iRoutingSender, CAmSocketHandler *iSocketHandler) :
        mCommandSender(iCommandSender), //
        mRoutingSender(iRoutingSender), //
        mTelnetServer(NULL), //
        mSerializer(iSocketHandler) //
{
    assert(mCommandSender!=0);
    assert(mRoutingSender!=0);
    assert(iSocketHandler!=0);
}

CAmDatabaseObserver::CAmDatabaseObserver(CAmCommandSender *iCommandSender, CAmRoutingSender *iRoutingSender, CAmSocketHandler *iSocketHandler, CAmTelnetServer *iTelnetServer) :
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

CAmDatabaseObserver::~CAmDatabaseObserver()
{
}

void CAmDatabaseObserver::newMainConnection(const am_MainConnectionType_s& mainConnection)
{
    mSerializer.asyncCall<CAmCommandSender, const am_MainConnectionType_s>(mCommandSender, &CAmCommandSender::cbNewMainConnection, mainConnection);
}

void CAmDatabaseObserver::removedMainConnection(const am_mainConnectionID_t mainConnection)
{
    mSerializer.asyncCall<CAmCommandSender, const am_mainConnectionID_t>(mCommandSender, &CAmCommandSender::cbRemovedMainConnection, mainConnection);
}

void CAmDatabaseObserver::newSink(const am_Sink_s& sink)
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
        mSerializer.asyncCall<CAmCommandSender, const am_SinkType_s>(mCommandSender, &CAmCommandSender::cbNewSink, s);
    }
}

void CAmDatabaseObserver::newSource(const am_Source_s& source)
{
    mRoutingSender->addSourceLookup(source);
    if (source.visible)
    {
        am_SourceType_s s;
        s.availability = source.available;
        s.name = source.name;
        s.sourceClassID = source.sourceClassID;
        s.sourceID = source.sourceID;
        mSerializer.asyncCall<CAmCommandSender, const am_SourceType_s>(mCommandSender, &CAmCommandSender::cbNewSource, s);
    }
}

void CAmDatabaseObserver::newDomain(const am_Domain_s& domain)
{
    mRoutingSender->addDomainLookup(domain);
}

void CAmDatabaseObserver::newGateway(const am_Gateway_s& gateway)
{
    (void) gateway;
    //todo: implement something
}

void CAmDatabaseObserver::newCrossfader(const am_Crossfader_s& crossfader)
{
    mRoutingSender->addCrossfaderLookup(crossfader);
}

void CAmDatabaseObserver::removedSink(const am_sinkID_t sinkID, const bool visible)
{
    mRoutingSender->removeSinkLookup(sinkID);

    if (visible)
        mSerializer.asyncCall<CAmCommandSender, const am_sinkID_t>(mCommandSender, &CAmCommandSender::cbRemovedSink, sinkID);
}

void CAmDatabaseObserver::removedSource(const am_sourceID_t sourceID, const bool visible)
{
    mRoutingSender->removeSourceLookup(sourceID);

    if (visible)
        mSerializer.asyncCall<CAmCommandSender, const am_sourceID_t>(mCommandSender, &CAmCommandSender::cbRemovedSource, sourceID);
}

void CAmDatabaseObserver::removeDomain(const am_domainID_t domainID)
{
    mRoutingSender->removeDomainLookup(domainID);
}

void CAmDatabaseObserver::removeGateway(const am_gatewayID_t gatewayID)
{
    (void) gatewayID;
    //todo: implement something?
}

void CAmDatabaseObserver::removeCrossfader(const am_crossfaderID_t crossfaderID)
{
    mRoutingSender->removeCrossfaderLookup(crossfaderID);
}

void CAmDatabaseObserver::numberOfSinkClassesChanged()
{
    mSerializer.asyncCall<CAmCommandSender>(mCommandSender, &CAmCommandSender::cbNumberOfSinkClassesChanged);
}

void CAmDatabaseObserver::numberOfSourceClassesChanged()
{
    mSerializer.asyncCall<CAmCommandSender>(mCommandSender, &CAmCommandSender::cbNumberOfSourceClassesChanged);
}

void CAmDatabaseObserver::mainConnectionStateChanged(const am_mainConnectionID_t connectionID, const am_ConnectionState_e connectionState)
{
    mSerializer.asyncCall<CAmCommandSender, const am_connectionID_t, const am_ConnectionState_e>(mCommandSender, &CAmCommandSender::cbMainConnectionStateChanged, connectionID, connectionState);
}

void CAmDatabaseObserver::mainSinkSoundPropertyChanged(const am_sinkID_t sinkID, const am_MainSoundProperty_s& SoundProperty)
{
    mSerializer.asyncCall<CAmCommandSender, const am_sinkID_t, const am_MainSoundProperty_s>(mCommandSender, &CAmCommandSender::cbMainSinkSoundPropertyChanged, sinkID, SoundProperty);
}

void CAmDatabaseObserver::mainSourceSoundPropertyChanged(const am_sourceID_t sourceID, const am_MainSoundProperty_s & SoundProperty)
{
    mSerializer.asyncCall<CAmCommandSender, const am_sourceID_t, const am_MainSoundProperty_s>(mCommandSender, &CAmCommandSender::cbMainSourceSoundPropertyChanged, sourceID, SoundProperty);
}

void CAmDatabaseObserver::sinkAvailabilityChanged(const am_sinkID_t sinkID, const am_Availability_s & availability)
{
    mSerializer.asyncCall<CAmCommandSender, const am_sinkID_t, const am_Availability_s>(mCommandSender, &CAmCommandSender::cbSinkAvailabilityChanged, sinkID, availability);
}

void CAmDatabaseObserver::sourceAvailabilityChanged(const am_sourceID_t sourceID, const am_Availability_s & availability)
{
    mSerializer.asyncCall<CAmCommandSender, const am_sourceID_t, const am_Availability_s>(mCommandSender, &CAmCommandSender::cbSourceAvailabilityChanged, sourceID, availability);
}

void CAmDatabaseObserver::volumeChanged(const am_sinkID_t sinkID, const am_mainVolume_t volume)
{
    mSerializer.asyncCall<CAmCommandSender, const am_sinkID_t, const am_mainVolume_t>(mCommandSender, &CAmCommandSender::cbVolumeChanged, sinkID, volume);
}

void CAmDatabaseObserver::sinkMuteStateChanged(const am_sinkID_t sinkID, const am_MuteState_e muteState)
{
    mSerializer.asyncCall<CAmCommandSender, const am_sinkID_t, const am_MuteState_e>(mCommandSender, &CAmCommandSender::cbSinkMuteStateChanged, sinkID, muteState);
}

void CAmDatabaseObserver::systemPropertyChanged(const am_SystemProperty_s& SystemProperty)
{
    mSerializer.asyncCall<CAmCommandSender, const am_SystemProperty_s>(mCommandSender, &CAmCommandSender::cbSystemPropertyChanged, SystemProperty);
}

void CAmDatabaseObserver::timingInformationChanged(const am_mainConnectionID_t mainConnection, const am_timeSync_t time)
{
    mSerializer.asyncCall<CAmCommandSender, const am_mainConnectionID_t, const am_timeSync_t>(mCommandSender, &CAmCommandSender::cbTimingInformationChanged, mainConnection, time);
}
}
