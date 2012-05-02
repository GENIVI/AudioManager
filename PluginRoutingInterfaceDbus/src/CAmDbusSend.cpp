/**
 *  Copyright (c) copyright 2011-2012 Aricent® Group  and its licensors
 *  Copyright (c) 2012 BMW
 *
 *  \author Sampreeth Ramavana
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

#include "CAmDbusSend.h"
#include "shared/CAmDltWrapper.h"

namespace am
{

DLT_IMPORT_CONTEXT(routingDbus)

CAmRoutingDbusSend::CAmRoutingDbusSend(DBusConnection* conn, std::string bus_name, std::string path, std::string interface, std::string method) :
        mpDbusMessage(NULL), //
        mpDbusConnection(conn), //
        mDbusMessageIter(), //
        mDBusError()
{
    dbus_error_init(&mDBusError);
    mpDbusMessage = dbus_message_new_method_call(bus_name.c_str(), path.c_str(), interface.c_str(), method.c_str());
    if (NULL == mpDbusMessage)
    {
        log(&routingDbus, DLT_LOG_ERROR, "CAmRoutingDbusSend::CAmRoutingDbusSend dbus error:", mDBusError.message);
        this->~CAmRoutingDbusSend();
    }
}

CAmRoutingDbusSend::~CAmRoutingDbusSend()
{
}

void CAmRoutingDbusSend::append(std::string string)
{
    dbus_message_iter_init_append(mpDbusMessage, &mDbusMessageIter);
    if (!dbus_message_iter_append_basic(&mDbusMessageIter, DBUS_TYPE_STRING, string.c_str()))
    {
        log(&routingDbus, DLT_LOG_ERROR, "CAmRoutingDbusSend::append no more memory");
        this->~CAmRoutingDbusSend();
    }
}

void CAmRoutingDbusSend::append(uint16_t integer)
{
    dbus_message_iter_init_append(mpDbusMessage, &mDbusMessageIter);
    if (!dbus_message_iter_append_basic(&mDbusMessageIter, DBUS_TYPE_UINT16, &integer))
    {
        log(&routingDbus, DLT_LOG_ERROR, "CAmRoutingDbusSend::append no more memory");
        this->~CAmRoutingDbusSend();
    }
}

void CAmRoutingDbusSend::append(int16_t integer)
{
    dbus_message_iter_init_append(mpDbusMessage, &mDbusMessageIter);
    if (!dbus_message_iter_append_basic(&mDbusMessageIter, DBUS_TYPE_INT16, &integer))
    {
        log(&routingDbus, DLT_LOG_ERROR, "CAmRoutingDbusSend::append no more memory");
        this->~CAmRoutingDbusSend();
    }
}

void CAmRoutingDbusSend::append(std::vector<am_SoundProperty_s> listSoundProperties)
{
    DBusMessageIter arrayIter;
    DBusMessageIter structIter;
    std::vector<am_SoundProperty_s>::const_iterator listIterator = listSoundProperties.begin();
    dbus_bool_t success = true;

    success = success && dbus_message_iter_open_container(&mDbusMessageIter, DBUS_TYPE_ARRAY, "(nn)", &arrayIter);
    for (; listIterator < listSoundProperties.end(); ++listIterator)
    {
        success = success && dbus_message_iter_open_container(&arrayIter, DBUS_TYPE_STRUCT, NULL, &structIter);
        success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_INT16, &listIterator->type);
        success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_INT16, &listIterator->value);
        success = success && dbus_message_iter_close_container(&arrayIter, &structIter);
    }
    success = success && dbus_message_iter_close_container(&mDbusMessageIter, &arrayIter);

    if (!success)
    {
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::append error", mDBusError.message);
    }
}

void CAmRoutingDbusSend::append(am_SoundProperty_s soundProperty)
{
    DBusMessageIter structIter;
    dbus_bool_t success = true;

    success = success && dbus_message_iter_open_container(&mDbusMessageIter, DBUS_TYPE_STRUCT, NULL, &structIter);
    success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_INT16, &soundProperty.type);
    success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_INT16, &soundProperty.value);
    success = success && dbus_message_iter_close_container(&mDbusMessageIter, &structIter);

    if (!success)
    {
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::append error", mDBusError.message);
    }
}

am_Error_e CAmRoutingDbusSend::send()
{

    int16_t error;
    DBusMessage* reply(dbus_connection_send_with_reply_and_block(mpDbusConnection, mpDbusMessage, -1, &mDBusError));
    if (!reply)
    {
        log(&routingDbus, DLT_LOG_ERROR, "CAmRoutingDbusSend::send failed, dbus error", mDBusError.message);
        return (E_UNKNOWN);
    }
    if(!dbus_message_get_args(reply, &mDBusError, //
            DBUS_TYPE_INT16, &error, //
            DBUS_TYPE_INVALID))
        return (E_UNKNOWN);
    dbus_message_unref(reply);
    return (static_cast<am_Error_e>(error));
}
}
