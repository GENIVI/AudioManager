/*
 * SocketHandler.h
 *
 *  Created on: Dec 18, 2011
 *      Author: christian
 */

#ifndef SOCKETHANDLER_H_
#define SOCKETHANDLER_H_

#include <audiomanagertypes.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <map>
#include <stdint.h>
#include <sys/poll.h>
#include <list>

namespace am {

class TBasicPollCallback;
class TBasicTimerCallback;

class SocketHandler
{
public:
	typedef uint16_t timerHandle_t;   //!<this is a handle for a timer to be used with the SocketHandler
	SocketHandler();
	virtual ~SocketHandler();

	am_Error_e addFDPoll(const int fd,const short event,TBasicPollCallback*& callback);
	am_Error_e removeFDPoll(const int fd);
	am_Error_e addTimer(const timespec timeouts,TBasicTimerCallback*& callback,timerHandle_t& handle);
	am_Error_e removeTimer(const timerHandle_t handle);
	void start_listenting();
	void stop_listening();

private:
	struct timer_s						//!<struct that holds information of timers
	{
		timerHandle_t handle;			//!<the handle of the timer
		timespec countdown;				//!<the countdown, this value is decreased every time the timer is up
		timespec timeout;				//!<the original timer value
		TBasicTimerCallback* callback;	//!<the callbackfunction
	};

	class SubstractTime					//!<functor to easy substract from each countdown value
	{
	private:
		timespec param;
	public:
		SubstractTime(timespec param): param(param) {}
		void operator()(timer_s& t) const;
	};

	typedef std::vector<pollfd> mPollfd_t;						//!<vector of filedescriptors
	typedef std::map<int,TBasicPollCallback*> mMapFdCallback_t;	//!<map for the callbacks

	bool fdIsValid(const int fd) const;
	void initTimer();
	void timerUp();
	int timespec2ms(const timespec& time);
	static bool compareCountdown(const timer_s& a, const timer_s& b)
	{
		return (a.countdown.tv_sec==b.countdown.tv_sec) ? (a.countdown.tv_nsec < b.countdown.tv_nsec) : (a.countdown.tv_sec < b.countdown.tv_sec);
	}


	mMapFdCallback_t mMapFdCallback;
	std::list<timer_s> mListTimer;
	mPollfd_t mListPollfd;
	timerHandle_t mNextTimer;
	timerHandle_t mLastInsertedHandle;
	timespec mTimeout;
	bool mDispatch;
};

/**
 * classic functor for the BasiCallCallback
 */
class TBasicPollCallback
{
public:
	virtual void Call (int fd, const short revent)=0;
	virtual ~TBasicPollCallback(){};
};

/**
 * classic functor for the BasicTimerCallback
 */
class TBasicTimerCallback
{
public:
	virtual void Call (SocketHandler::timerHandle_t handle)=0;
	virtual ~TBasicTimerCallback(){};
};


/**
 * template to create the functor for a class
 */
template <class TClass> class TSpecificPollCallback : public TBasicPollCallback
{
private:
    TClass* mInstance;
    void (TClass::*mFunction)(int fd, const short revent);

public:
    TSpecificPollCallback(TClass* instance, void(TClass::*function)(int fd, const short revent))
    :mInstance(instance), mFunction(function){};

    virtual void Call(int fd, const short revent)
    {
    	(*mInstance.*mFunction)(fd,revent);
    };
};

/**
 * template to create the functor for a class
 */
template <class TClass> class TSpecificTimerCallback : public TBasicTimerCallback
{
private:
    TClass* mInstance;
    void (TClass::*mFunction)(SocketHandler::timerHandle_t handle);

public:
    TSpecificTimerCallback(TClass* instance, void(TClass::*function)(SocketHandler::timerHandle_t handle))
    :mInstance(instance), mFunction(function){};

    virtual void Call(SocketHandler::timerHandle_t handle)
    {
    	(*mInstance.*mFunction)(handle);
    }
};
} /* namespace am */
#endif /* SOCKETHANDLER_H_ */
