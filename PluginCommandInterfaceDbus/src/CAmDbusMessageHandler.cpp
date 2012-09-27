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

#include "CAmDbusMessageHandler.h"
#include "config.h"
#include <cstdlib>
#include <cassert>
#include <vector>
#include "CAmCommandSenderDbus.h"
#include "shared/CAmDltWrapper.h"

DLT_IMPORT_CONTEXT(commandDbus)

namespace am
{

CAmDbusMessageHandler::CAmDbusMessageHandler() :
        mDBusMessageIter(), //
        mDBusError(), //
        mSerial(0), //
        mErrorName(""), //
        mErrorMsg(""), //
        mpDBusMessage(NULL), //
        mpReveiveMessage(NULL), //
        mpDBusConnection(NULL)
{
   // CAmDltWrapper::instance()->registerContext(commandDbus, "DBP", "DBus Plugin");
    log(&commandDbus, DLT_LOG_INFO, "DBusMessageHandler constructed");
}

CAmDbusMessageHandler::~CAmDbusMessageHandler()
{
    log(&commandDbus, DLT_LOG_INFO, "DBUSMessageHandler destructed");
}

void CAmDbusMessageHandler::initReceive(DBusMessage* msg)
{
    assert(msg!=NULL);
    mpReveiveMessage = msg;
    if (!dbus_message_iter_init(msg, &mDBusMessageIter))
    {
        log(&commandDbus, DLT_LOG_INFO, "DBusMessageHandler::initReceive DBus Message has no arguments!");
        mErrorName = std::string(DBUS_ERROR_INVALID_ARGS);
        mErrorMsg = "DBUS Message has no arguments!";
    }
}

void CAmDbusMessageHandler::initReply(DBusMessage* msg)
{
    assert(msg!=NULL);
    mpDBusMessage = dbus_message_new_method_return(msg);
    if (mpDBusMessage == NULL)
    {
        log(&commandDbus, DLT_LOG_ERROR, "DBusMessageHandler::initReply Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
    dbus_message_iter_init_append(mpDBusMessage, &mDBusMessageIter);
}

void CAmDbusMessageHandler::initSignal(std::string path, std::string signalName)
{
    assert(!path.empty());
    assert(!signalName.empty());
    std::string completePath = std::string(DBUS_SERVICE_OBJECT_PATH) + "/" + path;
	std::string completeInterface = std::string(DBUS_SERVICE_PREFIX) + "." + MY_NODE;
    mpDBusMessage = dbus_message_new_signal(completePath.c_str(), completeInterface.c_str(), signalName.c_str());

    if (mpDBusMessage == NULL)
    {
        log(&commandDbus, DLT_LOG_ERROR, "DBusMessageHandler::initSignal Cannot allocate DBus message!");
    }
    dbus_message_iter_init_append(mpDBusMessage, &mDBusMessageIter);
}

void CAmDbusMessageHandler::sendMessage()
{
    assert(mpDBusConnection!=NULL);
    if (mpReveiveMessage != 0)
    {
        mSerial = dbus_message_get_serial(mpReveiveMessage);
    }
    else
    {
        mSerial = 1;
    }
    if (!mErrorName.empty())
    {
        mpDBusMessage = dbus_message_new_error(mpReveiveMessage, mErrorName.c_str(), mErrorMsg.c_str());
    }
    if (!dbus_connection_send(mpDBusConnection, mpDBusMessage, &mSerial))
    {
        log(&commandDbus, DLT_LOG_ERROR, "DBusMessageHandler::sendMessage cannot send message!");
    }
    dbus_connection_flush(mpDBusConnection);
    dbus_message_unref(mpDBusMessage);
    mpDBusMessage = NULL;
}

char* CAmDbusMessageHandler::getString()
{
    char* param = NULL;

    if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&mDBusMessageIter))
    {
        log(&commandDbus, DLT_LOG_ERROR, "DBusMessageHandler::getString DBUS handler argument is no String!");
        mErrorName = std::string(DBUS_ERROR_INVALID_ARGS);
        mErrorMsg = "DBus argument is no string";
    }
    else
    {
        dbus_message_iter_get_basic(&mDBusMessageIter, &param);
        dbus_message_iter_next(&mDBusMessageIter);
    }
    return (param);
}

dbus_bool_t CAmDbusMessageHandler::getBool()
{
    dbus_bool_t boolparam = false;

    if (DBUS_TYPE_BOOLEAN != dbus_message_iter_get_arg_type(&mDBusMessageIter))
    {
        log(&commandDbus, DLT_LOG_ERROR, "DBusMessageHandler::getBool DBUS handler argument is no bool!");
        mErrorName = std::string(DBUS_ERROR_INVALID_ARGS);
        mErrorMsg = "DBus argument is no bool";
    }
    else
    {
        dbus_message_iter_get_basic(&mDBusMessageIter, &boolparam);
        dbus_message_iter_next(&mDBusMessageIter);
    }
    return (boolparam);
}

char CAmDbusMessageHandler::getByte()
{
    char param(0);

    if (DBUS_TYPE_BYTE != dbus_message_iter_get_arg_type(&mDBusMessageIter))
    {
        log(&commandDbus, DLT_LOG_ERROR, "DBusMessageHandler::getByte DBUS handler argument is no byte!");
        mErrorName = std::string(DBUS_ERROR_INVALID_ARGS);
        mErrorMsg = "DBus argument is no byte";
    }
    else
    {
        dbus_message_iter_get_basic(&mDBusMessageIter, &param);
        dbus_message_iter_next(&mDBusMessageIter);
    }
    return (param);
}

dbus_uint16_t CAmDbusMessageHandler::getUInt()
{
    dbus_uint16_t param(0);

#ifdef GLIB_DBUS_TYPES_TOLERANT
    if (DBUS_TYPE_UINT16 != dbus_message_iter_get_arg_type(&mDBusMessageIter) && DBUS_TYPE_UINT32 != dbus_message_iter_get_arg_type(&mDBusMessageIter))
#else
    if (DBUS_TYPE_UINT16 != dbus_message_iter_get_arg_type(&mDBusMessageIter))
#endif
    {
        log(&commandDbus, DLT_LOG_ERROR, "DBusMessageHandler::getUInt DBUS handler argument is no uint16_t!");
        mErrorName = std::string(DBUS_ERROR_INVALID_ARGS);
        mErrorMsg = "DBus argument is no uint16_t";
    }
    else
    {
        dbus_message_iter_get_basic(&mDBusMessageIter, &param);
        dbus_message_iter_next(&mDBusMessageIter);
    }
    return (param);
}

dbus_int16_t CAmDbusMessageHandler::getInt()
{
    dbus_int16_t param(0);

#ifdef GLIB_DBUS_TYPES_TOLERANT
    if (DBUS_TYPE_INT16 != dbus_message_iter_get_arg_type(&mDBusMessageIter) && DBUS_TYPE_INT32 != dbus_message_iter_get_arg_type(&mDBusMessageIter))
#else
    if (DBUS_TYPE_INT16 != dbus_message_iter_get_arg_type(&mDBusMessageIter))
#endif
    {
        log(&commandDbus, DLT_LOG_ERROR, "DBusMessageHandler::getInt DBUS handler argument is no int16_t!");
        mErrorName = std::string(DBUS_ERROR_INVALID_ARGS);
        mErrorMsg = "DBus argument is no int16_t";
    }
    else
    {
        dbus_message_iter_get_basic(&mDBusMessageIter, &param);
        dbus_message_iter_next(&mDBusMessageIter);
    }
    return (param);
}

double CAmDbusMessageHandler::getDouble()
{
    double param(0);
    if (DBUS_TYPE_DOUBLE != dbus_message_iter_get_arg_type(&mDBusMessageIter))
    {
        log(&commandDbus, DLT_LOG_ERROR, "DBusMessageHandler::getDouble DBUS handler argument is no double!");
        mErrorName = std::string(DBUS_ERROR_INVALID_ARGS);
        mErrorMsg = "DBus argument is no double";
    }
    else
    {
        dbus_message_iter_get_basic(&mDBusMessageIter, &param);
        dbus_message_iter_next(&mDBusMessageIter);
    }
    return (param);
}

void CAmDbusMessageHandler::getProperty(dbus_int16_t & type, dbus_int16_t & value)
{
    DBusMessageIter arrayIter;
    if (DBUS_TYPE_STRUCT != dbus_message_iter_get_arg_type(&mDBusMessageIter))
    {
        log(&commandDbus, DLT_LOG_ERROR, "DBusMessageHandler::getProperty DBUS handler argument is no array!");
        mErrorName = std::string(DBUS_ERROR_INVALID_ARGS);
        mErrorMsg = "DBus argument is no array";
    }
    else
    {
        dbus_message_iter_recurse(&mDBusMessageIter, &arrayIter);
        dbus_message_iter_get_basic(&arrayIter, &type);
        dbus_message_iter_next(&arrayIter);
        dbus_message_iter_get_basic(&arrayIter, &value);
        dbus_message_iter_next(&mDBusMessageIter);
    }
}

void CAmDbusMessageHandler::append(bool toAppend)
{
    dbus_bool_t mybool = toAppend;
    if (!dbus_message_iter_append_basic(&mDBusMessageIter, DBUS_TYPE_BOOLEAN, &mybool))
    {
        log(&commandDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmDbusMessageHandler::append(double toAppend)
{
    if (!dbus_message_iter_append_basic(&mDBusMessageIter, DBUS_TYPE_DOUBLE, &toAppend))
    {
        log(&commandDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmDbusMessageHandler::append(char toAppend)
{
    if (!dbus_message_iter_append_basic(&mDBusMessageIter, DBUS_TYPE_BYTE, &toAppend))
    {
        log(&commandDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmDbusMessageHandler::append(dbus_int16_t toAppend)
{
    if (!dbus_message_iter_append_basic(&mDBusMessageIter, DBUS_TYPE_INT16, &toAppend))
    {
        log(&commandDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmDbusMessageHandler::append(dbus_uint16_t toAppend)
{
    if (!dbus_message_iter_append_basic(&mDBusMessageIter, DBUS_TYPE_UINT16, &toAppend))
    {
        log(&commandDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmDbusMessageHandler::setDBusConnection(DBusConnection *& connection)
{
    assert(connection!=NULL);
    mpDBusConnection = connection;
}

void CAmDbusMessageHandler::append(const am::am_SinkType_s& sinkType)
{
    DBusMessageIter structIter;
    DBusMessageIter structAvailIter;
    dbus_bool_t success = true;
    success = success && dbus_message_iter_open_container(&mDBusMessageIter, DBUS_TYPE_STRUCT, NULL, &structIter);
    success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_UINT16, &sinkType.sinkID);
    success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_STRING, &sinkType.name);

    success = success && dbus_message_iter_open_container(&structIter, DBUS_TYPE_STRUCT, NULL, &structAvailIter);
    success = success && dbus_message_iter_append_basic(&structAvailIter, DBUS_TYPE_INT16, &sinkType.availability.availability);
    success = success && dbus_message_iter_append_basic(&structAvailIter, DBUS_TYPE_INT16, &sinkType.availability.availabilityReason);
    success = success && dbus_message_iter_close_container(&structIter, &structAvailIter);

    success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_INT16, &sinkType.volume);
    success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_INT16, &sinkType.muteState);
    success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_UINT16, &sinkType.sinkClassID);
    success = success && dbus_message_iter_close_container(&mDBusMessageIter, &structIter);

    if (!success)
    {
        log(&commandDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmDbusMessageHandler::append(const am::am_SourceType_s & sourceType)
{
    DBusMessageIter structIter;
    DBusMessageIter structAvailIter;
    dbus_bool_t success = true;
    success = success && dbus_message_iter_open_container(&mDBusMessageIter, DBUS_TYPE_STRUCT, NULL, &structIter);
    success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_UINT16, &sourceType.sourceID);
    success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_STRING, &sourceType.name);

    success = success && dbus_message_iter_open_container(&structIter, DBUS_TYPE_STRUCT, NULL, &structAvailIter);
    success = success && dbus_message_iter_append_basic(&structAvailIter, DBUS_TYPE_INT16, &sourceType.availability.availability);
    success = success && dbus_message_iter_append_basic(&structAvailIter, DBUS_TYPE_INT16, &sourceType.availability.availabilityReason);
    success = success && dbus_message_iter_close_container(&structIter, &structAvailIter);

    success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_UINT16, &sourceType.sourceClassID);
    success = success && dbus_message_iter_close_container(&mDBusMessageIter, &structIter);

    if (!success)
    {
        log(&commandDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmDbusMessageHandler::append(const am::am_MainSoundProperty_s mainSoundProperty)
{
    DBusMessageIter structIter;
    dbus_bool_t success = true;
    success = success && dbus_message_iter_open_container(&mDBusMessageIter, DBUS_TYPE_STRUCT, NULL, &structIter);
    success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_INT16, &mainSoundProperty.type);
    success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_INT16, &mainSoundProperty.value);
    success = success && dbus_message_iter_close_container(&mDBusMessageIter, &structIter);

    if (!success)
    {
        log(&commandDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmDbusMessageHandler::append(const am::am_Availability_s & availability)
{
    DBusMessageIter structAvailIter;
    dbus_bool_t success = true;
    success = success && dbus_message_iter_open_container(&mDBusMessageIter, DBUS_TYPE_STRUCT, NULL, &structAvailIter);
    success = success && dbus_message_iter_append_basic(&structAvailIter, DBUS_TYPE_INT16, &availability.availability);
    success = success && dbus_message_iter_append_basic(&structAvailIter, DBUS_TYPE_INT16, &availability.availabilityReason);
    success = success && dbus_message_iter_close_container(&mDBusMessageIter, &structAvailIter);

    if (!success)
    {
        log(&commandDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmDbusMessageHandler::append(const am::am_SystemProperty_s & SystemProperty)
{
    DBusMessageIter structIter;
    dbus_bool_t success = true;
    success = success && dbus_message_iter_open_container(&mDBusMessageIter, DBUS_TYPE_STRUCT, NULL, &structIter);
    success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_INT16, &SystemProperty.type);
    success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_INT16, &SystemProperty.value);
    success = success && dbus_message_iter_close_container(&mDBusMessageIter, &structIter);

    if (!success)
    {
        log(&commandDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmDbusMessageHandler::append(const std::vector<am::am_MainConnectionType_s>& listMainConnections)
{
    DBusMessageIter arrayIter;
    DBusMessageIter structIter;
    std::vector<am::am_MainConnectionType_s>::const_iterator listIterator = listMainConnections.begin();
    dbus_bool_t success = true;

    success = success && dbus_message_iter_open_container(&mDBusMessageIter, DBUS_TYPE_ARRAY, "(qqqnn)", &arrayIter);
    for (; listIterator < listMainConnections.end(); ++listIterator)
    {
        success = success && dbus_message_iter_open_container(&arrayIter, DBUS_TYPE_STRUCT, NULL, &structIter);
        success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_UINT16, &listIterator->mainConnectionID);
        success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_UINT16, &listIterator->sourceID);
        success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_UINT16, &listIterator->sinkID);
        success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_INT16, &listIterator->delay);
        success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_INT16, &listIterator->connectionState);
        success = success && dbus_message_iter_close_container(&arrayIter, &structIter);
    }
    success = success && dbus_message_iter_close_container(&mDBusMessageIter, &arrayIter);

    if (!success)
    {
        log(&commandDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmDbusMessageHandler::append(const std::vector<am::am_SinkType_s> & listMainSinks)
{
    DBusMessageIter arrayIter;
    DBusMessageIter structIter;
    DBusMessageIter availIter;
    std::vector<am::am_SinkType_s>::const_iterator listIterator = listMainSinks.begin();
    dbus_bool_t success = true;

    success = success && dbus_message_iter_open_container(&mDBusMessageIter, DBUS_TYPE_ARRAY, "(qs(nn)nnq)", &arrayIter);
    for (; listIterator < listMainSinks.end(); ++listIterator)
    {
        success = success && dbus_message_iter_open_container(&arrayIter, DBUS_TYPE_STRUCT, NULL, &structIter);
        success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_UINT16, &listIterator->sinkID);
        success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_STRING, &listIterator->name);
        success = success && dbus_message_iter_open_container(&structIter, DBUS_TYPE_STRUCT, NULL, &availIter);
        success = success && dbus_message_iter_append_basic(&availIter, DBUS_TYPE_INT16, &listIterator->availability.availability);
        success = success && dbus_message_iter_append_basic(&availIter, DBUS_TYPE_INT16, &listIterator->availability.availabilityReason);
        success = success && dbus_message_iter_close_container(&structIter, &availIter);
        success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_INT16, &listIterator->volume);
        success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_INT16, &listIterator->muteState);
        success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_UINT16, &listIterator->sinkClassID);
        success = success && dbus_message_iter_close_container(&arrayIter, &structIter);
    }
    success = success && dbus_message_iter_close_container(&mDBusMessageIter, &arrayIter);

    if (!success)
    {
        log(&commandDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmDbusMessageHandler::append(const std::vector<am::am_SourceType_s> & listMainSources)
{
    DBusMessageIter arrayIter;
    DBusMessageIter structIter;
    DBusMessageIter availIter;
    std::vector<am::am_SourceType_s>::const_iterator listIterator = listMainSources.begin();
    dbus_bool_t success = true;

    success = success && dbus_message_iter_open_container(&mDBusMessageIter, DBUS_TYPE_ARRAY, "(qs(nn)q)", &arrayIter);
    for (; listIterator < listMainSources.end(); ++listIterator)
    {
        success = success && dbus_message_iter_open_container(&arrayIter, DBUS_TYPE_STRUCT, NULL, &structIter);
        success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_UINT16, &listIterator->sourceID);
        success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_STRING, &listIterator->name);
        success = success && dbus_message_iter_open_container(&structIter, DBUS_TYPE_STRUCT, NULL, &availIter);
        success = success && dbus_message_iter_append_basic(&availIter, DBUS_TYPE_INT16, &listIterator->availability.availability);
        success = success && dbus_message_iter_append_basic(&availIter, DBUS_TYPE_INT16, &listIterator->availability.availabilityReason);
        success = success && dbus_message_iter_close_container(&structIter, &availIter);
        success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_UINT16, &listIterator->sourceClassID);
        success = success && dbus_message_iter_close_container(&arrayIter, &structIter);
    }
    success = success && dbus_message_iter_close_container(&mDBusMessageIter, &arrayIter);

    if (!success)
    {
        log(&commandDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmDbusMessageHandler::append(const std::vector<am::am_MainSoundProperty_s> & listMainSoundProperties)
{
    DBusMessageIter arrayIter;
    DBusMessageIter structIter;
    std::vector<am::am_MainSoundProperty_s>::const_iterator listIterator = listMainSoundProperties.begin();
    dbus_bool_t success = true;

    success = success && dbus_message_iter_open_container(&mDBusMessageIter, DBUS_TYPE_ARRAY, "(nn)", &arrayIter);
    for (; listIterator < listMainSoundProperties.end(); ++listIterator)
    {
        success = success && dbus_message_iter_open_container(&arrayIter, DBUS_TYPE_STRUCT, NULL, &structIter);
        success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_INT16, &listIterator->type);
        success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_INT16, &listIterator->value);
        success = success && dbus_message_iter_close_container(&arrayIter, &structIter);
    }
    success = success && dbus_message_iter_close_container(&mDBusMessageIter, &arrayIter);

    if (!success)
    {
        log(&commandDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmDbusMessageHandler::append(const std::vector<am::am_SourceClass_s> & listSourceClasses)
{
    DBusMessageIter arrayIter;
    DBusMessageIter structIter;
    DBusMessageIter propIter;
    DBusMessageIter innerIter;
    std::vector<am::am_SourceClass_s>::const_iterator listIterator = listSourceClasses.begin();
    dbus_bool_t success = true;

    success = success && dbus_message_iter_open_container(&mDBusMessageIter, DBUS_TYPE_ARRAY, "(qsa(nn))", &arrayIter);
    for (; listIterator < listSourceClasses.end(); ++listIterator)
    {
        success = success && dbus_message_iter_open_container(&arrayIter, DBUS_TYPE_STRUCT, NULL, &structIter);
        success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_UINT16, &listIterator->sourceClassID);
        success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_STRING, &listIterator->name);
        success = success && dbus_message_iter_open_container(&structIter, DBUS_TYPE_ARRAY, "(nn)", &innerIter);

        std::vector<am::am_ClassProperty_s>::const_iterator listInnerIterator = listIterator->listClassProperties.begin();
        for (; listInnerIterator < listIterator->listClassProperties.end(); ++listInnerIterator)
        {
            success = success && dbus_message_iter_open_container(&innerIter, DBUS_TYPE_STRUCT, NULL, &propIter);
            success = success && dbus_message_iter_append_basic(&propIter, DBUS_TYPE_INT16, &listInnerIterator->classProperty);
            success = success && dbus_message_iter_append_basic(&propIter, DBUS_TYPE_INT16, &listInnerIterator->value);
            success = success && dbus_message_iter_close_container(&innerIter, &propIter);
        }
        success = success && dbus_message_iter_close_container(&structIter, &innerIter);
        success = success && dbus_message_iter_close_container(&arrayIter, &structIter);
    }
    success = success && dbus_message_iter_close_container(&mDBusMessageIter, &arrayIter);

    if (!success)
    {
        log(&commandDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmDbusMessageHandler::append(const std::vector<am::am_SinkClass_s> & listSinkClasses)
{
    DBusMessageIter arrayIter;
    DBusMessageIter structIter;
    DBusMessageIter propIter;
    DBusMessageIter innerIter;
    std::vector<am::am_SinkClass_s>::const_iterator listIterator = listSinkClasses.begin();
    dbus_bool_t success = true;

    success = success && dbus_message_iter_open_container(&mDBusMessageIter, DBUS_TYPE_ARRAY, "(qsa(nn))", &arrayIter);
    for (; listIterator < listSinkClasses.end(); ++listIterator)
    {
        success = success && dbus_message_iter_open_container(&arrayIter, DBUS_TYPE_STRUCT, NULL, &structIter);
        success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_UINT16, &listIterator->sinkClassID);
        success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_STRING, &listIterator->name);
        success = success && dbus_message_iter_open_container(&structIter, DBUS_TYPE_ARRAY, "(nn)", &innerIter);

        std::vector<am::am_ClassProperty_s>::const_iterator listInnerIterator = listIterator->listClassProperties.begin();
        for (; listInnerIterator < listIterator->listClassProperties.end(); ++listInnerIterator)
        {
            success = success && dbus_message_iter_open_container(&innerIter, DBUS_TYPE_STRUCT, NULL, &propIter);
            success = success && dbus_message_iter_append_basic(&propIter, DBUS_TYPE_INT16, &listInnerIterator->classProperty);
            success = success && dbus_message_iter_append_basic(&propIter, DBUS_TYPE_INT16, &listInnerIterator->value);
            success = success && dbus_message_iter_close_container(&innerIter, &propIter);
        }
        success = success && dbus_message_iter_close_container(&structIter, &innerIter);
        success = success && dbus_message_iter_close_container(&arrayIter, &structIter);
    }
    success = success && dbus_message_iter_close_container(&mDBusMessageIter, &arrayIter);

    if (!success)
    {
        log(&commandDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmDbusMessageHandler::append(const std::vector<am::am_SystemProperty_s> & listSystemProperties)
{
    DBusMessageIter arrayIter;
    DBusMessageIter structIter;
    std::vector<am::am_SystemProperty_s>::const_iterator listIterator = listSystemProperties.begin();
    dbus_bool_t success = true;

    success = success && dbus_message_iter_open_container(&mDBusMessageIter, DBUS_TYPE_ARRAY, "(nn)", &arrayIter);
    for (; listIterator < listSystemProperties.end(); ++listIterator)
    {
        success = success && dbus_message_iter_open_container(&arrayIter, DBUS_TYPE_STRUCT, NULL, &structIter);
        success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_INT16, &listIterator->type);
        success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_INT16, &listIterator->value);
        success = success && dbus_message_iter_close_container(&arrayIter, &structIter);
    }
    success = success && dbus_message_iter_close_container(&mDBusMessageIter, &arrayIter);

    if (!success)
    {
        log(&commandDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}
}
