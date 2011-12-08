/*
 * ControlLoader.h
 *
 *  Created on: Oct 25, 2011
 *      Author: christian
 */

#ifndef CONTROLLOADER_H_
#define CONTROLLOADER_H_

#include "control/ControlSendInterface.h"

using namespace am;

class ControlLoader {
public:
	ControlLoader();
	virtual ~ControlLoader();
	ControlSendInterface* returnControl();
private:
	ControlSendInterface* mControler;
};

#endif /* CONTROLLOADER_H_ */
