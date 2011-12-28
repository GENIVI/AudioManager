/**
* Copyright (C) 2011, BMW AG
*
* GeniviAudioMananger AudioManagerDaemon
*
* \file DBusWrapper.cpp
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

#include "SocketHandler.h"
#include <assert.h>
#include <sys/fcntl.h>
#include <sys/errno.h>
#include <sys/poll.h>
#include <algorithm>
#include <time.h>
#include <features.h>

//todo: implement ppoll
//todo: signal handling here
//todo: implement time correction if timer was interrupted by call

#include <iostream>  //todo remove

namespace am {

SocketHandler::SocketHandler()
	:mListPoll(),
	 mListTimer(),
	 mListActiveTimer(),
	 mNextTimer(),
	 mLastInsertedHandle(0),
	 mLastInsertedPollHandle(0),
	 mDispatch(true),
	 mRecreatePollfds(true)
{
	mTimeout.tv_nsec=-1;
	mTimeout.tv_sec=-1;
}

SocketHandler::~SocketHandler()
{
}


//todo: maybe have some: give me more time returned?
/**
 * start the block listening for filedescriptors. This is the mainloop.
 */
void SocketHandler::start_listenting()
{
	int16_t pollStatus;
	std::list<int16_t>hitList;

	//init the timer
	initTimer();

	while (mDispatch)
	{
		//first we go through the registered filedescriptors and check if someone needs preparation:
		mListPoll_t::iterator prepIter=mListPoll.begin();
		shPollPrepare *prep=NULL;
		for(;prepIter!=mListPoll.end();++prepIter)
		{
			if((prep=prepIter->prepareCB)!=NULL) prep->Call(prepIter->handle,prepIter->userData);
		}

		if(mRecreatePollfds)
		{
			mfdPollingArray.clear();
			//there was a change in the setup, so we need to recreate the fdarray from the list
			std::for_each(mListPoll.begin(),mListPoll.end(),CopyPollfd(mfdPollingArray));
			mRecreatePollfds=false;
		}

		//block until something is on a filedescriptor
		if((pollStatus=poll(&mfdPollingArray.front(),mfdPollingArray.size(),timespec2ms(mTimeout)))==-1)
		{
			//todo enter DLT message here;
		}

		if (pollStatus!=0) //only check filedescriptors if there was a change
		{
			//todo: here could be a timer that makes sure naughty plugins return!

			//get all indexes of the fired events and save them int hitList
			hitList.clear();
			std::vector<pollfd>::iterator it=mfdPollingArray.begin();
			do
			{
				it=std::find_if(it,mfdPollingArray.end(),onlyFiredEvents);
				if (it!=mfdPollingArray.end()) hitList.push_back(std::distance(mfdPollingArray.begin(), it++));

			} while (it!=mfdPollingArray.end());

			//stage 1, call firedCB for all matched events, but only if callback is not zero!
			std::list<int16_t>::iterator hListIt=hitList.begin();
			for(;hListIt!=hitList.end();++hListIt)
			{
				shPollFired* fire=NULL;
				if ((fire=mListPoll.at(*hListIt).firedCB)!=NULL) fire->Call(mfdPollingArray.at(*hListIt),mListPoll.at(*hListIt).handle,mListPoll.at(*hListIt).userData);
			}

			//stage 2, lets ask around if some dispatching is necessary, if not, they are taken from the hitlist
			hListIt=hitList.begin();
			for(;hListIt!=hitList.end();++hListIt)
			{
				shPollCheck* check=NULL;
				if ((check=mListPoll.at(*hListIt).checkCB)!=NULL)
				{
					if (!check->Call(mListPoll.at(*hListIt).handle,mListPoll.at(*hListIt).userData))
					{
						hListIt=hitList.erase(hListIt);
					}
				}
			}

			//stage 3, the ones left need to dispatch, we do this as long as there is something to dispatch..
			do
			{
				hListIt=hitList.begin();
				for(;hListIt!=hitList.end();++hListIt)
				{
					shPollDispatch *dispatch=NULL;
					if((dispatch=mListPoll.at(*hListIt).dispatchCB)!=NULL)
					{
						if (!dispatch->Call(mListPoll.at(*hListIt).handle,mListPoll.at(*hListIt).userData))
						{
							hListIt=hitList.erase(hListIt);
						}
					}
					else //there is no dispatch function, so we just remove the file from the list...
					{
						hListIt=hitList.erase(hListIt);
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
void SocketHandler::stop_listening()
{
	mDispatch=false;
}

/**
 * Adds a filedescriptor to the polling loop
 * @param fd this is a valid filedescriptor
 * @param event the event flags
 * @param callback the callback that shall be called if the filedescriptor poll succeeded
 * @return E_OK if the descriptor was added, E_NON_EXISTENT if the fd is not valid
 */
am_Error_e SocketHandler::addFDPoll(const int fd,const int16_t event, shPollPrepare *prepare,shPollFired *fired,shPollCheck *check,shPollDispatch *dispatch, void* userData,sh_pollHandle_t& handle)
{
	if (!fdIsValid(fd)) return E_NON_EXISTENT;

	sh_poll_s pollData;
	pollData.pollfdValue.fd=fd;
	pollData.handle=++mLastInsertedPollHandle;
	pollData.pollfdValue.events=event;
	pollData.pollfdValue.revents=0;
	pollData.userData=userData;
	pollData.prepareCB=prepare;
	pollData.firedCB=fired;
	pollData.checkCB=check;
	pollData.dispatchCB=dispatch;

	//add new data to the list
	mListPoll.push_back(pollData);

	mRecreatePollfds=true;

	handle=pollData.handle;
	return E_OK;
}

/**
 * removes a filedescriptor from the poll loop
 * @param fd the filedescriptor to be removed
 * @return E_OK in case of sucess, E_NON_EXISTENT or E_UNKNOWN if the fd in not registered
 */
am_Error_e SocketHandler::removeFDPoll(const sh_pollHandle_t handle)
{
	mListPoll_t::iterator iterator=mListPoll.begin();

	for (;iterator!=mListPoll.end();++iterator)
	{
		if(iterator->handle==handle)
		{
			iterator=mListPoll.erase(iterator);
			mRecreatePollfds=true;
			return E_OK;
		}
	}
	return E_UNKNOWN;
}

/**
 * adds a timer to the list of timers. The callback will be fired when the timer is up.
 * This is not a high precise timer, it is very coarse. It is meant to be used for timeouts when waiting
 * for an answer via a filedescriptor.
 * One time timer. If you need again a timer, you need to add a new timer in the callback of the old one.
 * @param timeouts time until the callback is fired
 * @param callback the callback
 * @param handle the handle that is created for the timer is returned. Can be used to remove the timer
 * @return E_OK in case of success
 */
am_Error_e SocketHandler::addTimer(const timespec timeouts,shTimerCallBack*& callback,sh_timerHandle_t& handle, void * userData)
{
	assert(!((timeouts.tv_sec==0) && (timeouts.tv_nsec==0)));
	assert(callback!=NULL);

	timer_s timerItem;

	//create a new handle for the timer
	handle=++mLastInsertedHandle;  //todo: overflow ruling !
	timerItem.handle=handle;
	timerItem.countdown=timeouts;
	timerItem.timeout=timeouts;
	timerItem.callback=callback;
	timerItem.userData=userData;

	//add timer to the list
	mListActiveTimer.push_back(timerItem);
	mListTimer.push_back(timerItem);

	//very important: sort the list so that the smallest value is front
	mListActiveTimer.sort(compareCountdown);
	mTimeout=mListActiveTimer.front().countdown;
	return E_OK;
}

/**
 * removes a timer from the list of timers
 * @param handle the handle to the timer
 * @return E_OK in case of success, E_UNKNOWN if timer was not found.
 */
am_Error_e SocketHandler::removeTimer(const sh_timerHandle_t handle)
{
	assert(handle!=0);

	//stop the current timer
	stopTimer(handle);

	std::list<timer_s>::iterator it=mListTimer.begin();
	for(;it!=mListTimer.end();++it)
	{
		if(it->handle==handle)
		{
			it=mListTimer.erase(it);
			return E_OK;
		}
	}
	return E_UNKNOWN;
}

/**
 * restarts a timer and updates with a new interval
 * @param handle handle to the timer
 * @param timeouts new timout time
 * @return E_OK on success, E_NON_EXISTENT if the handle was not found
 */
am_Error_e SocketHandler::restartTimer(const sh_timerHandle_t handle, const timespec timeouts)
{
	timer_s timerItem;
	std::list<timer_s>::iterator it=mListTimer.begin();
	for(;it!=mListTimer.end();++it)
	{
		if (it->handle==handle)
		{
			timerItem=*it;
			break;
		}
	}

	if (timeouts.tv_nsec!=-1 && timeouts.tv_sec!=-1)
	{
		timerItem.timeout=timeouts;
	}

	mListActiveTimer.push_back(timerItem);

	//very important: sort the list so that the smallest value is front
	mListActiveTimer.sort(compareCountdown);
	mTimeout=mListActiveTimer.front().countdown;
	return E_OK;
}

am_Error_e SocketHandler::stopTimer(const sh_timerHandle_t handle)
{
	//go through the list and remove the timer with the handle
	std::list<timer_s>::iterator it=mListActiveTimer.begin();
	for(;it!=mListActiveTimer.end();++it)
	{
		if(it->handle==handle)
		{
			it=mListActiveTimer.erase(it);
			if (!mListActiveTimer.empty())
			{
				mTimeout=mListActiveTimer.front().countdown;
			}
			else
			{
				mTimeout.tv_nsec=-1;
				mTimeout.tv_sec=-1;
			}
			return E_OK;
		}
	}
	return E_NON_EXISTENT;
}

/**
 * updates the eventFlags of a poll
 * @param fd the filedescriptor of the poll
 * @param event the event flags
 * @return E_OK on succsess, E_NON_EXISTENT if fd was not found
 */
am_Error_e SocketHandler::updateEventFlags(const sh_pollHandle_t handle, const int16_t  events)
{
	mListPoll_t::iterator iterator=mListPoll.begin();

	for (;iterator!=mListPoll.end();++iterator)
	{
		if(iterator->handle==handle)
		{
			iterator->pollfdValue.events=events;
			mRecreatePollfds=true;
			return E_OK;
		}
	}
	return E_UNKNOWN;
}

/**
 * checks if a filedescriptor is valid
 * @param fd the filedescriptor
 * @return true if the fd is valid
 */
bool SocketHandler::fdIsValid(const int fd) const
{
	return (fcntl(fd, F_GETFL) != -1 || errno != EBADF);
}

/**
 * whenever a timer is up, this function needs to be called.
 * Removes the fired timer, calls the callback and resets mTimeout
 */
void SocketHandler::timerUp()
{
	//first fire the event
	mListActiveTimer.front().callback->Call(mListActiveTimer.front().handle,mListActiveTimer.front().userData);

	//then remove the first timer, the one who fired
	mListActiveTimer.pop_front();
	if(!mListActiveTimer.empty())
	{
		//substract the old value from all timers in the list
		std::for_each(mListActiveTimer.begin(),mListActiveTimer.end(),SubstractTime(mTimeout));
		mTimeout=mListActiveTimer.front().countdown;
	}
	else
	{
		mTimeout.tv_nsec=-1;
		mTimeout.tv_sec=-1;
	}
}

/**
 * init the timers
 */
void SocketHandler::initTimer()
{
	if(!mListActiveTimer.empty())
	{
		mTimeout=mListActiveTimer.front().countdown;
	}
	else
	{
		mTimeout.tv_nsec=-1;
		mTimeout.tv_sec=-1;
	}
}


/**
* convert timespec to milliseconds
* @param time time in timespec
* @return time in milliseconds
*/
inline int SocketHandler::timespec2ms(const timespec & time)
{
	return (time.tv_nsec == -1 && time.tv_sec == -1) ? -1 : time.tv_sec * 1000 + time.tv_nsec / 1000000;
}

/**
* functor to easy substract from each countdown
* @param t value to substract from
*/
void SocketHandler::SubstractTime::operator ()(timer_s & t) const
{
	int val = 0;
	if((val = t.countdown.tv_nsec - param.tv_nsec) < 0){
		t.countdown.tv_nsec = 1000000000 + val;
		t.countdown.tv_sec--;
	}else{
		t.countdown.tv_nsec = val;
	}
	(t.countdown.tv_sec - param.tv_sec) < 0 ? 0 : (t.countdown.tv_sec -= param.tv_sec);
}

void SocketHandler::CopyPollfd::operator ()(const sh_poll_s & row)
{
	pollfd temp=row.pollfdValue;
	temp.revents=0;
	mArray.push_back(temp);
}

/* namespace am */
}

