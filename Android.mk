LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= ps-avg.c

#LOCAL_SHARED_LIBRARIES := liblog libcutils

LOCAL_MODULE := ps-avg

LOCAL_CFLAGS := -Werror

LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_EXECUTABLES)

include $(BUILD_EXECUTABLE)
