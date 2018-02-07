/**
 * SPDX license identifier: MPL-2.0
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
 * \author Frank Herchet, frank.fh.herchet@bmw.de BMW 2012
 *
 * \file main.cpp
 * For further information see http://www.genivi.org/.
 *
 */

#include "audiomanagerconfig.h"

#ifdef WITH_CAPI_WRAPPER
    #include "CAmCommonAPIWrapper.h"
#endif

#ifdef WITH_DBUS_WRAPPER
	#include "CAmDbusWrapper.h"
#endif

#ifdef WITH_SYSTEMD_WATCHDOG
    #include "CAmWatchdog.h"
#endif

#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdlib>
#include <cassert>
#include <fcntl.h>
#include <csignal>
#include <cstring>
#include <cstdio>
#include <new>

#include "CAmRouter.h"
#include "CAmControlSender.h"
#include "CAmCommandSender.h"
#include "CAmRoutingSender.h"
#include "CAmRoutingReceiver.h"
#include "CAmCommandReceiver.h"
#include "CAmControlReceiver.h"
#include "CAmDltWrapper.h"
#include "CAmSocketHandler.h"
#include "CAmCommandLineSingleton.h"
#include "CAmDatabaseHandlerMap.h"

#ifndef AUDIOMANGER_APP_ID
	#define AUDIOMANGER_APP_ID "AUDI"
#endif

#ifndef AUDIOMANGER_APP_DESCRIPTION
	#define AUDIOMANGER_APP_DESCRIPTION "AudioManager"
#endif


using namespace am;

//we need these because we parse them beforehand.
std::vector<std::string> listCommandPluginDirs;
std::vector<std::string> listRoutingPluginDirs;

// List of signals to be handled with signalfd
std::vector<uint8_t> listOfSignalsFD = {SIGHUP, SIGTERM, SIGCHLD};

//commandline options used by the Audiomanager itself
TCLAP::ValueArg<std::string> controllerPlugin("c","controllerPlugin","use controllerPlugin full path with .so ending",false,CONTROLLER_PLUGIN_DIR,"string");
TCLAP::ValueArg<std::string> additionalCommandPluginDirs("L","additionalCommandPluginDirs","additional path for looking for command plugins, can be used after -l option",false," ","string");
TCLAP::ValueArg<std::string> additionalRoutingPluginDirs("R","additionalRoutingPluginDirs","additional path for looking for routing plugins, can be used after -r option ",false," ","string");
TCLAP::ValueArg<std::string> routingPluginDir("r","RoutingPluginDir","path for looking for routing plugins",false," ","string");
TCLAP::ValueArg<std::string> commandPluginDir("l","CommandPluginDir","path for looking for command plugins",false," ","string");
TCLAP::ValueArg<std::string> dltLogFilename("F","dltLogFilename","the name of the logfile, absolute path. Only if logging is et to file",false," ","string");
TCLAP::ValueArg<unsigned int> dltOutput ("O","dltOutput","defines where logs are written. 0=dlt-daemon(default), 1=command line, 2=file ",false,0,"int");
TCLAP::SwitchArg dltEnable ("e","dltEnable","Enables or disables dlt logging. Default = enabled",true);
TCLAP::SwitchArg dbusWrapperTypeBool ("T","dbusType","DbusType to be used by CAmDbusWrapper: if option is selected, DBUS_SYSTEM is used otherwise DBUS_SESSION",false);
TCLAP::SwitchArg currentSettings("i","currentSettings","print current settings and exit",false);
TCLAP::SwitchArg daemonizeAM("d","daemonize","daemonize Audiomanager. Better use systemd...",false);

int fd0, fd1, fd2;

#ifdef WITH_DBUS_WRAPPER
    DBusBusType dbusWrapperType=DBUS_BUS_SESSION;
#endif

/**
 * the out of memory handler
 */
void OutOfMemoryHandler()
{
    logError("No more memory - bye");
    throw std::runtime_error(std::string("SocketHandler::start_listenting ppoll returned with error."));
}

/**
 * daemonizes the AudioManager
 */
void daemonize()
{
    umask(0);
    std::string dir = "/";

    rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
    {
        logError("can't get file limit ");
    }

    pid_t pid;
    if ((pid = fork()) < 0)
    {
        logError("cannot fork!");
    }
    else if (pid != 0)
    {
        exit(0);
    }

    setsid();

    if (!dir.empty() && chdir(dir.c_str()) < 0)
    {
        logError("couldn't chdir to the new directory");
    }

    if (rl.rlim_max == RLIM_INFINITY)
    {
        rl.rlim_max = 1024;
    }

    for (unsigned int i = 0; i < rl.rlim_max; i++)
    {
        close(i);
    }

    fd0 = open("/dev/null", O_RDONLY);
    fd1 = open("/dev/null", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    fd2 = open("/dev/null", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);

    if (fd0 != STDIN_FILENO || fd1 != STDOUT_FILENO || fd2 != STDERR_FILENO)
    {
        logError("new standard file descriptors were not opened");
    }
}



void printCmdInformation()
{
	printf("\n\n\nCurrent settings:\n\n");
	printf("\tAudioManagerDaemon Version:\t\t%s\n", DAEMONVERSION);
	printf("\tControllerPlugin: \t\t\t%s\n", controllerPlugin.getValue().c_str());
	printf("\tDirectories of CommandPlugins: \t\t\n");
    std::vector<std::string>::const_iterator dirIter = listCommandPluginDirs.begin();
    std::vector<std::string>::const_iterator dirIterEnd = listCommandPluginDirs.end();
    for (; dirIter < dirIterEnd; ++dirIter)
    {
    	printf("\t                              \t\t%s\n", dirIter->c_str());
    }

	printf("\tDirectories of RoutingPlugins: \t\t\n");
    dirIter = listRoutingPluginDirs.begin();
    dirIterEnd = listRoutingPluginDirs.end();
    for (; dirIter < dirIterEnd; ++dirIter)
    {
    	printf("\t                              \t\t%s\n", dirIter->c_str());
    }
	exit(0);
}

/**
 * the signal handler
 * @param sig
 * @param siginfo
 * @param context
 */
static void signalHandler(int sig, siginfo_t *siginfo, void *context)
{
    (void) sig;
    (void) siginfo;
    (void) context;
    logInfo("signal handler was called, signal",sig);

    switch (sig)
    {
        /*ctl +c lets call direct controllerRundown, because we might be blocked at the moment.
        But there is the risk of interrupting something important */
        case SIGINT:
            CAmControlSender::CallsetControllerRundown(sig);
            break;

        /* huch- we are getting killed. Better take the fast but risky way: */
        case SIGQUIT:
            CAmControlSender::CallsetControllerRundown(sig);
            break;
        default:
            break;
    }
}


void mainProgram(int argc, char *argv[])
{

	//initialize the commandline parser, and add all neccessary commands
    try
    {
    	TCLAP::CmdLine* cmd(CAmCommandLineSingleton::instanciateOnce("The team of the AudioManager wishes you a nice day!",' ',DAEMONVERSION,true));
    	cmd->add(controllerPlugin);
    	cmd->add(additionalCommandPluginDirs);
    	cmd->add(commandPluginDir);
    	cmd->add(additionalRoutingPluginDirs);
    	cmd->add(routingPluginDir);
    	cmd->add(currentSettings);
    	cmd->add(daemonizeAM);
    	cmd->add(dltEnable);
    	cmd->add(dltLogFilename);
    	cmd->add(dltOutput);
#ifdef WITH_DBUS_WRAPPER
    	cmd->add(dbusWrapperTypeBool);
#endif
    }
    catch (TCLAP::ArgException &e)  // catch any exceptions
    { std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; }

    //hen and egg. We need to parse a part of the commandline options to get the paths of the controller and the plugins.
    //So we do some little parsing first and the real parsing later so that the plugins can profit from that.
    CAmCommandLineSingleton::instance()->preparse(argc,argv);
	if (daemonizeAM.getValue())
	{
		daemonize();
	}

    CAmDltWrapper::instanctiateOnce(AUDIOMANGER_APP_ID, AUDIOMANGER_APP_DESCRIPTION ,dltEnable.getValue(),static_cast<am::CAmDltWrapper::logDestination>(dltOutput.getValue()),dltLogFilename.getValue());

    //Instantiate all classes. Keep in same order !
    CAmSocketHandler iSocketHandler;
    if(iSocketHandler.fatalErrorOccurred())
    {
        throw std::runtime_error(std::string("CAmSocketHandler: Could not create pipe or file descriptor is invalid."));
    }

    if(E_OK != iSocketHandler.listenToSignals(listOfSignalsFD))
    {
      logWarning("CAmSocketHandler failed to register itself as signal handler.");
    }
    //Register signal handler
    sh_pollHandle_t signalHandler;
    iSocketHandler.addSignalHandler([&](const sh_pollHandle_t handle, const signalfd_siginfo & info, void* userData){

        unsigned sig = info.ssi_signo;
        unsigned user = info.ssi_uid;

        logInfo("signal handler was called from user", user, "with signal ",sig);

        switch (sig)
        {
            /* more friendly here assuming systemd wants to stop us, so we can use the mainloop */
            case SIGTERM:
                CAmControlSender::CallsetControllerRundownSafe(sig);
                break;

            /* looks friendly, too, so lets take the long run */
            case SIGHUP:
                CAmControlSender::CallsetControllerRundownSafe(sig);
                break;
            default:
                break;
        }
    },signalHandler,NULL);

    if(commandPluginDir.isSet())
    {
    	listCommandPluginDirs.clear();
    	listCommandPluginDirs.push_back(commandPluginDir.getValue());
    }

    if (additionalCommandPluginDirs.isSet())
    {
    	listCommandPluginDirs.push_back(additionalCommandPluginDirs.getValue());
    }

    if(routingPluginDir.isSet())
    {
    	listRoutingPluginDirs.clear();
    	listRoutingPluginDirs.push_back(routingPluginDir.getValue());
    }

    if (additionalRoutingPluginDirs.isSet())
    {
    	listRoutingPluginDirs.push_back(additionalRoutingPluginDirs.getValue());
    }

    //in this place, the plugins can get the gloval commandlineparser via CAmCommandLineSingleton::instance() and add their options to the commandline
    //this must be done in the constructor.
    //later when the plugins are started, the commandline is already parsed and the objects defined before can be used to get the neccesary information
    
    CAmDatabaseHandlerMap iDatabaseHandler;
    IAmDatabaseHandler *pDatabaseHandler = dynamic_cast<IAmDatabaseHandler*>( &iDatabaseHandler );

    CAmRoutingSender iRoutingSender(listRoutingPluginDirs,pDatabaseHandler);
    CAmCommandSender iCommandSender(listCommandPluginDirs, &iSocketHandler);
    CAmControlSender iControlSender(controllerPlugin.getValue(),&iSocketHandler);

    try
    {
    	//parse the commandline options
    	CAmCommandLineSingleton::instance()->reset();
    	CAmCommandLineSingleton::instance()->parse(argc,argv);
    	if (currentSettings.getValue())
    	{
    		printCmdInformation();
    	}
    }
    catch (TCLAP::ArgException &e)  // catch any exceptions
    { std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl; }

    logInfo("The Audiomanager is started");
    logInfo("The version of the Audiomanager", DAEMONVERSION);

#ifdef WITH_CAPI_WRAPPER
    //We instantiate a singleton with the current socket handler, which loads the common-api runtime.
    CAmCommonAPIWrapper *pCAPIWrapper = CAmCommonAPIWrapper::instantiateOnce(&iSocketHandler, "AudioManager");
#endif /*WITH_CAPI_WRAPPER */

#ifdef WITH_DBUS_WRAPPER
    if (dbusWrapperTypeBool.getValue())
    	dbusWrapperType=DBUS_BUS_SYSTEM;
    CAmDbusWrapper iDBusWrapper(&iSocketHandler,dbusWrapperType);
#endif /*WITH_DBUS_WRAPPER */

#ifdef WITH_SYSTEMD_WATCHDOG
    CAmWatchdog iWatchdog(&iSocketHandler);
#endif /*WITH_SYSTEMD_WATCHDOG*/

CAmRouter iRouter(pDatabaseHandler, &iControlSender);

#ifdef WITH_DBUS_WRAPPER
    CAmCommandReceiver iCommandReceiver(pDatabaseHandler, &iControlSender, &iSocketHandler, &iDBusWrapper);
    CAmRoutingReceiver iRoutingReceiver(pDatabaseHandler, &iRoutingSender, &iControlSender, &iSocketHandler, &iDBusWrapper);
#else /*WITH_DBUS_WRAPPER*/
	    CAmCommandReceiver iCommandReceiver(pDatabaseHandler,&iControlSender,&iSocketHandler);
	    CAmRoutingReceiver iRoutingReceiver(pDatabaseHandler,&iRoutingSender,&iControlSender,&iSocketHandler);
#endif /*WITH_DBUS_WRAPPER*/

CAmControlReceiver iControlReceiver(pDatabaseHandler,&iRoutingSender,&iCommandSender,&iSocketHandler, &iRouter);

iDatabaseHandler.registerObserver(&iRoutingSender);
iDatabaseHandler.registerObserver(&iCommandSender);
iDatabaseHandler.registerObserver(&iRouter);
//startup all the Plugins and Interfaces
//at this point, commandline arguments can be parsed
iControlSender.startupController(&iControlReceiver);
iCommandSender.startupInterfaces(&iCommandReceiver);
iRoutingSender.startupInterfaces(&iRoutingReceiver);

//when the routingInterface is done, all plugins are loaded:
iControlSender.setControllerReady();

#ifdef WITH_SYSTEMD_WATCHDOG
    iWatchdog.startWatchdog();
#endif /*WITH_SYSTEMD_WATCHDOG*/

    //start the mainloop here....
    iSocketHandler.start_listenting();
}

/**
 * main
 * @param argc
 * @param argv
 * @return
 */
int main(int argc, char *argv[], char** envp)
{
    (void) envp;
    listCommandPluginDirs.push_back(std::string(DEFAULT_PLUGIN_COMMAND_DIR));
    listRoutingPluginDirs.push_back(std::string(DEFAULT_PLUGIN_ROUTING_DIR));

    //critical signals are registered here:
    struct sigaction signalAction;
    memset(&signalAction, '\0', sizeof(signalAction));
    signalAction.sa_sigaction = &signalHandler;
    signalAction.sa_flags = SA_SIGINFO;
    sigaction(SIGINT, &signalAction, NULL);
    sigaction(SIGQUIT, &signalAction, NULL);
    
    //register new out of memory handler
    std::set_new_handler(&OutOfMemoryHandler);

    sigset_t signal_mask;
    sigemptyset(&signal_mask);
    for (auto it : listOfSignalsFD)
    {
        sigaddset(&signal_mask, it);
    }

    try
    {
        if (pthread_sigmask(SIG_BLOCK, &signal_mask, NULL) != 0)
        {
            throw std::runtime_error(std::string("Couldn't set mask for potential future threads"));
        }

        //we do this to catch all exceptions and have a graceful ending just in case
        mainProgram(argc,argv);
    }

    catch (std::exception& exc)
    {
        logError("The AudioManager ended by throwing the exception", exc.what());
        std::cerr<<"The AudioManager ended by throwing an exception "<<exc.what()<<std::endl;
        exit(EXIT_FAILURE);
    }

    close(fd0);
    close(fd1);
    close(fd2);
    exit(0);

}

