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
#include <time.h>
#include <algorithm>
#include <features.h>
#include <csignal>
#include "shared/CAmDltWrapper.h"

namespace am
{

CAmSocketHandler* CAmSocketHandler::mInstance=NULL;

CAmSocketHandler::CAmSocketHandler() :
        mPipe(),
        mListPoll(), //
        mListTimer(), //
        mListActiveTimer(), //
        mLastInsertedHandle(0), //
        mLastInsertedPollHandle(0), //
        mRecreatePollfds(true), //
        mStartTime(), //
        receiverCallbackT(this, &CAmSocketHandler::receiverCallback),//
        checkerCallbackT(this, &CAmSocketHandler::checkerCallback)//
{
    gDispatchDone = 1;
    mInstance=this;

    if (pipe(mPipe) == -1)
    {
        logError("CAmSerializer could not create pipe!");
    }

    //add the pipe to the poll - nothing needs to be proccessed here we just need the pipe to trigger the ppoll
    short event = 0;
    sh_pollHandle_t handle;
    event |= POLLIN;
    addFDPoll(mPipe[0], event, NULL, &receiverCallbackT, &checkerCallbackT, NULL, NULL, handle);
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

    //prepare the signalmask
    sigset_t sigmask;
    sigemptyset(&sigmask);
    sigaddset(&sigmask, SIGINT);
    sigaddset(&sigmask, SIGQUIT);
    sigaddset(&sigmask, SIGTERM);
    sigaddset(&sigmask, SIGHUP);
    sigaddset(&sigmask, SIGQUIT);

    clock_gettime(CLOCK_MONOTONIC, &mStartTime);
    while (!gDispatchDone)
    {
        //first we go through the registered filedescriptors and check if someone needs preparation:
        std::for_each(mListPoll.begin(), mListPoll.end(), CAmShCallPrep());

        if (mRecreatePollfds)
        {
            mfdPollingArray.clear();
            //there was a change in the setup, so we need to recreate the fdarray from the list
            std::for_each(mListPoll.begin(), mListPoll.end(), CAmShCopyPollfd(mfdPollingArray));
            mRecreatePollfds = false;
        }

        timerCorrection();

        //block until something is on a filedescriptor

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

        if (pollStatus != 0) //only check filedescriptors if there was a change
        {
            //todo: here could be a timer that makes sure naughty plugins return!

            //freeze mListPoll by copying it - otherwise we get problems when we want to manipulate it during the next lines
            std::list<sh_poll_s> listPoll;
            mListPoll_t::iterator listmPollIt;

            //remove all filedescriptors who did not fire
            std::vector<pollfd>::iterator it = mfdPollingArray.begin();
            do
            {
                it = std::find_if(it, mfdPollingArray.end(), eventFired);
                if (it != mfdPollingArray.end())
                {
                    listmPollIt = mListPoll.begin();
                    std::advance(listmPollIt, std::distance(mfdPollingArray.begin(), it));
                    listPoll.push_back(*listmPollIt);
                    listPoll.back().pollfdValue = *it;
                    it++;
                }
            } while (it != mfdPollingArray.end());

            //stage 1, call firedCB
            std::for_each(listPoll.begin(), listPoll.end(), CAmShCallFire());

            //stage 2, lets ask around if some dispatching is necessary, the ones who need stay on the list
            listPoll.remove_if(noDispatching);

            //stage 3, the ones left need to dispatch, we do this as long as there is something to dispatch..
            do
            {
                listPoll.remove_if(dispatchingFinished);
            } while (!listPoll.empty());

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

    //this is for all running timers only - we need to handle the additional offset here
    if (!mListActiveTimer.empty())
    {
        timespec currentTime, correctionTime;
        clock_gettime(CLOCK_MONOTONIC, &currentTime);
        correctionTime = timespecSub(currentTime, mStartTime);
        std::for_each(mListActiveTimer.begin(), mListActiveTimer.end(), CAmShSubstractTime(correctionTime));
    }

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
am_Error_e CAmSocketHandler::addTimer(const timespec timeouts, IAmShTimerCallBack* callback, sh_timerHandle_t& handle, void * userData)
{
    assert(!((timeouts.tv_sec==0) && (timeouts.tv_nsec==0)));
    assert(callback!=NULL);

    sh_timer_s timerItem;

    //create a new handle for the timer
    handle = ++mLastInsertedHandle; //todo: overflow ruling !o
    timerItem.handle = handle;
    timerItem.countdown = timeouts;
    timerItem.callback = callback;
    timerItem.userData = userData;

    mListTimer.push_back(timerItem);

    //we add here the time difference between startTime and currenttime, because this time will be substracted later on in timecorrection
    timespec currentTime;
    clock_gettime(CLOCK_MONOTONIC, &currentTime);
    if (!gDispatchDone) //the mainloop is started
        timerItem.countdown = timespecAdd(timeouts, timespecSub(currentTime, mStartTime));

    mListActiveTimer.push_back(timerItem);
    mListActiveTimer.sort(compareCountdown);
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

    std::list<sh_timer_s>::iterator it(mListTimer.begin());
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
 * restarts a timer and updates with a new interva
 * @param handle handle to the timer
 * @param timeouts new timout time
 * @return E_OK on success, E_NON_EXISTENT if the handle was not found
 */
am_Error_e CAmSocketHandler::updateTimer(const sh_timerHandle_t handle, const timespec timeouts)
{
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
    if (!gDispatchDone) //the mainloop is started
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
    return (E_OK);
}

/**
 * restarts a timer with the original value
 * @param handle
 * @return E_OK on success, E_NON_EXISTENT if the handle was not found
 */
am_Error_e CAmSocketHandler::restartTimer(const sh_timerHandle_t handle)
{
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
    if (!gDispatchDone) //the mainloop is started
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

    return (E_OK);
}

/**
 * stops a timer
 * @param handle
 * @return E_OK on success, E_NON_EXISTENT if the handle was not found
 */
am_Error_e CAmSocketHandler::stopTimer(const sh_timerHandle_t handle)
{
    //go through the list and remove the timer with the handle
    std::list<sh_timer_s>::iterator it(mListActiveTimer.begin());
    for (; it != mListActiveTimer.end(); ++it)
    {
        if (it->handle == handle)
        {
            it = mListActiveTimer.erase(it);
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
 * checks if a filedescriptor is validCAmShSubstractTime
 * @param fd the filedescriptor
 * @return true if the fd is valid
 */
bool CAmSocketHandler::fdIsValid(const int fd) const
{
    return (fcntl(fd, F_GETFL) != -1 || errno != EBADF);
}

/**
 * timer is up.
 */
void CAmSocketHandler::timerUp()
{
    //find out the timedifference to starttime
    timespec currentTime, diffTime;
    clock_gettime(CLOCK_MONOTONIC, &currentTime);
    diffTime = timespecSub(currentTime, mStartTime);

    //now we need to substract the diffTime from all timers and see if they are up
    std::list<sh_timer_s>::reverse_iterator overflowIter = std::find_if(mListActiveTimer.rbegin(), mListActiveTimer.rend(), CAmShCountdownUp(diffTime));

    //copy all fired timers into a list
    std::vector<sh_timer_s> tempList(overflowIter, mListActiveTimer.rend());

    //erase all fired timers
    std::list<sh_timer_s>::iterator it(overflowIter.base());
    mListActiveTimer.erase(mListActiveTimer.begin(), it);

    //call the callbacks for the timers
    std::for_each(tempList.begin(), tempList.end(), CAmShCallTimer());
}

/**
 * correct timers and fire the ones who are up
 */
void CAmSocketHandler::timerCorrection()
{
    //get the current time and calculate the correction value
    timespec currentTime, correctionTime;
    clock_gettime(CLOCK_MONOTONIC, &currentTime);
    correctionTime = timespecSub(currentTime, mStartTime);
    mStartTime = currentTime;

    if (!mListActiveTimer.empty())
    {

        //subtract the correction value from all items in the list
        std::for_each(mListActiveTimer.begin(), mListActiveTimer.end(), CAmShSubstractTime(correctionTime));

        //find the last occurrence of zero -> timer overflowed
        std::list<sh_timer_s>::reverse_iterator overflowIter = std::find_if(mListActiveTimer.rbegin(), mListActiveTimer.rend(), CAmShCountdownZero());

        //only if a timer overflowed
        if (overflowIter != mListActiveTimer.rend())
        {
            //copy all timers that need to be called to a new list
            std::vector<sh_timer_s> tempList(overflowIter, mListActiveTimer.rend());

            //erase all fired timers
            std::list<sh_timer_s>::iterator it(overflowIter.base());
            mListActiveTimer.erase(mListActiveTimer.begin(), it);

            //call the callbacks for the timers
            std::for_each(tempList.begin(), tempList.end(), CAmShCallTimer());
        }
    }
}

void CAmSocketHandler::exit_mainloop()
{
    //end the while loop
    stop_listening();

    //fire the ending filedescriptor
    int p(1);
    write(mPipe[1], &p, sizeof(p));
}

void CAmSocketHandler::static_exit_mainloop()
{
    if (mInstance!=0)
    {
        mInstance->exit_mainloop();
    }
}

/**
 * is used to set the pointer for the ppoll command
 * @param buffertime
 * @return
 */
inline timespec* CAmSocketHandler::insertTime(timespec& buffertime)
{
    if (!mListActiveTimer.empty())
    {
        buffertime = mListActiveTimer.front().countdown;
        return (&buffertime);
    }
    else
    {
        return (NULL);
    }
}

}

