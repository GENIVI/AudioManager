/**
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
 * \author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
 *
 * \file CAmSocketHandler.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include "shared/CAmSocketHandler.h"
#include <config.h>
#include <cassert>
#include <sys/fcntl.h>
#include <sys/errno.h>
#include <sys/poll.h>
#include <algorithm>
#include <time.h>
#include <features.h>
#include <csignal>
#include "shared/CAmDltWrapper.h"

//todo: implement time correction if timer was interrupted by call
//todo: change hitlist to a list that holds all information, because entering and removing items will be cheaper than with std::vector

namespace am
{

CAmSocketHandler::CAmSocketHandler() :
        mListPoll(), //
        mListTimer(), //
        mListActiveTimer(), //
        mNextTimer(), //
        mLastInsertedHandle(0), //
        mLastInsertedPollHandle(0), //
        mRecreatePollfds(true), //
        mTimeout()
{
    mTimeout.tv_nsec = -1;
    mTimeout.tv_sec = -1;
    gDispatchDone = 0;
}

CAmSocketHandler::~CAmSocketHandler()
{
}

//todo: maybe have some: give me more time returned?
/**
 * start the block listening for filedescriptors. This is the mainloop.
 */
void CAmSocketHandler::start_listenting()
{
    gDispatchDone = 0;
    int16_t pollStatus;
    std::list<int16_t> hitList;

    //init the timer
    initTimer();

    //prepare the signalmask
    sigset_t sigmask;
    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGINT);
    sigaddset(&sigmask, SIGQUIT);
    sigaddset(&sigmask, SIGTERM);
    sigaddset(&sigmask, SIGHUP);
    sigaddset(&sigmask, SIGQUIT);

    while (!gDispatchDone)
    {
        //first we go through the registered filedescriptors and check if someone needs preparation:
        mListPoll_t::iterator prepIter = mListPoll.begin();
        IAmShPollPrepare *prep = NULL;
        for (; prepIter != mListPoll.end(); ++prepIter)
        {
            if ((prep = prepIter->prepareCB) != NULL)
                prep->Call(prepIter->handle, prepIter->userData);
        }

        if (mRecreatePollfds)
        {
            mfdPollingArray.clear();
            //there was a change in the setup, so we need to recreate the fdarray from the list
            std::for_each(mListPoll.begin(), mListPoll.end(), CAmShCopyPollfd(mfdPollingArray));
            mRecreatePollfds = false;
        }

        //block until something is on a filedescriptor
#ifdef WITH_PPOLL

        timespec buffertime;
        if ((pollStatus = ppoll(&mfdPollingArray[0], mfdPollingArray.size(), insertTime(buffertime), &sigmask)) < 0)
        {
            if (errno == EINTR)
            {
                //a signal was received, that means it's time to go...
                pollStatus = 0;
            }
            else
            {
                logError("SocketHandler::start_listenting ppoll returned with error", errno);
                exit(0);
            }
        }

#else
        //sigprocmask (SIG_SETMASK, &mask, &oldmask);
        if((pollStatus=poll(&mfdPollingArray[0],mfdPollingArray.size(),timespec2ms(mTimeout)))<0)
        {

            if(errno==EINTR)
            {
                //a signal was received, that means it's time to go...
                //todo: add things to do here before going to sleep
                exit(0);
            }
            logError("SocketHandler::start_listenting poll returned with error",errno);
            exit(0);
        }
        //sigprocmask (SIG_SETMASK, &oldmask, NULL);
#endif

        if (pollStatus != 0) //only check filedescriptors if there was a change
        {
            //todo: here could be a timer that makes sure naughty plugins return!

            //freeze mListPoll by copying it - otherwise we get problems when we want to manipulate it during the next lines
            mListPoll_t listPoll(mListPoll);

            //get all indexes of the fired events and save them int hitList
            hitList.clear();
            std::vector<pollfd>::iterator it = mfdPollingArray.begin();
            do
            {
                it = std::find_if(it, mfdPollingArray.end(), onlyFiredEvents);
                if (it != mfdPollingArray.end())
                    hitList.push_back(std::distance(mfdPollingArray.begin(), it++));

            } while (it != mfdPollingArray.end());

            //stage 1, call firedCB for all matched events, but only if callback is not zero!
            std::list<int16_t>::iterator hListIt = hitList.begin();
            for (; hListIt != hitList.end(); ++hListIt)
            {
                IAmShPollFired* fire = NULL;
                if ((fire = listPoll.at(*hListIt).firedCB) != NULL)
                    fire->Call(mfdPollingArray.at(*hListIt), listPoll.at(*hListIt).handle, listPoll.at(*hListIt).userData);
            }

            //stage 2, lets ask around if some dispatching is necessary, if not, they are taken from the hitlist
            hListIt = hitList.begin();
            for (; hListIt != hitList.end(); ++hListIt)
            {
                IAmShPollCheck* check = NULL;
                if ((check = listPoll.at(*hListIt).checkCB) != NULL)
                {
                    if (!check->Call(listPoll.at(*hListIt).handle, listPoll.at(*hListIt).userData))
                    {
                        hListIt = hitList.erase(hListIt);
                    }
                }
            }

            //stage 3, the ones left need to dispatch, we do this as long as there is something to dispatch..
            do
            {
                hListIt = hitList.begin();
                for (; hListIt != hitList.end(); ++hListIt)
                {
                    IAmShPollDispatch *dispatch = NULL;
                    if ((dispatch = listPoll.at(*hListIt).dispatchCB) != NULL)
                    {
                        if (!dispatch->Call(listPoll.at(*hListIt).handle, listPoll.at(*hListIt).userData))
                        {
                            hListIt = hitList.erase(hListIt);
                        }
                    }
                    else //there is no dispatch function, so we just remove the file from the list...
                    {
                        hListIt = hitList.erase(hListIt);
                    }
                }
            } while (!hitList.empty());

        }
        else //Timerevent
        {
            //this was a timer event, we need to take care about the timers
            timerUp();
        }
    }
}

/**
 * exits the loop
 */
void CAmSocketHandler::stop_listening()
{
    gDispatchDone = 1;
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
 * @return E_OK if the descriptor was added, E_NON_EXISTENT if the fd is not valid
 */
am_Error_e CAmSocketHandler::addFDPoll(const int fd, const short event, IAmShPollPrepare *prepare, IAmShPollFired *fired, IAmShPollCheck *check, IAmShPollDispatch *dispatch, void *userData, sh_pollHandle_t & handle)
{
    if (!fdIsValid(fd))
        return (E_NON_EXISTENT);

    sh_poll_s pollData;
    pollData.pollfdValue.fd = fd;
    pollData.handle = ++mLastInsertedPollHandle;
    pollData.pollfdValue.events = event;
    pollData.pollfdValue.revents = 0;
    pollData.userData = userData;
    pollData.prepareCB = prepare;
    pollData.firedCB = fired;
    pollData.checkCB = check;
    pollData.dispatchCB = dispatch;

    //add new data to the list
    mListPoll.push_back(pollData);

    mRecreatePollfds = true;

    handle = pollData.handle;
    return (E_OK);
}

/**
 * removes a filedescriptor from the poll loop
 * @param handle
 * @return
 */
am_Error_e CAmSocketHandler::removeFDPoll(const sh_pollHandle_t handle)
{
    mListPoll_t::iterator iterator = mListPoll.begin();

    for (; iterator != mListPoll.end(); ++iterator)
    {
        if (iterator->handle == handle)
        {
            iterator = mListPoll.erase(iterator);
            mRecreatePollfds = true;
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
am_Error_e CAmSocketHandler::addTimer(const timespec timeouts, IAmShTimerCallBack*& callback, sh_timerHandle_t& handle, void * userData)
{
    assert(!((timeouts.tv_sec==0) && (timeouts.tv_nsec==0)));
    assert(callback!=NULL);

    sh_timer_s timerItem;

    //create a new handle for the timer
    handle = ++mLastInsertedHandle; //todo: overflow ruling !
    timerItem.handle = handle;
    timerItem.countdown = timeouts;
    timerItem.timeout = timeouts;
    timerItem.callback = callback;
    timerItem.userData = userData;

    //add timer to the list
    mListActiveTimer.push_back(timerItem);
    mListTimer.push_back(timerItem);

    //very important: sort the list so that the smallest value is front
    mListActiveTimer.sort(compareCountdown);
    mTimeout = mListActiveTimer.front().countdown;
    return (E_OK);
}

/**
 * removes a timer from the list of timers
 * @param handle the handle to the timer
 * @return E_OK in case of success, E_UNKNOWN if timer was not found.
 */
am_Error_e CAmSocketHandler::removeTimer(const sh_timerHandle_t handle)
{
    assert(handle!=0);

    //stop the current timer
    stopTimer(handle);

    std::list<sh_timer_s>::iterator it = mListTimer.begin();
    for (; it != mListTimer.end(); ++it)
    {
        if (it->handle == handle)
        {
            it = mListTimer.erase(it);
            return (E_OK);
        }
    }
    return (E_UNKNOWN);
}

/**
 * restarts a timer and updates with a new interval
 * @param handle handle to the timer
 * @param timeouts new timout time
 * @return E_OK on success, E_NON_EXISTENT if the handle was not found
 */
am_Error_e CAmSocketHandler::restartTimer(const sh_timerHandle_t handle, const timespec timeouts)
{
    sh_timer_s timerItem;
    std::list<sh_timer_s>::iterator it = mListTimer.begin();
    for (; it != mListTimer.end(); ++it)
    {
        if (it->handle == handle)
        {
            timerItem = *it;
            break;
        }
    }

    if (timeouts.tv_nsec != -1 && timeouts.tv_sec != -1)
    {
        timerItem.timeout = timeouts;
    }

    mListActiveTimer.push_back(timerItem);

    //very important: sort the list so that the smallest value is front
    mListActiveTimer.sort(compareCountdown);
    mTimeout = mListActiveTimer.front().countdown;
    return (E_OK);
}

am_Error_e CAmSocketHandler::stopTimer(const sh_timerHandle_t handle)
{
    //go through the list and remove the timer with the handle
    std::list<sh_timer_s>::iterator it = mListActiveTimer.begin();
    for (; it != mListActiveTimer.end(); ++it)
    {
        if (it->handle == handle)
        {
            it = mListActiveTimer.erase(it);
            if (!mListActiveTimer.empty())
            {
                mTimeout = mListActiveTimer.front().countdown;
            }
            else
            {
                mTimeout.tv_nsec = -1;
                mTimeout.tv_sec = -1;
            }
            return (E_OK);
        }
    }
    return (E_NON_EXISTENT);
}

/**
 * updates the eventFlags of a poll
 * @param handle
 * @param events
 * @return @return E_OK on succsess, E_NON_EXISTENT if fd was not found
 */
am_Error_e CAmSocketHandler::updateEventFlags(const sh_pollHandle_t handle, const short events)
{
    mListPoll_t::iterator iterator = mListPoll.begin();

    for (; iterator != mListPoll.end(); ++iterator)
    {
        if (iterator->handle == handle)
        {
            iterator->pollfdValue.events = events;
            mRecreatePollfds = true;
            return (E_OK);
        }
    }
    return (E_UNKNOWN);
}

/**
 * checks if a filedescriptor is valid
 * @param fd the filedescriptor
 * @return true if the fd is valid
 */
bool CAmSocketHandler::fdIsValid(const int fd) const
{
    return (fcntl(fd, F_GETFL) != -1 || errno != EBADF);
}

/**
 * whenever a timer is up, this function needs to be called.
 * Removes the fired timer, calls the callback and resets mTimeout
 */
void CAmSocketHandler::timerUp()
{
    //first fire the event
    mListActiveTimer.front().callback->Call(mListActiveTimer.front().handle, mListActiveTimer.front().userData);

    //then remove the first timer, the one who fired
    mListActiveTimer.pop_front();
    if (!mListActiveTimer.empty())
    {
        //substract the old value from all timers in the list
        std::for_each(mListActiveTimer.begin(), mListActiveTimer.end(), CAmShSubstractTime(mTimeout));
        mTimeout = mListActiveTimer.front().countdown;
    }
    else
    {
        mTimeout.tv_nsec = -1;
        mTimeout.tv_sec = -1;
    }
}

/**
 * init the timers
 */
void CAmSocketHandler::initTimer()
{
    if (!mListActiveTimer.empty())
    {
        mTimeout = mListActiveTimer.front().countdown;
    }
    else
    {
        mTimeout.tv_nsec = -1;
        mTimeout.tv_sec = -1;
    }
}

/**
 * convert timespec to milliseconds
 * @param time time in timespec
 * @return time in milliseconds
 */
inline int CAmSocketHandler::timespec2ms(const timespec & time)
{
    return ((time.tv_nsec == -1 && time.tv_sec == -1) ? -1 : time.tv_sec * 1000 + time.tv_nsec / 1000000);
}

inline timespec* CAmSocketHandler::insertTime(timespec& buffertime)
{
    buffertime.tv_nsec = mTimeout.tv_nsec;
    buffertime.tv_sec = mTimeout.tv_sec;
    return ((mTimeout.tv_nsec == -1 && mTimeout.tv_sec == -1) ? NULL : &buffertime);
}

/**
 * functor to easy substract from each countdown
 * @param t value to substract from
 */
void CAmSocketHandler::CAmShSubstractTime::operator ()(sh_timer_s & t) const
{
    int val = 0;
    if ((val = t.countdown.tv_nsec - param.tv_nsec) < 0)
    {
        t.countdown.tv_nsec = 1000000000 + val;
        t.countdown.tv_sec--;
    }
    else
    {
        t.countdown.tv_nsec = val;
    }
    (t.countdown.tv_sec - param.tv_sec) < 0 ? 0 : (t.countdown.tv_sec -= param.tv_sec);
}

void CAmSocketHandler::CAmShCopyPollfd::operator ()(const sh_poll_s & row)
{
    pollfd temp = row.pollfdValue;
    temp.revents = 0;
    mArray.push_back(temp);
}
}
