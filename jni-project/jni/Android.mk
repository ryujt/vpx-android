LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libvpx
LOCAL_SRC_FILES := ./libvpx/$(TARGET_ARCH_ABI)/lib/libvpx.a

include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := ./libvpx/$(TARGET_ARCH_ABI)/include
LOCAL_MODULE    := RyuVPX 
LOCAL_SRC_FILES := RyuVPX.c yuvTools.c
LOCAL_LDLIBS    := -lm -llog -ljnigraphics

LOCAL_STATIC_LIBRARIES := libvpx cpufeatures

LOCAL_CFLAGS := -Wall -DHAVE_MALLOC_H -DHAVE_PTHREAD -finline-functions -frename-registers -ffast-math -s -fomit-frame-pointer

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/cpufeatures)