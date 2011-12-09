/*
 * ControlLoader.cpp
 *
 *  Created on: Oct 25, 2011
 *      Author: christian
 */

#include "ControlSender.h"
#include "pluginTemplate.h"
#include <string>

ControlSender::ControlSender()
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



ControlSender::~ControlSender()
{
}



ControlSendInterface *ControlSender::returnControl()
{
	return mControler;
}

