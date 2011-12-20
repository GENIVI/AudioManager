/*
 * SocketHandler.cpp
 *
 *  Created on: Dec 18, 2011
 *      Author: christian
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

#include <iostream>  //todo remove

namespace am {

SocketHandler::SocketHandler()
	:mMapFdCallback(),
	 mListTimer(),
	 mListPollfd(),
	 mNextTimer(),
	 mLastInsertedHandle(1),
	 mDispatch(true)
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
	int pollStatus;

	//init the timer
	initTimer();

	while (mDispatch)
	{
		//block until something is on a filedescriptor
		if((pollStatus=poll(&mListPollfd.front(),mListPollfd.size(),timespec2ms(mTimeout)))==-1)
		{
			//todo enter DLT message here;
		}

		if (pollStatus!=0) //only check filedescriptors if there was a change
		{
			//todo: here could be a timer that makes sure naughty plugins return!

			//go through the list of fds and check if an event was set...
			mPollfd_t::iterator iterator=mListPollfd.begin();
			mPollfd_t::iterator iteratorEnd=mListPollfd.end();
			for(;iterator!=iteratorEnd;++iterator)
			{
				//oh yes! then, fire the callback !
				if(iterator->revents !=0)
				{
					//check the map for the right callback
					mMapFdCallback_t::iterator iteratorCB=mMapFdCallback.begin();
					iteratorCB=mMapFdCallback.find(iterator->fd);
					if(iteratorCB!=mMapFdCallback.end())
					{
						//fire!
						iteratorCB->second->Call(iterator->fd,iterator->events);
					}
				}
			}
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
am_Error_e SocketHandler::addFDPoll(const int fd, const short event, TBasicPollCallback*& callback)
{
	if (!fdIsValid(fd)) return E_NON_EXISTENT;

	pollfd tempPollfd;
	tempPollfd.fd=fd;
	tempPollfd.events=event;
	tempPollfd.revents=NULL;

	//insert the filedescriptor into the poll array
	mListPollfd.push_back(tempPollfd);

	//insert the callback into the map
	mMapFdCallback.insert(std::make_pair(fd,callback));
	return E_OK;
}

/**
 * removes a filedescriptor from the poll loop
 * @param fd the filedescriptor to be removed
 * @return E_OK in case of sucess, E_NON_EXISTENT or E_UNKNOWN if the fd in not registered
 */
am_Error_e SocketHandler::removeFDPoll(const int fd)
{
	mMapFdCallback_t::iterator iterator=mMapFdCallback.begin();

	//find the filedescriptor
	iterator=mMapFdCallback.find(fd);
	if (iterator==mMapFdCallback.end()) return E_NON_EXISTENT;

	//erase it
	mMapFdCallback.erase(iterator);

	//also remove from the callBackList
	mPollfd_t::iterator pollIterator=mListPollfd.begin();
	for(;pollIterator!=mListPollfd.end();++pollIterator)
	{
		if (pollIterator->fd==fd)
		{
			mListPollfd.erase(pollIterator);
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
am_Error_e SocketHandler::addTimer(const timespec timeouts,TBasicTimerCallback*& callback,timerHandle_t& handle)
{
	assert(!((timeouts.tv_sec==0) && (timeouts.tv_nsec==0)));
	assert(callback!=NULL);

	timer_s timerItem;

	//create a new handle for the timer
	handle=mLastInsertedHandle++;  //todo: overflow ruling !
	timerItem.handle=handle;
	timerItem.countdown=timeouts;
	timerItem.timeout=timeouts;
	timerItem.callback=callback;

	//add timer to the list
	mListTimer.push_back(timerItem);

	//very important: sort the list so that the smallest value is front
	mListTimer.sort(compareCountdown);
	mTimeout=mListTimer.front().countdown;
	return E_OK;
}

/**
 * removes a timer from the list of timers
 * @param handle the handle to the timer
 * @return E_OK in case of success, E_UNKNOWN if timer was not found.
 */
am_Error_e SocketHandler::removeTimer(const timerHandle_t handle)
{
	assert(handle!=0);

	//go through the list and remove the timer with the handle
	std::list<timer_s>::iterator it=mListTimer.begin();
	for(;it!=mListTimer.end();++it)
	{
		if(it->handle==handle)
		{
			it=mListTimer.erase(it);
			if (!mListTimer.empty())
			{
				mTimeout=mListTimer.front().countdown;
			}
			else
			{
				mTimeout.tv_nsec=-1;
				mTimeout.tv_sec=-1;
			}
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
	mListTimer.front().callback->Call(mListTimer.front().handle);

	//then remove the first timer, the one who fired
	mListTimer.pop_front();
	if(!mListTimer.empty())
	{
		//substract the old value from all timers in the list
		std::for_each(mListTimer.begin(),mListTimer.end(),SubstractTime(mTimeout));
		mTimeout=mListTimer.front().countdown;
	}
	else
	{
		mTimeout.tv_nsec=-1;
		mTimeout.tv_sec=-1;
	}
}

/**
 * inits the timers
 */
void SocketHandler::initTimer()
{
	if(!mListTimer.empty())
	{
		mTimeout=mListTimer.front().countdown;
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
inline int SocketHandler::timespec2ms(const timespec& time)
{
	return (time.tv_nsec==-1 && time.tv_sec==-1) ? -1 : time.tv_sec*1000 + time.tv_nsec/1000000;
}

/**
 * functor to easy substract from each countdown
 * @param t value to substract from
 */
void SocketHandler::SubstractTime::operator()(timer_s& t) const
{
	int val=0;
	if((val=t.countdown.tv_nsec-param.tv_nsec)<0)
	{
		t.countdown.tv_nsec=1000000000 + val;
		t.countdown.tv_sec--;
	}
	else
	{
		t.countdown.tv_nsec=val;
	}
	(t.countdown.tv_sec-param.tv_sec)<0 ? 0 : (t.countdown.tv_sec-=param.tv_sec);
}


} /* namespace am */
