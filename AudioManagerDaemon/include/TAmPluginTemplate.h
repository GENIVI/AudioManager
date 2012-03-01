/**
 * Copyright (C) 2011, BMW AG
 *
 * GeniviAudioMananger AudioManagerDaemon
 *
 * \file TAmPluginTemplate.h
 *
 * \date 20-Oct-2011 3:42:04 PM
 * \author Christian Mueller (christian.ei.mueller@bmw.de)
 *
 * \section License
 * GNU Lesser General Public License, version 2.1, with special exception (GENIVI clause)
 * Copyright (C) 2011, BMW AG Christian Mueller  Christian.ei.mueller@bmw.de
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License, version 2.1, for more details.
 * You should have received a copy of the GNU Lesser General Public License, version 2.1, along with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 * Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may also be applicable to programs even in cases in which the program is not a library in the technical sense.
 * Linking AudioManager statically or dynamically with other modules is making a combined work based on AudioManager. You may license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to license your linked modules under the GNU Lesser General Public License, version 2.1, you may use the program under the following exception.
 * As a special exception, the copyright holders of AudioManager give you permission to combine AudioManager with software programs or libraries that are released under any license unless such a combination is not permitted by the license of such a software program or library. You may copy and distribute such a system following the terms of the GNU Lesser General Public License, version 2.1, including this special exception, for AudioManager and the licenses of the other code concerned.
 * Note that people who make modified versions of AudioManager are not obligated to grant this special exception for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, version 2.1, gives permission to release a modified version without this exception; this exception also makes it possible to release a modified version which carries forward this exception.
 *
 */

#ifndef PLUGINTEMPLATE_H_
#define PLUGINTEMPLATE_H_

#include <dlfcn.h>
#include <libgen.h>
#include "shared/CAmDltWrapper.h"

namespace am
{

/**
 * This template tries to load a library and cast ot to a class
 * @param libname the full path to the library to be loaded
 * @return returns the pointer to the class to be loaded
 */
template<class T> T* getCreateFunction(const std::string& libname, void*& libraryHandle)
{

    logInfo("getCreateFunction : Trying to load library with name: ",libname);

    // cut off directories
    char* fileWithPath = const_cast<char*>(libname.c_str());
    std::string libFileName = basename(fileWithPath);

    // cut off "lib" in front and cut off .so end"
    std::string createFunctionName = libFileName.substr(3, libFileName.length() - 6) + "Factory";

    // open library
    dlerror(); // Clear any existing error
    libraryHandle = dlopen(libname.c_str(), RTLD_LAZY);
    const char* dlopen_error = dlerror();
    if (!libraryHandle || dlopen_error)
    {
        logError("getCreateFunction : dlopen failed",dlopen_error);
        return 0;
    }

    // get entry point from shared lib
    dlerror(); // Clear any existing error

    union
    {
        void* voidPointer;
        T* typedPointer;
    } functionPointer;

    // Note: direct cast is not allowed by ISO C++. e.g.
    // T* createFunction = reinterpret_cast<T*>(dlsym(libraryHandle, createFunctionName.c_str()));
    // compiler warning: "forbids casting between pointer-to-function and pointer-to-object"

    functionPointer.voidPointer = dlsym(libraryHandle, createFunctionName.c_str());
    T* createFunction = functionPointer.typedPointer;

    const char* dlsym_error = dlerror();
    if (!createFunction || dlsym_error)
    {
        logError("getCreateFunction: Failed to load shared lib entry point",dlsym_error);
    }
    else
    {
        logInfo("getCreateFunction : loaded successfully plugin", createFunctionName);
    }
    return createFunction;
}

}

#endif /* PLUGINTEMPLATE_H_ */
