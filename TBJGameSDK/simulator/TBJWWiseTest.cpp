
#ifdef TEST_TBJWWISE
#include <AK/Comm/AkCommunication.h>
#include <AK/MusicEngine/Common/AkMusicEngine.h>
#include <AK/Plugin/AllPluginsFactories.h>
#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/SoundEngine/Common/AkQueryParameters.h>
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>
#include <AK/SoundEngine/Common/AkMemoryMgr.h>
#include <AK/SoundEngine/Common/AkModule.h>
#include <AK/SoundEngine/Common/IAkPlugin.h>
#include <AK/Tools/Common/AkMonitorError.h>
#include <AK/Tools/Common/AkPlatformFuncs.h>
#include <AK/Plugin/AkVorbisFactory.h>
#include <AK/Plugin/AkSilenceSourceFactory.h>

void TestWWise()
{
    AkCommSettings _AkCommSettings;
    AK::Comm::GetDefaultInitSettings(_AkCommSettings);
    AK::Comm::Init(_AkCommSettings);
    AK::Comm::Term();

    AK::Comm::Reset();

    AkPlatformInitSettings _AkPlatformInitSettings;
    AkInitSettings _AkInitSettings;
    AkMemPoolId _AkMemPoolId;
    AkBankID _AkBankID;
    AK::SoundEngine::GetDefaultPlatformInitSettings(_AkPlatformInitSettings);
    AK::SoundEngine::GetDefaultInitSettings(_AkInitSettings);
    AK::SoundEngine::IsInitialized();
    AK::SoundEngine::Init(&_AkInitSettings, &_AkPlatformInitSettings);
    AK::SoundEngine::Term();
    AK::SoundEngine::LoadBank(nullptr, _AkMemPoolId, _AkBankID);
    AK::SoundEngine::UnloadBank((char *)nullptr, (void *)nullptr);

    AkSwitchGroupID _AkSwitchGroupID;
    AkSwitchStateID _AkSwitchStateID;
    AkGameObjectID _AkGameObjectID;
    AkStateGroupID _AkStateGroupID;
    AkStateID _AkStateID;
    AkUniqueID _AkUniqueID;
    AkUInt32 _AkUInt32;
    AkCallbackFunc _AkCallbackFunc;
    AkPlayingID _AkPlayingID;
    AkSoundPosition _AkSoundPosition;
    AkRtpcID _AkRtpcID;
    AkRtpcValue _AkRtpcValue;
    AkTimeMs _AkTimeMs;
    AK::SoundEngine::Query::RTPCValue_type _RTPCValue_type;
    unsigned int _unsigned_int;
    unsigned long _unsigned_long;
    float _float;
    bool _bool;
    AK::SoundEngine::PreparationType _PreparationType;
    AK::SoundEngine::AkBankContent _AkBankContent;
    AkPluginType _AkPluginType;
    AkCreatePluginCallback _AkCreatePluginCallback;
    AkCreateParamCallback _AkCreateParamCallback;
    AkCreateFileSourceCallback _AkCreateFileSourceCallback;
    AkCreateBankSourceCallback _AkCreateBankSourceCallback;
    AkCurveInterpolation _AkCurveInterpolation;
    AkListenerPosition _AkListenerPosition;
    
    AK::SoundEngine::SetSwitch(_AkSwitchGroupID, _AkSwitchStateID, _AkGameObjectID);
    AK::SoundEngine::SetState(_AkStateGroupID, _AkStateID);
    AK::SoundEngine::PostEvent(_AkUniqueID, _AkGameObjectID, _AkUInt32, _AkCallbackFunc, nullptr, _AkUInt32, nullptr, _AkPlayingID);
    AK::SoundEngine::PostEvent(nullptr, _AkGameObjectID, _AkUInt32, _AkCallbackFunc, nullptr, _AkUInt32, nullptr, _AkPlayingID);
    AK::SoundEngine::GetIDFromString(nullptr);
    AK::SoundEngine::RegisterGameObj(_AkGameObjectID, nullptr, _AkUInt32);
    AK::SoundEngine::UnregisterGameObj(_AkGameObjectID);
    AK::SoundEngine::SetPosition(_AkGameObjectID, _AkSoundPosition);
    AK::SoundEngine::StopAll(_AkGameObjectID);
    AK::SoundEngine::SetGameObjectAuxSendValues(_AkGameObjectID, nullptr, _AkUInt32);
    AK::SoundEngine::SetRTPCValue(_AkRtpcID, _AkRtpcValue, _AkGameObjectID, _AkTimeMs, _AkCurveInterpolation, true);
    AK::SoundEngine::LoadBank(nullptr, 0, _unsigned_int);
    AK::SoundEngine::LoadBank(nullptr, 0, _unsigned_int);
    AK::SoundEngine::UnloadBank(_unsigned_int, nullptr, nullptr);
    AK::SoundEngine::PrepareBank(_PreparationType, nullptr, _AkBankContent);
    AK::SoundEngine::Suspend(true);
    AK::SoundEngine::WakeupFromSuspend();
    AK::SoundEngine::CancelEventCallbackCookie(nullptr);
    AK::SoundEngine::UnregisterGameObj(_unsigned_long);
    AK::SoundEngine::RegisterGameObj(_unsigned_long, _unsigned_int);
    AK::SoundEngine::SeekOnEvent(nullptr, _unsigned_long, _float, _bool, _unsigned_int);
    AK::SoundEngine::SetSwitch(_unsigned_int, _unsigned_int, _unsigned_long);
    AK::SoundEngine::PostEvent(_unsigned_int, _unsigned_long, _unsigned_int, nullptr, nullptr, _unsigned_int, nullptr, _unsigned_int);
    AK::SoundEngine::RenderAudio();
    AK::SoundEngine::SetRTPCValue( nullptr, _AkRtpcValue, _AkGameObjectID, _AkTimeMs, _AkCurveInterpolation, _bool );
    AK::SoundEngine::SetSwitch( nullptr, nullptr, _AkGameObjectID );
    AK::SoundEngine::RegisterPlugin( _AkPluginType, _AkUInt32, _AkUInt32, _AkCreatePluginCallback, _AkCreateParamCallback);
    AK::SoundEngine::SetState( nullptr, nullptr );
    AK::SoundEngine::SetListenerPosition( _AkListenerPosition, _AkUInt32 );
    AK::SoundEngine::RegisterCodec(_AkUInt32, _AkUInt32, _AkCreateFileSourceCallback, _AkCreateBankSourceCallback);
    AK::SoundEngine::CancelEventCallback(_AkPlayingID);

    AK::SoundEngine::Query::GetRTPCValue(nullptr, _unsigned_long, _float, _RTPCValue_type);
    AK::SoundEngine::Query::GetRTPCValue(_AkRtpcID, _AkGameObjectID, _AkRtpcValue, _RTPCValue_type);
    
    AkStreamMgrSettings _AkStreamMgrSettings;
    AkDeviceSettings _AkDeviceSettings;
    AK::StreamMgr::GetDefaultSettings(_AkStreamMgrSettings);
    AK::StreamMgr::GetDefaultDeviceSettings(_AkDeviceSettings);
    AK::StreamMgr::Create(_AkStreamMgrSettings);
    AK::StreamMgr::SetCurrentLanguage(nullptr);

    AK::StreamMgr::DestroyDevice(_unsigned_int);
    AK::StreamMgr::CreateDevice(_AkDeviceSettings, nullptr);
    AK::StreamMgr::GetPoolID();
    AK::StreamMgr::SetFileLocationResolver(nullptr);
    AK::StreamMgr::GetFileLocationResolver();
    AK::StreamMgr::GetCurrentLanguage();

    AkMemSettings _AkMemSettings;
    AK::MemoryMgr::Init(&_AkMemSettings);
    AK::MemoryMgr::Term();

    int _int;
    AK::MemoryMgr::DestroyPool(_int);
    AK::MemoryMgr::Malloc(_int, _unsigned_long);
    AK::MemoryMgr::Free(_int, nullptr);
    AK::MemoryMgr::ReleaseBlock(_int, nullptr);

    AkMusicSettings _AkMusicSettings;
    AK::MusicEngine::GetDefaultInitSettings(_AkMusicSettings);
    AK::MusicEngine::Init(&_AkMusicSettings);
    AK::MusicEngine::Term();

    AkSegmentInfo _AkSegmentInfo;
    AK::MusicEngine::GetPlayingSegmentInfo( _AkPlayingID, _AkSegmentInfo, _bool );
    
    AK::Monitor::ErrorLevel _ErrorLevel;

    AK::Monitor::ErrorCode _ErrorCode;
    AK::Monitor::LocalOutputFunc _LocalOutputFunc;
    AK::Monitor::PostCode( _ErrorCode, _ErrorLevel );
    AK::Monitor::PostString( nullptr, _ErrorLevel );
    AK::Monitor::SetLocalOutput( _AkUInt32, _LocalOutputFunc );
    AK::Monitor::GetTimeStamp();

    CreateVorbisBankPlugin( nullptr );
    CreateVorbisFilePlugin( nullptr );
    CreateSilenceSourceParams( nullptr );
    CreateSilenceSource( nullptr );
}
#endif
