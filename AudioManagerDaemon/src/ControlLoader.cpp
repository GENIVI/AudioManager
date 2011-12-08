/*
 * ControlLoader.cpp
 *
 *  Created on: Oct 25, 2011
 *      Author: christian
 */

#include "ControlLoader.h"
#include "pluginTemplate.h"
#include <string>

ControlLoader::ControlLoader()
{
	ControlSendInterface* (*createFunc)();
	std::string libPath="/home/christian/workspace/gitserver/build/plugins/control/libPluginControlInterface.so";
	createFunc = getCreateFunction<ControlSendInterface*()>(libPath);

	if (!createFunc) {
		//DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Entry point of Communicator not found"));
	}

	//mControler = createFunc();

	if (!mControler) {
		//DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("RoutingPlugin initialization failed. Entry Function not callable"));
	}

}



ControlLoader::~ControlLoader()
{
}



ControlSendInterface *ControlLoader::returnControl()
{
	return mControler;
}

