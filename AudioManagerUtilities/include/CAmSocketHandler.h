/**
 * SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2012, BMW AG
 *
 * \author Christian Linke, christian.linke@bmw.de BMW 2011,2012
 * \author Aleksandar Donchev, aleksander.donchev@partner.bmw.de BMW 2017
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *  \file CAmSocketHandler.h
 *  For further information see http://www.genivi.org/.
 */

#ifndef SOCKETHANDLER_H_
#define SOCKETHANDLER_H_

#include <sys/socket.h>
#include <stdint.h>
#include <sys/poll.h>
#include <list>
#include <map>
#include <set>
#include <signal.h>
#include <vector>
#include <functional>
#include <sys/signalfd.h>
#include <audiomanagerconfig.h>
#include "audiomanagertypes.h"

#ifdef WITH_TIMERFD

#include <stdio.h>
#include <string>
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>

#endif

namespace am
{

#define MAX_NS 1000000000L
#define MAX_TIMERHANDLE UINT16_MAX
#define MAX_POLLHANDLE UINT16_MAX

typedef uint16_t sh_pollHandle_t; //!<this is a handle for a filedescriptor to be used with the SocketHandler
typedef sh_pollHandle_t sh_timerHandle_t; //!<this is a handle for a timer to be used with the SocketHandler

/**
  * prototype for poll prepared callback
  */
class IAmShPollPrepare
{
public:
    virtual void Call(const sh_pollHandle_t handle, void* userData) = 0;
    virtual ~IAmShPollPrepare() {}
};

/**
  * prototype for poll fired callback
  */
class IAmShPollFired
{
public:
    virtual void Call(const pollfd pollfd, const sh_pollHandle_t handle, void* userData) = 0;
    virtual ~ IAmShPollFired() {}
};

/**
  * prototype for poll check callback
  */
class IAmShPollCheck
{
public:
    virtual bool Call(const sh_pollHandle_t handle, void* userData) = 0;
    virtual ~ IAmShPollCheck() {}
};

/**
  * prototype for dispatch callback
  */
class IAmShPollDispatch
{
public:
    virtual bool Call(const sh_pollHandle_t handle, void* userData) = 0;
    virtual ~ IAmShPollDispatch() {}
};

/**
  * prototype for the timer callback
  */
class IAmShTimerCallBack
{
public:
    IAmShTimerCallBack(){};
    virtual void Call(const sh_timerHandle_t handle, void* userData) = 0;
    virtual ~IAmShTimerCallBack(){}
};

/**make private, not public
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
            mFunction(function)
    {}

    virtual void Call(const pollfd pollfd, const sh_pollHandle_t handle, void* userData)
    {
        (*mInstance.*mFunction)(pollfd, handle, userData);
    }
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
            mFunction(function)
    {}

    virtual bool Call(const sh_pollHandle_t handle, void* userData)
    {
        return ((*mInstance.*mFunction)(handle, userData));
    }
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
            mFunction(function)
    {}

    virtual bool Call(const sh_pollHandle_t handle, void* userData)
    {
        return ((*mInstance.*mFunction)(handle, userData));
    }
};

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
            IAmShTimerCallBack(), mInstance(instance), //
            mFunction(function)
    {}

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
    void (TClass::*mFunction)(const sh_pollHandle_t handle, void* userData);

public:
    TAmShPollPrepare(TClass* instance, void (TClass::*function)(const sh_pollHandle_t handle, void* userData)) :
            mInstance(instance), //
            mFunction(function)
    {}

    virtual void Call(const sh_pollHandle_t handle, void* userData)
    {
        (*mInstance.*mFunction)(handle, userData);
    }
};

/**
  * The am::CAmSocketHandler implements a mainloop for the AudioManager. Plugins and different parts of the AudioManager add their filedescriptors to the handler
  * to get called on communication of the filedescriptors.\n
  * More information can be found here : \ref mainl
  */
class CAmSocketHandler
{
    struct sh_poll_s //!<struct that holds information about polls
    {
        sh_pollHandle_t handle; //!<handle to uniquely adress a filedesriptor
        pollfd pollfdValue; //!<the array for polling the filedescriptors
        std::function<void(const sh_pollHandle_t handle, void* userData)> prepareCB; //preperation callback
        std::function<void(const pollfd pollfd, const sh_pollHandle_t handle, void* userData)> firedCB; //fired callback
        std::function<bool(const sh_pollHandle_t handle, void* userData)> checkCB; //check callback
        std::function<bool(const sh_pollHandle_t handle, void* userData)> dispatchCB; //dispatch callback
        void* userData;

        sh_poll_s() :
                handle(0), pollfdValue(), prepareCB(), firedCB(), checkCB(), dispatchCB(), userData(0)
        {}
    };

    struct sh_timer_s //!<struct that holds information of timers
    {
        sh_timerHandle_t handle; //!<the handle of the timer
#ifdef WITH_TIMERFD       
        int fd;
        itimerspec countdown; //!<the countdown, this value is decreased every time the timer is up
#else
        timespec countdown; //!<the countdown, this value is decreased every time the timer is up
#endif        
        std::function<void(const sh_timerHandle_t handle, void* userData)> callback; //timer callback
        void* userData;
        sh_timer_s() :
                handle(0)
#ifdef WITH_TIMERFD
                        , fd(0)
#endif
                        , countdown(), callback(), userData(0)
        {}
    };

    struct sh_signal_s
    {
        sh_pollHandle_t handle; //!<handle to uniquely adress a filedesriptor
        std::function<void(const sh_pollHandle_t handle, const signalfd_siginfo & info, void* userData)> callback;
        void* userData;
        sh_signal_s() :
                handle(0), callback(), userData(0)
        {}
    };

    struct sh_identifier_s
    {
        std::set<sh_pollHandle_t> pollHandles;
        uint16_t limit;
        uint16_t lastUsedID;
        sh_identifier_s(const uint16_t pollLimit = UINT16_MAX) :
                pollHandles(), limit(pollLimit), lastUsedID(0)
        {}
    };

    typedef std::reverse_iterator<sh_timer_s> rListTimerIter; //!<typedef for reverseiterator on timer lists
    typedef std::vector<pollfd> VectorListPollfd_t; //!<vector of filedescriptors
    typedef std::vector<sh_poll_s> VectorListPoll_t; //!<list for the callbacks
    typedef std::vector<sh_signal_s> VectorSignalHandlers_t; //!<list for the callbacks

    typedef enum:uint8_t
    {
        NO_ERROR = 0u,   // OK
        PIPE_ERROR = 1u, // Pipe error
        FD_ERROR = 2u,   // Invalid file descriptor
        SFD_ERROR = 4u,
    } internal_codes_e;
    typedef uint8_t internal_codes_t;

    int mPipe[2];
    bool mDispatchDone; //this starts / stops the mainloop

    sh_identifier_s mSetPollKeys; //!A set of all used ppoll keys
    VectorListPoll_t mListPoll; //!<list that holds all information for the ppoll
    sh_identifier_s mSetTimerKeys; //!A set of all used timer keys
    std::list<sh_timer_s> mListTimer; //!<list of all timers
    std::list<sh_timer_s> mListActiveTimer; //!<list of all currently active timers
    sh_identifier_s mSetSignalhandlerKeys; //!A set of all used signal handler keys
    VectorSignalHandlers_t mSignalHandlers;
    bool mRecreatePollfds; //!<when this is true, the poll list needs to be recreated
    internal_codes_t mInternalCodes;
    sh_pollHandle_t mSignalFdHandle;
#ifndef WITH_TIMERFD
    timespec mStartTime; //!<here the actual time is saved for timecorrection
#endif
private:
    bool fdIsValid(const int fd) const;

    timespec* insertTime(timespec& buffertime);
#ifdef WITH_TIMERFD      
    am_Error_e createTimeFD(const itimerspec & timeouts, int & fd);

#else 
    void timerUp();
    void timerCorrection();

    /**
      * compares countdown values
      * @param a
      * @param b
      * @return true if b greater a
      */
    inline static bool compareCountdown(const sh_timer_s& a, const sh_timer_s& b)
    {
        return ((a.countdown.tv_sec == b.countdown.tv_sec) ? (a.countdown.tv_nsec < b.countdown.tv_nsec) : (a.countdown.tv_sec < b.countdown.tv_sec));
    }

    /**
      * Subtracts b from a
      * @param a
      * @param b
      * @return subtracted value
      */
    inline static timespec timespecSub(const timespec& a, const timespec& b)
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
                result.tv_nsec = a.tv_nsec + MAX_NS - b.tv_nsec;
                result.tv_sec--; /* Borrow a second. */
            }
            else
            {
                result.tv_nsec = a.tv_nsec - b.tv_nsec;
            }
        }
        return (result);
    }

    /**
      * adds timespec values
      * @param a
      * @param b
      * @return the added values
      */
    inline timespec timespecAdd(const timespec& a, const timespec& b)
    {
        timespec result;
        result.tv_sec = a.tv_sec + b.tv_sec;
        result.tv_nsec = a.tv_nsec + b.tv_nsec;
        if (result.tv_nsec >= MAX_NS)
        {
            result.tv_sec++;
            result.tv_nsec = result.tv_nsec - MAX_NS;
        }
        return (result);
    }

    /**
      * comapares timespec values
      * @param a
      * @param b
      * @return
      */
    inline int timespecCompare(const timespec& a, const timespec& b)
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
        return (0);
    }
#endif  

    /**
      * functor to prepare all fire events
      * @param a
      * @return
      */
    inline static void prepare(sh_poll_s& row);

    /**
      * functor to return all fired events
      * @param a
      * @return
      */
    inline static void fire(sh_poll_s& a);

    /**
      * functor to return all fired events
      * @param a
      * @return
      */
    inline static bool eventFired(const pollfd& a);

    /**
      * functor to help find the items that do not need dispatching
      * @param a
      * @return
      */
    inline static bool noDispatching(const sh_poll_s& a);

    /**
      * checks if dispatching is already finished
      * @param a
      * @return
      */
    inline static bool dispatchingFinished(const sh_poll_s& a);

    /**
      * timer fire callback
      * @param a
      * @return
      */
    inline static void callTimer(sh_timer_s& a);

    /**
      * next handle id
      * @param std::set handles
      * @return handle
      */
    bool nextHandle(sh_identifier_s & handle);
    
    am_Error_e getFDPollData(const sh_pollHandle_t handle, sh_poll_s & outPollData);
    
public:

    CAmSocketHandler();
    ~CAmSocketHandler();
    
   /**
     * install the signal fd
     */
    am_Error_e listenToSignals(const std::vector<uint8_t> & listSignals);
    
    am_Error_e addFDPoll(const int fd, const short event, std::function<void(const sh_pollHandle_t handle, void* userData)> prepare, std::function<void(const pollfd pollfd, const sh_pollHandle_t handle, void* userData)> fired,
            std::function<bool(const sh_pollHandle_t handle, void* userData)> check, std::function<bool(const sh_pollHandle_t handle, void* userData)> dispatch, void* userData, sh_pollHandle_t& handle);

    am_Error_e addFDPoll(const int fd, const short event, IAmShPollPrepare *prepare, IAmShPollFired *fired, IAmShPollCheck *check, IAmShPollDispatch *dispatch, void* userData, sh_pollHandle_t& handle);
    am_Error_e removeFDPoll(const sh_pollHandle_t handle);
    am_Error_e updateEventFlags(const sh_pollHandle_t handle, const short events);
    am_Error_e addSignalHandler(std::function<void(const sh_pollHandle_t handle, const signalfd_siginfo & info, void* userData)> callback, sh_pollHandle_t& handle, void * userData);
    am_Error_e removeSignalHandler(const sh_pollHandle_t handle);
    am_Error_e addTimer(const timespec & timeouts, IAmShTimerCallBack* callback, sh_timerHandle_t& handle, void * userData,
#ifndef WITH_TIMERFD
            const bool __attribute__((__unused__)) repeats = false
#else
            const bool repeats = false
#endif
            );
    am_Error_e addTimer(const timespec & timeouts, std::function<void(const sh_timerHandle_t handle, void* userData)> callback, sh_timerHandle_t& handle, void* userData,
#ifndef WITH_TIMERFD
            const bool __attribute__((__unused__)) repeats = false
#else
            const bool repeats = false
#endif
            );
    am_Error_e removeTimer(const sh_timerHandle_t handle);
    am_Error_e restartTimer(const sh_timerHandle_t handle);
    am_Error_e updateTimer(const sh_timerHandle_t handle, const timespec & timeouts);
    am_Error_e stopTimer(const sh_timerHandle_t handle);
    void start_listenting();
    void stop_listening();
    void exit_mainloop();
    
    bool fatalErrorOccurred();
};

} /* namespace am */
#endif /* SOCKETHANDLER_H_ */
