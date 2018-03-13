/**
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2012, BMW AG
 *
 * \author Christian Linke, christian.linke@bmw.de BMW 2011,2012
 * \author Aleksandar Donchev, aleksander.donchev@partner.bmw.de BMW 2013
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \file CAmCommonAPIWrapper.cpp
 * For further information see http://www.genivi.org/.
 */

#include "audiomanagerconfig.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdlib>
#include <stdexcept>
#include <poll.h>
#include <tuple>
#include <sstream>
#include <vector>
#include "audiomanagertypes.h"
#include "CAmSocketHandler.h"
#include "CAmDltWrapper.h"
#include "CAmCommonAPIWrapper.h"


namespace am
{
static CAmCommonAPIWrapper* pSingleCommonAPIInstance = NULL;


CAmCommonAPIWrapper::CAmCommonAPIWrapper(CAmSocketHandler* socketHandler, const std::string & applicationName):
				pCommonPrepareCallback(this,&CAmCommonAPIWrapper::commonPrepareCallback), //
		        pCommonDispatchCallback(this, &CAmCommonAPIWrapper::commonDispatchCallback), //
		        pCommonFireCallback(this, &CAmCommonAPIWrapper::commonFireCallback), //
		        pCommonCheckCallback(this, &CAmCommonAPIWrapper::commonCheckCallback), //
		        pCommonTimerCallback(this, &CAmCommonAPIWrapper::commonTimerCallback), //
		        mpSocketHandler(socketHandler), //
		        mWatchToCheck(NULL)
{
	assert(NULL!=socketHandler);
//Get the runtime
	mRuntime = CommonAPI::Runtime::get();
	assert(NULL!=mRuntime);

	//Create the context
	if(applicationName.size())
		mContext = std::make_shared<CommonAPI::MainLoopContext>(applicationName);
	else
		mContext = std::make_shared<CommonAPI::MainLoopContext>();
	assert(NULL!=mContext);
	logInfo(__func__,"CommonAPI main loop context with name '", mContext->getName(), "' has been created!");

//Make subscriptions
	mDispatchSourceListenerSubscription = mContext->subscribeForDispatchSources(
			std::bind(&CAmCommonAPIWrapper::registerDispatchSource, this, std::placeholders::_1, std::placeholders::_2),
			std::bind(&CAmCommonAPIWrapper::deregisterDispatchSource, this, std::placeholders::_1));
	mWatchListenerSubscription = mContext->subscribeForWatches(
			std::bind(&CAmCommonAPIWrapper::registerWatch, this, std::placeholders::_1, std::placeholders::_2),
			std::bind(&CAmCommonAPIWrapper::deregisterWatch, this, std::placeholders::_1));
	mTimeoutSourceListenerSubscription = mContext->subscribeForTimeouts(
			std::bind(&CAmCommonAPIWrapper::registerTimeout, this, std::placeholders::_1, std::placeholders::_2),
			std::bind(&CAmCommonAPIWrapper::deregisterTimeout, this, std::placeholders::_1));
}

CAmCommonAPIWrapper::~CAmCommonAPIWrapper()
{
	mContext->unsubscribeForDispatchSources(mDispatchSourceListenerSubscription);
	mContext->unsubscribeForWatches(mWatchListenerSubscription);
	mContext->unsubscribeForTimeouts(mTimeoutSourceListenerSubscription);
	mContext.reset();
	mpSocketHandler = NULL;
	mWatchToCheck = NULL;
}

CAmCommonAPIWrapper* CAmCommonAPIWrapper::instantiateOnce(CAmSocketHandler* socketHandler, const std::string & applicationName)
{
	if(NULL==pSingleCommonAPIInstance)
	{
		if(NULL==socketHandler)
			throw std::runtime_error(std::string("Expected a valid socket handler. The socket handler pointer must not be NULL."));
		else
			pSingleCommonAPIInstance = new CAmCommonAPIWrapper(socketHandler, applicationName);
	}
	else
		throw std::logic_error(std::string("The singleton instance has been already instantiated. This method should be called only once."));
	return pSingleCommonAPIInstance;
}

void CAmCommonAPIWrapper::deleteInstance()
{
	try
	{
		if (pSingleCommonAPIInstance!=NULL)
			delete pSingleCommonAPIInstance;

		pSingleCommonAPIInstance=NULL;
	}
	catch(...)
	{
		logError(__func__,"error while deleting CAPIWrapper instance");
	}
}

CAmCommonAPIWrapper* CAmCommonAPIWrapper::getInstance()
{
	assert(NULL!=pSingleCommonAPIInstance);
	return pSingleCommonAPIInstance;
}

bool CAmCommonAPIWrapper::commonDispatchCallback(const sh_pollHandle_t handle, void *userData)
{
    (void) handle;
    (void) userData;

    std::list<CommonAPI::DispatchSource*>::iterator iterator(mSourcesToDispatch.begin());
    for(;iterator!=mSourcesToDispatch.end();)
    {
    	CommonAPI::DispatchSource* source = *iterator;
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
    std::vector<CommonAPI::DispatchSource*> vecDispatch=mWatchToCheck->getDependentDispatchSources();
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
        int64_t dispatchTimeout(CommonAPI::TIMEOUT_INFINITE);
        if(dispatchSourceIterator->second->prepare(dispatchTimeout))
        {
            while (dispatchSourceIterator->second->dispatch());
        }
    }
}

void CAmCommonAPIWrapper::registerDispatchSource(CommonAPI::DispatchSource* dispatchSource, const CommonAPI::DispatchPriority dispatchPriority)
{
    mRegisteredDispatchSources.insert({dispatchPriority, dispatchSource});
}

void CAmCommonAPIWrapper::deregisterDispatchSource(CommonAPI::DispatchSource* dispatchSource)
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

void CAmCommonAPIWrapper::deregisterWatch(CommonAPI::Watch* watch)
{
    for(std::map<int,CommonAPI::Watch*>::iterator iter(mMapWatches.begin());iter!=mMapWatches.end();iter++)
    {
        if (iter->second == watch)
        {
            mMapWatches.erase(iter);
            break;
        }
    }
}

void CAmCommonAPIWrapper::registerTimeout(CommonAPI::Timeout* timeout, const CommonAPI::DispatchPriority)
{
    timespec pollTimeout;
    int64_t localTimeout = timeout->getTimeoutInterval();

    pollTimeout.tv_sec = localTimeout / 1000;
    pollTimeout.tv_nsec = (localTimeout % 1000) * 1000000;

    //prepare handle and callback. new is eval, but there is no other choice because we need the pointer!
    sh_timerHandle_t handle;

    //add the timer to the pollLoop
    mpSocketHandler->addTimer(pollTimeout, &pCommonTimerCallback, handle, timeout);

    timerHandles myHandle({handle,timeout});
    mpListTimerhandles.push_back(myHandle);

    return;
}

void CAmCommonAPIWrapper::deregisterTimeout(CommonAPI::Timeout* timeout)
{
    for( std::vector<timerHandles>::iterator iter(mpListTimerhandles.begin());iter!=mpListTimerhandles.end();iter++)
    {
        if(iter->timeout==timeout)
        {
            mpSocketHandler->removeTimer(iter->handle);
        }
    }
}

void CAmCommonAPIWrapper::registerWatch(CommonAPI::Watch* watch, const CommonAPI::DispatchPriority)
{
    logInfo(__PRETTY_FUNCTION__);
    pollfd pollfd_ (watch->getAssociatedFileDescriptor());
    sh_pollHandle_t handle (0);

    am_Error_e error = mpSocketHandler->addFDPoll(pollfd_.fd, pollfd_.events, &pCommonPrepareCallback, &pCommonFireCallback, &pCommonCheckCallback, &pCommonDispatchCallback, watch, handle);

    //if everything is alright, add the watch and the handle to our map so we know this relationship
    if (error == !am_Error_e::E_OK || handle == 0)
        logError(__func__,"entering watch failed");

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

CAmCommonAPIWrapper* (*getCAPI)()  = CAmCommonAPIWrapper::getInstance;

}
