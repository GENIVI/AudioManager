/**
* Copyright (C) 2011, BMW AG
*
* GeniviAudioMananger AudioManagerDaemon
*
* \file controlInterfaceTest.h
*
* \date 20-Oct-2011 3:42:04 PM
* \author Christian Mueller (christian.ei.mueller@bmw.de)
*
* \section License
* GNU Lesser General Public License, version 2.1, with special exception (GENIVI clause)
* Copyright (C) 2011, BMW AG Christian Mueller  Christian.ei.mueller@bmw.de
*
* This program is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License, version 2.1, for more details.
* You should have received a copy of the GNU Lesser General Public License, version 2.1, along with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
* Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may also be applicable to programs even in cases in which the program is not a library in the technical sense.
* Linking AudioManager statically or dynamically with other modules is making a combined work based on AudioManager. You may license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to license your linked modules under the GNU Lesser General Public License, version 2.1, you may use the program under the following exception.
* As a special exception, the copyright holders of AudioManager give you permission to combine AudioManager with software programs or libraries that are released under any license unless such a combination is not permitted by the license of such a software program or library. You may copy and distribute such a system following the terms of the GNU Lesser General Public License, version 2.1, including this special exception, for AudioManager and the licenses of the other code concerned.
* Note that people who make modified versions of AudioManager are not obligated to grant this special exception for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, version 2.1, gives permission to release a modified version without this exception; this exception also makes it possible to release a modified version which carries forward this exception.
*
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
