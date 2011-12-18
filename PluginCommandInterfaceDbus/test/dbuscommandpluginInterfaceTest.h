/*
 * dbuscommandpluginInterfaceTest.h
 *
 *  Created on: Dec 14, 2011
 *      Author: christian
 */

#ifndef DBUSCOMMANDPLUGININTERFACETEST_H_
#define DBUSCOMMANDPLUGININTERFACETEST_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <dlt/dlt.h>
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include "CommandReceiver.h"
#include "CommandSender.h"
#include "MockInterfaces.h"
#include "DbusCommandInterfaceBackdoor.h"


#define DEFAULT_PLUGIN_COMMAND_DIR "/home/christian/workspace/gitserver/build/plugins/command"
#define UNIT_TEST 1

namespace am {

class DbusCommandInterfaceBackdoor;

/*
 * originally, I would want to have several tests, but there are problems implementing this with
 * Dbus.
 * I use python to send put the messages and to check the returns I get from the Plugin.
 *
 */


class dbuscommandpluginInterfaceTest :public ::testing::Test
{
public:
	dbuscommandpluginInterfaceTest();
	~dbuscommandpluginInterfaceTest();

	void SetUp();
	void TearDown();
};

}

#endif /* DBUSCOMMANDPLUGININTERFACETEST_H_ */
