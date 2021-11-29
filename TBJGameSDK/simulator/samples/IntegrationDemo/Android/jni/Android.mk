# Android.mk
# Copyright (C) 2011 Audiokinetic Inc.
# \file 
# Makefile for building the integration demo shared object.

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

ifndef AK_CONFIG
AK_CONFIG := Profile
endif

ifneq ($(AK_CONFIG), Debug)
NDK_DEBUG := 1
endif

include $(CLEAR_VARS)
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../../SoundEngine/Posix
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../../SoundEngine/Android
LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)/../../../../include
LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)/../../Common
LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)/../../MenuSystem
LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)/../../DemoPages
LOCAL_EXPORT_C_INCLUDES += $(LOCAL_PATH)/../

SDK_LIB_DIR := ../../../../$(TARGET_PLATFORM)_$(TARGET_ARCH_ABI)/$(AK_CONFIG)/lib
ifneq ($(AK_CONFIG), Release)
	LOCAL_MODULE            := CommunicationCentral
	LOCAL_SRC_FILES         := $(SDK_LIB_DIR)/libCommunicationCentral.a 
	include $(PREBUILT_STATIC_LIBRARY)
	include $(CLEAR_VARS)
else
	LOCAL_EXPORT_CFLAGS 	+= -DAK_OPTIMIZED
endif

LOCAL_MODULE            := AkMemoryMgr
LOCAL_SRC_FILES         := $(SDK_LIB_DIR)/libAkMemoryMgr.a 
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AkSoundEngine
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAkSoundEngine.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AkMeterFX
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAkMeterFX.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AkPeakLimiterFX
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAkPeakLimiterFX.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AkHarmonizerFX
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAkHarmonizerFX.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AkStereoDelayFX
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAkStereoDelayFX.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AkMusicEngine
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAkMusicEngine.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AkTimeStretchFX
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAkTimeStretchFX.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := McDSPLimiterFX
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libMcDSPLimiterFX.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AkConvolutionReverbFX
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAkConvolutionReverbFX.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AkCompressorFX
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAkCompressorFX.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AkFlangerFX
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAkFlangerFX.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AkPitchShifterFX
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAkPitchShifterFX.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AkSoundSeedImpactFX
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAkSoundSeedImpactFX.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := McDSPFutzBoxFX
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libMcDSPFutzBoxFX.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AkRoomVerbFX
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAkRoomVerbFX.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AkSineSource
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAkSineSource.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AkSilenceSource
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAkSilenceSource.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AkSynthOne
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAkSynthOne.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AkParametricEQFX
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAkParametricEQFX.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AkVorbisDecoder
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAkVorbisDecoder.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AkToneSource
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAkToneSource.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AkExpanderFX
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAkExpanderFX.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AkGuitarDistortionFX
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAkGuitarDistortionFX.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AkSoundSeedWoosh
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAkSoundSeedWoosh.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AkTremoloFX
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAkTremoloFX.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AkMatrixReverbFX
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAkMatrixReverbFX.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AkStreamMgr
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAkStreamMgr.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AkDelayFX
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAkDelayFX.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AkAudioInputSource
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAkAudioInputSource.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AkSoundSeedWind
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAkSoundSeedWind.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AkGainFX
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAkGainFX.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AstoundsoundExpanderFX
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAstoundsoundExpanderFX.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AstoundsoundFolddownFX
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAstoundsoundFolddownFX.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AstoundsoundRTIFX
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAstoundsoundRTIFX.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE            := AstoundsoundShared
LOCAL_SRC_FILES 		:= $(SDK_LIB_DIR)/libAstoundsoundShared.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
 
LOCAL_ARM_MODE   := arm

LOCAL_MODULE    := IntegrationDemo
LOCAL_SRC_FILES := main.cpp IntegrationDemo.cpp 
LOCAL_LDLIBS := -llog -lOpenSLES -landroid -lEGL -lGLESv1_CM
LOCAL_CFLAGS := -DLUA_USE_POSIX

LOCAL_STATIC_LIBRARIES := android_native_app_glue
LOCAL_WHOLE_STATIC_LIBRARIES += gnustl_static

ifneq ($(AK_CONFIG), Release)
	LOCAL_STATIC_LIBRARIES += CommunicationCentral 
endif
LOCAL_STATIC_LIBRARIES += AkMusicEngine \
AkSoundEngine \
AkAudioInputSource \
AkCompressorFX \
AkConvolutionReverbFX \
AkDelayFX \
AkExpanderFX \
AkFlangerFX \
AkGainFX \
AkGuitarDistortionFX \
AkHarmonizerFX \
AkMatrixReverbFX \
AkMemoryMgr \
AkMeterFX \
AkParametricEQFX \
AkPeakLimiterFX \
AkPitchShifterFX \
AkRoomVerbFX \
AkSilenceSource \
AkSynthOne \
AkSineSource \
AkSoundSeedImpactFX \
AkSoundSeedWind \
AkSoundSeedWoosh \
AkStereoDelayFX \
AkStreamMgr \
AkTimeStretchFX \
AkToneSource \
AkTremoloFX \
AkVorbisDecoder \
McDSPFutzBoxFX \
McDSPLimiterFX \
AstoundsoundExpanderFX \
AstoundsoundFolddownFX \
AstoundsoundRTIFX \
AstoundsoundShared

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)