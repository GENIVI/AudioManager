GENIVI_AudioManager
===================
:Author: Christian Linke <christian.linke@bmw.de>
:doctitle: GENIVI_AudioManager
 
Copyright
---------
Copyright (C) 2012, GENIVI Alliance, Inc.
Copyright (C) 2012, BMW AG

This file is part of GENIVI Project AudioManager.
 
Contributions are licensed to the GENIVI Alliance under one or more
Contribution License Agreements or MPL 2.0.
 
This Source Code Form is subject to the terms of the
Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 
Author Christian Linke = Christian Linke christian.linke@bmw.de BMW 2011-2015
For further information see https://at.projects.genivi.org/wiki/display/PROJ/Audio+Manager

== Repositories
The main repository is 
https://github.com/GENIVI/AudioManager

The repository for example plugins can be found here: 
https://github.com/GENIVI/AudioManagerPlugins

== License
The licenses of this project are split into two parts:

1) the AudioManagerDaemon, licensed under MPL 2.0

2) the plugins that serve as example code that can be taken to build up an own project with it -
    these parts are licensed with the MIT license
    
Contribution is done under MPL2.0 or MIT License, depending on the location of the code. 

== Version
The current version can be taken out of the git. The version 1.0.0 is the first GENIVI compliant code, in the compliance statement of Discovery (2.0). 
For every GENIVI release, there is a release of the AudioManager, each of the releases have their own bug fix branch that will get the most important fixes.
Development is done on the master branch.

=== Versioning Scheme
The versioning scheme was decided in the February face2face 2012.
The first number describes the release branch. This is 1 for Discovery, 2 for Excalibur and 3 for Foton. For major features or release, the second number will be increased.
Each new GENIVI version (releases are every half year, around April and October) will get the current HEAD (for example 2.0.34 for Excalibur) and release with the increase of the second number (2.1).
The next commit on the master branch gets then the new first number (3) and starts from zero.

For the daemon the third number (for example 1.0.X) describes the patch version. The versions are automatically created by git during the build process.
The versioning scheme is used for the AudioManager daemon itself and for each of it's interfaces. The versioning of the Interfaces in EA is defined via the tag "version" and the name of the interfaceversion versionName, for example "CommandReceiveVersion". This information is generated into the interface header files and is used then by cmake to set the interface versions.
Whenever changes are done, the minor version of the interface needs to be incremented.

Since 7.0, the AudioManager follows a new versioning scheme. Please check the documentation for that.

== Documentation
A very detailed documentation is provided by doxygen. In order to use this, please compile the AudioManager with
----
cmake -DWITH_DOCUMENTATION=ON
make

The README is compiled into README.html with asciidoc
----

== COMPILE PROGRAMS

=== Compile Options
These are the compile options with default values:
----
AudioManager Configuration:
-- CMAKE_BUILD_TYPE              = 
-- CMAKE_INSTALL_PREFIX          = /usr/local
-- BUILD_DOCUMENTATION           = OFF
-- WITH_TESTS                    = ON
-- WITH_DLT                      = ON
-- WITH_TESTS                    = ON
-- WITH_TELNET                   = ON
-- WITH_SYSTEMD_WATCHDOG         = OFF
-- WITH_CAPI_WRAPPER             = ON
-- WITH_DBUS_WRAPPER             = OFF
-- WITH_SHARED_UTILITIES         = OFF
-- WITH_SHARED_CORE              = OFF
-- DYNAMIC_ID_BOUNDARY           = 100
-- LIB_INSTALL_SUFFIX            = audiomanager
-- TEST_EXECUTABLE_INSTALL_PATH  = ~/tests
-- DEFAULT_PLUGIN_COMMAND_DIR    = /usr/local/lib/audiomanager/command
-- DEFAULT_PLUGIN_ROUTING_DIR    = /usr/local/lib/audiomanager/routing
-- CONTROLLER_PLUGIN_DIR         = /usr/local/lib/audiomanager/control
-- AM_SHARE_FOLDER               = /usr/local/share/audiomanager
-- AM_MAP_CAPACITY               = 10
-- AM_MAX_CONNECTIONS            = 0x1000
-- AM_MAX_MAIN_CONNECTIONS       = 0x1000
-- BUILD_TESTING                 = ON
-- CommandInterface version: 4.0
-- ControlInterface version: 5.0
-- RoutingInterface version: 5.0
----  
=== Passing options to cmake:

Standard CMake can be used to configure these options. Tools like ccmake can be used to visually change the values.
For each option, some hints are given.

=== Build dependencies
Basically, all build dependencies are optional- but you might need to use some if you want to have support for Dbus,
for example...

You will need optionally fulfill some dependencies in order to compile the GENIVI AudioManager Daemon, these are:

* dbus (only when WITH_DBUS_WRAPPER==ON) [tested on version 1.2.16]
* automotive-dlt [greater 2.5.0] (only when WITH_DLT==ON)        
* doxygen [tested on version 1.6.3] (only when WITH_DOCUMENTATION==ON) 
* commonAPI [version > 3.1.5] (only with WITH_CAPI_WRAPPER), more information here http://projects.genivi.org/commonapi/
* systemd [ version > 44 ] (only WITH_SYSTEMD_WATCHDOG)

=== AudioManagerUtilities

In the AudioManagerUtilites you can find helper functions that can be reused by other projects as well.
The library can be shipped as a static or a dynamic link library (WITH_SHARED_UTILITIES).

=== AudioManagerCore

The AudioMangerCore is build as a static (or with WITH_SHARED_CORE) library. Sometimes it is useful for unit testing of a plugin to compile against the core.

=== CommonAPI Wrapper

The commonapi wrapper provides the mainloop intergration for commonapi into the Mainloop of the audiomanager (CAmSockethandler).
In order to use it, just use:

----
CAPI->registerService(....)
CAPI->buildProxy(...)
----

instead of the standard calls. The CAPIWrapper will serialize the commands and integrate it smoothly with the mainloop.

=== Tests

For building the tests, you will need the following packages:

* python [tested on version 2.6, should work on higher versions as well]

GoogleMock and GoogleTest are as source code integrated in the source tree
To install them in a build environment like Ubuntu you can use:
----
sudo apt-get install python2.6-dev
----

For compiling, you will need a compiler, linker etc. On most Linux systems you will get this via
----
sudo apt-get install build-essential
----

More details in the CMake Files CmakeList.txt in the projects.

=== Compiling
To compile open a shell, browse to the AudioManager folder and 
----
mkdir /build
cd build
cmake ..
make
----

The AudioManager executable will be placed in the bin folder of your build folder, tests in a sub folder below.-

In order to install the AudioManager, you can do
----
sudo make install
----

this installs everything.

== Compiling plugins

Once the Audiomanager is installed, it will also install *.pc files for autotools and *Config.cmake files for cmake projects.
In order to compile and link against the AudioMananger, you can use:

----
find_package(AudioManager REQUIRED )  
find_package(AudioManagerUtilities)
----
to find the configuration files. To use the right include paths, use:
----
${AudioManager_INCLUDE_DIRS}
${AudioManagerUtilities_INCLUDE_DIRS}
----
to link agains the right libs use:
----
${AudioManagerUtilities_LIBRARIES}
----

for example, see the AudiomanagerPlugins

=== Adding own plugins
To keep the own sources away from the GENIVI code the project specific elements can be reconfigured with own type definitions.
You can copy paste the CMake scripts from the example plugins for example.

.The are already examples given in audiomanagertypes.h:
----
/**
 * This type gives the information about reason for Source/Sink change
 */
typedef uint16_t am_CustomAvailabilityReason_t;
static const am_CustomAvailabilityReason_t AR_UNKNOWN = 0;
/** new media was entered  */
static const am_CustomAvailabilityReason_t AR_GENIVI_NEWMEDIA = 1;
/** same media was entered */
static const am_CustomAvailabilityReason_t AR_GENIVI_SAMEMEDIA = 2;
/** there is no media or media is removed */
static const am_CustomAvailabilityReason_t AR_GENIVI_NOMEDIA = 3;
/** there was a temperature event */
static const am_CustomAvailabilityReason_t AR_GENIVI_TEMPERATURE = 4;
/** there was a voltage event */
static const am_CustomAvailabilityReason_t AR_GENIVI_VOLTAGE = 5;
/** fatal errors on reading or accessing media */
static const am_CustomAvailabilityReason_t AR_GENIVI_ERRORMEDIA = 6;

/**
 * This is a custom specific identifier of property. It can be used to
 * differentiate between interrupt source/sink, main source/sink, etc.
 */
typedef uint16_t am_CustomClassProperty_t;
static const am_CustomClassProperty_t CP_UNKNOWN = 0;
static const am_CustomClassProperty_t CP_GENIVI_SOURCE_TYPE = 1;
static const am_CustomClassProperty_t CP_GENIVI_SINK_TYPE = 2;

/**
 * This type classifies the format in which data is exchanged within a connection.
 * The type itself is project specific although there are some standard formats
 * defined.
 */
typedef uint16_t am_CustomConnectionFormat_t;
static const am_CustomConnectionFormat_t CF_UNKNOWN = 0;
/** plain mono */
static const am_CustomConnectionFormat_t CF_GENIVI_MONO = 1;
/** stereo connection */
static const am_CustomConnectionFormat_t CF_GENIVI_STEREO = 2;
/** analog connection */
static const am_CustomConnectionFormat_t CF_GENIVI_ANALOG = 3;
/** automatic connection.  */
static const am_CustomConnectionFormat_t CF_GENIVI_AUTO = 4;

/**
 * Here are all SoundProperties that can be set via the CommandInterface.
 * This type is product specific and can be changed or extended.
 */
typedef uint16_t am_CustomMainSoundPropertyType_t;
static const am_CustomMainSoundPropertyType_t MSP_UNKNOWN = 0;
/** example value between -10 and +10  */
static const am_CustomMainSoundPropertyType_t MSP_GENIVI_TREBLE = 1;
/** example value between -10 and +10  */
static const am_CustomMainSoundPropertyType_t MSP_GENIVI_MID = 2;
/** example value between -10 and +10  */
static const am_CustomMainSoundPropertyType_t MSP_GENIVI_BASS = 3;

/**
 * The notification types are project specific.
 */
typedef uint16_t am_CustomNotificationType_t;
static const am_CustomNotificationType_t NT_UNKNOWN = 0;

/**
 * The given ramp types here are just examples. For products, different ramp types
 * can be defined here. It is in the responsibility of the product to make sure
 * that the routing plugins are aware of the ramp types used.
 */
typedef uint16_t am_CustomRampType_t;
static const am_CustomRampType_t RAMP_UNKNOWN = 0;
/** sets directly the value without a ramp */
static const am_CustomRampType_t RAMP_GENIVI_DIRECT = 1;
/** Sets the volume as fast as possible */
static const am_CustomRampType_t RAMP_GENIVI_NO_PLOP = 2;
static const am_CustomRampType_t RAMP_GENIVI_EXP_INV = 3;
static const am_CustomRampType_t RAMP_GENIVI_LINEAR = 4;
static const am_CustomRampType_t RAMP_GENIVI_EXP = 5;

/**
 * Within GENIVI only the some example properties are defined.
 * For products these should be changed or extended.
 */
typedef uint16_t am_CustomSoundPropertyType_t;
static const am_CustomSoundPropertyType_t SP_UNKNOWN = 0;
/** example treble value min =-10 max =10 */
static const am_CustomSoundPropertyType_t SP_GENIVI_TREBLE = 1;
/** example mid value min =-10 max =10 */
static const am_CustomSoundPropertyType_t SP_GENIVI_MID = 2;
/** example bass value min =-10 max =10 */
static const am_CustomSoundPropertyType_t SP_GENIVI_BASS = 3;

/**
 * Describes the different system properties which are project specific.
 */
typedef uint16_t am_CustomSystemPropertyType_t;
static const am_CustomSystemPropertyType_t SYP_UNKNOWN = 0;
----

=== CommandLine options
The commandline options of the AudioManager:

----
USAGE: 

   ./AudioManager  [-K <string>] [-m <int>] [-t <int>] [-i] [-r <string>]
                   [-R <string>] [-l <string>] [-L <string>] [-c <string>]
                   [--] [--version] [-h]


Where: 

   -K <string>,  --controllerPluginArg <string>
     a test argument for the controller

   -m <int>,  --maxConnections <int>
     Maximal number of connections for telnet

   -t <int>,  --telnetport <int>
     The port that is used for telnet

   -i,  --currentSettings
     print current settings and exit

   -r <string>,  --RoutingPluginDir <string>
     path for looking for routing plugins

   -R <string>,  --additionalRoutingPluginDirs <string>
     additional path for looking for routing plugins, can be used after -r
     option 

   -l <string>,  --CommandPluginDir <string>
     path for looking for command plugins

   -L <string>,  --additionalCommandPluginDirs <string>
     additional path for looking for command plugins, can be used after -l
     option

   -c <string>,  --controllerPlugin <string>
     use controllerPlugin full path with .so ending

   --,  --ignore_rest
     Ignores the rest of the labeled arguments following this flag.

   --version
     Displays version information and exits.

   -h,  --help
     Displays usage information and exits.


   The team of the AudioManager wishes you a nice day!
----  	
To learn more about the commandline options, check the doxygen documentation.


== Code Formatting
The source code if formatted with eclipse, the style sheet used can be found in the Foo folder:
----
cmake/AudioManager_Codestyle.xml
----

== Working on the code & contribution

.First get the code from the git:
        git clone 

.Get an overview of all branches:
        git branch

.Switch to the branch you want to work on (see versioning scheme, the master is the feature branch) and verify that it has switched (* changed)
        git checkout <your branch>
        git branch

.Best practice is to create a local branch based on the current branch:
        git branch working_branch

Start working, best practice is to commit smaller, buildable peices during the work that makes it easier to 
handle later on.

.If you want to commit you changes, send them to the audiomanager-dev list, you can create a patch like this:
        git format-patch working_branch <your branch>

This creates a set of patches that are published via the mailing list.The patches will be discussed and then merged & uploaded on the git by the maintainer.

Patches can be accepted either under GENIVI Cla or MPL 2.0 (see section License). Please be sure that the signed-off-by is set correctly. For more, check out http://gerrit.googlecode.com/svn/documentation/2.0/user-signedoffby.html



----
					    _             _ _       __  __                                   
					   / \  _   _  __| (_) ___ |  \/  | __ _ _ __   __ _  __ _  ___ _ __ 
					  / _ \| | | |/ _` | |/ _ \| |\/| |/ _` | '_ \ / _` |/ _` |/ _ \ '__|
					 / ___ \ |_| | (_| | | (_) | |  | | (_| | | | | (_| | (_| |  __/ |   
					/_/   \_\__,_|\__,_|_|\___/|_|  |_|\__,_|_| |_|\__,_|\__, |\___|_|   
										             |___/    




                                					QQ
                                                                        QQ[
                                                                  qaap
                                                                  )4W? ayQap
                                                                       4QQQ[
                                                              .awQap   ==
                                                               $WWQF   aaaa,
                                                                 `    QWQWQQ
                                                            _aaap=    J?QQP'|
                                                           qQQWQQ6    -      qyQQQgp
                                                           )4QQQQ^ .yQQQQ,   QQQQQQQ
                                                         aa         4QQQW'   )?QQQP'  gmQQma
                                                       ]QQQQp         .               WQQQQW
                                                        ????    aQQQ6p    _aQQQQa      "??"+
                                                       qaaa,   ]QQQQQQ    jQQQQQQf   -aaaap
                                                      jQQWWQ    )????'    )4QQQQP'   mQWQWQf
                                                      ]WQQQQ    jaaa                 QQQQQQf
                                                      )WQQQQ   yQQWWQp    ayQQQap    QQQQQQf
                                                      )QQQQQ   QQQQQQf   ]QQQQQQQ,   QQQQQQf
                                                      ]QQQQD   QQQQQQf   ]QQQQQQQ[   QQQQQQf
                                                        ??':   QQQQQQf   ]QQQQQQQ[   4QQQQQf
                                                               QQQQQQf   ]QQQQQQQ[   =????'I
                                                               QQQQQQf   ]QQQQQQQ[
                                                              .4QQQQQ'   ]QQQQQQQ[
                                                                ]??"-    ]QQQQQQQ(
                                                                         ]4QQQQQ?
                                                                           :

                            qaayQQQQQQQQQwaa   ]mmmmmmmmmmmmmm  ]mmmmmmg,     ]mmm[  ]mmmm  4mmmg         ymmm' ]mmmm pwLq
                           jQQQQD???????QWWQf  ]QQQP??????????  ]QQQWQQQQp    ]QQQf  ]QQQQ   4QQQ6.     _yQQQ'  ]QQQQ \!'a'
                           ]QQQf        )???'  ]QQQ6aaaaaaaaap  ]QQQf)4QQQ6,  ]QQQf  ]QQQQ   |4QQQ6    jmQQQ'v  ]QQQQ
                           ]QQQ[   ]QQQQWWQQf  ]QQQQQQQWQWQWQf  ]QQQf  ?QQQQa ]QQQf  ]QQQQ    i4QQQ6  qQQQQ'    ]QQQQ
                           ]QQQ6,   .   qQQQf  ]QQQf            ]QQQf   ]4QQQ6jQQQf  ]QQQQ      4QQQ6gQWQQ'     ]QQQQ
                            4QQQQQQQQQQQQQQW'  ]QQQQQQWQWQWQWQ  ]QQQf    i?QQQQQQQf  ]QQQQ       ]QQQQQQQ'      ]QQQQ
                            ++"??????????`     ]""!"""""""""""S ]""!'      ]"""!""'  -""!"        """!""'       ]""!"


----
