/**
 *  Copyright (C) 2012, BMW AG
 *
 *  \author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
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
 *  \file CAmSocketHandler.h
 *  For further information see http://www.genivi.org/.
 */

#ifndef SOCKETHANDLER_H_
#define SOCKETHANDLER_H_

#include "audiomanagertypes.h"
#include <sys/socket.h>
#include <stdint.h>
#include <sys/poll.h>
#include <list>
#include <map>
#include <signal.h>

namespace am
{

static volatile sig_atomic_t gDispatchDone = 0; //this global is used to stop the mainloop

typedef uint16_t sh_timerHandle_t; //!<this is a handle for a timer to be used with the SocketHandler
typedef uint16_t sh_pollHandle_t; //!<this is a handle for a filedescriptor to be used with the SocketHandler

/**
 * prototype for poll prepared callback
 */
class IAmShPollPrepare
{
public:
    virtual void Call(const sh_pollHandle_t handle, void* userData)=0;
    virtual ~IAmShPollPrepare(){};
};

/**
 * prototype for poll fired callback
 */
class IAmShPollFired
{
public:
    virtual void Call(const pollfd pollfd, const sh_pollHandle_t handle, void* userData)=0;
    virtual ~ IAmShPollFired(){};
};

/**
 * prototype for poll check callback
 */
class IAmShPollCheck
{
public:
    virtual bool Call(const sh_pollHandle_t handle, void* userData)=0;
    virtual ~ IAmShPollCheck(){};
};

/**
 * prototype for dispatch callback
 */
class IAmShPollDispatch
{
public:
    virtual bool Call(const sh_pollHandle_t handle, void* userData)=0;
    virtual ~ IAmShPollDispatch(){};
};

/**
 * prototype for the timer callback
 */
class IAmShTimerCallBack
{
public:
    virtual void Call(const sh_timerHandle_t handle, void* userData)=0;
    virtual ~IAmShTimerCallBack(){};
};

/**
 * The am::CAmSocketHandler implements a mainloop for the AudioManager. Plugins and different parts of the AudioManager add their filedescriptors to the handler
 * to get called on communication of the filedescriptors.\n
 * More information can be found here : \ref mainl
 */
class CAmSocketHandler
{
public:
    CAmSocketHandler();
    ~CAmSocketHandler();

    am_Error_e addFDPoll(const int fd, const short event, IAmShPollPrepare *prepare, IAmShPollFired *fired, IAmShPollCheck *check, IAmShPollDispatch *dispatch, void* userData, sh_pollHandle_t& handle);
    am_Error_e removeFDPoll(const sh_pollHandle_t handle);
    am_Error_e updateEventFlags(const sh_pollHandle_t handle, const short events);
    am_Error_e addTimer(const timespec timeouts, IAmShTimerCallBack*& callback, sh_timerHandle_t& handle, void* userData);
    am_Error_e removeTimer(const sh_timerHandle_t handle);
    am_Error_e restartTimer(const sh_timerHandle_t handle, const timespec timeouts);
    am_Error_e stopTimer(const sh_timerHandle_t handle);
    void start_listenting();
    void stop_listening();
private:
    struct sh_timer_s //!<struct that holds information of timers
    {
        sh_timerHandle_t handle; //!<the handle of the timer
        timespec countdown; //!<the countdown, this value is decreased every time the timer is up
        timespec timeout; //!<the original timer value
        IAmShTimerCallBack* callback; //!<the callbackfunction
        void * userData; //!<saves a void pointer together with the rest.
    };

    typedef std::reverse_iterator<sh_timer_s> rListTimerIter;

    class CAmShSubstractTime //!<functor to easy substract from each countdown value
    {
    private:
        timespec param;
    public:
        CAmShSubstractTime(timespec param) :
                param(param){}
        inline void operator()(sh_timer_s& t) const;
    };

    struct sh_poll_s //!<struct that holds information about polls
    {
        sh_pollHandle_t handle; //!<handle to uniquely adress a filedesriptor
        IAmShPollPrepare *prepareCB;
        IAmShPollFired *firedCB;
        IAmShPollCheck *checkCB;
        IAmShPollDispatch *dispatchCB;
        pollfd pollfdValue; //!<the array for polling the filedescriptors
        void *userData; //!<userdata saved together with the callback.
    };

    typedef std::vector<pollfd> mListPollfd_t; //!<vector of filedescriptors
    typedef std::vector<sh_poll_s> mListPoll_t; //!<list for the callbacks

    class CAmShCopyPollfd
    {
    private:
        mListPollfd_t& mArray;
    public:
        CAmShCopyPollfd(mListPollfd_t& dest) :
                mArray(dest){}
        void operator()(const sh_poll_s& row)
        {
            pollfd temp = row.pollfdValue;
            temp.revents = 0;
            mArray.push_back(temp);
        }
    };

    class CAmShMatchingFd
    {
    private:
        int mFD, mEventFlag;
    public:
        CAmShMatchingFd(int fd, int eventFlag) :
                mFD(fd), //
                mEventFlag(eventFlag)
        { }
        bool operator()(const sh_poll_s& a)
        {
            return (((a.pollfdValue.fd == mFD) && (a.pollfdValue.events == mEventFlag)) ? true : false);
        }
    };

    class CAmShCallFire
    {
    public:
        CAmShCallFire(){};
        void operator()(sh_poll_s& row)
        {
            row.firedCB->Call(row.pollfdValue, row.handle, row.userData);
        }
    };

    class CAmShCallPrep
    {
    public:
        CAmShCallPrep(){};
        void operator()(sh_poll_s& row)
        {
            if (row.prepareCB)
                row.prepareCB->Call(row.handle, row.userData);
        }
    };

    class CAmShCallTimer
    {
    public:
        CAmShCallTimer(){};
        void operator()(sh_timer_s& row)
        {
            row.callback->Call(row.handle, row.userData);
        }
    };

    class CAmShCountdownUp
    {
    public:
        CAmShCountdownUp(){};
        bool operator()(sh_timer_s& row)
        {
            if (row.countdown.tv_nsec == 0 && row.countdown.tv_sec == 0)
                return (true);
            return (false);
        }
    };

    class CAmShCountdownSame
    {
    private:
        sh_timer_s mCompareValue;
    public:
        CAmShCountdownSame(sh_timer_s& a) :
                mCompareValue(a)
        {}
        bool operator()(const sh_timer_s& b)
        {
            return ((mCompareValue.countdown.tv_sec == b.countdown.tv_sec) && (mCompareValue.countdown.tv_nsec == b.countdown.tv_nsec) ? true : false);
        }
    };

    bool fdIsValid(const int fd) const;
    void initTimer();
    void timerUp();
    void timerCorrection();
    int timespec2ms(const timespec& time);
    timespec* insertTime(timespec& buffertime);

    static bool compareCountdown(const sh_timer_s& a, const sh_timer_s& b)
    {
        return ((a.countdown.tv_sec == b.countdown.tv_sec) ? (a.countdown.tv_nsec < b.countdown.tv_nsec) : (a.countdown.tv_sec < b.countdown.tv_sec));
    }

    /**
     * Subtracts b from a
     * @param a
     * @param b
     * @return subtracted value
     */
    static timespec timespecSub(const timespec& a, const timespec& b)
    {
        timespec result;

        if ((a.tv_sec < b.tv_sec) || ((a.tv_sec == b.tv_sec) && (a.tv_nsec <= b.tv_nsec)))
        {
            result.tv_sec = result.tv_nsec = 0;
        }
        else
        {
            result.tv_sec = a.tv_sec - b.tv_sec;
            if (a.tv_nsec < b.tv_nsec)
            {
                result.tv_nsec = a.tv_nsec + 1000000000L - b.tv_nsec;
                result.tv_sec--; /* Borrow a second. */
            }
            else
            {
                result.tv_nsec = a.tv_nsec - b.tv_nsec;
            }
        }
        return (result);
    }

    static timespec timespecAdd(const timespec& a, const timespec b)
    {
        timespec result;
        result.tv_sec = a.tv_sec + b.tv_sec;
        result.tv_nsec = a.tv_nsec + b.tv_nsec;
        if (result.tv_nsec >= 1000000000L)
        {
            result.tv_sec++;
            result.tv_nsec = result.tv_nsec - 1000000000L;
        }
        return (result);
    }

    static int timespecCompare(const timespec& a, const timespec& b)
    {
        //less
        if (a.tv_sec < b.tv_sec)
            return (-1);
        //greater
        else if (a.tv_sec > b.tv_sec)
            return (1);
        //less
        else if (a.tv_nsec < b.tv_nsec)
            return (-1);
        //greater
        else if (a.tv_nsec > b.tv_nsec)
            return (1);
        //equal
        else
            return (0);
    }

    inline static bool eventFired(const pollfd& a)
    {
        return (a.revents == 0 ? false : true);
    }

    inline static bool noDispatching(const sh_poll_s& a)
    {
        //remove from list of there is no checkCB
        if (!a.checkCB)
            return (true);
        return (!a.checkCB->Call(a.handle, a.userData));
    }

    inline static bool dispatchingFinished(const sh_poll_s& a)
    {
        //remove from list of there is no dispatchCB
        if (!a.dispatchCB)
            return (true);
        return (!a.dispatchCB->Call(a.handle, a.userData));
    }

    mListPollfd_t mfdPollingArray;
    mListPoll_t mListPoll;
    std::list<sh_timer_s> mListTimer; //!<list of all timers
    std::list<sh_timer_s> mListActiveTimer; //!<list of all currently active timers
    sh_timerHandle_t mLastInsertedHandle;
    sh_pollHandle_t mLastInsertedPollHandle;
    bool mRecreatePollfds;
    timespec mStartTime;
}
;

/**
 * template to create the functor for a class
 */
template<class TClass> class TAmShTimerCallBack: public IAmShTimerCallBack
{
private:
    TClass* mInstance;
    void (TClass::*mFunction)(sh_timerHandle_t handle, void* userData);

public:
    TAmShTimerCallBack(TClass* instance, void (TClass::*function)(sh_timerHandle_t handle, void* userData)) :
            mInstance(instance), //
            mFunction(function){};

    virtual void Call(sh_timerHandle_t handle, void* userData)
    {
        (*mInstance.*mFunction)(handle, userData);
    }
};

/**
 * template for a callback
 */
template<class TClass> class TAmShPollPrepare: public IAmShPollPrepare
{
private:
    TClass* mInstance;
    void (TClass::*mFunction)(const sh_timerHandle_t handle, void* userData);

public:
    TAmShPollPrepare(TClass* instance, void (TClass::*function)(const sh_timerHandle_t handle, void* userData)) :
            mInstance(instance), //
            mFunction(function){};

    virtual void Call(const sh_timerHandle_t handle, void* userData)
    {
        (*mInstance.*mFunction)(handle, userData);
    };
};

/**
 * template for a callback
 */
template<class TClass> class TAmShPollFired: public IAmShPollFired
{
private:
    TClass* mInstance;
    void (TClass::*mFunction)(const pollfd pollfd, const sh_pollHandle_t handle, void* userData);

public:
    TAmShPollFired(TClass* instance, void (TClass::*function)(const pollfd pollfd, const sh_pollHandle_t handle, void* userData)) :
            mInstance(instance), //
            mFunction(function){};

    virtual void Call(const pollfd pollfd, const sh_pollHandle_t handle, void* userData)
    {
        (*mInstance.*mFunction)(pollfd, handle, userData);
    };
};

/**
 * template for a callback
 */
template<class TClass> class TAmShPollCheck: public IAmShPollCheck
{
private:
    TClass* mInstance;
    bool (TClass::*mFunction)(const sh_pollHandle_t handle, void* userData);

public:
    TAmShPollCheck(TClass* instance, bool (TClass::*function)(const sh_pollHandle_t handle, void* userData)) :
            mInstance(instance), //
            mFunction(function){};

    virtual bool Call(const sh_pollHandle_t handle, void* userData)
    {
        return ((*mInstance.*mFunction)(handle, userData));
    };
};

/**
 * template for a callback
 */
template<class TClass> class TAmShPollDispatch: public IAmShPollDispatch
{
private:
    TClass* mInstance;
    bool (TClass::*mFunction)(const sh_pollHandle_t handle, void* userData);

public:
    TAmShPollDispatch(TClass* instance, bool (TClass::*function)(const sh_pollHandle_t handle, void* userData)) :
            mInstance(instance), //
            mFunction(function){};

    virtual bool Call(const sh_pollHandle_t handle, void* userData)
    {
        return ((*mInstance.*mFunction)(handle, userData));
    };
};
} /* namespace am */
#endif /* SOCKETHANDLER_H_ */
