LOCAL_PATH := $(call my-dir)
MAIN_LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libdobby
LOCAL_SRC_FILES := Dobby/$(TARGET_ARCH_ABI)/libdobby.a
include $(PREBUILT_STATIC_LIBRARY)

LOCAL_MODULE := Yeti
LOCAL_MODULE_FILENAME := libYeti

LOCAL_ARM_MODE := arm
LOCAL_CFLAGS := -Wno-error=format-security -fpermissive
LOCAL_CFLAGS += -fno-rtti -fno-exceptions -std=c++14
LOCAL_CPPFLAGS += -ffunction-sections -fdata-sections
LOCAL_LDFLAGS += -Wl,--strip-all

LOCAL_C_INCLUDES += $(MAIN_LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/src/UE4
LOCAL_SRC_FILES := src/Main.cpp \
                   src/Tools.cpp \
                   src/Logger.cpp \
                   src/UE4/FunctionFlags.cpp \
                   src/UE4/PropertyFlags.cpp \
                   src/ObjectsStore.cpp \
                   src/NamesStore.cpp \
                   src/Generator.cpp \
                   src/NameValidator.cpp \
                   src/UE4/GenericTypes.cpp \
                   src/PrintHelper.cpp \
                   src/Package.cpp

LOCAL_LDLIBS := -llog -landroid

LOCAL_STATIC_LIBRARIES := libdobby

include $(BUILD_SHARED_LIBRARY)
