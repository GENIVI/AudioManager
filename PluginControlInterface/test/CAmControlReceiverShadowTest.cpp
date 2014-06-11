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

#include "CAmControlReceiverShadowTest.h"

using namespace testing;
using namespace am;

CAmControlReceiverShadowTest::CAmControlReceiverShadowTest() :
        psocketHandler(), //
        pMockReceive(), //
        pShadow(&pMockReceive, &psocketHandler), //
        ptimerCallback(this, &CAmControlReceiverShadowTest::timerCallback)
{
    DefaultValue<am_Error_e>::Set(E_OK); // Sets the default value to be returned.
}

CAmControlReceiverShadowTest::~CAmControlReceiverShadowTest()
{
}

void CAmControlReceiverShadowTest::SetUp()
{
    timespec t;
    t.tv_nsec = 1000000;
    t.tv_sec = 0;

    sh_timerHandle_t handle;

    IAmShTimerCallBack *buf = &ptimerCallback;
    //lets use a timeout so the test will finish
    psocketHandler.addTimer(t, buf, handle, (void*) NULL);
}

void CAmControlReceiverShadowTest::timerCallback(sh_timerHandle_t handle, void* userData)
{
    (void)handle;
    (void)userData;
    psocketHandler.stop_listening();
}

void CAmControlReceiverShadowTest::TearDown()
{
}

void* run_the_loop(void* socketHandlerPtr)
{
    CAmSocketHandler* socketHandler = static_cast<CAmSocketHandler*>(socketHandlerPtr);
    socketHandler->start_listenting();
    return (NULL);
}

TEST_F(CAmControlReceiverShadowTest,getRoute)
{
    pthread_t ptestThread;
    bool onlyfree(true);
    am_sourceID_t sourceID(1);
    am_sinkID_t sinkID(2);
    am_Route_s route;
    am_RoutingElement_s routingElement;
    std::vector<am_RoutingElement_s> route_;
    std::vector<am_Route_s> returnList, List;
    routingElement.sinkID = 1;
    routingElement.sourceID = 2;
    routingElement.domainID = 3;
    routingElement.connectionFormat = CF_GENIVI_ANALOG;
    route_.push_back(routingElement);
    route.sinkID = 1;
    route.sourceID = 2;
    route.route = route_;
    returnList.push_back(route);
    EXPECT_CALL(pMockReceive,getRoute(onlyfree,sourceID,sinkID,_)).WillOnce(DoAll(SetArgReferee<3>(returnList),Return(E_OK)));
    pthread_create(&ptestThread, NULL, run_the_loop, (void*) &psocketHandler);
    ASSERT_EQ(E_OK, pShadow.getRoute(onlyfree, sourceID, sinkID, List));
    pthread_join(ptestThread, NULL);
}

TEST_F(CAmControlReceiverShadowTest,connect)
{
    pthread_t ptestThread;
    am_Handle_s handle, handleReturn;
    handle.handle = 1;
    handle.handleType = H_CONNECT;
    am_connectionID_t connectionID(3), connectionIDReturn;
    am_CustomAvailabilityReason_t connectionFormat(CF_GENIVI_ANALOG);
    am_sourceID_t sourceID(1);
    am_sinkID_t sinkID(2);

    EXPECT_CALL(pMockReceive, connect(_,_, connectionFormat, sourceID, sinkID)).WillOnce(DoAll(SetArgReferee<0>(handle),SetArgReferee<1>(connectionID),Return(E_OK)));
    pthread_create(&ptestThread, NULL, run_the_loop, (void*) &psocketHandler);
    ASSERT_EQ(E_OK,pShadow.connect(handleReturn,connectionIDReturn, connectionFormat, sourceID, sinkID));
    ASSERT_EQ(handleReturn.handle,handle.handle);
    ASSERT_EQ(handleReturn.handleType,handle.handleType);
    ASSERT_EQ(connectionIDReturn,connectionID);
    pthread_join(ptestThread, NULL);
}

TEST_F(CAmControlReceiverShadowTest,disconnect)
{
    pthread_t ptestThread;
    am_Handle_s handle, handleReturn;
    handle.handle = 1;
    handle.handleType = H_CONNECT;
    am_connectionID_t connectionID(3);

    EXPECT_CALL(pMockReceive, disconnect(_,connectionID)).WillOnce(DoAll(SetArgReferee<0>(handle),Return(E_OK)));
    pthread_create(&ptestThread, NULL, run_the_loop, (void*) &psocketHandler);
    ASSERT_EQ(E_OK,pShadow.disconnect(handleReturn,connectionID));
    ASSERT_EQ(handleReturn.handle,handle.handle);
    ASSERT_EQ(handleReturn.handleType,handle.handleType);
    pthread_join(ptestThread, NULL);
}

//todo: implement more tests !

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

