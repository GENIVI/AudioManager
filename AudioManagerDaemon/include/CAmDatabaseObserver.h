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
 * \file CAmDatabaseObserver.h
 * For further information see http://www.genivi.org/.
 *
 */

#ifndef DATABASEOBSERVER_H_
#define DATABASEOBSERVER_H_

#include "audiomanagertypes.h"
#include <queue>
#include "shared/CAmSerializer.h"

namespace am
{

class CAmTelnetServer;
class CAmCommandSender;
class CAmRoutingSender;
class CAmSocketHandler;

/**
 * This class observes the Database and notifies other classes about important events, mainly the CommandSender.
 */
class CAmDatabaseObserver
{
public:
    CAmDatabaseObserver(CAmCommandSender *iCommandSender, CAmRoutingSender *iRoutingSender, CAmSocketHandler *iSocketHandler);
    CAmDatabaseObserver(CAmCommandSender *iCommandSender, CAmRoutingSender *iRoutingSender, CAmSocketHandler *iSocketHandler, CAmTelnetServer *iTelnetServer);
    ~CAmDatabaseObserver();
    void numberOfSinkClassesChanged();
    void numberOfSourceClassesChanged();
    void newSink(const am_Sink_s& sink);
    void newSource(const am_Source_s& source);
    void newDomain(const am_Domain_s& domain);
    void newGateway(const am_Gateway_s& gateway);
    void newCrossfader(const am_Crossfader_s& crossfader);
    void newMainConnection(const am_MainConnectionType_s& mainConnection);
    void removedMainConnection(const am_mainConnectionID_t mainConnection);
    void removedSink(const am_sinkID_t sinkID, const bool visible);
    void removedSource(const am_sourceID_t sourceID, const bool visible);
    void removeDomain(const am_domainID_t domainID);
    void removeGateway(const am_gatewayID_t gatewayID);
    void removeCrossfader(const am_crossfaderID_t crossfaderID);
    void mainConnectionStateChanged(const am_mainConnectionID_t connectionID, const am_ConnectionState_e connectionState);
    void mainSinkSoundPropertyChanged(const am_sinkID_t sinkID, const am_MainSoundProperty_s& SoundProperty);
    void mainSourceSoundPropertyChanged(const am_sourceID_t sourceID, const am_MainSoundProperty_s& SoundProperty);
    void sinkAvailabilityChanged(const am_sinkID_t sinkID, const am_Availability_s& availability);
    void sourceAvailabilityChanged(const am_sourceID_t sourceID, const am_Availability_s& availability);
    void volumeChanged(const am_sinkID_t sinkID, const am_mainVolume_t volume);
    void sinkMuteStateChanged(const am_sinkID_t sinkID, const am_MuteState_e muteState);
    void systemPropertyChanged(const am_SystemProperty_s& SystemProperty);
    void timingInformationChanged(const am_mainConnectionID_t mainConnection, const am_timeSync_t time);

private:
    CAmCommandSender *mCommandSender; //!< pointer to the comandSender
    CAmRoutingSender* mRoutingSender; //!< pointer to the routingSender
    CAmTelnetServer* mTelnetServer; //!< pointer to the telnetserver
    CAmSerializer mSerializer; //!< serializer to handle the CommandInterface via the mainloop
};

}

#endif /* DATABASEOBSERVER_H_ */
