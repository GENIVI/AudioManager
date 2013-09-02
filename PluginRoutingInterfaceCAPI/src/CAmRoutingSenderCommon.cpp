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


#include "CAmRoutingSenderCommon.h"

/**
 * Utility functions
 */

void CAmConvertCAPI2AM(const am_gen::am_Domain_s & source, am::am_Domain_s & destination)
{
	destination.domainID = source.domainID;
	destination.name = source.name;
	destination.busname = source.busname;
	destination.nodename = source.nodename;
	destination.early = source.early;
	destination.complete = source.complete;
	destination.state = static_cast<am::am_DomainState_e>(source.state);
}

void CAmConvertCAPI2AM(const am_gen::am_SoundProperty_s & source, am::am_SoundProperty_s & destination)
{
	destination.type = static_cast<am::am_SoundPropertyType_e>(source.type);
	destination.value = source.value;
}

void CAmConvertCAPIVector2AM(const std::vector<am_gen::am_SoundProperty_s> & source, std::vector<am::am_SoundProperty_s> & destination)
{
	am::am_SoundProperty_s soundProp;
	destination.clear();
	for(std::vector<am_gen::am_SoundProperty_s>::const_iterator iter = source.begin(); iter!=source.end(); iter++)
	{
		CAmConvertCAPI2AM(*iter, soundProp);
		destination.push_back(soundProp);
	}
}

void CAmConvertCAPI2AM(const am_gen::am_MainSoundProperty_s & source, am::am_MainSoundProperty_s & destination)
{
	destination.type = static_cast<am::am_MainSoundPropertyType_e>(source.type);
	destination.value = source.value;
}


void CAmConvertCAPI2AM(const am_gen::notificationPayload_s & source, am::am_NotificationPayload_s & destination)
{
	destination.type = static_cast<am::am_NotificationType_e>(source.type);
	destination.value = source.payload;
}

void CAmConvertCAPIVector2AM(const std::vector<am_gen::am_Volumes_s> & source, std::vector<am::am_Volumes_s> & destination)
{
	destination.clear();
	for(std::vector<am_gen::am_Volumes_s>::const_iterator iter = source.begin(); iter!=source.end(); iter++)
	{
		am::am_Volumes_s volume;
		CAmConvertCAPI2AM(*iter, volume);
		destination.push_back(volume);
	}
}

void CAmConvertCAPIVector2AM(const std::vector<am_gen::am_MainSoundProperty_s> & source, std::vector<am::am_MainSoundProperty_s> & destination)
{
	am::am_MainSoundProperty_s soundProp;
	destination.clear();
	for(std::vector<am_gen::am_MainSoundProperty_s>::const_iterator iter = source.begin(); iter!=source.end(); iter++)
	{
		CAmConvertCAPI2AM(*iter, soundProp);
		destination.push_back(soundProp);
	}
}

void CAmConvertCAPI2AM(const am_gen::am_NotificationConfiguration_s & source, am::am_NotificationConfiguration_s & destination)
{
	destination.type = static_cast<am::am_NotificationType_e>(source.type);
	destination.status = static_cast<am::am_NotificationStatus_e>(source.status);
	destination.parameter = source.parameter;
}

void CAmConvertCAPIVector2AM(const std::vector<am_gen::am_NotificationConfiguration_s> & source, std::vector<am::am_NotificationConfiguration_s> & destination)
{
	am::am_NotificationConfiguration_s soundProp;
	destination.clear();
	for(std::vector<am_gen::am_NotificationConfiguration_s>::const_iterator iter = source.begin(); iter!=source.end(); iter++)
	{
		CAmConvertCAPI2AM(*iter, soundProp);
		destination.push_back(soundProp);
	}
}

void CAmConvertCAPIVector2AM(const std::vector<am_gen::am_ConnectionFormat_e> & source, std::vector<am::am_ConnectionFormat_e> & destination)
{
	destination.clear();
	for(std::vector<am_gen::am_ConnectionFormat_e>::const_iterator iter = source.begin(); iter!=source.end(); iter++)
		destination.push_back(static_cast<am::am_ConnectionFormat_e>(*iter));
}



void CAmConvertCAPI2AM(const am_gen::sourceData_s & source, am::am_Source_s & destination)
{
	destination.sourceID = source.sourceID;
	destination.domainID = source.domainID;
	destination.name = source.name;
	destination.sourceClassID = source.sourceClassID;
	destination.sourceState = static_cast<am::am_SourceState_e>(source.sourceState);
	destination.volume = source.volume;
	destination.visible = source.visible;
	CAmConvertCAPI2AM(source.available, destination.available);
	destination.interruptState =  static_cast<am::am_InterruptState_e>(source.interruptState);
	CAmConvertCAPIVector2AM(source.listSoundProperties, destination.listSoundProperties);
	CAmConvertCAPIVector2AM(source.listConnectionFormats, destination.listConnectionFormats);
	CAmConvertCAPIVector2AM(source.listMainSoundProperties, destination.listMainSoundProperties);
	CAmConvertCAPIVector2AM(source.listNotificationConfigurations, destination.listNotificationConfigurations);
	CAmConvertCAPIVector2AM(source.listMainNotificationConfigurations, destination.listMainNotificationConfigurations);
}

void CAmConvertCAPI2AM(const am_gen::sinkData_s & source, am::am_Sink_s & destination)
{
	destination.sinkID = source.sinkID;
	destination.domainID = source.domainID;
	destination.name = source.name;
	destination.sinkClassID = source.sinkClassID;
	destination.muteState = static_cast<am::am_MuteState_e>(source.muteState);
	destination.volume = source.volume;
	destination.visible = source.visible;
	destination.mainVolume = source.mainVolume;
	CAmConvertCAPI2AM(source.available, destination.available);
	CAmConvertCAPIVector2AM(source.listSoundProperties, destination.listSoundProperties);
	CAmConvertCAPIVector2AM(source.listConnectionFormats, destination.listConnectionFormats);
	CAmConvertCAPIVector2AM(source.listMainSoundProperties, destination.listMainSoundProperties);
	CAmConvertCAPIVector2AM(source.listNotificationConfigurations, destination.listNotificationConfigurations);
	CAmConvertCAPIVector2AM(source.listMainNotificationConfigurations, destination.listMainNotificationConfigurations);
}

void CAmConvertCAPI2AM(const am_gen::am_Volumes_s & source, am::am_Volumes_s & destination)
{
	CAmConvertCAPI2AM(source.volumeID, destination.volumeID);
	destination.volume = source.volume;
	destination.time = source.time;
	destination.volumeType = static_cast<am::am_VolumeType_e>(source.volumeType);
	destination.ramp = static_cast<am::am_RampType_e>(source.ramp);
}


void CAmConvertCAPI2AM(const am_gen::crossfaderData_s & source, am::am_Crossfader_s & destination)
{
	destination.crossfaderID = source.crossfaderID;
	destination.sinkID_A = source.sinkID_A;
	destination.sinkID_B = source.sinkID_B;
	destination.name = source.name;
	destination.sourceID = source.sourceID;
	destination.hotSink = static_cast<am::am_HotSink_e>(source.hotSink);
}


void CAmConvertCAPI2AM(const am_gen::am_Gateway_s & source, am::am_Gateway_s & destination)
{
	destination.sinkID = source.sinkID;
	destination.gatewayID = source.gatewayID;
	destination.name = source.name;
	destination.sourceID = source.sourceID;
	destination.domainSinkID = source.domainSinkID;
	destination.domainSourceID = source.domainSourceID;
	destination.controlDomainID = source.controlDomainID;
	CAmConvertCAPIVector2AM(source.listSourceFormats, destination.listSourceFormats);
	CAmConvertCAPIVector2AM(source.listSinkFormats, destination.listSinkFormats);
	destination.convertionMatrix = source.convertionMatrix;
}

void CAmConvertCAPI2AM(const am_gen::am_EarlyData_u & source, am::am_EarlyData_u & destination)
{
	if(source.isType<am_gen::am_volume_t>())
	{
		am_volume_t value = static_cast<am_volume_t>(source.get<am_gen::am_volume_t>());
		destination.volume = value;
	}
	else if(source.isType<am_gen::am_SoundProperty_s>())
	{
		am_gen::am_SoundProperty_s value = source.get<am_gen::am_SoundProperty_s>();
		am_SoundProperty_s converted;
		CAmConvertCAPI2AM(value, converted);
		destination.soundProperty = converted;
	}
}

void CAmConvertCAPI2AM(const am_gen::am_DataType_u & source, am::am_DataType_u & destination)
{
	if(source.isType<am_gen::am_sinkID_t>())
	{
		am_sinkID_t value = static_cast<am_sinkID_t>(source.get<am_gen::am_sinkID_t>());
		destination.sink = value;
	}
	else if(source.isType<am_gen::am_sourceID_t>())
	{
		am_sourceID_t value = static_cast<am_sourceID_t>(source.get<am_gen::am_sourceID_t>());
		destination.source = value;
	}
}

void CAmConvertCAPI2AM(const am_gen::am_EarlyData_s & source, am::am_EarlyData_s & destination)
{
	CAmConvertCAPI2AM(source.data, destination.data);
	CAmConvertCAPI2AM(source.sinksource, destination.sinksource);
	destination.type = static_cast<am_EarlyDataType_e>(source.type);
}


void CAmConvertCAPI2AM(const am_gen::am_Availability_s  & source,  am_Availability_s & destination)
{
	destination.availability = static_cast<am_Availability_e>(source.availability);
	destination.availabilityReason = static_cast<am_AvailabilityReason_e>(source.availabilityReason);
}

void CAmConvertAM2CAPI(const am_Availability_s & source,  am_gen::am_Availability_s & destination)
{
	destination.availability = static_cast<am_gen::am_Availability_e>(source.availability);
	destination.availabilityReason = static_cast<am_gen::am_AvailabilityReason_e>(source.availabilityReason);
}

void CAmConvertAM2CAPI(const am::am_SoundProperty_s & source, am_gen::am_SoundProperty_s & destination)
{
	destination.type = static_cast<am_gen::am_SoundPropertyType_e>(source.type);
	destination.value = source.value;
}

void CAmConvertAM2CAPI(const am::am_NotificationConfiguration_s & source, am_gen::am_NotificationConfiguration_s & destination)
{
	destination.type = static_cast<am_gen::am_NotificationType_e>(source.type);
	destination.status = static_cast<am_gen::am_NotificationStatus_e>(source.status);
	destination.parameter = source.parameter;
}

void CAmConvertAM2CAPI(const am::am_Volumes_s & source, am_gen::am_Volumes_s & destination)
{
	if(source.volumeType == VT_SINK)
		destination.volumeID = am_gen::am_DataType_u(static_cast<am_gen::am_sinkID_t>(source.volumeID.sink));
	else if(source.volumeType == VT_SOURCE)
		destination.volumeID = am_gen::am_DataType_u(static_cast<am_gen::am_sourceID_t>(source.volumeID.source));
	destination.volumeType = static_cast<am_gen::am_VolumeType_e>(source.volumeType);
	destination.volume = static_cast<am_gen::am_volume_t>(source.volume);
	destination.ramp = static_cast<am_gen::am_RampType_e>(source.ramp);
	destination.time = static_cast<am_gen::am_time_t>(source.time);
}


void CAmConvertAMVector2CAPI(const std::vector<am::am_Volumes_s> & source, std::vector<am_gen::am_Volumes_s> & destination)
{
	destination.clear();
	for(std::vector<am::am_Volumes_s>::const_iterator iter = source.begin(); iter!=source.end(); iter++)
	{
		am_gen::am_Volumes_s volume;
		CAmConvertAM2CAPI(*iter, volume);
		destination.push_back(volume);
	}
}

void CAmConvertAMVector2CAPI(const std::vector<am::am_SoundProperty_s> & source, std::vector<am_gen::am_SoundProperty_s> & destination)
{
	am_gen::am_SoundProperty_s soundProp;
	destination.clear();
	for(std::vector<am::am_SoundProperty_s>::const_iterator iter = source.begin(); iter!=source.end(); iter++)
	{
		CAmConvertAM2CAPI(*iter, soundProp);
		destination.push_back(soundProp);
	}
}
