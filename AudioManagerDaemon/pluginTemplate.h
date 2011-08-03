/*
 * pluginTemplate.h
 *
 *  Created on: Jul 27, 2011
 *      Author: christian
 */

#ifndef PLUGINTEMPLATE_H_
#define PLUGINTEMPLATE_H_

#include <iostream>
#include <stdio.h>
#include <dirent.h>
#include <dlfcn.h>
#include <libgen.h>
#include <unistd.h>
#include <string>

#include "audioManagerIncludes.h"

template<class T>T* getCreateFunction(std::string libname) {

	// cut off directories
	char* fileWithPath = const_cast<char*>(libname.c_str());
	std::string libFileName = basename(fileWithPath);

	// cut off "lib" in front and cut off .so end"
	std::string createFunctionName = libFileName.substr(3, libFileName.length() - 6) + "Factory";
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Lib entry point name "),DLT_STRING(createFunctionName.c_str()));

	// open library
	void *libraryHandle;
	dlerror(); // Clear any existing error
	libraryHandle = dlopen(libname.c_str(), RTLD_NOW /*LAZY*/);
	const char* dlopen_error = dlerror();
	if (!libraryHandle || dlopen_error)
	{
		DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("dlopen failed"),DLT_STRING(dlopen_error));
		return 0;
	}

	// get entry point from shared lib
	dlerror(); // Clear any existing error
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("loading external function with name"),DLT_STRING(createFunctionName.c_str()));

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
		DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Failed to load shared lib entry point"),DLT_STRING(dlsym_error));
	}

	return createFunction;
}


#endif /* PLUGINTEMPLATE_H_ */
