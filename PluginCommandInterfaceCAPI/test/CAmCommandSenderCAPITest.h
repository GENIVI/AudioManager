/**
 *  Copyright (c) 2012 BMW
 *
 *  \author Aleksandar Donchev, aleksander.donchev@partner.bmw.de BMW 2013
 *
 *  \copyright
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction,
 *  including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 *  subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 *  THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *  For further information see http://www.genivi.org/.
 */

#ifndef COMMANDPLUGININTERFACETEST_H_
#define COMMANDPLUGININTERFACETEST_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "shared/CAmSocketHandler.h"
#include "CAmTestCAPIWrapper.h"
#include "../include/CAmCommandSenderCAPI.h"
#include "MockIAmCommandReceive.h"
#include <../src-gen/org/genivi/audiomanager/CommandInterfaceProxy.h>

#define UNIT_TEST 1

using namespace testing;
using namespace CommonAPI;
namespace am {

class CAmCommandSenderDbusBackdoor;
class IAmCommandSend;

class CAmTestsEnvironment : public ::testing::Environment
{
	pthread_t mListenerThread;
	pthread_t mServicePThread;
	pthread_t mClientPThread;

public:
	CAmSocketHandler *mSocketHandlerService;
	CAmSocketHandler *mSocketHandlerClient;
	bool mIsProxyInitilized;
	bool mIsServiceAvailable;
	MockIAmCommandReceive *mpCommandReceive;
	CAmCommandSenderCAPI *mpPlugin;

	std::shared_ptr<CommandInterfaceProxy<> >  mProxy;

	CAmTestsEnvironment();
    ~CAmTestsEnvironment();
    void SetUp();
    // Override this to define how to tear down the environment.
    void TearDown();
    void onServiceStatusEvent(const CommonAPI::AvailabilityStatus& serviceStatus);
};


class CAmCommandSenderCAPITest :public ::testing::Test
{

public:
	CAmCommandSenderCAPITest();
	~CAmCommandSenderCAPITest();

	void SetUp();
	void TearDown();

};

}

#endif /* COMMANDPLUGININTERFACETEST_H_ */
