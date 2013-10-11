/*
* This file was generated by the CommonAPI Generators. 
* Used org.genivi.commonapi.core 2.1.2.201309301424.
* Used org.franca.core 0.8.9.201308271211.
*
*  Copyright (c) 2012 BMW
*  
*   \author Aleksandar Donchev, aleksander.donchev@partner.bmw.de BMW 2013
*  
*   \copyright
*   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction,
*   including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
*   subject to the following conditions:
*   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
*   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
*   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
*   THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*    For further information see http://www.genivi.org/.
*  
*/
/**
 * The interface towards the Controlling Instance (e.g HMI). It handles the
 *  communication towards the HMI and other system components who need to interact
 *  with the audiomanagement.
There are two rules that have to be kept in mind
 *  when implementing against this interface:
 * @author Christian Mueller
 */
#ifndef ORG_GENIVI_AM_Command_Control_H_
#define ORG_GENIVI_AM_Command_Control_H_



#if !defined (COMMONAPI_INTERNAL_COMPILATION)
#define COMMONAPI_INTERNAL_COMPILATION
#endif

#include <CommonAPI/types.h>

#undef COMMONAPI_INTERNAL_COMPILATION

namespace org {
namespace genivi {
namespace am {

class CommandControl {
 public:
    virtual ~CommandControl() { }

    static inline const char* getInterfaceId();
    static inline CommonAPI::Version getInterfaceVersion();
};

const char* CommandControl::getInterfaceId() {
    static const char* interfaceId = "org.genivi.am.CommandControl";
    return interfaceId;
}

CommonAPI::Version CommandControl::getInterfaceVersion() {
    return CommonAPI::Version(1, 0);
}


} // namespace am
} // namespace genivi
} // namespace org

namespace CommonAPI {

}


namespace std {
    //hashes for types
    
    //hashes for error types
}

#endif // ORG_GENIVI_AM_Command_Control_H_
