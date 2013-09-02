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

#include "IAmRoutingSenderBackdoor.h"
#include "../include/CAmRoutingSenderCAPI.h"


namespace am {

bool IAmRoutingSenderBackdoor::containsDomainWithID(const am_domainID_t & domainID)
{
	return NULL!=routingSender->mLookupData.getValueForKey(domainID, routingSender->mLookupData.mMapDomains);
}

bool IAmRoutingSenderBackdoor::containsSourceWithID(const am_sourceID_t & sourceID)
{
	return NULL!=routingSender->mLookupData.getValueForKey(sourceID, routingSender->mLookupData.mMapSources);
}

bool IAmRoutingSenderBackdoor::containsSinkWithID(const am_sinkID_t & sinkID)
{
	return NULL!=routingSender->mLookupData.getValueForKey(sinkID, routingSender->mLookupData.mMapSinks);
}

bool IAmRoutingSenderBackdoor::containsCrossfader(const am_crossfaderID_t & crossfaderID)
{
	return routingSender->mLookupData.mMapCrossfaders.find(crossfaderID)!=routingSender->mLookupData.mMapCrossfaders.end();
}

bool IAmRoutingSenderBackdoor::containsHandle(const uint16_t & handle)
{
	return routingSender->mLookupData.mMapHandles.find(handle)!=routingSender->mLookupData.mMapHandles.end();
}

bool IAmRoutingSenderBackdoor::containsConnection(const am_connectionID_t & connID)
{
	return NULL!=routingSender->mLookupData.getValueForKey(connID, routingSender->mLookupData.mMapConnections);
}

unsigned IAmRoutingSenderBackdoor::domainsCount()
{
	return routingSender->mLookupData.mMapDomains.size();
}

unsigned IAmRoutingSenderBackdoor::sourcesCount()
{
	return routingSender->mLookupData.mMapSources.size();
}

unsigned IAmRoutingSenderBackdoor::sinksCount()
{
	return routingSender->mLookupData.mMapSinks.size();
}

unsigned IAmRoutingSenderBackdoor::crossfadersCount()
{
	return routingSender->mLookupData.mMapCrossfaders.size();
}

unsigned IAmRoutingSenderBackdoor::handlesCount()
{
	return routingSender->mLookupData.mMapHandles.size();
}

unsigned IAmRoutingSenderBackdoor::connectionsCount()
{
	return routingSender->mLookupData.mMapConnections.size();
}


}


