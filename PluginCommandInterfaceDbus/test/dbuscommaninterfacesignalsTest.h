/*
 * dbuscommaninterfacesignalsTest.h
 *
 *  Created on: Dec 16, 2011
 *      Author: christian
 */

#ifndef DBUSCOMMANINTERFACESIGNALSTEST_H_
#define DBUSCOMMANINTERFACESIGNALSTEST_H_
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

#define UNIT_TEST 1

namespace am {

class dbuscommaninterfacesignalsTest: public ::testing::Test
{
public:
	dbuscommaninterfacesignalsTest();
	virtual ~dbuscommaninterfacesignalsTest();
	void SetUp();
	void TearDown();
};

}

#endif /* DBUSCOMMANINTERFACESIGNALSTEST_H_ */
