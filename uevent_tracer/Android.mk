LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	uevent_tracer.c

LOCAL_STATIC_LIBRARIES :=
LOCAL_STATIC_LIBRARIES += libminzip libunz libmtdutil libmincrypt libm
LOCAL_STATIC_LIBRARIES += libminiui libpixelflinger_static libpng libz libcutils
LOCAL_STATIC_LIBRARIES += libstdc++ libc 

LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false
LOCAL_MODULE:= uevent_tracer
LOCAL_ARM_MODE := arm

include $(BUILD_EXECUTABLE)

