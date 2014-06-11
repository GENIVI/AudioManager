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

void CAmCommandSenderService::connect(org::genivi::am::am_sourceID_t sourceID, org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_mainConnectionID_t& mainConnectionID,org::genivi::am::am_Error_e& result) {
	assert(mpIAmCommandReceive);
	result = CAmConvert2CAPIType(mpIAmCommandReceive->connect(sourceID, sinkID, mainConnectionID));
}

void CAmCommandSenderService::disconnect(org::genivi::am::am_mainConnectionID_t mainConnectionID, org::genivi::am::am_Error_e& result) {
	assert(mpIAmCommandReceive);
	result = CAmConvert2CAPIType(mpIAmCommandReceive->disconnect(mainConnectionID));
}

void CAmCommandSenderService::setVolume(org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_mainVolume_t volume, org::genivi::am::am_Error_e& result) {

	assert(mpIAmCommandReceive);
	result = CAmConvert2CAPIType(mpIAmCommandReceive->setVolume(sinkID, volume));
}

void CAmCommandSenderService::volumeStep(org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_mainVolume_t volumeStep, org::genivi::am::am_Error_e& result) {
	assert(mpIAmCommandReceive);
	result = CAmConvert2CAPIType(mpIAmCommandReceive->volumeStep(sinkID, volumeStep));
}

void CAmCommandSenderService::setSinkMuteState(org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_MuteState_e muteState, org::genivi::am::am_Error_e& result) {
	assert(mpIAmCommandReceive);
	result = CAmConvert2CAPIType(mpIAmCommandReceive->setSinkMuteState(sinkID, CAmConvertFromCAPIType(muteState)));
}

void CAmCommandSenderService::setMainSinkSoundProperty(org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_MainSoundProperty_s soundProperty, org::genivi::am::am_Error_e& result) {
	assert(mpIAmCommandReceive);
	am_MainSoundProperty_s property = {static_cast<am_CustomMainSoundPropertyType_t>(soundProperty.type), soundProperty.value};
	result = CAmConvert2CAPIType(mpIAmCommandReceive->setMainSinkSoundProperty(property, sinkID));
}

void CAmCommandSenderService::setMainSourceSoundProperty(org::genivi::am::am_sourceID_t sourceID, org::genivi::am::am_MainSoundProperty_s soundProperty, org::genivi::am::am_Error_e& result) {
    assert(mpIAmCommandReceive);
    am_MainSoundProperty_s property = {static_cast<am_CustomMainSoundPropertyType_t>(soundProperty.type), soundProperty.value};
    result = CAmConvert2CAPIType(mpIAmCommandReceive->setMainSourceSoundProperty(property, sourceID));
}

void CAmCommandSenderService::setSystemProperty(org::genivi::am::am_SystemProperty_s soundProperty, org::genivi::am::am_Error_e& result) {
    assert(mpIAmCommandReceive);
    am_SystemProperty_s property = {static_cast<am_CustomSystemPropertyType_t>(soundProperty.type), soundProperty.value};
    result = CAmConvert2CAPIType(mpIAmCommandReceive->setSystemProperty(property));
}

void CAmCommandSenderService::getListMainConnections(org::genivi::am::am_MainConnection_L& listConnections,org::genivi::am::am_Error_e& result) {
    assert(mpIAmCommandReceive);
    std::vector<am_MainConnectionType_s> list;
    result = CAmConvert2CAPIType(mpIAmCommandReceive->getListMainConnections(list));
    if(result==org::genivi::am::am_Error_e::E_OK)
    {
    	org::genivi::am::am_MainConnectionType_s item;
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

void CAmCommandSenderService::getListMainSinks(org::genivi::am::am_SinkType_L& listMainSinks,org::genivi::am::am_Error_e& result) {
    assert(mpIAmCommandReceive);
    std::vector<am_SinkType_s> list;
    result = CAmConvert2CAPIType(mpIAmCommandReceive->getListMainSinks(list));
    if(result==org::genivi::am::am_Error_e::E_OK)
    {
    	org::genivi::am::am_SinkType_s item;
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

void CAmCommandSenderService::getListMainSources(org::genivi::am::am_SourceType_L& listMainSources,org::genivi::am::am_Error_e& result) {
    assert(mpIAmCommandReceive);
    std::vector<am_SourceType_s> list;
	result = CAmConvert2CAPIType(mpIAmCommandReceive->getListMainSources(list));
	if(result==org::genivi::am::am_Error_e::E_OK)
	{
		org::genivi::am::am_SourceType_s item;
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

void CAmCommandSenderService::getListMainSinkSoundProperties(org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_MainSoundProperty_L& listSoundProperties,org::genivi::am::am_Error_e& result) {
    assert(mpIAmCommandReceive);
    std::vector<am_MainSoundProperty_s> list;
    result = CAmConvert2CAPIType(mpIAmCommandReceive->getListMainSinkSoundProperties(sinkID, list));
	if(result==org::genivi::am::am_Error_e::E_OK)
	{
		org::genivi::am::am_MainSoundProperty_s item;
		for(std::vector<am_MainSoundProperty_s>::const_iterator iter = list.begin(); iter!=list.end(); iter++)
		{
			item.type = iter->type;
			item.value = iter->value;
			listSoundProperties.push_back (item);
		}
	}
}

void CAmCommandSenderService::getListMainSourceSoundProperties(org::genivi::am::am_sourceID_t sourceID, org::genivi::am::am_MainSoundProperty_L& listSourceProperties,org::genivi::am::am_Error_e& result) {
    assert(mpIAmCommandReceive);
    std::vector<am_MainSoundProperty_s> list;
    result = CAmConvert2CAPIType(mpIAmCommandReceive->getListMainSourceSoundProperties(sourceID, list));
	if(result==org::genivi::am::am_Error_e::E_OK)
	{
		org::genivi::am::am_MainSoundProperty_s item;
		for(std::vector<am_MainSoundProperty_s>::const_iterator iter = list.begin(); iter!=list.end(); iter++)
		{
			item.type = iter->type;
			item.value = iter->value;
			listSourceProperties.push_back (item);
		}
	}
}

void CAmCommandSenderService::getListSourceClasses(org::genivi::am::am_SourceClass_L& listSourceClasses,org::genivi::am::am_Error_e& result) {
    assert(mpIAmCommandReceive);
    std::vector<am_SourceClass_s> list;
    result = CAmConvert2CAPIType(mpIAmCommandReceive->getListSourceClasses(list));
	if(result==org::genivi::am::am_Error_e::E_OK)
	{
		org::genivi::am::am_SourceClass_s item;
		for(std::vector<am_SourceClass_s>::const_iterator iter = list.begin(); iter!=list.end(); iter++)
		{
			item.sourceClassID = iter->sourceClassID;
			item.name = iter->name;
			item.listClassProperties.clear();
			std::for_each(iter->listClassProperties.begin(), iter->listClassProperties.end(), [&](const am_ClassProperty_s & ref) {
				org::genivi::am::am_ClassProperty_s classProp(ref.classProperty, ref.value);
				item.listClassProperties.push_back(classProp);
			});
			listSourceClasses.push_back (item);
		}
	}
}

void CAmCommandSenderService::getListSinkClasses(org::genivi::am::am_SinkClass_L& listSinkClasses,org::genivi::am::am_Error_e& result) {
    assert(mpIAmCommandReceive);
    std::vector<am_SinkClass_s> list;
    result = CAmConvert2CAPIType(mpIAmCommandReceive->getListSinkClasses(list));
	if(result==org::genivi::am::am_Error_e::E_OK)
	{
		org::genivi::am::am_SinkClass_s item;
		for(std::vector<am_SinkClass_s>::const_iterator iter = list.begin(); iter!=list.end(); iter++)
		{
			item.sinkClassID = iter->sinkClassID;
			item.name = iter->name;
			item.listClassProperties.clear();
			std::for_each(iter->listClassProperties.begin(), iter->listClassProperties.end(), [&](const am_ClassProperty_s & ref) {
				org::genivi::am::am_ClassProperty_s classProp(ref.classProperty, ref.value);
				item.listClassProperties.push_back(classProp);
			});
			listSinkClasses.push_back (item);
		}
	}
}

void CAmCommandSenderService::getListSystemProperties(org::genivi::am::am_SystemProperty_L& listSystemProperties,org::genivi::am::am_Error_e& result) {
    assert(mpIAmCommandReceive);
    std::vector<am_SystemProperty_s> list;
    result = CAmConvert2CAPIType(mpIAmCommandReceive->getListSystemProperties(list));
    if(result==org::genivi::am::am_Error_e::E_OK)
    	{
    		org::genivi::am::am_SystemProperty_s item;
    		for(std::vector<am_SystemProperty_s>::const_iterator iter = list.begin(); iter!=list.end(); iter++)
    		{
    			item.type = iter->type;
    			item.value = iter->value;
    			listSystemProperties.push_back (item);
    		}
    	}
}

void CAmCommandSenderService::getTimingInformation(org::genivi::am::am_mainConnectionID_t mainConnectionID, org::genivi::am::am_timeSync_t& delay,org::genivi::am::am_Error_e& result) {
    assert(mpIAmCommandReceive);
    result = CAmConvert2CAPIType(mpIAmCommandReceive->getTimingInformation(mainConnectionID, delay));
}

} /* namespace am */
