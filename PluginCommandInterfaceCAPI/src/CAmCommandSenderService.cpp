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

#include "CAmCommandSenderService.h"
#include <assert.h>
#include <algorithm>
#include "CAmCommandSenderCommon.h"


namespace am {

CAmCommandSenderService::CAmCommandSenderService():mpIAmCommandReceive(NULL) {
	// TODO Auto-generated constructor stub

}

CAmCommandSenderService::CAmCommandSenderService(IAmCommandReceive *aReceiver):mpIAmCommandReceive(aReceiver) {
	// TODO Auto-generated constructor stub

}

CAmCommandSenderService::~CAmCommandSenderService() {
	// TODO Auto-generated destructor stub
}

void CAmCommandSenderService::Connect(CommandInterface::am_sourceID_t sourceID, CommandInterface::am_sinkID_t sinkID, CommandInterface::am_Error_e& result, CommandInterface::am_mainConnectionID_t& mainConnectionID) {
	assert(mpIAmCommandReceive);
	result = CAmConvert2CAPIType(mpIAmCommandReceive->connect(sourceID, sinkID, mainConnectionID));
}

void CAmCommandSenderService::Disconnect(CommandInterface::am_mainConnectionID_t mainConnectionID, CommandInterface::am_Error_e& result) {
	assert(mpIAmCommandReceive);
	result = CAmConvert2CAPIType(mpIAmCommandReceive->disconnect(mainConnectionID));
}

void CAmCommandSenderService::SetVolume(CommandInterface::am_sinkID_t sinkID, CommandInterface::am_mainVolume_t volume, CommandInterface::am_Error_e& result) {

	assert(mpIAmCommandReceive);
	result = CAmConvert2CAPIType(mpIAmCommandReceive->setVolume(sinkID, volume));
}

void CAmCommandSenderService::VolumeStep(CommandInterface::am_sinkID_t sinkID, CommandInterface::am_mainVolume_t volumeStep, CommandInterface::am_Error_e& result) {
	assert(mpIAmCommandReceive);
	result = CAmConvert2CAPIType(mpIAmCommandReceive->volumeStep(sinkID, volumeStep));
}

void CAmCommandSenderService::SetSinkMuteState(CommandInterface::am_sinkID_t sinkID, CommandInterface::am_MuteState_e muteState, CommandInterface::am_Error_e& result) {
	assert(mpIAmCommandReceive);
	result = CAmConvert2CAPIType(mpIAmCommandReceive->setSinkMuteState(sinkID, CAmConvertFromCAPIType(muteState)));
}

void CAmCommandSenderService::SetMainSinkSoundProperty(CommandInterface::am_sinkID_t sinkID, CommandInterface::am_MainSoundProperty_s soundProperty, CommandInterface::am_Error_e& result) {
	assert(mpIAmCommandReceive);
	am_MainSoundProperty_s property = {CAmConvertFromCAPIType(soundProperty.type), soundProperty.value};
	result = CAmConvert2CAPIType(mpIAmCommandReceive->setMainSinkSoundProperty(property, sinkID));
}

void CAmCommandSenderService::SetMainSourceSoundProperty(CommandInterface::am_sourceID_t sourceID, CommandInterface::am_MainSoundProperty_s soundProperty, CommandInterface::am_Error_e& result) {
    assert(mpIAmCommandReceive);
    am_MainSoundProperty_s property = {CAmConvertFromCAPIType(soundProperty.type), soundProperty.value};
    result = CAmConvert2CAPIType(mpIAmCommandReceive->setMainSourceSoundProperty(property, sourceID));
}

void CAmCommandSenderService::SetSystemProperty(CommandInterface::am_SystemProperty_s soundProperty, CommandInterface::am_Error_e& result) {
    assert(mpIAmCommandReceive);
    am_SystemProperty_s property = {CAmConvertFromCAPIType(soundProperty.type), soundProperty.value};
    result = CAmConvert2CAPIType(mpIAmCommandReceive->setSystemProperty(property));
}

void CAmCommandSenderService::GetListMainConnections(CommandInterface::am_Error_e& result, CommandInterface::am_MainConnectionType_l& listConnections) {
    assert(mpIAmCommandReceive);
    std::vector<am_MainConnectionType_s> list;
    result = CAmConvert2CAPIType(mpIAmCommandReceive->getListMainConnections(list));
    if(result==CommandInterface::am_Error_e::E_OK)
    {
    	CommandInterface::am_MainConnectionType_s item;
		for(std::vector<am_MainConnectionType_s>::const_iterator iter = list.begin(); iter!=list.end(); iter++)
		{
			item.mainConnectionID = iter->mainConnectionID;
			item.sourceID = iter->sourceID;
			item.sinkID = iter->sinkID;
			item.delay = iter->delay;
			item.connectionState = CAmConvert2CAPIType(iter->connectionState);
			listConnections.push_back (item);
		}
    }
}

void CAmCommandSenderService::GetListMainSinks(CommandInterface::am_Error_e& result, CommandInterface::am_SinkType_l& listMainSinks) {
    assert(mpIAmCommandReceive);
    std::vector<am_SinkType_s> list;
    result = CAmConvert2CAPIType(mpIAmCommandReceive->getListMainSinks(list));
    if(result==CommandInterface::am_Error_e::E_OK)
    {
    	CommandInterface::am_SinkType_s item;
		for(std::vector<am_SinkType_s>::const_iterator iter = list.begin(); iter!=list.end(); iter++)
		{
			item.sinkID = iter->sinkID;
			item.name = iter->name;
			item.sinkClassID = iter->sinkClassID;
			item.volume = iter->volume;
			item.muteState = CAmConvert2CAPIType(iter->muteState);
		    CAmConvertAvailablility(iter->availability, item.availability);
			listMainSinks.push_back (item);
		}
    }
}

void CAmCommandSenderService::GetListMainSources(CommandInterface::am_Error_e& result, CommandInterface::am_SourceType_l& listMainSources) {
    assert(mpIAmCommandReceive);
    std::vector<am_SourceType_s> list;
	result = CAmConvert2CAPIType(mpIAmCommandReceive->getListMainSources(list));
	if(result==CommandInterface::am_Error_e::E_OK)
	{
		CommandInterface::am_SourceType_s item;
		for(std::vector<am_SourceType_s>::const_iterator iter = list.begin(); iter!=list.end(); iter++)
		{
			item.sourceID = iter->sourceID;
			item.name = iter->name;
			item.sourceClassID = iter->sourceClassID;
			CAmConvertAvailablility(iter->availability, item.availability);
			listMainSources.push_back (item);
		}
	}
}

void CAmCommandSenderService::GetListMainSinkSoundProperties(CommandInterface::am_sinkID_t sinkID, CommandInterface::am_Error_e& result, CommandInterface::am_MainSoundProperty_l& listSoundProperties) {
    assert(mpIAmCommandReceive);
    std::vector<am_MainSoundProperty_s> list;
    result = CAmConvert2CAPIType(mpIAmCommandReceive->getListMainSinkSoundProperties(sinkID, list));
	if(result==CommandInterface::am_Error_e::E_OK)
	{
		CommandInterface::am_MainSoundProperty_s item;
		for(std::vector<am_MainSoundProperty_s>::const_iterator iter = list.begin(); iter!=list.end(); iter++)
		{
			item.type = CAmConvert2CAPIType(iter->type);
			item.value = iter->value;
			listSoundProperties.push_back (item);
		}
	}
}

void CAmCommandSenderService::GetListMainSourceSoundProperties(CommandInterface::am_sourceID_t sourceID, CommandInterface::am_Error_e& result, CommandInterface::am_MainSoundProperty_l& listSourceProperties) {
    assert(mpIAmCommandReceive);
    std::vector<am_MainSoundProperty_s> list;
    result = CAmConvert2CAPIType(mpIAmCommandReceive->getListMainSourceSoundProperties(sourceID, list));
	if(result==CommandInterface::am_Error_e::E_OK)
	{
		CommandInterface::am_MainSoundProperty_s item;
		for(std::vector<am_MainSoundProperty_s>::const_iterator iter = list.begin(); iter!=list.end(); iter++)
		{
			item.type = CAmConvert2CAPIType(iter->type);
			item.value = iter->value;
			listSourceProperties.push_back (item);
		}
	}
}

void CAmCommandSenderService::GetListSourceClasses(CommandInterface::am_Error_e& result, CommandInterface::am_SourceClass_l& listSourceClasses) {
    assert(mpIAmCommandReceive);
    std::vector<am_SourceClass_s> list;
    result = CAmConvert2CAPIType(mpIAmCommandReceive->getListSourceClasses(list));
	if(result==CommandInterface::am_Error_e::E_OK)
	{
		CommandInterface::am_SourceClass_s item;
		for(std::vector<am_SourceClass_s>::const_iterator iter = list.begin(); iter!=list.end(); iter++)
		{
			item.sourceClassID = iter->sourceClassID;
			item.name = iter->name;
			item.listClassProperties.clear();
			std::for_each(iter->listClassProperties.begin(), iter->listClassProperties.end(), [&](const am_ClassProperty_s & ref) {
				CommandInterface::am_ClassProperty_s classProp(CAmConvert2CAPIType(ref.classProperty), ref.value);
				item.listClassProperties.push_back(classProp);
			});
			listSourceClasses.push_back (item);
		}
	}
}

void CAmCommandSenderService::GetListSinkClasses(CommandInterface::am_Error_e& result, CommandInterface::am_SinkClass_l& listSinkClasses) {
    assert(mpIAmCommandReceive);
    std::vector<am_SinkClass_s> list;
    result = CAmConvert2CAPIType(mpIAmCommandReceive->getListSinkClasses(list));
	if(result==CommandInterface::am_Error_e::E_OK)
	{
		CommandInterface::am_SinkClass_s item;
		for(std::vector<am_SinkClass_s>::const_iterator iter = list.begin(); iter!=list.end(); iter++)
		{
			item.sinkClassID = iter->sinkClassID;
			item.name = iter->name;
			item.listClassProperties.clear();
			std::for_each(iter->listClassProperties.begin(), iter->listClassProperties.end(), [&](const am_ClassProperty_s & ref) {
				CommandInterface::am_ClassProperty_s classProp(CAmConvert2CAPIType(ref.classProperty), ref.value);
				item.listClassProperties.push_back(classProp);
			});
			listSinkClasses.push_back (item);
		}
	}
}

void CAmCommandSenderService::GetListSystemProperties(CommandInterface::am_Error_e& result, CommandInterface::am_SystemProperty_l& listSystemProperties) {
    assert(mpIAmCommandReceive);
    std::vector<am_SystemProperty_s> list;
    result = CAmConvert2CAPIType(mpIAmCommandReceive->getListSystemProperties(list));
    if(result==CommandInterface::am_Error_e::E_OK)
    	{
    		CommandInterface::am_SystemProperty_s item;
    		for(std::vector<am_SystemProperty_s>::const_iterator iter = list.begin(); iter!=list.end(); iter++)
    		{
    			item.type = CAmConvert2CAPIType(iter->type);
    			item.value = iter->value;
    			listSystemProperties.push_back (item);
    		}
    	}
}

void CAmCommandSenderService::GetTimingInformation(CommandInterface::am_mainConnectionID_t mainConnectionID, CommandInterface::am_Error_e& result, CommandInterface::am_timeSync_t& delay) {
    assert(mpIAmCommandReceive);
    result = CAmConvert2CAPIType(mpIAmCommandReceive->getTimingInformation(mainConnectionID, delay));
}

} /* namespace am */
