#ifndef _CONFIG_H
#define _CONFIG_H

#define DAEMONVERSION "ver-0.0.1-8-g126870e"

#define WITH_DBUS_WRAPPER
#define WITH_SOCKETHANDLER_LOOP 
/* #undef WITH_SIMPLEDBUS_LOOP */
#define WITH_PPOLL
/* #undef WITH_TELNET */

#define DEFAULT_PLUGIN_COMMAND_DIR "/usr/local/lib/audioManager/command"
#define DEFAULT_PLUGIN_ROUTING_DIR "/usr/local/lib/audioManager/routing"
#define CONTROLLER_PLUGIN "/usr/local/lib/audioManager/control/libPluginControlInterface.so"

#define DEFAULT_TELNETPORT 6060
#define MAX_TELNETCONNECTIONS 3

#define DBUS_SERVICE_PREFIX "org.genivi.audiomanager"
#define DBUS_SERVICE_OBJECT_PATH "/org/genivi/audiomanager"

#define INTROSPECTION_COMMAND_XML_FILE "/home/christian/workspace/gitserver/includes/dbus/CommandInterface.xml"

#endif /* _CONFIG_H */
