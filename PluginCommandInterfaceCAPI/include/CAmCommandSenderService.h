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

#include <org/genivi/am/CommandControlStubDefault.h>
#include "../../include/command/IAmCommandReceive.h"

namespace am {

/**
 * A concrete stub implementation used by the command sender plug-in.
 */
class CAmCommandSenderService: public org::genivi::am::CommandControlStubDefault {
	IAmCommandReceive* mpIAmCommandReceive;
public:
	CAmCommandSenderService();
	CAmCommandSenderService(IAmCommandReceive *aReceiver);
	virtual ~CAmCommandSenderService();

		virtual void connect(org::genivi::am::am_sourceID_t sourceID, org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_mainConnectionID_t& mainConnectionID, org::genivi::am::am_Error_e& result);

	    virtual void disconnect(org::genivi::am::am_mainConnectionID_t mainConnectionID, org::genivi::am::am_Error_e& result);

	    virtual void setVolume(org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_mainVolume_t volume, org::genivi::am::am_Error_e& result);

	    virtual void volumeStep(org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_mainVolume_t volumeStep, org::genivi::am::am_Error_e& result);

	    virtual void setSinkMuteState(org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_MuteState_e muteState, org::genivi::am::am_Error_e& result);

	    virtual void setMainSinkSoundProperty(org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_MainSoundProperty_s soundProperty, org::genivi::am::am_Error_e& result);

	    virtual void setMainSourceSoundProperty(org::genivi::am::am_sourceID_t sourceID, org::genivi::am::am_MainSoundProperty_s soundProperty, org::genivi::am::am_Error_e& result);

	    virtual void setSystemProperty(org::genivi::am::am_SystemProperty_s soundProperty, org::genivi::am::am_Error_e& result);

	    virtual void getListMainConnections(org::genivi::am::am_Error_e& result, org::genivi::am::am_MainConnection_L& listConnections);

	    virtual void getListMainSinks(org::genivi::am::am_SinkType_L& listMainSinks, org::genivi::am::am_Error_e& result);

	    virtual void getListMainSources(org::genivi::am::am_SourceType_L& listMainSources, org::genivi::am::am_Error_e& result);

	    virtual void getListMainSinkSoundProperties(org::genivi::am::am_sinkID_t sinkID, org::genivi::am::am_MainSoundProperty_L& listSoundProperties, org::genivi::am::am_Error_e& result);

	    virtual void getListMainSourceSoundProperties(org::genivi::am::am_sourceID_t sourceID, org::genivi::am::am_MainSoundProperty_L& listSourceProperties, org::genivi::am::am_Error_e& result);

	    virtual void getListSourceClasses( org::genivi::am::am_SourceClass_L& listSourceClasses,org::genivi::am::am_Error_e& result);

	    virtual void getListSinkClasses( org::genivi::am::am_SinkClass_L& listSinkClasses,org::genivi::am::am_Error_e& result);

	    virtual void getListSystemProperties(org::genivi::am::am_SystemProperty_L& listSystemProperties,org::genivi::am::am_Error_e& result);

	    virtual void getTimingInformation(org::genivi::am::am_mainConnectionID_t mainConnectionID, org::genivi::am::am_timeSync_t& delay,org::genivi::am::am_Error_e& result);
};

} /* namespace am */
#endif /* CAMCOMMANDSENDERSERVICE_H_ */
