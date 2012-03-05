/*
 * CAmControlReceiverShadowTest.h
 *
 *  Created on: Mar 2, 2012
 *      Author: christian
 */

#ifndef CAMCONTROLRECEIVERSHADOWTEST_H_
#define CAMCONTROLRECEIVERSHADOWTEST_H_

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "MockIAmControlReceive.h"
#include "shared/CAmSocketHandler.h"
#include "shared/CAmSerializer.h"
#include "../include/IAmControlReceiverShadow.h"

namespace am
{

class CAmControlReceiverShadowTest: public ::testing::Test
{
public:
    CAmSocketHandler psocketHandler;
    MockIAmControlReceive pMockReceive;
    IAmControlReceiverShadow pShadow;
    void timerCallback(sh_timerHandle_t handle, void* userData);
    TAmShTimerCallBack<CAmControlReceiverShadowTest> ptimerCallback;
    CAmControlReceiverShadowTest();
    ~CAmControlReceiverShadowTest();
    void SetUp();
    void TearDown();
};

} /* namespace am */
#endif /* CAMCONTROLRECEIVERSHADOWTEST_H_ */
