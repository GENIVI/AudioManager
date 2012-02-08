/*
 * CAmTelnetServerTest.h
 *
 *  Created on: Feb 7, 2012
 *      Author: Frank Herchet
 */

#ifndef CAMTELNETSERVERTEST_H_
#define CAMTELNETSERVERTEST_H_

#include "gtest/gtest.h"


namespace am {

class CAmTelnetServerTest : public ::testing::Test{
   public:
      CAmTelnetServerTest();
      virtual ~CAmTelnetServerTest();


   virtual void SetUp() ;

   virtual void TearDown() ;

};

}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

#endif /* CAMTELNETSERVERTEST_H_ */
