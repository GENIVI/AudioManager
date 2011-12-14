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
#include "MockInterfaces.h"
#include "DatabaseHandler.h"
#include "ControlReceiver.h"
#include "RoutingReceiver.h"
#include "DatabaseObserver.h"
#include "ControlSender.h"
#include "RoutingSender.h"
#include "../RoutingInterfaceBackdoor.h"
#include "../CommandInterfaceBackdoor.h"
#include "../ControlInterfaceBackdoor.h"
#include "../CommonFunctions.h"

DLT_DECLARE_CONTEXT(AudioManager)

using namespace testing;
using namespace am;

using ::testing::Field;
using ::testing::Property;
using ::testing::Matcher;
using ::testing::Pointee;
using ::testing::AllOf;
using ::testing::SafeMatcherCast;
using ::testing::MatcherCast;
using ::testing::DefaultValue;
using ::testing::Eq;
using ::testing::An;
using ::testing::ElementsAreArray;
using ::testing::ElementsAre;
using ::testing::NotNull;
using ::testing::Assign;
using ::testing::SetArgReferee;
using ::testing::DoAll;
using ::testing::MatcherInterface;
using ::testing::MatchResultListener;

class controlInterfaceTest : public Test{
public:
	controlInterfaceTest();
	~controlInterfaceTest();
	std::vector<std::string> plistRoutingPluginDirs;
	std::vector<std::string> plistCommandPluginDirs;
	DatabaseHandler pDatabaseHandler;
	RoutingSender pRoutingSender;
	CommandSender pCommandSender;
	MockRoutingSendInterface pMockRoutingInterface;
	MockControlSendInterface pMockControlInterface;
	ControlSender pControlSender;
	RoutingInterfaceBackdoor pRoutingInterfaceBackdoor;
	CommandInterfaceBackdoor pCommandInterfaceBackdoor;
	ControlInterfaceBackdoor pControlInterfaceBackdoor;
	DatabaseObserver pDatabaseObserver;
	ControlReceiver pControlReceiver;
	RoutingReceiver pRoutingReceiver;
	CommonFunctions pCF;
	void SetUp();
	void TearDown();
};

#endif /* ROUTINGINTERFACETEST_H_ */
