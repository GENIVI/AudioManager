/*
 * CommandReceiver.cpp
 *
 *  Created on: Oct 24, 2011
 *      Author: christian
 */

#include "CommandReceiver.h"

CommandReceiver::CommandReceiver(DatabaseHandler* iDatabaseHandler, DBusWrapper* iDBusWrapper)
: mDatabaseHandler(iDatabaseHandler), mDBusWrapper(iDBusWrapper)
{
}

CommandReceiver::~CommandReceiver()
{
}

am_Error_e CommandReceiver::connect(const am_sourceID_t sourceID, const am_sinkID_t sinkID, am_mainConnectionID_t & mainConnectionID)
{
	mainConnectionID=4;
	return E_OK;
}



am_Error_e CommandReceiver::disconnect(const am_mainConnectionID_t mainConnectionID)
{
}



am_Error_e CommandReceiver::setVolume(const am_sinkID_t sinkID, const am_mainVolume_t volume)
{
}



am_Error_e CommandReceiver::volumeStep(const am_sinkID_t sinkID, const int16_t volumeStep)
{
}



am_Error_e CommandReceiver::setSinkMuteState(const am_sinkID_t sinkID, const am_MuteState_e muteState)
{
}



am_Error_e CommandReceiver::setMainSinkSoundProperty(const am_MainSoundProperty_s & soundProperty, const am_sinkID_t sinkID)
{
}



am_Error_e CommandReceiver::setMainSourceSoundProperty(const am_MainSoundProperty_s & soundProperty, const am_sourceID_t sourceID)
{
}



am_Error_e CommandReceiver::setSystemProperty(const am_SystemProperty_s & property)
{
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







