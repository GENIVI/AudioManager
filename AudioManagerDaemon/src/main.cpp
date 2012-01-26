/**
 * Copyright (C) 2011, BMW AG
 *
 * GeniviAudioMananger AudioManagerDaemon
 *
 * \file main.cpp
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

/**
 * Please make sure to have read the documentation on genivi.org!
 */

//todo: make real daemon out of it- systemd conform
//todo: versioning of PluginInterfaces on linux level (.symver stuff)
//todo: all communication like all plugins loaded etc...
//todo: seperate documentation of test from normal project
//todo: check the startup sequence. Dbus shall be activated last...
//todo: there is a bug in the visible flags of sinks and sources. fix it.
//todo: make sure that iterators have a fixed end to prevent crashed while adding vectors while iterating on critical vectors
//todo: make sure all configurations are tested
//todo: clean up startup sequences controller, command and routing interfaces----
#include <config.h>
#include <SocketHandler.h>
#ifdef WITH_DBUS_WRAPPER
#include <dbus/DBusWrapper.h>
#endif
#include "DatabaseHandler.h"
#include "ControlSender.h"
#include "CommandSender.h"
#include "RoutingSender.h"
#include "RoutingReceiver.h"
#include "CommandReceiver.h"
#include "ControlReceiver.h"
#include "DatabaseObserver.h"
#include "TelnetServer.h"
#include "Router.h"
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <cstdlib>
#include <assert.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>

#include <dlt/dlt.h>

DLT_DECLARE_CONTEXT(AudioManager)

using namespace am;

const char* USAGE_DESCRIPTION = "Usage:\tAudioManagerDaemon [options]\n"
        "options:\t\n"
        "\t-h: print this message\t\n"
        "\t-i: info about current settings \t\n"
        "\t-v: print version\t\n"
        "\t-d: daemonize AudioManager \t\n"
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

void daemonize()
{
    umask(0);
    std::string dir = "/";

    rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
    {
        DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("can't get file limit "));
    }

    pid_t pid;
    if ((pid = fork()) < 0)
    {
        DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("cannot fork!"));
    }
    else if (pid != 0)
    {
        exit(0);
    }

    setsid();

    if (!dir.empty() && chdir(dir.c_str()) < 0)
    {
        DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("couldn't chdir to the new directory"));
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
        DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("new standard file descriptors were not opened"));
    }
}

void parseCommandLine(int argc, char **argv)
{
    while (optind < argc)
    {
        int option = getopt(argc, argv, "h::v::c::l::r::L::R::d::t::m::i::p::");

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
        case 'h':
        default:
            printf("AudioManagerDaemon Version: %s\n", DAEMONVERSION);
            puts(USAGE_DESCRIPTION);
            exit(-1);
        }
    }
}

static void signalHandler(int sig, siginfo_t *siginfo, void *context)
{
    (void) sig;
    (void) siginfo;
    (void) context;
    DLT_LOG(AudioManager, DLT_LOG_ERROR, DLT_STRING("signal handler was called, exit now..."));
    gDispatchDone = 1;
    //todo: maually fire the mainloop
    //todo: ifdef no sockethandler
    exit(1);
}

int main(int argc, char *argv[])
{
    DLT_REGISTER_APP("AudioManagerDeamon", "AudioManagerDeamon");
    DLT_REGISTER_CONTEXT(AudioManager, "Main", "Main Context");
    DLT_LOG(AudioManager, DLT_LOG_INFO, DLT_STRING("The AudioManager is started, "), DLT_STRING(DAEMONVERSION));

    listCommandPluginDirs.push_back(std::string(DEFAULT_PLUGIN_COMMAND_DIR));
    listRoutingPluginDirs.push_back(std::string(DEFAULT_PLUGIN_ROUTING_DIR));

    //parse the commandline options
    parseCommandLine(argc, (char**) argv);

    //now the signal handler:
    struct sigaction signalAction;
    memset(&signalAction, '\0', sizeof(signalAction));
    signalAction.sa_sigaction = &signalHandler;
    signalAction.sa_flags = SA_SIGINFO;
    sigaction(SIGINT, &signalAction, NULL);
    sigaction(SIGQUIT, &signalAction, NULL);
    sigaction(SIGTERM, &signalAction, NULL);
    sigaction(SIGHUP, &signalAction, NULL);
    sigaction(SIGQUIT, &signalAction, NULL);

    struct sigaction signalChildAction;
    memset(&signalChildAction, '\0', sizeof(signalChildAction));
    signalChildAction.sa_flags = SA_NOCLDWAIT;
    sigaction(SIGCHLD, &signalChildAction, NULL);

    //Instantiate all classes. Keep in same order !
#ifdef WITH_SOCKETHANDLER_LOOP
    SocketHandler iSocketHandler;
#endif

#ifdef WITH_DBUS_WRAPPER
#ifdef WITH_SOCKETHANDLER_LOOP
    DBusWrapper iDBusWrapper(&iSocketHandler);
#else /*WITH_SOCKETHANDLER_LOOP*/
    DBusWrapper iDBusWrapper;
#endif /*WITH_SOCKETHANDLER_LOOP*/
#endif /*WITH_DBUS_WRAPPER */

    DatabaseHandler iDatabaseHandler(databasePath);
    RoutingSender iRoutingSender(listRoutingPluginDirs);
    CommandSender iCommandSender(listCommandPluginDirs);
    ControlSender iControlSender(controllerPlugin);
    Router iRouter(&iDatabaseHandler, &iControlSender);

#ifdef WITH_DBUS_WRAPPER
#ifdef WITH_SOCKETHANDLER_LOOP
    CommandReceiver iCommandReceiver(&iDatabaseHandler, &iControlSender, &iSocketHandler, &iDBusWrapper);
    RoutingReceiver iRoutingReceiver(&iDatabaseHandler, &iRoutingSender, &iControlSender, &iSocketHandler, &iDBusWrapper);
    ControlReceiver iControlReceiver(&iDatabaseHandler, &iRoutingSender, &iCommandSender, &iSocketHandler, &iRouter);
#ifdef WITH_TELNET
    TelnetServer iTelnetServer(&iSocketHandler,&iCommandSender,&iCommandReceiver,&iRoutingSender,&iRoutingReceiver,&iControlSender,&iControlReceiver,&iDatabaseHandler,telnetport,maxConnections);
#endif
#else /*WITH_SOCKETHANDLER_LOOP */
    CommandReceiver iCommandReceiver(&iDatabaseHandler,&iControlSender,&iDBusWrapper);
    RoutingReceiver iRoutingReceiver(&iDatabaseHandler,&iRoutingSender,&iControlSender,&iDBusWrapper);
    ControlReceiver iControlReceiver(&iDatabaseHandler,&iRoutingSender,&iCommandSender, &iRouter);
#endif /*WITH_SOCKETHANDLER_LOOP*/
#else /*WITH_DBUS_WRAPPER*/
    CommandReceiver iCommandReceiver(&iDatabaseHandler,&iControlSender,&iSocketHandler);
    RoutingReceiver iRoutingReceiver(&iDatabaseHandler,&iRoutingSender,&iControlSender,&iSocketHandler);
    ControlReceiver iControlReceiver(&iDatabaseHandler,&iRoutingSender,&iCommandSender,&iSocketHandler, &iRouter);
#ifdef WITH_TELNET
    TelnetServer iTelnetServer(&iSocketHandler,telnetport,maxConnections);
#endif
#endif /*WITH_DBUS_WRAPPER*/

#ifdef WITH_TELNET
    DatabaseObserver iObserver(&iCommandSender, &iRoutingSender,&iTelnetServer);
#else
    DatabaseObserver iObserver(&iCommandSender, &iRoutingSender);
#endif

    //since the plugins have been loaded by the *Senders before, we can tell the Controller this:
    iControlSender.hookAllPluginsLoaded();

    //the controller should startup the interfaces - this is just for testing
    iCommandSender.startupInterface(&iCommandReceiver);
    iRoutingSender.startupRoutingInterface(&iRoutingReceiver);
    iRoutingSender.routingInterfacesReady();

#ifdef WITH_SOCKETHANDLER_LOOP
    iSocketHandler.start_listenting();
#endif /*WITH_SOCKETHANDLER_LOOP*/

#ifdef WITH_DBUS_WRAPPER
#ifdef WITH_SIMPLEDBUS_LOOP
    iDBusWrapper.dbusMainLoop();
#endif/*WITH_SIMPLEDBUS_LOOP*/
#endif /*WITH_DBUS_WRAPPER*/

    close(fd0);
    close(fd1);
    close(fd2);
    exit(0);

}

