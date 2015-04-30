/*****************************************************************
/
/ File   :   ifdhandler.c
/ Author :   David Corcoran <corcoran@linuxnet.com>
/ Date   :   June 15, 2000
/ Purpose:   This provides reader specific low-level calls.
/            See http://www.linuxnet.com for more information.
/ License:   See file LICENSE
/
******************************************************************/

// Copyright 2010 Giesecke & Devrient GmbH.  All rights reserved.

#define _GNU_SOURCE

#include <pcscdefines.h>
#include <ifdsmartcardapi.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <utils/Log.h>


#undef LOG_TAG
#define LOG_TAG "ifdhandler smartcardapi"

#define TAG_IFD_SIMULTANEOUS_ACCESS 0x0FAF
#define TAG_IFD_SLOTS_NUMBER	0x0FAE

RESPONSECODE IFDHCreateChannel ( DWORD Lun, DWORD Channel ) {

	ALOGI("IFDHCreateChannel Lun: %lu  Channel: %lu  ",Lun,Channel);
	if (proxy_init()!=0)
   	{
		return IFD_COMMUNICATION_ERROR;
   	}

   	proxy_readerlist_addchannel(Lun, Channel); 	
   	return IFD_SUCCESS;

  /* Lun - Logical Unit Number, use this for multiple card slots 
     or multiple readers. 0xXXXXYYYY -  XXXX multiple readers,
     YYYY multiple slots. The resource manager will set these 
     automatically.  By default the resource manager loads a new
     instance of the driver so if your reader does not have more than
     one smartcard slot then ignore the Lun in all the functions.
     Future versions of PC/SC might support loading multiple readers
     through one instance of the driver in which XXXX would be important
     to implement if you want this.
  */
  
  /* Channel - Channel ID.  This is denoted by the following:
     0x000001 - /dev/pcsc/1
     0x000002 - /dev/pcsc/2
     0x000003 - /dev/pcsc/3
     
     USB readers may choose to ignore this parameter and query 
     the bus for the particular reader.
  */

  /* This function is required to open a communications channel to the 
     port listed by Channel.  For example, the first serial reader on COM1 would
     link to /dev/pcsc/1 which would be a sym link to /dev/ttyS0 on some machines
     This is used to help with intermachine independance.
     
     Once the channel is opened the reader must be in a state in which it is possible
     to query IFDHICCPresence() for card status.
 
     returns:

     IFD_SUCCESS
     IFD_COMMUNICATION_ERROR
  */
  
	
}

RESPONSECODE IFDHCloseChannel ( DWORD Lun ) {
	
	ALOGI("IFDHCloseChannel Lun: %lu ",Lun);
	if (proxy_init()!=0)
	{
		return IFD_COMMUNICATION_ERROR;
	}

	proxy_closeAllChannels(Lun);

	/*	
	if (proxy_getLastError() != 0) {
		return IFD_COMMUNICATION_ERROR;	
	}
	*/
	        
	return IFD_SUCCESS;

  
  /* This function should close the reader communication channel
     for the particular reader.  Prior to closing the communication channel
     the reader should make sure the card is powered down and the terminal
     is also powered down.

     returns:

     IFD_SUCCESS
     IFD_COMMUNICATION_ERROR     
  */
	
}

RESPONSECODE IFDHGetCapabilities ( DWORD Lun, DWORD Tag, 
				   PDWORD Length, PUCHAR Value ) {

	ALOGI("IFDHGetCapabilities");
	if (proxy_init()!=0)
	{
		ALOGI("IFDHGetCapabilities - return IFD_COMMUNICATION_ERROR");		
		return IFD_COMMUNICATION_ERROR;
	}

	RESPONSECODE ret = IFD_ERROR_TAG;

	switch(Tag) {

	case TAG_IFD_SLOTS_NUMBER:
		*Length = 1;
		*Value = 1;

		ALOGI("IFDHGetCapabilities/TAG_IFD_SLOTS_NUMBER - return IFD_SUCCESS");		
		ret = IFD_SUCCESS;
		break;
	case TAG_IFD_SIMULTANEOUS_ACCESS:
		*Length = 1;
		*Value = 255;
		ALOGI("IFDHGetCapabilities/TAG_IFD_SIMULTANEOUS_ACCESS - return IFD_SUCCESS");	
		ret = IFD_SUCCESS;
		break;
	default:
		break;
	}
	ALOGI("IFDHGetCapabilities - return IFD_ERROR_TAG");
	return ret;
  
  /* This function should get the slot/card capabilities for a particular
     slot/card specified by Lun.  Again, if you have only 1 card slot and don't mind
     loading a new driver for each reader then ignore Lun.

     Tag - the tag for the information requested
         example: TAG_IFD_ATR - return the Atr and it's size (required).
         these tags are defined in ifdhandler.h

     Length - the length of the returned data
     Value  - the value of the data

     returns:
     
     IFD_SUCCESS
     IFD_ERROR_TAG
  */

}

RESPONSECODE IFDHSetCapabilities ( DWORD Lun, DWORD Tag, 
			       DWORD Length, PUCHAR Value ) {

	ALOGI("IFDHSetCapabilities");
	
	if (proxy_init()!=0)
	{
		return IFD_COMMUNICATION_ERROR;
	}

	return IFD_ERROR_VALUE_READ_ONLY;

  /* This function should set the slot/card capabilities for a particular
     slot/card specified by Lun.  Again, if you have only 1 card slot and don't mind
     loading a new driver for each reader then ignore Lun.

     Tag - the tag for the information needing set

     Length - the length of the returned data
     Value  - the value of the data

     returns:
     
     IFD_SUCCESS
     IFD_ERROR_TAG
     IFD_ERROR_SET_FAILURE
     IFD_ERROR_VALUE_READ_ONLY
  */
  
	
}

RESPONSECODE IFDHSetProtocolParameters ( DWORD Lun, DWORD Protocol, 
				   UCHAR Flags, UCHAR PTS1,
				   UCHAR PTS2, UCHAR PTS3) {

	ALOGI("IFDHSetProtocolParameters");

	if (proxy_init()!=0)
	{
		ALOGI("IFDHSetProtocolParameters - return IFD_SUCCESS");		
		return IFD_COMMUNICATION_ERROR;
	}
	
	ALOGI("IFDHSetProtocolParameters - return IFD_SUCCESS");
	return IFD_SUCCESS;

  /* This function should set the PTS of a particular card/slot using
     the three PTS parameters sent

     Protocol  - 0 .... 14  T=0 .... T=14
     Flags     - Logical OR of possible values:
     IFD_NEGOTIATE_PTS1 IFD_NEGOTIATE_PTS2 IFD_NEGOTIATE_PTS3
     to determine which PTS values to negotiate.
     PTS1,PTS2,PTS3 - PTS Values.

     returns:

     IFD_SUCCESS
     IFD_ERROR_PTS_FAILURE
     IFD_COMMUNICATION_ERROR
     IFD_PROTOCOL_NOT_SUPPORTED
  */

}


RESPONSECODE IFDHPowerICC ( DWORD Lun, DWORD Action, 
			    PUCHAR Atr, PDWORD AtrLength ) {

	ALOGI("IFDHPowerICC");

	if (proxy_init()!=0)
	{
		return IFD_COMMUNICATION_ERROR;
	}

	//check if card present
	int bCardPresent = proxy_isCardPresent(Lun);

	if (proxy_getLastError() != 0) {
		return IFD_COMMUNICATION_ERROR;	
	}			
	
	if (!bCardPresent)
	{
		ALOGI("card not present");
		return IFD_COMMUNICATION_ERROR;
 	}
		
	RESPONSECODE ret = IFD_NOT_SUPPORTED;
	char *res;
	int len;

	switch(Action) {

	case IFD_POWER_UP:
		ret = IFD_COMMUNICATION_ERROR;


		*AtrLength = 5;
			memcpy(Atr, "\x3B\x80\x80\x01\x01", 5);
		

		ret = IFD_SUCCESS;
		break;

	default:
		break;
	}
	return ret;



  /* This function controls the power and reset signals of the smartcard reader
     at the particular reader/slot specified by Lun.

     Action - Action to be taken on the card.

     IFD_POWER_UP - Power and reset the card if not done so 
     (store the ATR and return it and it's length).
 
     IFD_POWER_DOWN - Power down the card if not done already 
     (Atr/AtrLength should
     be zero'd)
 
    IFD_RESET - Perform a quick reset on the card.  If the card is not powered
     power up the card.  (Store and return the Atr/Length)

     Atr - Answer to Reset of the card.  The driver is responsible for caching
     this value in case IFDHGetCapabilities is called requesting the ATR and it's
     length.  This should not exceed MAX_ATR_SIZE.

     AtrLength - Length of the Atr.  This should not exceed MAX_ATR_SIZE.

     Notes:

     Memory cards without an ATR should return IFD_SUCCESS on reset
     but the Atr should be zero'd and the length should be zero

     Reset errors should return zero for the AtrLength and return 
     IFD_ERROR_POWER_ACTION.

     returns:

     IFD_SUCCESS
     IFD_ERROR_POWER_ACTION
     IFD_COMMUNICATION_ERROR
     IFD_NOT_SUPPORTED
  */

}

RESPONSECODE IFDHTransmitToICC ( DWORD Lun, SCARD_IO_HEADER SendPci, 
				 PUCHAR TxBuffer, DWORD TxLength, 
				 PUCHAR RxBuffer, PDWORD RxLength, 
				 PSCARD_IO_HEADER RecvPci ) {

	ALOGI("IFDHTransmitToICC");

	if (proxy_init()!=0)
	{
		return IFD_COMMUNICATION_ERROR;
	}

	int bSuccess = 0;
	bSuccess = proxy_transmit(Lun, TxBuffer, TxLength, RxBuffer, RxLength, RecvPci );
	proxy_getLastError();
	return (bSuccess ? IFD_SUCCESS : IFD_COMMUNICATION_ERROR);
}
		


  
  /* This function performs an APDU exchange with the card/slot specified by
     Lun.  The driver is responsible for performing any protocol specific exchanges
     such as T=0/1 ... differences.  Calling this function will abstract all protocol
     differences.

     SendPci
     Protocol - 0, 1, .... 14
     Length   - Not used.

     TxBuffer - Transmit APDU example (0x00 0xA4 0x00 0x00 0x02 0x3F 0x00)
     TxLength - Length of this buffer.
     RxBuffer - Receive APDU example (0x61 0x14)
     RxLength - Length of the received APDU.  This function will be passed
     the size of the buffer of RxBuffer and this function is responsible for
     setting this to the length of the received APDU.  This should be ZERO
     on all errors.  The resource manager will take responsibility of zeroing
     out any temporary APDU buffers for security reasons.
  
     RecvPci
     Protocol - 0, 1, .... 14
     Length   - Not used.

     Notes:
     The driver is responsible for knowing what type of card it has.  If the current
     slot/card contains a memory card then this command should ignore the Protocol
     and use the MCT style commands for support for these style cards and transmit 
     them appropriately.  If your reader does not support memory cards or you don't
     want to then ignore this.

     RxLength should be set to zero on error.

     returns:
     
     IFD_SUCCESS
     IFD_COMMUNICATION_ERROR
     IFD_RESPONSE_TIMEOUT
     IFD_ICC_NOT_PRESENT
     IFD_PROTOCOL_NOT_SUPPORTED
  */
  


RESPONSECODE IFDHControl ( DWORD Lun, PUCHAR TxBuffer, 
			 DWORD TxLength, PUCHAR RxBuffer, 
			 PDWORD RxLength ) {

	ALOGI("IFDHControl");

	if (proxy_init()!=0)
	{
		return IFD_COMMUNICATION_ERROR;
	}

	*RxLength = 0;
	return IFD_SUCCESS;

  /* This function performs a data exchange with the reader (not the card)
     specified by Lun.  Here XXXX will only be used.
     It is responsible for abstracting functionality such as PIN pads,
     biometrics, LCD panels, etc.  You should follow the MCT, CTBCS 
     specifications for a list of accepted commands to implement.

     TxBuffer - Transmit data
     TxLength - Length of this buffer.
     RxBuffer - Receive data
     RxLength - Length of the received data.  This function will be passed
     the length of the buffer RxBuffer and it must set this to the length
     of the received data.

     Notes:
     RxLength should be zero on error.
  */

}

RESPONSECODE IFDHICCPresence( DWORD Lun ) {

  /* This function returns the status of the card inserted in the 
     reader/slot specified by Lun.  It will return either:

     returns:
     IFD_ICC_PRESENT
     IFD_ICC_NOT_PRESENT
     IFD_COMMUNICATION_ERROR
  */

	//ALOGI("IFDHICCPresence");

	if (proxy_init()!=0)
	{
		return IFD_COMMUNICATION_ERROR;
	}

	int bCardPresent = 0;
	bCardPresent = proxy_isCardPresent(Lun);
	if (proxy_getLastError() != 0) {
		return IFD_COMMUNICATION_ERROR;	
	}
	if (bCardPresent)
	{
		return 	IFD_ICC_PRESENT;
	}
	return IFD_ICC_NOT_PRESENT;
}

