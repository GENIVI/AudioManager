/**
 *  Copyright (c) 2012 BMW
 *
 *  \author Aleksandar Donchev, aleksander.donchev@partner.bmw.de BMW 2013
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

#ifndef CAMCOMMANDSENDERSERVICE_H_
#define CAMCOMMANDSENDERSERVICE_H_

#include <org/genivi/audiomanager/CommandInterfaceStubDefault.h>
#include "../../include/command/IAmCommandReceive.h"

namespace am {

using namespace org::genivi::audiomanager;

/**
 * A concrete stub implementation used by the command sender plug-in.
 */
class CAmCommandSenderService: public CommandInterfaceStubDefault {
	IAmCommandReceive* mpIAmCommandReceive;
public:
	CAmCommandSenderService();
	CAmCommandSenderService(IAmCommandReceive *aReceiver);
	virtual ~CAmCommandSenderService();

		virtual void Connect(CommandInterface::am_sourceID_t sourceID, CommandInterface::am_sinkID_t sinkID, CommandInterface::am_Error_e& result, CommandInterface::am_mainConnectionID_t& mainConnectionID);

	    virtual void Disconnect(CommandInterface::am_mainConnectionID_t mainConnectionID, CommandInterface::am_Error_e& result);

	    virtual void SetVolume(CommandInterface::am_sinkID_t sinkID, CommandInterface::am_mainVolume_t volume, CommandInterface::am_Error_e& result);

	    virtual void VolumeStep(CommandInterface::am_sinkID_t sinkID, CommandInterface::am_mainVolume_t volumeStep, CommandInterface::am_Error_e& result);

	    virtual void SetSinkMuteState(CommandInterface::am_sinkID_t sinkID, CommandInterface::am_MuteState_e muteState, CommandInterface::am_Error_e& result);

	    virtual void SetMainSinkSoundProperty(CommandInterface::am_sinkID_t sinkID, CommandInterface::am_MainSoundProperty_s soundProperty, CommandInterface::am_Error_e& result);

	    virtual void SetMainSourceSoundProperty(CommandInterface::am_sourceID_t sourceID, CommandInterface::am_MainSoundProperty_s soundProperty, CommandInterface::am_Error_e& result);

	    virtual void SetSystemProperty(CommandInterface::am_SystemProperty_s soundProperty, CommandInterface::am_Error_e& result);

	    virtual void GetListMainConnections(CommandInterface::am_Error_e& result, CommandInterface::am_MainConnectionType_l& listConnections);

	    virtual void GetListMainSinks(CommandInterface::am_Error_e& result, CommandInterface::am_SinkType_l& listMainSinks);

	    virtual void GetListMainSources(CommandInterface::am_Error_e& result, CommandInterface::am_SourceType_l& listMainSources);

	    virtual void GetListMainSinkSoundProperties(CommandInterface::am_sinkID_t sinkID, CommandInterface::am_Error_e& result, CommandInterface::am_MainSoundProperty_l& listSoundProperties);

	    virtual void GetListMainSourceSoundProperties(CommandInterface::am_sourceID_t sourceID, CommandInterface::am_Error_e& result, CommandInterface::am_MainSoundProperty_l& listSourceProperties);

	    virtual void GetListSourceClasses(CommandInterface::am_Error_e& result, CommandInterface::am_SourceClass_l& listSourceClasses);

	    virtual void GetListSinkClasses(CommandInterface::am_Error_e& result, CommandInterface::am_SinkClass_l& listSinkClasses);

	    virtual void GetListSystemProperties(CommandInterface::am_Error_e& result, CommandInterface::am_SystemProperty_l& listSystemProperties);

	    virtual void GetTimingInformation(CommandInterface::am_mainConnectionID_t mainConnectionID, CommandInterface::am_Error_e& result, CommandInterface::am_timeSync_t& delay);
};

} /* namespace am */
#endif /* CAMCOMMANDSENDERSERVICE_H_ */
