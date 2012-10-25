/**
 *  Copyright (c) 2012 BMW
 *
 *  \author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
 *
 *  \copyright
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction,
 *  including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 *  subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 *  THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *  For further information see http://www.genivi.org/.
 */

#include "IAmCommandReceiverShadow.h"
#include <string>
#include <fstream>
#include <stdexcept>
#include <cassert>
#include "audiomanagertypes.h"
#include "CAmCommandSenderDbus.h"
#include "shared/CAmDltWrapper.h"
#include "configCommandDbus.h"

using namespace am;

DLT_IMPORT_CONTEXT(commandDbus)

/**
 * static ObjectPathTable is needed for DBus Callback handling
 */
static DBusObjectPathVTable gObjectPathVTable;

IAmCommandReceiverShadow::IAmCommandReceiverShadow() :
        mFunctionMap(createMap()), //
        mDBUSMessageHandler(), //
        mpIAmCommandReceive(NULL), //
        mpCAmDbusWrapper(NULL)
{
    log(&commandDbus, DLT_LOG_INFO, "CommandReceiverShadow constructed");
}

IAmCommandReceiverShadow::~IAmCommandReceiverShadow()
{
    log(&commandDbus, DLT_LOG_INFO, "CommandReceiverShadow destructed");
}

void IAmCommandReceiverShadow::connect(DBusConnection *conn, DBusMessage *msg)
{
    log(&commandDbus, DLT_LOG_INFO, "CommandReceiverShadow::connect called");

    (void) conn;
    assert(mpIAmCommandReceive!=NULL);

    mDBUSMessageHandler.initReceive(msg);
    am_sourceID_t sourceID = (am_sourceID_t) mDBUSMessageHandler.getUInt();
    am_sinkID_t sinkID = (am_sinkID_t) mDBUSMessageHandler.getUInt();
    am_mainConnectionID_t mainConnectionID = 0;
    am_Error_e returnCode = mpIAmCommandReceive->connect(sourceID, sinkID, mainConnectionID);
    mDBUSMessageHandler.initReply(msg);
    mDBUSMessageHandler.append((dbus_int16_t) returnCode);
    mDBUSMessageHandler.append((dbus_uint16_t) mainConnectionID);
    mDBUSMessageHandler.sendMessage();
}

void IAmCommandReceiverShadow::disconnect(DBusConnection *conn, DBusMessage *msg)
{
    log(&commandDbus, DLT_LOG_INFO, "CommandReceiverShadow::disconnect called");

    (void) conn;
    assert(mpIAmCommandReceive!=NULL);

    mDBUSMessageHandler.initReceive(msg);
    am_mainConnectionID_t mainConnnectionID = (am_mainConnectionID_t) mDBUSMessageHandler.getUInt();
    am_Error_e returnCode = mpIAmCommandReceive->disconnect(mainConnnectionID);
    mDBUSMessageHandler.initReply(msg);
    mDBUSMessageHandler.append((dbus_int16_t) returnCode);
    mDBUSMessageHandler.sendMessage();
}

void IAmCommandReceiverShadow::setVolume(DBusConnection *conn, DBusMessage *msg)
{
    log(&commandDbus, DLT_LOG_INFO, "CommandReceiverShadow::setVolume called");

    (void) conn;
    assert(mpIAmCommandReceive!=NULL);

    mDBUSMessageHandler.initReceive(msg);
    am_sinkID_t sinkID = (am_sinkID_t) mDBUSMessageHandler.getUInt();
    am_volume_t volume = (am_volume_t) mDBUSMessageHandler.getInt();
    am_Error_e returnCode = mpIAmCommandReceive->setVolume(sinkID, volume);
    mDBUSMessageHandler.initReply(msg);
    mDBUSMessageHandler.append((dbus_int16_t) returnCode);
    mDBUSMessageHandler.sendMessage();
}

void IAmCommandReceiverShadow::volumeStep(DBusConnection *conn, DBusMessage *msg)
{
    log(&commandDbus, DLT_LOG_INFO, "CommandReceiverShadow::volumeStep called");

    (void) conn;
    assert(mpIAmCommandReceive!=NULL);

    mDBUSMessageHandler.initReceive(msg);
    am_sinkID_t sinkID = (am_sinkID_t) mDBUSMessageHandler.getUInt();
    int16_t volumeStep = (int16_t) mDBUSMessageHandler.getInt();
    am_Error_e returnCode = mpIAmCommandReceive->volumeStep(sinkID, volumeStep);
    mDBUSMessageHandler.initReply(msg);
    mDBUSMessageHandler.append((dbus_int16_t) returnCode);
    mDBUSMessageHandler.sendMessage();
}

void IAmCommandReceiverShadow::setSinkMuteState(DBusConnection *conn, DBusMessage *msg)
{
    log(&commandDbus, DLT_LOG_INFO, "CommandReceiverShadow::setSinkMuteState called");

    (void) conn;
    assert(mpIAmCommandReceive!=NULL);

    mDBUSMessageHandler.initReceive(msg);
    am_sinkID_t sinkID = (am_sinkID_t) mDBUSMessageHandler.getUInt();
    am_MuteState_e muteState = (am_MuteState_e) mDBUSMessageHandler.getInt();
    am_Error_e returnCode = mpIAmCommandReceive->setSinkMuteState(sinkID, muteState);
    mDBUSMessageHandler.initReply(msg);
    mDBUSMessageHandler.append((dbus_int16_t) returnCode);
    mDBUSMessageHandler.sendMessage();
}

void IAmCommandReceiverShadow::setMainSinkSoundProperty(DBusConnection *conn, DBusMessage *msg)
{
    log(&commandDbus, DLT_LOG_INFO, "CommandReceiverShadow::setMainSinkSoundProperty called");

    (void) conn;
    assert(mpIAmCommandReceive!=NULL);

    mDBUSMessageHandler.initReceive(msg);
    am_sinkID_t sinkID = (am_sinkID_t) mDBUSMessageHandler.getUInt();
    dbus_int16_t type = 0;
    dbus_int16_t value = 0;
    mDBUSMessageHandler.getProperty(type, value);
    am_MainSoundProperty_s mainSoundProperty;
    mainSoundProperty.type = (am_MainSoundPropertyType_e) type;
    mainSoundProperty.value = (int32_t) value;
    am_Error_e returnCode = mpIAmCommandReceive->setMainSinkSoundProperty(mainSoundProperty, sinkID);
    mDBUSMessageHandler.initReply(msg);
    mDBUSMessageHandler.append((dbus_int16_t) returnCode);
    mDBUSMessageHandler.sendMessage();
}

void IAmCommandReceiverShadow::setMainSourceSoundProperty(DBusConnection *conn, DBusMessage *msg)
{
    log(&commandDbus, DLT_LOG_INFO, "CommandReceiverShadow::setMainSourceSoundProperty called");

    (void) conn;
    assert(mpIAmCommandReceive!=NULL);

    mDBUSMessageHandler.initReceive(msg);
    am_sourceID_t sourceID = (am_sinkID_t) mDBUSMessageHandler.getUInt();
    dbus_int16_t type = 0;
    dbus_int16_t value = 0;
    mDBUSMessageHandler.getProperty(type, value);
    am_MainSoundProperty_s mainSoundProperty;
    mainSoundProperty.type = (am_MainSoundPropertyType_e) type;
    mainSoundProperty.value = (int32_t) value;
    am_Error_e returnCode = mpIAmCommandReceive->setMainSourceSoundProperty(mainSoundProperty, sourceID);
    mDBUSMessageHandler.initReply(msg);
    mDBUSMessageHandler.append((dbus_int16_t) returnCode);
    mDBUSMessageHandler.sendMessage();
}

void IAmCommandReceiverShadow::setSystemProperty(DBusConnection *conn, DBusMessage *msg)
{
    log(&commandDbus, DLT_LOG_INFO, "CommandReceiverShadow::setSystemProperty called");

    (void) conn;
    assert(mpIAmCommandReceive!=NULL);

    mDBUSMessageHandler.initReceive(msg);
    dbus_int16_t type = 0;
    dbus_int16_t value = 0;
    mDBUSMessageHandler.getProperty(type, value);
    am_SystemProperty_s systemProperty;
    systemProperty.type = (am_SystemPropertyType_e) type;
    systemProperty.value = (int32_t) value;
    am_Error_e returnCode = mpIAmCommandReceive->setSystemProperty(systemProperty);
    mDBUSMessageHandler.initReply(msg);
    mDBUSMessageHandler.append((dbus_int16_t) returnCode);
    mDBUSMessageHandler.sendMessage();
}

void IAmCommandReceiverShadow::getListMainConnections(DBusConnection *conn, DBusMessage *msg)
{
    log(&commandDbus, DLT_LOG_INFO, "CommandReceiverShadow::getListMainConnections called");

    (void) conn;
    assert(mpIAmCommandReceive!=NULL);
    std::vector<am_MainConnectionType_s> listMainConnections;
    am_Error_e returnCode = mpIAmCommandReceive->getListMainConnections(listMainConnections);
    mDBUSMessageHandler.initReply(msg);
    mDBUSMessageHandler.append((dbus_int16_t) returnCode);
    mDBUSMessageHandler.append(listMainConnections);
    mDBUSMessageHandler.sendMessage();
}

void IAmCommandReceiverShadow::getListMainSinks(DBusConnection *conn, DBusMessage *msg)
{
    log(&commandDbus, DLT_LOG_INFO, "CommandReceiverShadow::getListMainSinks called");

    (void) conn;
    assert(mpIAmCommandReceive!=NULL);
    std::vector<am_SinkType_s> listSinks;
    am_Error_e returnCode = mpIAmCommandReceive->getListMainSinks(listSinks);
    mDBUSMessageHandler.initReply(msg);
    mDBUSMessageHandler.append((dbus_int16_t) returnCode);
    mDBUSMessageHandler.append(listSinks);
    mDBUSMessageHandler.sendMessage();
}

void IAmCommandReceiverShadow::getListMainSources(DBusConnection *conn, DBusMessage *msg)
{
    log(&commandDbus, DLT_LOG_INFO, "CommandReceiverShadow::getListMainSources called");

    (void) conn;
    assert(mpIAmCommandReceive!=NULL);
    std::vector<am_SourceType_s> listSources;
    am_Error_e returnCode = mpIAmCommandReceive->getListMainSources(listSources);
    mDBUSMessageHandler.initReply(msg);
    mDBUSMessageHandler.append((dbus_int16_t) returnCode);
    mDBUSMessageHandler.append(listSources);
    mDBUSMessageHandler.sendMessage();
}

void IAmCommandReceiverShadow::getListMainSinkSoundProperties(DBusConnection *conn, DBusMessage *msg)
{
    log(&commandDbus, DLT_LOG_INFO, "CommandReceiverShadow::getListMainSinkSoundProperties called");

    (void) conn;
    assert(mpIAmCommandReceive!=NULL);

    mDBUSMessageHandler.initReceive(msg);
    am_sinkID_t sinkID = (am_sinkID_t) mDBUSMessageHandler.getUInt();
    std::vector<am_MainSoundProperty_s> listSinkSoundProperties;
    am_Error_e returnCode = mpIAmCommandReceive->getListMainSinkSoundProperties(sinkID, listSinkSoundProperties);
    mDBUSMessageHandler.initReply(msg);
    mDBUSMessageHandler.append((dbus_int16_t) returnCode);
    mDBUSMessageHandler.append(listSinkSoundProperties);
    mDBUSMessageHandler.sendMessage();
}

void IAmCommandReceiverShadow::getListMainSourceSoundProperties(DBusConnection *conn, DBusMessage *msg)
{
    log(&commandDbus, DLT_LOG_INFO, "CommandReceiverShadow::getListMainSourceSoundProperties called");

    (void) conn;
    assert(mpIAmCommandReceive!=NULL);

    mDBUSMessageHandler.initReceive(msg);
    am_sourceID_t sourceID = (am_sourceID_t) mDBUSMessageHandler.getUInt();
    std::vector<am_MainSoundProperty_s> listSinkSoundProperties;
    am_Error_e returnCode = mpIAmCommandReceive->getListMainSourceSoundProperties(sourceID, listSinkSoundProperties);
    mDBUSMessageHandler.initReply(msg);
    mDBUSMessageHandler.append((dbus_int16_t) returnCode);
    mDBUSMessageHandler.append(listSinkSoundProperties);
    mDBUSMessageHandler.sendMessage();
}

void IAmCommandReceiverShadow::getListSourceClasses(DBusConnection *conn, DBusMessage *msg)
{
    log(&commandDbus, DLT_LOG_INFO, "CommandReceiverShadow::getListSourceClasses called");

    (void) conn;
    assert(mpIAmCommandReceive!=NULL);
    std::vector<am_SourceClass_s> listSourceClasses;
    am_Error_e returnCode = mpIAmCommandReceive->getListSourceClasses(listSourceClasses);
    mDBUSMessageHandler.initReply(msg);
    mDBUSMessageHandler.append((dbus_int16_t) returnCode);
    mDBUSMessageHandler.append(listSourceClasses);
    mDBUSMessageHandler.sendMessage();
}

void IAmCommandReceiverShadow::getListSinkClasses(DBusConnection *conn, DBusMessage *msg)
{
    log(&commandDbus, DLT_LOG_INFO, "CommandReceiverShadow::getListSinkClasses called");

    (void) conn;
    assert(mpIAmCommandReceive!=NULL);
    std::vector<am_SinkClass_s> listSinkClasses;
    am_Error_e returnCode = mpIAmCommandReceive->getListSinkClasses(listSinkClasses);
    mDBUSMessageHandler.initReply(msg);
    mDBUSMessageHandler.append((dbus_int16_t) returnCode);
    mDBUSMessageHandler.append(listSinkClasses);
    mDBUSMessageHandler.sendMessage();
}

void IAmCommandReceiverShadow::getListSystemProperties(DBusConnection *conn, DBusMessage *msg)
{
    log(&commandDbus, DLT_LOG_INFO, "CommandReceiverShadow::getListSystemProperties called");

    (void) conn;
    assert(mpIAmCommandReceive!=NULL);
    std::vector<am_SystemProperty_s> listSystemProperties;
    am_Error_e returnCode = mpIAmCommandReceive->getListSystemProperties(listSystemProperties);
    mDBUSMessageHandler.initReply(msg);
    mDBUSMessageHandler.append((dbus_int16_t) returnCode);
    mDBUSMessageHandler.append(listSystemProperties);
    mDBUSMessageHandler.sendMessage();
}

void IAmCommandReceiverShadow::getTimingInformation(DBusConnection *conn, DBusMessage *msg)
{
    log(&commandDbus, DLT_LOG_INFO, "CommandReceiverShadow::getTimingInformation called");

    (void) conn;
    assert(mpIAmCommandReceive!=NULL);

    mDBUSMessageHandler.initReceive(msg);
    am_mainConnectionID_t mainConnectionID = (am_mainConnectionID_t) mDBUSMessageHandler.getUInt();
    am_timeSync_t delay = 0;
    am_Error_e returnCode = mpIAmCommandReceive->getTimingInformation(mainConnectionID, delay);
    mDBUSMessageHandler.initReply(msg);
    mDBUSMessageHandler.append((dbus_int16_t) returnCode);
    mDBUSMessageHandler.append((dbus_int16_t) delay);
    mDBUSMessageHandler.sendMessage();
}

DBusHandlerResult IAmCommandReceiverShadow::receiveCallback(DBusConnection *conn, DBusMessage *msg, void *user_data)
{
    assert(conn!=NULL);
    assert(msg!=NULL);
    assert(user_data!=NULL);
    IAmCommandReceiverShadow* reference = (IAmCommandReceiverShadow*) user_data;
    return (reference->receiveCallbackDelegate(conn, msg));
}

void IAmCommandReceiverShadow::sendIntrospection(DBusConnection *conn, DBusMessage *msg)
{
    assert(conn!=NULL);
    assert(msg!=NULL);
    DBusMessage* reply;
    DBusMessageIter args;
    dbus_uint32_t serial = 0;

    // create a reply from the message
    reply = dbus_message_new_method_return(msg);
    std::string fullpath(COMMAND_DBUS_INTROSPECTION_FILE);
    std::ifstream in(fullpath.c_str(), std::ifstream::in);
    if (!in)
    {
        logError("IAmCommandReceiverShadow::sendIntrospection could not load xml file ",fullpath);
        throw std::runtime_error("IAmCommandReceiverShadow::sendIntrospection Could not load introspecton XML");
    }
    std::string introspect((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    const char* string = introspect.c_str();

    // add the arguments to the reply
    dbus_message_iter_init_append(reply, &args);
    if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &string))
    {
        //	DLT_LOG(DLT_CONTEXT,DLT_LOG_ERROR, DLT_STRING("DBUS handler Out Of Memory!"));
    }

    // send the reply && flush the connection
    if (!dbus_connection_send(conn, reply, &serial))
    {
        //	DLT_LOG(DLT_CONTEXT,DLT_LOG_ERROR, DLT_STRING("DBUS handler Out Of Memory!"));
    }
    dbus_connection_flush(conn);

    // free the reply
    dbus_message_unref(reply);
}

DBusHandlerResult IAmCommandReceiverShadow::receiveCallbackDelegate(DBusConnection *conn, DBusMessage *msg)
{
    //DLT_LOG(dMain, DLT_LOG_INFO, DLT_STRING("message received"));

    if (dbus_message_is_method_call(msg, DBUS_INTERFACE_INTROSPECTABLE, "Introspect"))
    {
        sendIntrospection(conn, msg);
        return (DBUS_HANDLER_RESULT_HANDLED);
    }

    functionMap_t::iterator iter = mFunctionMap.begin();
    std::string k(dbus_message_get_member(msg));
    iter = mFunctionMap.find(k);
    if (iter != mFunctionMap.end())
    {
        std::string p(iter->first);
        CallBackMethod cb = iter->second;
        (this->*cb)(conn, msg);
        return (DBUS_HANDLER_RESULT_HANDLED);
    }

    return (DBUS_HANDLER_RESULT_NOT_YET_HANDLED);
}

void IAmCommandReceiverShadow::setCommandReceiver(IAmCommandReceive*& receiver)
{
    assert(receiver!=NULL);
    mpIAmCommandReceive = receiver;

    gObjectPathVTable.message_function = IAmCommandReceiverShadow::receiveCallback;

    DBusConnection* connection;
    mpIAmCommandReceive->getDBusConnectionWrapper(mpCAmDbusWrapper);
    assert(mpCAmDbusWrapper!=NULL);

    mpCAmDbusWrapper->getDBusConnection(connection);
    assert(connection!=NULL);
    mDBUSMessageHandler.setDBusConnection(connection);

    std::string path(MY_NODE);
    mpCAmDbusWrapper->registerCallback(&gObjectPathVTable, path, this);
}

IAmCommandReceiverShadow::functionMap_t IAmCommandReceiverShadow::createMap()
{
    functionMap_t m;
    m["Connect"] = &IAmCommandReceiverShadow::connect;
    m["Disconnect"] = &IAmCommandReceiverShadow::disconnect;
    m["SetVolume"] = &IAmCommandReceiverShadow::setVolume;
    m["VolumeStep"] = &IAmCommandReceiverShadow::volumeStep;
    m["SetSinkMuteState"] = &IAmCommandReceiverShadow::setSinkMuteState;
    m["SetMainSinkSoundProperty"] = &IAmCommandReceiverShadow::setMainSinkSoundProperty;
    m["SetMainSourceSoundProperty"] = &IAmCommandReceiverShadow::setMainSourceSoundProperty;
    m["GetListMainConnections"] = &IAmCommandReceiverShadow::getListMainConnections;
    m["GetListMainSinks"] = &IAmCommandReceiverShadow::getListMainSinks;
    m["GetListMainSources"] = &IAmCommandReceiverShadow::getListMainSources;
    m["GetListMainSinkSoundProperties"] = &IAmCommandReceiverShadow::getListMainSinkSoundProperties;
    m["GetListMainSourceSoundProperties"] = &IAmCommandReceiverShadow::getListMainSourceSoundProperties;
    m["GetListSourceClasses"] = &IAmCommandReceiverShadow::getListSourceClasses;
    m["GetListSinkClasses"] = &IAmCommandReceiverShadow::getListSinkClasses;
    m["GetListSystemProperties"] = &IAmCommandReceiverShadow::getListSystemProperties;
    m["GetTimingInformation"] = &IAmCommandReceiverShadow::getTimingInformation;
    m["SetSystemProperty"] = &IAmCommandReceiverShadow::setSystemProperty;
    return (m);
}

