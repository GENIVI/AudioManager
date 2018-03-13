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


bool timeoutToTimespec(const int64_t & localTimeout, timespec & pollTimeout)
{    
    if(CommonAPI::TIMEOUT_INFINITE == localTimeout)//dispatch never
    {
        return false;
    }
    else
    {
        if(CommonAPI::TIMEOUT_NONE==localTimeout)//dispatch immediately
        {
            pollTimeout.tv_sec = 0;
            pollTimeout.tv_nsec = 5000000;//5 ms
        }
        else
        {
            pollTimeout.tv_sec = localTimeout / 1000;
            pollTimeout.tv_nsec = (localTimeout % 1000) * 1000000;
        }
        return true;
    }
}


CAmCommonAPIWrapper::CAmCommonAPIWrapper(CAmSocketHandler* socketHandler, const std::string & applicationName):
				pCommonPrepareCallback(this,&CAmCommonAPIWrapper::commonPrepareCallback), //
		        pCommonFireCallback(this, &CAmCommonAPIWrapper::commonFireCallback), //
		        pCommonCheckCallback(this, &CAmCommonAPIWrapper::commonCheckCallback), //
                pCommonDispatchCallback(this, &CAmCommonAPIWrapper::commonDispatchCallback), //
		        pCommonTimerCallback(this, &CAmCommonAPIWrapper::commonTimerCallback), //
                mpSocketHandler(socketHandler),
                mRegisteredDispatchSources(),
                mMapWatches(),
                mSourcesToDispatch(),
                mListTimerhandles()
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
    deregisterAllDispatchSource();
    deregisterAllTimeouts();
    deregisterAllWatches();
	mContext.reset();
	mpSocketHandler = NULL;
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

void CAmCommonAPIWrapper::commonPrepareCallback(const sh_pollHandle_t, void*)
{
    for (auto dispatchSourceIterator = mRegisteredDispatchSources.begin();
            dispatchSourceIterator != mRegisteredDispatchSources.end();
            dispatchSourceIterator++)
    {
        int64_t dispatchTimeout(CommonAPI::TIMEOUT_INFINITE);
        if((*dispatchSourceIterator)->prepare(dispatchTimeout))
        {
            while ((*dispatchSourceIterator)->dispatch());
        }
    }
}

void CAmCommonAPIWrapper::commonFireCallback(const pollfd pollfd, const sh_pollHandle_t handle, void *)
{
    CommonAPI::Watch* pWatchToCheck = watchWithHandle(handle);
    if( pWatchToCheck )   
        pWatchToCheck->dispatch(pollfd.revents);
}

bool CAmCommonAPIWrapper::commonCheckCallback(const sh_pollHandle_t handle, void *)
{
    CommonAPI::Watch* pWatchToCheck = watchWithHandle(handle);
    if( pWatchToCheck )
    {
        const ArrayDispatchSources & vecDispatch = pWatchToCheck->getDependentDispatchSources();
        if(vecDispatch.size()>0)
        {
            mSourcesToDispatch[handle].insert(mSourcesToDispatch[handle].end(), vecDispatch.begin(), vecDispatch.end());
            return true;
        }
    }
    return false;
}

bool CAmCommonAPIWrapper::commonDispatchCallback(const sh_pollHandle_t handle, void *)
{
    CommonAPI::Watch* pWatchToCheck = watchWithHandle(handle);
    if( pWatchToCheck )
    {        
        std::list<CommonAPI::DispatchSource*> & srcList = mSourcesToDispatch[handle];
        for(auto it = srcList.begin();it!=srcList.end();)
        {
            if (false==(*it)->dispatch())
                it=srcList.erase(it);
            else
                it++;
        }
        if (!srcList.empty())
            return (true);
    }
    mSourcesToDispatch.erase(handle);
    return false;
}

void CAmCommonAPIWrapper::commonTimerCallback(sh_timerHandle_t handle, void *)
{
    CommonAPI::Timeout* pTimeout = timeoutWithHandle(handle);
    
    if( NULL==pTimeout )
    {
        //erroneous call because deregisterTimeout has been called, so try to remove the timer from the sockethandler
        mpSocketHandler->removeTimer(handle);
    }
    else
    {
        if ( false==pTimeout->dispatch() ) //it should be removed
        {               
            mpSocketHandler->removeTimer(handle);
            mListTimerhandles.erase(handle);
        }
    #ifndef WITH_TIMERFD
        else //the timeout should be rescheduled 
            mpSocketHandler->restartTimer(handle);           
    #endif        
    }
}

void CAmCommonAPIWrapper::registerDispatchSource(CommonAPI::DispatchSource* dispatchSource, const CommonAPI::DispatchPriority)
{
    mRegisteredDispatchSources.push_back(dispatchSource);
}

void CAmCommonAPIWrapper::deregisterDispatchSource(CommonAPI::DispatchSource* dispatchSource)
{
    for(IteratorArrayDispatchSources dispatchSourceIterator = mRegisteredDispatchSources.begin(); dispatchSourceIterator != mRegisteredDispatchSources.end(); dispatchSourceIterator++) 
    {
        if( *dispatchSourceIterator == dispatchSource ) 
        {
            mRegisteredDispatchSources.erase(dispatchSourceIterator);
            break;
        }
    }
}

void CAmCommonAPIWrapper::deregisterAllDispatchSource()
{
    mRegisteredDispatchSources.clear();
}

void CAmCommonAPIWrapper::registerWatch(CommonAPI::Watch* watch, const CommonAPI::DispatchPriority)
{
    logInfo(__PRETTY_FUNCTION__);
    pollfd pollfd_ (watch->getAssociatedFileDescriptor());
    sh_pollHandle_t handle (0);

    am_Error_e error = mpSocketHandler->addFDPoll(pollfd_.fd, pollfd_.events, &pCommonPrepareCallback, &pCommonFireCallback, &pCommonCheckCallback, &pCommonDispatchCallback, watch, handle);

    //if everything is alright, add the watch and the handle to our map so we know this relationship
    if (error != am_Error_e::E_OK || handle == 0)
    {
        logError(__func__,"entering watch failed");
    }
    else
        mMapWatches.insert(std::make_pair(handle,watch));
}

void CAmCommonAPIWrapper::deregisterWatch(CommonAPI::Watch* watch)
{
    for(IteratorMapWatches iter=mMapWatches.begin();iter!=mMapWatches.end();iter++)
    {
        if (iter->second == watch)
        {
            mpSocketHandler->removeFDPoll(iter->first);
            mMapWatches.erase(iter);
            break;
        }
    }
}

void CAmCommonAPIWrapper::deregisterAllWatches()
{
    for(IteratorMapWatches iter=mMapWatches.begin();iter!=mMapWatches.end();iter++)
        mpSocketHandler->removeFDPoll(iter->first);
    mMapWatches.clear();
}

void CAmCommonAPIWrapper::registerTimeout(CommonAPI::Timeout* timeout, const CommonAPI::DispatchPriority)
{
    timespec pollTimeout;
    if(timeoutToTimespec(timeout->getTimeoutInterval(), pollTimeout))
    {
        //prepare handle and callback. new is eval, but there is no other choice because we need the pointer!
        sh_timerHandle_t handle;

        //add the timer to the pollLoop
        am_Error_e error = mpSocketHandler->addTimer(pollTimeout, &pCommonTimerCallback, handle, timeout, true);
        if (error != am_Error_e::E_OK || handle == 0)
        {
            logError(__func__,"adding timer failed");
        }
        else
        {
            mListTimerhandles.insert(std::make_pair(handle,timeout));
        }
    }
}

void CAmCommonAPIWrapper::deregisterTimeout(CommonAPI::Timeout* timeout)
{
    for( IteratorMapTimeouts iter=mListTimerhandles.begin();iter!= mListTimerhandles.end();iter++)
    {
        if(iter->second==timeout)
        {
            mpSocketHandler->removeTimer(iter->first);
            mListTimerhandles.erase(iter->first);
            break;
        }
    }
}

void CAmCommonAPIWrapper::deregisterAllTimeouts()
{
    for( IteratorMapTimeouts iter=mListTimerhandles.begin();iter!= mListTimerhandles.end();iter++)
        mpSocketHandler->removeTimer(iter->first);
    mListTimerhandles.clear();
}

CommonAPI::Watch* CAmCommonAPIWrapper::watchWithHandle(const sh_pollHandle_t handle)
{
    CommonAPI::Watch* pWatchToCheck = NULL;
    try
    {
        pWatchToCheck = mMapWatches.at(handle);        
    }
    catch (const std::out_of_range& error) 
    {
        logInfo(__PRETTY_FUNCTION__,error.what());
    } 
    return pWatchToCheck;
}

CommonAPI::Timeout* CAmCommonAPIWrapper::timeoutWithHandle(const sh_pollHandle_t handle)
{
    CommonAPI::Timeout* pTimeout = NULL;
    try
    {
        pTimeout = mListTimerhandles.at(handle);        
    }
    catch (const std::out_of_range& error) 
    {
        logInfo(__PRETTY_FUNCTION__,error.what());
    } 
    return pTimeout;
}


CAmCommonAPIWrapper* (*getCAPI)()  = CAmCommonAPIWrapper::getInstance;

}
