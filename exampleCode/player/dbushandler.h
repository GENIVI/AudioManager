#ifndef DBUSHANDLER_H
#define DBUSHANDLER_H

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
 * \author Sruthi Mohan
 *
 *
 */

/**
     * This is the Class that handles dbus message message communication.
     * This is implemented using glib-dbus-c binding
     * @author Sruthi Mohan
     * @version 1.0
     * @created 10-Feb-2012 1:31:06 PM
     */
#include <stdio.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <dbus/dbus.h>
#include <glib.h>
#include <stdint.h>
#include <dbus/dbus-glib.h>
#include "audiomanagertypes.h"
#include <dbus/dbus-glib-bindings.h>
#include <QObject>
#include <iostream>
#include <vector>

#define SERVICE_NAME "org.genivi.audiomanager"
#define PATH_NAME "/org/genivi/audiomanager/CommandInterface"
#define INTERFACENAME "org.genivi.audiomanager.CommandInterface"

using namespace std;
using namespace am;
class AudiomanagerHMIController;

class DbusHandler : public QObject
{
    Q_OBJECT
public:
    explicit DbusHandler();
signals:
    void NumberOfConnectionsChanged();
    void Volumechanged(am_sinkID_t,am_volume_t);
    void SoundpropertyChanged(am_sinkID_t,am_MainSoundProperty_s);
    void Mutestatechanged(am_sinkID_t,am_MuteState_e);
    void SourceActivity(am_sourceID_t source,am_SourceState_e state);
public slots:
    static void* Initialize(void*);
    int Sender();
    int Sender_forlists();
    am_Error_e connect(am_sourceID_t SourceID,am_sinkID_t SinkID ,am_connectionID_t &ConnectionID);
    am_Error_e disconnect(unsigned int ConnectionID);
    am_Error_e SetVolume(const am_sinkID_t sinkID, const am_mainVolume_t volume) ;
    am_Error_e VolumeStep(const am_sinkID_t sinkID, const int16_t volumeStep) ;
    am_Error_e SetSinkMuteState(const am_sinkID_t sinkID, const am_MuteState_e muteState) ;
    am_Error_e SetMainSinkSoundProperty( const am_sinkID_t sinkID,const am_MainSoundProperty_s& soundProperty) ;
    am_Error_e GetListMainConnections(std::vector<am_MainConnectionType_s* >& listConnections)  ;
    am_Error_e GetListMainSinks(std::vector<am_SinkType_s>& listSinks)  ;
    am_Error_e GetListMainSources(std::vector<am_SourceType_s>& listSources)  ;
    am_Error_e GetListMainSinkSoundProperties(const am_sinkID_t sinkID, std::vector<am_MainSoundProperty_s*>& listSoundProperties)  ;
    static DBusHandlerResult signal_filter(DBusConnection *c, DBusMessage *message, void *user_data);

private:
    static DBusMessage* msg;
    static DBusMessageIter args;
    static DBusConnection* conn;
    static DBusError err;
    static DBusPendingCall* pending;
    static gboolean ret;
    static dbus_uint16_t result;
    static GMainLoop* mainloop;
};
struct s_userdata
{
    GMainLoop* Tempmainloop;
    DbusHandler *ptrDbusHanndler;
};

#endif // DBUSHANDLER_H
