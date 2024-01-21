###############################################################################
# All static executable files
# We only have arm version now
###############################################################################
LOCAL_PATH := $(call my-dir)
#ifeq ($(strip $(TARGET_ARCH)), arm)

###############################################################################
# custommadebin
include $(CLEAR_VARS)
LOCAL_MODULE := cellsbin
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_SHARED_LIBRARIES := libc libdl liblog
LOCAL_MODULE_STEM := $(LOCAL_MODULE)
LOCAL_SRC_FILES := $(TARGET_ARCH)/$(LOCAL_MODULE)
include $(BUILD_PREBUILT)

#endif
