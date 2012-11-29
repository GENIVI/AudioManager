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
 * \author Frank Herchet, frank.fh.herchet@bmw.de BMW 2012
 *
 * \file main.cpp
 * For further information see http://www.genivi.org/.
 *
 */

/**
 * \todo create systemd compatibility
 * \todo all communication like all plugins loaded etc...
 * \todo check the startup sequence. Dbus shall be activated last...
 * \bug package generation only works if package directory exists...
 */

#include "config.h"

#ifdef  WITH_TELNET
#include "CAmTelnetServer.h"
#endif
#ifdef WITH_DBUS_WRAPPER
#include <shared/CAmDbusWrapper.h>
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
#include "CAmDatabaseHandler.h"
#include "CAmControlSender.h"
#include "CAmCommandSender.h"
#include "CAmRoutingSender.h"
#include "CAmRoutingReceiver.h"
#include "CAmCommandReceiver.h"
#include "CAmControlReceiver.h"
#include "CAmDatabaseObserver.h"
#include "CAmWatchdog.h"
#include "shared/CAmDltWrapper.h"
#include "shared/CAmSocketHandler.h"


using namespace am;
DLT_DECLARE_CONTEXT(AudioManager)

const char* USAGE_DESCRIPTION = "Usage:\tAudioManagerDaemon [options]\n"
        "options:\t\n"
        "\t-h: print this message\t\n"
        "\t-i: info about current settings \t\n"
        "\t-v: print version\t\n"
#ifndef WITH_DLT
        "\t-V: print DLT logs to stdout\t\n"
#endif
        "\t-d: daemonize AudioManager \t\n"
#ifdef WITH_DBUS_WRAPPER
        "\t-T: DbusType to be used by CAmDbusWrapper (0=DBUS_SESSION[default], 1=DBUS_SYSTEM)\t\n"
#endif
        "\t-p<path> path for sqlite database (default is in memory)\t\n"
        "\t-t<port> port for telnetconnection\t\n"
        "\t-m<max> number of max telnetconnections\t\n"
        "\t-c<Name> use controllerPlugin <Name> (full path with .so ending)\t\n"
        "\t-l<Name> replace command plugin directory with <Name> (full path)\t\n"
        "\t-r<Name> replace routing plugin directory with <Name> (full path)\t\n"
        "\t-L<Name> add command plugin directory with <Name> (full path)\t\n"
        "\t-R<Name> add routing plugin directory with <Name> (full path)\t\n";

std::string controllerPlugin = std::string(CONTROLLER_PLUGIN);
std::vector<std::string> listCommandPluginDirs;
std::vector<std::string> listRoutingPluginDirs;
std::string databasePath = std::string(":memory:");
unsigned int telnetport = DEFAULT_TELNETPORT;
unsigned int maxConnections = MAX_TELNETCONNECTIONS;
int fd0, fd1, fd2;
bool enableNoDLTDebug = false;

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

/**
 * parses the command line
 * @param argc
 * @param argv
 */
void parseCommandLine(int argc, char **argv)
{
    while (optind < argc)
    {
#ifdef WITH_DLT
    #ifdef WITH_DBUS_WRAPPER
            int option = getopt(argc, argv, "h::v::c::l::r::L::R::d::t::m::i::p::T::");
    #else
            int option = getopt(argc, argv, "h::v::c::l::r::L::R::d::t::m::i::p::");
    #endif //WITH_DBUS_WRAPPER
#else
    #ifdef WITH_DBUS_WRAPPER
            int option = getopt(argc, argv, "h::v::V::c::l::r::L::R::d::t::m::i::p::T::");
    #else
            int option = getopt(argc, argv, "h::v::V::c::l::r::L::R::d::t::m::i::p::");
    #endif //WITH_DBUS_WRAPPER
#endif

        switch (option)
        {
        case 'i':
            printf("Current settings:\n");
            printf("\tAudioManagerDaemon Version:\t\t%s\n", DAEMONVERSION);
            printf("\tTelnet portNumber:\t\t\t%i\n", telnetport);
            printf("\tTelnet maxConnections:\t\t\t%i\n", maxConnections);
            printf("\tSqlite Database path:\t\t\t%s\n", databasePath.c_str());
            printf("\tControllerPlugin: \t\t\t%s\n", controllerPlugin.c_str());
            printf("\tDirectory of CommandPlugins: \t\t%s\n", listCommandPluginDirs.front().c_str());
            printf("\tDirectory of RoutingPlugins: \t\t%s\n", listRoutingPluginDirs.front().c_str());
            exit(0);
            break;
        case 't':
            assert(atoi(optarg)!=0);
            telnetport = atoi(optarg);
            break;
        case 'm':
            assert(atoi(optarg)!=0);
            maxConnections = atoi(optarg);
            break;
        case 'p':
            assert(!controllerPlugin.empty());
            databasePath = std::string(optarg);
            break;
        case 'd':
            daemonize();
            break;
        case 'l':
            listCommandPluginDirs.clear();
            listCommandPluginDirs.push_back(std::string(optarg));
            break;
        case 'r':
            listRoutingPluginDirs.clear();
            listRoutingPluginDirs.push_back(std::string(optarg));
            break;
        case 'L':
            listCommandPluginDirs.push_back(std::string(optarg));
            break;
        case 'R':
            listRoutingPluginDirs.push_back(std::string(optarg));
            break;
        case 'c':
            controllerPlugin = std::string(optarg);
            assert(!controllerPlugin.empty());
            assert(controllerPlugin.find(".so")!=std::string::npos);
            break;
        case 'v':
            printf("AudioManagerDaemon Version: %s\n", DAEMONVERSION);
            exit(-1);
            break;
#ifndef WITH_DLT
            case 'V':
            printf("\e[0;34m[DLT]\e[0;30m\tDebug output to stdout enabled\n");
            enableNoDLTDebug = true;
            break;
#endif
#ifdef WITH_DBUS_WRAPPER
            case 'T':
            dbusWrapperType=static_cast<DBusBusType>(atoi(optarg));
            break;
#endif
        case 'h':
        default:
            printf("AudioManagerDaemon Version: %s\n", DAEMONVERSION);
            puts(USAGE_DESCRIPTION);
            exit(-1);
        }
    }
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
    //todo: maually fire the mainloop
    CAmControlSender::CallsetControllerRundown();

    //deinit the DLT
    CAmDltWrapper* inst(getWrapper());
    inst->deinit();

    CAmSocketHandler::static_exit_mainloop();

}

void mainProgram()
{
    //Instantiate all classes. Keep in same order !
    CAmSocketHandler iSocketHandler;

#ifdef WITH_DBUS_WRAPPER
    CAmDbusWrapper iDBusWrapper(&iSocketHandler,dbusWrapperType);
#endif /*WITH_DBUS_WRAPPER */

#ifdef WITH_SYSTEMD_WATCHDOG
    CAmWatchdog iWatchdog(&iSocketHandler);
#endif /*WITH_SYSTEMD_WATCHDOG*/

    CAmDatabaseHandler iDatabaseHandler(databasePath);
    CAmRoutingSender iRoutingSender(listRoutingPluginDirs);
    CAmCommandSender iCommandSender(listCommandPluginDirs);
    CAmControlSender iControlSender(controllerPlugin);
    CAmRouter iRouter(&iDatabaseHandler, &iControlSender);

#ifdef WITH_DBUS_WRAPPER
    CAmCommandReceiver iCommandReceiver(&iDatabaseHandler, &iControlSender, &iSocketHandler, &iDBusWrapper);
    CAmRoutingReceiver iRoutingReceiver(&iDatabaseHandler, &iRoutingSender, &iControlSender, &iSocketHandler, &iDBusWrapper);
    CAmControlReceiver iControlReceiver(&iDatabaseHandler, &iRoutingSender, &iCommandSender, &iSocketHandler, &iRouter);
#else /*WITH_DBUS_WRAPPER*/
    CAmCommandReceiver iCommandReceiver(&iDatabaseHandler,&iControlSender,&iSocketHandler);
    CAmRoutingReceiver iRoutingReceiver(&iDatabaseHandler,&iRoutingSender,&iControlSender,&iSocketHandler);
    CAmControlReceiver iControlReceiver(&iDatabaseHandler,&iRoutingSender,&iCommandSender,&iSocketHandler, &iRouter);
#endif /*WITH_DBUS_WRAPPER*/

#ifdef WITH_TELNET
    CAmTelnetServer iTelnetServer(&iSocketHandler, &iCommandSender, &iCommandReceiver, &iRoutingSender, &iRoutingReceiver, &iControlSender, &iControlReceiver, &iDatabaseHandler, &iRouter, telnetport, maxConnections);
    CAmDatabaseObserver iObserver(&iCommandSender, &iRoutingSender, &iSocketHandler, &iTelnetServer);
#else /*WITH_TELNET*/
    CAmDatabaseObserver iObserver(&iCommandSender,&iRoutingSender, &iSocketHandler);
#endif

    iDatabaseHandler.registerObserver(&iObserver);

    //startup all the Plugins and Interfaces
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

    //parse the commandline options
    parseCommandLine(argc, (char**) argv);

    CAmDltWrapper::instance(enableNoDLTDebug)->registerApp("AudioManagerDeamon", "AudioManagerDeamon");
    CAmDltWrapper::instance()->registerContext(AudioManager, "Main", "Main Context");
    logInfo("The Audiomanager is started");
    logInfo("The version of the Audiomanager", DAEMONVERSION);

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
        mainProgram();
    }

    catch (std::exception& exc)
    {
        logError("The AudioManager ended by throwing the exception", exc.what());
        //todo: ergency exit here... call destructors etc...
        exit(EXIT_FAILURE);
    }

    close(fd0);
    close(fd1);
    close(fd2);
    exit(0);

}

