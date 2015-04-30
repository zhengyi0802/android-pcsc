/*
 * Copyright (C) 2010 Giesecke & Devrient GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <binder/IInterface.h>

namespace android {

class ISmartcardAPI : public IInterface
{
public:
    DECLARE_META_INTERFACE(SmartcardAPI);    
    virtual String16 getReaders(void) = 0;
    virtual bool isCardPresent(const String16& reader) = 0;
    virtual long long int openBasicChannel(const String16& reader) = 0;
    virtual long long int openBasicChannelAid(const String16& reader, const String16& aid) = 0;
    virtual long long int openLogicalChannel(const String16& reader, const String16& aid) = 0;
    virtual String16 transmit(const long long int handle, const String16& command) = 0;
    virtual void closeChannel(const long long int handle) = 0;
    virtual String16 getLastError(void) = 0;


    enum {
		transact_closeChannel = IBinder::FIRST_CALL_TRANSACTION + 0,		
		transact_getReaders = IBinder::FIRST_CALL_TRANSACTION + 1,
		transact_isCardPresent = IBinder::FIRST_CALL_TRANSACTION + 2,
		transact_openBasicChannel = IBinder::FIRST_CALL_TRANSACTION + 3,
		transact_openBasicChannelAid = IBinder::FIRST_CALL_TRANSACTION + 4,
		transact_openLogicalChannel = IBinder::FIRST_CALL_TRANSACTION + 5,
		transact_transmit = IBinder::FIRST_CALL_TRANSACTION + 6,
		transact_getLastError = IBinder::FIRST_CALL_TRANSACTION + 7
	};
}; //class

}; //namespace


