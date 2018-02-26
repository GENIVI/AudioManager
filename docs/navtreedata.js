var NAVTREE =
[
  [ "AudioManager", "index.html", [
    [ "License", "lic.html", [
      [ "Open Source Projects in the source tree", "lic.html#proj", null ],
      [ "License Split", "lic.html#split", null ],
      [ "Mozilla Public License, v. 2.0", "lic.html#mpl", null ],
      [ "MIT license", "lic.html#mit", null ]
    ] ],
    [ "Dependencies", "dep.html", [
      [ "Dependency Graph", "dep.html#deps", null ],
      [ "Depedency Graph for Tests", "dep.html#deptest", null ],
      [ "Generated Dependency Graph", "dep.html#depgen", null ]
    ] ],
    [ "Versioning", "ver.html", [
      [ "Versioning Mechanism", "ver.html#mec_ver", null ],
      [ "New versioning scheme", "ver.html#new_ver", null ],
      [ "The versioning scheme until 7.0", "ver.html#ver_graph", null ]
    ] ],
    [ "Architecture Overview", "architecturepage.html", [
      [ "Audio Domains", "architecturepage.html#domains", null ],
      [ "Routing Adapter", "architecturepage.html#routing_adaptor", null ],
      [ "Gateway", "architecturepage.html#gateway", null ],
      [ "Converter", "architecturepage.html#converter", null ]
    ] ],
    [ "UML Model auf the AudioManager", "uml.html", [
      [ "Audio Manager Branch", "uml.html#svn", null ]
    ] ],
    [ "AudioManager Components", "audiomanagercomponentspage.html", [
      [ "AudioManagerDaemon", "audiomanagercomponentspage.html#audiomanagercomponents", [
        [ "Daemon Overview", "audiomanagercomponentspage.html#daemonover", null ]
      ] ],
      [ "AudioManagerCommandPlugin", "audiomanagercomponentspage.html#commander", [
        [ "Interfaces", "audiomanagercomponentspage.html#commandIface", null ]
      ] ],
      [ "AudioManagerController", "audiomanagercomponentspage.html#controller", [
        [ "Interfaces", "audiomanagercomponentspage.html#controlIface", null ]
      ] ],
      [ "Routing AudioManagerRoutingPlugin", "audiomanagercomponentspage.html#router", [
        [ "Interfaces", "audiomanagercomponentspage.html#routingIface", null ],
        [ "Bus topology", "audiomanagercomponentspage.html#subrouter", null ],
        [ "Busname", "audiomanagercomponentspage.html#busname", null ],
        [ "CommonAPI plugins", "audiomanagercomponentspage.html#CAPIplugins", null ]
      ] ]
    ] ],
    [ "AudioManager and CommonAPI", "comminAPI.html", null ],
    [ "Elements of the AudioManagement", "elementspage.html", [
      [ "Overview Class Diagram", "elementspage.html#cDiag", null ],
      [ "Sources", "elementspage.html#source", [
        [ "Attributes", "elementspage.html#sourceattributes", null ]
      ] ],
      [ "Sinks", "elementspage.html#sinks", [
        [ "Attributes", "elementspage.html#sinkattributes", null ]
      ] ],
      [ "Gateways", "elementspage.html#gw", [
        [ "Attributes", "elementspage.html#gwattributes", null ]
      ] ],
      [ "Crossfaders", "elementspage.html#crossfaders", [
        [ "Attributes", "elementspage.html#cfattributes", null ]
      ] ]
    ] ],
    [ "The relation of sources & sinks with the AudioManager", "sourcesink.html", [
      [ "Class diagramm of the relation between sources, sinks and the AudioManager", "sourcesink.html#claDi", null ],
      [ "The REAL interaction", "sourcesink.html#boil", null ],
      [ "Connection Formats", "sourcesink.html#conFormats", null ],
      [ "Source States", "sourcesink.html#sstates", null ],
      [ "Availability", "sourcesink.html#avail", null ],
      [ "Volumes", "sourcesink.html#vol", null ],
      [ "SoundProperties", "sourcesink.html#SoundProperties", null ],
      [ "Interrupt States", "sourcesink.html#in", null ]
    ] ],
    [ "About unique IDs : Static vs Dynamic IDs", "uniquepage.html", [
      [ "Why having two different kinds of ids?", "uniquepage.html#why", null ],
      [ "The setup", "uniquepage.html#setup", null ]
    ] ],
    [ "Classification of Sinks and Sources", "classficationpage.html", [
      [ "Classification", "classficationpage.html#classification", null ],
      [ "Attributes", "classficationpage.html#attributes", null ]
    ] ],
    [ "Interrups & Low Level Interrupts", "interrupts.html", [
      [ "Differences", "interrupts.html#diff", null ],
      [ "Criterias", "interrupts.html#crit", null ]
    ] ],
    [ "Connections & MainConnections", "connpage.html", [
      [ "Connections", "connpage.html#con", null ],
      [ "Mainconnections", "connpage.html#maincon", null ],
      [ "Attributes", "connpage.html#att", null ]
    ] ],
    [ "Lipsync", "lip.html", [
      [ "The Task of the Audiomanager", "lip.html#t", null ]
    ] ],
    [ "Early Audio", "early.html", [
      [ "The Requirement", "early.html#req", null ],
      [ "Early Startup", "early.html#earlys", null ],
      [ "Late Rundown", "early.html#late", null ]
    ] ],
    [ "The two views of the AudioManager", "views.html", [
      [ "The CommandInterface View View", "views.html#command", null ],
      [ "RoutingInterface View", "views.html#route", null ],
      [ "Overview", "views.html#over", null ]
    ] ],
    [ "Volumes & MainVolumes", "vol.html", [
      [ "MainVolumes", "vol.html#mainVol", null ],
      [ "Volumes", "vol.html#volv", null ]
    ] ],
    [ "Properties", "prop.html", [
      [ "SoundProperties & MainSoundProperties", "prop.html#soundprop", null ],
      [ "SystemProperties", "prop.html#sys", null ]
    ] ],
    [ "Notifications", "notifi.html", [
      [ "What are notifications?", "notifi.html#notifi_ex", null ],
      [ "Overview", "notifi.html#notifi_overview", null ],
      [ "CommandInterface", "notifi.html#notifi_command", null ],
      [ "ControlInterface", "notifi.html#notifi_control", null ],
      [ "RoutingInterface", "notifi.html#notifi_routing", null ],
      [ "Notification Levels", "notifi.html#notifi_levels", null ]
    ] ],
    [ "Miscellaneous", "misc.html", [
      [ "Connection Formats", "misc.html#misc_connfor", null ],
      [ "Persistence", "misc.html#misc_pers", null ],
      [ "Speed dependent volume", "misc.html#misc_speed", null ]
    ] ],
    [ "Last User Mode", "luc.html", [
      [ "Last User Mode concept", "luc.html#luc_concept", null ],
      [ "The handling in the rundown context:", "luc.html#luc_rundown", null ],
      [ "The next startup:", "luc.html#luc_startup", null ]
    ] ],
    [ "Mainloop concept", "mainl.html", [
      [ "Mainloop", "mainl.html#mconcept", null ],
      [ "Using the Mainloop", "mainl.html#sec", null ],
      [ "Utilizing The Mainloop as Threadsafe Call Method", "mainl.html#util", [
        [ "Asynchronous calls", "mainl.html#async", null ],
        [ "Synchronous calls", "mainl.html#sync", null ]
      ] ]
    ] ],
    [ "The watchdog", "watchd.html", [
      [ "The watchdog concept", "watchd.html#watchdconcept", null ],
      [ "Watchdog configuration", "watchd.html#configwatch", null ],
      [ "Integration with systemd", "watchd.html#winteg", null ]
    ] ],
    [ "Startup and Rundown", "start.html", [
      [ "Startup", "start.html#start_Start", null ],
      [ "Rundown", "start.html#start_Rundown", null ],
      [ "Cancelled Rundown", "start.html#start_Cancel", null ]
    ] ],
    [ "CommandLineParsing", "cmdline.html", [
      [ "TCLAP", "cmdline.html#tclap", null ],
      [ "CommandLine Parsing in the Plugins", "cmdline.html#cmdplugins", null ]
    ] ],
    [ "Dlt support", "dlt.html", [
      [ "Compilerswitch", "dlt.html#compile", null ]
    ] ],
    [ "Download Compile Debug", "eclip.html", [
      [ "Get the source", "eclip.html#dw", null ],
      [ "Compile", "eclip.html#build", null ],
      [ "Using Eclipse", "eclip.html#ec", null ],
      [ "Debugging with eclipse", "eclip.html#deb", null ]
    ] ],
    [ "Compiling & Co", "comp.html", null ],
    [ "Modules", "modules.html", "modules" ],
    [ "Namespaces", null, [
      [ "Namespace List", "namespaces.html", "namespaces" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", null ],
        [ "Functions", "namespacemembers_func.html", null ],
        [ "Variables", "namespacemembers_vars.html", null ],
        [ "Typedefs", "namespacemembers_type.html", null ],
        [ "Enumerations", "namespacemembers_enum.html", null ],
        [ "Enumerator", "namespacemembers_eval.html", null ]
      ] ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", "functions_func" ],
        [ "Variables", "functions_vars.html", "functions_vars" ],
        [ "Enumerations", "functions_enum.html", null ],
        [ "Enumerator", "functions_eval.html", null ],
        [ "Related Functions", "functions_rela.html", null ]
      ] ]
    ] ],
    [ "Files", null, [
      [ "File List", "files.html", "files" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Functions", "globals_func.html", null ],
        [ "Variables", "globals_vars.html", null ],
        [ "Enumerations", "globals_enum.html", null ],
        [ "Enumerator", "globals_eval.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
".html",
"audiomanagertypes_8h.html#a58a230b5da10699a7ce1b1f2a1c550e6",
"classam_1_1CAmControlReceiver.html#a9d6eae2312f5629f748ec293ef8ef118",
"classam_1_1CAmDbusWrapper.html#a817fcd0bce4f833cadd5767e7b36a007",
"classam_1_1CAmRoutingSender_1_1handleSinkVolume.html#a5020dcd4e51c4b30ce440c8099c2c49a",
"classam_1_1IAmControlSend.html#a7f54417c24637b91e0079187caecd3d3",
"classam_1_1IAmShPollCheck.html#a5851d4f160f9a9ab42965b67cee6fe97",
"mainl.html#mconcept",
"structam_1_1am__Source__s.html#a9904977c28ce558db9c8aec2172de7b7"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';