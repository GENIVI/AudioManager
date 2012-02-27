/** Copyright (c) 2012 GENIVI Alliance
 *  Copyright (c) 2012 BMW
 *
 *  @author Christian Mueller, BMW
 *
 *  @copyright
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction,
 *  including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 *  subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 *  THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef CAMSERIALIZER_H_
#define CAMSERIALIZER_H_

#include <pthread.h>
#include <deque>
#include <cassert>
#include <memory>
#include <stdexcept>
#include "DLTWrapper.h"
#include "SocketHandler.h"

#include "iostream" //todo remove

namespace am
{
/**
 * magic class that does the serialization of functions calls
 * The constructor must be called within the main threadcontext, after that using the
 * overloaded template function call will serialize all calls and call them within the
 * main thread context.\n
 * If you want to use synchronous calls make sure that you use one instance of this class
 * per thread otherwise you could be lost in never returning calls.
 */
class CAmSerializer
{
private:

    /**
     * Prototype for a delegate
     */
    class CAmDelegate
    {
    public:
        virtual ~CAmDelegate(){};
        virtual void call()=0;

    };

    typedef CAmDelegate* CAmDelegagePtr; //!< pointer to a delegate

    /**
     * delegate template for no argument
     */
    template<class TClass> class CAmNoArgDelegate: public CAmDelegate
    {
    private:
        TClass* mInstance;
        void (TClass::*mFunction)();

    public:
        CAmNoArgDelegate(TClass* instance, void(TClass::*function)()) :
                mInstance(instance), //
                mFunction(function){};

        void call()
        {
            (*mInstance.*mFunction)();
        };
    };

    /**
     * delegate template for one argument
     */
    template<class TClass, typename Targ> class CAmOneArgDelegate: public CAmDelegate
    {
    private:
        TClass* mInstance;
        void (TClass::*mFunction)(Targ);
        Targ mArgument;

    public:
        CAmOneArgDelegate(TClass* instance, void(TClass::*function)(Targ), Targ argument) :
                mInstance(instance), //
                mFunction(function), //
                mArgument(argument) { };

        void call()
        {
            (*mInstance.*mFunction)(mArgument);
        };
    };

    /**
     * delegate template for two arguments
     */
    template<class TClass, typename Targ, typename Targ1> class CAmTwoArgDelegate: public CAmDelegate
    {
    private:
        TClass* mInstance;
        void (TClass::*mFunction)(Targ argument,Targ1 argument1);
        Targ mArgument;
        Targ1 mArgument1;

    public:
        CAmTwoArgDelegate(TClass* instance, void(TClass::*function)(Targ argument,Targ1 argument1), Targ argument, Targ1 argument1) :
                mInstance(instance), //
                mFunction(function), //
                mArgument(argument), //
                mArgument1(argument1){};

        void call()
        {
            (*mInstance.*mFunction)(mArgument,mArgument1);
        };
    };

    /**
     * delegate template for three arguments
     */
    template<class TClass, typename Targ, typename Targ1, typename Targ2> class CAmThreeArgDelegate: public CAmDelegate
    {
    private:
        TClass* mInstance;
        void (TClass::*mFunction)(Targ argument,Targ1 argument1,Targ2 argument2);
        Targ mArgument;
        Targ1 mArgument1;
        Targ2 mArgument2;

    public:
        CAmThreeArgDelegate(TClass* instance, void(TClass::*function)(Targ argument,Targ1 argument1,Targ2 argument2), Targ argument, Targ1 argument1, Targ2 argument2) :
                mInstance(instance), //
                mFunction(function), //
                mArgument(argument), //
                mArgument1(argument1), //
                mArgument2(argument2){};

        void call()
        {
            (*mInstance.*mFunction)(mArgument,mArgument1,mArgument2);
        };
    };

    /**
     * delegate template for four arguments
     */
    template<class TClass, typename Targ, typename Targ1, typename Targ2, typename Targ3> class CAmFourArgDelegate: public CAmDelegate
    {
    private:
        TClass* mInstance;
        void (TClass::*mFunction)(Targ argument, Targ1 argument1, Targ2 argument2, Targ3 argument3);
        Targ mArgument;
        Targ1 mArgument1;
        Targ2 mArgument2;
        Targ3 mArgument3;

    public:
        CAmFourArgDelegate(TClass* instance, void(TClass::*function)(Targ argument,Targ1 argument1,Targ2 argument2, Targ3 argument3), Targ argument, Targ1 argument1, Targ2 argument2, Targ3 argument3) :
                mInstance(instance), //
                mFunction(function), //
                mArgument(argument), //
                mArgument1(argument1), //
                mArgument2(argument2), //
                mArgument3(argument3){};

        void call()
        {
            (*mInstance.*mFunction)(mArgument,mArgument1,mArgument2,mArgument3);
        };
    };

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
    std::deque<CAmDelegagePtr> mListDelegatePoiters; //!< intermediate queue to store the pipe results

public:

    /**
     * calls a function with no arguments threadsafe
     * @param instance the instance of the class that shall be called
     * @param function the function that shall be called as memberfunction pointer.
     * Here is an example:
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
    void asyncCall(TClass* instance, void(TClass::*function)())
    {
        CAmDelegagePtr p(new CAmNoArgDelegate<TClass>(instance, function));
        send(p);
    }

    /**
     * calls a function with one arguments asynchronously threadsafe
     * @param instance the instance of the class that shall be called
     * @param function the function that shall be called as memberfunction pointer.
     * Here is an example:
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
     */
    template<class TClass1, class Targ>
    void asyncCall(TClass1* instance, void(TClass1::*function)(Targ), Targ argument)
    {
        CAmDelegagePtr p(new CAmOneArgDelegate<TClass1, Targ>(instance, function, argument));
        send(p);
    }

    /**
     * calls a function with two arguments asynchronously threadsafe. for more see asyncCall with one argument
     * @param instance
     * @param function
     * @param argument
     * @param argument1
     */
    template<class TClass1, class Targ, class Targ1>
    void asyncCall(TClass1* instance, void(TClass1::*function)(Targ argument,Targ1 argument1), Targ argument, Targ1 argument1)
    {
        CAmDelegagePtr p(new CAmTwoArgDelegate<TClass1, Targ, Targ1>(instance, function, argument,argument1));
        send(p);
    }

    /**
     * calls a function with three arguments asynchronously threadsafe. for more see asyncCall with one argument
     * @param instance
     * @param function
     * @param argument
     * @param argument1
     * @param argument2
     */
    template<class TClass1, class Targ, class Targ1, class Targ2>
    void asyncCall(TClass1* instance, void(TClass1::*function)(Targ argument,Targ1 argument1, Targ2 argument2), Targ argument, Targ1 argument1, Targ2 argument2)
    {
        CAmDelegagePtr p(new CAmThreeArgDelegate<TClass1, Targ, Targ1, Targ2>(instance, function, argument,argument1, argument2));
        send(p);
    }

    /**
     * calls a function with four arguments asynchronously threadsafe. for more see asyncCall with one argument
     * @param instance
     * @param function
     * @param argument
     * @param argument1
     * @param argument2
     * @param argument3
     */
    template<class TClass1, class Targ, class Targ1, class Targ2, class Targ3>
    void asyncCall(TClass1* instance, void(TClass1::*function)(Targ argument,Targ1 argument1, Targ2 argument2, Targ3 argument3), Targ argument, Targ1 argument1, Targ2 argument2, Targ3 argument3)
    {
        CAmDelegagePtr p(new CAmFourArgDelegate<TClass1, Targ, Targ1, Targ2, Targ3>(instance, function, argument,argument1, argument2, argument3));
        send(p);
    }

    void receiverCallback(const pollfd pollfd, const sh_pollHandle_t handle, void* userData)
    {
        (void) handle;
        (void) userData;
        int numReads;
        CAmDelegagePtr listPointers[3];
        if ((numReads=read(pollfd.fd,&listPointers, sizeof(listPointers))) == -1)
        {
            logError("CAmSerializer::receiverCallback could not read pipe!");
            throw std::runtime_error("CAmSerializer Could not read pipe!");
        }
        mListDelegatePoiters.assign(listPointers, listPointers+(numReads/sizeof(CAmDelegagePtr)));
    }

    bool checkerCallback(const sh_pollHandle_t handle, void* userData)
    {
        (void) handle;
        (void) userData;
        if (mListDelegatePoiters.empty())
            return false;
        return true;
    }

    bool dispatcherCallback(const sh_pollHandle_t handle, void* userData)
    {
        (void) handle;
        (void) userData;
        CAmDelegagePtr delegatePoiter = mListDelegatePoiters.front();
        mListDelegatePoiters.pop_front();
        delegatePoiter->call();
        delete delegatePoiter;
        if (mListDelegatePoiters.empty())
            return false;
        return true;
    }

    shPollFired_T<CAmSerializer> receiverCallbackT;
    shPollDispatch_T<CAmSerializer> dispatcherCallbackT;
    shPollCheck_T<CAmSerializer> checkerCallbackT;

    /**
     * The constructor must be called in the mainthread context !
     * @param iSocketHandler pointer to the sockethandler
     */
    CAmSerializer(SocketHandler *iSocketHandler) :
            mPipe(), //
            mListDelegatePoiters(), //
            receiverCallbackT(this, &CAmSerializer::receiverCallback), //
            dispatcherCallbackT(this, &CAmSerializer::dispatcherCallback), //
            checkerCallbackT(this, &CAmSerializer::checkerCallback)
    {
        if (pipe(mPipe) == -1)
        {
            logError("CAmSerializer could not create pipe!");
            throw std::runtime_error("CAmSerializer Could not open pipe!");
        }

        short event = 0;
        sh_pollHandle_t handle;
        event |= POLLIN;
        iSocketHandler->addFDPoll(mPipe[0], event, NULL, &receiverCallbackT, NULL, &dispatcherCallbackT, NULL, handle);
    }

    virtual ~CAmSerializer(){}
};
} /* namespace am */
#endif /* CAMSERIALIZER_H_ */
