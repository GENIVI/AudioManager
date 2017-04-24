/**
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2012-2017, BMW AG
 *
 * \author Christian Linke, christian.linke@bmw.de BMW 2011,2012
 * \author Alesksandar Donchev, aleksander.donchev@partner.bmw.de BMW 2015, 2017
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * \file CAmSerializer.h
 * For further information see http://www.genivi.org/.
 */

#ifndef CAMSERIALIZER_H_
#define CAMSERIALIZER_H_

#include <pthread.h>
#include <deque>
#include <cassert>
#include <memory>
#include <stdexcept>
#include <unistd.h>
#include "CAmDltWrapper.h"
#include "CAmSocketHandler.h"

/*!
 * \brief Helper structures used within std::bind for automatically identification of all placeholders.
 */
template<std::size_t ... Is>
struct indices
{
};

template<std::size_t N, std::size_t ... Is>
struct build_indices: build_indices<N - 1, N - 1, Is...>
{
};

template<std::size_t ... Is>
struct build_indices<0, Is...> : indices<Is...>
{
};

template<int I> struct placeholder
{
};

namespace std
{
    template<int I>
    struct is_placeholder<::placeholder<I>> : std::integral_constant<int, I>
    {
    };
}

#if defined(__GNUC__)
# define DEPRECATED(MSG) __attribute__ ((__deprecated__((#MSG))))
#else
# define DEPRECATED(MSG)
#endif

namespace am
{
    /**
     * magic class that does the serialization of functions calls
     * The constructor must be called within the main threadcontext, after that using the
     * overloaded template function call will serialize all calls and call them within the
     * main thread context.\n
     * More details can be found here: \ref util
     * \warning asynchronous calls may be used in the mainthread context, but if you want to use synchronous calls make sure that you use one
     * instance of this class per thread otherwise you could be lost in never returning calls.\n
     * Examples of the usage can be found in IAmCommandReceiverShadow of the ControlPlugin or IAmRoutingReceiverShadow of the
     * PluginRoutingInterfaceAsync.
     *
     */

    /**
     * \defgroup Deprecated Obsolete class!
     * @{
     */

    namespace V1
    {
        class CAmSerializer
        {
        private:

            /**
             * Prototype for a delegate
             */
            class CAmDelegate
            {
            public:

                typedef enum
                    :bool
                    {
                        SyncCallType = false, AsyncCallType = true
                } CallType;

                virtual ~CAmDelegate()
                {
                }
                ;
                virtual CallType call(int* pipe)=0;

            };

            /**
             * Prototype for a delegate with variadic template arguments in conjunction with the following class.
             */
            template<class Class, typename Method, typename Tuple, bool Done, int Total, int ... N>
            class CAmDelegateAsyncImpl: public CAmDelegate
            {
                Class mInstance;
                Method mMethod;
                Tuple mArguments;
            public:
                friend class CAmSerializer;
                static void call(Class instance, Method method, Tuple && arguments)
                {
                    CAmDelegateAsyncImpl<Class, Method, Tuple, Total == 1 + sizeof...(N), Total, N..., sizeof...(N)>::call(instance, method, std::forward<Tuple>(arguments));
                }

                CAmDelegateAsyncImpl(Class instance, Method method, Tuple && arguments)
                {
                    mInstance = instance;
                    mMethod = method;
                    mArguments = std::move(arguments);
                }

                CallType call(int* pipe)
                {
                    (void) pipe;
                    call(mInstance, mMethod, std::forward<Tuple>(mArguments));
                    return (AsyncCallType);
                }
                ;
            };

            /**
             * Prototype for a delegate with variadic template arguments.
             */
            template<class Class, typename Method, typename Tuple, int Total, int ... N>
            class CAmDelegateAsyncImpl<Class, Method, Tuple, true, Total, N...> : public CAmDelegate
            {
                Class mInstance;
                Method mMethod;
                Tuple mArguments;
            public:
                friend class CAmSerializer;
                static void call(Class instance, Method method, Tuple && t)
                {
                    (*instance.*method)(std::get<N>(std::forward<Tuple>(t))...);
                }

                CAmDelegateAsyncImpl(Class instance, Method method, Tuple && arguments)
                {
                    mInstance = instance;
                    mMethod = method;
                    mArguments = std::move(arguments);
                }

                CallType call(int* pipe)
                {
                    (void) pipe;
                    call(mInstance, mMethod, std::forward<Tuple>(mArguments));
                    return (AsyncCallType);
                }
                ;
            };

            /**
             * Prototype for a delegate with variadic template arguments in conjunction with the following class.
             */
            template<class Class, typename Method, typename Return, typename Tuple, bool Done, int Total, int ... N>
            class CAmDelegateSyncImpl: public CAmDelegate
            {
                Class mInstance;
                Method mMethod;
                Tuple mArguments;
                Return mReturn;
            public:
                friend class CAmSerializer;
                static void call(Class instance, Method method, Return & result, Tuple && arguments)
                {
                    CAmDelegateSyncImpl<Class, Method, Return, Tuple, Total == 1 + sizeof...(N), Total, N..., sizeof...(N)>::call(instance, method, result, std::forward<Tuple>(arguments));
                }

                CAmDelegateSyncImpl(Class instance, Method method, Tuple && arguments)
                {
                    mInstance = instance;
                    mMethod = method;
                    mArguments = std::move(arguments);
                }

                CallType call(int* pipe)
                {
                    call(mInstance, mMethod, mReturn, std::forward<Tuple>(mArguments));
                    ssize_t result(-1);
                    result = write(pipe[1], this, sizeof(this));
                    if (result == -1)
                        logError("CAmSerializer: Problem writing into pipe! Error No:", errno);
                    return (SyncCallType);
                }
                ;
            };

            /**
             * Prototype for a delegate with variadic template arguments.
             */
            template<class Class, typename Method, typename Return, typename Tuple, int Total, int ... N>
            class CAmDelegateSyncImpl<Class, Method, Return, Tuple, true, Total, N...> : public CAmDelegate
            {
                Class mInstance;
                Method mMethod;
                Tuple mArguments;
                Return mReturn;
            public:
                friend class CAmSerializer;
                static void call(Class instance, Method method, Return & result, Tuple && t)
                {
                    result = (*instance.*method)(std::get<N>(t)...);
                }

                CAmDelegateSyncImpl(Class instance, Method method, Tuple && arguments)
                {
                    mInstance = instance;
                    mMethod = method;
                    mArguments = std::move(arguments);
                }

                CallType call(int* pipe)
                {
                    call(mInstance, mMethod, mReturn, std::forward<Tuple>(mArguments));
                    ssize_t result(-1);
                    result = write(pipe[1], this, sizeof(this));
                    if (result == -1)
                        logError("CAmSerializer: Problem writing into pipe! Error No:", errno);
                    return (SyncCallType);
                }
                ;
            };

            typedef CAmDelegate* CAmDelegagePtr; //!< pointer to a delegate

        public:
            /**
             * instantiates a async delegate with given arguments and sends the delegate pointer over the pipe
             */
            template<typename Class, typename Method, typename Tuple>
            void doAsyncCall(Class intsance, Method method, Tuple & arguments)
            {
                typedef typename std::decay<Tuple>::type ttype;
                typedef CAmDelegateAsyncImpl<Class, Method, Tuple, 0 == std::tuple_size<ttype>::value, std::tuple_size<ttype>::value> AsyncDelegate;
                AsyncDelegate *pImp = new AsyncDelegate(intsance, method, std::forward<Tuple>(arguments));
                send(pImp);
                //Do not delete the pointer. It will be deleted automatically later.
            }

            /**
             * instantiates a sync delegate with given arguments and sends the delegate pointer over the pipe
             */
            template<typename Class, typename Method, typename Return, typename Tuple>
            void doSyncCall(Class intsance, Method method, Return & result, Tuple & arguments)
            {
                typedef typename std::decay<Tuple>::type ttype;
                typedef CAmDelegateSyncImpl<Class, Method, Return, Tuple, 0 == std::tuple_size<ttype>::value, std::tuple_size<ttype>::value> SyncDelegate;
                SyncDelegate *pImp = new SyncDelegate(intsance, method, std::forward<Tuple>(arguments));
                send(pImp);
                int numReads;
                SyncDelegate *p = NULL;
                if ((numReads = read(mReturnPipe[0], &p, sizeof(p))) == -1)
                {
                    logError("CAmSerializer::doSyncCall could not read pipe!");
                    throw std::runtime_error("CAmSerializer Could not read pipe!");
                }
                result = std::move(pImp->mReturn);
                arguments = std::move(pImp->mArguments);
                //Delete the pointer.
                delete pImp;
            }
        private:

            /**
             * rings the line of the pipe and adds the delegate pointer to the queue
             * @param p delegate pointer
             */
            inline void send(CAmDelegagePtr p)
            {
                if (write(mPipe[1], &p, sizeof(p)) == -1)
                {
                    throw std::runtime_error("could not write to pipe !");
                }
            }

            int mPipe[2]; //!< the pipe
            int mReturnPipe[2]; //!< pipe handling returns
            sh_pollHandle_t mHandle;
            CAmSocketHandler* mpSocketHandler;
            std::deque<CAmDelegagePtr> mListDelegatePoiters; //!< intermediate queue to store the pipe results

        public:

            /**
             * get the size of delegate pointers
             */
            int getListDelegatePoiters()
            {
                return mListDelegatePoiters.size();
            }

            /**
             * calls a function with variadic arguments threadsafe
             * @param instance the instance of the class that shall be called
             * @param function the function that shall be called as member function pointer.
             * @param output variable.
             * @tparam TClass the type of the Class to be called
             * @tparam TRet the type of the result
             * @tparam TArgs argument list
             * \section ex Example:
             * @code
             * class MyGreatClass
             * {
             * public:
             *      int AGreatMethod(int x);
             * }
             * CAmSerializer serial(&Sockethandler);
             * MyGreatClass anInstance;
             * int result;
             * serial.syncCall<MyGreatClass, int, int>(&anInstance,&MyGreatClass::AGreatMethod, result, 100);
             * @endcode
             */
            template<class TClass, class TRet, class ... TArgs>
            void syncCall(TClass* instance, TRet (TClass::*method)(TArgs ...), TRet & result, TArgs & ... arguments)
            {
                auto t = std::make_tuple(arguments...);
                doSyncCall(instance, method, result, t);
                std::tie(arguments...) = t;
            }

            /**
             * calls a function with variadic arguments threadsafe
             * @param instance the instance of the class that shall be called
             * @param function the function that shall be called as member function pointer.
             * @tparam TClass the type of the Class to be called
             * @tparam TRet the type of the result
             * @tparam TArgs argument list
             * \section ex Example:
             * @code
             * class MyGreatClass
             * {
             * public:
             *      int AGreatMethod(int x);
             * }
             * CAmSerializer serial(&Sockethandler);
             * MyGreatClass anInstance;
             * serial.asyncCall<MyGreatClass, void, int>(&anInstance,&MyGreatClass::AGreatMethod, 100);
             * @endcode
             */
            template<class TClass, class TRet, class ... TArgs>
            void asyncCall(TClass* instance, TRet (TClass::*method)(TArgs ...), TArgs & ... arguments)
            {
                auto t = std::make_tuple(arguments...);
                doAsyncCall(instance, method, t);
            }

            /**
             * calls a function with no arguments threadsafe
             * @param instance the instance of the class that shall be called
             * @param function the function that shall be called as memberfunction pointer.
             * @tparam TClass1 the type of the Class to be called
             * \section ex Example:
             * @code
             * class myClass
             * {
             * public:
             *      void myfunction();
             * }
             * CAmSerializer serial(&Sockethandler);
             * myClass instanceMyClass;
             * serial<CommandSender>(&instanceMyClass,&myClass::myfunction);
             * @endcode
             */
            template<class TClass>
            void asyncCall(TClass* instance, void (TClass::*function)())
            {
                auto t = std::make_tuple();
                doAsyncCall(instance, function, t);
            }

            /**
             * calls a function with one arguments asynchronously threadsafe
             * @param instance the instance of the class that shall be called
             * @param function the function that shall be called as memberfunction pointer.
             * @param argument the argument
             * @tparam TClass1 the type of the Class to be called
             * @tparam Targ the type of the argument to be called
             * \section ex Example:
             * @code
             * class myClass
             * {
             * public:
             *      void myfunction(int k);
             * }
             * CAmSerializer serial(&Sockethandler);
             * myClass instanceMyClass;
             * serial<CommandSender,int>(&instanceMyClass,&myClass::myfunction,k);
             * @endcode
             *
             */
            template<class TClass1, class Targ>
            void asyncCall(TClass1* instance, void (TClass1::*function)(Targ), Targ argument)
            {
                auto t = std::make_tuple(argument);
                doAsyncCall(instance, function, t);
            }

            /**
             * calls a function with one argument called by reference asynchronously threadsafe
             * @param instance the instance of the class that shall be called
             * @param function the function that shall be called as memberfunction pointer.
             * @param argument the argument
             * @tparam TClass1 the type of the Class to be called
             * @tparam Targ the type of the argument to be called
             * \section ex Example:
             * @code
             * class myClass
             * {
             * public:
             *      void myfunction(int k);
             * }
             * CAmSerializer serial(&Sockethandler);
             * myClass instanceMyClass;
             * serial<CommandSender,int>(&instanceMyClass,&myClass::myfunction,k);
             * @endcode
             *
             */
            template<class TClass1, class Targ>
            void asyncCall(TClass1* instance, void (TClass1::*function)(Targ&), Targ& argument)
            {
                auto t = std::make_tuple(argument);
                doAsyncCall(instance, function, t);
            }

            /**
             * calls a function with two arguments asynchronously threadsafe. for more see asyncCall with one argument
             * @param instance pointer to the instance of the class
             * @param function memberfunction poitner
             * @param argument the first argument
             * @param argument1 the second argument
             * @tparam TClass1 the type of the Class to be called
             * @tparam Targ the type of the argument to be called
             * @tparam Targ1 the type of the first argument to be called
             */
            template<class TClass1, class Targ, class Targ1>
            void asyncCall(TClass1* instance, void (TClass1::*function)(Targ argument, Targ1 argument1), Targ argument, Targ1 argument1)
            {
                auto t = std::make_tuple(argument, argument1);
                doAsyncCall(instance, function, t);
            }

            /**
             * calls a function with two arguments asynchronously threadsafe, first argument is a reference. for more see asyncCall with one argument
             * @param instance pointer to the instance of the class
             * @param function memberfunction poitner
             * @param argument the first argument
             * @param argument1 the second argument
             * @tparam TClass1 the type of the Class to be called
             * @tparam Targ the type of the argument to be called
             * @tparam Targ1 the type of the first argument to be called
             */
            template<class TClass1, class Targ, class Targ1>
            void asyncCall(TClass1* instance, void (TClass1::*function)(Targ& argument, Targ1 argument1), Targ& argument, Targ1 argument1)
            {
                auto t = std::make_tuple(argument, argument1);
                doAsyncCall(instance, function, t);
            }

            /**
             * calls a function with two arguments asynchronously threadsafe, second argument is a reference. for more see asyncCall with one argument
             * @param instance pointer to the instance of the class
             * @param function memberfunction poitner
             * @param argument the first argument
             * @param argument1 the second argument
             * @tparam TClass1 the type of the Class to be called
             * @tparam Targ the type of the argument to be called
             * @tparam Targ1 the type of the first argument to be called
             */
            template<class TClass1, class Targ, class Targ1>
            void asyncCall(TClass1* instance, void (TClass1::*function)(Targ argument, Targ1& argument1), Targ argument, Targ1& argument1)
            {
                auto t = std::make_tuple(argument, argument1);
                doAsyncCall(instance, function, t);
            }

            /**
             * calls a function with two arguments asynchronously threadsafe, both arguments are references. for more see asyncCall with one argument
             * @param instance pointer to the instance of the class
             * @param function memberfunction poitner
             * @param argument the first argument
             * @param argument1 the second argument
             * @tparam TClass1 the type of the Class to be called
             * @tparam Targ the type of the argument to be called
             * @tparam Targ1 the type of the first argument to be called
             */
            template<class TClass1, class Targ, class Targ1>
            void asyncCall(TClass1* instance, void (TClass1::*function)(Targ& argument, Targ1& argument1), Targ& argument, Targ1& argument1)
            {
                auto t = std::make_tuple(argument, argument1);
                doAsyncCall(instance, function, t);
            }

            /**
             * calls a function with three arguments asynchronously threadsafe. for more see other asycCall
             */
            template<class TClass1, class Targ, class Targ1, class Targ2>
            void asyncCall(TClass1* instance, void (TClass1::*function)(Targ argument, Targ1 argument1, Targ2 argument2), Targ argument, Targ1 argument1, Targ2 argument2)
            {
                auto t = std::make_tuple(argument, argument1, argument2);
                doAsyncCall(instance, function, t);
            }

            /**
             * calls a function with three arguments asynchronously threadsafe. for more see other asycCall
             */
            template<class TClass1, class Targ, class Targ1, class Targ2>
            void asyncCall(TClass1* instance, void (TClass1::*function)(Targ& argument, Targ1 argument1, Targ2 argument2), Targ& argument, Targ1 argument1, Targ2 argument2)
            {
                auto t = std::make_tuple(argument, argument1, argument2);
                doAsyncCall(instance, function, t);
            }

            /**
             * calls a function with three arguments asynchronously threadsafe. for more see other asycCall
             */
            template<class TClass1, class Targ, class Targ1, class Targ2>
            void asyncCall(TClass1* instance, void (TClass1::*function)(Targ argument, Targ1& argument1, Targ2 argument2), Targ argument, Targ1& argument1, Targ2 argument2)
            {
                auto t = std::make_tuple(argument, argument1, argument2);
                doAsyncCall(instance, function, t);
            }

            /**
             * calls a function with three arguments asynchronously threadsafe. for more see other asycCall
             */
            template<class TClass1, class Targ, class Targ1, class Targ2>
            void asyncCall(TClass1* instance, void (TClass1::*function)(Targ argument, Targ1 argument1, Targ2& argument2), Targ argument, Targ1 argument1, Targ2& argument2)
            {
                auto t = std::make_tuple(argument, argument1, argument2);
                doAsyncCall(instance, function, t);
            }

            /**
             * calls a function with three arguments asynchronously threadsafe. for more see other asycCall
             */
            template<class TClass1, class Targ, class Targ1, class Targ2>
            void asyncCall(TClass1* instance, void (TClass1::*function)(Targ argument, Targ1& argument1, Targ2& argument2), Targ argument, Targ1& argument1, Targ2& argument2)
            {
                auto t = std::make_tuple(argument, argument1, argument2);
                doAsyncCall(instance, function, t);
            }

            /**
             * calls a function with three arguments asynchronously threadsafe. for more see other asycCall
             */
            template<class TClass1, class Targ, class Targ1, class Targ2>
            void asyncCall(TClass1* instance, void (TClass1::*function)(Targ& argument, Targ1& argument1, Targ2& argument2), Targ& argument, Targ1& argument1, Targ2& argument2)
            {
                auto t = std::make_tuple(argument, argument1, argument2);
                doAsyncCall(instance, function, t);
            }

            /**
             * calls a function with three arguments asynchronously threadsafe. for more see other asycCall
             */
            template<class TClass1, class Targ, class Targ1, class Targ2>
            void asyncCall(TClass1* instance, void (TClass1::*function)(Targ& argument, Targ1& argument1, Targ2 argument2), Targ& argument, Targ1& argument1, Targ2 argument2)
            {
                auto t = std::make_tuple(argument, argument1, argument2);
                doAsyncCall(instance, function, t);
            }

            /**
             * calls a function with three arguments asynchronously threadsafe. for more see other asycCall
             */
            template<class TClass1, class Targ, class Targ1, class Targ2>
            void asyncCall(TClass1* instance, void (TClass1::*function)(Targ& argument, Targ1 argument1, Targ2& argument2), Targ& argument, Targ1 argument1, Targ2& argument2)
            {
                auto t = std::make_tuple(argument, argument1, argument2);
                doAsyncCall(instance, function, t);
            }

            /**
             * calls a function with four arguments asynchronously threadsafe. for more see other asycCall
             */
            template<class TClass1, class Targ, class Targ1, class Targ2, class Targ3>
            void asyncCall(TClass1* instance, void (TClass1::*function)(Targ argument, Targ1 argument1, Targ2 argument2, Targ3 argument3), Targ argument, Targ1 argument1, Targ2 argument2, Targ3 argument3)
            {
                auto t = std::make_tuple(argument, argument1, argument2, argument3);
                doAsyncCall(instance, function, t);
            }

            /**
             * calls a synchronous function with no arguments threadsafe
             * @param instance the instance of the class that shall be called
             * @param function the function that shall be called as memberfunction pointer.
             * @param retVal the return parameter, no const allowed !
             * @tparam TClass1 the type of the class to be called
             * @tparam TretVal the type of the return parameter
             * \section ex Example:
             * @code
             * class myClass
             * {
             * public:
             *      am_Error_e myfunction();
             * }
             * CAmSerializer serial(&Sockethandler);
             * myClass instanceMyClass;
             * am_Error_e error;
             * serial<CommandSender,am_Error_e>(&instanceMyClass,&myClass::myfunction, error);
             * @endcode
             * All arguments given to synchronous functions must be non-const since the results of the operations will be written back to the arguments.
             *
             */
            template<class TClass1, class TretVal>
            void syncCall(TClass1* instance, TretVal (TClass1::*function)(), TretVal& retVal)
            {
                auto t = std::make_tuple();
                doSyncCall(instance, function, retVal, t);
            }

            /**
             * calls a function with one argument synchronous threadsafe
             * @param instance the instance of the class that shall be called
             * @param function the function that shall be called as memberfunction pointer.
             * @param retVal the return parameter, no const allowed !
             * @param argument the argument, no const allowed !
             * @tparam TClass1 the type of the class to be called
             * @tparam TretVal the type of the return parameter
             * @tparam TargCall the type of the argument like in the function to be called. here all references and const must be
             * respected!
             * @tparam Targ the type of the argument, here no const and no references allowed !
             * \section ex Example:
             * @code
             * class myClass
             * {
             * public:
             *      am_Error_e myfunction(int k);
             * }
             * CAmSerializer serial(&Sockethandler);
             * myClass instanceMyClass;
             * am_Error_e error;
             * int l;
             * serial<CommandSender,am_Error_e,int>(&instanceMyClass,&myClass::myfunction,error,l);
             * @endcode
             * All arguments given to synchronous functions must be non-const since the results of the operations will be written back to the arguments.
             */
            template<class TClass1, class TretVal, class TargCall, class Targ>
            void syncCall(TClass1* instance, TretVal (TClass1::*function)(TargCall), TretVal& retVal, Targ& argument)
            {
                auto t = std::make_tuple(argument);
                doSyncCall(instance, function, retVal, t);
                std::tie(argument) = t;
            }

            /**
             * calls a function with one argument synchronous threadsafe for const functions. For more see syncCall with one argument
             */
            template<class TClass1, class TretVal, class TargCall, class Targ>
            void syncCall(TClass1* instance, TretVal (TClass1::*function)(TargCall) const, TretVal& retVal, Targ& argument)
            {
                auto t = std::make_tuple(argument);
                doSyncCall(instance, function, retVal, t);
                std::tie(argument) = t;
            }

            /**
             * calls a function with two arguments synchronously threadsafe. For more see syncCall with one argument
             */
            template<class TClass1, class TretVal, class TargCall, class Targ1Call, class Targ, class Targ1>
            void syncCall(TClass1* instance, TretVal (TClass1::*function)(TargCall, Targ1Call), TretVal& retVal, Targ& argument, Targ1& argument1)
            {
                auto t = std::make_tuple(argument, argument1);
                doSyncCall(instance, function, retVal, t);
                std::tie(argument, argument1) = t;
            }
            /**
             * calls a function with two arguments synchronously threadsafe const. For more see syncCall with one argument
             */
            template<class TClass1, class TretVal, class TargCall, class Targ1Call, class Targ, class Targ1>
            void syncCall(TClass1* instance, TretVal (TClass1::*function)(TargCall, Targ1Call) const, TretVal& retVal, Targ& argument, Targ1& argument1)
            {
                auto t = std::make_tuple(argument, argument1);
                doSyncCall(instance, function, retVal, t);
                std::tie(argument, argument1) = t;
            }

            /**
             * calls a function with three arguments synchronously threadsafe. for more see syncCall with one argument
             */
            template<class TClass1, class TretVal, class TargCall, class TargCall1, class TargCall2, class Targ, class Targ1, class Targ2>
            void syncCall(TClass1* instance, TretVal (TClass1::*function)(TargCall, TargCall1, TargCall2), TretVal& retVal, Targ& argument, Targ1& argument1, Targ2& argument2)
            {
                auto t = std::make_tuple(argument, argument1, argument2);
                doSyncCall(instance, function, retVal, t);
                std::tie(argument, argument1, argument2) = t;
            }

            /**
             * calls a const function with three arguments synchronously threadsafe. for more see syncCall with one argument
             */
            template<class TClass1, class TretVal, class TargCall, class TargCall1, class TargCall2, class Targ, class Targ1, class Targ2>
            void syncCall(TClass1* instance, TretVal (TClass1::*function)(TargCall, TargCall1, TargCall2) const, TretVal& retVal, Targ& argument, Targ1& argument1, Targ2& argument2)
            {
                auto t = std::make_tuple(argument, argument1, argument2);
                doSyncCall(instance, function, retVal, t);
                std::tie(argument, argument1, argument2) = t;
            }

            /**
             * calls a function with four arguments synchronously threadsafe. for more see syncCall with one argument
             */
            template<class TClass1, class TretVal, class TargCall, class TargCall1, class TargCall2, class TargCall3, class Targ, class Targ1, class Targ2, class Targ3>
            void syncCall(TClass1* instance, TretVal (TClass1::*function)(TargCall, TargCall1, TargCall2, TargCall3), TretVal& retVal, Targ& argument, Targ1& argument1, Targ2& argument2, Targ3& argument3)
            {
                auto t = std::make_tuple(argument, argument1, argument2, argument3);
                doSyncCall(instance, function, retVal, t);
                std::tie(argument, argument1, argument2, argument3) = t;
            }

            /**
             * calls a function with five arguments synchronously threadsafe. for more see syncCall with one argument
             */
            template<class TClass1, class TretVal, class TargCall, class TargCall1, class TargCall2, class TargCall3, class TargCall4, class Targ, class Targ1, class Targ2, class Targ3, class Targ4>
            void syncCall(TClass1* instance, TretVal (TClass1::*function)(TargCall, TargCall1, TargCall2, TargCall3, TargCall4), TretVal& retVal, Targ& argument, Targ1& argument1, Targ2& argument2, Targ3& argument3, Targ4& argument4)
            {
                auto t = std::make_tuple(argument, argument1, argument2, argument3, argument4);
                doSyncCall(instance, function, retVal, t);
                std::tie(argument, argument1, argument2, argument3, argument4) = t;
            }

            /**
             * calls a function with six arguments synchronously threadsafe. for more see syncCall with one argument
             */
            template<class TClass1, class TretVal, class TargCall, class TargCall1, class TargCall2, class TargCall3, class TargCall4, class TargCall5, class Targ, class Targ1, class Targ2, class Targ3, class Targ4, class Targ5>
            void syncCall(TClass1* instance, TretVal (TClass1::*function)(TargCall, TargCall1, TargCall2, TargCall3, TargCall4, TargCall5), TretVal& retVal, Targ& argument, Targ1& argument1, Targ2& argument2, Targ3& argument3,
                    Targ4& argument4, Targ5& argument5)
            {
                auto t = std::make_tuple(argument, argument1, argument2, argument3, argument4, argument5);
                doSyncCall(instance, function, retVal, t);
                std::tie(argument, argument1, argument2, argument3, argument4, argument5) = t;
            }

            /**
             * receiver callback for sockethandling, for more, see CAmSocketHandler
             */
            void receiverCallback(const pollfd pollfd, const sh_pollHandle_t handle, void* userData)
            {
                (void) handle;
                (void) userData;
                int numReads;
                CAmDelegagePtr listPointers[3];
                if ((numReads = read(pollfd.fd, &listPointers, sizeof(listPointers))) == -1)
                {
                    logError("CAmSerializer::receiverCallback could not read pipe!");
                    throw std::runtime_error("CAmSerializer Could not read pipe!");
                }
                mListDelegatePoiters.assign(listPointers, listPointers + (numReads / sizeof(CAmDelegagePtr)));
            }

            /**
             * checker callback for sockethandling, for more, see CAmSocketHandler
             */
            bool checkerCallback(const sh_pollHandle_t handle, void* userData)
            {
                (void) handle;
                (void) userData;
                if (mListDelegatePoiters.empty())
                    return (false);
                return (true);
            }

            /**
             * dispatcher callback for sockethandling, for more, see CAmSocketHandler
             */
            bool dispatcherCallback(const sh_pollHandle_t handle, void* userData)
            {
                (void) handle;
                (void) userData;
                CAmDelegagePtr delegatePoiter = mListDelegatePoiters.front();
                mListDelegatePoiters.pop_front();
                if (delegatePoiter->call(mReturnPipe))
                    delete delegatePoiter;
                if (mListDelegatePoiters.empty())
                    return (false);
                return (true);
            }

            TAmShPollFired<CAmSerializer> receiverCallbackT;
            TAmShPollDispatch<CAmSerializer> dispatcherCallbackT;
            TAmShPollCheck<CAmSerializer> checkerCallbackT;

            /**
             * The constructor must be called in the mainthread context !
             * @param iSocketHandler pointer to the CAmSocketHandler
             */
            CAmSerializer(CAmSocketHandler *iSocketHandler) :
                    mPipe(), //
                            mReturnPipe(), //
                            mHandle(),
                            mpSocketHandler(iSocketHandler),
                            mListDelegatePoiters(), //
                            receiverCallbackT(this, &CAmSerializer::receiverCallback), //
                            dispatcherCallbackT(this, &CAmSerializer::dispatcherCallback), //
                            checkerCallbackT(this, &CAmSerializer::checkerCallback)
            {
                assert(NULL!=iSocketHandler);

                if (pipe(mPipe) == -1)
                {
                    logError("CAmSerializer could not create pipe!");
                    throw std::runtime_error("CAmSerializer Could not open pipe!");
                }

                if (pipe(mReturnPipe) == -1)
                {
                    logError("CAmSerializer could not create mReturnPipe!");
                    throw std::runtime_error("CAmSerializer Could not open mReturnPipe!");
                }

                short event = 0;
                event |= POLLIN;
                mpSocketHandler->addFDPoll(mPipe[0], event, NULL, &receiverCallbackT, &checkerCallbackT, &dispatcherCallbackT, NULL, mHandle);
            }

            ~CAmSerializer()
            {
                mpSocketHandler->removeFDPoll(mHandle);
                close(mPipe[0]);
                close(mPipe[1]);
                close(mReturnPipe[0]);
                close(mReturnPipe[1]);
            }
        };

    } /* namespace V1 */

    /**@}*/

    namespace V2
    {
        class CAmSerializer
        {
            /**
             * Prototype for a delegate
             */
            class CAmDelegate
            {
            public:
                typedef enum
                    :bool
                    {
                        SyncCallType = false, AsyncCallType = true
                } CallType;

                virtual ~CAmDelegate()
                {
                }
                ;
                virtual CallType call(int* pipe)=0;
            };

            /**
             * Prototype for a delegate with variadic template arguments.
             */
            template<class TInvocation>
            class CAmDelegateAsyncImpl: public CAmDelegate
            {
                TInvocation mInvocation;
            public:
                friend class CAmSerializer;
                CAmDelegateAsyncImpl(TInvocation && invocation) :
                        mInvocation(std::move(invocation))
                {
                }

                CallType call(int* pipe)
                {
                    (void) pipe;
                    mInvocation();
                    return (AsyncCallType);
                }
                ;
            };

            template<class TInvocation, class TRet>
            class CAmDelegateSyncImpl: public CAmDelegate
            {
                TInvocation mInvocation;
                TRet & mReturn;
            public:
                friend class CAmSerializer;
                CAmDelegateSyncImpl(TInvocation && invocation, TRet && ret) :
                        mInvocation(std::move(invocation)), mReturn(ret)
                {
                }

                CallType call(int* pipe)
                {
                    mReturn = mInvocation();
                    ssize_t result(-1);
                    result = write(pipe[1], this, sizeof(this));
                    if (result == -1)
                        logError("CAmSerializer: Problem writing into pipe! Error No:", errno);
                    return (SyncCallType);
                }
                ;
            };

            template<class TInvocation>
            class CAmDelegateSyncVoidImpl: public CAmDelegate
            {
                TInvocation mInvocation;
            public:
                friend class CAmSerializer;
                CAmDelegateSyncVoidImpl(TInvocation && invocation) :
                        mInvocation(std::move(invocation))
                {
                }

                CallType call(int* pipe)
                {
                    mInvocation();
                    ssize_t result(-1);
                    result = write(pipe[1], this, sizeof(this));
                    if (result == -1)
                        logError("CAmSerializer: Problem writing into pipe! Error No:", errno);
                    return (SyncCallType);
                }
                ;
            };

            typedef CAmDelegate* CAmDelegagePtr; //!< pointer to a delegate

            void sendSync(CAmDelegagePtr pDelegate)
            {
                send(pDelegate);
                int numReads;
                CAmDelegagePtr *p = NULL;
                if ((numReads = read(mReturnPipe[0], &p, sizeof(p))) == -1)
                {
                    logError("CAmSerializer::doSyncCall could not read pipe!");
                    throw std::runtime_error("CAmSerializer Could not read pipe!");
                }
            }

            /**
             * rings the line of the pipe and adds the delegate pointer to the queue
             * @param p delegate pointer
             */
            inline void send(CAmDelegagePtr p)
            {
                if (write(mPipe[1], &p, sizeof(p)) == -1)
                {
                    throw std::runtime_error("could not write to pipe !");
                }
            }

            int mPipe[2]; //!< the pipe
            int mReturnPipe[2]; //!< pipe handling returns
            sh_pollHandle_t mHandle;
            CAmSocketHandler* mpSocketHandler;
            std::deque<CAmDelegagePtr> mListDelegatePointers; //!< intermediate queue to store the pipe results

        public:

            /**
             * get the size of delegate pointers
             */
            size_t getListDelegatePointers()
            {
                return mListDelegatePointers.size();
            }

            /**
             * calls a function with variadic arguments threadsafe
             * @param invocation is a type is produced by std::bind
             * \section ex Example:
             * @code
             * CAmSerializer serial(&Sockethandler);
             * serial.asyncInvocation(std::bind([]()->bool{return true;}));
             * @endcode
             */
            template<class TFunc>
            void asyncInvocation(TFunc invocation)
            {
                static_assert(std::is_bind_expression<TFunc>::value,"The type is not produced by std::bind");
                typedef CAmDelegateAsyncImpl<TFunc> AsyncDelegate;
                AsyncDelegate *pImp = new AsyncDelegate(std::forward<TFunc>(invocation));
                send(pImp);
                //Do not delete the pointer. It will be deleted automatically later.
            }

            /**
             * calls a function with variadic arguments threadsafe
             * @param instance the instance of the class that shall be called
             * @param function the function that shall be called as member function pointer.
             * @tparam TClass the type of the Class to be called
             * @tparam TRet the type of the result
             * @tparam TArgs argument list
             * \section ex Example:
             * @code
             * class AClass
             * {
             * public:
             *      void instanceMethod(int x);
             * }
             * CAmSerializer serial(&Sockethandler);
             * AClass anInstance;
             * serial.asyncCall(&anInstance,&AClass::instanceMethod, 100);
             * @endcode
             */
            template<class TClass, class TMeth, class TRet, class ... TArgs>
            void asyncCall(TClass* instance, TMeth method, TArgs && ... arguments)
            {
                auto invocation = std::bind(method, instance, std::forward<TArgs>(arguments)...);
                asyncInvocation(invocation);
            }

            template<class TClass, class TMeth, class ... TArgs>
            void asyncCall(TClass* instance, TMeth method, TArgs && ... arguments)
            {
                auto invocation = std::bind(method, instance, std::forward<TArgs>(arguments)...);
                asyncInvocation(invocation);
            }

            /**
             * calls a function with variadic arguments threadsafe
             * @param invocation is a type is produced by std::bind
             * @param result from type TRet
             * \section ex Example:
             * @code
             * CAmSerializer serial(&Sockethandler);
             * bool result;
             * serial.syncCall(std::bind([]()->bool{return true;}), result);
             * @endcode
             */
            template<class TFunc, class TRet>
            void syncInvocation(TFunc invocation, TRet && result)
            {
                static_assert(std::is_bind_expression<TFunc>::value,"The type is not produced by std::bind");

                typedef CAmDelegateSyncImpl<TFunc, TRet> SyncDelegate;

                SyncDelegate *pImp = new SyncDelegate(std::forward<TFunc>(invocation), std::forward<TRet>(result));
                sendSync(pImp);
                //Delete the pointer.
                delete pImp;
            }

            /**
             * calls a function with variadic arguments threadsafe
             * @param invocation is a type produced by std::bind
             * \section ex Example:
             * @code
             * CAmSerializer serial(&Sockethandler);
             * serial.syncCall(std::bind([]()->bool{return true;}));
             * @endcode
             */
            template<class TFunc>
            void syncInvocation(TFunc invocation)
            {
                static_assert(std::is_bind_expression<TFunc>::value,"The type is not produced by std::bind");

                typedef CAmDelegateSyncVoidImpl<TFunc> SyncDelegate;

                SyncDelegate *pImp = new SyncDelegate(std::forward<TFunc>(invocation));
                sendSync(pImp);
                //Delete the pointer.
                delete pImp;
            }
            /**
             * calls a function with variadic arguments threadsafe
             * @param instance the instance of the class that shall be called
             * @param function the function that shall be called as member function pointer.
             * @param output variable.
             * @tparam TClass the type of the Class to be called
             * @tparam TRet the type of the result
             * @tparam TArgs argument list
             * \section ex Example:
             * @code
             * class AClass
             * {
             * public:
             *      int instanceMethod(int x);
             * }
             * CAmSerializer serial(&Sockethandler);
             * AClass anInstance;
             * int result;
             * serial.syncCall(&anInstance,&AClass::instanceMethod, result, 100);
             * @endcode
             */
            template<class TClass, class TMeth, class TRet, class ... TArgs>
            void syncCall(TClass* instance, TMeth method, TRet & result, TArgs && ... arguments)
            {
                auto invocation = std::bind(method, instance, std::ref(arguments)...);
                syncInvocation(invocation, result);
            }

            template<class TClass, class TMeth, class ... TArgs>
            void syncCall(TClass* instance, TMeth method, TArgs && ... arguments)
            {
                auto invocation = std::bind(method, instance, std::ref(arguments)...);
                syncInvocation(invocation);
            }

            /**
             * receiver callback for sockethandling, for more, see CAmSocketHandler
             */
            void receiverCallback(const pollfd pollfd, const sh_pollHandle_t handle, void* userData)
            {
                (void) handle;
                (void) userData;
                int numReads;
                CAmDelegagePtr listPointers[3];
                if ((numReads = read(pollfd.fd, &listPointers, sizeof(listPointers))) == -1)
                {
                    logError("CAmSerializer::receiverCallback could not read pipe!");
                    throw std::runtime_error("CAmSerializer Could not read pipe!");
                }
                mListDelegatePointers.assign(listPointers, listPointers + (numReads / sizeof(CAmDelegagePtr)));
            }

            /**
             * checker callback for sockethandling, for more, see CAmSocketHandler
             */
            bool checkerCallback(const sh_pollHandle_t handle, void* userData)
            {
                (void) handle;
                (void) userData;
                if (mListDelegatePointers.empty())
                    return (false);
                return (true);
            }

            /**
             * dispatcher callback for sockethandling, for more, see CAmSocketHandler
             */
            bool dispatcherCallback(const sh_pollHandle_t handle, void* userData)
            {
                (void) handle;
                (void) userData;
                CAmDelegagePtr delegatePoiter = mListDelegatePointers.front();
                mListDelegatePointers.pop_front();
                if (delegatePoiter->call(mReturnPipe))
                    delete delegatePoiter;
                if (mListDelegatePointers.empty())
                    return (false);
                return (true);
            }

            TAmShPollFired<CAmSerializer> receiverCallbackT;
            TAmShPollDispatch<CAmSerializer> dispatcherCallbackT;
            TAmShPollCheck<CAmSerializer> checkerCallbackT;

            /**
             * The constructor must be called in the mainthread context !
             * @param iSocketHandler pointer to the CAmSocketHandler
             */
            CAmSerializer(CAmSocketHandler *iSocketHandler) :
                    mPipe(), //
                            mReturnPipe(), //
                            mHandle(),
                            mpSocketHandler(iSocketHandler),
                            mListDelegatePointers(), //
                            receiverCallbackT(this, &CAmSerializer::receiverCallback), //
                            dispatcherCallbackT(this, &CAmSerializer::dispatcherCallback), //
                            checkerCallbackT(this, &CAmSerializer::checkerCallback)
            {
                assert(NULL!=iSocketHandler);

                if (pipe(mPipe) == -1)
                {
                    logError("CAmSerializer could not create pipe!");
                    throw std::runtime_error("CAmSerializer Could not open pipe!");
                }

                if (pipe(mReturnPipe) == -1)
                {
                    logError("CAmSerializer could not create mReturnPipe!");
                    throw std::runtime_error("CAmSerializer Could not open mReturnPipe!");
                }

                short event = 0;
                event |= POLLIN;
                mpSocketHandler->addFDPoll(mPipe[0], event, NULL, &receiverCallbackT, &checkerCallbackT, &dispatcherCallbackT, NULL, mHandle);
            }

            ~CAmSerializer()
            {
                mpSocketHandler->removeFDPoll(mHandle);
                close(mPipe[0]);
                close(mPipe[1]);
                close(mReturnPipe[0]);
                close(mReturnPipe[1]);
            }
        };
    } /* namespace V2 */

    typedef V1::CAmSerializer CAmSerializer DEPRECATED("You should use V2::CAmSerializer instead!");

} /* namespace am */
#endif /* CAMSERIALIZER_H_ */
