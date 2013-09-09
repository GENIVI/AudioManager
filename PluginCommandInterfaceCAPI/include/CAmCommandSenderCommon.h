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

#ifndef CAMCOMMANDSENDERCOMMON_H_
#define CAMCOMMANDSENDERCOMMON_H_

#include <memory>
#include "audiomanagertypes.h"
#include <org/genivi/am.h>

using namespace am;

/**
 * The following functions convert the basics AudiomManager types from/to CommonAPI types.
 */

extern void CAmConvertAvailablility(const am_Availability_s & , org::genivi::am::am_Availability_s & );
extern void CAmConvertMainSoundProperty(const am_MainSoundProperty_s & , org::genivi::am::am_MainSoundProperty_s & );
extern void CAmConvertSystemProperty(const am_SystemProperty_s &, org::genivi::am::am_SystemProperty_s &);

extern org::genivi::am::am_ClassProperty_pe CAmConvert2CAPIType(const am_ClassProperty_e & property);
extern org::genivi::am::am_SystemPropertyType_pe CAmConvert2CAPIType(const am_SystemPropertyType_e &);
extern org::genivi::am::am_Availability_e CAmConvert2CAPIType(const am_Availability_e & );
extern org::genivi::am::am_AvailabilityReason_pe CAmConvert2CAPIType(const am_AvailabilityReason_e & );

extern org::genivi::am::am_MuteState_e CAmConvert2CAPIType(const am_MuteState_e &);
extern am_MuteState_e CAmConvertFromCAPIType(const org::genivi::am::am_MuteState_e &);

extern org::genivi::am::am_MainSoundPropertyType_pe CAmConvert2CAPIType(const am_MainSoundPropertyType_e &);

extern org::genivi::am::am_ConnectionState_e CAmConvert2CAPIType(const am_ConnectionState_e &);
extern am_ConnectionState_e CAmConvertFromCAPIType(const org::genivi::am::am_ConnectionState_e &);

extern am_NotificationType_e CAmConvert2CAPIType(const org::genivi::am::am_NotificationType_pe &);
extern org::genivi::am::am_NotificationType_pe CAmConvertFromCAPIType(const am_NotificationType_e &);

extern am_NotificationStatus_e CAmConvert2CAPIType(const org::genivi::am::am_NotificationStatus_e &);
extern org::genivi::am::am_NotificationStatus_e CAmConvertFromCAPIType(const am_NotificationStatus_e &);

extern org::genivi::am::am_Error_e CAmConvert2CAPIType(const am_Error_e &);
extern am_Error_e CAmConvertFromCAPIType(const org::genivi::am::am_Error_e & error);

#endif /* CAMCOMMANDSENDERCOMMON_H_ */
