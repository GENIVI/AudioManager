/**
 *  Copyright (c) 2012 BMW
 *
 *  \author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
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

#ifndef TESTROUTINGINTERFACEASYNC_H_
#define TESTROUTINGINTERFACEASYNC_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "MockIAmRoutingReceive.h"
#include "shared/CAmSocketHandler.h"
#include "shared/CAmSerializer.h"

#define UNIT_TEST 1

namespace am
{

class CAmEnvironment: public ::testing::Environment
{
public:
    static std::vector<std::string> returnListPlugins();
    static am_Error_e handleDomainRegister(const am_Domain_s& domainData, am_domainID_t& domainID);
    static am_Error_e handleSourceRegister(const am_Source_s& sourceData, am_sourceID_t& sourceID);
    static am_Error_e handleSinkRegister(const am_Sink_s& sinkData, am_sinkID_t& sinkID);
    void timerCallback(sh_timerHandle_t handle, void* userData);
    TAmShTimerCallBack<CAmEnvironment> ptimerCallback;
    CAmEnvironment();
    ~CAmEnvironment();
    // Override this to define how to set up the environment.
    void SetUp();
    // Override this to define how to tear down the environment.
    void TearDown();
private:
    static am_domainID_t mDomainIDCount;
};

class CAmRoutingReceiverAsync: public ::testing::Test
{
public:
    CAmRoutingReceiverAsync();
    ~CAmRoutingReceiverAsync();
    void timerCallback(sh_timerHandle_t handle, void* userData);
    TAmShTimerCallBack<CAmRoutingReceiverAsync> ptimerCallback;
    void SetUp();
    void TearDown();
private:
};

} /* namespace am */
#endif /* TESTROUTINGINTERFACEASYNC_H_ */
