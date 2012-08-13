LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

# copy-n-paste from Makefile.am
LOCAL_SRC_FILES := \
charonservice.c

# build libandroidbridge -------------------------------------------------------

LOCAL_C_INCLUDES += \
	$(libvstr_PATH) \
	$(strongswan_PATH)/src/libipsec \
	$(strongswan_PATH)/src/libhydra \
	$(strongswan_PATH)/src/libcharon \
	$(strongswan_PATH)/src/libstrongswan

LOCAL_CFLAGS := $(strongswan_CFLAGS) \
	-DPLUGINS='"$(strongswan_CHARON_PLUGINS)"'

LOCAL_MODULE := libandroidbridge

LOCAL_MODULE_TAGS := optional

LOCAL_ARM_MODE := arm

LOCAL_PRELINK_MODULE := false

LOCAL_LDLIBS := -llog

LOCAL_SHARED_LIBRARIES := libstrongswan libhydra libipsec libcharon

include $(BUILD_SHARED_LIBRARY)

