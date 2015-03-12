/**
 *  Copyright (C) 2012, BMW AG
 *
 *  \author Christian Linke, christian.linke@bmw.de BMW 2011,2012
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
 *  \file CAmCommonAPIWrapper.cpp
 *  For further information see http://www.genivi.org/.
 */


#include <config.h>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include <poll.h>
#include <tuple>
#include "audiomanagertypes.h"
#include "CAmSocketHandler.h"
#include "CAmDltWrapper.h"
#include "CAmCommonAPIWrapper.h"


namespace am
{
static CAmCommonAPIWrapper* pSingleCommonAPIInstance = NULL;


using namespace CommonAPI;

CAmCommonAPIWrapper::CAmCommonAPIWrapper(CAmSocketHandler* socketHandler):
				pCommonPrepareCallback(this,&CAmCommonAPIWrapper::commonPrepareCallback), //
		        pCommonDispatchCallback(this, &CAmCommonAPIWrapper::commonDispatchCallback), //
		        pCommonFireCallback(this, &CAmCommonAPIWrapper::commonFireCallback), //
		        pCommonCheckCallback(this, &CAmCommonAPIWrapper::commonCheckCallback), //
		        pCommonTimerCallback(this, &CAmCommonAPIWrapper::commonTimerCallback), //
		        mpSocketHandler(socketHandler), //
		        mWatchToCheck(NULL)
{
	assert(NULL!=socketHandler);
//1. Load the runtime
	std::shared_ptr<CommonAPI::Runtime> runtime = CommonAPI::Runtime::load();
//2. Get the context and store a pointer to it
	mContext = runtime->getNewMainLoopContext();
//3. Make subscriptions
   mDispatchSourceListenerSubscription = mContext->subscribeForDispatchSources(
			std::bind(&CAmCommonAPIWrapper::registerDispatchSource, this, std::placeholders::_1, std::placeholders::_2),
			std::bind(&CAmCommonAPIWrapper::deregisterDispatchSource, this, std::placeholders::_1));
	mWatchListenerSubscription = mContext->subscribeForWatches(
			std::bind(&CAmCommonAPIWrapper::registerWatch, this, std::placeholders::_1, std::placeholders::_2),
			std::bind(&CAmCommonAPIWrapper::deregisterWatch, this, std::placeholders::_1));
	mTimeoutSourceListenerSubscription = mContext->subscribeForTimeouts(
			std::bind(&CAmCommonAPIWrapper::registerTimeout, this, std::placeholders::_1, std::placeholders::_2),
			std::bind(&CAmCommonAPIWrapper::deregisterTimeout, this, std::placeholders::_1));
//4. Create the factory
	std::shared_ptr<CommonAPI::Factory> factory = runtime->createFactory(mContext);
	assert(factory);
	logInfo(__PRETTY_FUNCTION__,"CommonAPI -> Factory created");
	mFactory = factory;
//5. Get the publisher V.2.1
//	std::shared_ptr<CommonAPI::ServicePublisher> servicePublisher = runtime->getServicePublisher();
//	assert(servicePublisher);
//	logInfo(__PRETTY_FUNCTION__,"CommonAPI -> Publisher available");
//6. Instantiate your concrete stub implementations
//	std::shared_ptr<StubImpl> theStub = std::make_shared<StubImpl>(1);
//7. Register the services
//	std::string capiAddress("local:org.genivi.audiomanager.sourcestate:de.bmw.infotainment.broadcast.ta");
//	registerStub(theStub, capiAddress);
}

CAmCommonAPIWrapper::~CAmCommonAPIWrapper()
{
	mContext->unsubscribeForDispatchSources(mDispatchSourceListenerSubscription);
	mContext->unsubscribeForWatches(mWatchListenerSubscription);
	mContext->unsubscribeForTimeouts(mTimeoutSourceListenerSubscription);
//The following objects must be released in the given order.
	mFactory.reset();
	mContext.reset();

	mpSocketHandler = NULL;
	mWatchToCheck = NULL;
}

CAmCommonAPIWrapper* CAmCommonAPIWrapper::instantiateOnce(CAmSocketHandler* socketHandler)
{
	if(NULL==pSingleCommonAPIInstance)
	{
		if(NULL==socketHandler)
			throw std::runtime_error(std::string("Expected a valid socket handler. The socket handler pointer must not be NULL."));
		else
			pSingleCommonAPIInstance = new CAmCommonAPIWrapper(socketHandler);
	}
	else
		throw std::logic_error(std::string("The singleton instance has been already instantiated. This method should be called only once."));
	return pSingleCommonAPIInstance;
}

CAmCommonAPIWrapper* CAmCommonAPIWrapper::getInstance()
{
	assert(NULL!=pSingleCommonAPIInstance);
	return pSingleCommonAPIInstance;
}

std::shared_ptr<CommonAPI::Factory> CAmCommonAPIWrapper::factory() const
{
	return mFactory;
}


std::shared_ptr<CommonAPI::Runtime> CAmCommonAPIWrapper::runtime() const
{
	return  mFactory->getRuntime();
}

bool CAmCommonAPIWrapper::commonDispatchCallback(const sh_pollHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;

    std::list<DispatchSource*>::iterator iterator(mSourcesToDispatch.begin());
    for(;iterator!=mSourcesToDispatch.end();)
    {
        DispatchSource* source = *iterator;
        if (!source->dispatch()) {
            iterator=mSourcesToDispatch.erase(iterator);
        }
        else
            iterator++;
    }
    if (!mSourcesToDispatch.empty())
        return (true);

    return false;
}

bool CAmCommonAPIWrapper::commonCheckCallback(const sh_pollHandle_t, void *)
{
    std::vector<DispatchSource*> vecDispatch=mWatchToCheck->getDependentDispatchSources();
    mSourcesToDispatch.insert(mSourcesToDispatch.end(), vecDispatch.begin(), vecDispatch.end());

    return (mWatchToCheck || !mSourcesToDispatch.empty());
}

void CAmCommonAPIWrapper::commonFireCallback(const pollfd pollfd, const sh_pollHandle_t, void *)
{
    mWatchToCheck=NULL;
    try
    {
        mWatchToCheck=mMapWatches.at(pollfd.fd);
    }
    catch (const std::out_of_range& error) {
      logInfo(__PRETTY_FUNCTION__,error.what());
      return;
    }

    mWatchToCheck->dispatch(pollfd.events);
}

void CAmCommonAPIWrapper::commonPrepareCallback(const sh_pollHandle_t, void*)
{
    for (auto dispatchSourceIterator = mRegisteredDispatchSources.begin();
                            dispatchSourceIterator != mRegisteredDispatchSources.end();
                            dispatchSourceIterator++)
    {
        int64_t dispatchTimeout(TIMEOUT_INFINITE);
        if(dispatchSourceIterator->second->prepare(dispatchTimeout))
        {
            while (dispatchSourceIterator->second->dispatch());
        }
    }
}

void CAmCommonAPIWrapper::registerDispatchSource(DispatchSource* dispatchSource, const DispatchPriority dispatchPriority)
{
    mRegisteredDispatchSources.insert({dispatchPriority, dispatchSource});
}

void CAmCommonAPIWrapper::deregisterDispatchSource(DispatchSource* dispatchSource)
{
    for(auto dispatchSourceIterator = mRegisteredDispatchSources.begin();
            dispatchSourceIterator != mRegisteredDispatchSources.end();
            dispatchSourceIterator++) {

        if(dispatchSourceIterator->second == dispatchSource) {
            mRegisteredDispatchSources.erase(dispatchSourceIterator);
            break;
        }
    }
}

void CAmCommonAPIWrapper::deregisterWatch(Watch* watch)
{
    logInfo(__PRETTY_FUNCTION__);
    for(std::map<int,Watch*>::iterator iter(mMapWatches.begin());iter!=mMapWatches.end();iter++)
    {
        if (iter->second == watch)
        {
            mMapWatches.erase(iter);
            break;
        }
    }
}

void CAmCommonAPIWrapper::registerTimeout(Timeout* timeout, const DispatchPriority)
{
    logInfo(__PRETTY_FUNCTION__);
    timespec pollTimeout;
    int64_t localTimeout = timeout->getTimeoutInterval();

    pollTimeout.tv_sec = localTimeout / 1000;
    pollTimeout.tv_nsec = (localTimeout % 1000) * 1000000;

    //prepare handle and callback. new is eval, but there is no other choice because we need the pointer!
    sh_timerHandle_t handle;
    timerHandles myHandle({handle,timeout});
    mpListTimerhandles.push_back(myHandle);

    //add the timer to the pollLoop
    mpSocketHandler->addTimer(pollTimeout, &pCommonTimerCallback, handle, timeout);

    return;
}

void CAmCommonAPIWrapper::deregisterTimeout(Timeout* timeout)
{
    logInfo(__PRETTY_FUNCTION__);
    for( std::vector<timerHandles>::iterator iter(mpListTimerhandles.begin());iter!=mpListTimerhandles.end();iter++)
    {
        if(iter->timeout==timeout)
        {
            mpSocketHandler->removeTimer(iter->handle);
        }
    }
}

void CAmCommonAPIWrapper::registerWatch(Watch* watch, const DispatchPriority)
{
    logInfo(__PRETTY_FUNCTION__);
    pollfd pollfd_ (watch->getAssociatedFileDescriptor());
    sh_pollHandle_t handle (0);

    am_Error_e error = mpSocketHandler->addFDPoll(pollfd_.fd, pollfd_.events, &pCommonPrepareCallback, &pCommonFireCallback, &pCommonCheckCallback, &pCommonDispatchCallback, watch, handle);

    //if everything is alright, add the watch and the handle to our map so we know this relationship
    if (error == !am_Error_e::E_OK || handle == 0)
        logError(__PRETTY_FUNCTION__,"entering watch failed");

    mMapWatches.insert(std::make_pair(pollfd_.fd,watch));
}

void CAmCommonAPIWrapper::commonTimerCallback(sh_timerHandle_t handle, void *)
{
    for( std::vector<timerHandles>::iterator iter(mpListTimerhandles.begin());iter!=mpListTimerhandles.end();iter++)
    {
        if(iter->handle==handle)
        {
            iter->timeout->dispatch();
        }
    }
}

}
