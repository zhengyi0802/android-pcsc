# Copyright (C) 2010  Free Software Foundation, Inc.
# This Android.mk is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.


LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := proxy.cpp ifdsmartcardapi.c ISmartcardAPI.cpp 

LOCAL_C_INCLUDES	:= $(common_c_includes)
LOCAL_CFLAGS		:= -g -fpic -w 
LOCAL_MODULE		:= libifdsmartcardapi
LOCAL_MODULE_TAGS	:= optional
LOCAL_LDLIBS            := -llog -L~/usr/lib 
LOCAL_SHARED_LIBRARIES  := libutils libbinder liblog
LOCAL_PRELINK_MODULE	:= false

include $(BUILD_SHARED_LIBRARY)


                                                               
#PRODUCT_COPY_FILES += $(LOCAL_PATH)/reader.conf:system/etc/reader.conf

#include $(call all-makefiles-under,$(LOCAL_PATH))


