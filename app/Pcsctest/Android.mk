LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_STATIC_JAVA_LIBRARIES := android-support-v4
LOCAL_SRC_FILES := $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := PcscTest
LOCAL_CERTIFICATE := platform
LOCAL_REQUIRED_MODULES  := libpcscjni
#LOCAL_JNI_SHARED_LIBRARIES := libscreen_adjust_jni

include $(BUILD_PACKAGE)

# Use the folloing include to make our test apk.
include $(call all-makefiles-under,$(LOCAL_PATH))
