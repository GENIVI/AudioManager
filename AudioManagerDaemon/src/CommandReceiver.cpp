/*
 * CommandReceiver.cpp
 *
 *  Created on: Oct 24, 2011
 *      Author: christian
 */

#include "CommandReceiver.h"
#include <dlt/dlt.h>
#include <assert.h>

DLT_IMPORT_CONTEXT(AudioManager)

CommandReceiver::CommandReceiver (DatabaseHandler* iDatabaseHandler, DBusWrapper* iDBusWrapper, ControlSender* iControlSender)
	: mDatabaseHandler(iDatabaseHandler),
	  mDBusWrapper(iDBusWrapper),
	  mControlSender(iControlSender)
{
	assert(mDatabaseHandler!=NULL);
	assert(mDBusWrapper!=NULL);
	assert(mControlSender!=NULL);
}

CommandReceiver::~CommandReceiver()
{
}

am_Error_e CommandReceiver::connect(const am_sourceID_t sourceID, const am_sinkID_t sinkID, am_mainConnectionID_t & mainConnectionID)
{
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("CommandReceiver::connect got called:"),DLT_STRING("sourceID"),DLT_INT(sourceID), DLT_STRING("sinkID"), DLT_INT(sinkID));
	return mControlSender->hookUserConnectionRequest(sourceID,sinkID,mainConnectionID);
}



am_Error_e CommandReceiver::disconnect(const am_mainConnectionID_t mainConnectionID)
{
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("CommandReceiver::disconnect got called, mainConnectionID="),DLT_INT(mainConnectionID));
	return mControlSender->hookUserDisconnectionRequest(mainConnectionID);
}



am_Error_e CommandReceiver::setVolume(const am_sinkID_t sinkID, const am_mainVolume_t volume)
{
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("CommandReceiver::setVolume got called, sinkID="),DLT_INT(sinkID),DLT_STRING("volume="),DLT_INT(volume));
	return mControlSender->hookUserVolumeChange(sinkID,volume);
}



am_Error_e CommandReceiver::volumeStep(const am_sinkID_t sinkID, const int16_t volumeStep)
{
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("CommandReceiver::volumeStep got called, sinkID="),DLT_INT(sinkID),DLT_STRING("volumeStep="),DLT_INT(volumeStep));
	return mControlSender->hookUserVolumeStep(sinkID,volumeStep);
}



am_Error_e CommandReceiver::setSinkMuteState(const am_sinkID_t sinkID, const am_MuteState_e muteState)
{
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("CommandReceiver::setSinkMuteState got called, sinkID="),DLT_INT(sinkID),DLT_STRING("muteState="),DLT_INT(muteState));
	return mControlSender->hookUserSetSinkMuteState(sinkID,muteState);
}



am_Error_e CommandReceiver::setMainSinkSoundProperty(const am_MainSoundProperty_s & soundProperty, const am_sinkID_t sinkID)
{
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("CommandReceiver::setMainSinkSoundProperty got called, sinkID="),DLT_INT(sinkID),DLT_STRING("soundPropertyType="),DLT_INT(soundProperty.type),DLT_STRING("soundPropertyValue="),DLT_INT(soundProperty.value));
	return mControlSender->hookUserSetMainSinkSoundProperty(sinkID,soundProperty);
}



am_Error_e CommandReceiver::setMainSourceSoundProperty(const am_MainSoundProperty_s & soundProperty, const am_sourceID_t sourceID)
{
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("CommandReceiver::setMainSourceSoundProperty got called, sourceID="),DLT_INT(sourceID),DLT_STRING("soundPropertyType="),DLT_INT(soundProperty.type),DLT_STRING("soundPropertyValue="),DLT_INT(soundProperty.value));
	return mControlSender->hookUserSetMainSourceSoundProperty(sourceID,soundProperty);
}



am_Error_e CommandReceiver::setSystemProperty(const am_SystemProperty_s & property)
{
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("CommandReceiver::setSystemProperty got called"),DLT_STRING("type="),DLT_INT(property.type),DLT_STRING("soundPropertyValue="),DLT_INT(property.value));
	return mControlSender->hookUserSetSystemProperty(property);
}



am_Error_e CommandReceiver::getListMainConnections(std::vector<am_MainConnectionType_s> & listConnections) const
{
	return mDatabaseHandler->getListVisibleMainConnections(listConnections);

}



am_Error_e CommandReceiver::getListMainSinks(std::vector<am_SinkType_s>& listMainSinks) const
{
	return mDatabaseHandler->getListMainSinks(listMainSinks);
}



am_Error_e CommandReceiver::getListMainSources(std::vector<am_SourceType_s>& listMainSources) const
{
	return mDatabaseHandler->getListMainSources(listMainSources);
}



am_Error_e CommandReceiver::getListMainSinkSoundProperties(const am_sinkID_t sinkID, std::vector<am_MainSoundProperty_s> & listSoundProperties) const
{
	return mDatabaseHandler->getListMainSinkSoundProperties(sinkID,listSoundProperties);
}



am_Error_e CommandReceiver::getListMainSourceSoundProperties(const am_sourceID_t sourceID, std::vector<am_MainSoundProperty_s> & listSourceProperties) const
{
	return mDatabaseHandler->getListMainSourceSoundProperties(sourceID,listSourceProperties);
}



am_Error_e CommandReceiver::getListSourceClasses(std::vector<am_SourceClass_s> & listSourceClasses) const
{
	return mDatabaseHandler->getListSourceClasses(listSourceClasses);
}



am_Error_e CommandReceiver::getListSinkClasses(std::vector<am_SinkClass_s> & listSinkClasses) const
{
	return mDatabaseHandler->getListSinkClasses(listSinkClasses);
}



am_Error_e CommandReceiver::getListSystemProperties(std::vector<am_SystemProperty_s> & listSystemProperties) const
{
	return mDatabaseHandler->getListSystemProperties(listSystemProperties);
}



am_Error_e CommandReceiver::getTimingInformation(const am_mainConnectionID_t mainConnectionID, am_timeSync_t & delay) const
{
	return mDatabaseHandler->getTimingInformation(mainConnectionID,delay);
}



am_Error_e CommandReceiver::getDBusConnectionWrapper(DBusWrapper*& dbusConnectionWrapper) const
{
	dbusConnectionWrapper=mDBusWrapper;
	return E_OK;
}







