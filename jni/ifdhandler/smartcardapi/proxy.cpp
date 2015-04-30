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

#include <binder/IServiceManager.h>
#include <utils/Log.h>
#include <sys/stat.h>
#include "ISmartcardAPI.h"

#undef LOG_TAG
#define LOG_TAG "ifdhandler proxy"

using namespace android;

static sp<ISmartcardAPI> gSmartcardAPI = 0; 

typedef struct 
{
	unsigned long lun;
	unsigned long channel; //pcsc channel id
	long long int handle;  //handle for basic channel
	long long int handle1; //handle for logical channel '1'
	long long int handle2;
	long long int handle3;
	String16 readername;
	bool bValidHandle;     //handle for basic channel successfully retrieved from SmartcardAPI       
	bool bValidHandle1;        
	bool bValidHandle2;        
	bool bValidHandle3;   
	bool bLogicalChannelRequested0;	    //always true for basic channel
	bool bLogicalChannelRequested1;     //true, if logical channel requested with MANAGE CHANNEL OPEN '00 70 00 ..'       
	bool bLogicalChannelRequested2; 
	bool bLogicalChannelRequested3;     
} channel_entry;

Vector<channel_entry> readerlist;

char convert_hex_digit_to_char(unsigned char value, int lowercase);
String16 convert_buffer_to_string16(unsigned char* buffer, int length);
unsigned char convert_char_to_hex_digit(char c);
unsigned char* convert_string16_to_buffer(String16 str16, int* outlen);
unsigned char* convert_string16_to_string8(String16 str16, int* outlen);

extern "C"
void proxy_readerlist_addchannel (unsigned long Lun, unsigned long Channel ) 
{
	//LOGI("proxy_readerlist_addchannel - Lun: %lu, channel %lu",Lun, Channel);
	bool bExist = false;	
	for (int i=0; i<readerlist.size();++i)
	{
		if (readerlist[i].lun == Lun)
		{
			readerlist.editItemAt(i).channel = Channel;	
			readerlist.editItemAt(i).bValidHandle = false;
			readerlist.editItemAt(i).bValidHandle1 = false;
			readerlist.editItemAt(i).bValidHandle2 = false;
			readerlist.editItemAt(i).bValidHandle3 = false;
			readerlist.editItemAt(i).handle = 0;
			readerlist.editItemAt(i).handle1 = 0;
			readerlist.editItemAt(i).handle2 = 0;
			readerlist.editItemAt(i).handle3 = 0;
			readerlist.editItemAt(i).readername = String16("");
			readerlist.editItemAt(i).bLogicalChannelRequested0 = true;  //true by default, as there is no need to call MANAGE CHANNEL OPEN for basic channel
			readerlist.editItemAt(i).bLogicalChannelRequested1 = false;
			readerlist.editItemAt(i).bLogicalChannelRequested2 = false;
			readerlist.editItemAt(i).bLogicalChannelRequested3 = false;
			bExist = true;
		}
	}
	if (!bExist) 
	{
		channel_entry entry;
		entry.lun = Lun;
		entry.channel = Channel;
		entry.handle = 0;
		entry.handle1 = 0;
		entry.handle2 = 0;
		entry.handle3 = 0;
		entry.readername = String16("");
		entry.bValidHandle = false;
		entry.bValidHandle1 = false;
		entry.bValidHandle2 = false;
		entry.bValidHandle3 = false;
		entry.bLogicalChannelRequested0 = true;		
		entry.bLogicalChannelRequested1 = false;
		entry.bLogicalChannelRequested2 = false;
		entry.bLogicalChannelRequested3 = false;
		readerlist.push(entry);
	}
		
}	


void proxy_readerlist_addhandle (unsigned long Lun, long long int handle, int iLogicalChannel ) 
{
	//LOGI("proxy_readerlist_addhandle - Lun %lu, handle: %lli, logical channel: %i",Lun, handle, iLogicalChannel);			
	for (int i=0; i<readerlist.size();++i)
	{
		if (readerlist[i].lun == Lun)
		{
			switch 	(iLogicalChannel)
			{
				case 0:
				readerlist.editItemAt(i).handle = handle;
				readerlist.editItemAt(i).bValidHandle = true;
				break;

				case 1:
				readerlist.editItemAt(i).handle1 = handle;
				readerlist.editItemAt(i).bValidHandle1 = true;
				break;

				case 2:
				readerlist.editItemAt(i).handle2 = handle;
				readerlist.editItemAt(i).bValidHandle2 = true;
				break;

				case 3:
				readerlist.editItemAt(i).handle3 = handle;
				readerlist.editItemAt(i).bValidHandle3 = true;
				break;

				default: break;

			}

		}
	}
}	



void proxy_readerlist_invalidateHandle (unsigned long Lun, int iLogicalChannel ) 
{
	//LOGI("proxy_readerlist_invalidateHandle, Lun: %lu, log. channel: %i",Lun,iLogicalChannel);		
	for (int i=0; i<readerlist.size();++i)
	{
		if (readerlist[i].lun == Lun)
		{
			switch 	(iLogicalChannel)
			{
				case 0:
				readerlist.editItemAt(i).bValidHandle = false;
				break;

				case 1:
				readerlist.editItemAt(i).bValidHandle1 = false;
				readerlist.editItemAt(i).bLogicalChannelRequested1 = false;
				break;

				case 2:
				readerlist.editItemAt(i).bValidHandle2 = false;
				readerlist.editItemAt(i).bLogicalChannelRequested2 = false;
				break;

				case 3:
				readerlist.editItemAt(i).bValidHandle3 = false;
				readerlist.editItemAt(i).bLogicalChannelRequested3 = false;
				break;

				default: break;

			}

		}
	}

}


void proxy_readerlist_removeChannelEntry(unsigned long Lun)
{
	//LOGI("proxy_readerlist_removeChannelEntry Lun: %lu",Lun);		
	for (int i=0; i<readerlist.size();++i)
	{
		if (readerlist[i].lun == Lun)
		{
			readerlist.removeAt(i);
		}
	}
}
	


/*
*get handle for specific channel from list
*/
long long int proxy_readerlist_gethandle (unsigned long Lun, int iLogChannel)
{
	//LOGI("proxy_readerlist_gethandle - Lun %lu, logical channel %i", Lun,iLogChannel);	
	for (int i=0; i<readerlist.size();++i)
	{
		if (readerlist[i].lun == Lun)
		{
			long long int handle = -1;			
			switch (iLogChannel)
			{
				case 0:
				handle =  readerlist[i].handle;
				break;
				
				case 1:
				handle = readerlist[i].handle1;
				break;

				case 2:
				handle = readerlist[i].handle2;
				break;
				

				case 3:
				handle = readerlist[i].handle3;
				break;

				default: break;
			}
			//LOGI("proxy_readerlist_gethandle - Lun %lu, handle from list: %lli ",Lun, handle);	
			return handle;
		}
	}
	//LOGI("proxy_readerlist_gethandle - handle not in list");
	return -1;
}

bool proxy_readerlist_isLogicalChannelOpen (unsigned long Lun, int iLogChannel)
{
	//LOGI("proxy_readerlist_isLogicalChannelOpen Lun: %lu, logical channel: %i", Lun,iLogChannel);	
	bool bChannelOpen = false;		
	for (int i=0; i<readerlist.size();++i)
	{
		if (readerlist[i].lun == Lun)
		{
			switch (iLogChannel)
			{
				
				case 0:
				bChannelOpen = readerlist[i].bLogicalChannelRequested0;
				break;

				case 1:
				bChannelOpen = readerlist[i].bLogicalChannelRequested1;
				break;
				
				case 2:
				bChannelOpen = readerlist[i].bLogicalChannelRequested2;
				break;

				case 3:
				bChannelOpen = readerlist[i].bLogicalChannelRequested3;
				break;
				
				default: break;
			}
			
		}
	}
	//LOGI("proxy_readerlist_isLogicalChannelOpen - Lun %lu, channel open: %s", Lun, bChannelOpen ? "true" : "false");	
	return bChannelOpen;
}


void proxy_readerlist_setLogicalChannelOpen (unsigned long Lun, int iLogChannel)
{
	//LOGI("proxy_readerlist_setLogicalChannelOpen Lun: %lu, logical channel: %i", Lun,iLogChannel);	
	for (int i=0; i<readerlist.size();++i)
	{
		if (readerlist[i].lun == Lun)
		{
			switch (iLogChannel)
			{
				case 1:
				readerlist.editItemAt(i).bLogicalChannelRequested1 = true;
				break;
				
				case 2:
				readerlist.editItemAt(i).bLogicalChannelRequested2 = true;
				break;

				case 3:
				readerlist.editItemAt(i).bLogicalChannelRequested3 = true;
				break;
				
				default: break;
			}
			
		}
	}
}


bool proxy_readerlist_hasValidHandle (unsigned long Lun, int iLogChannel)
{
	//LOGI("proxy_readerlist_hasValidHandle Lun: %lu, logical channel: %i", Lun,iLogChannel);	
	bool bHandleIsValid = false;		
	for (int i=0; i<readerlist.size();++i)
	{
		if (readerlist[i].lun == Lun)
		{
			switch (iLogChannel)
			{
				case 0:
				bHandleIsValid =  readerlist[i].bValidHandle;
				break;
				
				case 1:
				bHandleIsValid = readerlist[i].bValidHandle1;
				break;

				case 2:
				bHandleIsValid = readerlist[i].bValidHandle2;
				break;
				
				case 3:
				bHandleIsValid = readerlist[i].bValidHandle3;
				break;

				default: break;
			}
			
		}

	}
	//LOGI("proxy_readerlist_hasValidHandle - Lun %lu, handle valid: %s", Lun, bHandleIsValid ? "true" : "false");	
	return bHandleIsValid;
}



long int proxy_readerlist_getchannel (unsigned long Lun)
{
	//LOGI("proxy_readerlist_getchannel- Lun %lu", Lun);
	long int channel = -1;	

	for (int i=0; i<readerlist.size();++i)
	{
		if (readerlist[i].lun == Lun)
		{
			channel = readerlist[i].channel;
		}
	}
	return channel;
}


int proxy_readerlist_RequestLogicalChannel (unsigned long Lun) 
{
	int iFreeLogicalChannel = -1;	
	//LOGI("proxy_readerlist_RequestLogicalChannel - Lun %lu",Lun);			
	for (int i=0; i<readerlist.size();++i)
	{
		if (readerlist[i].lun == Lun)
		{
			if (readerlist[i].bLogicalChannelRequested1 == false)
			{
				iFreeLogicalChannel = 1;				
				readerlist.editItemAt(i).bLogicalChannelRequested1 = true;
				break;
			}
			else if (readerlist[i].bLogicalChannelRequested2 == false)
			{
				iFreeLogicalChannel = 2;				
				readerlist.editItemAt(i).bLogicalChannelRequested2 = true;
				break;
			}
			else if (readerlist[i].bLogicalChannelRequested3 == false)
			{
				iFreeLogicalChannel = 3;				
				readerlist.editItemAt(i).bLogicalChannelRequested3 = true;
				break;
			}
			else 
			{
				ALOGW("no channel free");						
				iFreeLogicalChannel = -1;	
				break;			
			}
		}
	}
	//LOGI("proxy_readerlist_RequestLogicalChannel - free channel %i",iFreeLogicalChannel);	
	return iFreeLogicalChannel;
}	



String16 proxy_readerlist_getReaderName(unsigned long Lun) 
{
	//LOGI("proxy_readerlist_getReaderName - Lun %lu",Lun);
	
	String16 strReader = String16("");
	unsigned long channel = -1;
	bool bValidReaderName = false;
	int iListIndex = -1;
	String16 strOneReader = String16("");	
	for (int i=0; i<readerlist.size();++i)
	{
		if (readerlist[i].lun == Lun)
		{
			iListIndex = i;	
			channel = readerlist[i].channel;
			strReader = readerlist[i].readername;
		}
	}
	int iLenBuffer = 0;
	unsigned char* strName = convert_string16_to_string8(strReader, &iLenBuffer);
	//LOGI("proxy_readerlist_getReaderName Lun: %lu, reader from list: %s",Lun,strName);
	if ((iListIndex == -1) || (strReader != String16(""))) return strReader;

	if (strReader == String16(""))  //get readername at index = channel
	{
		//LOGI("proxy_readerlist_getReaderName - retrieve readernames from service");		
		//get readernames at index specified by channel
		String16 readerNames = gSmartcardAPI->getReaders();
		int iLenBuffer = 0;
		unsigned char* strName = convert_string16_to_string8(readerNames, &iLenBuffer);
		//LOGI("proxy_readerlist_getReaderName - readerNames: %s",strName);
		if (readerNames.size() > 0)
		{
			for (int i=0;i<=channel;++i)
			{			
				String16 separator = String16(";");
				int iIndexSeparator = readerNames.findFirst(separator[0]);
				int iLenBuffer = 0;
				unsigned char* strName = convert_string16_to_string8(readerNames, &iLenBuffer); 					
				//LOGI("proxy_readerlist_getReaderName - readerlist: %s",strName);			
				if (iIndexSeparator != -1)
				{
					//LOGI("indexSeparator = %d / size of list %d" ,iIndexSeparator,readerNames.size() );					
					strOneReader = String16(readerNames,iIndexSeparator,0);
					readerNames.remove(readerNames.size()-iIndexSeparator,iIndexSeparator+1);
					int iLenBuffer = 0;
					unsigned char* strName = convert_string16_to_string8(strOneReader, &iLenBuffer); 					
					//LOGI("proxy_readerlist_getReaderName - oneReaderName: %s",strName);					
					if (i==channel)
					{
						//LOGI("proxy_readerlist_getReaderName - add reader to list: %s, lun: %lu, channel: %lu",strName,Lun,channel);	
						readerlist.editItemAt(iListIndex).readername = strOneReader;
						break;
					}
				}
				else break;
			}
		}
	}
	return strOneReader;

}


extern "C"
int proxy_init() {

	if (gSmartcardAPI !=0) 
	{
		//LOGI("proxy_init - already connected to service manager");
		return 0;
	}
	

	//LOGI("proxy_init - try connect to service manager");

        sp<IServiceManager> sm = defaultServiceManager();
        sp<IBinder> binder;
        
			
	//LOGI("List available services:");                
	Vector<String16> servicelist;
	servicelist = sm->listServices();
	int iCount = servicelist.size();
	//LOGI("Numer of services available: %i",iCount);
	for (int i=0; i<iCount;++i) 
	{
		String16 strNameUnicode = servicelist.itemAt(i);
		int iLenName = strNameUnicode.size();			
		int iLenBuffer = 0;
		unsigned char* strName = convert_string16_to_string8(strNameUnicode, &iLenBuffer);
		//LOGI("Service #%i: %s",i,strName); 
	}
	
		
	binder = sm->getService(android::String16("smartcardservice"));
        if (binder == 0)
	{
		ALOGW("Smartcard System Service not published");
		return -1;
	}
        gSmartcardAPI = interface_cast<ISmartcardAPI>(binder);
	if (gSmartcardAPI == 0) 
	{ 
		ALOGW("Smartcard System Service is null");  
		return -1;
	}
	//LOGI("proxy_init - Smartcard System Service connected");
	return 0;
}

/*
String16 proxy_getReaders(void)
{
	LOGI("proxy_getreaders");
	String16 rsp = gSmartcardAPI->getReaders();
	return rsp;
}
*/

extern "C"
bool proxy_isCardPresent(unsigned long Lun)
{
	//LOGI("proxy_isCardPresent - Lun %lu",Lun);

	String16 strReader = proxy_readerlist_getReaderName(Lun);
	int iLenBuffer = 0;
	unsigned char* strName = convert_string16_to_string8(strReader, &iLenBuffer);
	bool bValue = gSmartcardAPI->isCardPresent(strReader);
	//LOGI("readername used to check for card present: %s,card present: %s",strName,(bValue)?"true":"false");
	return bValue;
}


//open channel if not yet open
long long int proxy_openBasicChannel(unsigned long Lun, String16& aid)
{
	//LOGI("proxy_openBasicChannel- Lun %lu",Lun);	
	if (!proxy_readerlist_hasValidHandle(Lun,0))
	{
		String16 strReader = proxy_readerlist_getReaderName(Lun);
				
		int iLenBuffer = 0;
		long long int handle = 0;
		unsigned char* strName = convert_string16_to_string8(strReader, &iLenBuffer);
		unsigned char* strAID = convert_string16_to_string8(aid, &iLenBuffer);
		//LOGI("proxy_openBasicChannel - readername: %s, AID: %s", strName, strAID);
		
		if (aid == String16(""))
		{
			//LOGI("proxy_openBasicChannel with AID");			
			handle = gSmartcardAPI->openBasicChannel(strReader);
		}
		else 
		{
	
			handle = gSmartcardAPI->openBasicChannelAid(strReader,aid);
		}

		//LOGI("proxy_openChannel - handle retrieved: %lli ", handle);			
		proxy_readerlist_addhandle (Lun, handle, 0);		    			
		return handle;
	}
}

  
long long int proxy_openLogicalChannel(const String16& reader, const String16& aid)
{
	//LOGI("proxy_openLogicalChannel");    	
	long long lValue = gSmartcardAPI->openLogicalChannel(reader, aid);
	//LOGI("proxy_openChannel - handle retrieved: %lli ", lValue);	
	return lValue;
}

void proxy_closeLogicalChannel(long long int handle)
{
	//LOGI("proxy_closeLogicalChannel - handle: %lli ", handle);	     	
	gSmartcardAPI->closeChannel(handle);
}


String16 proxy_transmit_internal(long long int handle, const String16& command)
{
	//LOGI("proxy_transmit_internal - handle: %lli ", handle);	   	
        String16 strValue = gSmartcardAPI->transmit(handle, command);
	return strValue;
}   



//***************************************************
//* return:
//* 0 -> no error
//* -1 -> general error, e.g. transmission error
//* sw1/sw2 returncode otherwise
//****************************************************
extern "C"
int proxy_getLastError()
{
	int iErrorCode = -1;	
	// errormessage if select failed, because AID not available: "message SELECT SW1/2 error: 6a82"
	//LOGI("call to proxy_getLastError");
	String16 strError = gSmartcardAPI->getLastError();
	if (strError.size() == 0)
	{
		//LOGI("proxy_getLastError - no error");		
		return 0;  //no error
	}
	
	int iLenBuffer = 0;
	unsigned char* strMessage = convert_string16_to_string8(strError, &iLenBuffer);
	//LOGI("proxy_getLastError - error message: %s", strMessage);


	//if errormessage contains a SW1/2 error code e.g. "message SELECT SW1/2 error: 6a82" then mask out the errorcode and return as int
	//check if error message contain a String part "SW1/2 error:"
	
	bool bSubstringFound = false;
	String16 strCompare("SW1/2 error:");
	int iIndex = 0;
	int iErrorSize = strError.size();
	for (iIndex=0;(iErrorSize-iIndex) > 0;++iIndex)
	{
		String16 strMessagePart = String16(strError, strError.size()-iIndex, iIndex);
		int iLenBuffer = 0; unsigned char* strMessage = convert_string16_to_string8(strMessagePart, &iLenBuffer);		
		//LOGI("strMessagePart: %s",strMessage);
		if (strMessagePart.startsWith(strCompare)) 
		{ 
		  //LOGI("substring found ");
		  bSubstringFound = true;
		  int returncodelen = 0;
		  String16 strReturncode = String16(strError, 4, strError.size()-4);
		  
		  iLenBuffer = 0; strMessage = convert_string16_to_string8(strReturncode, &iLenBuffer);	
		  //LOGI("strReturncode: %s, .length(): %i",strMessage,iLenBuffer);
		  unsigned char* returncode = convert_string16_to_buffer(strReturncode, &returncodelen);
		  //LOGI("returncodelen (converted): %i",returncodelen);
		  if (returncodelen == 2)
		  {
			iErrorCode = 0;
			iErrorCode += returncode[1];
			iErrorCode += returncode[0]*256;
		  }
		  break;
		}
	}       
	//LOGI("proxy_getLastError - returned error code: %i", iErrorCode);
	return iErrorCode; //error occured
	
}

 

extern "C"
bool proxy_transmit ( unsigned long Lun, unsigned char* TxBuffer, unsigned long TxLength, unsigned char* RxBuffer, unsigned long* RxLength) 
{

	//LOGI("call to proxy_transmit");
	String16 strCommandLogging("");	
	strCommandLogging = convert_buffer_to_string16(TxBuffer, TxLength);
	int iLenBufferLogging = 0;
	unsigned char* strMessageLogging = convert_string16_to_string8(strCommandLogging, &iLenBufferLogging);
	//LOGI("APDU: %s",strMessageLogging);
	
	//unsigned char* strMessage = convert_string16_to_string8(strError, &iLenBuffer);

	if ( (TxBuffer == 0) || (TxLength < 4) || (RxBuffer == 0) || (*RxLength < 2) )
	{
		//LOGI("parameter check failed");
		return false;
	}

	
	//APDU filtering

	// do not care about CLA = 0x80 or 0xA0
	if (((TxBuffer[0] & 0x80) == 0) && ((TxBuffer[0] & 0x60) != 0x20))
	{
		// MANAGE CHANNEL OPEN (unspecific channel) -> open a virtual channel if one available -> return '90 00', else  
		if ((TxBuffer[1] == 0x70) && (TxBuffer[2] == 0x00) && (TxBuffer[3] == 0x00))
		{
			//determine the lowest logical channel available
			int iLogicalChannel = -1;			
			iLogicalChannel = proxy_readerlist_RequestLogicalChannel(Lun);
			if (iLogicalChannel != -1)
			{
				if (*RxLength < 3) return false;  //not enough space to return channel byte 
				RxBuffer[0] = (char)iLogicalChannel;
				RxBuffer[1] = 0x90;
				RxBuffer[2] = 0x00;
				*RxLength = 3;				
			}
			else 
			{
				if (*RxLength < 2) return false;  //not enough space to return SW1/SW2 				
				RxBuffer[0] = 0x6A;
				RxBuffer[1] = 0x86;
				*RxLength = 2;
			}
			return true;
		}		


		// MANAGE CHANNEL OPEN (specific channel) 
		//-> in the current implementation we do not support this, as it seem to be not allowed for UICC, nevertheles it may 
		//be supported by othe smartcards -> return '6A 86' (incorrect P1/P2) 
		if ((TxBuffer[1] == 0x70) && (TxBuffer[2] == 0x00) && (TxBuffer[3] != 0x00))
		{
				if (*RxLength < 2) return false;  //not enough space to return SW1/SW2 				
				RxBuffer[0] = 0x6A;
				RxBuffer[1] = 0x86;
				*RxLength = 2;
				return true;
		}		


		// MANAGE CHANNEL CLOSE -> close the channel to the Smartcard API, if already open -> '90 00', else '62 00' (channel not yet open)  
		if ((TxBuffer[1] == 0x70) && (TxBuffer[2] == 0x80) &&  ((TxBuffer[3] & 0xFC) == 0x00) && ((TxBuffer[3] & 0x3) != 0x00)   )
		{
			int iLogicalChannel = TxBuffer[3];			
			bool bLogicalChannelOpen = false;
			bool bHandleValid = false;
			long long int handle = -1;
			

			bLogicalChannelOpen = proxy_readerlist_isLogicalChannelOpen (Lun,iLogicalChannel);
			bHandleValid = proxy_readerlist_hasValidHandle(Lun,iLogicalChannel);
			
			if (bLogicalChannelOpen)  //virtual close the logical channel
			{
				proxy_readerlist_invalidateHandle (Lun, iLogicalChannel );
			}
			else 
			{
				//the logical channel is not open
				if (*RxLength < 2) return false;  //not enough space 
				RxBuffer[0] = 0x62;
				RxBuffer[1] = 0x00;
				*RxLength = 2;
				return true;				
			}

			if (bHandleValid)
			{
				//close the channel to the SmartCard API
				long long int handle = -1;
				handle = proxy_readerlist_gethandle(Lun, iLogicalChannel);			
				proxy_closeLogicalChannel(handle);
			}

						
			//the logical channel is not open
			if (*RxLength < 2) return false;  //not enough space 
			RxBuffer[0] = 0x90;
			RxBuffer[1] = 0x00;
			*RxLength = 2;
			return true;
		}

		// SELECT APPLET (applet selection on a logical channel) -> if virtual logical channel is already open -> Open a logical channel within the
		// Smartcard API -> return '90 00'. If the logical channel coded in CLA not yet opened -> '68 81'
		if ( (TxBuffer[1] == 0xA4) &&  (TxBuffer[2] == 0x04) )
		{
			//check if virtual logical channel already open
			int iLogicalChannel = (TxBuffer[0] & 0x03);
			bool bLogicalChannelOpen = false;
			bool bHandleValid = false;
					
			bLogicalChannelOpen = proxy_readerlist_isLogicalChannelOpen (Lun,iLogicalChannel);
			bHandleValid = proxy_readerlist_hasValidHandle(Lun,iLogicalChannel);	


			//the logical channel is virtually open, and handle to SmartcardAPI exist			
			if ((bLogicalChannelOpen == true) && (bHandleValid == true))  
			{
				//close channel if a select occurs on a logical channel and reopen it
				//close the channel to the SmartCard API
				//LOGI("requested logical channel already opened with an AID - close/open channel performed");				
				long long int handle = -1;
				handle = proxy_readerlist_gethandle(Lun, iLogicalChannel);			
				proxy_readerlist_invalidateHandle (Lun, iLogicalChannel );  //invalidate the handle, so the logical channel will be reopened	
				proxy_closeLogicalChannel(handle);

				int iError = proxy_getLastError();
				if (proxy_getLastError() == -1) 
				{ 
					return false; 
				}
				else if (proxy_getLastError() > 0)   // returncode contains an SW1/SW2 error, e.g. if AID not exist
				{
					if (*RxLength < 2) return false;  //not enough space 
					RxBuffer[0] = iError / 256;
					RxBuffer[1] = iError % 256;
					*RxLength = 2;
					return true;		
				}
			}   
			
			//update info
			bLogicalChannelOpen = proxy_readerlist_isLogicalChannelOpen (Lun,iLogicalChannel);			
			bHandleValid = proxy_readerlist_hasValidHandle(Lun,iLogicalChannel);				

			//the logical channel is virtually open, but no handle from SmartcardAPI, or open implicitely by logical channel set in CLA of SELECT 	
			if ( ((bLogicalChannelOpen == true) || (bLogicalChannelOpen == false))  && (bHandleValid == false) )  
			{
				//LOGI("request logical channel with an AID - perform open channel");				
				long long int handle = 0;
				String16 strAID("");	
				
				if (TxLength < 5) return false;	//no Le-Byte			
				int iLenAID = TxBuffer[4];
				
				if (TxLength < (5+iLenAID)) return false; //input buffer smaller than indicated in Le-Byte
				strAID = convert_buffer_to_string16(&TxBuffer[5], iLenAID);
				
				String16 strReader = proxy_readerlist_getReaderName(Lun);		
				
				if (iLogicalChannel == 0)
				{
					handle = proxy_openBasicChannel(Lun, strAID);					
				}
				else
				{
					handle = proxy_openLogicalChannel(strReader, strAID);	
				}
												
				int iError = proxy_getLastError();
				if (proxy_getLastError() == -1) 
				{ 
					return false; 
				}
				else if (proxy_getLastError() > 0)   // returncode contains an SW1/SW2 error, e.g. if AID not exist
				{
					if (*RxLength < 2) return false;  //not enough space 
					RxBuffer[0] = iError / 256;
					RxBuffer[1] = iError % 256;
					*RxLength = 2;
					return true;		
				}
				else
				{
					proxy_readerlist_addhandle (Lun, handle, iLogicalChannel);
					proxy_readerlist_setLogicalChannelOpen (Lun, iLogicalChannel);
				}	

				if (*RxLength < 2) return false;  //not enough space 
				RxBuffer[0] = 0x90;
				RxBuffer[1] = 0x00;
				*RxLength = 2;
				return true;		
			}

		} 			
	}

	int iLogicalChannel = (TxBuffer[0] & 0x03);	
	if (iLogicalChannel == 0x00)  //basic channel is opened implicitely, if not yet open
	{
		//open basic channel if not yet open
		String16 strEmptyString = String16("");		
		proxy_openBasicChannel(Lun, strEmptyString);
		if (proxy_getLastError() != 0) 
		{
			return false;	
		}
	}

	if (iLogicalChannel != 0x00)  //not possible to send APDUs to a logical channel, before the we got a handle from the Smartcard API
	{			      //possibly the logical channel is virtually opened, but there was no SELECT AID command for this logical channel.
				      //so it was not possible to already get a handle from the Smartcard API for this logical channel	
                                      //we return a "logical channel not supported" '68 81' error in this case				      
 
		bool bHandleValid = false;
		bHandleValid = proxy_readerlist_hasValidHandle(Lun,iLogicalChannel);	
		if (bHandleValid == false)
		{
				if (*RxLength < 2) return false;  //not enough space 
				RxBuffer[0] = 0x68;
				RxBuffer[1] = 0x81;
				*RxLength = 2;
				return true;	
		}
	}


	long long int handle = 0;
	String16 strCommand("");	
	String16 strResponse("");
	unsigned char* responseAPDU = 0;
	int iLenResponseAPDU = 0;

	handle = proxy_readerlist_gethandle(Lun, iLogicalChannel);
	//LOGI("proxy_transmit - handle: %lli ", handle);		
	strCommand = convert_buffer_to_string16(TxBuffer, TxLength);
	strResponse = proxy_transmit_internal(handle, strCommand);
	responseAPDU = convert_string16_to_buffer(strResponse, &iLenResponseAPDU);

	if ((responseAPDU == 0) || (iLenResponseAPDU < 2)) {
		//LOGI("IFDHTransmitToICC - no response or too short");		
		return false;
	}

	if (*RxLength < iLenResponseAPDU) {
		//LOGI("IFDHTransmitToICC - response buffer too short");
		return false;
	}
	else {
		memcpy(RxBuffer, responseAPDU, iLenResponseAPDU);
		*RxLength = iLenResponseAPDU;
	}

	return true;
}



extern "C"
void proxy_closeAllChannels(unsigned long Lun)
{
	//LOGI("call to proxy_closeAllChannels %lu", Lun);
	//close channel for all handles
	for (int i=0;i<=3;++i)
	{
		long long int handle = proxy_readerlist_gethandle(Lun,i);
		gSmartcardAPI->closeChannel(handle);
	}
	proxy_readerlist_removeChannelEntry (Lun); 	       	
	return;
}



 

char convert_hex_digit_to_char(unsigned char value, int lowercase)
{
   value &= 0x0F;
   if (value <= 9) return (char)(value + '0');
   if (lowercase) return (unsigned char)(value - 10 + 'a');
   return (unsigned char)(value - 10 + 'A');
}

String16 convert_buffer_to_string16(unsigned char* buffer, int length) {
	String16 str16("");
	char* s;
	int pos;
	s = (char*)malloc((int)(2*length + 1));
	if (s == 0)
		str16 = String16("");
	else {
		pos = 0;
		while (length-- != 0) {
			unsigned char b = *buffer++;
			s[pos++] = convert_hex_digit_to_char((unsigned char)(b >> 4), 0);
			s[pos++] = convert_hex_digit_to_char((unsigned char)(b     ), 0);
		}
		s[pos++] = 0;
		str16 = String16(s);		
		
    	free(s);	
	}
	return str16;
}

unsigned char convert_char_to_hex_digit(char c)
{
   if (c <= '9') return (unsigned char)((c - '0') & 0xF);
   if (c <= 'F') return (unsigned char)((c - 'A' + 10) & 0xF);
   return (unsigned char)((c - 'a' + 10) & 0xF);
}

unsigned char* convert_string16_to_buffer(String16 str16, int* outlen) {
	unsigned char* buffer;
	const char16_t* s;
	int length, i;
   s = str16.string();
	length = str16.size() / 2;
	buffer = (unsigned char*)malloc(length);
	if (buffer == 0)
		 length = 0;
	for (i = 0; i < length; i++) {
       unsigned char b;
	    b  = (unsigned char)(convert_char_to_hex_digit((char)*s++) << 4);
       b |= (unsigned char)(convert_char_to_hex_digit((char)*s++)     );
	    buffer[i] = b;
	}
	*outlen = length;
	return buffer;
}

unsigned char* convert_string16_to_string8(String16 str16, int* outlen) {
	unsigned char* buffer;
	const char* s;
	int length, i;
	s = (char*)str16.string();
	length = str16.size() ;
	buffer = (unsigned char*)malloc(length+1);
	buffer[length] = 0x00;
	if (buffer == 0)
		 length = 0;
	for (i = 0; i < length; i++) {
       unsigned char b;
	    b  = (unsigned char)(s[i*2]);
	    buffer[i] = b;
	}
	*outlen = length;
	return buffer;
}




