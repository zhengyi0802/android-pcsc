/*
 * MUSCLE SmartCard Development ( http://pcsclite.alioth.debian.org/pcsclite.html )
 *
 * Copyright (C) 1999
 *  David Corcoran <corcoran@musclecard.com>
 * Copyright (C) 2004-2010
 *  Ludovic Rousseau <ludovic.rousseau@free.fr>
 *
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the author may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: testpcsc.c 7004 2014-10-02 09:26:36Z rousseau $
 */
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jni.h>

#define LOG_TAG "pcsc_jni"
#include <utils/Log.h>

#include "pcsclite.h"
#include "winscard.h"
#include "reader.h"
#include "pcsc_jni.h"

#define PANIC 0
#define DONT_PANIC 1

#define USE_AUTOALLOCATE

static SCARDCONTEXT hContext;
static SCARDHANDLE hCard;
static DWORD dwReaderLen, dwState, dwProt, dwAtrLen;
static DWORD dwPref, dwReaders = 0;
static char *pcReaders = NULL, *mszReaders;
static SCARD_READERSTATE rgReaderStates[1];
#ifdef USE_AUTOALLOCATE
	static unsigned char *pbAtr = NULL;
#else
	static unsigned char pbAtr[MAX_ATR_SIZE];
#endif
static char *mszGroups;
static DWORD dwGroups = 0;
static int iList[16] = {0};
static SCARD_IO_REQUEST ioRecvPci;	/* use a default value */
static const SCARD_IO_REQUEST *pioSendPci;
static unsigned char bSendBuffer[MAX_BUFFER_SIZE];
static unsigned char bRecvBuffer[MAX_BUFFER_SIZE];
static unsigned char *pbAttr = NULL;

using namespace std;


JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    ioRecvPci = *SCARD_PCI_T0;
    dwGroups = 0;
    return JNI_VERSION_1_6;
}

/*
 * Class:     tw_com_umedia_smartcard_service_PCSCService
 * Method:    EstablishContext
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_tw_com_umedia_smartcard_service_PCSCService_EstablishContext
  (JNIEnv *env, jobject jobj, jint scope) {
   long rv;

   ALOGD("EstablishContext");
   switch(scope) {
       case SCARD_SCOPE_USER :
            rv = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hContext);
            break;
       case SCARD_SCOPE_TERMINAL :
            rv = SCardEstablishContext(SCARD_SCOPE_TERMINAL, NULL, NULL, &hContext);
            break;
       case SCARD_SCOPE_SYSTEM :
            rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);
            break;
       default:
            rv = SCARD_E_INVALID_PARAMETER;
            break;
   }
   return rv;
}

/*
 * Class:     tw_com_umedia_smartcard_service_PCSCService
 * Method:    ReleaseContext
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_tw_com_umedia_smartcard_service_PCSCService_ReleaseContext
  (JNIEnv *env, jobject jobj) {
   long rv;

   ALOGD("ReleaseContext");
   rv = SCardReleaseContext(hContext);

   return rv;
}

/*
 * Class:     tw_com_umedia_smartcard_service_PCSCService
 * Method:    IsValidContext
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_tw_com_umedia_smartcard_service_PCSCService_IsValidContext
  (JNIEnv *env, jobject jobj) {
   long rv;

   ALOGD("IsValidContext hContext = %d", hContext);
   rv = SCardIsValidContext(hContext);
   if (rv != SCARD_S_SUCCESS) {
        ALOGD("Error code = %x", rv);
	(void)SCardReleaseContext(hContext);
	return rv;
   }
/*
   rv = SCardIsValidContext(hContext+1);
   ALOGD("Passed 1 : Error code = %x", rv);
*/
   return rv;
}

/*
 * Class:     tw_com_umedia_smartcard_service_PCSCService
 * Method:    Connect
 * Signature: (II)J
 */
JNIEXPORT jlong JNICALL Java_tw_com_umedia_smartcard_service_PCSCService_Connect
  (JNIEnv *env, jobject jobj, jint index, jint sharedmode) {
   long rv;

   ALOGD("Connect");
   rv = SCardConnect(hContext, &mszReaders[iList[index]],
		sharedmode, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
		&hCard, &dwPref);
   if (rv != SCARD_S_SUCCESS) {
	(void)SCardReleaseContext(hContext);
	return rv;
   }

   switch(dwPref) {
	case SCARD_PROTOCOL_T0:
		pioSendPci = SCARD_PCI_T0;
		break;
	case SCARD_PROTOCOL_T1:
		pioSendPci = SCARD_PCI_T1;
		break;
	case SCARD_PROTOCOL_RAW:
		pioSendPci = SCARD_PCI_RAW;
		break;
	default:
		ALOGE("Unknown protocol\n");
		return rv;
   }

   return rv;
}

/*
 * Class:     tw_com_umedia_smartcard_service_PCSCService
 * Method:    Reconnect
 * Signature: (II)J
 */
JNIEXPORT jlong JNICALL Java_tw_com_umedia_smartcard_service_PCSCService_Reconnect
  (JNIEnv *env, jobject jobj, jint sharedmode, jint initial) {
   long rv;

   ALOGD("Reconnect");
   rv = SCardReconnect(hCard, sharedmode, 
		SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, 
		initial, &dwPref);

   return rv;
}

/*
 * Class:     tw_com_umedia_smartcard_service_PCSCService
 * Method:    Disconnect
 * Signature: (I)J
 */
JNIEXPORT jlong JNICALL Java_tw_com_umedia_smartcard_service_PCSCService_Disconnect
  (JNIEnv *env, jobject jobj, jint option) {
   long rv;

   ALOGD("Disconnect");
   rv = SCardDisconnect(hCard, option);

   return rv;
}

/*
 * Class:     tw_com_umedia_smartcard_service_PCSCService
 * Method:    BeginTransaction
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_tw_com_umedia_smartcard_service_PCSCService_BeginTransaction
  (JNIEnv *env, jobject jobj) {
   long rv;

   ALOGD("BeginTransaction");
   rv = SCardBeginTransaction(hCard);

   return rv;
}

/*
 * Class:     tw_com_umedia_smartcard_service_PCSCService
 * Method:    EndTransaction
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_tw_com_umedia_smartcard_service_PCSCService_EndTransaction
  (JNIEnv *env, jobject jobj, jint option) {
   long rv;

   ALOGD("SCardEndTransaction");
   rv = SCardEndTransaction(hCard, option);

   return rv;
}

/*
 * Class:     tw_com_umedia_smartcard_service_PCSCService
 * Method:    Status
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_tw_com_umedia_smartcard_service_PCSCService_Status
  (JNIEnv *env, jobject jobj) {
   long rv;
   unsigned int i;

   ALOGD("Status");
#ifdef USE_AUTOALLOCATE
   dwReaderLen = SCARD_AUTOALLOCATE;
   dwAtrLen = SCARD_AUTOALLOCATE;
   rv = SCardStatus(hCard, (LPSTR)&pcReaders, &dwReaderLen, &dwState, &dwProt,
		(LPBYTE)&pbAtr, &dwAtrLen);
#else
   dwReaderLen = 100;
   pcReaders   = malloc(sizeof(char) * 100);
   dwAtrLen    = MAX_ATR_SIZE;

   rv = SCardStatus(hCard, pcReaders, &dwReaderLen, &dwState, &dwProt,
		pbAtr, &dwAtrLen);
#endif
   if (rv != SCARD_S_SUCCESS) {
	(void)SCardReleaseContext(hContext);
	return rv;
   }
   ALOGD("Current Reader Name\t\t: %s\n", pcReaders);
   ALOGD("Current Reader State\t\t: 0x%.4lx\n", dwState);
   ALOGD("Current Reader Protocol\t\t: T=%ld\n", dwProt - 1);
   ALOGD("Current Reader ATR Size\t\t: %ld bytes\n", dwAtrLen);
   ALOGD("Current Reader ATR Value\t: ");

   for (i = 0; i < dwAtrLen; i++) {
	ALOGD("%02X ", pbAtr[i]);
   }
   ALOGD("\n");

   return rv;
}

/*
 * Class:     tw_com_umedia_smartcard_service_PCSCService
 * Method:    GetStatusChange
 * Signature: (I)J
 */
JNIEXPORT jlong JNICALL Java_tw_com_umedia_smartcard_service_PCSCService_GetStatusChange
  (JNIEnv *env, jobject jobj, jint index) {
   long rv;

   ALOGD("GetStatusChange");

   if(index <= 0) {
	rgReaderStates[0].szReader = "\\\\?PnP?\\Notification";
	rgReaderStates[0].dwCurrentState = SCARD_STATE_EMPTY;
   } else {
	rgReaderStates[0].szReader = &mszReaders[iList[index]];
	rgReaderStates[0].dwCurrentState = SCARD_STATE_EMPTY;
   }
   rv = SCardGetStatusChange(hContext, INFINITE, rgReaderStates, 1);
   if(rv == SCARD_S_SUCCESS) {
	return rgReaderStates[0].dwEventState;
   }

   return rv;
}

/*
 * Class:     tw_com_umedia_smartcard_service_PCSCService
 * Method:    Control
 * Signature: (I[B)[B
 */
JNIEXPORT jbyteArray JNICALL Java_tw_com_umedia_smartcard_service_PCSCService_Control
  (JNIEnv *env, jobject jobj, jint code, jbyteArray array) {
   long rv;
   jbyte* bufferPtr = env->GetByteArrayElements(array, NULL);
   jsize lengthOfArray = env->GetArrayLength(array);
   jbyte *rbuffer;
   jbyteArray rarray;   
   char buffer[1024];
   DWORD cbRecvLength = sizeof(buffer);

   ALOGD("Control");
   memcpy(buffer, bufferPtr, lengthOfArray);

   rv = SCardControl(hCard, SCARD_CTL_CODE(code), buffer, lengthOfArray, buffer, sizeof(buffer), &cbRecvLength);
   if(rv != SCARD_S_SUCCESS) {
      return NULL;
   }

   rarray = env->NewByteArray(cbRecvLength);     
   rbuffer = (jbyte*) buffer;
   env->SetByteArrayRegion(rarray, 0, cbRecvLength, rbuffer);  

   env->ReleaseByteArrayElements(array, bufferPtr, JNI_ABORT);

   return rarray;
}

/*
 * Class:     tw_com_umedia_smartcard_service_PCSCService
 * Method:    Transmit
 * Signature: ([B)[B
 */
JNIEXPORT jbyteArray JNICALL Java_tw_com_umedia_smartcard_service_PCSCService_Transmit
  (JNIEnv *env, jobject jobj, jbyteArray array) {
   long rv;
   jbyte* bufferPtr = env->GetByteArrayElements(array, NULL);
   jsize lengthOfArray = env->GetArrayLength(array);
   jbyte *rbuffer;
   jbyteArray rarray;   
   DWORD length = sizeof(bRecvBuffer);

   ALOGD("Transmit");
   rv = SCardTransmit(hCard, pioSendPci, (LPCBYTE)bufferPtr, lengthOfArray,
		&ioRecvPci, bRecvBuffer, &length);

   rarray = env->NewByteArray(length);     
   rbuffer = (jbyte*) bRecvBuffer;
   env->SetByteArrayRegion(rarray, 0, length, rbuffer);  

   env->ReleaseByteArrayElements(array, bufferPtr, JNI_ABORT);

   return rarray;
}

/*
 * Class:     tw_com_umedia_smartcard_service_PCSCService
 * Method:    ListReaderGroups
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_tw_com_umedia_smartcard_service_PCSCService_ListReaderGroups
  (JNIEnv *env, jobject jobj) {
   long rv;
   int p, i;

   ALOGD("ListReaderGroups hContext = %d", hContext);
#ifdef USE_AUTOALLOCATE
   dwGroups = SCARD_AUTOALLOCATE;
   rv = SCardListReaderGroups(hContext, (LPSTR)&mszGroups, &dwGroups);
#else
   rv = SCardListReaderGroups(hContext, NULL, &dwGroups);
   if (rv != SCARD_S_SUCCESS) {
	(void)SCardReleaseContext(hContext);
	return rv;
   }

   ALOGD("Testing SCardListReaderGroups\t: ");
   mszGroups = calloc(dwGroups, sizeof(char));
   rv = SCardListReaderGroups(hContext, mszGroups, &dwGroups);
#endif

   if(rv != SCARD_S_SUCCESS) {
	(void)SCardReleaseContext(hContext);
	return rv;
   }

   /*
    * Have to understand the multi-string here
    */
   p = 0;
   for (i = 0; i+1 < dwGroups; i++) {
	++p;
	ALOGD("Group %02d: %s\n", p, &mszGroups[i]);
	while (mszGroups[++i] != 0);
   }

   rv = (long)p;

   return rv;
}

/*
 * Class:     tw_com_umedia_smartcard_service_PCSCService
 * Method:    ListReaders
 * Signature: ()[Ljava/lang/String
 */
JNIEXPORT jobjectArray JNICALL Java_tw_com_umedia_smartcard_service_PCSCService_ListReaders
  (JNIEnv *env, jobject jobj) {
   long rv;
   jobjectArray ret = NULL;
   unsigned int i, p;

   ALOGD("ListReaders hContext = %d", hContext);
#ifdef USE_AUTOALLOCATE
   dwReaders = SCARD_AUTOALLOCATE;
   rv = SCardListReaders(hContext, mszGroups, (LPSTR)&mszReaders, &dwReaders);
#else
   rv = SCardListReaders(hContext, mszGroups, NULL, &dwReaders);
   if (rv != SCARD_S_SUCCESS) {
	(void)SCardReleaseContext(hContext);
	return NULL;
   }

   printf("Testing SCardListReaders\t: ");
   mszReaders = calloc(dwReaders, sizeof(char));
   rv = SCardListReaders(hContext, mszGroups, mszReaders, &dwReaders);
#endif

   p=0;
   for (i = 0; i+1 < dwReaders; i++) {
        ++p;
	iList[p] = i;
	while (mszReaders[++i] != 0) ;
   }

   if(p > 0) {
	ret= (jobjectArray)env->NewObjectArray(p, env->FindClass("java/lang/String"), env->NewStringUTF(""));

	for(i=0; i < p; i++) {
		env->SetObjectArrayElement(ret, i, env->NewStringUTF(&mszReaders[iList[i+1]]));
	}
   }

   return ret;
}

#define MEMORY_MSZGROUPS 	1
#define MEMORY_MSZREADERS 	2
#define MEMORY_ATR_DATA		3
#define MEMORY_ATTR_DATA	4
#define MEMORY_PC_READERS	5

/*
 * Class:     tw_com_umedia_smartcard_service_PCSCService
 * Method:    FreeMemory
 * Signature: (I)J
 */
JNIEXPORT jlong JNICALL Java_tw_com_umedia_smartcard_service_PCSCService_FreeMemory
  (JNIEnv *env, jobject jobj, jint memory_id) {
   long rv;
   LPCVOID p;
   ALOGD("FreeMemory");
   switch(memory_id) {
	case MEMORY_MSZGROUPS:
		p = mszGroups;
		break;
	case MEMORY_MSZREADERS:
		p = mszReaders;
		break;
	case MEMORY_ATR_DATA:
		p = pbAtr;
		break;
	case MEMORY_ATTR_DATA:
		p = pbAttr;
		break;
	case MEMORY_PC_READERS:
		p = pcReaders;
		break;
	default:
		break;
   }

#ifdef USE_AUTOALLOCATE
   rv = SCardFreeMemory(hContext, p);
   if (rv != SCARD_S_SUCCESS) {
	(void)SCardReleaseContext(hContext);
	return rv;
   }
#else
   free(p);
#endif

   return rv;
}

/*
 * Class:     tw_com_umedia_smartcard_service_PCSCService
 * Method:    Cancel
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_tw_com_umedia_smartcard_service_PCSCService_Cancel
  (JNIEnv *env, jobject jobj) {
   long rv;

   ALOGD("Cancel");
   rv = SCardCancel(hContext);

   return rv;
}

/*
 * Class:     tw_com_umedia_smartcard_service_PCSCService
 * Method:    GetAttrib
 * Signature: (J)[B
 */
JNIEXPORT jbyteArray JNICALL Java_tw_com_umedia_smartcard_service_PCSCService_GetAttrib
  (JNIEnv *env, jobject jobj, jlong command) {
   long rv;
   jbyte *buffer;
   jbyteArray array;   
   DWORD pcbAttrLen;

   ALOGD("GetAttrib");
#ifdef USE_AUTOALLOCATE
   pcbAttrLen = SCARD_AUTOALLOCATE;
   rv = SCardGetAttrib(hCard, command, (unsigned char *)&pbAttr,
		&pcbAttrLen);
#else
   rv = SCardGetAttrib(hCard, command, NULL, &pcbAttrLen);
   if(rv != SCARD_S_SUCCESS) {
       return NULL;
   } else {
	ALOGD("command = %d length: %ld\n", command, pcbAttrLen);
	pbAttr = malloc(pcbAttrLen);
   }

   ALOGD("Testing SCardGetAttrib\t\t: ");
   rv = SCardGetAttrib(hCard, command, pbAttr, &pcbAttrLen);
#endif
   
   if(rv != SCARD_S_SUCCESS) {
       return NULL;
   }

   array = env->NewByteArray(pcbAttrLen);     
   buffer = (jbyte*) pbAttr;
   env->SetByteArrayRegion(array, 0, pcbAttrLen, buffer);  

   return array;
}

/*
 * Class:     tw_com_umedia_smartcard_service_PCSCService
 * Method:    SetAttrib
 * Signature: (J[B)J
 */
JNIEXPORT jlong JNICALL Java_tw_com_umedia_smartcard_service_PCSCService_SetAttrib
  (JNIEnv *env, jobject jobj, jlong command, jbyteArray array) {
   long rv;
   jbyte* bufferPtr = env->GetByteArrayElements(array, NULL);
   jsize lengthOfArray = env->GetArrayLength(array);

   ALOGD("SetAttrib");
   rv = SCardSetAttrib(hCard, command, (LPCBYTE)bufferPtr, lengthOfArray);

   env->ReleaseByteArrayElements(array, bufferPtr, JNI_ABORT);

   return rv;
}



