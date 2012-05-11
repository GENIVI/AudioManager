/**
 *  Copyright (c) 2012 BMW
 *
 *  \author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
 *
 *  \copyright
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction,
 *  including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 *  subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 *  THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *  For further information see http://www.genivi.org/.
 */

#include "CAmCommandSenderDbusSignalTest.h"
#include <Python.h>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include "CAmCommandSenderDbusBackdoor.h"
#include "MockIAmCommandReceive.h"
#include "../include/CAmCommandSenderDbus.h"
#include "../include/CAmDbusMessageHandler.h"
#include "../../AudioManagerDaemon/include/TAmPluginTemplate.h"
#include "shared/CAmDltWrapper.h"

using namespace am;
using namespace testing;

CAmCommandSenderDbusSignalTest::CAmCommandSenderDbusSignalTest() :
        ppCommandSend(NULL) //
{
    CAmDltWrapper::instance()->registerApp("dbusTest", "dbusTest");
    logInfo("dbusCommandInterfaceSignalTest started");
}

CAmCommandSenderDbusSignalTest::~CAmCommandSenderDbusSignalTest()
{
}

void* NumberOfMainConnectionsChanged(void* ppCommandSend)
{
    sleep(1);
    IAmCommandSend* sender=static_cast<IAmCommandSend*>(ppCommandSend);
    am_MainConnectionType_s mainConnection;
    sender->cbNewMainConnection(mainConnection);
    return (NULL);
}

void* cbSinkAdded(void* ppCommandSend)
{
    sleep(1);
    IAmCommandSend* sender=static_cast<IAmCommandSend*>(ppCommandSend);
    std::vector<am_SinkType_s> list;
    am_SinkType_s mysink;
    mysink.name = "MySink";
    mysink.sinkID = 23;
    mysink.availability.availability = A_MAX;
    mysink.availability.availabilityReason = AR_UNKNOWN;
    mysink.muteState = MS_UNKNOWN;
    mysink.sinkClassID = 3;
    mysink.volume = 234;
    sender->cbNewSink(mysink);
    return (NULL);
}

void* cbSourceAdded(void* ppCommandSend)
{
    sleep(1);
    IAmCommandSend* sender=static_cast<IAmCommandSend*>(ppCommandSend);
    am_SourceType_s mysource;
    mysource.name = "MySink";
    mysource.sourceID = 42;
    mysource.availability.availability = A_MAX;
    mysource.availability.availabilityReason = AR_UNKNOWN;
    mysource.sourceClassID = 15;
    sender->cbNewSource(mysource);
    return (NULL);
}

void* cbSourceRemoved(void* ppCommandSend)
{
    sleep(1);
    IAmCommandSend* sender=static_cast<IAmCommandSend*>(ppCommandSend);
    am_SourceType_s mysource;
    mysource.name = "MySink";
    mysource.sourceID = 42;
    sender->cbRemovedSource(mysource.sourceID);
    return (NULL);
}

void* cbSinkRemoved(void* ppCommandSend)
{
    sleep(1);
    IAmCommandSend* sender=static_cast<IAmCommandSend*>(ppCommandSend);
    am_SinkType_s mysink;
    mysink.name = "MySink";
    mysink.sinkID = 23;
    sender->cbRemovedSink(mysink.sinkID);
    return (NULL);
}

void* NumberOfSinkClassesChanged(void* ppCommandSend)
{
    sleep(1);
    IAmCommandSend* sender=static_cast<IAmCommandSend*>(ppCommandSend);
    sender->cbNumberOfSinkClassesChanged();
    return (NULL);
}

void* NumberOfSourceClassesChanged(void* ppCommandSend)
{
    sleep(1);
    IAmCommandSend* sender=static_cast<IAmCommandSend*>(ppCommandSend);
    sender->cbNumberOfSourceClassesChanged();
    return (NULL);
}

void* MainConnectionStateChanged(void* ppCommandSend)
{
    sleep(1);
    IAmCommandSend* sender=static_cast<IAmCommandSend*>(ppCommandSend);
    am_connectionID_t id = 4;
    am_ConnectionState_e state = CS_CONNECTING;
    sender->cbMainConnectionStateChanged(id, state);
    return (NULL);
}

void* MainSinkSoundPropertyChanged(void* ppCommandSend)
{
    sleep(1);
    IAmCommandSend* sender=static_cast<IAmCommandSend*>(ppCommandSend);
    am_sinkID_t sinkID = 3;
    am_MainSoundProperty_s soundProperty;
    soundProperty.type = MSP_UNKNOWN;
    soundProperty.value = 23;
    sender->cbMainSinkSoundPropertyChanged(sinkID, soundProperty);
    return (NULL);
}

void* MainSourceSoundPropertyChanged(void* ppCommandSend)
{
    sleep(1);
    IAmCommandSend* sender=static_cast<IAmCommandSend*>(ppCommandSend);
    am_sourceID_t sourceID = 35;
    am_MainSoundProperty_s soundProperty;
    soundProperty.type = MSP_UNKNOWN;
    soundProperty.value = 233;
    sender->cbMainSourceSoundPropertyChanged(sourceID, soundProperty);
    return (NULL);
}

void* cbSinkAvailabilityChangedLoop(void* ppCommandSend)
{
    sleep(1);
    IAmCommandSend* sender=static_cast<IAmCommandSend*>(ppCommandSend);
    am_Availability_s av;
    av.availability = A_AVAILABLE;
    av.availabilityReason = AR_UNKNOWN;
    sender->cbSinkAvailabilityChanged(4, av);
    return (NULL);
}

void* VolumeChanged(void* ppCommandSend)
{
    sleep(1);
    IAmCommandSend* sender=static_cast<IAmCommandSend*>(ppCommandSend);
    am_volume_t volume = 344;
    sender->cbVolumeChanged(23, volume);
    return (NULL);
}

void* cbSourceAvailabilityChangedLoop(void* ppCommandSend)
{
    sleep(1);
    IAmCommandSend* sender=static_cast<IAmCommandSend*>(ppCommandSend);
    am_Availability_s av;
    av.availability = A_AVAILABLE;
    av.availabilityReason = AR_UNKNOWN;
    sender->cbSourceAvailabilityChanged(2, av);
    return (NULL);
}

void* SinkMuteStateChanged(void* ppCommandSend)
{
    sleep(1);
    IAmCommandSend* sender=static_cast<IAmCommandSend*>(ppCommandSend);
    sender->cbSinkMuteStateChanged(42, MS_MUTED);
    return (NULL);
}

void* SystemPropertyChanged(void* ppCommandSend)
{
    sleep(1);
    IAmCommandSend* sender=static_cast<IAmCommandSend*>(ppCommandSend);
    am_SystemProperty_s property;
    property.type = SYP_UNKNOWN;
    property.value = 355;
    sender->cbSystemPropertyChanged(property);
    return (NULL);
}

void* TimingInformationChanged(void* ppCommandSend)
{
    sleep(1);
    IAmCommandSend* sender=static_cast<IAmCommandSend*>(ppCommandSend);
    sender->cbTimingInformationChanged(42, 233);
    return (NULL);
}

TEST_F(CAmCommandSenderDbusSignalTest,cbSourceAvailabilityChanged)
{

    //ok, now we want to test the signals. It did not work out that the python receiver worked in an own thread (as originally intended)
    //so it is running in the main context and the signals are send from threads...
    CAmSocketHandler pSocketHandler;
    CAmDbusWrapper pDBusWrapper(&pSocketHandler);
    MockIAmCommandReceive pReceiveInterface;

    IAmCommandSend* (*createFunc)();
    void* tempLibHandle = NULL;
    std::string libname("../plugins/command/libPluginCommandInterfaceDbus.so");
    createFunc = getCreateFunction<IAmCommandSend*()>(libname, tempLibHandle);

    if (!createFunc)
    {
        logError("CommandSendInterface Test Entry point of RoutingPlugin not found");
        exit(1);
    }

    ppCommandSend = createFunc();

    if (!ppCommandSend)
    {
        logError("CommandSendInterface Test RoutingPlugin initialization failed. Entry Function not callable");
        exit(1);
    }

    //  ok, here we give the DBusWrapper pointer to the Plugin and start the interface
    EXPECT_CALL(pReceiveInterface,getDBusConnectionWrapper(_)).WillRepeatedly(DoAll(SetArgReferee<0>(&pDBusWrapper), Return(E_OK)));
    EXPECT_CALL(pReceiveInterface, confirmCommandReady(10));

    ppCommandSend->startupInterface(&pReceiveInterface);
    ppCommandSend->setCommandReady(10);

    pthread_t pythonloop;
    pthread_create(&pythonloop, NULL, NumberOfMainConnectionsChanged, (void*) ppCommandSend);
    PyRun_SimpleStringFlags("import sys\n"
            "import traceback\n"
            "import gobject\n"
            "import dbus\n"
            "import dbus.mainloop.glib\n"
            "loop = gobject.MainLoop()\n"
            "def catchsignal(*arg, **kwarg):\n"
            "	print ('Caught NumberOfMainConnectionsChanged') \n"
            "	loop.quit()\n"
            "dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
            "bus = dbus.SessionBus()\n"
            "bus.add_signal_receiver(catchsignal, signal_name='NumberOfMainConnectionsChanged', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
            "loop.run()\n", NULL);
    pthread_join(pythonloop, NULL);

    pthread_create(&pythonloop, NULL, cbSinkAdded, (void*) ppCommandSend);
    PyRun_SimpleStringFlags("import sys\n"
            "import traceback\n"
            "import gobject\n"
            "import dbus\n"
            "import dbus.mainloop.glib\n"
            "loop = gobject.MainLoop()\n"
            "def catchSinkAdded(*arg, **karg):\n"
            "	print ('Caught signal (in SinkAdded handler) ') \n"
            "	print (arg[0])\n"
            "	f = open('/tmp/result.txt','w')\n"
            "	f.write(str(arg[0]));\n"
            "	f.close()\n"
            "	loop.quit()\n"
            "dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
            "bus = dbus.SessionBus()\n"
            "bus.add_signal_receiver(catchSinkAdded, signal_name='SinkAdded', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
            "loop.run()\n", NULL);
    pthread_join(pythonloop, NULL);

    std::ifstream ifs("/tmp/result.txt");
    std::string line;
    int lineCounter = 0, result = 0;
    while (std::getline(ifs, line))
    {
        ASSERT_EQ(line.compare("dbus.Struct((dbus.UInt16(23), dbus.String(u'MySink'), dbus.Struct((dbus.Int16(3), dbus.Int16(0)), signature=None), dbus.Int16(234), dbus.Int16(0), dbus.UInt16(3)), signature=None)"), 0);
    }
    ifs.close();

    pthread_create(&pythonloop, NULL, cbSinkRemoved, (void*) ppCommandSend);
    PyRun_SimpleStringFlags("import sys\n"
            "import traceback\n"
            "import gobject\n"
            "import dbus\n"
            "import dbus.mainloop.glib\n"
            "loop = gobject.MainLoop()\n"
            "def catchSinkRemoved(*arg, **karg):\n"
            "	print ('Caught signal (in SinkRemoved handler) ') \n"
            "	print (arg[0])\n"
            "	f = open('/tmp/result.txt','w')\n"
            "	f.write(str(arg[0]));\n"
            "	f.close()\n"
            "	loop.quit()\n"
            "dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
            "bus = dbus.SessionBus()\n"
            "bus.add_signal_receiver(catchSinkRemoved, signal_name='SinkRemoved', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
            "loop.run()\n", NULL);
    pthread_join(pythonloop, NULL);

    ifs.open("/tmp/result.txt");
    lineCounter = 0;
    result = 0;
    while (std::getline(ifs, line))
    {
        std::stringstream(line) >> result;
        ASSERT_EQ(result, 23);
    }
    ifs.close();

    pthread_create(&pythonloop, NULL, cbSourceAdded, (void*) ppCommandSend);
    PyRun_SimpleStringFlags("import sys\n"
            "import traceback\n"
            "import gobject\n"
            "import dbus\n"
            "import dbus.mainloop.glib\n"
            "loop = gobject.MainLoop()\n"
            "def catchSourceAdded(*arg, **karg):\n"
            "	print ('Caught signal (in SourceAdded handler) ') \n"
            "	print (arg[0])\n"
            "	f = open('/tmp/result.txt','w')\n"
            "	f.write(str(arg[0]));\n"
            "	f.close()\n"
            "	loop.quit()\n"
            "dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
            "bus = dbus.SessionBus()\n"
            "bus.add_signal_receiver(catchSourceAdded, signal_name='SourceAdded', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
            "loop.run()\n", NULL);
    pthread_join(pythonloop, NULL);

    ifs.open("/tmp/result.txt");
    lineCounter = 0;
    result = 0;
    while (std::getline(ifs, line))
    {
        ASSERT_EQ(line.compare("dbus.Struct((dbus.UInt16(42), dbus.String(u'MySink'), dbus.Struct((dbus.Int16(3), dbus.Int16(0)), signature=None), dbus.UInt16(15)), signature=None)"), 0);
    }
    ifs.close();

    pthread_create(&pythonloop, NULL, cbSourceRemoved, (void*) ppCommandSend);
    PyRun_SimpleStringFlags("import sys\n"
            "import traceback\n"
            "import gobject\n"
            "import dbus\n"
            "import dbus.mainloop.glib\n"
            "loop = gobject.MainLoop()\n"
            "def catchSourceRemoved(*arg, **karg):\n"
            "	print ('Caught signal (in SinkRemoved handler) ') \n"
            "	print (arg[0])\n"
            "	f = open('/tmp/result.txt','w')\n"
            "	f.write(str(arg[0]));\n"
            "	f.close()\n"
            "	loop.quit()\n"
            "dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
            "bus = dbus.SessionBus()\n"
            "bus.add_signal_receiver(catchSourceRemoved, signal_name='SourceRemoved', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
            "loop.run()\n", NULL);
    pthread_join(pythonloop, NULL);

    ifs.open("/tmp/result.txt");
    lineCounter = 0;
    result = 0;
    while (std::getline(ifs, line))
    {
        std::stringstream(line) >> result;
        ASSERT_EQ(result, 42);
    }
    ifs.close();

    pthread_create(&pythonloop, NULL, NumberOfSinkClassesChanged, (void*) ppCommandSend);
    PyRun_SimpleStringFlags("import sys\n"
            "import traceback\n"
            "import gobject\n"
            "import dbus\n"
            "import dbus.mainloop.glib\n"
            "loop = gobject.MainLoop()\n"
            "def catchNumberOfSinkClassesChanged(*arg, **kwarg):\n"
            "	print ('Caught catchNumberOfSinkClassesChanged') \n"
            "	loop.quit()\n"
            "dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
            "bus = dbus.SessionBus()\n"
            "bus.add_signal_receiver(catchNumberOfSinkClassesChanged, signal_name='NumberOfSinkClassesChanged', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
            "loop.run()\n", NULL);
    pthread_join(pythonloop, NULL);

    pthread_create(&pythonloop, NULL, NumberOfSourceClassesChanged, (void*) ppCommandSend);
    PyRun_SimpleStringFlags("import sys\n"
            "import traceback\n"
            "import gobject\n"
            "import dbus\n"
            "import dbus.mainloop.glib\n"
            "loop = gobject.MainLoop()\n"
            "def CatchNumberOfSourceClassesChanged(*arg, **kwarg):\n"
            "	print ('Caught CatchNumberOfSourceClassesChanged') \n"
            "	loop.quit()\n"
            "dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
            "bus = dbus.SessionBus()\n"
            "bus.add_signal_receiver(CatchNumberOfSourceClassesChanged, signal_name='NumberOfSourceClassesChanged', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
            "loop.run()\n", NULL);
    pthread_join(pythonloop, NULL);

    //ok, now we want to test the signals. It did not work out that the python receiver worked in an own thread (as originally intended)
    //so it is running in the main context and the signals are send from threads...
    pthread_create(&pythonloop, NULL, MainConnectionStateChanged, (void*) ppCommandSend);
    PyRun_SimpleStringFlags("import sys\n"
            "import traceback\n"
            "import gobject\n"
            "import dbus\n"
            "import dbus.mainloop.glib\n"
            "loop = gobject.MainLoop()\n"
            "def catchMainConnectionStateChanged(*arg, **karg):\n"
            "	print ('Caught signal (in catchMainConnectionStateChanged handler) ') \n"
            "	print (arg[0])\n"
            "	print (arg[1])\n"
            "	f = open('/tmp/result.txt','w')\n"
            "	f.write(str(arg[0]) + '\\n' + str (arg[1]));\n"
            "	f.close()\n"
            "	loop.quit()\n"
            "dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
            "bus = dbus.SessionBus()\n"
            "bus.add_signal_receiver(catchMainConnectionStateChanged, signal_name='MainConnectionStateChanged', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
            "loop.run()\n", NULL);
    pthread_join(pythonloop, NULL);

    ifs.open("/tmp/result.txt");
    lineCounter = 0;
    result = 0;
    while (std::getline(ifs, line))
    {
        if (lineCounter == 0)
        {
            std::stringstream(line) >> result;
            ASSERT_EQ(result, 4);
        }
        else if (lineCounter == 1)
        {
            std::stringstream(line) >> result;
            ASSERT_EQ(result, CS_CONNECTING);
        }
        lineCounter++;
    }
    ifs.close();

    //ok, now we want to test the signals. It did not work out that the python receiver worked in an own thread (as originally intended)
    //so it is running in the main context and the signals are send from threads...
    pthread_create(&pythonloop, NULL, MainSinkSoundPropertyChanged, (void*) ppCommandSend);
    PyRun_SimpleStringFlags("import sys\n"
            "import traceback\n"
            "import gobject\n"
            "import dbus\n"
            "import dbus.mainloop.glib\n"
            "loop = gobject.MainLoop()\n"
            "def catchMainSinkSoundPropertyChanged(*arg, **karg):\n"
            "	print ('Caught signal (in catchMainSinkSoundPropertyChanged handler) ') \n"
            "	print (arg[0])\n"
            "	print (arg[1])\n"
            "	f = open('/tmp/result.txt','w')\n"
            "	f.write(str(arg[0]) + '\\n' + str (arg[1]));\n"
            "	f.close()\n"
            "	loop.quit()\n"
            "dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
            "bus = dbus.SessionBus()\n"
            "bus.add_signal_receiver(catchMainSinkSoundPropertyChanged, signal_name='MainSinkSoundPropertyChanged', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
            "loop.run()\n", NULL);
    pthread_join(pythonloop, NULL);

    ifs.open("/tmp/result.txt");
    lineCounter = 0;
    result = 0;
    while (std::getline(ifs, line))
    {
        if (lineCounter == 0)
        {
            std::stringstream(line) >> result;
            ASSERT_EQ(result, 3);
        }
        else if (lineCounter == 1)
        {
            ASSERT_EQ(line.compare("dbus.Struct((dbus.Int16(0), dbus.Int16(23)), signature=None)"), 0);
        }
        lineCounter++;
    }
    ifs.close();

    //ok, now we want to test the signals. It did not work out that the python receiver worked in an own thread (as originally intended)
    //so it is running in the main context and the signals are send from threads...
    pthread_create(&pythonloop, NULL, MainSourceSoundPropertyChanged, (void*) ppCommandSend);
    PyRun_SimpleStringFlags("import sys\n"
            "import traceback\n"
            "import gobject\n"
            "import dbus\n"
            "import dbus.mainloop.glib\n"
            "loop = gobject.MainLoop()\n"
            "def catchMainSourceSoundPropertyChanged(*arg, **karg):\n"
            "	print ('Caught signal (in catchMainSourceSoundPropertyChanged handler) ') \n"
            "	print (arg[0])\n"
            "	print (arg[1])\n"
            "	f = open('/tmp/result.txt','w')\n"
            "	f.write(str(arg[0]) + '\\n' + str (arg[1]));\n"
            "	f.close()\n"
            "	loop.quit()\n"
            "dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
            "bus = dbus.SessionBus()\n"
            "bus.add_signal_receiver(catchMainSourceSoundPropertyChanged, signal_name='MainSourceSoundPropertyChanged', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
            "loop.run()\n", NULL);
    pthread_join(pythonloop, NULL);

    ifs.open("/tmp/result.txt");
    lineCounter = 0;
    result = 0;
    while (std::getline(ifs, line))
    {
        if (lineCounter == 0)
        {
            std::stringstream(line) >> result;
            ASSERT_EQ(result, 35);
        }
        else if (lineCounter == 1)
        {
            ASSERT_EQ(line.compare("dbus.Struct((dbus.Int16(0), dbus.Int16(233)), signature=None)"), 0);
        }
        lineCounter++;
    }
    ifs.close();

    //ok, now we want to test the signals. It did not work out that the python receiver worked in an own thread (as originally intended)
    //so it is running in the main context and the signals are send from threads...
    pthread_create(&pythonloop, NULL, cbSinkAvailabilityChangedLoop, (void*) ppCommandSend);
    PyRun_SimpleStringFlags("import sys\n"
            "import traceback\n"
            "import gobject\n"
            "import dbus\n"
            "import dbus.mainloop.glib\n"
            "loop = gobject.MainLoop()\n"
            "def catchSinkAvailabilityChanged(*arg, **karg):\n"
            "	print ('Caught signal (in catchSinkAvailabilityChanged handler) ') \n"
            "	print (arg[0])\n"
            "	print (arg[1])\n"
            "	f = open('/tmp/result.txt','w')\n"
            "	f.write(str(arg[0]) + '\\n' + str (arg[1]));\n"
            "	f.close()\n"
            "	loop.quit()\n"
            "dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
            "bus = dbus.SessionBus()\n"
            "bus.add_signal_receiver(catchSinkAvailabilityChanged, signal_name='SinkAvailabilityChanged', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
            "loop.run()\n", NULL);
    pthread_join(pythonloop, NULL);

    ifs.open("/tmp/result.txt");
    lineCounter = 0;
    result = 0;
    while (std::getline(ifs, line))
    {
        if (lineCounter == 0)
        {
            std::stringstream(line) >> result;
            ASSERT_EQ(result, 4);
        }
        else if (lineCounter == 1)
        {
            ASSERT_EQ(line.compare("dbus.Struct((dbus.Int16(1), dbus.Int16(0)), signature=None)"), 0);
        }
        lineCounter++;
    }
    ifs.close();

    //ok, now we want to test the signals. It did not work out that the python receiver worked in an own thread (as originally intended)
    //so it is running in the main context and the signals are send from threads...
    pthread_create(&pythonloop, NULL, cbSourceAvailabilityChangedLoop, (void*) ppCommandSend);
    PyRun_SimpleStringFlags("import sys\n"
            "import traceback\n"
            "import gobject\n"
            "import dbus\n"
            "import dbus.mainloop.glib\n"
            "loop = gobject.MainLoop()\n"
            "def catchSourceAvailability(*arg, **karg):\n"
            "	print ('Caught signal (in catchSourceAvailability handler) ') \n"
            "	print (arg[0])\n"
            "	print (arg[1])\n"
            "	f = open('/tmp/result.txt','w')\n"
            "	f.write(str(arg[0]) + '\\n' + str (arg[1]));\n"
            "	f.close()\n"
            "	loop.quit()\n"
            "dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
            "bus = dbus.SessionBus()\n"
            "bus.add_signal_receiver(catchSourceAvailability, signal_name='SourceAvailabilityChanged', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
            "loop.run()\n", NULL);
    pthread_join(pythonloop, NULL);

    ifs.open("/tmp/result.txt");
    lineCounter = 0;
    result = 0;
    while (std::getline(ifs, line))
    {
        if (lineCounter == 0)
        {
            std::stringstream(line) >> result;
            ASSERT_EQ(result, 2);
        }
        else if (lineCounter == 1)
        {
            ASSERT_EQ(line.compare("dbus.Struct((dbus.Int16(1), dbus.Int16(0)), signature=None)"), 0);
        }
        lineCounter++;
    }
    ifs.close();

    //ok, now we want to test the signals. It did not work out that the python receiver worked in an own thread (as originally intended)
    //so it is running in the main context and the signals are send from threads...
    pthread_create(&pythonloop, NULL, VolumeChanged, (void*) ppCommandSend);
    PyRun_SimpleStringFlags("import sys\n"
            "import traceback\n"
            "import gobject\n"
            "import dbus\n"
            "import dbus.mainloop.glib\n"
            "loop = gobject.MainLoop()\n"
            "def catchVolumeChanged(*arg, **karg):\n"
            "	print ('Caught signal (in catchVolumeChanged handler) ') \n"
            "	print (arg[0])\n"
            "	print (arg[1])\n"
            "	f = open('/tmp/result.txt','w')\n"
            "	f.write(str(arg[0]) + '\\n' + str (arg[1]));\n"
            "	f.close()\n"
            "	loop.quit()\n"
            "dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
            "bus = dbus.SessionBus()\n"
            "bus.add_signal_receiver(catchVolumeChanged, signal_name='VolumeChanged', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
            "loop.run()\n", NULL);
    pthread_join(pythonloop, NULL);

    ifs.open("/tmp/result.txt");
    lineCounter = 0;
    result = 0;
    while (std::getline(ifs, line))
    {
        if (lineCounter == 0)
        {
            std::stringstream(line) >> result;
            ASSERT_EQ(result, 23);
        }
        else if (lineCounter == 1)
        {
            std::stringstream(line) >> result;
            ASSERT_EQ(result, 344);
        }
        lineCounter++;
    }
    ifs.close();

    //ok, now we want to test the signals. It did not work out that the python receiver worked in an own thread (as originally intended)
    //so it is running in the main context and the signals are send from threads...
    pthread_create(&pythonloop, NULL, SinkMuteStateChanged, (void*) ppCommandSend);
    PyRun_SimpleStringFlags("import sys\n"
            "import traceback\n"
            "import gobject\n"
            "import dbus\n"
            "import dbus.mainloop.glib\n"
            "loop = gobject.MainLoop()\n"
            "def catchSinkMuteStateChanged(*arg, **karg):\n"
            "	print ('Caught signal (in catchSinkMuteStateChanged handler) ') \n"
            "	print (arg[0])\n"
            "	print (arg[1])\n"
            "	f = open('/tmp/result.txt','w')\n"
            "	f.write(str(arg[0]) + '\\n' + str (arg[1]));\n"
            "	f.close()\n"
            "	loop.quit()\n"
            "dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
            "bus = dbus.SessionBus()\n"
            "bus.add_signal_receiver(catchSinkMuteStateChanged, signal_name='SinkMuteStateChanged', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
            "loop.run()\n", NULL);
    pthread_join(pythonloop, NULL);

    ifs.open("/tmp/result.txt");
    lineCounter = 0;
    result = 0;
    while (std::getline(ifs, line))
    {
        if (lineCounter == 0)
        {
            std::stringstream(line) >> result;
            ASSERT_EQ(result, 42);
        }
        else if (lineCounter == 1)
        {
            std::stringstream(line) >> result;
            ASSERT_EQ(result, MS_MUTED);
        }
        lineCounter++;
    }
    ifs.close();

    //ok, now we want to test the signals. It did not work out that the python receiver worked in an own thread (as originally intended)
    //so it is running in the main context and the signals are send from threads...
    pthread_create(&pythonloop, NULL, SystemPropertyChanged, (void*) ppCommandSend);
    PyRun_SimpleStringFlags("import sys\n"
            "import traceback\n"
            "import gobject\n"
            "import dbus\n"
            "import dbus.mainloop.glib\n"
            "loop = gobject.MainLoop()\n"
            "def catchSystemPropertyChanged(*arg, **karg):\n"
            "	print ('Caught signal (in catchSystemPropertyChanged handler) ') \n"
            "	print (arg[0])\n"
            "	f = open('/tmp/result.txt','w')\n"
            "	f.write(str(arg[0]));\n"
            "	f.close()\n"
            "	loop.quit()\n"
            "dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
            "bus = dbus.SessionBus()\n"
            "bus.add_signal_receiver(catchSystemPropertyChanged, signal_name='SystemPropertyChanged', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
            "loop.run()\n", NULL);
    pthread_join(pythonloop, NULL);

    ifs.open("/tmp/result.txt");
    lineCounter = 0;
    result = 0;
    while (std::getline(ifs, line))
    {
        ASSERT_EQ(line.compare("dbus.Struct((dbus.Int16(0), dbus.Int16(355)), signature=None)"), 0);
    }
    ifs.close();

    //ok, now we want to test the signals. It did not work out that the python receiver worked in an own thread (as originally intended)
    //so it is running in the main context and the signals are send from threads...
    pthread_create(&pythonloop, NULL, TimingInformationChanged, (void*) ppCommandSend);
    PyRun_SimpleStringFlags("import sys\n"
            "import traceback\n"
            "import gobject\n"
            "import dbus\n"
            "import dbus.mainloop.glib\n"
            "loop = gobject.MainLoop()\n"
            "def catchTimingInformationChanged(*arg, **karg):\n"
            "	print ('Caught signal (in catchTimingInformationChanged handler) ') \n"
            "	print (arg[0])\n"
            "	print (arg[1])\n"
            "	f = open('/tmp/result.txt','w')\n"
            "	f.write(str(arg[0]) + '\\n' + str (arg[1]));\n"
            "	f.close()\n"
            "	loop.quit()\n"
            "dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
            "bus = dbus.SessionBus()\n"
            "bus.add_signal_receiver(catchTimingInformationChanged, signal_name='TimingInformationChanged', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
            "loop.run()\n", NULL);
    pthread_join(pythonloop, NULL);

    ifs.open("/tmp/result.txt");
    lineCounter = 0;
    result = 0;
    while (std::getline(ifs, line))
    {
        if (lineCounter == 0)
        {
            std::stringstream(line) >> result;
            ASSERT_EQ(result, 42);
        }
        else if (lineCounter == 1)
        {
            std::stringstream(line) >> result;
            ASSERT_EQ(result, 233);
        }
        lineCounter++;
    }
    ifs.close();

}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

void CAmCommandSenderDbusSignalTest::SetUp()
{
    Py_Initialize();
}

void CAmCommandSenderDbusSignalTest::TearDown()
{
    Py_Finalize();
}

