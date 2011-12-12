/*
 * databasetest.h
 *
 *  Created on: Dec 6, 2011
 *      Author: christian
 */

#ifndef DATABASETEST_H_
#define DATABASETEST_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <dlt/dlt.h>
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include "DatabaseHandler.h"
#include "../CommonFunctions.h"

DLT_DECLARE_CONTEXT(AudioManager)

using namespace testing;

class databaseTest : public Test {
public:

	DatabaseHandler pDatabaseHandler;
	CommonFunctions pCF;

	void SetUp();
	void TearDown();

	void createMainConnectionSetup();
};

#endif /* DATABASETEST_H_ */
