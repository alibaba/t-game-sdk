# Android.mk
# Copyright (C) 2011 Audiokinetic Inc.
# \file 
# Makefile for building the integration demo application.

APP_PLATFORM := $(ANDROID_PLATFORM)
APP_STL := gnustl_static
APP_ABI := armeabi-v7a x86 arm64-v8a x86_64
APP_CFLAGS += -Wno-error=format-security