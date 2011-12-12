/*
 * routingInterfaceTest.h
 *
 *  Created on: Dec 10, 2011
 *      Author: christian
 */

#ifndef ROUTINGINTERFACETEST_H_
#define ROUTINGINTERFACETEST_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <dlt/dlt.h>
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include "MockControlInterface.h"
#include "DatabaseHandler.h"
#include "ControlReceiver.h"
#include "ControlSender.h"
#include "Observer.h"
#include "RoutingReceiver.h"
#include "../RoutingInterfaceBackdoor.h"
#include "../CommandInterfaceBackdoor.h"
#include "../CommonFunctions.h"

DLT_DECLARE_CONTEXT(AudioManager)

using namespace testing;
using namespace am;

class controlInterfaceTest : public Test{
public:
	controlInterfaceTest();
	virtual ~controlInterfaceTest();
	DatabaseHandler pDatabaseHandler;
	RoutingSender pRoutingSender;
	CommandSender pCommandSender;
	MockControlSendInterface pMockInterface;
	ControlSender pControlSender;
	RoutingInterfaceBackdoor pRoutingInterfaceBackdoor;
	CommandInterfaceBackdoor pCommandInterfaceBackdoor;
	ControlInterfaceBackdoor pControlInterfaceBackdoor;
	Observer pObserver;
	ControlReceiver pControlReceiver;
	RoutingReceiver pRoutingReceiver;
	CommonFunctions pCF;
	void SetUp();
	void TearDown();
};

#endif /* ROUTINGINTERFACETEST_H_ */
