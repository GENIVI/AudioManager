/**
 *  Copyright (c) copyright 2011-2012 Aricent® Group  and its licensors
 *
 *  \author: Sampreeth Ramavana
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

#ifndef _CAMDBUSSEND_H_
#define _CAMDBUSSEND_H_

//#include "headers.h"
#include <dbus/dbus.h>

class CAmDbusSend {
public:
        CAmDbusSend(DBusConnection* conn, const char* bus_name,const char* path, const char* interface, const char* method);
        virtual ~CAmDbusSend();
	void appendString(char* string);
	void appendInteger(int integer);
	void sendReply(bool* reply);
	void sendReply(int* reply);
        void sendReply(void);
        void Replyint32(int *reply);

private:
	DBusMessage* m_msg;
	DBusMessageIter m_args;
	DBusConnection* m_conn;
        DBusMessage* replymsg;
        //DBusMessageIter args;
        DBusMessageIter mDBusMessageIter;
};

#endif /* _CAMDBUSSEND_H_ */
