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
#include "dbushandler.h"
#include <stdio.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <dbus/dbus.h>
#include <glib.h>
#include <stdint.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <pthread.h>

DBusMessage* DbusHandler::msg;
DBusMessageIter DbusHandler::args;
DBusConnection* DbusHandler::conn;
DBusError DbusHandler::err;
DBusPendingCall* DbusHandler::pending;
gboolean DbusHandler:: ret;
dbus_uint16_t DbusHandler::result;
GMainLoop* DbusHandler::mainloop;

DbusHandler::DbusHandler()
{
    dbus_error_init(&err);
    conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
    if (!conn)
        printf("did not get connection error %s", err.message);
    pthread_t thread1;
    int threadid;
    threadid = pthread_create(&thread1,NULL,DbusHandler::Initialize,(void*)this);
}

 DBusHandlerResult DbusHandler::signal_filter(DBusConnection *c, DBusMessage *message, void *user_data)
{
    fprintf(stderr,"entered into signal_filter\n");

  //  s_userdata * tempUserdata=static_cast<s_userdata *>(user_data);
  //  GMainLoop *loop = tempUserdata->Tempmainloop;
    DbusHandler * tempHandler=static_cast<DbusHandler *>(user_data);
    DBusMessageIter args,args1;
    dbus_uint16_t sink;


   fprintf(stderr,"received: member %s\n",dbus_message_get_member(message));

    if (dbus_message_is_signal(message, "org.genivi.audiomanager", "VolumeChanged"))
    {
          dbus_int16_t Vol;
        fprintf(stderr,"Message customized received\n");
        if (!dbus_message_iter_init(message, &args))
            fprintf(stderr, "VolumeChanged :Message Has No Parameters\n");
           else
            dbus_message_iter_get_basic(&args, &sink);
        if (!dbus_message_iter_next(&args))
            fprintf(stderr, "VolumeChanged :Message has too few arguments!\n");
        else
            dbus_message_iter_get_basic(&args, &Vol);
        emit tempHandler->Volumechanged(sink,Vol);
        fprintf(stderr,"Got Signal with value %d ,%d\n", sink, Vol);
        return DBUS_HANDLER_RESULT_HANDLED;
    }

    if (dbus_message_is_signal(message, "org.genivi.audiomanager", "NumberOfMainConnectionsChanged"))
    {

        fprintf(stderr,"Message customized received\n");
       emit tempHandler->NumberOfConnectionsChanged();
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    if (dbus_message_is_signal(message, "org.genivi.audiomanager", "MainConnectionStateChanged"))
    {

        fprintf(stderr,"Message customized received\n");
       emit tempHandler->NumberOfConnectionsChanged();
        return DBUS_HANDLER_RESULT_HANDLED;
    }

    if (dbus_message_is_signal(message, "org.genivi.audiomanager", "SinkMuteStateChanged"))
    {dbus_int16_t mutestate;
        fprintf(stderr,"Message customized received\n");
        if (!dbus_message_iter_init(message, &args))
            fprintf(stderr, "SinkMuteStateChanged :Message Has No Parameters\n");
           else
            dbus_message_iter_get_basic(&args, &sink);
        if (!dbus_message_iter_next(&args))
            fprintf(stderr, "SinkMuteStateChanged :Message has too few arguments!\n");
        else{
              dbus_message_iter_get_basic(&args, &mutestate);

              emit tempHandler->Mutestatechanged(sink,static_cast<am_MuteState_e>(mutestate));
        }
        return DBUS_HANDLER_RESULT_HANDLED;
    }
    if (dbus_message_is_signal(message, "org.genivi.audiomanager", "MainSinkSoundPropertyChanged"))
    {dbus_int16_t type,value;
        if (!dbus_message_iter_init(message, &args))
            fprintf(stderr, "SoundProperychanged :Message Has No Parameters\n");
           else
            dbus_message_iter_get_basic(&args, &sink);
            if (!dbus_message_iter_next(&args))
            fprintf(stderr, "SoundProperychanged :Message has too few arguments!\n");
        else
           if (DBUS_TYPE_STRUCT == dbus_message_iter_get_arg_type(&args))
        {

            dbus_message_iter_recurse(&args, &args1);
            dbus_message_iter_get_basic(&args1, &type);
            fprintf(stderr, "type :%d\n",type);
            dbus_message_iter_next(&args1);
            dbus_message_iter_get_basic(&args1, &value);
            fprintf(stderr, "value :%d\n",value);
        }
        fprintf(stderr,"Message customized received\n");
        am_MainSoundProperty_s tempsoundproperty;
        tempsoundproperty.type=static_cast<am_MainSoundPropertyType_e>(type);
        tempsoundproperty.value=value;
       emit tempHandler->SoundpropertyChanged(sink,tempsoundproperty);
        return DBUS_HANDLER_RESULT_HANDLED;
    }


    const char* interface = dbus_message_get_interface(message);
    const char* member = dbus_message_get_member(message);
    const char* path = dbus_message_get_path(message);
    fprintf(stderr,"path %s\n",path);

    if (strcmp(member,"asyncSetSourceState")==0)
    {
        uint16_t sourceID=0, handle=0;
        int16_t state=0;
        if (!dbus_message_iter_init(message, &args))
            fprintf(stderr, "asyncSetSourceState :Message Has No Parameters\n");
           else
             dbus_message_iter_get_basic(&args, &handle);
        if (!dbus_message_iter_next(&args))
            fprintf(stderr, "asyncSetSourceState :Message has too few arguments!\n");
        else
            dbus_message_iter_get_basic(&args, &sourceID);
        if (!dbus_message_iter_next(&args))
            fprintf(stderr, "asyncSetSourceState :Message has too few arguments!\n");
        else
            dbus_message_iter_get_basic(&args, &state);

        emit tempHandler->SourceActivity(static_cast<am_sourceID_t>(sourceID),static_cast<am_SourceState_e>(state));
    }
    return DBUS_HANDLER_RESULT_HANDLED;
 }

void* DbusHandler::Initialize(void * userdata)
{
   // DbusHandler * abc =static_cast<DbusHandler *>(userdata);
    static DBusObjectPathVTable vtable_root;
    vtable_root.message_function = signal_filter;
    mainloop = g_main_loop_new(NULL, FALSE);
       dbus_connection_setup_with_g_main (conn, NULL);
       dbus_bus_add_match(conn, "type='signal',interface='org.genivi.audiomanager'", &err);
       dbus_bus_add_match(conn, "sender='org.genivi.audiomanager',member='asyncSetSourceState'", &err);
       if (dbus_error_is_set(&err))
       {
           printf("Match Error (%s)\n", err.message);
           exit(1);
       }

       ret = dbus_connection_add_filter(conn,signal_filter,userdata, NULL);
       if (!ret)
       {
           fprintf(stderr,"dbus_connection_add_filter failed");
       }

       dbus_connection_flush(conn);
       if (dbus_error_is_set(&err))
       {
           printf("Problem to flush (%s)\n", err.message);
           exit(1);
       }

       fprintf(stderr,"Match rule sent\n");

       g_main_loop_run (mainloop);
}
//-------------------------------------------------------------------------
int DbusHandler::Sender()
{
    if (NULL == msg)
    {
        fprintf(stderr, "Message Null\n");
        exit(1);
    }
   //sleep(1);

    if (!dbus_connection_send_with_reply(conn, msg, &pending, -1))
    {
        fprintf(stderr, "Sender: Out Of Memory!\n");
        exit(1);
    }
    if (NULL == pending)
    {
        fprintf(stderr, "Pending Call Null\n");
        exit(1);
    }
    dbus_connection_flush(conn);
    printf("Request Sent\n");

    dbus_message_unref(msg);

    dbus_pending_call_block(pending);

    msg = dbus_pending_call_steal_reply(pending);
    if (NULL == msg)
    {
        fprintf(stderr, "Reply Null\n");
        exit(1);
    }
    return 0;
}
//---------------------------------------------------------------------
int DbusHandler::Sender_forlists()
{
    if (NULL == msg)
    {
        fprintf(stderr, "Message Null\n");
        exit(1);
    }
   sleep(1);

    if (!dbus_connection_send_with_reply(conn, msg, &pending, -1))
    {
        fprintf(stderr, "Sender: Out Of Memory!\n");
        exit(1);
    }
    if (NULL == pending)
    {
        fprintf(stderr, "Pending Call Null\n");
        exit(1);
    }
    dbus_connection_flush(conn);
    printf("Request Sent\n");

    dbus_message_unref(msg);

    dbus_pending_call_block(pending);

    msg = dbus_pending_call_steal_reply(pending);
    if (NULL == msg)
    {
        fprintf(stderr, "Reply Null\n");
        exit(1);
    }
    return 0;
}

//----------------------------------------------------------------
am_Error_e DbusHandler::connect(am_sourceID_t SourceID,am_sinkID_t SinkID ,am_connectionID_t &ConnectionID)
{
    msg = dbus_message_new_method_call("org.genivi.audiomanager", // target for the method call
                                            "/org/genivi/audiomanager/CommandInterface", // object to call on
                                            "org.genivi.audiomanager.CommandInterface", // interface to call on
                                            "Connect");

    dbus_message_iter_init_append(msg, &args);
       if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT16, &SourceID))
       {
           fprintf(stderr, "in Connect 1st Arg Out Of Memory!\n");
           exit(1);
       }
       if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT16, &SinkID))
       {
           fprintf(stderr, "in Connect 2nd Arg Out Of Memory!\n");
           exit(1);
       }
    Sender();
    dbus_pending_call_unref(pending);
    DBusMessageIter iter1 ;


    if (!dbus_message_iter_init(msg, &iter1))
        fprintf(stderr, "Message Has No Parameters\n");
    if (DBUS_TYPE_INT16!= dbus_message_iter_get_arg_type(&iter1))
    {   printf("Incorrect value recieved %c",dbus_message_iter_get_arg_type(&iter1));
        exit(1);
    }
    dbus_message_iter_get_basic(&iter1, &result);
    fprintf(stderr,"Got result ,%d\n",  result);
    dbus_message_iter_next(&iter1);
    if (DBUS_TYPE_UINT16!= dbus_message_iter_get_arg_type(&iter1))
    {   printf("Incorrect value recieved %c",dbus_message_iter_get_arg_type(&iter1));
        exit(1);
    }
    dbus_message_iter_get_basic(&iter1, &ConnectionID);
    fprintf(stderr,"Got result ,%d\n",  ConnectionID);
return static_cast <am_Error_e>(result);
}

////------------------------------------------------------------------------------------
am_Error_e DbusHandler::disconnect(unsigned int ConnectionID)
{
    msg = dbus_message_new_method_call("org.genivi.audiomanager", // target for the method call
                                            "/org/genivi/audiomanager/CommandInterface", // object to call on
                                            "org.genivi.audiomanager.CommandInterface", // interface to call on
                                            "Disconnect");
    dbus_message_iter_init_append(msg, &args);
       if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT16, &ConnectionID))
       {
           fprintf(stderr, "Out Of Memory!\n");
           exit(1);
       }
       cout<<"Attempting to send"<<endl;
    Sender();
    dbus_pending_call_unref(pending);
    DBusMessageIter iter1 ;
    if (!dbus_message_iter_init(msg, &iter1))
        fprintf(stderr, "Message Has No Parameters\n");
    if (DBUS_TYPE_INT16!= dbus_message_iter_get_arg_type(&iter1))
    {   printf("Incorrect value recieved %c",dbus_message_iter_get_arg_type(&iter1));
        exit(1);
    }
    dbus_message_iter_get_basic(&iter1, &result);
    fprintf(stderr,"Got result ,%d\n",  result);
 return static_cast <am_Error_e>(result);
}
////----------------------------------------------------------------------------
am_Error_e DbusHandler:: SetVolume(const am_sinkID_t sinkID, const am_mainVolume_t volume)
{
    msg = dbus_message_new_method_call("org.genivi.audiomanager", // target for the method call
                                            "/org/genivi/audiomanager/CommandInterface", // object to call on
                                            "org.genivi.audiomanager.CommandInterface", // interface to call on
                                            "SetVolume");
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT16, &sinkID))
       {
           fprintf(stderr, "Out Of Memory!\n");
           exit(1);
       }
       if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_INT16, &volume))
       {
           fprintf(stderr, "Out Of Memory!\n");
           exit(1);
       }

    Sender();
    dbus_pending_call_unref(pending);
    DBusMessageIter iter1 ;


    if (!dbus_message_iter_init(msg, &iter1))
        fprintf(stderr, "Message Has No Parameters\n");
    if (DBUS_TYPE_INT16!= dbus_message_iter_get_arg_type(&iter1))
    {
        printf("Incorrect value recieved %c",dbus_message_iter_get_arg_type(&iter1));

        exit(1);
    }
    dbus_message_iter_get_basic(&iter1, &result);
    fprintf(stderr,"Got result ,%d\n",  result);
 return static_cast <am_Error_e>(result);
}
////------------------------------------------------------------------------------------------

am_Error_e DbusHandler:: VolumeStep(const am_sinkID_t sinkID, const int16_t volumeStep)
{msg = dbus_message_new_method_call("org.genivi.audiomanager", // target for the method call
                                    "/org/genivi/audiomanager/CommandInterface", // object to call on
                                    "org.genivi.audiomanager.CommandInterface", // interface to call on
                                    "VolumeStep");
dbus_message_iter_init_append(msg, &args);
if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT16, &sinkID))
{
   fprintf(stderr, "Out Of Memory!\n");
   exit(1);
}
if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_INT16, &volumeStep))
{
   fprintf(stderr, "Out Of Memory!\n");
   exit(1);
}

Sender();
dbus_pending_call_unref(pending);
DBusMessageIter iter1 ;


if (!dbus_message_iter_init(msg, &iter1))
fprintf(stderr, "Message Has No Parameters\n");
if (DBUS_TYPE_INT16!= dbus_message_iter_get_arg_type(&iter1))
{   printf("Incorrect value recieved %c",dbus_message_iter_get_arg_type(&iter1));
exit(1);
}
dbus_message_iter_get_basic(&iter1, &result);
fprintf(stderr,"Got result ,%d\n",  result);
return static_cast <am_Error_e>(result);

}
////-----------------------------------------------------------------------------------------
am_Error_e DbusHandler:: SetSinkMuteState(const am_sinkID_t sinkID, const am_MuteState_e muteState)
{
    msg = dbus_message_new_method_call("org.genivi.audiomanager", // target for the method call
                                        "/org/genivi/audiomanager/CommandInterface", // object to call on
                                        "org.genivi.audiomanager.CommandInterface", // interface to call on
                                        "SetSinkMuteState");
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT16, &sinkID))
    {
       fprintf(stderr, "Out Of Memory!\n");
       exit(1);
    }
    int temp_mutestate = static_cast<int>(muteState);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_INT16, &temp_mutestate))
    {
       fprintf(stderr, "Out Of Memory!\n");
       exit(1);
    }

    Sender();
    dbus_pending_call_unref(pending);
    DBusMessageIter iter1 ;


    if (!dbus_message_iter_init(msg, &iter1))
    fprintf(stderr, "Message Has No Parameters\n");
    if (DBUS_TYPE_INT16!= dbus_message_iter_get_arg_type(&iter1))
    {   printf("Incorrect value recieved %c",dbus_message_iter_get_arg_type(&iter1));
    exit(1);
    }
    dbus_message_iter_get_basic(&iter1, &result);
    fprintf(stderr,"Got result ,%d\n",  result);
    return static_cast <am_Error_e>(result);


}
////---------------------------------------------------------------------------------------------------
am_Error_e DbusHandler ::SetMainSinkSoundProperty(const am_sinkID_t sinkID, const am_MainSoundProperty_s &soundProperty)
{
    msg = dbus_message_new_method_call("org.genivi.audiomanager", // target for the method call
                                        "/org/genivi/audiomanager/CommandInterface", // object to call on
                                        "org.genivi.audiomanager.CommandInterface", // interface to call on
                                        "SetMainSinkSoundProperty");
    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT16, &sinkID))
    {
       fprintf(stderr, "Out Of Memory!\n");
       exit(1);
    }
    DBusMessageIter structIter;
       dbus_bool_t success = true;
       dbus_message_iter_open_container(&args, DBUS_TYPE_STRUCT, NULL, &structIter);
       success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_INT16, &soundProperty.type);
       success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_INT16, &soundProperty.value);
       success = success && dbus_message_iter_close_container(&args, &structIter);

    Sender();
    dbus_pending_call_unref(pending);
    DBusMessageIter iter1 ;
    if (!dbus_message_iter_init(msg, &iter1))
    fprintf(stderr, "Message Has No Parameters\n");
    if (DBUS_TYPE_INT16!= dbus_message_iter_get_arg_type(&iter1))
    {   printf("Incorrect value recieved %c",dbus_message_iter_get_arg_type(&iter1));
        char * abc;
      dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &abc);
        printf("Incorrect value recieved %s",abc);
    exit(1);
    }
    dbus_message_iter_get_basic(&iter1, &result);
    fprintf(stderr,"Got result ,%d\n",  result);
    return static_cast <am_Error_e>(result);


}
////----------------------------------------------------------------------------------------------------------------------
am_Error_e DbusHandler::GetListMainConnections(std::vector<am_MainConnectionType_s *> &listConnections)
{
    msg = dbus_message_new_method_call("org.genivi.audiomanager", // target for the method call
                                            "/org/genivi/audiomanager/CommandInterface", // object to call on
                                            "org.genivi.audiomanager.CommandInterface", // interface to call on
                                            "GetListMainConnections");
    Sender();
    dbus_pending_call_unref(pending);
    DBusMessageIter subIter,iter1,iter2 ;
    if (!dbus_message_iter_init(msg, &iter1))
        fprintf(stderr, "Message Has No Parameters\n");
    if (DBUS_TYPE_INT16!= dbus_message_iter_get_arg_type(&iter1))
    {   printf("Incorrect value recieved");
        exit(1);
    }
    dbus_message_iter_get_basic(&iter1, &result);
    fprintf(stderr,"Got result ,%d\n",  result);
    dbus_message_iter_next(&iter1);
    if (DBUS_TYPE_ARRAY== dbus_message_iter_get_arg_type(&iter1))
    {  // fprintf(stderr, "a:%d\n", a);
        for (dbus_message_iter_recurse(&iter1, &iter2);
             dbus_message_iter_get_arg_type(&iter2) != DBUS_TYPE_INVALID;
             dbus_message_iter_next(&iter2))
        {
            am_MainConnectionType_s* TempConnection= new am_MainConnectionType_s();
            if (DBUS_TYPE_STRUCT == dbus_message_iter_get_arg_type(&iter2))
            {
                dbus_message_iter_recurse(&iter2, &subIter);
                dbus_uint16_t ConnID,SourceID,SinkID;
                dbus_int16_t Timedelay,ConnectionState;
                dbus_message_iter_get_basic(&subIter, &ConnID);
                dbus_message_iter_next(&subIter);
                dbus_message_iter_get_basic(&subIter, &SourceID);
                dbus_message_iter_next(&subIter);
                dbus_message_iter_get_basic(&subIter, &SinkID);
                dbus_message_iter_next(&subIter);
                dbus_message_iter_get_basic(&subIter, &Timedelay);
                dbus_message_iter_next(&subIter);
                dbus_message_iter_get_basic(&subIter, &ConnectionState);
                dbus_message_iter_next(&subIter);

                {
                TempConnection->mainConnectionID=ConnID;
                TempConnection->sourceID=SourceID;
                TempConnection->sinkID=SinkID;
                TempConnection->delay=Timedelay;
                TempConnection->connectionState=static_cast<am_ConnectionState_e>(ConnectionState);
                listConnections.push_back(TempConnection);
                }
            }

        }
    }



return static_cast <am_Error_e>(result);
}
////------------------------------------------------------------Rossini--------------------------------------------------
am_Error_e DbusHandler::GetListMainSinks(std::vector<am_SinkType_s> &listSinks)
{     msg = dbus_message_new_method_call("org.genivi.audiomanager", // target for the method call
                                         "/org/genivi/audiomanager/CommandInterface", // object to call on
                                         "org.genivi.audiomanager.CommandInterface", // interface to call on
                                         "GetListMainSinks");

      Sender();
      dbus_pending_call_unref(pending);
      DBusMessageIter subIter,iter1,iter2,subsubiter ;

      if (!dbus_message_iter_init(msg, &iter1))
          fprintf(stderr, "Message Has No Parameters\n");
      if (DBUS_TYPE_INT16!= dbus_message_iter_get_arg_type(&iter1))
      {   printf("Incorrect value recieved");
          exit(1);
      }
      dbus_message_iter_get_basic(&iter1, &result);
      fprintf(stderr,"Got result ,%d\n",  result);
      dbus_message_iter_next(&iter1);
      if (DBUS_TYPE_ARRAY== dbus_message_iter_get_arg_type(&iter1))
      {  // fprintf(stderr, "a:%d\n", a);
          for (dbus_message_iter_recurse(&iter1, &iter2);
               dbus_message_iter_get_arg_type(&iter2) != DBUS_TYPE_INVALID;
               dbus_message_iter_next(&iter2))
          {
              am_SinkType_s TempSink;
              if (DBUS_TYPE_STRUCT == dbus_message_iter_get_arg_type(&iter2))
              {
                  dbus_message_iter_recurse(&iter2, &subIter);
                  dbus_uint16_t SinkID,SinkClassID,availability,availability_reason,volume,mute_state;
                  char* Sinkname;
                  dbus_message_iter_get_basic(&subIter, &SinkID);
                  //fprintf(stderr, "sourceID:%d\n",sourceID);
                  dbus_message_iter_next(&subIter);
                  dbus_message_iter_get_basic(&subIter, &Sinkname);
                  //fprintf(stderr, "name :%s\n",sourcename);
                  dbus_message_iter_next(&subIter);
                  if (DBUS_TYPE_STRUCT == dbus_message_iter_get_arg_type(&subIter))
                  {
                      dbus_message_iter_recurse(&subIter, &subsubiter);
                      dbus_message_iter_get_basic(&subsubiter, &availability);
                      fprintf(stderr, "availability value:%d\n",availability);
                      dbus_message_iter_next(&subsubiter);
                      dbus_message_iter_get_basic(&subsubiter, &availability_reason);
                      fprintf(stderr, "availability reason value:%d\n",availability_reason);
                  }
                  dbus_message_iter_next(&subIter);
                  dbus_message_iter_get_basic(&subIter, &volume);
                  dbus_message_iter_next(&subIter);
                  dbus_message_iter_get_basic(&subIter, &mute_state);
                  dbus_message_iter_next(&subIter);
                  dbus_message_iter_get_basic(&subIter, &SinkClassID);
                  fprintf(stderr, "muteState value:%d\n",SinkClassID);
                  fprintf(stderr, "*****************************************\n");
                  TempSink.sinkID=SinkID;
                  TempSink.name=Sinkname;
                  TempSink.availability.availability=static_cast<am_Availablility_e>(availability);
                  TempSink.availability.availabilityReason=static_cast<am_AvailabilityReason_e>(availability_reason);
                  TempSink.sinkClassID=SinkClassID;
                  TempSink.volume=volume;
                  TempSink.muteState=static_cast<am_MuteState_e>(mute_state);
                  listSinks.push_back(TempSink);
              }

          }
      }
      return static_cast <am_Error_e>(result);
}
////-------------------------------------------------------------------------------------------------

am_Error_e DbusHandler::GetListMainSources(std::vector<am_SourceType_s> &listSources)
{

    msg = dbus_message_new_method_call("org.genivi.audiomanager", // target for the method call
                                       "/org/genivi/audiomanager/CommandInterface", // object to call on
                                       "org.genivi.audiomanager.CommandInterface", // interface to call on
                                       "GetListMainSources");

    Sender_forlists();
    dbus_pending_call_unref(pending);
    DBusMessageIter subIter,iter1,iter2,subsubiter ;
    dbus_bool_t success = true;
    if (!dbus_message_iter_init(msg, &iter1))
        fprintf(stderr, "Message Has No Parameters\n");
    if (DBUS_TYPE_INT16!= dbus_message_iter_get_arg_type(&iter1))
    {   printf("Incorrect value recieved");
        exit(1);
    }
    dbus_message_iter_get_basic(&iter1, &result);
    fprintf(stderr,"Got result ,%d\n",  result);
    dbus_message_iter_next(&iter1);
    if (DBUS_TYPE_ARRAY== dbus_message_iter_get_arg_type(&iter1))
    {  // fprintf(stderr, "a:%d\n", a);
        for (dbus_message_iter_recurse(&iter1, &iter2);
             dbus_message_iter_get_arg_type(&iter2) != DBUS_TYPE_INVALID;
             dbus_message_iter_next(&iter2))
        {
            am_SourceType_s TempSource;

            if (DBUS_TYPE_STRUCT == dbus_message_iter_get_arg_type(&iter2))
            {
                dbus_message_iter_recurse(&iter2, &subIter);
                dbus_uint16_t sourceID,SourceClassID,availability,availability_reason;
                char* sourcename;
                dbus_message_iter_get_basic(&subIter, &sourceID);
                //fprintf(stderr, "sourceID:%d\n",sourceID);
                dbus_message_iter_next(&subIter);
                dbus_message_iter_get_basic(&subIter, &sourcename);
                //fprintf(stderr, "name :%s\n",sourcename);
                dbus_message_iter_next(&subIter);
                if (DBUS_TYPE_STRUCT == dbus_message_iter_get_arg_type(&subIter))
                {
                    dbus_message_iter_recurse(&subIter, &subsubiter);
                    dbus_message_iter_get_basic(&subsubiter, &availability);
                    fprintf(stderr, "availability value:%d\n",availability);
                    dbus_message_iter_next(&subsubiter);
                    dbus_message_iter_get_basic(&subsubiter, &availability_reason);
                    fprintf(stderr, "availability reason value:%d\n",availability_reason);
                }
                dbus_message_iter_next(&subIter);
                dbus_message_iter_get_basic(&subIter, &SourceClassID);
                fprintf(stderr, "muteState value:%d\n",SourceClassID);
                fprintf(stderr, "*****************************************\n");
                TempSource.sourceID=sourceID;
                TempSource.name=sourcename;
                TempSource.availability.availability=static_cast<am_Availablility_e>(availability);
                TempSource.availability.availabilityReason=static_cast<am_AvailabilityReason_e>(availability_reason);
                TempSource.sourceClassID=SourceClassID;
                listSources.push_back(TempSource);
            }

        }
    }
    return static_cast <am_Error_e>(result);
}


////--------------------------------------------------------------------------------------------------------------------
am_Error_e DbusHandler::GetListMainSinkSoundProperties(const am_sinkID_t sinkID, std::vector<am_MainSoundProperty_s *> &listSoundProperties)
{ msg = dbus_message_new_method_call("org.genivi.audiomanager", // target for the method call
                                     "/org/genivi/audiomanager/CommandInterface", // object to call on
                                     "org.genivi.audiomanager.CommandInterface", // interface to call on
                                     "GetListMainSinkSoundProperties");


    dbus_message_iter_init_append(msg, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT16, &sinkID))
    {
        fprintf(stderr, "GetListMainSinkSoundProperties Out Of Memory!\n");
        exit(1);
    }
  Sender();
  dbus_pending_call_unref(pending);
  DBusMessageIter subIter,iter1,iter2 ;

  if (!dbus_message_iter_init(msg, &iter1))
      fprintf(stderr, "Message Has No Parameters\n");
  if (DBUS_TYPE_INT16!= dbus_message_iter_get_arg_type(&iter1))
  {   printf("Incorrect value recieved,%c",dbus_message_iter_get_arg_type(&iter1));
      exit(1);
  }

  dbus_message_iter_get_basic(&iter1, &result);

  dbus_message_iter_next(&iter1);
  if (DBUS_TYPE_ARRAY== dbus_message_iter_get_arg_type(&iter1))
  {  // fprintf(stderr, "a:%d\n", a);
      for (dbus_message_iter_recurse(&iter1, &iter2);
           dbus_message_iter_get_arg_type(&iter2) != DBUS_TYPE_INVALID;
           dbus_message_iter_next(&iter2))
      {am_MainSoundProperty_s* SoundProperty= new am_MainSoundProperty_s();

          if (DBUS_TYPE_STRUCT == dbus_message_iter_get_arg_type(&iter2))
          {
              dbus_message_iter_recurse(&iter2, &subIter);
              dbus_uint16_t Type,Value;
              dbus_message_iter_get_basic(&subIter, &Type);
              //fprintf(stderr, "sourceID:%d\n",sourceID);
              dbus_message_iter_next(&subIter);
              dbus_message_iter_get_basic(&subIter, &Value);
              SoundProperty->type=static_cast<am_MainSoundPropertyType_e>(Type);
              SoundProperty->value=Value;
              listSoundProperties.push_back(SoundProperty);
          }

      }
  }
  return static_cast <am_Error_e>(result);

}

