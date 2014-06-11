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

#ifndef CAMROUTINGSENDERCOMMON_H_
#define CAMROUTINGSENDERCOMMON_H_

#include <memory>
#include "audiomanagertypes.h"
#include <org/genivi/am.h>

using namespace am;
using namespace CommonAPI;

/**
 * Utility functions
 */
extern void CAmConvertCAPI2AM(const org::genivi::am::am_Domain_s &, am_Domain_s &);
extern void CAmConvertCAPI2AM(const org::genivi::am::am_Source_s & , am::am_Source_s & );
extern void CAmConvertCAPI2AM(const org::genivi::am::am_Sink_s & , am::am_Sink_s & );
extern void CAmConvertCAPI2AM(const org::genivi::am::am_Availability_s  & ,  am_Availability_s & );
extern void CAmConvertCAPI2AM(const org::genivi::am::am_SoundProperty_s &, am::am_SoundProperty_s &);
extern void CAmConvertCAPI2AM(const org::genivi::am::am_MainSoundProperty_s &, am::am_MainSoundProperty_s & );
extern void CAmConvertCAPI2AM(const org::genivi::am::am_NotificationConfiguration_s & , am::am_NotificationConfiguration_s & );
extern void CAmConvertCAPI2AM(const org::genivi::am::am_Gateway_s & , am::am_Gateway_s & );
extern void CAmConvertCAPI2AM(const org::genivi::am::am_Crossfader_s & , am::am_Crossfader_s & );
extern void CAmConvertCAPI2AM(const org::genivi::am::am_EarlyData_s & , am::am_EarlyData_s & );
extern void CAmConvertCAPI2AM(const org::genivi::am::am_EarlyData_u &, am::am_EarlyData_u & );
extern void CAmConvertCAPI2AM(const org::genivi::am::am_DataType_u & , am::am_DataType_u & );
extern void CAmConvertCAPI2AM(const org::genivi::am::am_Volumes_s &, am::am_Volumes_s &);
extern void CAmConvertCAPI2AM(const org::genivi::am::am_Handle_s &, am::am_Handle_s &);
extern void CAmConvertCAPI2AM(const org::genivi::am::am_NotificationPayload_s & , am::am_NotificationPayload_s & );
extern void CAmConvertCAPIVector2AM(const std::vector<org::genivi::am::am_NotificationConfiguration_s> & , std::vector<am::am_NotificationConfiguration_s> & );
extern void CAmConvertCAPIVector2AM(const std::vector<org::genivi::am::am_SoundProperty_s> &, std::vector<am::am_SoundProperty_s> &);
extern void CAmConvertCAPIVector2AM(const std::vector<org::genivi::am::am_CustomConnectionFormat_t> &, std::vector<am::am_CustomConnectionFormat_t> & );
extern void CAmConvertCAPIVector2AM(const std::vector<org::genivi::am::am_MainSoundProperty_s> &, std::vector<am::am_MainSoundProperty_s> & );
extern void CAmConvertCAPIVector2AM(const std::vector<org::genivi::am::am_Volumes_s> &, std::vector<am::am_Volumes_s> & );
extern void CAmConvertCAPIVector2AM(const org::genivi::am::am_EarlyData_L &, std::vector<am::am_EarlyData_s> &);

extern void CAmConvertAM2CAPI(const am_Availability_s & ,  org::genivi::am::am_Availability_s & );
extern void CAmConvertAM2CAPI(const am::am_SoundProperty_s &, org::genivi::am::am_SoundProperty_s &);
extern void CAmConvertAM2CAPI(const am::am_NotificationConfiguration_s &, org::genivi::am::am_NotificationConfiguration_s &);
extern void CAmConvertAM2CAPI(const am::am_Volumes_s &, org::genivi::am::am_Volumes_s &);
extern void CAmConvertAMVector2CAPI(const std::vector<am::am_SoundProperty_s> &, std::vector<org::genivi::am::am_SoundProperty_s> &);
extern void CAmConvertAMVector2CAPI(const std::vector<am::am_Volumes_s> & , std::vector<org::genivi::am::am_Volumes_s> & );
extern void CAmConvertAM2CAPI(const am::am_Handle_s &, org::genivi::am::am_Handle_s &);

#endif /* CAMROUTINGSENDERCOMMON_H_ */
