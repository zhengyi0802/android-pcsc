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
 
#define LOG_TAG "ISmartcardAPI"

#include "ISmartcardAPI.h"

#include <utils/Debug.h>
#include <utils/Log.h>
#include <binder/Parcel.h>
#include <utils/String8.h>

//#include <private/utils/Static.h>

#undef LOG_TAG
#define LOG_TAG "ifdhandler ISmartcardAPI"


namespace android {

// Proxy
class BpSmartcardAPI : public BpInterface<ISmartcardAPI>  
{
public:
    BpSmartcardAPI(const sp<IBinder>& impl)
        : BpInterface<ISmartcardAPI>(impl)
    {
    }


    virtual String16 getReaders(void)
    {
       ALOGI(" getReaders");
       Parcel data, reply;
       data.writeInterfaceToken(ISmartcardAPI::getInterfaceDescriptor());
       
	remote()->transact(transact_getReaders, data, &reply);
       //fail on exception
       	if (reply.readInt32() != 0) 
	{ 
		ALOGI("transact_getReaders returned error");
		return String16(""); 
	}
        return reply.readString16();
    }

    virtual bool isCardPresent(const String16& reader)
    {
	//ALOGI(" isCardPresent");	
    	Parcel data, reply;
        data.writeInterfaceToken(ISmartcardAPI::getInterfaceDescriptor());
        data.writeString16(reader);
        
	remote()->transact(transact_isCardPresent, data, &reply);
	if (reply.readInt32() != 0) 
	{
		ALOGI("transact_isCardPresent returned error");
		return false;
	}
		return reply.readInt32();
    }

    virtual long long int openBasicChannel(const String16& reader)
    {
	ALOGI(" openBasicChannel");	    	
	Parcel data, reply;
        data.writeInterfaceToken(ISmartcardAPI::getInterfaceDescriptor());
        data.writeString16(reader);

        remote()->transact(transact_openBasicChannel, data, &reply);
	if (reply.readInt32() != 0) 
	{
		ALOGI("transact_openBasicChannel returned error");
		return -1;
	}
	return reply.readInt64();
    }


    virtual long long int openBasicChannelAid(const String16& reader, const String16& aid)
    {
	ALOGI(" openBasicChannelAid");	    	
	Parcel data, reply;
        data.writeInterfaceToken(ISmartcardAPI::getInterfaceDescriptor());
        data.writeString16(reader);
	data.writeString16(aid);

        remote()->transact(transact_openBasicChannelAid, data, &reply);
	if (reply.readInt32() != 0) 
	{
		ALOGI("transact_openBasicChannelAid returned error");
		return -1;
	}
	return reply.readInt64();
    }
    
    virtual long long int openLogicalChannel(const String16& reader, const String16& aid)
    {
	ALOGI(" openLogicalChannel");    	
	Parcel data, reply;
        data.writeInterfaceToken(ISmartcardAPI::getInterfaceDescriptor());
        data.writeString16(reader);
        data.writeString16(aid);

        remote()->transact(transact_openLogicalChannel, data, &reply);
	if (reply.readInt32() != 0) 
	{
		ALOGI("error on transact_openLogicalChannel");
		return -1;
	}
		
	long long int handle = reply.readInt64();
	ALOGI(" retrieved handle: %lli", handle);
	return handle;
    }
    
    virtual String16 transmit(const long long int handle, const String16& command)
    {
	ALOGI(" transmit");      	
	Parcel data, reply;
        data.writeInterfaceToken(ISmartcardAPI::getInterfaceDescriptor());
        data.writeInt64(handle);
        data.writeString16(command);

        remote()->transact(transact_transmit, data, &reply);
	if (reply.readInt32() != 0) 
	{
		ALOGI("error on transact_transmit");
		return String16("error in transact_transmit");
	}
	return reply.readString16();
    }    

    
    virtual void closeChannel(const long long int handle)
    {
	ALOGI(" closeChannel");    	
	Parcel data, reply;
        data.writeInterfaceToken(ISmartcardAPI::getInterfaceDescriptor());
        data.writeInt64(handle);
        
        remote()->transact(transact_closeChannel, data, &reply);
	if (reply.readInt32() != 0) 
	{
		ALOGI("error on transact_closeChannel");
		return;
	}
    }    


    virtual String16 getLastError()
    {
	//ALOGI(" getLastError");      	
	Parcel data, reply;
        data.writeInterfaceToken(ISmartcardAPI::getInterfaceDescriptor());
 
        remote()->transact(transact_getLastError, data, &reply);
	if (reply.readInt32() != 0) 
	{
		ALOGI("error on transact_getLastError");
		return String16("error in getLastError");
	}
	return reply.readString16();
    }    

};

IMPLEMENT_META_INTERFACE(SmartcardAPI,"org.simalliance.openmobileapi.service.ISmartcardSystemService"); 
//IMPLEMENT_META_INTERFACE(SmartcardAPI,"org.simalliance.openmobileapi.service.ISmartcardService"); 

}; // namespace android


