package tw.com.umedia.smartcard.service;

import android.util.Log;

public class PCSCService {
	
	private static final String TAG = "PCSCService";
	private static boolean JNILoaded = false;
	
	// from pcsclite.h
	public static long SCARD_S_SUCCESS = 0x00000000;
	public static long SCARD_F_INTERNAL_ERROR = 0x80100001;
	public static long SCARD_E_CANCELLED = 0x80100002;
	public static long SCARD_E_INVALID_HANDLE = 0x80100003;
	public static long SCARD_E_INVALID_PARAMETER = 0x80100004;
	public static long SCARD_E_INVALID_TARGET = 0x80100005;
	public static long SCARD_E_NO_MEMORY = 0x80100006;
	public static long SCARD_F_WAITED_TOO_LONG = 0x80100007;
	public static long SCARD_E_INSUFFICIENT_BUFFER = 0x80100008;
	public static long SCARD_E_UNKNOWN_READER = 0x80100009;
	public static long SCARD_E_TIMEOUT	= 0x8010000a;
	public static long SCARD_E_SHARING_VIOLATION = 0x8010000b;
	public static long SCARD_E_NO_SMARTCARD = 0x8010000c;
	public static long SCARD_E_UNKNOWN_CARD = 0x8010000d;
	public static long SCARD_E_CANT_DISPOSE = 0x8010000e;
	public static long SCARD_E_PROTO_MISMATCH = 0x8010000f;
	public static long SCARD_E_NOT_READY = 0x80100010;
	public static long SCARD_E_INVALID_VALUE = 0x80100011;
	public static long SCARD_E_SYSTEM_CANCELLED = 0x80100012;
	public static long SCARD_F_COMM_ERROR = 0x80100013;
	public static long SCARD_F_UNKNOWN_ERROR = 0x80100014;
	public static long SCARD_E_INVALID_ATR = 0x80100015;
	public static long SCARD_E_NOT_TRANSACTED = 0x80100016;
	public static long SCARD_E_READER_UNAVAILABLE = 0x80100017;
	public static long SCARD_P_SHUTDOWN = 0x80100018;
	public static long SCARD_E_PCI_TOO_SMALL = 0x80100019;
	public static long SCARD_E_READER_UNSUPPORTED = 0x8010001a;
	public static long SCARD_E_DUPLICATE_READER = 0x8010001b;
	public static long SCARD_E_CARD_UNSUPPORTED = 0x8010001c;
	public static long SCARD_E_NO_SERVICE = 0x8010001d;
	public static long SCARD_E_SERVICE_STOPPED = 0x8010001e;
	public static long SCARD_E_UNEXPECTED = 0x8010001f;
	public static long SCARD_E_UNSUPPORTED_FEATURE = 0x8010001f;
	public static long SCARD_E_ICC_INSTALLATION = 0x80100020;
	public static long SCARD_E_ICC_CREATEORDER = 0x80100021;
	// public static long SCARD_E_UNSUPPORTED_FEATURE = 0x80100022;
	public static long SCARD_E_DIR_NOT_FOUND = 0x80100023;
	public static long SCARD_E_FILE_NOT_FOUND = 0x80100024;
	public static long SCARD_E_NO_DIR = 0x80100025;
	public static long SCARD_E_NO_FILE = 0x80100026;
	public static long SCARD_E_NO_ACCESS = 0x80100027;
	public static long SCARD_E_WRITE_TOO_MANY = 0x80100028;
	public static long SCARD_E_BAD_SEEK = 0x80100029;
	public static long SCARD_E_INVALID_CHV = 0x8010002a;
	public static long SCARD_E_UNKNOWN_RES_MNG = 0x8010002b;
	public static long SCARD_E_NO_SUCH_CERTIFICATE = 0x8010002c;
	public static long SCARD_E_CERTIFICATE_UNAVAILABLE = 0x8010002d;
	public static long SCARD_E_NO_READERS_AVAILABLE = 0x8010002e;
	public static long SCARD_E_COMM_DATA_LOST = 0x8010002f;
	public static long SCARD_E_NO_KEY_CONTAINER = 0x80100030;
	public static long SCARD_E_SERVER_TOO_BUSY = 0x80100031;
	
	public static long SCARD_W_UNSUPPORTED_CARD = 0x80100065;
	public static long SCARD_W_UNRESPONSIVE_CARD = 0x80100066;
	public static long SCARD_W_UNPOWERED_CARD = 0x80100067;
	public static long SCARD_W_RESET_CARD = 0x80100068;
	public static long SCARD_W_REMOVED_CARD = 0x80100069;
	
	public static long SCARD_W_SECURITY_VIOLATION = 0x8010006a;
	public static long SCARD_W_WRONG_CHV = 0x8010006b;
	public static long SCARD_W_CHV_BLOCKED = 0x8010006c;
	
	public static long SCARD_W_EOF = 0x8010006d;
	public static long SCARD_W_CANCELLED_BY_USER = 0x8010006e;
	public static long SCARD_W_CARD_NOT_AUTHENTICATED = 0x8010006f;
	
	public static int SCARD_SCOPE_USER = 0x0000;
	public static int SCARD_SCOPE_TERMINAL = 0x0001;
	public static int SCARD_SCOPE_SYSTEM = 0x0002;
	
	public static int SCARD_PROTOCOL_UNDEFINED = 0x0000;
	public static int SCARD_PROTOCOL_UNSET = 0x0000;
	public static int SCARD_PROTOCOL_T0 = 0x0001;
	public static int SCARD_PROTOCOL_T1 = 0x0002;
	public static int SCARD_PROTOCOL_RAW = 0x0004;
	public static int SCARD_PROTOCOL_T15 = 0x0008;
	public static int SCARD_PROTOCOL_ANY = (SCARD_PROTOCOL_T0|SCARD_PROTOCOL_T1);
	
	public static int SCARD_SHARE_EXCLUSIVE = 0x0001;
	public static int SCARD_SHARE_SHARED = 0x0002;
	public static int SCARD_SHARE_DIRECT = 0x0003;
	
	public static int SCARD_LEAVE_CARD = 0x0000;
	public static int SCARD_RESET_CARD = 0x0001;
	public static int SCARD_UNPOWER_CARD = 0x0002;
	public static int SCARD_EJECT_CARD = 0x0003;
	
	public static int SCARD_UNKNOWN = 0x0001;
	public static int SCARD_ABSENT = 0x0002;
	public static int SCARD_PRESENT = 0x0004;
	public static int SCARD_SWALLOWED	= 0x0008;
	public static int SCARD_POWERED = 0x0010;
	public static int SCARD_NEGOTIABLE = 0x0020;
	public static int SCARD_SPECIFIC = 0x0040;
	
	public static int SCARD_STATE_UNAWARE = 0x0000;
	public static int SCARD_STATE_IGNORE = 0x0001;
	public static int SCARD_STATE_CHANGED = 0x0002;
	public static int SCARD_STATE_UNKNOWN = 0x0004;
	public static int SCARD_STATE_UNAVAILABLE = 0x0008;
	public static int SCARD_STATE_EMPTY = 0x0010;
	public static int SCARD_STATE_PRESENT = 0x0020;
	public static int SCARD_STATE_ATRMATCH = 0x0040;
	public static int SCARD_STATE_EXCLUSIVE = 0x0080;
	public static int SCARD_STATE_INUSE = 0x0100;
	public static int SCARD_STATE_MUTE = 0x0200;
	public static int SCARD_STATE_UNPOWERED = 0x0400;
	
	public static int MAX_READERNAME = 128;
	public static int MAX_BUFFER_SIZE = 264;
	public static int MAX_BUFFER_SIZE_EXTENDED = (4 + 3 + (1<<16) + 3 + 2);
	public static int MAX_ATR_SIZE = 33;
	public static int SCARD_ATR_LENGTH = MAX_ATR_SIZE;


	// from reader.h
	public static int SCARD_CLASS_VENDOR_INFO = 0x0001;
	public static int SCARD_CLASS_COMMUNICATIONS = 0x0002;
	public static int SCARD_CLASS_PROTOCOL = 0x0003;
	public static int SCARD_CLASS_POWER_MGMT = 0x0004;
	public static int SCARD_CLASS_SECURITY = 0x0005;
	public static int SCARD_CLASS_MECHANICAL = 0x0006;
	public static int SCARD_CLASS_VENDOR_DEFINED = 0x0007;
	public static int SCARD_CLASS_IFD_PROTOCOL = 0x0008;
	public static int SCARD_CLASS_ICC_STATE = 0x0009;
	public static int SCARD_CLASS_SYSTEM = 0x7fff;

	public static long SCARD_ATTR_VENDOR_NAME = ((SCARD_CLASS_VENDOR_INFO)<<16 | 0x0100); 
	public static long SCARD_ATTR_VENDOR_IFD_TYPE = ((SCARD_CLASS_VENDOR_INFO)<<16 | 0x0101); 
	public static long SCARD_ATTR_VENDOR_IFD_VERSION = ((SCARD_CLASS_VENDOR_INFO)<<16 | 0x0102); 
	public static long SCARD_ATTR_VENDOR_IFD_SERIAL_NO = ((SCARD_CLASS_VENDOR_INFO)<<16 | 0x0103); 
	public static long SCARD_ATTR_CHANNEL_ID = ((SCARD_CLASS_COMMUNICATIONS)<<16 | 0x0110);
 
	public static long SCARD_ATTR_ASYNC_PROTOCOL_TYPES = ((SCARD_CLASS_PROTOCOL)<<16 | 0x0120); 
	public static long SCARD_ATTR_DEFAULT_CLK = ((SCARD_CLASS_PROTOCOL)<<16 | 0x0121); 
	public static long SCARD_ATTR_MAX_CLK = ((SCARD_CLASS_PROTOCOL)<<16 | 0x0122); 
	public static long SCARD_ATTR_DEFAULT_DATA_RATE = ((SCARD_CLASS_PROTOCOL)<<16 | 0x0123); 
	public static long SCARD_ATTR_MAX_DATA_RATE = ((SCARD_CLASS_PROTOCOL)<<16 | 0x0124); 
	public static long SCARD_ATTR_MAX_IFSD = ((SCARD_CLASS_PROTOCOL)<<16 | 0x0125); 
	public static long SCARD_ATTR_SYNC_PROTOCOL_TYPES = ((SCARD_CLASS_PROTOCOL)<<16 | 0x0126); 
	public static long SCARD_ATTR_POWER_MGMT_SUPPORT = ((SCARD_CLASS_POWER_MGMT)<<16 | 0x0131);
 
	public static long SCARD_ATTR_USER_TO_CARD_AUTH_DEVICE = ((SCARD_CLASS_SECURITY)<<16 | 0x0140); 
	public static long SCARD_ATTR_USER_AUTH_INPUT_DEVICE = ((SCARD_CLASS_SECURITY)<<16 | 0x0142); 
	public static long SCARD_ATTR_CHARACTERISTICS = ((SCARD_CLASS_MECHANICAL)<<16 | 0x0150); 

	public static long SCARD_ATTR_CURRENT_PROTOCOL_TYPE = ((SCARD_CLASS_IFD_PROTOCOL)<<16 | 0x0201); 
	public static long SCARD_ATTR_CURRENT_CLK = ((SCARD_CLASS_IFD_PROTOCOL)<<16 | 0x0202); 
	public static long SCARD_ATTR_CURRENT_F = ((SCARD_CLASS_IFD_PROTOCOL)<<16 | 0x0203); 
	public static long SCARD_ATTR_CURRENT_D = ((SCARD_CLASS_IFD_PROTOCOL)<<16 | 0x0204); 
	public static long SCARD_ATTR_CURRENT_N = ((SCARD_CLASS_IFD_PROTOCOL)<<16 | 0x0205); 
	public static long SCARD_ATTR_CURRENT_W = ((SCARD_CLASS_IFD_PROTOCOL)<<16 | 0x0206); 
	public static long SCARD_ATTR_CURRENT_IFSC = ((SCARD_CLASS_IFD_PROTOCOL)<<16 | 0x0207); 
	public static long SCARD_ATTR_CURRENT_IFSD = ((SCARD_CLASS_IFD_PROTOCOL)<<16 | 0x0208); 
	public static long SCARD_ATTR_CURRENT_BWT = ((SCARD_CLASS_IFD_PROTOCOL)<<16 | 0x0209); 
	public static long SCARD_ATTR_CURRENT_CWT = ((SCARD_CLASS_IFD_PROTOCOL)<<16 | 0x020a); 
	public static long SCARD_ATTR_CURRENT_EBC_ENCODING = ((SCARD_CLASS_IFD_PROTOCOL)<<16 | 0x020b); 
	public static long SCARD_ATTR_EXTENDED_BWT = ((SCARD_CLASS_IFD_PROTOCOL)<<16 | 0x020c); 

	public static long SCARD_ATTR_ICC_PRESENCE = ((SCARD_CLASS_ICC_STATE)<<16 | 0x0300); 
	public static long SCARD_ATTR_ICC_INTERFACE_STATUS = ((SCARD_CLASS_ICC_STATE)<<16 | 0x0301); 
	public static long SCARD_ATTR_CURRENT_IO_STATE = ((SCARD_CLASS_ICC_STATE)<<16 | 0x0302); 
	public static long SCARD_ATTR_ATR_STRING = ((SCARD_CLASS_ICC_STATE)<<16 | 0x0303); 
	public static long SCARD_ATTR_ICC_TYPE_PER_ATR = ((SCARD_CLASS_ICC_STATE)<<16 | 0x0304);
 
	public static long SCARD_ATTR_ESC_RESET = ((SCARD_CLASS_VENDOR_DEFINED)<<16 | 0xA000); 
	public static long SCARD_ATTR_ESC_CANCEL = ((SCARD_CLASS_VENDOR_DEFINED)<<16 | 0xA003); 
	public static long SCARD_ATTR_ESC_AUTHREQUEST = ((SCARD_CLASS_VENDOR_DEFINED)<<16 | 0xA005); 
	public static long SCARD_ATTR_MAXINPUT = ((SCARD_CLASS_VENDOR_DEFINED)<<16 | 0xA007);
 
	public static long SCARD_ATTR_DEVICE_UNIT = ((SCARD_CLASS_SYSTEM)<<16 | 0x0001);
 	public static long SCARD_ATTR_DEVICE_IN_USE = ((SCARD_CLASS_SYSTEM)<<16 | 0x0002); 
	public static long SCARD_ATTR_DEVICE_FRIENDLY_NAME_A = ((SCARD_CLASS_SYSTEM)<<16 | 0x0003); 
	public static long SCARD_ATTR_DEVICE_SYSTEM_NAME_A = ((SCARD_CLASS_SYSTEM)<<16 | 0x0004); 
	public static long SCARD_ATTR_DEVICE_FRIENDLY_NAME_W = ((SCARD_CLASS_SYSTEM)<<16 | 0x0005); 
	public static long SCARD_ATTR_DEVICE_SYSTEM_NAME_W = ((SCARD_CLASS_SYSTEM)<<16 | 0x0006); 
	public static long SCARD_ATTR_SUPRESS_T1_IFS_REQUEST = ((SCARD_CLASS_SYSTEM)<<16 | 0x0007); 

	public static long SCARD_ATTR_DEVICE_FRIENDLY_NAME = SCARD_ATTR_DEVICE_FRIENDLY_NAME_W; 
	public static long SCARD_ATTR_DEVICE_SYSTEM_NAME = SCARD_ATTR_DEVICE_SYSTEM_NAME_W; 
	
	public static int FEATURE_VERIFY_PIN_START = 0x01;
	public static int FEATURE_VERIFY_PIN_FINISH = 0x02;
	public static int FEATURE_MODIFY_PIN_START = 0x03;
	public static int FEATURE_MODIFY_PIN_FINISH = 0x04;
	public static int FEATURE_GET_KEY_PRESSED = 0x05;
	public static int FEATURE_VERIFY_PIN_DIRECT = 0x06;
	public static int FEATURE_MODIFY_PIN_DIRECT = 0x07;
	public static int FEATURE_MCT_READER_DIRECT = 0x08;
	public static int FEATURE_MCT_UNIVERSAL = 0x09;
	public static int FEATURE_IFD_PIN_PROPERTIES = 0x0A;
	public static int FEATURE_ABORT = 0x0B;
	public static int FEATURE_SET_SPE_MESSAGE = 0x0C;
	public static int FEATURE_VERIFY_PIN_DIRECT_APP_ID = 0x0D;
	public static int FEATURE_MODIFY_PIN_DIRECT_APP_ID = 0x0E;
	public static int FEATURE_WRITE_DISPLAY = 0x0F;
	public static int FEATURE_GET_KEY = 0x10;
	public static int FEATURE_IFD_DISPLAY_PROPERTIES = 0x11;
	public static int FEATURE_GET_TLV_PROPERTIES = 0x12;
	public static int FEATURE_CCID_ESC_COMMAND = 0x13;
	public static int FEATURE_EXECUTE_PACE = 0x20;
	
	static {
		try {
			System.loadLibrary("pcscjni");
			JNILoaded = true;
			Log.d(TAG, "load library libpcscjni.so successful!");
		} catch (Throwable t) {
			Log.e(TAG,"error load library libpcscjni.so");
		}
	}

	public PCSCService() {
	}

	public native long EstablishContext(int scope);
	public native long ReleaseContext();
	public native long IsValidContext();
	public native long Connect(int index, int sharedmode);
	public native long Reconnect(int sharedmode, int initial);
	public native long Disconnect(int option);
	public native long BeginTransaction();
	public native long EndTransaction(int option);
	public native long Status();
	public native long GetStatusChange(int index);
	public native byte[] Control(int code, byte[] txdata);
	public native byte[] Transmit(byte[] txdata);
	public native long ListReaderGroups();
	public native String[] ListReaders();
	public native long FreeMemory(int memory_id);
	public native long Cancel();
	public native byte[] GetAttrib(long command);
	public native long SetAttrib(long command, byte[] attrs);
}
