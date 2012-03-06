/**
 * Copyright (C) 2012, BMW AG
 *
 * This file is part of GENIVI Project AudioManager.
 *
 * Contributions are licensed to the GENIVI Alliance under one or more
 * Contribution License Agreements.
 *
 * \copyright
 * This Source Code Form is subject to the terms of the
 * Mozilla Public License, v. 2.0. If a  copy of the MPL was not distributed with
 * this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 *
 * \author Christian Mueller, christian.ei.mueller@bmw.de BMW 2011,2012
 *
 * \file TAmPluginTemplate.h
 * For further information see http://www.genivi.org/.
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
 *  * This template tries to load a library and cast to a class
 * @param libname the full path to the library to be loaded
 * @param libraryHandle the handle to the library that gets returned
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
        return (0);
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
    return (createFunction);
}

}

#endif /* PLUGINTEMPLATE_H_ */
