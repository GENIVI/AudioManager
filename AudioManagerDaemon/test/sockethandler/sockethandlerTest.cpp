/*
 * sockethandlerTest.cpp
 *
 *  Created on: Dec 19, 2011
 *      Author: christian
 */

#include "sockethandlerTest.h"
#include <stdio.h>
#include <sys/socket.h> /* for socket(), connect(), (), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <sys/ioctl.h>
#include <string.h>
#include <netdb.h>


using namespace testing;
using namespace am;

sockethandlerTest::sockethandlerTest()
{
}

sockethandlerTest::~sockethandlerTest()
{
}

void fdCallBack::connectSocket(int fd, const short  events,void * userData)
{
	std::cout<<"Socket connection received and open"<<std::endl;

	//accept the connection
	mSocketConnection = accept(fd, NULL, NULL);
	short event = 0;
	event |=POLLIN;

	shPollCallBack* buf=&pSocketDataCallback;
	//add new socketConnection to the handler
	mSocketHandler->addFDPoll(mSocketConnection,event,buf,NULL);
}




fdCallBack::fdCallBack(SocketHandler *SocketHandler)
:mSocketConnection(0),
 mSocketHandler(SocketHandler),
 pSocketDataCallback(this, &fdCallBack::handleSocketData),
 pSocketConnectionCallback(this, &fdCallBack::connectSocket)
{
}



void am::fdCallBack::handleSocketData(int fd, const short  events, void* userdata)
{
	char buffer[3000];
	std::string msg;

	//there is something for us, read it
	int read=recv(mSocketConnection,buffer,sizeof(buffer),NULL);
	msg=std::string(buffer,read);
	if (msg.compare("stopit")==0)
	{
		mSocketHandler->stop_listening();
	}
	else if (msg.compare("answer")==0)
	{
		std::string answer="myAnswer";
		send(mSocketConnection,answer.c_str(),answer.size(),NULL);
	}
}

fdCallBack::~fdCallBack()
{
}

am::timerCallBack::timerCallBack(SocketHandler *myHandler)
 :pTimer1Callback(this, &timerCallBack::timer1Callback),
  pTimer2Callback(this, &timerCallBack::timer2Callback),
  pTimer3Callback(this, &timerCallBack::timer3Callback),
  pTimer4Callback(this, &timerCallBack::timer4Callback),
  mSocketHandler(myHandler)

{
}



am::timerCallBack::~timerCallBack()
{
}



void am::timerCallBack::timer1Callback(SocketHandler::sh_timerHandle_t handle, void* userData)
{
	std::cout<<"callback1 called"<<std::endl;
	timespec timeout;
	timeout.tv_nsec=0;
	timeout.tv_sec=1;
	TBasicTimerCallback *buf=&pTimer1Callback;
	SocketHandler::sh_timerHandle_t handle_;
	mSocketHandler->addTimer(timeout,buf,handle_,NULL);
}



void am::timerCallBack::timer2Callback(SocketHandler::sh_timerHandle_t handle, void* userData)
{
	std::cout<<"callback2 called"<<std::endl;
	timespec timeout;
	timeout.tv_nsec=0;
	timeout.tv_sec=1;
	TBasicTimerCallback *buf=&pTimer2Callback;
	SocketHandler::sh_timerHandle_t handle_;
	mSocketHandler->addTimer(timeout,buf,handle_,NULL);
}

void am::timerCallBack::timer3Callback(SocketHandler::sh_timerHandle_t handle, void* userData)
{
	std::cout<<"callback3 called"<<std::endl;
}

void am::timerCallBack::timer4Callback(SocketHandler::sh_timerHandle_t handle, void* userData)
{
	std::cout<<"callback4 called"<<std::endl;
}

void* playWithSocketServer(void* data)
{
	int yes = 1;

	//get a SocketHandler
	SocketHandler myHandler;

	//get a class that handles the callbacks from the handler
	fdCallBack testCallback(&myHandler);

	//prepare the socket, bind etc...
	struct sockaddr_in servAddr;
	int socketHandle = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	setsockopt(socketHandle, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
	servAddr.sin_family      = AF_INET;
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servAddr.sin_port = htons(6060);
	bind(socketHandle, (struct sockaddr*)(&servAddr), sizeof (servAddr));
	listen(socketHandle, 3);

	//prepare the event (we want POLLIN because we need to listen)
	short event = 0;
	event |=POLLIN;

	shPollCallBack* buf=&testCallback.pSocketConnectionCallback;
	//add the callback to the Sockethandler
	myHandler.addFDPoll(socketHandle, event, buf, NULL);

	//start the mainloop
	myHandler.start_listenting();
	close(socketHandle);
}



TEST(sockethandlerTest,playWithSockets)
{
	pthread_t serverThread;
	char buffer[3000];

	//creates a thread that handles the serverpart
	pthread_create(&serverThread,NULL,playWithSocketServer,NULL);

	sleep(1); //give a little time to settle everything
	//make everything ready to send data
	struct sockaddr_in servAddr;
	int socketHandle = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct hostent *host;
	host = (struct hostent*) gethostbyname("localhost");

	memset(&servAddr, 0, sizeof(servAddr));
	memcpy(&servAddr.sin_addr, host->h_addr_list[0], host->h_length);
	servAddr.sin_family      = AF_INET;
	servAddr.sin_port = htons(6060);

	//connect to the socket
	int k =connect(socketHandle,(struct sockaddr *) &servAddr, sizeof(servAddr));

	std::string msg="answer";

	//send first the answer message and wait for the reply
	int p=send(socketHandle,msg.c_str(),msg.size(),NULL);
	int read=recv(socketHandle,buffer,sizeof(buffer),NULL);
	msg=std::string(buffer,read);
	ASSERT_TRUE(msg.compare("myAnswer")==0);

	msg="stopit";
	//now send a message causing the handler to stop and end the loop
	p=send(socketHandle,msg.c_str(),msg.size(),NULL);
	pthread_join(serverThread,NULL);
}

TEST(sockethandlerTest,playWithTimers)
{
	SocketHandler myHandler;
	timerCallBack testCallback(&myHandler);
	timespec timeoutTime, timeout2, timeout3, timeout4;
	timeoutTime.tv_sec=3;
	timeoutTime.tv_nsec=0;
	timeout2.tv_nsec=0;
	timeout2.tv_sec=1;
	timeout3.tv_nsec=000000000;
	timeout3.tv_sec=2;
	timeout4.tv_nsec=0;
	timeout4.tv_sec=30;
	TBasicTimerCallback* buf=&testCallback.pTimer1Callback;
	TBasicTimerCallback* buf2=&testCallback.pTimer2Callback;
	TBasicTimerCallback* buf3=&testCallback.pTimer3Callback;
	TBasicTimerCallback* buf4=&testCallback.pTimer4Callback;
	SocketHandler::sh_timerHandle_t handle;
	myHandler.addTimer(timeoutTime,buf,handle,NULL);
	myHandler.addTimer(timeout2,buf2,handle,NULL);
	myHandler.addTimer(timeout3,buf3,handle,NULL);
	myHandler.addTimer(timeout4,buf4,handle,NULL);
	myHandler.start_listenting();

}

void sockethandlerTest::SetUp()
{
}

void sockethandlerTest::TearDown()
{
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}




