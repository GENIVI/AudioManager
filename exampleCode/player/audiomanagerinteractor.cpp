/**
 * Copyright (C) 2012, BMW AG
 *
 *
 * \copyright
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, version 2
 * of the License.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 *
 * \author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
 *
 *
 */

#include "audiomanagerinteractor.h"
#include <iostream>

using namespace am;

AudioManagerInteractor::AudioManagerInteractor(DbusHandler* DbusHandler, am_sourceID_t sourceID, am_sinkID_t sinkID) :
    mDbusHandler(DbusHandler),
    mSourceID(sourceID),
    mSinkID(sinkID),
    mConnectionID(0),
    mState(Phonon::StoppedState),
    mConnectedState(DISCONNECTED)
{
}

void AudioManagerInteractor::playPause()
{
    if (mState==Phonon::StoppedState || mState==Phonon::PausedState)
    {
        mDbusHandler->connect(mSourceID,mSinkID,mConnectionID);
        mConnectedState=CONNECTING;
    }
    else
    {
        mDbusHandler->disconnect(mConnectionID);
        mConnectedState=DISCONNECTED;
     }
}

void AudioManagerInteractor::SourceActivity(am_sourceID_t source,am_SourceState_e state)
{
    std::cout<<"got source activity ID "<<source<<" "<<state<<std::endl;
    if (source==mSourceID && state==SS_ON)
        emit this->play();
    else if (source==mSourceID && (state==SS_OFF || state==SS_PAUSED))
        emit this->stop();
}

void AudioManagerInteractor::getPlayerState(Phonon::State newstate)
{
    mState=newstate;
    if (newstate==Phonon::StoppedState && mConnectedState==CONNECTED)
        mDbusHandler->disconnect(mConnectionID);
    else if (mConnectedState==CONNECTING)
        mConnectedState=CONNECTED;
}


AudioManagerInteractor::~AudioManagerInteractor()
{

}
