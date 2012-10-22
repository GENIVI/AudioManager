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


#ifndef AUDIOMANAGERINTERACTOR_H
#define AUDIOMANAGERINTERACTOR_H

#include <dbus/dbus.h>
#include <glib.h>
#include <stdint.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <QtGui/QWidget>
#include <QtGui/QApplication>
#include <QtCore/QTimerEvent>
#include <QtGui/QShowEvent>
#include <QtGui/QIcon>
#include <QtCore/QBasicTimer>
#include <QtGui/QAction>
#include <QObject>
#include <phonon/phononnamespace.h>
#include "dbushandler.h"

class AudioManagerInteractor : public QObject
{
    Q_OBJECT
public:
    AudioManagerInteractor(DbusHandler* DbusHandler, am_sourceID_t sourceID, am_sinkID_t sinkID);
    ~AudioManagerInteractor();

signals:
    void play();
    void stop();

public slots:
    void playPause();
    void SourceActivity(am_sourceID_t source,am_SourceState_e state);
    void getPlayerState(Phonon::State newstate);
private:
    DbusHandler* mDbusHandler;
    am_sourceID_t mSourceID;
    am_sinkID_t mSinkID;
    am_connectionID_t mConnectionID;
    Phonon::State mState;
    enum connected_e
    {
        CONNECTED,
        CONNECTING,
        DISCONNECTED
    };
    connected_e mConnectedState;
};

#endif // AUDIOMANAGERINTERACTOR_H
