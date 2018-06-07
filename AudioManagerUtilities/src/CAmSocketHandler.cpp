/**
 *  SPDX license identifier: MPL-2.0
 *
 * Copyright (C) 2012, BMW AG
 *
 * This file is part of GENIVI Project AudioManager.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Christian Linke, christian.linke@bmw.de BMW 2011,2012
 * \author Aleksandar Donchev, aleksander.donchev@partner.bmw.de BMW 2017
 *
 * \file CAmSocketHandler.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include <cassert>
#include <sys/fcntl.h>
#include <sys/errno.h>
#include <sys/poll.h>
#include <sys/eventfd.h>
#include <time.h>
#include <algorithm>
#include <features.h>
#include <csignal>
#include <cstring>

#include "CAmDltWrapper.h"
#include "CAmSocketHandler.h"

#ifdef WITH_TIMERFD
#include <sys/timerfd.h>
#endif

#define END_EVENT                  (UINT64_MAX >> 1)

namespace am
{

CAmSocketHandler::CAmSocketHandler() :
        mEventFd(-1), //
        mSignalFd(-1), //
        mDispatchDone(true), //
        mSetPollKeys(MAX_POLLHANDLE), //
        mMapShPoll(), //
        mSetTimerKeys(MAX_TIMERHANDLE),
        mListTimer(), //
#ifndef WITH_TIMERFD
        mListActiveTimer(), //
#endif
        mSetSignalhandlerKeys(MAX_POLLHANDLE), //
        mSignalHandlers(), //
        mInternalCodes(internal_codes_e::NO_ERROR)
#ifndef WITH_TIMERFD
        ,mStartTime() //
#endif
{

    auto actionPoll = [this](const pollfd pollfd, const sh_pollHandle_t, void*)
    {
          /* We have a valid signal, read the info from the fd */
          uint64_t events;
          ssize_t bytes = read(pollfd.fd, &events, sizeof(events));
          if (bytes == sizeof(events))
          {
              if (events >= END_EVENT)
              {
                  for (auto & elem : mMapShPoll)
                  {
                      if (elem.second.state == poll_states_e::UPDATE ||
                          elem.second.state == poll_states_e::VALID)
                      {
                          elem.second.state = poll_states_e::ADD;
                      }
                  }
                  mDispatchDone = true;
              }
              return;
          }

          // ppoll on EAGAIN
          if ((bytes == -1) && (errno == EAGAIN))
              return;

          //Failed to read from event fd...
          std::ostringstream msg;
          msg << "Failed to read from event fd: " << pollfd.fd << " errno: " << std::strerror(errno);
          throw std::runtime_error(msg.str());
    };

    //add the pipe to the poll - nothing needs to be processed here we just need the pipe to trigger the ppoll
    sh_pollHandle_t handle;
    mEventFd = eventfd(1, EFD_NONBLOCK | EFD_CLOEXEC);
    if (addFDPoll(mEventFd, POLLIN, NULL, actionPoll, NULL, NULL, NULL, handle) != E_OK)
    {
        mInternalCodes |= internal_codes_e::FD_ERROR;
    }
}

CAmSocketHandler::~CAmSocketHandler()
{
    for (const auto& it : mMapShPoll)
    {
        close(it.second.pollfdValue.fd);
    }
}

//todo: maybe have some: give me more time returned?
/**
  * start the block listening for filedescriptors. This is the mainloop.
  */
void CAmSocketHandler::start_listenting()
{
    mDispatchDone = false;

#ifndef WITH_TIMERFD 
    clock_gettime(CLOCK_MONOTONIC, &mStartTime);
#endif    
    timespec buffertime;

    VectorPollfd_t fdPollingArray; //!<the polling array for ppoll

    while (!mDispatchDone)
    {
        /* Iterate all times through map and synchronize the polling array accordingly.
         * In case a new element in map appears the polling array will be extended and
         * in case an element gets removed the map and the polling array needs to be adapted.
         */
        auto fdPollIt = fdPollingArray.begin();
        for (auto it = mMapShPoll.begin(); it != mMapShPoll.end(); )
        {
            // NOTE: The order of the switch/case statement reflects the state flow
            auto& elem = it->second;
            switch (elem.state)
            {
                case poll_states_e::ADD:
                    elem.state = poll_states_e::UPDATE;
                    fdPollIt = fdPollingArray.emplace(fdPollIt);
                    break;

                case poll_states_e::UPDATE:
                    elem.state = poll_states_e::VALID;
                    CAmSocketHandler::prepare(elem);
                    *fdPollIt = elem.pollfdValue;
                    break;

                case poll_states_e::VALID:
                    // check for multi-thread access
                    assert(fdPollIt != fdPollingArray.end());
                    ++fdPollIt;
                    ++it;
                    break;

                case poll_states_e::REMOVE:
                    elem.state = poll_states_e::INVALID;
                    fdPollIt = fdPollingArray.erase(fdPollIt);
                    break;

                case poll_states_e::INVALID:
                    it = mMapShPoll.erase(it);
                    break;
            }
        }

        if (fdPollingArray.size() != mMapShPoll.size())
        {
            mInternalCodes |= internal_codes_e::MT_ERROR;
            logError("CAmSocketHandler::start_listenting is NOT multi-thread safe!");
            return;
        }

#ifndef WITH_TIMERFD
        timerCorrection();
#endif

        // block until something is on a file descriptor
        int16_t pollStatus = ppoll(&fdPollingArray[0], fdPollingArray.size(), insertTime(buffertime), NULL);
        if (pollStatus > 0)
        {
            // stage 0+1, call firedCB
            std::list<sh_poll_s*> listPoll;
            for (auto& it : fdPollingArray)
            {
                it.revents &= it.events;
                if (it.revents == 0)
                    continue;

                sh_poll_s& pollObj = mMapShPoll.at(it.fd);
                if (pollObj.state != poll_states_e::VALID)
                    continue;

                // ensure to copy the revents fired in fdPollingArray
                pollObj.pollfdValue.revents = it.revents;
                listPoll.push_back(&pollObj);
                CAmSocketHandler::fire(pollObj);
                it.revents = 0;
            }
            
            //stage 2, lets ask around if some dispatching is necessary, the ones who need stay on the list
            listPoll.remove_if(CAmSocketHandler::noDispatching);

            //stage 3, the ones left need to dispatch, we do this as long as there is something to dispatch..
            do
            {
                listPoll.remove_if(CAmSocketHandler::dispatchingFinished);
            } while (!listPoll.empty());
        }
        else if ((pollStatus < 0) && (errno != EINTR))
        {
            logError("SocketHandler::start_listenting ppoll returned with error", errno);
            throw std::runtime_error(std::string("SocketHandler::start_listenting ppoll returned with error."));
        }
        else //Timerevent
        {
#ifndef WITH_TIMERFD
            //this was a timer event, we need to take care about the timers
            //find out the timedifference to starttime
            timerUp();
#endif
        }
    }
}

/**
  * exits the loop
  */
void CAmSocketHandler::stop_listening()
{
    //fire the ending event
    if (mDispatchDone)
        return;

    wakeupWorker("stop_listening", END_EVENT);

#ifndef WITH_TIMERFD
    //this is for all running timers only - we need to handle the additional offset here
    if (!mListActiveTimer.empty())
    {
        timespec currentTime, correctionTime;
        clock_gettime(CLOCK_MONOTONIC, &currentTime);
        correctionTime = timespecSub(currentTime, mStartTime);
        std::for_each(mListActiveTimer.begin(), mListActiveTimer.end(), [&correctionTime](sh_timer_s& t)
                {   t.countdown = timespecSub(t.countdown, correctionTime);});
    }
#endif
}

void CAmSocketHandler::exit_mainloop()
{
    //end the while loop
    stop_listening();
}

void CAmSocketHandler::wakeupWorker(const std::string & func, const uint64_t value)
{
    if (write(mEventFd, &value, sizeof(value)) < 0)
    {
        // no log message here, it is already done in main.cpp
        std::ostringstream msg("CAmSocketHandler::");
        msg << func << " Failed to write to event fd: " << mEventFd << " errno: " << std::strerror(errno);
        throw std::runtime_error(msg.str());
    }
}

bool CAmSocketHandler::fatalErrorOccurred() 
{
    return (mInternalCodes != internal_codes_e::NO_ERROR);
}

/**
  * Adds a signal handler filedescriptor to the polling loop
  *
  */
am_Error_e CAmSocketHandler::listenToSignals(const std::vector<uint8_t> & listSignals)
{
    int fdErr;
    uint8_t addedSignals = 0;
    sigset_t sigset;
    
    if(0==listSignals.size())
    {
        logWarning("Empty signal list!");
        return (E_NOT_POSSIBLE);
    }
    
    /* Create a sigset of all the signals that we're interested in */
    fdErr = sigemptyset(&sigset);
    if (fdErr != 0)
    {
        logError("Could not create sigset!");
        return (E_NOT_POSSIBLE);
    }
    
    for(uint8_t itSignal : listSignals)
    {
        fdErr = sigaddset(&sigset, itSignal);
        if (fdErr != 0)
            logWarning("Could not add", itSignal);
        else
          addedSignals++;
    }
    
    if(0==addedSignals)
    {
        logWarning("None of the signals were added!");
        return (E_NOT_POSSIBLE);
    }

    /* We must block the signals in order for signalfd to receive them */
    fdErr = sigprocmask(SIG_BLOCK, &sigset, NULL);
    if (fdErr != 0)
    {
        logError("Could not block signals! They must be blocked in order to receive them!");
        return (E_NOT_POSSIBLE);
    }

    if (mSignalFd < 0)
    {
        /* Create the signalfd */
        mSignalFd = signalfd(-1, &sigset, SFD_NONBLOCK);
        if (mSignalFd == -1)
        {
            logError("Could not open signal fd!", std::strerror(errno));
            return (E_NOT_POSSIBLE);
        }

        auto actionPoll = [this](const pollfd pollfd, const sh_pollHandle_t, void*)
        {
            /* We have a valid signal, read the info from the fd */
            struct signalfd_siginfo info;
            ssize_t bytes = read(pollfd.fd, &info, sizeof(info));
            if (bytes == sizeof(info))
            {
                /* Notify all listeners */
                for(const auto& it: mSignalHandlers)
                    it.callback(it.handle, info, it.userData);
                return;
            }

            // ppoll on EAGAIN
            if ((bytes == -1) && (errno == EAGAIN))
                return;

            //Failed to read from fd...
            std::ostringstream msg;
            msg << "Failed to read from signal fd: " << pollfd.fd << " errno: " << std::strerror(errno);
            throw std::runtime_error(msg.str());
        };
        /* We're going to add the signal fd through addFDPoll. At this point we don't have any signal listeners. */
        sh_pollHandle_t handle;
        return addFDPoll(mSignalFd, POLLIN | POLLERR | POLLHUP, NULL, actionPoll, NULL, NULL, NULL, handle);
    }    
    else
    {
        if (signalfd(mSignalFd, &sigset, 0) == -1)
        {
            logError("Could not update signal fd!", std::strerror(errno));
            return (E_NOT_POSSIBLE);
        }
        return E_OK;
    }
}

/**
  * Adds a filedescriptor to the polling loop
  * @param fd the filedescriptor
  * @param event the event flags
  * @param prepare a std::function that is called before the loop is entered
  * @param fired a std::function that is called when the filedescriptor needs to be read
  * @param check a std::function that is called to check if further actions are neccessary
  * @param dispatch a std::function that is called to dispatch the received data
  * @param userData a pointer to userdata that is always passed around
  * @param handle the handle of this poll
  * @return E_OK if the descriptor was added
  *         E_NON_EXISTENT if the fd is not valid
  *         E_ALREADY_EXISTS if the fd is already known
  *         E_NOT_POSSIBLE if the maximum handle threshold is reached
  */

am_Error_e CAmSocketHandler::addFDPoll(const int fd,
                                       const short event,
                                       std::function<void(const sh_pollHandle_t handle, void* userData)> prepare,
                                       std::function<void(const pollfd pollfd, const sh_pollHandle_t handle, void* userData)> fired,
                                       std::function<bool(const sh_pollHandle_t handle, void* userData)> check,
                                       std::function<bool(const sh_pollHandle_t handle, void* userData)> dispatch,
                                       void* userData,
                                       sh_pollHandle_t& handle)
{
    sh_poll_s pollData;

    if (!fdIsValid(fd))
        return E_NON_EXISTENT;

    const auto elem = mMapShPoll.find(fd);
    if (elem != mMapShPoll.end())
    {
        // The fd was already in map therefore we need to trigger an update instead
        switch (elem->second.state)
        {
            case poll_states_e::REMOVE:
                pollData.state = poll_states_e::UPDATE;
                break;

            case poll_states_e::INVALID:
                pollData.state = poll_states_e::ADD;
                break;

            default:
                logError("CAmSocketHandler::addFDPoll fd", fd, "already registered!");
                return E_ALREADY_EXISTS;
        }
    }

    //create a new handle for the poll
    if (!nextHandle(mSetPollKeys))
    {
        logError("CAmSocketHandler::addFDPoll Max handle count reached!");
        return (E_NOT_POSSIBLE);
    }

    pollData.pollfdValue.fd = fd;
    pollData.handle = mSetPollKeys.lastUsedID;
    pollData.pollfdValue.events = event;
    pollData.pollfdValue.revents = 0;
    pollData.prepareCB = prepare;
    pollData.firedCB = fired;
    pollData.checkCB = check;
    pollData.dispatchCB = dispatch;
    pollData.userData = userData;

    //add new data to the list
    mMapShPoll[fd] = pollData;
    wakeupWorker("addFDPoll");

    handle = pollData.handle;
    return (E_OK);

}

/**
  * Adds a filedescriptor to the polling loop
  * @param fd the filedescriptor
  * @param event the event flags
  * @param prepare a callback that is called before the loop is entered
  * @param fired a callback that is called when the filedescriptor needs to be read
  * @param check a callback that is called to check if further actions are neccessary
  * @param dispatch a callback that is called to dispatch the received data
  * @param userData a pointer to userdata that is always passed around
  * @param handle the handle of this poll
  * @return E_OK if the descriptor was added
  *         E_NON_EXISTENT if the fd is not valid
  *         E_ALREADY_EXISTS if the fd is already known
  *         E_NOT_POSSIBLE if the maximum handle threshold is reached
  */
am::am_Error_e CAmSocketHandler::addFDPoll(const int fd, const short event, IAmShPollPrepare *prepare, IAmShPollFired *fired, IAmShPollCheck *check, IAmShPollDispatch *dispatch, void *userData, sh_pollHandle_t & handle)
{

    std::function<void(const sh_pollHandle_t, void*)> prepareCB; //preperation callback
    std::function<void(const pollfd, const sh_pollHandle_t, void*)> firedCB; //fired callback
    std::function<bool(const sh_pollHandle_t, void*)> checkCB; //check callback
    std::function<bool(const sh_pollHandle_t, void*)> dispatchCB; //check callback

    if (prepare)
        prepareCB = std::bind(&IAmShPollPrepare::Call, prepare, std::placeholders::_1, std::placeholders::_2);
    if (fired)
        firedCB = std::bind(&IAmShPollFired::Call, fired, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    if (check)
        checkCB = std::bind(&IAmShPollCheck::Call, check, std::placeholders::_1, std::placeholders::_2);
    if (dispatch)
        dispatchCB = std::bind(&IAmShPollDispatch::Call, dispatch, std::placeholders::_1, std::placeholders::_2);

    return addFDPoll(fd, event, prepareCB, firedCB, checkCB, dispatchCB, userData, handle);
}

/**
  * removes a filedescriptor from the poll loop
  * @param handle
  * @param [rmv] default RMV_ONLY_FDPOLL
  * @return
  */
am_Error_e CAmSocketHandler::removeFDPoll(const sh_pollHandle_t handle)
{
    for (auto& it : mMapShPoll)
    {
        if (it.second.handle == handle)
        {
            it.second.state = (it.second.state == poll_states_e::ADD ? poll_states_e::INVALID : poll_states_e::REMOVE);
            wakeupWorker("removeFDPoll");
            mSetPollKeys.pollHandles.erase(handle);
            return E_OK;
        }
    }
    logWarning("CAmSocketHandler::removeFDPoll handle unknown", handle);
    return E_UNKNOWN;
}

/**
  * Adds a callback for any signals
  * @param callback
  * @param handle the handle of this poll
  * @param userData a pointer to userdata that is always passed around
  * @return E_OK if the descriptor was added, E_NON_EXISTENT if the fd is not valid
  */
am_Error_e CAmSocketHandler::addSignalHandler(std::function<void(const sh_pollHandle_t handle, const signalfd_siginfo & info, void* userData)> callback, sh_pollHandle_t& handle, void * userData)
{
    if (!nextHandle(mSetSignalhandlerKeys))
    {
        logError("CAmSocketHandler::addSignalHandler Could not create new polls, too many open!");
        return (E_NOT_POSSIBLE);
    }

    mSignalHandlers.emplace_back();
    mSignalHandlers.back().callback = callback;
    mSignalHandlers.back().handle = mSetSignalhandlerKeys.lastUsedID;
    mSignalHandlers.back().userData = userData;
    handle = mSetSignalhandlerKeys.lastUsedID;

    return E_OK;
}

/**
  * removes a signal handler from the list
  * @param handle is signal handler id
  * @return E_OK in case of success, E_UNKNOWN if the handler was not found.
  */
am_Error_e CAmSocketHandler::removeSignalHandler(const sh_pollHandle_t handle)
{
    VectorSignalHandlers_t::iterator it(mSignalHandlers.begin());
    for (; it != mSignalHandlers.end(); ++it)
    {
        if (it->handle == handle)
        {
            mSignalHandlers.erase(it);
            mSetSignalhandlerKeys.pollHandles.erase(handle);
            return (E_OK);
        }
    }
    return (E_UNKNOWN);
}

/**
  * adds a timer to the list of timers. The callback will be fired when the timer is up.
  * This is not a high precise timer, it is very coarse. It is meant to be used for timeouts when waiting
  * for an answer via a filedescriptor.
  * One time timer. If you need again a timer, you need to add a new timer in the callback of the old one.
  * @param timeouts timeouts time until the callback is fired
  * @param callback callback the callback
  * @param handle handle the handle that is created for the timer is returned. Can be used to remove the timer
  * @param userData pointer always passed with the call
  * @return E_OK in case of success
  */

am_Error_e CAmSocketHandler::addTimer(const timespec & timeouts, IAmShTimerCallBack* callback, sh_timerHandle_t& handle, void * userData, const bool repeats)
{
    assert(callback!=NULL);

    std::function<void(const sh_timerHandle_t handle, void* userData)> callbackFunc;
    callbackFunc = std::bind(&IAmShTimerCallBack::Call, callback, std::placeholders::_1, std::placeholders::_2);

    return addTimer(timeouts, callbackFunc, handle, userData, repeats);
}

am_Error_e CAmSocketHandler::addTimer(const timespec & timeouts, std::function<void(const sh_timerHandle_t handle, void* userData)> callback, sh_timerHandle_t& handle, void * userData, const bool repeats)
{
    assert(!((timeouts.tv_sec == 0) && (timeouts.tv_nsec == 0)));

#ifndef WITH_TIMERFD 
    //create a new handle for the timer
    if (!nextHandle(mSetTimerKeys))
    {
        logError("CAmSocketHandler::addTimer Could not create new timers, too many open!");
        return (E_NOT_POSSIBLE);
    }

    mListTimer.emplace_back();
    sh_timer_s & timerItem = mListTimer.back();

    //create a new handle for the timer
    handle = mSetTimerKeys.lastUsedID;

    timerItem.countdown = timeouts;
    timerItem.callback = callback;
    timerItem.userData = userData;

    timerItem.handle = handle;

    //we add here the time difference between startTime and currenttime, because this time will be substracted later on in timecorrection
    timespec currentTime;
    clock_gettime(CLOCK_MONOTONIC, &currentTime);
    if (!mDispatchDone)//the mainloop is started
    timerItem.countdown = timespecAdd(timeouts, timespecSub(currentTime, mStartTime));
    mListActiveTimer.push_back(timerItem);
    mListActiveTimer.sort(compareCountdown);
    return (E_OK);

#else   
    sh_timer_s timerItem;

    timerItem.countdown.it_value = timeouts;
    if (repeats)
        timerItem.countdown.it_interval = timeouts;
    else
    {
        timespec zero;
        zero.tv_sec = 0;
        zero.tv_nsec = 0;
        timerItem.countdown.it_interval = zero;
    }

    timerItem.fd = -1;
    timerItem.userData = userData;
    am_Error_e err = createTimeFD(timerItem.countdown, timerItem.fd);
    if (err != E_OK)
        return err;

    auto actionPoll = [this](const pollfd pollfd, const sh_pollHandle_t handle, void* userData)
    {
        uint64_t expCnt;
        ssize_t bytes = read(pollfd.fd, &expCnt, sizeof(expCnt));
        if (bytes == sizeof(expCnt))
            return;

        // ppoll has to be called again in following case
        if ((bytes == -1) && (errno == EAGAIN))
            return;

        // failed to read data from timer_fd...
        std::ostringstream msg;
        msg << "Failed to read from timer fd: " << pollfd.fd << " errno: " << std::strerror(errno);
        throw std::runtime_error(msg.str());
    };

    err = addFDPoll(timerItem.fd, POLLIN | POLLERR, NULL, actionPoll,
                        [callback](const sh_pollHandle_t handle, void* userData)->bool {
                                callback(handle, userData);
                                return false;
                            },
                        NULL, userData, handle);

    if (err == E_OK)
    {
        timerItem.handle = handle;
        mListTimer.push_back(timerItem);
        return E_OK;
    }

    // E_NOT_POSSIBLE is the only case were we need to close the timer
    if (err == E_NOT_POSSIBLE)
        close(timerItem.fd);

    return err;
#endif    

}

/**
  * removes a timer from the list of timers
  * @param handle the handle to the timer
  * @return E_OK in case of success, E_UNKNOWN if timer was not found.
  */
am_Error_e CAmSocketHandler::removeTimer(const sh_timerHandle_t handle)
{
    assert(handle != 0);

    //stop the current timer
#ifdef WITH_TIMERFD 
    std::list<sh_timer_s>::iterator it(mListTimer.begin());
    while (it != mListTimer.end())
    {
        if (it->handle == handle)
        {
            am_Error_e err = removeFDPoll(handle);
            close(it->fd);
            mListTimer.erase(it);
            return err;
        }
        ++it;
    }
    return (E_NON_EXISTENT);

#else
    stopTimer(handle);
    std::list<sh_timer_s>::iterator it(mListTimer.begin());
    while (it != mListTimer.end())
    {
        if (it->handle == handle)
        {
            mListTimer.erase(it);
            mSetTimerKeys.pollHandles.erase(handle);
            return (E_OK);
        }
        ++it;
    }
    return (E_UNKNOWN);
#endif
}

/**
  * restarts a timer and updates with a new interva
  * @param handle handle to the timer
  * @param timeouts new timout time
  * @return E_OK on success, E_NON_EXISTENT if the handle was not found
  */
am_Error_e CAmSocketHandler::updateTimer(const sh_timerHandle_t handle, const timespec & timeouts)
{
#ifdef WITH_TIMERFD
    std::list<sh_timer_s>::iterator it = mListTimer.begin();
    for (; it != mListTimer.end(); ++it)
    {
        if (it->handle == handle)
            break;
    }
    if (it == mListTimer.end())
        return (E_NON_EXISTENT);

    if (it->countdown.it_interval.tv_nsec != 0 || it->countdown.it_interval.tv_sec != 0)
        it->countdown.it_interval = timeouts;
    it->countdown.it_value = timeouts;

    if (!fdIsValid(it->fd))
    {
        am_Error_e err = createTimeFD(it->countdown, it->fd);
        if (err != E_OK)
            return err;
    }
    else
    {
        if (timerfd_settime(it->fd, 0, &it->countdown, NULL))
        {
            logError("Failed to set timer duration");
            return E_NOT_POSSIBLE;
        }
    }
#else

    //update the mList ....
    sh_timer_s timerItem;
    std::list<sh_timer_s>::iterator it(mListTimer.begin()), activeIt(mListActiveTimer.begin());
    bool found(false);
    for (; it != mListTimer.end(); ++it)
    {
        if (it->handle == handle)
        {
            it->countdown = timeouts;
            timerItem = *it;
            found = true;
            break;
        }
    }
    if (!found)
    return (E_NON_EXISTENT);

    found = false;

    //we add here the time difference between startTime and currenttime, because this time will be substracted later on in timecorrection
    timespec currentTime, timeoutsCorrected;
    currentTime.tv_nsec=timeoutsCorrected.tv_nsec=0;
    currentTime.tv_sec=timeoutsCorrected.tv_sec=0;
    clock_gettime(CLOCK_MONOTONIC, &currentTime);
    if (!mDispatchDone)//the mainloop is started
    timeoutsCorrected = timespecAdd(timeouts, timespecSub(currentTime, mStartTime));

    for (; activeIt != mListActiveTimer.end(); ++activeIt)
    {
        if (activeIt->handle == handle)
        {
            activeIt->countdown = timeoutsCorrected;
            found = true;
            break;
        }
    }

    if (!found)
    timerItem.countdown = timeoutsCorrected;
    mListActiveTimer.push_back(timerItem);

    mListActiveTimer.sort(compareCountdown);

#endif
    return (E_OK);
}

/**
  * restarts a timer with the original value
  * @param handle
  * @return E_OK on success, E_NON_EXISTENT if the handle was not found
  */
am_Error_e CAmSocketHandler::restartTimer(const sh_timerHandle_t handle)
{
#ifdef WITH_TIMERFD
    std::list<sh_timer_s>::iterator it = mListTimer.begin();
    for (; it != mListTimer.end(); ++it)
    {
        if (it->handle == handle)
            break;
    }
    if (it == mListTimer.end())
        return (E_NON_EXISTENT);

    if (!fdIsValid(it->fd))
    {
        am_Error_e err = createTimeFD(it->countdown, it->fd);
        if (err != E_OK)
            return err;
    }
    else
    {
        if (timerfd_settime(it->fd, 0, &it->countdown, NULL))
        {
            logError("Failed to set timer duration");
            return E_NOT_POSSIBLE;
        }
    }
#else

    sh_timer_s timerItem; //!<the original timer value
    //find the original value
    std::list<sh_timer_s>::iterator it(mListTimer.begin()), activeIt(mListActiveTimer.begin());
    bool found(false);
    for (; it != mListTimer.end(); ++it)
    {
        if (it->handle == handle)
        {
            timerItem = *it;
            found = true;
            break;
        }
    }
    if (!found)
    return (E_NON_EXISTENT);

    found = false;

    //we add here the time difference between startTime and currenttime, because this time will be substracted later on in timecorrection
    timespec currentTime, timeoutsCorrected;
    clock_gettime(CLOCK_MONOTONIC, &currentTime);
    if (!mDispatchDone)//the mainloop is started
    {
        timeoutsCorrected = timespecAdd(timerItem.countdown, timespecSub(currentTime, mStartTime));
        timerItem.countdown = timeoutsCorrected;
    }

    for (; activeIt != mListActiveTimer.end(); ++activeIt)
    {
        if (activeIt->handle == handle)
        {
            activeIt->countdown = timerItem.countdown;
            found = true;
            break;
        }
    }

    if (!found)
    mListActiveTimer.push_back(timerItem);

    mListActiveTimer.sort(compareCountdown);
#endif
    return (E_OK);
}

/**
  * stops a timer
  * @param handle
  * @return E_OK on success, E_NON_EXISTENT if the handle was not found
  */
am_Error_e CAmSocketHandler::stopTimer(const sh_timerHandle_t handle)
{
#ifdef WITH_TIMERFD
    for (auto elem : mListTimer)
    {
        if (elem.handle != handle)
            continue;

        itimerspec countdown = elem.countdown;
        countdown.it_value.tv_nsec = 0;
        countdown.it_value.tv_sec = 0;

        if (timerfd_settime(elem.fd, 0, &countdown, NULL) < 0)
        {
            logError("Failed to set timer duration");
            return E_NOT_POSSIBLE;
        }

        return E_OK;
    }
#else
    //go through the list and remove the timer with the handle
    std::list<sh_timer_s>::iterator it(mListActiveTimer.begin());
    while (it != mListActiveTimer.end())
    {
        if (it->handle == handle)
        {
            mListActiveTimer.erase(it);
            return E_OK;
        }
        ++it;
    }
#endif

    return E_NON_EXISTENT;
}

/**
  * updates the eventFlags of a poll
  * @param handle
  * @param events
  * @return @return E_OK on succsess, E_NON_EXISTENT if fd was not found
  */
am_Error_e CAmSocketHandler::updateEventFlags(const sh_pollHandle_t handle, const short events)
{
    for (auto& it : mMapShPoll)
    {
        auto& elem = it.second;
        if (elem.handle != handle)
            continue;

        switch (elem.state)
        {
            case poll_states_e::ADD:
                elem.pollfdValue.events = events;
                return (E_OK);

            case poll_states_e::UPDATE:
            case poll_states_e::VALID:
                elem.state = poll_states_e::UPDATE;
                elem.pollfdValue.revents = 0;
                elem.pollfdValue.events = events;
                return (E_OK);

            default:
                // This issue should never happen!
                return (E_DATABASE_ERROR);
        }
    }
    return (E_UNKNOWN);
}

/**
  * checks if a filedescriptor is validCAmShSubstractTime
  * @param fd the filedescriptor
  * @return true if the fd is valid
  */
bool CAmSocketHandler::fdIsValid(const int fd) const
{
    return (fcntl(fd, F_GETFL) != -1 || errno != EBADF);
}

#ifndef WITH_TIMERFD
/**
  * timer is up.
  */
void CAmSocketHandler::timerUp()
{
    //find out the timedifference to starttime
    static timespec currentTime, diffTime;
    clock_gettime(CLOCK_MONOTONIC, &currentTime);
    diffTime = timespecSub(currentTime, mStartTime);

    static auto countdownUp = [&](const sh_timer_s& row)->bool
    {
        timespec sub = timespecSub(row.countdown, diffTime);
        if (sub.tv_nsec == 0 && sub.tv_sec == 0)
        return (true);
        return (false);
    };

    //now we need to substract the diffTime from all timers and see if they are up
    std::list<sh_timer_s>::reverse_iterator overflowIter = std::find_if(mListActiveTimer.rbegin(), mListActiveTimer.rend(), countdownUp);

    //copy all fired timers into a list
    std::vector<sh_timer_s> tempList(overflowIter, mListActiveTimer.rend());

    //erase all fired timers
    std::list<sh_timer_s>::iterator it(overflowIter.base());
    mListActiveTimer.erase(mListActiveTimer.begin(), it);

    //call the callbacks for the timers
    std::for_each(tempList.begin(), tempList.end(), CAmSocketHandler::callTimer);
}

/**
  * correct timers and fire the ones who are up
  */
void CAmSocketHandler::timerCorrection()
{
    //get the current time and calculate the correction value
    static timespec currentTime, correctionTime;
    clock_gettime(CLOCK_MONOTONIC, &currentTime);
    correctionTime = timespecSub(currentTime, mStartTime);
    mStartTime = currentTime;

    static auto countdownZero = [](const sh_timer_s& row)->bool
    {
        if (row.countdown.tv_nsec == 0 && row.countdown.tv_sec == 0)
        return (true);
        return (false);
    };

    static auto substractTime = [&](sh_timer_s& t)
    {
        t.countdown = timespecSub(t.countdown, correctionTime);
    };

    if (!mListActiveTimer.empty())
    {

        //subtract the correction value from all items in the list
        std::for_each(mListActiveTimer.begin(), mListActiveTimer.end(), substractTime);

        //find the last occurrence of zero -> timer overflowed
        std::list<sh_timer_s>::reverse_iterator overflowIter = std::find_if(mListActiveTimer.rbegin(), mListActiveTimer.rend(), countdownZero);

        //only if a timer overflowed
        if (overflowIter != mListActiveTimer.rend())
        {
            //copy all timers that need to be called to a new list
            std::vector<sh_timer_s> tempList(overflowIter, mListActiveTimer.rend());

            //erase all fired timers
            std::list<sh_timer_s>::iterator it(overflowIter.base());
            mListActiveTimer.erase(mListActiveTimer.begin(), it);

            //call the callbacks for the timers
            std::for_each(tempList.begin(), tempList.end(), CAmSocketHandler::callTimer);
        }
    }
}
#endif

/**
  * prepare for poll
  */
void CAmSocketHandler::prepare(am::CAmSocketHandler::sh_poll_s& row)
{
    if (!row.prepareCB)
        return;

    try
    {
        row.prepareCB(row.handle, row.userData);
    }
    catch (std::exception& e)
    {
        logError("CAmSocketHandler::prepare Exception caught", e.what());
    }
}

/**
  * fire callback
  */
void CAmSocketHandler::fire(const sh_poll_s& a)
{
    try
    {
        a.firedCB(a.pollfdValue, a.handle, a.userData);
    }
    catch (std::exception& e)
    {
        logError("CAmSocketHandler::fire Exception caught", e.what());
    }
}

/**
  * should disptach
  */
bool CAmSocketHandler::noDispatching(const sh_poll_s* a)
{
    //remove from list of there is no checkCB
    if (nullptr == a->checkCB || a->state != poll_states_e::VALID)
        return (true);
    return (!a->checkCB(a->handle, a->userData));
}

/**
  * disptach
  */
bool CAmSocketHandler::dispatchingFinished(const sh_poll_s* a)
{
    //remove from list of there is no dispatchCB
    if (nullptr == a->dispatchCB || a->state != poll_states_e::VALID)
        return (true);

    return (!a->dispatchCB(a->handle, a->userData));
}

/**
  * is used to set the pointer for the ppoll command
  * @param buffertime
  * @return
  */
inline timespec* CAmSocketHandler::insertTime(timespec& buffertime)
{
#ifndef WITH_TIMERFD
    if (!mListActiveTimer.empty())
    {
        buffertime = mListActiveTimer.front().countdown;
        return (&buffertime);
    }
    else
#endif    
    {
        return (NULL);
    }
}

#ifdef WITH_TIMERFD   
am_Error_e CAmSocketHandler::createTimeFD(const itimerspec & timeouts, int & fd)
{
    fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (fd < 0)
    {
        logError("CAmSocketHandler::createTimeFD Failed with", static_cast<const char*>(std::strerror(errno)));
        return E_NOT_POSSIBLE;
    }

    if (timerfd_settime(fd, 0, &timeouts, NULL) < 0)
    {
        logError("CAmSocketHandler::createTimeFD Failed to set duration for", fd);
        close(fd);
        fd = -1;
        return E_NOT_POSSIBLE;
    }
    return E_OK;
}
#endif 

void CAmSocketHandler::callTimer(sh_timer_s& a)
{
    try
    {
        a.callback(a.handle, a.userData);
    }
    catch (std::exception& e)
    {
        logError("CAmSocketHandler::callTimer() Exception caught", e.what());
    }
}

bool CAmSocketHandler::nextHandle(sh_identifier_s & handle)
{
    //create a new handle for the poll
    const sh_pollHandle_t lastHandle(handle.lastUsedID);
    do
    {
        ++handle.lastUsedID;
        if (handle.lastUsedID == handle.limit)
        {
            handle.lastUsedID = 1;
        }
        if (handle.lastUsedID == lastHandle)
        {
            return (false);
        }

    } while (handle.pollHandles.find(handle.lastUsedID) != handle.pollHandles.end());

    handle.pollHandles.insert(handle.lastUsedID);

    return (true);
}

}

