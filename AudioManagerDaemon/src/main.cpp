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

#ifdef  WITH_TELNET
    #include "CAmTelnetServer.h"
#endif

#ifdef WITH_CAPI_WRAPPER
    #include "CAmCommonAPIWrapper.h"
#else
	#ifdef WITH_DBUS_WRAPPER
		#include "CAmDbusWrapper.h"
	#endif
#endif

#ifdef WITH_NSM
	#ifdef WITH_DBUS_WRAPPER
		#include "CAmNodeStateCommunicatorDBus.h"
	#else
		#include "CAmNodeStateCommunicatorCAPI.h"
	#endif
#endif

#ifdef WITH_DATABASE_STORAGE
    #include "CAmDatabaseHandlerSQLite.h"
#else
    #include "CAmDatabaseHandlerMap.h"
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
#include "CAmDatabaseObserver.h"
#include "CAmDltWrapper.h"
#include "CAmSocketHandler.h"
#include "CAmCommandLineSingleton.h"


using namespace am;
DLT_DECLARE_CONTEXT(AudioManager)

//we need these because we parse them beforehand.
std::vector<std::string> listCommandPluginDirs;
std::vector<std::string> listRoutingPluginDirs;

//commandline options used by the Audiomanager itself
TCLAP::ValueArg<std::string> controllerPlugin("c","controllerPlugin","use controllerPlugin full path with .so ending",false,CONTROLLER_PLUGIN,"string");
TCLAP::ValueArg<std::string> additionalCommandPluginDirs("L","additionalCommandPluginDirs","additional path for looking for command plugins, can be used after -l option",false," ","string");
TCLAP::ValueArg<std::string> additionalRoutingPluginDirs("R","additionalRoutingPluginDirs","additional path for looking for routing plugins, can be used after -r option ",false," ","string");
TCLAP::ValueArg<std::string> routingPluginDir("r","RoutingPluginDir","path for looking for routing plugins",false," ","string");
TCLAP::ValueArg<std::string> commandPluginDir("l","CommandPluginDir","path for looking for command plugins",false," ","string");
TCLAP::ValueArg<std::string> databasePath ("p","databasePath","path for sqlite database (default is in memory)",false,":memory:","string");
TCLAP::ValueArg<unsigned int> telnetPort ("t","telnetport","The port that is used for telnet",false,DEFAULT_TELNETPORT,"int");
TCLAP::ValueArg<unsigned int> maxConnections ("m","maxConnections","Maximal number of connections for telnet",false,MAX_TELNETCONNECTIONS,"int");
TCLAP::SwitchArg dbusWrapperTypeBool ("t","dbusType","DbusType to be used by CAmDbusWrapper: if option is selected, DBUS_SYSTEM is used otherwise DBUS_SESSION",false);
TCLAP::SwitchArg enableNoDLTDebug ("V","logDlt","print DLT logs to stdout",false);
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
    //todo: add gracefull dead here. Do what can be done persistence wise
    exit(1);
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
#ifdef WITH_TELNET
	printf("\tTelnet portNumber:\t\t\t%i\n", telnetPort.getValue());
	printf("\tTelnet maxConnections:\t\t\t%i\n", maxConnections.getValue());
#endif
#ifdef WITH_DATABASE_STORAGE
	printf("\tSqlite Database path:\t\t\t%s\n", databasePath.getValue().c_str());
#endif
#ifndef WITH_DLT
	printf("\tDlt Command Line Output: \t\t%s\n", enableNoDLTDebug.getValue()?"enabled":"not enabled");
#endif
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
#ifndef WITH_DLT
    	cmd->add(enableNoDLTDebug);
#endif
#ifdef WITH_DBUS_WRAPPER
    	cmd->add(dbusWrapperTypeBool);
#endif
#ifdef WITH_TELNET
    	cmd->add(telnetPort);
    	cmd->add(maxConnections);
#endif
#ifdef WITH_DATABASE_STORAGE
    	cmd->add(databasePath);
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

    CAmDltWrapper::instance(enableNoDLTDebug.getValue())->registerApp("AudioManagerDeamon", "AudioManagerDeamon");
    CAmDltWrapper::instance()->registerContext(AudioManager, "Main", "Main Context");

    //Instantiate all classes. Keep in same order !
    CAmSocketHandler iSocketHandler;

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

    CAmRoutingSender iRoutingSender(listRoutingPluginDirs);
    CAmCommandSender iCommandSender(listCommandPluginDirs);
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
    CAmCommonAPIWrapper *pCAPIWrapper = CAmCommonAPIWrapper::instantiateOnce(&iSocketHandler);
    CAmCommonAPIWrapper iDBusWrapper = *pCAPIWrapper;
#ifdef WITH_NSM
    CAmNodeStateCommunicatorCAPI iNodeStateCommunicator(&iDBusWrapper);
#endif /*WITH_NSM*/
#endif /*WITH_CAPI_WRAPPER */

#ifdef WITH_DBUS_WRAPPER
    if (dbusWrapperTypeBool.getValue())
    	dbusWrapperType=DBUS_BUS_SYSTEM;
    CAmDbusWrapper iDBusWrapper(&iSocketHandler,dbusWrapperType);
#ifdef WITH_NSM
    CAmNodeStateCommunicatorDBus iNodeStateCommunicator(&iDBusWrapper);
#endif /*WITH_NSM*/
#endif /*WITH_DBUS_WRAPPER */

#ifdef WITH_SYSTEMD_WATCHDOG
    CAmWatchdog iWatchdog(&iSocketHandler);
#endif /*WITH_SYSTEMD_WATCHDOG*/

#ifdef WITH_DATABASE_STORAGE
    CAmDatabaseHandlerSQLite iDatabaseHandler(databasePath.getValue());
#else
    CAmDatabaseHandlerMap iDatabaseHandler;
#endif /*WITH_DATABASE_STORAGE*/
    IAmDatabaseHandler *pDatabaseHandler = dynamic_cast<IAmDatabaseHandler*>( &iDatabaseHandler );
    CAmRouter iRouter(pDatabaseHandler, &iControlSender);

#ifdef WITH_DBUS_WRAPPER
    CAmCommandReceiver iCommandReceiver(pDatabaseHandler, &iControlSender, &iSocketHandler, &iDBusWrapper);
    CAmRoutingReceiver iRoutingReceiver(pDatabaseHandler, &iRoutingSender, &iControlSender, &iSocketHandler, &iDBusWrapper);
#else /*WITH_DBUS_WRAPPER*/
	    CAmCommandReceiver iCommandReceiver(pDatabaseHandler,&iControlSender,&iSocketHandler);
	    CAmRoutingReceiver iRoutingReceiver(pDatabaseHandler,&iRoutingSender,&iControlSender,&iSocketHandler);
#endif /*WITH_DBUS_WRAPPER*/

#ifdef WITH_NSM
	CAmControlReceiver iControlReceiver(pDatabaseHandler,&iRoutingSender,&iCommandSender,&iSocketHandler, &iRouter, &iNodeStateCommunicator);
	iNodeStateCommunicator.registerControlSender(&iControlSender);
#else /*WITH_NSM*/
	CAmControlReceiver iControlReceiver(pDatabaseHandler,&iRoutingSender,&iCommandSender,&iSocketHandler, &iRouter);
#endif /*WITH_NSM*/

#ifdef WITH_TELNET
    CAmTelnetServer iTelnetServer(&iSocketHandler, &iCommandSender, &iCommandReceiver, &iRoutingSender, &iRoutingReceiver, &iControlSender, &iControlReceiver, pDatabaseHandler, &iRouter, telnetPort.getValue(), maxConnections.getValue());
    CAmDatabaseObserver iObserver(&iCommandSender, &iRoutingSender, &iSocketHandler, &iTelnetServer);
#else /*WITH_TELNET*/
    CAmDatabaseObserver iObserver(&iCommandSender,&iRoutingSender, &iSocketHandler);
#endif

    iDatabaseHandler.registerObserver(&iObserver);

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

    //now the signal handler:
    struct sigaction signalAction;
    memset(&signalAction, '\0', sizeof(signalAction));
    signalAction.sa_sigaction = &signalHandler;
    signalAction.sa_flags = SA_SIGINFO;
    sigaction(SIGINT, &signalAction, NULL);
    sigaction(SIGQUIT, &signalAction, NULL);
    sigaction(SIGTERM, &signalAction, NULL);
    sigaction(SIGHUP, &signalAction, NULL);

    struct sigaction signalChildAction;
    memset(&signalChildAction, '\0', sizeof(signalChildAction));
    signalChildAction.sa_flags = SA_NOCLDWAIT;
    sigaction(SIGCHLD, &signalChildAction, NULL);

    //register new out of memory handler
    std::set_new_handler(&OutOfMemoryHandler);

    try
    {
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

    //deinit the DLT
    CAmDltWrapper* inst(getWrapper());
    inst->deinit();

    exit(0);

}

