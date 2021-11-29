// MenuPages.h
// Copyright (C) 2010 Audiokinetic Inc.
/// \file 
/// Include this file to have access to all Menu Pages.

#pragma once

#include "BaseMenuPage.h"
#include "DemoRTPCCarEngine.h"
#include "DemoDialogueMenu.h"
#include "DemoDynamicDialogue.h"
#include "DemoLocalization.h"
#include "DemoMarkers.h"
#include "DemoMusicCallbacks.h"
#include "DemoInteractiveMusic.h"
#include "DemoMotion.h"
#include "DemoExternalSources.h"
#include "DemoFootstepsManyVariables.h"
#include "DemoPositioning.h"

#if defined INTEGRATIONDEMO_MICROPHONE
#include "DemoMicrophone.h"
#endif 

#ifdef AK_PS3
#include "PS3/DemoRSX.h"
#endif

#if defined AK_XBOXONE || defined AK_PS4
#include "DemoBGM.h"
#endif
