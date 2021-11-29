emcc `
../source/StreamManager/Common/AkStreamMgr.cpp `
../samples/IntegrationDemo/Android/jni/main.cpp `
../samples/IntegrationDemo/Common/IntegrationDemo.cpp `
../samples/SoundEngine/Android/AkDefaultIOHookBlocking.cpp `
../samples/SoundEngine/Common/AkFilePackage.cpp `
-o main.wasm `
-g3 `
-O0 `
-std=c++14 `
-I../samples/IntegrationDemo/Android `
-I../source/StreamManager/Android `
-I../include `
-I../source/StreamManager/POSIX `
-I../samples/SoundEngine/POSIX `
-I../samples/IntegrationDemo/Common `
-I../samples/IntegrationDemo/DemoPages `
-I../samples/IntegrationDemo/MenuSystem `
-I../samples/IntegrationDemo/WwiseProject/GeneratedSoundBanks `
-I../samples/IntegrationDemo/FreetypeRenderer `
-I../samples/IntegrationDemo/FreetypeRenderer/Assets `
-s ERROR_ON_UNDEFINED_SYMBOLS=0 `
-s WARN_ON_UNDEFINED_SYMBOLS=1 `
-s LINKABLE=1 `
-s EXPORT_ALL=1 `
-s EXPORTED_FUNCTIONS="[]" `
