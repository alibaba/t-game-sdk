#define ENABLE_TRACE 1

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
#include <algorithm>
#include <cctype>

#define MAX_STRING 4096

#ifdef WIN32
#include <AkDefaultIOHookBlocking.h>
#include <JenovaAudioEngineApi.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <map>

static int trace_va(char const * const format, va_list args)
{
	char buffer[MAX_STRING];
	int const len = vsnprintf(buffer, sizeof(buffer), format, args);
	OutputDebugStringA(buffer);
	return len;
}

static int trace_a(char const * const format, ...)
{
	va_list args;
	va_start(args, format);
	int const len = trace_va(format, args);
	va_end(args);
	return len;
}

#define Trace(format, ...) trace_a(format, __VA_ARGS__)
#define ENABLE_JENOVA 1

#else

#define Trace printf
#define ENABLE_JENOVA 0

#endif

#if ENABLE_TRACE
#ifdef WIN32
#define TraceFLF(format, ...) Trace("[JenovaAudioEngine], %s, %4u, %8ju, %64s" format "\n", __FILE__, __LINE__, (uintmax_t)clock(), __FUNCTION__, __VA_ARGS__)
#else
#define TraceFLF(format, ...) Trace("[JenovaAudioEngine], %s, %4u, %8ju, %64s" format "\n", __FILE__, __LINE__, (uintmax_t)clock(), __FUNCTION__,##__VA_ARGS__)
#endif

#define ARRAY_TO_STRING(out, sig, express, count) \
{ \
	for(unsigned int ii = 0 ; ii < (unsigned int)(count) ; ++ii) \
		sprintf(out, "%s, " sig, out, express); \
}

static char const* BoolToString(bool const v)
{
	return v ? "true" : "false";
}

#else
#define TraceFLF(format, ...)
#define ARRAY_TO_STRING(out, sig, express, count)
static char const* BoolToString(bool const v) {return NULL;}

#endif

#define ROOT_PATH "./"
#define SB_PATH "./bundle.bnd"
#define WWISE_BANK_POSTFIX ".bnk"
#define WWISE_BANK_INIT "Init"

namespace AK
{
	AkReal32 g_fFreqRatio = 0;
}

namespace AKPLATFORM
{
	static bool EndsWith(const std::string& mainStr, const std::string& toMatch)
	{
#if ENABLE_JENOVA
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
		void GetDefaultInitSettings(AkCommSettings&) { TraceFLF(""); }
		AKRESULT Init(AkCommSettings const&) { TraceFLF(""); return AK_Success; }
		void Term(void) { TraceFLF(""); }

		AKRESULT Reset() { TraceFLF(""); return AK_Success; }
	}

	namespace SoundEngine
	{
#if ENABLE_JENOVA
		static JenovaAudioEngineImpl* jenovaAudioEngineImpl = nullptr;
		static std::map<AkGameObjectID, JENOVA_ID> _gameobject_map;

		static bool JenovaAudioEngineLoadBankCallbackFunc(JENOVA_ID bankID) { TraceFLF(""); return true; }
#endif

		void GetDefaultPlatformInitSettings(AkPlatformInitSettings&) { TraceFLF(""); }
		void GetDefaultInitSettings(AkInitSettings&) { TraceFLF(""); }
		bool IsInitialized(void) { TraceFLF(""); return true; }

		AKRESULT Init(AkInitSettings*, AkPlatformInitSettings*)
		{
			TraceFLF(", CLOCKS_PER_SEC%8ju", (uintmax_t)CLOCKS_PER_SEC);
#if ENABLE_JENOVA
			_gameobject_map[0] = 0;
			JENOVA_AUDIO_ERRORNO res = JenovaAudioEngineInit(&jenovaAudioEngineImpl, strlen(ROOT_PATH), ROOT_PATH);
			if (JENOVA_AUDIO_ENGINE_SUCCESS != res) return AK_Success;

			res = JenovaAudioEngineLoadBundle(jenovaAudioEngineImpl, strlen(SB_PATH), SB_PATH);
			if (JENOVA_AUDIO_ENGINE_SUCCESS != res) return AK_Success;

			res = JenovaAudioEngineStart(jenovaAudioEngineImpl);
			if (JENOVA_AUDIO_ENGINE_SUCCESS != res) return AK_Success;

			res = JenovaAudioEngineResume(jenovaAudioEngineImpl);
			if (JENOVA_AUDIO_ENGINE_SUCCESS != res) return AK_Success;
#endif

			return AK_Success;
		}

		void Term(void)
		{
			TraceFLF(""); 
#if ENABLE_JENOVA
			JenovaAudioEngineDeinit(jenovaAudioEngineImpl);
#endif
		}

		AKRESULT LoadBank(char const* in_pszString, AkBankID& out_bankID)
		{
			TraceFLF(", '%s'", in_pszString); 
#if ENABLE_JENOVA
			std::string jbank_name = in_pszString;
			if (AKPLATFORM::EndsWith(jbank_name, WWISE_BANK_POSTFIX))
				jbank_name = jbank_name.substr(0, jbank_name.length() - strlen(WWISE_BANK_POSTFIX));
			out_bankID = GetIDFromString(jbank_name.c_str());
			JENOVA_AUDIO_ERRORNO res = JenovaAudioEngineLoadBank(jenovaAudioEngineImpl, out_bankID, JenovaAudioEngineLoadBankCallbackFunc);
			if (JENOVA_AUDIO_ENGINE_SUCCESS != res)
			{
				if (0 == strcmp(jbank_name.c_str(), WWISE_BANK_INIT))
					return AK_Success;
				else
					return AK_Success;
			}
#endif
			return AK_Success;
		}
        AKRESULT LoadBank(const char* in_pszString, AkMemPoolId in_memPoolId, AkBankID & out_bankID)
		{
			TraceFLF(", '%s', %d", in_pszString, in_memPoolId); 
#if ENABLE_JENOVA
			std::string jbank_name = in_pszString;
			if (AKPLATFORM::EndsWith(jbank_name, WWISE_BANK_POSTFIX))
				jbank_name = jbank_name.substr(0, jbank_name.length() - strlen(WWISE_BANK_POSTFIX));
			out_bankID = GetIDFromString(jbank_name.c_str());
			JENOVA_AUDIO_ERRORNO res = JenovaAudioEngineLoadBank(jenovaAudioEngineImpl, out_bankID, JenovaAudioEngineLoadBankCallbackFunc);
			if (JENOVA_AUDIO_ENGINE_SUCCESS != res)
			{
				if (0 == strcmp(jbank_name.c_str(), WWISE_BANK_INIT))
					return AK_Success;
				else
					return AK_Success;
			}
#endif
			return AK_Success;
		}
    
        AKRESULT LoadBank(const void* buffer, int bufferSize, AkBankID& out_bankID)
        {
            TraceFLF(", '%d', return bankid %d", bufferSize, out_bankID);
            return AK_Success;
        }
    
		AKRESULT UnloadBank(char const* bank_name, void const* pa, AkMemPoolId * pb)
		{
			TraceFLF(", '%s', %p, %p", bank_name, pa, pb); 
#if ENABLE_JENOVA
			JENOVA_AUDIO_ERRORNO res = JenovaAudioEngineUnloadBank(jenovaAudioEngineImpl, GetIDFromString(bank_name));
			if (JENOVA_AUDIO_ENGINE_SUCCESS != res) return AK_Success;
#endif
			return AK_Success;
		}

		AKRESULT SetSwitch(AkSwitchGroupID in_switchGroup, AkSwitchStateID in_switchState, AkGameObjectID in_gameObjectID)
		{
			TraceFLF(", %u, %u, %u", in_switchGroup, in_switchState, in_gameObjectID); 
#if ENABLE_JENOVA
			JENOVA_AUDIO_ERRORNO res = JenovaAudioEngineSetSwitch(jenovaAudioEngineImpl, in_switchGroup, in_switchState, _gameobject_map[in_gameObjectID]);
			if (JENOVA_AUDIO_ENGINE_SUCCESS != res) return AK_Success;
#endif
			return AK_Success;
		}
		AKRESULT SetState(AkStateGroupID in_stateGroup, AkStateID in_state)
		{
			TraceFLF(", %u, %u", in_stateGroup, in_state);
#if ENABLE_JENOVA
			JENOVA_AUDIO_ERRORNO res = JenovaAudioEngineSetSwitch(jenovaAudioEngineImpl, in_stateGroup, in_state, 0);
			if (JENOVA_AUDIO_ENGINE_SUCCESS != res) return AK_Success;
#endif
			return AK_Success;
		}

		AkPlayingID PostEvent(AkUniqueID in_eventID, AkGameObjectID in_gameObjectID, AkUInt32 in_uFlags
			, AkCallbackFunc in_pfnCallback, void* in_pCookie
			, AkUInt32 in_cExternals, AkExternalSourceInfo* in_pExternalSources, AkPlayingID in_PlayingID)
		{
			TraceFLF(", %u, %u, %u, %p, %p, %u, %p, %u"
				, in_eventID, in_gameObjectID, in_uFlags
				, in_pfnCallback, in_pCookie
				, in_cExternals, in_pExternalSources, in_PlayingID); 
#if ENABLE_JENOVA
			JENOVA_AUDIO_ERRORNO res = JenovaAudioEnginePostEvent(jenovaAudioEngineImpl, in_eventID, _gameobject_map[in_gameObjectID]);
			if (JENOVA_AUDIO_ENGINE_SUCCESS != res) return 0;
#endif
			return 0;
		}

		AkPlayingID PostEvent( const char* in_pszEventName, AkGameObjectID in_gameObjectID, AkUInt32 in_uFlags
			, AkCallbackFunc in_pfnCallback, void * in_pCookie
			, AkUInt32 in_cExternals, AkExternalSourceInfo *in_pExternalSources, AkPlayingID in_PlayingID )
		{
			TraceFLF(", '%s', %u, %u, %p, %p, %u, %p, %u"
				, in_pszEventName, in_gameObjectID, in_uFlags
				, in_pfnCallback, in_pCookie
				, in_cExternals, in_pExternalSources, in_PlayingID); 
#if ENABLE_JENOVA
			JENOVA_AUDIO_ERRORNO res = JenovaAudioEnginePostEvent(jenovaAudioEngineImpl, GetIDFromString(in_pszEventName), _gameobject_map[in_gameObjectID]);
			if (JENOVA_AUDIO_ENGINE_SUCCESS != res) return 0;
#endif
			return 0;
		}

		AKRESULT RenderAudio(bool)
		{
			TraceFLF(""); 
#if ENABLE_JENOVA
			JENOVA_AUDIO_ERRORNO res = JenovaAudioEngineRenderAudio(jenovaAudioEngineImpl);
			if (JENOVA_AUDIO_ENGINE_SUCCESS != res) return AK_Success;
#endif
			return AK_Success;
		}

		AkUInt32 GetIDFromString(char const* str)
		{
			TraceFLF(", '%s'", str); 
			AK::FNVHash32 hash32;
			std::string lstr = str;
			std::transform(lstr.begin(), lstr.end(), lstr.begin(), [](unsigned char c){ return std::tolower(c); });
			AkUInt32 const result = hash32.Compute(lstr.c_str(), lstr.length());
			return result;
		}

		AKRESULT SetDefaultListeners(const AkGameObjectID* in_pListenerObjs, AkUInt32 in_uNumListeners)
		{
			char lo_str[MAX_STRING] = "";
			ARRAY_TO_STRING(lo_str, "%u", in_pListenerObjs[ii], in_uNumListeners);
			TraceFLF(", %u [%s]", in_pListenerObjs[0], lo_str);
			return AK_Success;
		}
		AKRESULT RegisterGameObj(AkGameObjectID in_gameObjectID, const char * in_pszObjName, AkUInt32 in_uListenerMask)
		{ 
			TraceFLF(", %u, '%s', %u", in_gameObjectID, in_pszObjName, in_uListenerMask); 
#if ENABLE_JENOVA
			JENOVA_ID jid = JenovaAudioEngineRegisterGameObject(jenovaAudioEngineImpl, strlen(in_pszObjName), in_pszObjName);
			_gameobject_map[in_gameObjectID] = jid;
#endif
			return AK_Success;
		}
		AKRESULT UnregisterGameObj(AkGameObjectID in_gameObjectID)
		{ 
			TraceFLF(", %u", in_gameObjectID); 
#if ENABLE_JENOVA
			JenovaAudioEngineUnregisterGameObject(jenovaAudioEngineImpl, _gameobject_map[in_gameObjectID]);
			_gameobject_map.erase(in_gameObjectID);
#endif
			return AK_Success;
		}
		AKRESULT SetPosition(AkGameObjectID in_GameObjectID, const AkSoundPosition & in_Position)
		{
			TraceFLF(", %u, (%f, %f, %f)-(%f, %f, %f)"
				, in_GameObjectID
				, in_Position.Position.X, in_Position.Position.Y, in_Position.Position.Z
				, in_Position.Orientation.X, in_Position.Orientation.Y, in_Position.Orientation.Z);
			return AK_Success;
		}
		void StopAll(AkGameObjectID in_gameObjectID) { TraceFLF(", %u", in_gameObjectID); }
		AKRESULT SetGameObjectAuxSendValues(AkGameObjectID in_gameObjectID, AkAuxSendValue* in_aAuxSendValues, AkUInt32 in_uNumSendValues)
		{
			char aasv_str[MAX_STRING] = "";
			ARRAY_TO_STRING(aasv_str, "%u", in_aAuxSendValues[ii].auxBusID, in_uNumSendValues);
			ARRAY_TO_STRING(aasv_str, "%f", in_aAuxSendValues[ii].fControlValue, in_uNumSendValues);
			TraceFLF(", %u [%s]", in_gameObjectID, aasv_str); return AK_Success;
		}
		AKRESULT SetRTPCValue(AkRtpcID in_rtpcID, AkRtpcValue in_value
			, AkGameObjectID in_gameObjectID
			, AkTimeMs in_uValueChangeDuration, AkCurveInterpolation in_eFadeCurve
			, bool in_bBypassInternalValueInterpolation)
		{
			TraceFLF(", %u, %f, %u, %d, %u, %s"
				, in_rtpcID, in_value
				, in_gameObjectID
				, in_uValueChangeDuration, in_eFadeCurve
				, BoolToString(in_bBypassInternalValueInterpolation));
			return AK_Success;
		}

        AKRESULT UnloadBank(AkBankID bankId, void const* pa, int* pb) { TraceFLF(", %u, %p, %p", bankId, pa, pb); return AK_Success; }
        AKRESULT PrepareBank(PreparationType pt, char const* n, AkBankContent bc) { TraceFLF("%u, '%s', %u", pt, n, bc); return AK_Success; } 
        AKRESULT Suspend(bool in_bRenderAnyway) { TraceFLF(", %s", BoolToString(in_bRenderAnyway)); return AK_Success; } 
        AKRESULT WakeupFromSuspend() { TraceFLF(""); return AK_Success; } 
        void CancelEventCallbackCookie(void* p) { TraceFLF(", %p", p); } 
        AKRESULT RegisterGameObj(AkGameObjectID in_gameObjectID, AkUInt32 in_uListenerMask) { TraceFLF(", %u, %u", in_gameObjectID, in_uListenerMask); return AK_Success; }
        AKRESULT SeekOnEvent(const char* in_pszEventName, AkGameObjectID in_gameObjectID, AkReal32 in_fPercent, bool in_bSeekToNearestMarker, AkPlayingID in_PlayingID)
		{
			TraceFLF(", '%s', %u, %f, %s, %u", in_pszEventName, in_gameObjectID, in_fPercent, BoolToString(in_bSeekToNearestMarker), in_PlayingID);
			return AK_Success;
		}
        AKRESULT RenderAudio() { TraceFLF(""); return AK_Success; } 
        AKRESULT SetRTPCValue( const char* in_pszRtpcName, AkRtpcValue in_value, AkGameObjectID in_gameObjectID, AkTimeMs in_uValueChangeDuration, AkCurveInterpolation in_eFadeCurve, bool in_bBypassInternalValueInterpolation )
		{
			TraceFLF(", '%s', %f, %u, %d, %u, %s"
				, in_pszRtpcName, in_value
				, in_gameObjectID
				, in_uValueChangeDuration, in_eFadeCurve
				, BoolToString(in_bBypassInternalValueInterpolation));
			return AK_Success;
		} 
        AKRESULT SetSwitch( const char* in_pszSwitchGroup, const char* in_pszSwitchState, AkGameObjectID in_gameObjectID )
		{
			TraceFLF(", '%s', '%s', %u", in_pszSwitchGroup, in_pszSwitchState, in_gameObjectID); 
			return AK_Success;
		} 
        AKRESULT RegisterPlugin(AkPluginType in_eType, AkUInt32 in_ulCompanyID, AkUInt32 in_ulPluginID, AkCreatePluginCallback in_pCreateFunc, AkCreateParamCallback in_pCreateParamFunc)
		{
			TraceFLF(", %u, %u, %u, %p, %p", in_eType, in_ulCompanyID, in_ulPluginID, in_pCreateFunc, in_pCreateParamFunc);
			return AK_Success;
		}
        AKRESULT SetState( const char* in_pszStateGroup, const char* in_pszState ) { TraceFLF(", '%s', '%s'", in_pszStateGroup, in_pszState); return AK_Success; } 
        AKRESULT SetListenerPosition( const AkListenerPosition & in_Position, AkUInt32 in_uIndex )
		{
			TraceFLF(", (%f, %f, %f)-(%f, %f, %f)-(%f, %f, %f), %u"
				, in_Position.Position.X, in_Position.Position.Y, in_Position.Position.Z
				, in_Position.OrientationFront.X, in_Position.OrientationFront.Y, in_Position.OrientationFront.Z
				, in_Position.OrientationTop.X, in_Position.OrientationTop.Y, in_Position.OrientationTop.Z
				, in_uIndex);
			return AK_Success;
		} 
        AKRESULT RegisterCodec( AkUInt32 in_ulCompanyID, AkUInt32 in_ulCodecID, AkCreateFileSourceCallback in_pFileCreateFunc, AkCreateBankSourceCallback in_pBankCreateFunc  )
		{
			TraceFLF(", %u, %u, %p, %p", in_ulCompanyID, in_ulCodecID, in_pFileCreateFunc, in_pBankCreateFunc);
			return AK_Success;
		}
		void CancelEventCallback(AkPlayingID in_playingID) { TraceFLF(", %u", in_playingID); }
    
        namespace Query
        {
            AKRESULT GetRTPCValue(const char* in_pszRtpcName, AkGameObjectID in_gameObjectID, AkRtpcValue& out_rValue, RTPCValue_type& io_rValueType)
			{
				TraceFLF(", '%s', %u, %u", in_pszRtpcName, in_gameObjectID, io_rValueType);
				return AK_Success;
			}
			AKRESULT GetRTPCValue(AkRtpcID in_rtpcID, AkGameObjectID in_gameObjectID, AkRtpcValue& out_rValue, RTPCValue_type& io_rValueType)
			{
				TraceFLF(", %u, %u, %u", in_rtpcID, in_gameObjectID, io_rValueType);
				return AK_Success;
			}
        }
	}

	namespace StreamMgr
	{
		class AkStreamMgr : public IAkStreamMgr
		{
		public:
			inline static IAkStreamMgr * Get()
			{
				TraceFLF(""); 
				return m_pStreamMgr;
			}
			virtual void Destroy() { TraceFLF(""); }
			virtual IAkStreamMgrProfile * GetStreamMgrProfile() { TraceFLF(""); return nullptr;}
			virtual AKRESULT CreateStd( const AkOSChar* in_pszFileName, AkFileSystemFlags * in_pFSFlags, AkOpenMode in_eOpenMode, IAkStdStream *& out_pStream, bool in_bSyncOpen )
			{
				TraceFLF(", '%s', %u, %s", in_pszFileName, in_eOpenMode, BoolToString(in_bSyncOpen));
				return AK_Success;
			}
			virtual AKRESULT CreateStd( AkFileID in_fileID, AkFileSystemFlags * in_pFSFlags, AkOpenMode in_eOpenMode, IAkStdStream *& out_pStream, bool in_bSyncOpen )
			{
				TraceFLF(", %u, %u, %s", in_fileID, in_eOpenMode, BoolToString(in_bSyncOpen));
				return AK_Success;
			}
			virtual AKRESULT CreateAuto( const AkOSChar* in_pszFileName, AkFileSystemFlags * in_pFSFlags, const AkAutoStmHeuristics & in_heuristics, AkAutoStmBufSettings * in_pBufferSettings, IAkAutoStream *& out_pStream, bool in_bSyncOpen )
			{
				TraceFLF(", '%s', %s", in_pszFileName, BoolToString(in_bSyncOpen));
				return AK_Success;
			}
			virtual AKRESULT CreateAuto( AkFileID in_fileID, AkFileSystemFlags * in_pFSFlags, const AkAutoStmHeuristics & in_heuristics, AkAutoStmBufSettings * in_pBufferSettings, IAkAutoStream *& out_pStream, bool in_bSyncOpen )
			{
				TraceFLF(", %u, %s", in_fileID, BoolToString(in_bSyncOpen));
				return AK_Success;
			}
			virtual AKRESULT PinFileInCache( AkFileID in_fileID, AkFileSystemFlags * in_pFSFlags, AkPriority in_uPriority ) { TraceFLF(", %u, %d", in_fileID, in_uPriority); return AK_Success; }
			virtual AKRESULT UnpinFileInCache( AkFileID in_fileID, AkPriority in_uPriority ) { TraceFLF(", %u, %d", in_fileID, in_uPriority); return AK_Success; }
			virtual AKRESULT UpdateCachingPriority( AkFileID in_fileID, AkPriority in_uPriority, AkPriority in_uOldPriority ) { TraceFLF(", %u, %d, %d", in_fileID, in_uPriority, in_uOldPriority); return AK_Success; }
			virtual AKRESULT GetBufferStatusForPinnedFile( AkFileID in_fileID, AkReal32& out_fPercentBuffered, bool& out_bCacheFull ) { TraceFLF(", %u", in_fileID); return AK_Success; }
		};

		void GetDefaultSettings(AkStreamMgrSettings&) { TraceFLF(""); }
		void GetDefaultDeviceSettings(AkDeviceSettings&) { TraceFLF(""); }
		IAkStreamMgr* Create(AkStreamMgrSettings const&) { TraceFLF(""); return new AkStreamMgr; }
		AKRESULT SetCurrentLanguage(char const* l) { TraceFLF(", '%s'", l); return AK_Success; }

		AKRESULT DestroyDevice(unsigned int d) { TraceFLF(", %u", d); return AK_Success; } 
        AkDeviceID CreateDevice(AkDeviceSettings const&, IAkLowLevelIOHook*) { TraceFLF(""); return 0; } 
        AkMemPoolId GetPoolID() { TraceFLF(""); return 0; } 
        void SetFileLocationResolver(IAkFileLocationResolver*) { TraceFLF(""); }
        IAkFileLocationResolver* GetFileLocationResolver() { TraceFLF(""); return nullptr; }
		const AkOSChar *GetCurrentLanguage() { TraceFLF(""); return "en"; }
	}

	namespace MemoryMgr
	{
		AKRESULT Init(AkMemSettings*) { TraceFLF(""); return AK_Success; }
		void GetDefaultSettings(AkMemSettings&) { TraceFLF(""); }
		void Term(void) { TraceFLF(""); }

		AKRESULT DestroyPool(int p) { TraceFLF(", %d", p); return AK_Success; } 
        void* Malloc(int p, unsigned long s) { TraceFLF(", %u, %lu", p, s); return nullptr; } 
        AKRESULT Free(int p, void* b) { TraceFLF(", %u, %p", p, b); return AK_Success; } 
        AKRESULT ReleaseBlock(int p, void* b) { TraceFLF(", %u, %p", p, b); return AK_Success; }
	}

	namespace MusicEngine
	{
		void GetDefaultInitSettings(AkMusicSettings&) { TraceFLF(""); }
		AKRESULT Init(AkMusicSettings*) { TraceFLF(""); return AK_Success; }
		void Term(void) { TraceFLF(""); }

		AKRESULT GetPlayingSegmentInfo( AkPlayingID in_PlayingID, AkSegmentInfo & out_segmentInfo, bool in_bExtrapolate )
		{
			TraceFLF(", %u, %s", in_PlayingID, BoolToString(in_bExtrapolate));
			return AK_Success;
		}
	}

	namespace Monitor
	{
		AKRESULT PostString(const char* in_pszError, ErrorLevel in_eErrorLevel, AkPlayingID in_playingID, AkGameObjectID in_gameObjID, AkUniqueID in_audioNodeID, bool in_bIsBus)
		{
			TraceFLF(", '%s', %u, %u, %u, %u, %s", in_pszError, in_eErrorLevel, in_playingID, in_gameObjID, in_audioNodeID, BoolToString(in_bIsBus));
			return AK_Success;
		}

		const AkOSChar* s_aszErrorCodes[ Num_ErrorCodes ];
		AKRESULT PostCode( ErrorCode in_eError, ErrorLevel in_eErrorLevel ) { TraceFLF(", %u, %u", in_eError, in_eErrorLevel); return AK_Success; }
		AKRESULT PostString( const char* in_pszError, ErrorLevel in_eErrorLevel ) { TraceFLF(", '%s', %u", in_pszError, in_eErrorLevel); return AK_Success; }
		AKRESULT SetLocalOutput( AkUInt32 in_uErrorLevel, LocalOutputFunc in_pMonitorFunc ) { TraceFLF(", %u", in_uErrorLevel); return AK_Success; }
		AkTimeMs GetTimeStamp() { TraceFLF(""); return 0; }
	}

	IAkStreamMgr * IAkStreamMgr::m_pStreamMgr = NULL;
	//void IAkStreamMgr::Destroy() {}
}

#if ENABLE_JENOVA
AKRESULT CAkDefaultIOHookBlocking::Init(AkDeviceSettings const&, bool) { TraceFLF(""); return AK_Success; }
void CAkDefaultIOHookBlocking::Term(void) {}
AKRESULT CAkDefaultIOHookBlocking::SetBasePath(char const* p) { TraceFLF(", '%s'", p); return AK_Success; }
#endif

IAkSoftwareCodec* CreateVorbisBankPlugin( void* in_pCtx ) { TraceFLF(""); return nullptr; }
IAkSoftwareCodec* CreateVorbisFilePlugin( void* in_pCtx ) { TraceFLF(""); return nullptr; }
AK::IAkPluginParam * CreateSilenceSourceParams( AK::IAkPluginMemAlloc * in_pAllocator ) { TraceFLF(""); return nullptr; }
AK::IAkPlugin* CreateSilenceSource( AK::IAkPluginMemAlloc * in_pAllocator ) { TraceFLF(""); return nullptr; }

AKSOUNDENGINE_API AkAssertHook g_pAssertHook = nullptr;
