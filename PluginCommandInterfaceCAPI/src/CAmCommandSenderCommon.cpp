/**
 *  Copyright (c) 2012 BMW
 *
 *  \author Aleksandar Donchev, aleksander.donchev@partner.bmw.de BMW 2013
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


#include "CAmCommandSenderCommon.h"

/**
 * Utility functions
 */
void CAmConvertAvailablility(const am_Availability_s & amAavailability, CommandInterface::am_Availability_s & result)
{
	result.availability = CAmConvert2CAPIType(amAavailability.availability);
	result.availabilityReason = CAmConvert2CAPIType(amAavailability.availabilityReason);
}

void CAmConvertMainSoundProperty(const am_MainSoundProperty_s & amMainSoundProperty, CommandInterface::am_MainSoundProperty_s & result)
{
	result.type = CAmConvert2CAPIType(amMainSoundProperty.type);
	result.value = amMainSoundProperty.value;
}

void CAmConvertSystemProperty(const am_SystemProperty_s & amSystemProperty, CommandInterface::am_SystemProperty_s & result)
{
	result.type = CAmConvert2CAPIType(amSystemProperty.type);
	result.value = amSystemProperty.value;
}

CommandInterface::am_ClassProperty_e CAmConvert2CAPIType(const am_ClassProperty_e & property)
{
	return CP_MAX==property?
			CommandInterface::am_ClassProperty_e::CP_MAX
			:
			static_cast<CommandInterface::am_ClassProperty_e>(property);
}

CommandInterface::am_SystemPropertyType_e CAmConvert2CAPIType(const am_SystemPropertyType_e & property)
{
	return SYP_MAX==property?
			CommandInterface::am_SystemPropertyType_e::SYP_MAX
			:
			static_cast<CommandInterface::am_SystemPropertyType_e>(property);
}

CommandInterface::am_Availablility_e CAmConvert2CAPIType(const am_Availability_e & availability)
{
	return (A_MAX==availability)?CommandInterface::am_Availablility_e::A_MAX
								 :
								 static_cast<CommandInterface::am_Availablility_e>(availability);
}

CommandInterface::am_AvailabilityReason_e CAmConvert2CAPIType(const am_AvailabilityReason_e & availabilityReason)
{
	return (AR_MAX==availabilityReason)?CommandInterface::am_AvailabilityReason_e::AR_MAX
										:
										static_cast<CommandInterface::am_AvailabilityReason_e>(availabilityReason);
}

CommandInterface::am_MuteState_e CAmConvert2CAPIType(const am_MuteState_e & muteState)
{
	return MS_MAX==muteState?
							CommandInterface::am_MuteState_e::MS_MAX
							:
							static_cast<CommandInterface::am_MuteState_e>(muteState);
}

am_MuteState_e CAmConvertFromCAPIType(const CommandInterface::am_MuteState_e & muteState)
{
	return CommandInterface::am_MuteState_e::MS_MAX==muteState?
							MS_MAX:static_cast<am_MuteState_e>(muteState);
}

CommandInterface::am_MainSoundPropertyType_e CAmConvert2CAPIType(const am_MainSoundPropertyType_e & type)
{
	return MSP_MAX==type?
							CommandInterface::am_MainSoundPropertyType_e::MSP_MAX
							:
							static_cast<CommandInterface::am_MainSoundPropertyType_e>(type);
}
am_MainSoundPropertyType_e CAmConvertFromCAPIType(const CommandInterface::am_MainSoundPropertyType_e & type)
{
	return CommandInterface::am_MainSoundPropertyType_e::MSP_MAX==type?
			MSP_MAX:static_cast<am_MainSoundPropertyType_e>(type);
}



CommandInterface::am_ConnectionState_e CAmConvert2CAPIType(const am_ConnectionState_e & connectionState)
{
	return CS_MAX==connectionState?
							CommandInterface::am_ConnectionState_e::CS_MAX
							:
							static_cast<CommandInterface::am_ConnectionState_e>(connectionState);
}
am_ConnectionState_e CAmConvertFromCAPIType(const CommandInterface::am_ConnectionState_e & connectionState)
{
	return CommandInterface::am_ConnectionState_e::CS_MAX==connectionState?
			CS_MAX:static_cast<am_ConnectionState_e>(connectionState);
}

org::genivi::audiomanager::am::am_NotificationType_e CAmConvert2CAPIType(const am_NotificationType_e & notificationType)
{
	return NT_MAX==notificationType?
							org::genivi::audiomanager::am::am_NotificationType_e::NT_MAX
							:
							static_cast<org::genivi::audiomanager::am::am_NotificationType_e>(notificationType);
}
am_NotificationType_e CAmConvertFromCAPIType(const org::genivi::audiomanager::am::am_NotificationType_e & notificationType)
{
	return org::genivi::audiomanager::am::am_NotificationType_e::NT_MAX==notificationType?
			NT_MAX:static_cast<am_NotificationType_e>(notificationType);
}

org::genivi::audiomanager::am::am_NotificationStatus_e CAmConvert2CAPIType(const am_NotificationStatus_e & notificationStatus)
{
	return NS_MAX==notificationStatus?
							org::genivi::audiomanager::am::am_NotificationStatus_e::NS_MAX
							:
							static_cast<org::genivi::audiomanager::am::am_NotificationStatus_e>(notificationStatus);
}
am_NotificationStatus_e CAmConvertFromCAPIType(const org::genivi::audiomanager::am::am_NotificationStatus_e & notificationStatus)
{
	return org::genivi::audiomanager::am::am_NotificationStatus_e::NS_MAX==notificationStatus?
			NS_MAX:static_cast<am_NotificationStatus_e>(notificationStatus);
}

CommandInterface::am_Error_e CAmConvert2CAPIType(const am_Error_e & error)
{
	return E_MAX==error?
							CommandInterface::am_Error_e::E_MAX
							:
							static_cast<CommandInterface::am_Error_e>(error);
}
am_Error_e CAmConvertFromCAPIType(const CommandInterface::am_Error_e & error)
{
	return CommandInterface::am_Error_e::E_MAX==error?
			E_MAX:static_cast<am_Error_e>(error);
}


am_SystemPropertyType_e CAmConvertFromCAPIType(const CommandInterface::am_SystemPropertyType_e & propType)
{
	return CommandInterface::am_SystemPropertyType_e::SYP_MAX==propType?
			SYP_MAX:static_cast<am_SystemPropertyType_e>(propType);
}

