/*
 * ControlInterfaceBackdoor.h
 *
 *  Created on: Dec 11, 2011
 *      Author: christian
 */

#ifndef CONTROLINTERFACEBACKDOOR_H_
#define CONTROLINTERFACEBACKDOOR_H_

#include "control/ControlSendInterface.h"
#include "ControlSender.h"

class ControlSender;

using namespace am;

class ControlInterfaceBackdoor {
public:
	ControlInterfaceBackdoor();
	virtual ~ControlInterfaceBackdoor();
	bool replaceController(ControlSender *controlSender, ControlSendInterface *newController);
};

#endif /* CONTROLINTERFACEBACKDOOR_H_ */
