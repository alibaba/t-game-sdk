#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>
#include <AK/SoundEngine/Common/AkModule.h>
#include <AK/SoundEngine/Common/IAkPlugin.h>
#include <AK/SoundEngine/Common/AkQueryParameters.h>
#include <AK/MusicEngine/Common/AkMusicEngine.h>
#include <AK/Tools/Common/AkMonitorError.h>
#include <AK/Tools/Common/AkPlatformFuncs.h>
#include <AK/Tools/Common/AkFNVHash.h>
#include <AK/Comm/AkCommunication.h>
#include <string>

#ifdef WIN32
#include <AkDefaultIOHookBlocking.h>
#include <JenovaAudioEngineApi.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#define SB_PATH "./bundle.bnd"
#define WWISE_BANK_POSTFIX ".bnk"
#define WWISE_BANK_INIT "Init"

namespace AK
{
	AkReal32 g_fFreqRatio = 0;
}

namespace AKPLATFORM
{
#ifdef WIN32
	void AkSleep(AkUInt32 in_ulMilliseconds)
	{
		::Sleep(in_ulMilliseconds);
	}

	void PerformanceCounter(AkInt64* out_piLastTime)
	{
		::QueryPerformanceCounter((LARGE_INTEGER*)out_piLastTime);
	}

	/// Platform Independent Helper
	void PerformanceFrequency(AkInt64* out_piFreq)
	{
		::QueryPerformanceFrequency((LARGE_INTEGER*)out_piFreq);
	}

	/// Platform Independent Helper
	void UpdatePerformanceFrequency()
	{
		AkInt64 iFreq;
		PerformanceFrequency(&iFreq);
		AK::g_fFreqRatio = (AkReal32)(iFreq / 1000);
	}

	void SafeStrCpy(wchar_t* in_pDest, const wchar_t* in_pSrc, size_t in_uDestMaxNumChars)
	{
		size_t iSizeCopy = AkMin(in_uDestMaxNumChars - 1, wcslen(in_pSrc) + 1);
		wcsncpy_s(in_pDest, in_uDestMaxNumChars, in_pSrc, iSizeCopy);
		in_pDest[iSizeCopy] = '\0';
	}

	/// Safe string copy.
	void SafeStrCpy(char* in_pDest, const char* in_pSrc, size_t in_uDestMaxNumChars)
	{
		size_t iSizeCopy = AkMin(in_uDestMaxNumChars - 1, strlen(in_pSrc) + 1);
		strncpy_s(in_pDest, in_uDestMaxNumChars, in_pSrc, iSizeCopy);
		in_pDest[iSizeCopy] = '\0';
	}
#endif

	static bool EndsWith(const std::string& mainStr, const std::string& toMatch)
	{
#ifdef WIN32
		if (mainStr.size() >= toMatch.size() &&
			mainStr.compare(mainStr.size() - toMatch.size(), toMatch.size(), toMatch) == 0)
			return true;
#endif
		return false;
	}
}

namespace AK
{
	namespace Comm
	{
		void GetDefaultInitSettings(AkCommSettings&) {}
		AKRESULT Init(AkCommSettings const&) { return AK_Success; }
		void Term(void) {}

		AKRESULT Reset() { return AK_Fail; }
	}

	namespace SoundEngine
	{
#ifdef WIN32
		static JenovaAudioEngineImpl* jenovaAudioEngineImpl = nullptr;

		static bool JenovaAudioEngineLoadBankCallbackFunc(JENOVA_ID bankID) { return true; }
#endif

		void GetDefaultPlatformInitSettings(AkPlatformInitSettings&) {}
		void GetDefaultInitSettings(AkInitSettings&) {}
		bool IsInitialized(void) { return true; }

		AKRESULT Init(AkInitSettings*, AkPlatformInitSettings*)
		{
#ifdef WIN32
			JENOVA_AUDIO_ERRORNO res = JenovaAudioEngineInit(&jenovaAudioEngineImpl, 0, nullptr);
			if (JENOVA_AUDIO_ENGINE_SUCCESS != res) return AK_Fail;

			res = JenovaAudioEngineLoadBundle(jenovaAudioEngineImpl, strlen(SB_PATH), SB_PATH);
			if (JENOVA_AUDIO_ENGINE_SUCCESS != res) return AK_Fail;

			//res = JenovaAudioEngineStart(jenovaAudioEngineImpl);
			//if (JENOVA_AUDIO_ENGINE_SUCCESS != res) return AK_Fail;

			//res = JenovaAudioEngineResume(jenovaAudioEngineImpl);
			//if (JENOVA_AUDIO_ENGINE_SUCCESS != res) return AK_Fail;
#endif

			return AK_Success;
		}

		void Term(void)
		{
			//JenovaAudioEngineDeinit(jenovaAudioEngineImpl);
		}

		AKRESULT LoadBank(char const* bank_name, AkBankID&)
		{
#ifdef WIN32
			std::string jbank_name = bank_name;
			if (AKPLATFORM::EndsWith(jbank_name, WWISE_BANK_POSTFIX))
				jbank_name = jbank_name.substr(0, jbank_name.length() - strlen(WWISE_BANK_POSTFIX));
			JENOVA_AUDIO_ERRORNO res = JenovaAudioEngineLoadBank(jenovaAudioEngineImpl, GetIDFromString(jbank_name.c_str()), JenovaAudioEngineLoadBankCallbackFunc);
			if (JENOVA_AUDIO_ENGINE_SUCCESS != res)
			{
				if (0 == strcmp(jbank_name.c_str(), WWISE_BANK_INIT))
					return AK_Success;
				else
					return AK_Fail;
			}
#endif
			return AK_Success;
		}
		AKRESULT UnloadBank(char const* bank_name, void const*, AkMemPoolId *)
		{
#ifdef WIN32
			JENOVA_AUDIO_ERRORNO res = JenovaAudioEngineUnloadBank(jenovaAudioEngineImpl, GetIDFromString(bank_name));
			if (JENOVA_AUDIO_ENGINE_SUCCESS != res) return AK_Fail;
#endif
			return AK_Success;
		}

		AKRESULT SetSwitch(AkSwitchGroupID in_switchGroup, AkSwitchStateID in_switchState, AkGameObjectID in_gameObjectID)
		{
#ifdef WIN32
			JENOVA_AUDIO_ERRORNO res = JenovaAudioEngineSetSwitch(jenovaAudioEngineImpl, in_switchGroup, in_switchState, in_gameObjectID);
			if (JENOVA_AUDIO_ENGINE_SUCCESS != res) return AK_Fail;
#endif
			return AK_Success;
		}
		AKRESULT SetState(AkStateGroupID in_stateGroup, AkStateID in_state)
		{
#ifdef WIN32
			JENOVA_AUDIO_ERRORNO res = JenovaAudioEngineSetSwitch(jenovaAudioEngineImpl, in_stateGroup, in_state, 0);
			if (JENOVA_AUDIO_ENGINE_SUCCESS != res) return AK_Fail;
#endif
			return AK_Success;
		}

		AkPlayingID PostEvent(AkUniqueID in_eventID, AkGameObjectID in_gameObjectID, AkUInt32 in_uFlags, AkCallbackFunc in_pfnCallback, void* in_pCookie, AkUInt32 in_cExternals, AkExternalSourceInfo* in_pExternalSources, AkPlayingID in_PlayingID)
		{
#ifdef WIN32
			JENOVA_AUDIO_ERRORNO res = JenovaAudioEnginePostEvent(jenovaAudioEngineImpl, in_eventID, in_gameObjectID);
			if (JENOVA_AUDIO_ENGINE_SUCCESS != res) return 0;
#endif
			return 0;
		}

		AkPlayingID PostEvent( const char* in_pszEventName, AkGameObjectID in_gameObjectID, AkUInt32 in_uFlags, AkCallbackFunc in_pfnCallback, void * in_pCookie, AkUInt32 in_cExternals, AkExternalSourceInfo *in_pExternalSources, AkPlayingID in_PlayingID )
		{
#ifdef WIN32
			JENOVA_AUDIO_ERRORNO res = JenovaAudioEnginePostEvent(jenovaAudioEngineImpl, GetIDFromString(in_pszEventName), in_gameObjectID);
			if (JENOVA_AUDIO_ENGINE_SUCCESS != res) return 0;
#endif
			return 0;
		}

		AKRESULT RenderAudio(bool)
		{
			//JENOVA_AUDIO_ERRORNO res = JenovaAudioEngineRenderAudio(jenovaAudioEngineImpl);
			//if (JENOVA_AUDIO_ENGINE_SUCCESS != res) return AK_Fail;
			return AK_Success;
		}

		AkUInt32 GetIDFromString(char const* str)
		{
			AK::FNVHash32 hash32;
			AkUInt32 const result = hash32.Compute(str, strlen(str));
			return result;
		}

		AKRESULT SetDefaultListeners(const AkGameObjectID* in_pListenerObjs, AkUInt32 in_uNumListeners) { return AK_Success; }
		AKRESULT RegisterGameObj(AkGameObjectID in_gameObjectID, const char * in_pszObjName, AkUInt32 in_uListenerMask) { return AK_Success; }
		AKRESULT UnregisterGameObj(AkGameObjectID in_gameObjectID) { return AK_Success; }
		AKRESULT SetPosition(AkGameObjectID in_GameObjectID, const AkSoundPosition & in_Position) { return AK_Success; }
		void StopAll(AkGameObjectID in_gameObjectID) {}
		AKRESULT SetGameObjectAuxSendValues(AkGameObjectID in_gameObjectID, AkAuxSendValue* in_aAuxSendValues, AkUInt32            in_uNumSendValues) { return AK_Success; }
		AKRESULT SetRTPCValue(AkRtpcID in_rtpcID, AkRtpcValue in_value, AkGameObjectID in_gameObjectID, AkTimeMs in_uValueChangeDuration, AkCurveInterpolation in_eFadeCurve, bool in_bBypassInternalValueInterpolation) { return AK_Success; }

		AKRESULT LoadBank(void const* in_pszString, unsigned int in_memPoolId, unsigned int& out_bankID) { return AK_Fail; } 
        AKRESULT LoadBank(char const*, int, unsigned int&) { return AK_Fail; } 
        AKRESULT UnloadBank(unsigned int, void const*, int*) { return AK_Success; }
        AKRESULT PrepareBank(PreparationType, char const*, AkBankContent) { return AK_Fail; } 
        AKRESULT Suspend(bool in_bRenderAnyway) { return AK_Fail; } 
        AKRESULT WakeupFromSuspend() { return AK_Fail; } 
        void CancelEventCallbackCookie(void*) { } 
        AKRESULT RegisterGameObj(AkGameObjectID in_gameObjectID, AkUInt32 in_uListenerMask) { return AK_Fail; }
        AKRESULT SeekOnEvent(const char* in_pszEventName, AkGameObjectID in_gameObjectID, AkReal32 in_fPercent, bool in_bSeekToNearestMarker, AkPlayingID in_PlayingID) { return AK_Fail; }
        AKRESULT RenderAudio() { return AK_Fail; } 
        AKRESULT SetRTPCValue( const char* in_pszRtpcName, AkRtpcValue in_value, AkGameObjectID in_gameObjectID, AkTimeMs in_uValueChangeDuration, AkCurveInterpolation in_eFadeCurve, bool in_bBypassInternalValueInterpolation ) { return AK_Fail; } 
        AKRESULT SetSwitch( const char* in_pszSwitchGroup, const char* in_pszSwitchState, AkGameObjectID in_gameObjectID ) { return AK_Fail; } 
        AKRESULT RegisterPlugin(AkPluginType in_eType, AkUInt32 in_ulCompanyID, AkUInt32 in_ulPluginID, AkCreatePluginCallback in_pCreateFunc, AkCreateParamCallback in_pCreateParamFunc) {return AK_Fail;}
        AKRESULT SetState( const char* in_pszStateGroup, const char* in_pszState ) { return AK_Fail; } 
        AKRESULT SetListenerPosition( const AkListenerPosition & in_Position, AkUInt32 in_uIndex ) { return AK_Fail; } 
        AKRESULT RegisterCodec( AkUInt32 in_ulCompanyID, AkUInt32 in_ulCodecID, AkCreateFileSourceCallback in_pFileCreateFunc, AkCreateBankSourceCallback in_pBankCreateFunc  ) { return AK_Fail; }
		void CancelEventCallback(AkPlayingID in_playingID) { }
    
        namespace Query
        {
            AKRESULT GetRTPCValue(const char* in_pszRtpcName, AkGameObjectID in_gameObjectID, AkRtpcValue& out_rValue, RTPCValue_type&    io_rValueType) { return AK_Fail; }
			AKRESULT GetRTPCValue(AkRtpcID in_rtpcID, AkGameObjectID in_gameObjectID, AkRtpcValue& out_rValue, RTPCValue_type& io_rValueType) { return AK_Fail; }
        }
	}

	namespace StreamMgr
	{
		void GetDefaultSettings(AkStreamMgrSettings&) {}
		void GetDefaultDeviceSettings(AkDeviceSettings&) {}
		IAkStreamMgr* Create(AkStreamMgrSettings const&) { return nullptr; }
		AKRESULT SetCurrentLanguage(char const*) { return AK_Success; }

		AKRESULT DestroyDevice(unsigned int) { return AK_Success; } 
        AkDeviceID CreateDevice(AkDeviceSettings const&, IAkLowLevelIOHook*) { return 0; } 
        AkMemPoolId GetPoolID() { return 0; } 
        void SetFileLocationResolver(IAkFileLocationResolver*) {}
        IAkFileLocationResolver* GetFileLocationResolver() {return nullptr; }
		const AkOSChar *GetCurrentLanguage() { return "en"; }
	}

	namespace MemoryMgr
	{
		AKRESULT Init(AkMemSettings*) { return AK_Success; }
		void GetDefaultSettings(AkMemSettings&) {}
		void Term(void) {}

		AKRESULT DestroyPool(int) { return AK_Success; } 
        void* Malloc(int, unsigned long) { return nullptr; } 
        AKRESULT Free(int, void*) { return AK_Success; } 
        AKRESULT ReleaseBlock(int, void*) { return AK_Success; }
	}

	namespace MusicEngine
	{
		void GetDefaultInitSettings(AkMusicSettings&) {}
		AKRESULT Init(AkMusicSettings*) { return AK_Success; }
		void Term(void) {}

		AKRESULT GetPlayingSegmentInfo( AkPlayingID in_PlayingID, AkSegmentInfo & out_segmentInfo, bool in_bExtrapolate ) { return AK_Fail; }
	}

	namespace Monitor
	{
		AKRESULT PostString(const char* in_pszError, ErrorLevel in_eErrorLevel, AkPlayingID in_playingID, AkGameObjectID in_gameObjID, AkUniqueID in_audioNodeID, bool in_bIsBus) { return AK_Success; }

		const AkOSChar* s_aszErrorCodes[ Num_ErrorCodes ];
		AKRESULT PostCode( ErrorCode in_eError, ErrorLevel in_eErrorLevel ) { return AK_Fail; }
		AKRESULT PostString( const char* in_pszError, ErrorLevel in_eErrorLevel ) { return AK_Fail; }
		AKRESULT SetLocalOutput( AkUInt32 in_uErrorLevel, LocalOutputFunc in_pMonitorFunc ) { return AK_Fail; }
		AkTimeMs GetTimeStamp() { return 0; }
	}

	IAkStreamMgr * IAkStreamMgr::m_pStreamMgr = NULL;
	//void IAkStreamMgr::Destroy() {}
}

#ifdef WIN32
AKRESULT CAkDefaultIOHookBlocking::Init(AkDeviceSettings const&, bool) { return AK_Success; }
void CAkDefaultIOHookBlocking::Term(void) {}
AKRESULT CAkDefaultIOHookBlocking::SetBasePath(char const*) { return AK_Success; }
#endif

IAkSoftwareCodec* CreateVorbisBankPlugin( void* in_pCtx ) { return nullptr; }
IAkSoftwareCodec* CreateVorbisFilePlugin( void* in_pCtx ) { return nullptr; }
AK::IAkPluginParam * CreateSilenceSourceParams( AK::IAkPluginMemAlloc * in_pAllocator ) { return nullptr; }
AK::IAkPlugin* CreateSilenceSource( AK::IAkPluginMemAlloc * in_pAllocator ) { return nullptr; }
