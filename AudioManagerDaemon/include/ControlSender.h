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

class ControlSender : private ControlSendInterface {
public:
	ControlSender();
	virtual ~ControlSender();
};

#endif /* CONTROLLOADER_H_ */
