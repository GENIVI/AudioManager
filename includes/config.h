#ifndef _CONFIG_H
#define _CONFIG_H

#define WITH_DBUS_WRAPPER
#define WITH_SOCKETHANDLER_LOOP 
/* #undef WITH_SIMPLEDBUS_LOOP */

#define DEFAULT_PLUGIN_COMMAND_DIR "/home/christian/workspace/gitserver/bin/plugins/command"
#define DEFAULT_PLUGIN_ROUTING_DIR "/home/christian/workspace/gitserver/bin/plugins/routing"
#define CONTROLLER_PLUGIN "/home/christian/workspace/gitserver/bin/plugins/control/libPluginControlInterface.so"

#define DBUS_SERVICE_PREFIX "org.genivi.audiomanager"
#define DBUS_SERVICE_OBJECT_PATH "/org/genivi/audiomanager"

#define INTROSPECTION_COMMAND_XML_FILE "/home/christian/workspace/gitserver/includes/dbus/CommandInterface.xml"

#endif /* _CONFIG_H */
