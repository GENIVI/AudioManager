/**
 * Copyright (C) 2011, BMW AG
 *
 * GeniviAudioMananger AudioManagerDaemon
 *
 * \file SocketHandler.h
 *
 * \date 20-Oct-2011 3:42:04 PM
 * \author Christian Mueller (christian.ei.mueller@bmw.de)
 *
 * \section License
 * GNU Lesser General Public License, version 2.1, with special exception (GENIVI clause)
 * Copyright (C) 2011, BMW AG Christian Mueller  Christian.ei.mueller@bmw.de
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License, version 2.1, for more details.
 * You should have received a copy of the GNU Lesser General Public License, version 2.1, along with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 * Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may also be applicable to programs even in cases in which the program is not a library in the technical sense.
 * Linking AudioManager statically or dynamically with other modules is making a combined work based on AudioManager. You may license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to license your linked modules under the GNU Lesser General Public License, version 2.1, you may use the program under the following exception.
 * As a special exception, the copyright holders of AudioManager give you permission to combine AudioManager with software programs or libraries that are released under any license unless such a combination is not permitted by the license of such a software program or library. You may copy and distribute such a system following the terms of the GNU Lesser General Public License, version 2.1, including this special exception, for AudioManager and the licenses of the other code concerned.
 * Note that people who make modified versions of AudioManager are not obligated to grant this special exception for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, version 2.1, gives permission to release a modified version without this exception; this exception also makes it possible to release a modified version which carries forward this exception.
 *
 */

#ifndef SOCKETHANDLER_H_
#define SOCKETHANDLER_H_

#include <audiomanagertypes.h>
#include <sys/socket.h>
#include <sys/time.h>
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

class shPollPrepare;
class shPollCheck;
class shPollFired;
class shPollDispatch;
class shTimerCallBack;

class SocketHandler
{
public:
    SocketHandler();
    virtual ~SocketHandler();

    am_Error_e addFDPoll(const int fd, const short event, shPollPrepare *prepare, shPollFired *fired, shPollCheck *check, shPollDispatch *dispatch, void* userData, sh_pollHandle_t& handle);
    am_Error_e removeFDPoll(const sh_pollHandle_t handle);
    am_Error_e updateEventFlags(const sh_pollHandle_t handle, const short events);
    am_Error_e addTimer(const timespec timeouts, shTimerCallBack*& callback, sh_timerHandle_t& handle, void* userData);
    am_Error_e removeTimer(const sh_timerHandle_t handle);
    am_Error_e restartTimer(const sh_timerHandle_t handle, const timespec timeouts);
    am_Error_e stopTimer(const sh_timerHandle_t handle);
    void start_listenting();
    void stop_listening();
private:
    struct timer_s //!<struct that holds information of timers
    {
        sh_timerHandle_t handle; //!<the handle of the timer
        timespec countdown; //!<the countdown, this value is decreased every time the timer is up
        timespec timeout; //!<the original timer value
        shTimerCallBack* callback; //!<the callbackfunction
        void * userData; //!<saves a void pointer together with the rest.
    };

    class SubstractTime //!<functor to easy substract from each countdown value
    {
    private:
        timespec param;
    public:
        SubstractTime(timespec param) :
                param(param){}
        void operator()(timer_s& t) const;
    };

    struct sh_poll_s //!<struct that holds information about polls
    {
        sh_pollHandle_t handle; //!<handle to uniquely adress a filedesriptor
        shPollPrepare *prepareCB;
        shPollFired *firedCB;
        shPollCheck *checkCB;
        shPollDispatch *dispatchCB;
        pollfd pollfdValue; //!<the array for polling the filedescriptors
        void *userData; //!<userdata saved together with the callback.
    };

    typedef std::vector<pollfd> mPollfd_t; //!<vector of filedescriptors
    typedef std::vector<sh_poll_s> mListPoll_t; //!<list for the callbacks

    class CopyPollfd
    {
    private:
        mPollfd_t& mArray;
    public:
        CopyPollfd(mPollfd_t& dest) :
                mArray(dest){}
        void operator()(const sh_poll_s& row);
    };

    bool fdIsValid(const int fd) const;
    void initTimer();
    void timerUp();
    int timespec2ms(const timespec& time);
    timespec* insertTime(timespec& buffertime);
    static bool compareCountdown(const timer_s& a, const timer_s& b)
    {
        return (a.countdown.tv_sec == b.countdown.tv_sec) ? (a.countdown.tv_nsec < b.countdown.tv_nsec) : (a.countdown.tv_sec < b.countdown.tv_sec);
    }

    static bool onlyFiredEvents(const pollfd& a)
    {
        return a.revents == 0 ? false : true;
    }

    //todo: maybe we could simplify mListActiveTimer to hold only the handle and the countdown ....
    mPollfd_t mfdPollingArray;
    mListPoll_t mListPoll;
    std::list<timer_s> mListTimer; //!<list of all timers
    std::list<timer_s> mListActiveTimer; //!<list of all currently active timers
    sh_timerHandle_t mNextTimer;
    sh_timerHandle_t mLastInsertedHandle;
    sh_pollHandle_t mLastInsertedPollHandle;
    bool mRecreatePollfds;
    timespec mTimeout;
};

/**
 * classic functor for the BasicTimerCallback
 */
class shTimerCallBack
{
public:
    virtual void Call(const sh_timerHandle_t handle, void* userData)=0;
    virtual ~shTimerCallBack(){};
};

class shPollPrepare
{
public:
    virtual void Call(const sh_pollHandle_t handle, void* userData)=0;
    virtual ~shPollPrepare(){};
};

class shPollFired
{
public:
    virtual void Call(const pollfd pollfd, const sh_pollHandle_t handle, void* userData)=0;
    virtual ~ shPollFired(){};
};

class shPollCheck
{
public:
    virtual bool Call(const sh_pollHandle_t handle, void* userData)=0;
    virtual ~ shPollCheck(){};
};

class shPollDispatch
{
public:
    virtual bool Call(const sh_pollHandle_t handle, void* userData)=0;
    virtual ~ shPollDispatch() {};
};

/**
 * template to create the functor for a class
 */
template<class TClass> class shTimerCallBack_T: public shTimerCallBack
{
private:
    TClass* mInstance;
    void (TClass::*mFunction)(sh_timerHandle_t handle, void* userData);

public:
    shTimerCallBack_T(TClass* instance, void(TClass::*function)(sh_timerHandle_t handle, void* userData)) :
            mInstance(instance),//
            mFunction(function) {};

    virtual void Call(sh_timerHandle_t handle, void* userData)
    {
        (*mInstance.*mFunction)(handle, userData);
    }
};

/**
 * template to create the functor for a class
 */
template<class TClass> class shPollPrepare_T: public shPollPrepare
{
private:
    TClass* mInstance;
    void (TClass::*mFunction)(const sh_timerHandle_t handle, void* userData);

public:
    shPollPrepare_T(TClass* instance, void(TClass::*function)(const sh_timerHandle_t handle, void* userData)) :
            mInstance(instance), //
            mFunction(function){};

    virtual void Call(const sh_timerHandle_t handle, void* userData)
    {
        (*mInstance.*mFunction)(handle, userData);
    };
};

template<class TClass> class shPollFired_T: public shPollFired
{
private:
    TClass* mInstance;
    void (TClass::*mFunction)(const pollfd pollfd, const sh_pollHandle_t handle, void* userData);

public:
    shPollFired_T(TClass* instance, void(TClass::*function)(const pollfd pollfd, const sh_pollHandle_t handle, void* userData)) :
            mInstance(instance), //
            mFunction(function){};

    virtual void Call(const pollfd pollfd, const sh_pollHandle_t handle, void* userData)
    {
        (*mInstance.*mFunction)(pollfd, handle, userData);
    };
};

template<class TClass> class shPollCheck_T: public shPollCheck
{
private:
    TClass* mInstance;
    bool (TClass::*mFunction)(const sh_pollHandle_t handle, void* userData);

public:
    shPollCheck_T(TClass* instance, bool(TClass::*function)(const sh_pollHandle_t handle, void* userData)) :
            mInstance(instance), //
            mFunction(function){};

    virtual bool Call(const sh_pollHandle_t handle, void* userData)
    {
        return (*mInstance.*mFunction)(handle, userData);
    };
};

template<class TClass> class shPollDispatch_T: public shPollDispatch
{
private:
    TClass* mInstance;
    bool (TClass::*mFunction)(const sh_pollHandle_t handle, void* userData);

public:
    shPollDispatch_T(TClass* instance, bool(TClass::*function)(const sh_pollHandle_t handle, void* userData)) :
            mInstance(instance), //
            mFunction(function) {};

    virtual bool Call(const sh_pollHandle_t handle, void* userData)
    {
        return (*mInstance.*mFunction)(handle, userData);
    };
};
} /* namespace am */
#endif /* SOCKETHANDLER_H_ */
