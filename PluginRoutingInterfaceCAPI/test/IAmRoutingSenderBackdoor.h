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


#ifndef ROUTINGSENDERINTERFACEBACKDOOR_H_
#define ROUTINGSENDERINTERFACEBACKDOOR_H_

#include "audiomanagertypes.h"

namespace am
{

class CAmRoutingSenderCAPI;

class IAmRoutingSenderBackdoor
{
	CAmRoutingSenderCAPI *routingSender;
public:
	IAmRoutingSenderBackdoor():routingSender(NULL){};
	IAmRoutingSenderBackdoor(CAmRoutingSenderCAPI *aSender):routingSender(aSender){};
    virtual ~IAmRoutingSenderBackdoor(){routingSender=NULL;};

    bool containsDomainWithID(const am_domainID_t & domainID);
    bool containsSourceWithID(const am_sourceID_t & sourceID);
    bool containsSinkWithID(const am_sinkID_t & sinkID);
    bool containsCrossfader(const am_crossfaderID_t & crossfaderID);
    bool containsHandle(const uint16_t & handle);
    bool containsConnection(const am_connectionID_t & connID);
    unsigned domainsCount();
    unsigned sourcesCount();
    unsigned sinksCount();
    unsigned crossfadersCount();
    unsigned handlesCount();
    unsigned connectionsCount();
};

}

#endif /* ROUTINGSENDERINTERFACEBACKDOOR_H_ */
