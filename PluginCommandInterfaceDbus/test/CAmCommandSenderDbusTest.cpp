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

#include "CAmCommandSenderDbusTest.h"
#include <Python.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <vector>
#include <set>
#include "TAmPluginTemplate.h"
#include "MockIAmCommandReceive.h"
#include "shared/CAmDltWrapper.h"
#include "shared/CAmSocketHandler.h"
#include "shared/CAmDbusWrapper.h"
#include "../include/CAmCommandSenderDbus.h"
#include "../include/CAmDbusMessageHandler.h"

using namespace am;
using namespace testing;

std::string DBUSCOMMAND = "dbus-send --session --print-reply --dest=org.genivi.audiomanager /org/genivi/audiomanager/CommandInterface org.genivi.audiomanager.CommandInterface.";

void* run_the_loop(void* wrapper)
{
    CAmSocketHandler* wrap = (CAmSocketHandler*) wrapper;
    wrap->start_listenting();
    return (NULL);
}

CAmCommandSenderDbusTest::CAmCommandSenderDbusTest() :
        ppCommandSend(NULL) //
{
    CAmDltWrapper::instance()->registerApp("dbusTest", "dbusTest");
}

CAmCommandSenderDbusTest::~CAmCommandSenderDbusTest()
{

}

void CAmCommandSenderDbusTest::SetUp()
{

}

void CAmCommandSenderDbusTest::TearDown()
{

}

ACTION(returnListConnections){
std::vector<am::am_MainConnectionType_s> list;
am::am_MainConnectionType_s listItem;
listItem.mainConnectionID=15;
listItem.sinkID=4;
listItem.sourceID=3;
listItem.connectionState=CS_UNKNOWN;
listItem.delay=34;
list.push_back(listItem);
arg0=list;
}

ACTION(returnListSinks){
std::vector<am::am_SinkType_s> list;
am::am_SinkType_s listItem;
listItem.availability.availability=A_UNAVAILABLE;
listItem.availability.availabilityReason=AR_GENIVI_NOMEDIA;
listItem.muteState=MS_UNMUTED;
listItem.name="mySink";
listItem.sinkClassID=34;
listItem.sinkID=24;
listItem.volume=124;
list.push_back(listItem);
arg0=list;
}

ACTION(returnListSources){
std::vector<am::am_SourceType_s> list;
am::am_SourceType_s listItem;
listItem.availability.availability=A_MAX;
listItem.availability.availabilityReason=AR_GENIVI_SAMEMEDIA;
listItem.name="MySource";
listItem.sourceClassID=12;
listItem.sourceID=224;
list.push_back(listItem);
listItem.name="NextSource";
listItem.sourceID=22;
list.push_back(listItem);
arg0=list;
}

ACTION(returnListMainSinkSoundProperties){
std::vector<am::am_MainSoundProperty_s> list;
am::am_MainSoundProperty_s listItem;
listItem.type=MSP_EXAMPLE_MID;
listItem.value=223;
list.push_back(listItem);
listItem.type=MSP_UNKNOWN;
listItem.value=2;
list.push_back(listItem);
arg1=list;
}

ACTION(returnListSourceClasses){
std::vector<am::am_SourceClass_s> list;
am::am_SourceClass_s listItem;
am::am_ClassProperty_s property;
property.classProperty=CP_MAX;
property.value=12;
listItem.name="FirstCLass";
listItem.sourceClassID=23;
listItem.listClassProperties.push_back(property);
list.push_back(listItem);
listItem.name="SecondCLass";
listItem.sourceClassID=2;
listItem.listClassProperties.push_back(property);
list.push_back(listItem);
arg0=list;
}

ACTION(returnListSinkClasses){
std::vector<am::am_SinkClass_s> list;
am::am_SinkClass_s listItem;
am::am_ClassProperty_s property;
property.classProperty=CP_MAX;
property.value=122;
listItem.name="FirstCLass";
listItem.sinkClassID=2123;
listItem.listClassProperties.push_back(property);
list.push_back(listItem);
listItem.name="SecondCLass";
listItem.sinkClassID=23;
listItem.listClassProperties.push_back(property);
list.push_back(listItem);
arg0=list;
}

ACTION(returnListSystemProperties){
std::vector<am::am_SystemProperty_s> list;
am::am_SystemProperty_s listItem;
listItem.type=SYP_UNKNOWN;
listItem.value=-2245;
list.push_back(listItem);
arg0=list;
}

ACTION(returnTimingInfo){
am::am_timeSync_t time=23;
arg1=time;
}

TEST_F(CAmCommandSenderDbusTest, MessageTest)
{
    Py_Initialize();
    //unfortunatly we need to put all in one testcase because testing with the dbus loop caused problems...
    CAmSocketHandler pSocketHandler;
    CAmDbusWrapper pDBusWrapper(&pSocketHandler);
    pthread_t ptestThread;
    std::vector<std::string> plistCommandPluginDirs;
    plistCommandPluginDirs.push_back(std::string(DEFAULT_PLUGIN_COMMAND_DIR));

    MockIAmCommandReceive pReceiveInterface;

    //this class just creates the thread that will handle the mainloop...
    pthread_create(&ptestThread, NULL, run_the_loop, (void*) &pSocketHandler);

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

//	ok, here we give the DBusWrapper pointer to the Plugin and start the interface
    EXPECT_CALL(pReceiveInterface,getDBusConnectionWrapper(_)).WillRepeatedly(DoAll(SetArgReferee<0>(&pDBusWrapper), Return(E_OK)));
    EXPECT_CALL(pReceiveInterface, confirmCommandReady(10));

    ppCommandSend->startupInterface(&pReceiveInterface);
    ppCommandSend->setCommandReady(10);

    EXPECT_CALL(pReceiveInterface,connect(2,3,_)).WillRepeatedly(DoAll(SetArgReferee<2>(35), Return(E_OK)));
    system((DBUSCOMMAND + std::string("Connect uint16:2 uint16:3 > /tmp/result.txt ")).c_str());

    //check the results
    std::ifstream ifs("/tmp/result.txt");
    std::string line;
    int lineCounter = 0, result = 0;
    while (std::getline(ifs, line))
    {
        if (lineCounter == 1)
        {
            std::stringstream(line.replace(line.begin(), line.begin() + 9, "")) >> result;
            ASSERT_EQ(result, E_OK);
        }
        else if (lineCounter == 2)
        {
            std::stringstream(line.replace(line.begin(), line.begin() + 10, "")) >> result;
            ASSERT_EQ(result, 35);
        }
        lineCounter++;
    }
    ifs.close();

    std::cout << "[connect   ]" << std::endl;

    EXPECT_CALL(pReceiveInterface,disconnect(2)).WillOnce(Return(E_OK));
    system((DBUSCOMMAND + std::string("Disconnect uint16:2 > /tmp/result.txt ")).c_str());

    //check the results
    lineCounter = 0;
    result = 0;
    ifs.open("/tmp/result.txt");
    while (std::getline(ifs, line))
    {
        if (lineCounter == 1)
        {
            std::stringstream(line.replace(line.begin(), line.begin() + 9, "")) >> result;
            ASSERT_EQ(result, E_OK);
        }
        lineCounter++;
    }
    ifs.close();

    std::cout << "[disconnect]" << std::endl;

    EXPECT_CALL(pReceiveInterface,setVolume(22,12)).WillOnce(Return(E_OK));
    system((DBUSCOMMAND + std::string("SetVolume uint16:22 int16:12 > /tmp/result.txt ")).c_str());

    //check the results
    lineCounter = 0;
    result = 0;
    ifs.open("/tmp/result.txt");
    while (std::getline(ifs, line))
    {
        if (lineCounter == 1)
        {
            std::stringstream(line.replace(line.begin(), line.begin() + 9, "")) >> result;
            ASSERT_EQ(result, E_OK);
        }
        lineCounter++;
    }
    ifs.close();

    std::cout << "[setVolume ]" << std::endl;

    EXPECT_CALL(pReceiveInterface,volumeStep(2,1)).WillOnce(Return(E_OK));
    system((DBUSCOMMAND + std::string("VolumeStep uint16:2 int16:1 > /tmp/result.txt ")).c_str());

    //check the results
    lineCounter = 0;
    result = 0;
    ifs.open("/tmp/result.txt");
    while (std::getline(ifs, line))
    {
        if (lineCounter == 1)
        {
            std::stringstream(line.replace(line.begin(), line.begin() + 9, "")) >> result;
            ASSERT_EQ(result, E_OK);
        }
        lineCounter++;
    }
    ifs.close();

    std::cout << "[volumeStep]" << std::endl;

    EXPECT_CALL(pReceiveInterface,setSinkMuteState(1,MS_UNKNOWN)).WillOnce(Return(E_OK));
    system((DBUSCOMMAND + std::string("SetSinkMuteState uint16:1 int16:0 > /tmp/result.txt ")).c_str());

    //check the results
    lineCounter = 0;
    result = 0;
    ifs.open("/tmp/result.txt");
    while (std::getline(ifs, line))
    {
        if (lineCounter == 1)
        {
            std::stringstream(line.replace(line.begin(), line.begin() + 9, "")) >> result;
            ASSERT_EQ(result, E_OK);
        }
        lineCounter++;
    }
    ifs.close();

    std::cout << "[sinkmutest]" << std::endl;

    EXPECT_CALL(pReceiveInterface,setMainSinkSoundProperty(AllOf(Field(&am_MainSoundProperty_s::value, 3),
                            Field(&am_MainSoundProperty_s::type,MSP_UNKNOWN)),1)).WillOnce(Return(E_ABORTED));

    PyRun_SimpleStringFlags("import dbus\n"
            "f = open('/tmp/result.txt','w')\n"
            "bus = dbus.SessionBus()\n"
            "retVal=dbus.Bus().call_blocking(\n"
            "bus_name='org.genivi.audiomanager',\n"
            "object_path='/org/genivi/audiomanager/CommandInterface',\n"
            "dbus_interface='org.genivi.audiomanager.CommandInterface',\n"
            "method='SetMainSinkSoundProperty',\n"
            "signature='q(nn)',\n"
            "args=[1,(0,3)],) \n"
            "f.write(str(retVal));\n"
            "f.close()", NULL);
    result = 0;
    ifs.open("/tmp/result.txt");
    while (std::getline(ifs, line))
    {
        std::stringstream(line) >> result;
    }ASSERT_EQ(result, E_ABORTED);
    ifs.close();

    std::cout << "[sinksound ]" << std::endl;

    EXPECT_CALL(pReceiveInterface,setMainSourceSoundProperty(AllOf(Field(&am_MainSoundProperty_s::value, 3),
                            Field(&am_MainSoundProperty_s::type,MSP_UNKNOWN)),1)).WillOnce(Return(E_ABORTED));

    PyRun_SimpleStringFlags("import dbus\n"
            "f = open('/tmp/result.txt','w')\n"
            "bus = dbus.SessionBus()\n"
            "retVal=dbus.Bus().call_blocking(\n"
            "bus_name='org.genivi.audiomanager',\n"
            "object_path='/org/genivi/audiomanager/CommandInterface',\n"
            "dbus_interface='org.genivi.audiomanager.CommandInterface',\n"
            "method='SetMainSourceSoundProperty',\n"
            "signature='q(nn)',\n"
            "args=[1,(0,3)],) \n"
            "f.write(str(retVal));\n"
            "f.close()", NULL);
    result = 0;
    ifs.open("/tmp/result.txt");
    while (std::getline(ifs, line))
    {
        std::stringstream(line) >> result;
    }ASSERT_EQ(result, E_ABORTED);
    ifs.close();

    std::cout << "[sourcesnd ]" << std::endl;

    EXPECT_CALL(pReceiveInterface,setSystemProperty(Field(&am_SystemProperty_s::value,2))).WillOnce(Return(E_DATABASE_ERROR));

    PyRun_SimpleStringFlags("import dbus\n"
            "f = open('/tmp/result.txt','w')\n"
            "bus = dbus.SessionBus()\n"
            "retVal=dbus.Bus().call_blocking(\n"
            "bus_name='org.genivi.audiomanager',\n"
            "object_path='/org/genivi/audiomanager/CommandInterface',\n"
            "dbus_interface='org.genivi.audiomanager.CommandInterface',\n"
            "method='SetSystemProperty',\n"
            "signature='(nn)',\n"
            "args=[(2,2)],) \n"
            "f.write(str(retVal));\n"
            "f.close()", NULL);

    result = 0;
    ifs.open("/tmp/result.txt");
    while (std::getline(ifs, line))
    {
        std::stringstream(line) >> result;
    }ASSERT_EQ(result, E_DATABASE_ERROR);
    ifs.close();

    std::cout << "[systemprop]" << std::endl;

    EXPECT_CALL(pReceiveInterface,getListMainConnections(_)).WillOnce(DoAll(returnListConnections(), Return(E_ABORTED)));

    PyRun_SimpleStringFlags("import dbus\n"
            "f = open('/tmp/result.txt','w')\n"
            "bus = dbus.SessionBus()\n"
            "retVal=dbus.Bus().call_blocking(\n"
            "bus_name='org.genivi.audiomanager',\n"
            "object_path='/org/genivi/audiomanager/CommandInterface',\n"
            "dbus_interface='org.genivi.audiomanager.CommandInterface',\n"
            "method='GetListMainConnections',\n"
            "signature='',\n"
            "args=[],) \n"
            "f.write(str(retVal));\n"
            "f.close()", NULL);
    result = 0;
    ifs.open("/tmp/result.txt");
    while (std::getline(ifs, line))
    {
        //we could parse here, but this is the fastest way....
        ASSERT_EQ(line.compare("(dbus.Int16(9), dbus.Array([dbus.Struct((dbus.UInt16(15), dbus.UInt16(3), dbus.UInt16(4), dbus.Int16(34), dbus.Int16(0)), signature=None)], signature=dbus.Signature('(qqqnn)')))"), 0);
    }
    ifs.close();

    std::cout << "[listmainc ]" << std::endl;

    EXPECT_CALL(pReceiveInterface,getListMainSinks(_)).WillOnce(DoAll(returnListSinks(), Return(E_ABORTED)));

    PyRun_SimpleStringFlags("import dbus\n"
            "f = open('/tmp/result.txt','w')\n"
            "bus = dbus.SessionBus()\n"
            "retVal=dbus.Bus().call_blocking(\n"
            "bus_name='org.genivi.audiomanager',\n"
            "object_path='/org/genivi/audiomanager/CommandInterface',\n"
            "dbus_interface='org.genivi.audiomanager.CommandInterface',\n"
            "method='GetListMainSinks',\n"
            "signature='',\n"
            "args=[],) \n"
            "f.write(str(retVal));\n"
            "f.close()", NULL);
    result = 0;
    ifs.open("/tmp/result.txt");
    while (std::getline(ifs, line))
    {
        //we could parse here, but this is the fastest way....
        ASSERT_EQ(line.compare("(dbus.Int16(9), dbus.Array([dbus.Struct((dbus.UInt16(24), dbus.String(u'mySink'), dbus.Struct((dbus.Int16(2), dbus.Int16(3)), signature=None), dbus.Int16(124), dbus.Int16(2), dbus.UInt16(34)), signature=None)], signature=dbus.Signature('(qs(nn)nnq)')))"), 0);
    }
    ifs.close();

    std::cout << "[listsinks ]" << std::endl;

    EXPECT_CALL(pReceiveInterface,getListMainSources(_)).WillOnce(DoAll(returnListSources(), Return(E_ABORTED)));

    PyRun_SimpleStringFlags("import dbus\n"
            "f = open('/tmp/result.txt','w')\n"
            "bus = dbus.SessionBus()\n"
            "retVal=dbus.Bus().call_blocking(\n"
            "bus_name='org.genivi.audiomanager',\n"
            "object_path='/org/genivi/audiomanager/CommandInterface',\n"
            "dbus_interface='org.genivi.audiomanager.CommandInterface',\n"
            "method='GetListMainSources',\n"
            "signature='',\n"
            "args=[],) \n"
            "f.write(str(retVal));\n"
            "f.close()", NULL);
    result = 0;
    ifs.open("/tmp/result.txt");
    while (std::getline(ifs, line))
    {
        //we could parse here, but this is the fastest way....
        ASSERT_EQ(line.compare("(dbus.Int16(9), dbus.Array([dbus.Struct((dbus.UInt16(224), dbus.String(u'MySource'), dbus.Struct((dbus.Int16(3), dbus.Int16(2)), signature=None), dbus.UInt16(12)), signature=None), dbus.Struct((dbus.UInt16(22), dbus.String(u'NextSource'), dbus.Struct((dbus.Int16(3), dbus.Int16(2)), signature=None), dbus.UInt16(12)), signature=None)], signature=dbus.Signature('(qs(nn)q)')))"), 0);
    }
    ifs.close();

    std::cout << "[listsource]" << std::endl;

    EXPECT_CALL(pReceiveInterface,getListMainSinkSoundProperties(1,_)).WillOnce(DoAll(returnListMainSinkSoundProperties(), Return(E_ABORTED)));

    PyRun_SimpleStringFlags("import dbus\n"
            "f = open('/tmp/result.txt','w')\n"
            "bus = dbus.SessionBus()\n"
            "retVal=dbus.Bus().call_blocking(\n"
            "bus_name='org.genivi.audiomanager',\n"
            "object_path='/org/genivi/audiomanager/CommandInterface',\n"
            "dbus_interface='org.genivi.audiomanager.CommandInterface',\n"
            "method='GetListMainSinkSoundProperties',\n"
            "signature='q',\n"
            "args=[1],) \n"
            "f.write(str(retVal));\n"
            "f.close()", NULL);
    result = 0;
    ifs.open("/tmp/result.txt");
    while (std::getline(ifs, line))
    {
        //we could parse here, but this is the fastest way....
        ASSERT_EQ(line.compare("(dbus.Int16(9), dbus.Array([dbus.Struct((dbus.Int16(2), dbus.Int16(223)), signature=None), dbus.Struct((dbus.Int16(0), dbus.Int16(2)), signature=None)], signature=dbus.Signature('(nn)')))"), 0);
    }
    ifs.close();

    std::cout << "[lMainSiPro]" << std::endl;

    EXPECT_CALL(pReceiveInterface,getListMainSourceSoundProperties(1,_)).WillOnce(DoAll(returnListMainSinkSoundProperties(), Return(E_ABORTED)));

    PyRun_SimpleStringFlags("import dbus\n"
            "f = open('/tmp/result.txt','w')\n"
            "bus = dbus.SessionBus()\n"
            "retVal=dbus.Bus().call_blocking(\n"
            "bus_name='org.genivi.audiomanager',\n"
            "object_path='/org/genivi/audiomanager/CommandInterface',\n"
            "dbus_interface='org.genivi.audiomanager.CommandInterface',\n"
            "method='GetListMainSourceSoundProperties',\n"
            "signature='q',\n"
            "args=[1],) \n"
            "f.write(str(retVal));\n"
            "f.close()", NULL);
    result = 0;
    ifs.open("/tmp/result.txt");
    while (std::getline(ifs, line))
    {
        //we could parse here, but this is the fastest way....
        ASSERT_EQ(line.compare("(dbus.Int16(9), dbus.Array([dbus.Struct((dbus.Int16(2), dbus.Int16(223)), signature=None), dbus.Struct((dbus.Int16(0), dbus.Int16(2)), signature=None)], signature=dbus.Signature('(nn)')))"), 0);
    }
    ifs.close();

    std::cout << "[lMainSoPro]" << std::endl;

    EXPECT_CALL(pReceiveInterface,getListSourceClasses(_)).WillOnce(DoAll(returnListSourceClasses(), Return(E_ABORTED)));

    PyRun_SimpleStringFlags("import dbus\n"
            "f = open('/tmp/result.txt','w')\n"
            "bus = dbus.SessionBus()\n"
            "retVal=dbus.Bus().call_blocking(\n"
            "bus_name='org.genivi.audiomanager',\n"
            "object_path='/org/genivi/audiomanager/CommandInterface',\n"
            "dbus_interface='org.genivi.audiomanager.CommandInterface',\n"
            "method='GetListSourceClasses',\n"
            "signature='',\n"
            "args=[],) \n"
            "f.write(str(retVal));\n"
            "f.close()", NULL);
    result = 0;
    ifs.open("/tmp/result.txt");
    while (std::getline(ifs, line))
    {
        //we could parse here, but this is the fastest way....
        ASSERT_EQ(line.compare("(dbus.Int16(9), dbus.Array([dbus.Struct((dbus.UInt16(23), dbus.String(u'FirstCLass'), dbus.Array([dbus.Struct((dbus.Int16(2), dbus.Int16(12)), signature=None)], signature=dbus.Signature('(nn)'))), signature=None), dbus.Struct((dbus.UInt16(2), dbus.String(u'SecondCLass'), dbus.Array([dbus.Struct((dbus.Int16(2), dbus.Int16(12)), signature=None), dbus.Struct((dbus.Int16(2), dbus.Int16(12)), signature=None)], signature=dbus.Signature('(nn)'))), signature=None)], signature=dbus.Signature('(qsa(nn))')))"), 0);
    }
    ifs.close();

    std::cout << "[lSourceCla]" << std::endl;

    EXPECT_CALL(pReceiveInterface,getListSinkClasses(_)).WillOnce(DoAll(returnListSinkClasses(), Return(E_ABORTED)));

    PyRun_SimpleStringFlags("import dbus\n"
            "f = open('/tmp/result.txt','w')\n"
            "bus = dbus.SessionBus()\n"
            "retVal=dbus.Bus().call_blocking(\n"
            "bus_name='org.genivi.audiomanager',\n"
            "object_path='/org/genivi/audiomanager/CommandInterface',\n"
            "dbus_interface='org.genivi.audiomanager.CommandInterface',\n"
            "method='GetListSinkClasses',\n"
            "signature='',\n"
            "args=[],) \n"
            "f.write(str(retVal));\n"
            "f.close()", NULL);
    result = 0;
    ifs.open("/tmp/result.txt");
    while (std::getline(ifs, line))
    {
        //we could parse here, but this is the fastest way....
        ASSERT_EQ(line.compare("(dbus.Int16(9), dbus.Array([dbus.Struct((dbus.UInt16(2123), dbus.String(u'FirstCLass'), dbus.Array([dbus.Struct((dbus.Int16(1), dbus.Int16(122)), signature=None)], signature=dbus.Signature('(nn)'))), signature=None), dbus.Struct((dbus.UInt16(23), dbus.String(u'SecondCLass'), dbus.Array([dbus.Struct((dbus.Int16(1), dbus.Int16(122)), signature=None), dbus.Struct((dbus.Int16(1), dbus.Int16(122)), signature=None)], signature=dbus.Signature('(nn)'))), signature=None)], signature=dbus.Signature('(qsa(nn))')))"), 0);
    }
    ifs.close();

    std::cout << "[lSinkClass]" << std::endl;

    EXPECT_CALL(pReceiveInterface,getListSystemProperties(_)).WillOnce(DoAll(returnListSystemProperties(), Return(E_ABORTED)));

    PyRun_SimpleStringFlags("import dbus\n"
            "f = open('/tmp/result.txt','w')\n"
            "bus = dbus.SessionBus()\n"
            "retVal=dbus.Bus().call_blocking(\n"
            "bus_name='org.genivi.audiomanager',\n"
            "object_path='/org/genivi/audiomanager/CommandInterface',\n"
            "dbus_interface='org.genivi.audiomanager.CommandInterface',\n"
            "method='GetListSystemProperties',\n"
            "signature='',\n"
            "args=[],) \n"
            "f.write(str(retVal));\n"
            "f.close()", NULL);
    result = 0;
    ifs.open("/tmp/result.txt");
    while (std::getline(ifs, line))
    {
        //we could parse here, but this is the fastest way....
        ASSERT_EQ(line.compare("(dbus.Int16(9), dbus.Array([dbus.Struct((dbus.Int16(0), dbus.Int16(-2245)), signature=None)], signature=dbus.Signature('(nn)')))"), 0);
    }
    ifs.close();

    std::cout << "[lSysProp  ]" << std::endl;

    EXPECT_CALL(pReceiveInterface,getTimingInformation(2,_)).WillOnce(DoAll(returnTimingInfo(), Return(E_ABORTED)));

    PyRun_SimpleStringFlags("import dbus\n"
            "f = open('/tmp/result.txt','w')\n"
            "bus = dbus.SessionBus()\n"
            "retVal=dbus.Bus().call_blocking(\n"
            "bus_name='org.genivi.audiomanager',\n"
            "object_path='/org/genivi/audiomanager/CommandInterface',\n"
            "dbus_interface='org.genivi.audiomanager.CommandInterface',\n"
            "method='GetTimingInformation',\n"
            "signature='q',\n"
            "args=[2],) \n"
            "f.write(str(retVal));\n"
            "f.close()", NULL);
    result = 0;
    ifs.open("/tmp/result.txt");
    while (std::getline(ifs, line))
    {
        //we could parse here, but this is the fastest way....
        ASSERT_EQ(line.compare("(dbus.Int16(9), dbus.Int16(23))"), 0);
    }
    ifs.close();

    std::cout << "[timingInfo]" << std::endl;
    Py_Finalize();
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

