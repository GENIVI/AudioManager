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

#include "CAmDbusMessageHandler.h"
#include "config.h"
#include <cstdlib>
#include <cassert>
#include <vector>
#include "CAmRoutingSenderDbus.h"
#include "shared/CAmDltWrapper.h"

namespace am
{

DLT_IMPORT_CONTEXT(routingDbus)

CAmRoutingDbusMessageHandler::CAmRoutingDbusMessageHandler() :
        mDBusMessageIter(), //
        mDBusError(), //
        mSerial(0), //
        mErrorName(""), //
        mErrorMsg(""), //
        mpDBusMessage(NULL), //
        mpReveiveMessage(NULL), //
        mpDBusConnection(NULL)
{
    //CAmDltWrapper::instance()->registerContext(routingDbus, "DRS", "DBus Plugin");
    log(&routingDbus, DLT_LOG_INFO, "DBusMessageHandler constructed");
}

CAmRoutingDbusMessageHandler::~CAmRoutingDbusMessageHandler()
{
    log(&routingDbus, DLT_LOG_INFO, "DBUSMessageHandler destructed");
}

void CAmRoutingDbusMessageHandler::initReceive(DBusMessage* msg)
{
    assert(msg!=NULL);
    mpReveiveMessage = msg;
    if (!dbus_message_iter_init(msg, &mDBusMessageIter))
    {
        log(&routingDbus, DLT_LOG_INFO, "DBusMessageHandler::initReceive DBus Message has no arguments!");
        mErrorName = std::string(DBUS_ERROR_INVALID_ARGS);
        mErrorMsg = "DBUS Message has no arguments!";
    }
}

void CAmRoutingDbusMessageHandler::initReply(DBusMessage* msg)
{
    assert(msg!=NULL);
    mpDBusMessage = dbus_message_new_method_return(msg);
    if (mpDBusMessage == NULL)
    {
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::initReply Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
    dbus_message_iter_init_append(mpDBusMessage, &mDBusMessageIter);
}

void CAmRoutingDbusMessageHandler::initSignal(std::string path, std::string signalName)
{
    assert(!path.empty());
    assert(!signalName.empty());
    std::string completePath = std::string(DBUS_SERVICE_OBJECT_PATH) + "/" + path;
	std::string completeInterface = std::string(DBUS_SERVICE_PREFIX) + "." + ROUTING_NODE;
    mpDBusMessage = dbus_message_new_signal(completePath.c_str(), completeInterface.c_str(), signalName.c_str());

    if (mpDBusMessage == NULL)
    {
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::initSignal Cannot allocate DBus message!");
    }
    dbus_message_iter_init_append(mpDBusMessage, &mDBusMessageIter);
}

void CAmRoutingDbusMessageHandler::sendMessage()
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
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::sendMessage cannot send message!");
    }
    dbus_connection_flush(mpDBusConnection);
    dbus_message_unref(mpDBusMessage);
    mpDBusMessage = NULL;
}

char* CAmRoutingDbusMessageHandler::getString(DBusMessageIter& iter, bool next)
{
    char* param = NULL;

    if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&iter))
    {
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::getString DBUS handler argument is no String!");
        mErrorName = std::string(DBUS_ERROR_INVALID_ARGS);
        mErrorMsg = "DBus argument is no string";
    }
    else
    {
        dbus_message_iter_get_basic(&iter, &param);
        if (next)
            dbus_message_iter_next(&iter);
    }
    return (param);
}

char* CAmRoutingDbusMessageHandler::getString()
{
    return (getString(mDBusMessageIter, true));
}

dbus_bool_t CAmRoutingDbusMessageHandler::getBool(DBusMessageIter& iter, bool next)
{
    dbus_bool_t boolparam = false;

    if (DBUS_TYPE_BOOLEAN != dbus_message_iter_get_arg_type(&iter))
    {
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::getBool DBUS handler argument is no bool!");
        mErrorName = std::string(DBUS_ERROR_INVALID_ARGS);
        mErrorMsg = "DBus argument is no bool";
    }
    else
    {
        dbus_message_iter_get_basic(&iter, &boolparam);
        if (next)
            dbus_message_iter_next(&iter);
    }
    return (boolparam);
}

dbus_bool_t CAmRoutingDbusMessageHandler::getBool()
{
    return (getBool(mDBusMessageIter, true));
}

char CAmRoutingDbusMessageHandler::getByte(DBusMessageIter& iter, bool next)
{
    char param(0);

    if (DBUS_TYPE_BYTE != dbus_message_iter_get_arg_type(&iter))
    {
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::getByte DBUS handler argument is no byte!");
        mErrorName = std::string(DBUS_ERROR_INVALID_ARGS);
        mErrorMsg = "DBus argument is no byte";
    }
    else
    {
        dbus_message_iter_get_basic(&iter, &param);
        if (next)
            dbus_message_iter_next(&iter);
    }
    return (param);
}

char CAmRoutingDbusMessageHandler::getByte()
{
    return (getByte(mDBusMessageIter, true));
}

dbus_uint16_t CAmRoutingDbusMessageHandler::getUInt(DBusMessageIter& iter, bool next)
{
    dbus_uint16_t param(0);
#ifdef GLIB_DBUS_TYPES_TOLERANT
    if (DBUS_TYPE_UINT16 != dbus_message_iter_get_arg_type(&iter) && DBUS_TYPE_UINT32 != dbus_message_iter_get_arg_type(&iter))
#else
    if (DBUS_TYPE_UINT16 != dbus_message_iter_get_arg_type(&iter))
#endif
    {
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::getUInt DBUS handler argument is no uint16_t!");
        mErrorName = std::string(DBUS_ERROR_INVALID_ARGS);
        mErrorMsg = "DBus argument is no uint16_t";
    }
    else
    {
        dbus_message_iter_get_basic(&iter, &param);
        if (next)
            dbus_message_iter_next(&iter);
    }
    return (param);
}

dbus_uint16_t CAmRoutingDbusMessageHandler::getUInt()
{
    return (getUInt(mDBusMessageIter, true));
}

dbus_int16_t CAmRoutingDbusMessageHandler::getInt(DBusMessageIter& iter, bool next)
{
    dbus_int16_t param(0);
#ifdef GLIB_DBUS_TYPES_TOLERANT
    if (DBUS_TYPE_INT16 != dbus_message_iter_get_arg_type(&iter) && DBUS_TYPE_INT32 != dbus_message_iter_get_arg_type(&iter))
#else
    if (DBUS_TYPE_INT16 != dbus_message_iter_get_arg_type(&iter))
#endif
    {
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::getInt DBUS handler argument is no int16_t!");
        mErrorName = std::string(DBUS_ERROR_INVALID_ARGS);
        mErrorMsg = "DBus argument is no int16_t";
    }
    else
    {
        dbus_message_iter_get_basic(&iter, &param);
        if (next)
            dbus_message_iter_next(&iter);
    }
    return (param);
}

dbus_int16_t CAmRoutingDbusMessageHandler::getInt()
{
    return (getInt(mDBusMessageIter, true));
}

double CAmRoutingDbusMessageHandler::getDouble(DBusMessageIter& iter, bool next)
{
    double param(0);
    if (DBUS_TYPE_DOUBLE != dbus_message_iter_get_arg_type(&iter))
    {
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::getDouble DBUS handler argument is no double!");
        mErrorName = std::string(DBUS_ERROR_INVALID_ARGS);
        mErrorMsg = "DBus argument is no double";
    }
    else
    {
        dbus_message_iter_get_basic(&iter, &param);
        if (next)
            dbus_message_iter_next(&iter);
    }
    return (param);
}

double CAmRoutingDbusMessageHandler::getDouble()
{
    return (getDouble(mDBusMessageIter, true));
}

am_Availability_s CAmRoutingDbusMessageHandler::getAvailability()
{
    am::am_Availability_s availability;
    availability.availability=A_UNKNOWN;
    availability.availabilityReason=AR_UNKNOWN;
    if (DBUS_TYPE_STRUCT != dbus_message_iter_get_arg_type(&mDBusMessageIter))
    {
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::getAvailability DBUS handler argument is no struct!");
        mErrorName = std::string(DBUS_ERROR_INVALID_ARGS);
        mErrorMsg = "DBus argument is no struct";
    }
    else
    {
        DBusMessageIter structIter;
        dbus_message_iter_recurse(&mDBusMessageIter, &structIter);
        availability.availability = static_cast<am_Availability_e>(getInt(structIter, true));
        availability.availabilityReason = static_cast<am_CustomAvailabilityReason_t>(getInt(structIter, false));
        dbus_message_iter_next(&mDBusMessageIter);
    }
    return (availability);
}

std::vector<am_EarlyData_s> CAmRoutingDbusMessageHandler::getEarlyData()
{
    std::vector<am_EarlyData_s> listEarlyData;
    if (DBUS_TYPE_ARRAY != dbus_message_iter_get_arg_type(&mDBusMessageIter))
    {
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::getProperty DBUS handler argument is no array!");
        mErrorName = std::string(DBUS_ERROR_INVALID_ARGS);
        mErrorMsg = "DBus argument is no array";
    }
    else
    {
        DBusMessageIter arrayIter, structIter, soundpropIter;
        am_EarlyData_s earlyData;

        //first the volume array
        dbus_message_iter_recurse(&mDBusMessageIter, &arrayIter);
        do
        {
            dbus_message_iter_recurse(&arrayIter, &structIter);
            earlyData.type = static_cast<am_EarlyDataType_e>(getUInt(structIter, true));
            if (earlyData.type==ED_SINK_VOLUME)
                earlyData.sinksource.sink = static_cast<am_sinkID_t>(getUInt(structIter, true));
            else
                earlyData.sinksource.source = static_cast<am_sourceID_t>(getUInt(structIter, true));
            earlyData.data.volume = static_cast<am_volume_t>(getInt(structIter, false));
            listEarlyData.push_back(earlyData);
        } while (dbus_message_iter_next(&arrayIter));
        dbus_message_iter_next(&mDBusMessageIter);

        //then the soundproperty array
        dbus_message_iter_recurse(&mDBusMessageIter, &arrayIter);
        do
        {
            dbus_message_iter_recurse(&arrayIter, &structIter);
            earlyData.type = static_cast<am_EarlyDataType_e>(getUInt(structIter, true));
            if (earlyData.type==ED_SINK_PROPERTY)
                earlyData.sinksource.sink = static_cast<am_sinkID_t>(getUInt(structIter, true));
            else
                earlyData.sinksource.source = static_cast<am_sourceID_t>(getUInt(structIter, true));
            dbus_message_iter_recurse(&structIter, &soundpropIter);
            earlyData.data.soundProperty.type = static_cast<am_CustomSoundPropertyType_t>(getInt(soundpropIter, true));
            earlyData.data.soundProperty.value = (getInt(soundpropIter, false));
            listEarlyData.push_back(earlyData);
        } while (dbus_message_iter_next(&arrayIter));
    }
    return (listEarlyData);
}

am_Domain_s CAmRoutingDbusMessageHandler::getDomainData()
{
    am_Domain_s domainData;
    DBusMessageIter domainDataIter;

    if (DBUS_TYPE_STRUCT != dbus_message_iter_get_arg_type(&mDBusMessageIter))
    {
        log(&routingDbus, DLT_LOG_ERROR, "CAmRoutingDbusMessageHandler::getDomainData DBUS handler argument is no struct!");
        mErrorName = std::string(DBUS_ERROR_INVALID_ARGS);
        mErrorMsg = "DBus argument is no struct";
    }
    else
    {
        dbus_message_iter_recurse(&mDBusMessageIter, &domainDataIter);
	domainData.domainID = static_cast<am_domainID_t>(getUInt(domainDataIter, true));
	domainData.name = getString(domainDataIter, true);
	domainData.busname = getString(domainDataIter, true);
	domainData.nodename = getString(domainDataIter, true);
	domainData.early = getBool(domainDataIter, true);
	domainData.complete = getBool(domainDataIter, true);
	domainData.state = static_cast<am_DomainState_e>(getInt(domainDataIter, false));
        dbus_message_iter_next(&mDBusMessageIter);
    }
    return (domainData);
}

am_Source_s CAmRoutingDbusMessageHandler::getSourceData()
{
    am_Source_s sourceData;
    DBusMessageIter sourceDataIter, availIter, arrayIter, structIter;
    am_SoundProperty_s soundProperty;
    am_CustomAvailabilityReason_t connectionFormat;
    am_MainSoundProperty_s mainSoundProperty;
    am_NotificationConfiguration_s notificationConfiguration;
    am_NotificationConfiguration_s MainnotificationConfiguration;

    if (DBUS_TYPE_STRUCT != dbus_message_iter_get_arg_type(&mDBusMessageIter))
    {
        log(&routingDbus, DLT_LOG_ERROR, "CAmRoutingDbusMessageHandler::getSourceData DBUS handler argument is no struct!");
        mErrorName = std::string(DBUS_ERROR_INVALID_ARGS);
        mErrorMsg = "DBus argument is no struct";
    }
    else
    {
        dbus_message_iter_recurse(&mDBusMessageIter, &sourceDataIter);
	sourceData.sourceID = static_cast<am_sourceID_t>(getUInt(sourceDataIter, true));
	sourceData.domainID = static_cast<am_domainID_t>(getUInt(sourceDataIter, true));
	sourceData.name = getString(sourceDataIter, true);
	sourceData.sourceClassID = static_cast<am_sourceClass_t>(getUInt(sourceDataIter, true));
	sourceData.sourceState = static_cast<am_SourceState_e>(getInt32(sourceDataIter, true));
	sourceData.volume = static_cast<am_volume_t>(getInt(sourceDataIter, true));
	sourceData.visible = getBool(sourceDataIter, true);
	dbus_message_iter_recurse(&sourceDataIter, &availIter);
	sourceData.available.availability = static_cast<am_Availability_e>(getInt32(availIter, true));
	sourceData.available.availabilityReason = static_cast<am_CustomAvailabilityReason_t>(getInt32(availIter, false));
	dbus_message_iter_next(&sourceDataIter);
	sourceData.interruptState = static_cast<am_InterruptState_e>(getUInt(sourceDataIter, true));
	dbus_message_iter_recurse(&sourceDataIter, &arrayIter);
	do
	{
	    dbus_message_iter_recurse(&arrayIter, &structIter);
	    soundProperty.type = static_cast<am_CustomSoundPropertyType_t>(getInt32(structIter, true));
	    soundProperty.value = static_cast<int16_t>(getInt(structIter, false));
	    sourceData.listSoundProperties.push_back(soundProperty);
	} while (dbus_message_iter_next(&arrayIter));
	dbus_message_iter_next(&sourceDataIter);
	dbus_message_iter_recurse(&sourceDataIter, &arrayIter);
	do
	{
	    connectionFormat = static_cast<am_CustomAvailabilityReason_t>(getInt32(arrayIter, false));
	    sourceData.listConnectionFormats.push_back(connectionFormat);
	} while (dbus_message_iter_next(&arrayIter));
	dbus_message_iter_next(&sourceDataIter);

	dbus_message_iter_recurse(&sourceDataIter, &arrayIter);
	do
	{
	    dbus_message_iter_recurse(&arrayIter, &structIter);
	    mainSoundProperty.type = static_cast<am_CustomMainSoundPropertyType_t>(getInt32(structIter, true));
	    mainSoundProperty.value = static_cast<int16_t>(getInt(structIter, false));
	    sourceData.listMainSoundProperties.push_back(mainSoundProperty);
	} while (dbus_message_iter_next(&arrayIter));
        dbus_message_iter_next(&sourceDataIter);

    dbus_message_iter_recurse(&sourceDataIter, &arrayIter);
    do
    {
        dbus_message_iter_recurse(&arrayIter, &structIter);
        MainnotificationConfiguration.type = static_cast<am_CustomNotificationType_t>(getInt32(structIter, true));
        MainnotificationConfiguration.parameter = static_cast<int16_t>(getInt32(structIter, false));
        MainnotificationConfiguration.status = static_cast<am_NotificationStatus_e>(getInt(structIter, true));
        sourceData.listMainNotificationConfigurations.push_back(MainnotificationConfiguration);
    } while (dbus_message_iter_next(&arrayIter));
        dbus_message_iter_next(&sourceDataIter);

    dbus_message_iter_recurse(&sourceDataIter, &arrayIter);
    do
    {
        dbus_message_iter_recurse(&arrayIter, &structIter);
        notificationConfiguration.type = static_cast<am_CustomNotificationType_t>(getInt32(structIter, true));
        notificationConfiguration.parameter = static_cast<int16_t>(getInt32(structIter, false));
        notificationConfiguration.status = static_cast<am_NotificationStatus_e>(getInt(structIter, true));
        sourceData.listNotificationConfigurations.push_back(notificationConfiguration);
    } while (dbus_message_iter_next(&arrayIter));
        dbus_message_iter_next(&mDBusMessageIter);
    }
    return (sourceData);
}

am_Sink_s CAmRoutingDbusMessageHandler::getSinkData()
{
    am_Sink_s sinkData;
    DBusMessageIter sinkDataIter, structIter, availIter, arrayIter;
    am_SoundProperty_s soundProperty;
    am_CustomAvailabilityReason_t connectionFormat;
    am_MainSoundProperty_s mainSoundProperty;
    am_NotificationConfiguration_s notificationConfiguration;
    am_NotificationConfiguration_s MainnotificationConfiguration;

    if (DBUS_TYPE_STRUCT != dbus_message_iter_get_arg_type(&mDBusMessageIter))
    {
        log(&routingDbus, DLT_LOG_ERROR, "CAmRoutingDbusMessageHandler::getSinkData DBUS handler argument is no struct!");
        mErrorName = std::string(DBUS_ERROR_INVALID_ARGS);
        mErrorMsg = "DBus argument is no struct";
    }
    else
    {
        dbus_message_iter_recurse(&mDBusMessageIter, &sinkDataIter);
	sinkData.sinkID = static_cast<am_sinkID_t>(getUInt(sinkDataIter, true));
	sinkData.name = getString(sinkDataIter, true);
	sinkData.domainID = static_cast<am_domainID_t>(getUInt(sinkDataIter, true));
	sinkData.sinkClassID = static_cast<am_sinkClass_t>(getInt32(sinkDataIter, true));
	sinkData.volume = static_cast<am_volume_t>(getInt(sinkDataIter, true));
	sinkData.visible = getBool(sinkDataIter, true);
	dbus_message_iter_recurse(&sinkDataIter, &availIter);
	sinkData.available.availability = static_cast<am_Availability_e>(getInt32(availIter, true));
	sinkData.available.availabilityReason = static_cast<am_CustomAvailabilityReason_t>(getInt32(availIter, false));
	dbus_message_iter_next(&sinkDataIter);
	sinkData.muteState = static_cast<am_MuteState_e>(getInt(sinkDataIter, true));
	sinkData.mainVolume = static_cast<am_mainVolume_t>(getInt(sinkDataIter, true));

	dbus_message_iter_recurse(&sinkDataIter, &arrayIter);
	do
	{
	    dbus_message_iter_recurse(&arrayIter, &structIter);
	    soundProperty.type = static_cast<am_CustomSoundPropertyType_t>(getInt32(structIter, true));
	    soundProperty.value = static_cast<int16_t>(getInt(structIter, false));
	    sinkData.listSoundProperties.push_back(soundProperty);
	} while (dbus_message_iter_next(&arrayIter));

	dbus_message_iter_next(&sinkDataIter);

	dbus_message_iter_recurse(&sinkDataIter, &arrayIter);
	do
	{
	    connectionFormat = static_cast<am_CustomAvailabilityReason_t>(getInt32(arrayIter, false));
	    sinkData.listConnectionFormats.push_back(connectionFormat);
	} while (dbus_message_iter_next(&arrayIter));
	dbus_message_iter_next(&sinkDataIter);

    dbus_message_iter_recurse(&sinkDataIter, &arrayIter);
    do
    {
        dbus_message_iter_recurse(&arrayIter, &structIter);
        mainSoundProperty.type = static_cast<am_CustomMainSoundPropertyType_t>(getInt32(structIter, true));
        mainSoundProperty.value = static_cast<int16_t>(getInt(structIter, false));
        sinkData.listMainSoundProperties.push_back(mainSoundProperty);
    } while (dbus_message_iter_next(&arrayIter));
        dbus_message_iter_next(&sinkDataIter);

    dbus_message_iter_recurse(&sinkDataIter, &arrayIter);
    do
    {
        dbus_message_iter_recurse(&arrayIter, &structIter);
        MainnotificationConfiguration.type = static_cast<am_CustomNotificationType_t>(getInt32(structIter, true));
        MainnotificationConfiguration.parameter = static_cast<int16_t>(getInt32(structIter, false));
        MainnotificationConfiguration.status = static_cast<am_NotificationStatus_e>(getInt(structIter, true));
        sinkData.listMainNotificationConfigurations.push_back(MainnotificationConfiguration);
    } while (dbus_message_iter_next(&arrayIter));
        dbus_message_iter_next(&sinkDataIter);

    dbus_message_iter_recurse(&sinkDataIter, &arrayIter);
    do
    {
        dbus_message_iter_recurse(&arrayIter, &structIter);
        notificationConfiguration.type = static_cast<am_CustomNotificationType_t>(getInt32(structIter, true));
        notificationConfiguration.parameter = static_cast<int16_t>(getInt32(structIter, false));
        notificationConfiguration.status = static_cast<am_NotificationStatus_e>(getInt(structIter, true));
        sinkData.listNotificationConfigurations.push_back(notificationConfiguration);
    } while (dbus_message_iter_next(&arrayIter));
        dbus_message_iter_next(&mDBusMessageIter);
    }
    return (sinkData);
}

am_Gateway_s CAmRoutingDbusMessageHandler::getGatewayData()
{
    am_Gateway_s gatewayData;
    DBusMessageIter gatewayDataIter, arrayIter;
    am_CustomAvailabilityReason_t connectionFormat;
    bool convertion;
    if (DBUS_TYPE_STRUCT != dbus_message_iter_get_arg_type(&mDBusMessageIter))
    {
        log(&routingDbus, DLT_LOG_ERROR, "CAmRoutingDbusMessageHandler::getDomainData DBUS handler argument is no struct!");
        mErrorName = std::string(DBUS_ERROR_INVALID_ARGS);
        mErrorMsg = "DBus argument is no struct";
    }
    else
    {
        dbus_message_iter_recurse(&mDBusMessageIter, &gatewayDataIter);
	gatewayData.gatewayID = static_cast<am_gatewayID_t>(getUInt(gatewayDataIter, true));
	gatewayData.name = getString(gatewayDataIter, true);
	gatewayData.sinkID = static_cast<am_sinkID_t>(getUInt(gatewayDataIter, true));
	gatewayData.sourceID = static_cast<am_sourceID_t>(getUInt(gatewayDataIter, true));
	gatewayData.domainSinkID = static_cast<am_domainID_t>(getUInt(gatewayDataIter, true));
	gatewayData.domainSourceID = static_cast<am_domainID_t>(getUInt(gatewayDataIter, true));
	gatewayData.controlDomainID = static_cast<am_domainID_t>(getUInt(gatewayDataIter, true));
    dbus_message_iter_recurse(&gatewayDataIter, &arrayIter);
	do
	{
	    connectionFormat = static_cast<am_CustomAvailabilityReason_t>(getInt32(arrayIter, false));
	    gatewayData.listSourceFormats.push_back(connectionFormat);
	} while (dbus_message_iter_next(&arrayIter));
	dbus_message_iter_next(&gatewayDataIter);
    dbus_message_iter_recurse(&gatewayDataIter, &arrayIter);
	do
	{
	    connectionFormat = static_cast<am_CustomAvailabilityReason_t>(getInt32(arrayIter, false));
	    gatewayData.listSinkFormats.push_back(connectionFormat);
	} while (dbus_message_iter_next(&arrayIter));
	dbus_message_iter_next(&gatewayDataIter);
    dbus_message_iter_recurse(&gatewayDataIter, &arrayIter);
	do
	{
	    convertion = getBool(arrayIter, false);
	    gatewayData.convertionMatrix.push_back(convertion);
	} while (dbus_message_iter_next(&arrayIter));
        dbus_message_iter_next(&mDBusMessageIter);
    }
    return (gatewayData);
}

am_Crossfader_s CAmRoutingDbusMessageHandler::getCrossfaderData()
{
    am_Crossfader_s crossfaderData;
    DBusMessageIter crossfaderDataIter;

    if (DBUS_TYPE_STRUCT != dbus_message_iter_get_arg_type(&mDBusMessageIter))
    {
        log(&routingDbus, DLT_LOG_ERROR, "CAmRoutingDbusMessageHandler::getCrossfaderData DBUS handler argument is no struct!");
        mErrorName = std::string(DBUS_ERROR_INVALID_ARGS);
        mErrorMsg = "DBus argument is no struct";
    }
    else
    {
        dbus_message_iter_recurse(&mDBusMessageIter, &crossfaderDataIter);
	crossfaderData.crossfaderID = static_cast<am_crossfaderID_t> (getUInt(crossfaderDataIter, true));
	crossfaderData.name = static_cast<std::string> (getString(crossfaderDataIter, true));
	crossfaderData.sinkID_A = static_cast<am_sinkID_t> (getUInt(crossfaderDataIter, true));
	crossfaderData.sinkID_B = static_cast<am_sinkID_t> (getUInt(crossfaderDataIter, true));
	crossfaderData.sourceID = static_cast<am_sourceID_t> (getUInt(crossfaderDataIter, true));
	crossfaderData.hotSink = static_cast<am_HotSink_e> (getUInt(crossfaderDataIter, false));
        dbus_message_iter_next(&mDBusMessageIter);
    }
    return (crossfaderData);
}

am_MainSoundProperty_s CAmRoutingDbusMessageHandler::getMainSoundProperty()
{
    am_MainSoundProperty_s mainSoundProperty;
    DBusMessageIter structIter;

    dbus_message_iter_recurse(&mDBusMessageIter, &structIter);
    mainSoundProperty.type = static_cast<am_CustomMainSoundPropertyType_t>(getInt(structIter, true));
    mainSoundProperty.value = static_cast<int16_t>(getInt(structIter, false));
    dbus_message_iter_next(&mDBusMessageIter);

    return (mainSoundProperty);
}

void CAmRoutingDbusMessageHandler::append(bool toAppend)
{
    dbus_bool_t mybool = toAppend;
    if (!dbus_message_iter_append_basic(&mDBusMessageIter, DBUS_TYPE_BOOLEAN, &mybool))
    {
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmRoutingDbusMessageHandler::append(double toAppend)
{
    if (!dbus_message_iter_append_basic(&mDBusMessageIter, DBUS_TYPE_DOUBLE, &toAppend))
    {
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmRoutingDbusMessageHandler::append(char toAppend)
{
    if (!dbus_message_iter_append_basic(&mDBusMessageIter, DBUS_TYPE_BYTE, &toAppend))
    {
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmRoutingDbusMessageHandler::append(dbus_int16_t toAppend)
{
    if (!dbus_message_iter_append_basic(&mDBusMessageIter, DBUS_TYPE_INT16, &toAppend))
    {
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmRoutingDbusMessageHandler::append(dbus_uint16_t toAppend)
{
    if (!dbus_message_iter_append_basic(&mDBusMessageIter, DBUS_TYPE_UINT16, &toAppend))
    {
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmRoutingDbusMessageHandler::setDBusConnection(DBusConnection*& connection)
{
    assert(connection != NULL);
    mpDBusConnection = connection;
}

void CAmRoutingDbusMessageHandler::append(const am::am_SinkType_s& sinkType)
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
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmRoutingDbusMessageHandler::append(const am::am_SourceType_s& sourceType)
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
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmRoutingDbusMessageHandler::append(const am::am_MainSoundProperty_s mainSoundProperty)
{
    DBusMessageIter structIter;
    dbus_bool_t success = true;
    success = success && dbus_message_iter_open_container(&mDBusMessageIter, DBUS_TYPE_STRUCT, NULL, &structIter);
    success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_INT16, &mainSoundProperty.type);
    success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_INT16, &mainSoundProperty.value);
    success = success && dbus_message_iter_close_container(&mDBusMessageIter, &structIter);
    if (!success)
    {
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmRoutingDbusMessageHandler::append(const am::am_Availability_s& availability)
{
    DBusMessageIter structAvailIter;
    dbus_bool_t success = true;
    success = success && dbus_message_iter_open_container(&mDBusMessageIter, DBUS_TYPE_STRUCT, NULL, &structAvailIter);
    success = success && dbus_message_iter_append_basic(&structAvailIter, DBUS_TYPE_INT16, &availability.availability);
    success = success && dbus_message_iter_append_basic(&structAvailIter, DBUS_TYPE_INT16, &availability.availabilityReason);
    success = success && dbus_message_iter_close_container(&mDBusMessageIter, &structAvailIter);
    if (!success)
    {
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmRoutingDbusMessageHandler::append(const am::am_SystemProperty_s& SystemProperty)
{
    DBusMessageIter structIter;
    dbus_bool_t success = true;
    success = success && dbus_message_iter_open_container(&mDBusMessageIter, DBUS_TYPE_STRUCT, NULL, &structIter);
    success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_INT16, &SystemProperty.type);
    success = success && dbus_message_iter_append_basic(&structIter, DBUS_TYPE_INT16, &SystemProperty.value);
    success = success && dbus_message_iter_close_container(&mDBusMessageIter, &structIter);
    if (!success)
    {
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmRoutingDbusMessageHandler::append(const std::vector<am::am_MainConnectionType_s>& listMainConnections)
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
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmRoutingDbusMessageHandler::append(const std::vector<am::am_SinkType_s>& listMainSinks)
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
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmRoutingDbusMessageHandler::append(const std::vector<am::am_SourceType_s>& listMainSources)
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
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmRoutingDbusMessageHandler::append(const std::vector<am::am_MainSoundProperty_s>& listMainSoundProperties)
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
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmRoutingDbusMessageHandler::append(const std::vector<am::am_SourceClass_s>& listSourceClasses)
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
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmRoutingDbusMessageHandler::append(const std::vector<am::am_SinkClass_s>& listSinkClasses)
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
        {
            log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
            mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
            mErrorMsg = "Cannot create reply!";
        }
    }
}

std::vector<am_CustomAvailabilityReason_t> CAmRoutingDbusMessageHandler::getListconnectionFormats()
{
    DBusMessageIter arrayIter;
    std::vector<am_CustomAvailabilityReason_t> listConnectionFormats;
    if (DBUS_TYPE_ARRAY != dbus_message_iter_get_arg_type(&mDBusMessageIter))
       {
           log(&routingDbus, DLT_LOG_ERROR, "CAmRoutingDbusMessageHandler::getListconnectionFormats DBUS handler argument is no array!");
           mErrorName = std::string(DBUS_ERROR_INVALID_ARGS);
           mErrorMsg = "DBus argument is no array";
       }
   else
   {
       dbus_message_iter_recurse(&mDBusMessageIter, &arrayIter);
       do
       {
           am_CustomAvailabilityReason_t connectionformat(static_cast<am_CustomAvailabilityReason_t>(getUInt(arrayIter, false)));
           listConnectionFormats.push_back(connectionformat);
       } while (dbus_message_iter_next(&arrayIter));
       dbus_message_iter_next(&mDBusMessageIter);
   }
   return (listConnectionFormats);

}

std::vector<bool> CAmRoutingDbusMessageHandler::getListBool()
{
    DBusMessageIter arrayIter;
    std::vector<bool> listBools;
    if (DBUS_TYPE_ARRAY != dbus_message_iter_get_arg_type(&mDBusMessageIter))
       {
           log(&routingDbus, DLT_LOG_ERROR, "CAmRoutingDbusMessageHandler::getListBool DBUS handler argument is no array!");
           mErrorName = std::string(DBUS_ERROR_INVALID_ARGS);
           mErrorMsg = "DBus argument is no array";
       }
   else
   {
       dbus_message_iter_recurse(&mDBusMessageIter, &arrayIter);
       do
       {
           bool myBool(static_cast<bool>(getBool(arrayIter, false)));
           listBools.push_back(myBool);
       } while (dbus_message_iter_next(&arrayIter));
       dbus_message_iter_next(&mDBusMessageIter);
   }
   return (listBools);
}

am_SoundProperty_s CAmRoutingDbusMessageHandler::getSoundProperty()
{
    am_SoundProperty_s soundProperty;
    DBusMessageIter structIter;

    dbus_message_iter_recurse(&mDBusMessageIter, &structIter);
    soundProperty.type = static_cast<am_CustomSoundPropertyType_t>(getInt(structIter, true));
    soundProperty.value = static_cast<int16_t>(getInt(structIter, false));
    dbus_message_iter_next(&mDBusMessageIter);

    return(soundProperty);
}

std::vector<am_SoundProperty_s> CAmRoutingDbusMessageHandler::getListSoundProperties()
{
    DBusMessageIter arrayIter;
    DBusMessageIter structIter;
    am_SoundProperty_s soundProperty;
    std::vector<am_SoundProperty_s> listSoundProperties;
    if (DBUS_TYPE_ARRAY != dbus_message_iter_get_arg_type(&mDBusMessageIter))
       {
           log(&routingDbus, DLT_LOG_ERROR, "CAmRoutingDbusMessageHandler::getListSoundProperties DBUS handler argument is no array!");
           mErrorName = std::string(DBUS_ERROR_INVALID_ARGS);
           mErrorMsg = "DBus argument is no array";
       }
   else
   {
       dbus_message_iter_recurse(&mDBusMessageIter, &arrayIter);
       do
       {
           dbus_message_iter_recurse(&arrayIter, &structIter);
           soundProperty.type = static_cast<am_CustomSoundPropertyType_t>(getInt(structIter, true));
           soundProperty.value = static_cast<int16_t>(getInt(structIter, false));
           listSoundProperties.push_back(soundProperty);
       } while (dbus_message_iter_next(&arrayIter));
       dbus_message_iter_next(&structIter);
   }
   return (listSoundProperties);
}

std::vector<am_MainSoundProperty_s> CAmRoutingDbusMessageHandler::getListMainSoundProperties()
{
    DBusMessageIter arrayIter;
    DBusMessageIter structIter;
    am_MainSoundProperty_s soundProperty;
    std::vector<am_MainSoundProperty_s> listSoundProperties;
    if (DBUS_TYPE_ARRAY != dbus_message_iter_get_arg_type(&mDBusMessageIter))
       {
           log(&routingDbus, DLT_LOG_ERROR, "CAmRoutingDbusMessageHandler::getListSoundProperties DBUS handler argument is no array!");
           mErrorName = std::string(DBUS_ERROR_INVALID_ARGS);
           mErrorMsg = "DBus argument is no array";
       }
   else
   {
       dbus_message_iter_recurse(&mDBusMessageIter, &arrayIter);
       do
       {
           dbus_message_iter_recurse(&arrayIter, &structIter);
           soundProperty.type = static_cast<am_CustomMainSoundPropertyType_t>(getInt(structIter, true));
           soundProperty.value = static_cast<int16_t>(getInt(structIter, false));
           listSoundProperties.push_back(soundProperty);
       } while (dbus_message_iter_next(&arrayIter));
       dbus_message_iter_next(&structIter);
   }
   return (listSoundProperties);
}

am_NotificationPayload_s CAmRoutingDbusMessageHandler::getNotificationPayload()
{
    am_NotificationPayload_s notificationPayload;
    DBusMessageIter structIter;

    dbus_message_iter_recurse(&mDBusMessageIter, &structIter);
    notificationPayload.type = static_cast<am_CustomNotificationType_t>(getInt(structIter, true));
    notificationPayload.value = static_cast<int16_t>(getInt(structIter, false));
    dbus_message_iter_next(&mDBusMessageIter);

    return (notificationPayload);
}

dbus_int32_t CAmRoutingDbusMessageHandler::getInt32()
{
    return (getInt32(mDBusMessageIter, true));
}

dbus_int32_t CAmRoutingDbusMessageHandler::getInt32(DBusMessageIter& iter, bool next)
{
    dbus_int32_t param(0);
  #ifdef GLIB_DBUS_TYPES_TOLERANT
      if (DBUS_TYPE_INT32 != dbus_message_iter_get_arg_type(&iter) && DBUS_TYPE_INT32 != dbus_message_iter_get_arg_type(&iter))
  #else
      if (DBUS_TYPE_INT32 != dbus_message_iter_get_arg_type(&iter))
  #endif
      {
          log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::getInt DBUS handler argument is no int32_t!");
          mErrorName = std::string(DBUS_ERROR_INVALID_ARGS);
          mErrorMsg = "DBus argument is no int32_t";
      }
      else
      {
          dbus_message_iter_get_basic(&iter, &param);
          if (next)
              dbus_message_iter_next(&iter);
      }
      return (param);
}

void CAmRoutingDbusMessageHandler::append(const std::vector<am::am_SystemProperty_s>& listSystemProperties)
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
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

void CAmRoutingDbusMessageHandler::append(const am::am_Error_e error)
{
    if (!dbus_message_iter_append_basic(&mDBusMessageIter, DBUS_TYPE_UINT16, &error))
    {
        log(&routingDbus, DLT_LOG_ERROR, "DBusMessageHandler::append Cannot allocate DBus message!");
        mErrorName = std::string(DBUS_ERROR_NO_MEMORY);
        mErrorMsg = "Cannot create reply!";
    }
}

}













