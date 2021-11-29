#! /usr/bin/bash
SDK_ROOT=`dirname $0`
SDK_ROOT=`cygpath -a -d -m $SDK_ROOT`
if [ ! -d ${SDK_ROOT}/../Build/Android/android-9_armeabi-v7a ]; then
	 ${SDK_ROOT}/../Build/Android/MakeStandaloneToolchain.sh;
fi
if [ ! -d ${SDK_ROOT}/../Build/Android/android-9_armeabi-v7a-hf ]; then
	 ${SDK_ROOT}/../Build/Android/MakeStandaloneToolchain.sh;
fi
if [ ! -d ${SDK_ROOT}/../Build/Android/android-9_x86 ]; then
	 ${SDK_ROOT}/../Build/Android/MakeStandaloneToolchain.sh;
fi
SDK_ROOT=$SDK_ROOT/../../
make -f AllAndroidSoundEngine.make $1 WWISE_SDK_ROOT="$SDK_ROOT" ANDROID_PLATFORM=android-9 ANDROID_ARCH=armeabi-v7a config=debug_armeabi-v7a
make -f AllAndroidSoundEngine.make $1 WWISE_SDK_ROOT="$SDK_ROOT" ANDROID_PLATFORM=android-9 ANDROID_ARCH=armeabi-v7a config=profile_armeabi-v7a
make -f AllAndroidSoundEngine.make $1 WWISE_SDK_ROOT="$SDK_ROOT" ANDROID_PLATFORM=android-9 ANDROID_ARCH=armeabi-v7a config=release_armeabi-v7a

make -f AllAndroidSoundEngine.make $1 WWISE_SDK_ROOT="$SDK_ROOT" ANDROID_PLATFORM=android-9 ANDROID_ARCH=armeabi-v7a HF=-hf config=debug_armeabi-v7a
make -f AllAndroidSoundEngine.make $1 WWISE_SDK_ROOT="$SDK_ROOT" ANDROID_PLATFORM=android-9 ANDROID_ARCH=armeabi-v7a HF=-hf config=profile_armeabi-v7a
make -f AllAndroidSoundEngine.make $1 WWISE_SDK_ROOT="$SDK_ROOT" ANDROID_PLATFORM=android-9 ANDROID_ARCH=armeabi-v7a HF=-hf config=release_armeabi-v7a

make -f AllAndroidSoundEngine.make $1 WWISE_SDK_ROOT="$SDK_ROOT" ANDROID_PLATFORM=android-9 ANDROID_ARCH=x86 config=debug_x86
make -f AllAndroidSoundEngine.make $1 WWISE_SDK_ROOT="$SDK_ROOT" ANDROID_PLATFORM=android-9 ANDROID_ARCH=x86 config=profile_x86
make -f AllAndroidSoundEngine.make $1 WWISE_SDK_ROOT="$SDK_ROOT" ANDROID_PLATFORM=android-9 ANDROID_ARCH=x86 config=release_x86