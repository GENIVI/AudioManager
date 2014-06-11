/**
 *  Copyright (C) 2012, BMW AG
 *
 *  \author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
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
 *  \file CAmCommonAPIWrapper.h
 *  For further information see http://www.genivi.org/.
 */


#ifndef COMMONAPIWRAPPER_H_
#define COMMONAPIWRAPPER_H_

#include <string>
#include <list>
#include <map>
#include <queue>
#include <memory>
#include <CommonAPI/CommonAPI.h>
#include "config.h"
#include "CAmSocketHandler.h"


/**
 * A Common-API wrapper class, that loads the common-api runtime and instantiates all necessary other objects. Works with the CAmSocketHandler.
 * It is implemented as singleton and usually once instantiated at the beginning with CAmSocketHandler.
 * Example: CAmCommonAPIWrapper *pCAPIWrapper = CAmCommonAPIWrapper::instantiateOnce( aSocketHandlerPointer );
 */

namespace am
{
using namespace CommonAPI;

class CAmSocketHandler;

class CAmCommonAPIWrapper
{
public:

    virtual ~CAmCommonAPIWrapper();
	/**
	* \brief Returns an already instantiated object.
	*
	* This method should be called after the instantiateOnce(...) has been called with non null socket handler parameter.
	*
	* @return The common-api wrapper object.
	*/
	static CAmCommonAPIWrapper* getInstance();
	/**
	* \brief Creates a singleton instance attached to the provided socket handler object.
	*
	* This method should be called only once because it instantiates a single object.
	* Otherwise it will throw an exception.
	* The first call of this method with non null parameter loads the common-api and attaches it to the main loop.
	*
	* @param socketHandler: A pointer to socket handler or NULL
	*
	* @return The common-api wrapper object.
	*/
	static CAmCommonAPIWrapper* instantiateOnce(CAmSocketHandler* socketHandler);

	void registerDispatchSource(DispatchSource* dispatchSource, const DispatchPriority dispatchPriority);
	void deregisterDispatchSource(DispatchSource* dispatchSource);
	void registerWatch(Watch* watch, const DispatchPriority dispatchPriority);
	void deregisterWatch(Watch* watch);
	void registerTimeout(Timeout* timeout, const DispatchPriority dispatchPriority);
	void deregisterTimeout(Timeout* timeout);
	void wakeup();

	std::shared_ptr<CommonAPI::Factory> factory() const;
	std::shared_ptr<CommonAPI::Runtime> runtime() const;
	//Wraps the invitation to the service publisher
	template <class TStubImp> bool registerStub(const std::shared_ptr<TStubImp> & shStub, const std::string & aCommonAPIAddress)
	{
		return runtime()->getServicePublisher()->registerService(shStub, aCommonAPIAddress, factory());
	}
	bool unregisterStub(const std::string & aCommonAPIAddress)
	{
		(void)aCommonAPIAddress;
		/** Not implemented yet
			todo: Check whether the appropriate method is available and uncomment...

			return runtime()->getServicePublisher()->unregisterService(aCommonAPIAddress);
		*/
		return true;
	}


protected:
	CAmCommonAPIWrapper(CAmSocketHandler* socketHandler) ;
private:
    void commonPrepareCallback(const sh_pollHandle_t handle, void* userData);
	TAmShPollPrepare<CAmCommonAPIWrapper> pCommonPrepareCallback;

    bool commonDispatchCallback(const sh_pollHandle_t handle, void* userData);
    TAmShPollDispatch<CAmCommonAPIWrapper> pCommonDispatchCallback;

    void commonFireCallback(const pollfd pollfd, const sh_pollHandle_t, void*);
    TAmShPollFired<CAmCommonAPIWrapper> pCommonFireCallback;

    bool commonCheckCallback(const sh_pollHandle_t handle, void*);
    TAmShPollCheck<CAmCommonAPIWrapper> pCommonCheckCallback;

    void commonTimerCallback(sh_timerHandle_t handle, void* userData);
    TAmShTimerCallBack<CAmCommonAPIWrapper> pCommonTimerCallback;

     struct timerHandles
    {
        sh_timerHandle_t handle;
        Timeout* timeout;
    };
     //!< reference to the dbus instance
    CAmSocketHandler *mpSocketHandler; //!< pointer to the sockethandler

    std::shared_ptr<CommonAPI::Factory> mFactory;
    std::shared_ptr<CommonAPI::MainLoopContext> mContext;

    DispatchSourceListenerSubscription mDispatchSourceListenerSubscription;
    WatchListenerSubscription mWatchListenerSubscription;
    TimeoutSourceListenerSubscription mTimeoutSourceListenerSubscription;
    WakeupListenerSubscription mWakeupListenerSubscription;
    std::multimap<DispatchPriority, DispatchSource*> mRegisteredDispatchSources;
    std::map<int,Watch*> mMapWatches;
    Watch* mWatchToCheck;
    std::list<DispatchSource*> mSourcesToDispatch;
    std::vector<timerHandles> mpListTimerhandles;
};

#define Am_CAPI CAmCommonAPIWrapper::getInstance()

}

#endif /* COMMONAPIWRAPPER_H_ */
