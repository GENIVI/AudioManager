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
 * \file CAmCommonAPIWrapper.h
 * For further information see http://www.genivi.org/.
 */


#ifndef COMMONAPIWRAPPER_H_
#define COMMONAPIWRAPPER_H_

#include <string>
#include <list>
#include <map>
#include <queue>
#include <memory>
#include <cassert>
#include <CommonAPI/CommonAPI.hpp>
#ifndef COMMONAPI_INTERNAL_COMPILATION
#define COMMONAPI_INTERNAL_COMPILATION
#include <CommonAPI/MainLoopContext.hpp>
#undef COMMONAPI_INTERNAL_COMPILATION
#endif
#include <CommonAPI/Utils.hpp>
#include "audiomanagerconfig.h"
#include "CAmSocketHandler.h"

/**
 * A Common-API wrapper class, which loads the common-api runtime and instantiates all necessary objects.
 * It is implemented as singleton and usually instantiated at the beginning with CAmSocketHandler as parameter.
 * Example: CAmCommonAPIWrapper *pCAPIWrapper = CAmCommonAPIWrapper::instantiateOnce( aSocketHandlerPointer );
 */

namespace am
{

class CAmSocketHandler;

class CAmCommonAPIWrapper
{
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
        CommonAPI::Timeout* timeout;
    };

    CAmSocketHandler *mpSocketHandler; //!< pointer to the sockethandler

    std::shared_ptr<CommonAPI::Runtime> mRuntime;
    std::shared_ptr<CommonAPI::MainLoopContext> mContext;

    CommonAPI::DispatchSourceListenerSubscription mDispatchSourceListenerSubscription;
    CommonAPI::WatchListenerSubscription mWatchListenerSubscription;
    CommonAPI::TimeoutSourceListenerSubscription mTimeoutSourceListenerSubscription;
    CommonAPI::WakeupListenerSubscription mWakeupListenerSubscription;
    std::multimap<CommonAPI::DispatchPriority, CommonAPI::DispatchSource*> mRegisteredDispatchSources;
    std::map<int,CommonAPI::Watch*> mMapWatches;
    CommonAPI::Watch* mWatchToCheck;
    std::list<CommonAPI::DispatchSource*> mSourcesToDispatch;
    std::vector<timerHandles> mpListTimerhandles;

	void registerDispatchSource(CommonAPI::DispatchSource* dispatchSource, const CommonAPI::DispatchPriority dispatchPriority);
	void deregisterDispatchSource(CommonAPI::DispatchSource* dispatchSource);
	void registerWatch(CommonAPI::Watch* watch, const CommonAPI::DispatchPriority dispatchPriority);
	void deregisterWatch(CommonAPI::Watch* watch);
	void registerTimeout(CommonAPI::Timeout* timeout, const CommonAPI::DispatchPriority dispatchPriority);
	void deregisterTimeout(CommonAPI::Timeout* timeout);
	void wakeup();

protected:
    CAmCommonAPIWrapper(CAmSocketHandler* socketHandler, const std::string & applicationName = "") ;

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
	* \brief Deletes the instanciated object
	*/	
	static void deleteInstance();

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
	static CAmCommonAPIWrapper* instantiateOnce(CAmSocketHandler* socketHandler, const std::string & applicationName = "");


	/**
	* \brief Getter for the socket handler.
	*
	* @return Pointer to the socket handler.
	*/
	CAmSocketHandler *getSocketHandler() const { return mpSocketHandler; }
#if COMMONAPI_VERSION_NUMBER >= 300
	/**
	* \brief Register stub objects.
	*
	* Example: std::shared_ptr<ConcreteStubClass> aStub;
	* 		   registerService(	aStub, "local", "com.your_company.instance_name", "service-name");
	*
	* @param shStub: Shared pointer to a stub instance
	* @param domain: A string with the domain name, usually "local"
	* @param instance: Common-api instance string as example "com.your_company.instance_name"
	* @param connectionId: A string connection id, which is used by CommonAPI to group applications
	*
	*/
	template <class TStubImp> bool registerService(const std::shared_ptr<TStubImp> & shStub, const std::string & domain, const std::string & instance, const CommonAPI::ConnectionId_t & connectionId)
	{
		return mRuntime->registerService(domain, instance, shStub, connectionId);
	}
#endif
	/**
	* \brief Register stub objects.
	*
	* Example: std::shared_ptr<ConcreteStubClass> aStub;
	* 		   registerService(	aStub, "local", "com.your_company.instance_name");
	*
	* @param shStub: Shared pointer to a stub instance
	* @param domain: A string with the domain name, usually "local"
	* @param instance: Common-api instance string as example "com.your_company.instance_name"
	*
	*/
	template <class TStubImp> bool registerService(const std::shared_ptr<TStubImp> & shStub, const std::string & domain, const std::string & instance)
	{
		return mRuntime->registerService(domain, instance, shStub, mContext);
	}

	/**
	* \brief Unregister stub objects.
	*
	* @param domain: A string with the domain name, usually "local"
	* @param interface: Common-api interface string as example "com.your_company.interface_name"
	* @param instance: Common-api instance string as example "com.your_company.instance_name"
	*
	*/
	bool unregisterService(const std::string &domain, const std::string &interface, const std::string &instance)
	{
		return mRuntime->unregisterService(domain, interface, instance);
	}

	/**
	* \brief Deprecated method. Instead you should use bool registerService(const std::shared_ptr<TStubImp> & shStub, const std::string & domain, const std::string & instance).
	*
	* Register stub objects.
	*
	* Example: std::shared_ptr<ConcreteStubClass> aStub;
	* 		   registerService(	aStub, "local:com.your_company.interface_name:com.your_company.instance_name");
	*
	* @param shStub: Shared pointer to a stub instance
	* @param address: Complete common-api address as example "local:com.your_company.interface_name:com.your_company.instance_name"
	*
	*/
	template <class TStubImp> bool __attribute__((deprecated)) registerStub(const std::shared_ptr<TStubImp> & shStub, const std::string & address)
 	{
		std::vector<std::string> parts = CommonAPI::split(address, ':');
		assert(parts.size()==3);

		return registerService(shStub, parts[0], parts[2]);
 	}

	/**
	* \brief Deprecated method. Instead you should use bool unregisterService(const std::string &domain, const std::string &interface, const std::string &instance).
	*
	* Unregister stub objects.
	*
	* @param address: Complete common-api address as example "local:com.your_company.interface_name:com.your_company.instance_name"
	*
	*/
	bool __attribute__((deprecated)) unregisterStub(const std::string & address)
	{
		std::vector<std::string> parts = CommonAPI::split(address, ':');
		assert(parts.size()==3);

		return unregisterService(parts[0], parts[1], parts[2]);
 	}

#if COMMONAPI_VERSION_NUMBER >= 300
	/**
	* \brief Build proxy objects.
	*
	* Example: std::shared_ptr<AProxyClass<>> aProxy = buildProxy<AProxyClass>("local", "com.your_company.instance_name", "client-name");
	*
	* @param domain: A string with the domain name, usually "local"
	* @param instance: Common-api instance string as example "com.your_company.instance_name"
	* @param connectionId: A string connection id, which is used by CommonAPI to group applications
	*
	* @return A proxy object.
	*/
	template<template<typename ...> class ProxyClass, typename ... AttributeExtensions>
	std::shared_ptr<ProxyClass<AttributeExtensions...>> buildProxy(const std::string &domain, const std::string &instance, const CommonAPI::ConnectionId_t & connectionId)
	{
		return mRuntime->buildProxy<ProxyClass>(domain, instance, connectionId);
	}
#endif

	/**
	* \brief Build proxy objects.
	*
	* Example: std::shared_ptr<AProxyClass<>> aProxy = buildProxy<AProxyClass>("local", "com.your_company.instance_name");
	*
	* @param domain: A string with the domain name, usually "local"
	* @param instance: Common-api instance string as example "com.your_company.instance_name"
	*
	* @return A proxy object.
	*/
	template<template<typename ...> class ProxyClass, typename ... AttributeExtensions>
	std::shared_ptr<ProxyClass<AttributeExtensions...>> buildProxy(const std::string &domain, const std::string &instance)
	{
		return mRuntime->buildProxy<ProxyClass>(domain, instance, mContext);
	}


	/**
	* \brief  Deprecated method. Instead you should use buildProxy(const std::string &domain, const std::string &instance).
	*
	* Build proxy objects.
	* Example: std::shared_ptr<AProxyClass<>> aProxy = buildProxy<AProxyClass>("local:com.your_company.interface_name:com.your_company.instance_name");
	*
	* @param address: Complete common-api address as example "local:com.your_company.interface_name:com.your_company.instance_name"
	*
	* @return A proxy object.
	*/
	template<template<typename ...> class ProxyClass, typename ... AttributeExtensions>
	std::shared_ptr<ProxyClass<AttributeExtensions...>> __attribute__((deprecated)) buildProxy(const std::string & address)
	{
		std::vector<std::string> parts=CommonAPI::split(address, ':');
		assert(parts.size()==3);

		return buildProxy<ProxyClass>(parts[0], parts[2]);
	}

};


//Alias
extern CAmCommonAPIWrapper* (*getCAPI)();

#ifndef AMCAPI
	#define AMCAPI getCAPI()
#endif

#ifndef AM_CAPI
	#define AM_CAPI getCAPI()
#endif

#ifndef CAPI
	#define CAPI getCAPI()
#endif


}

#endif /* COMMONAPIWRAPPER_H_ */
