/*
 * PhoneUtils.h
 */

#include <Hs.h>
#include <HsPhone.h>
#include <HsNav.h>
#include <HsExt.h>
#include <palmOneResources.h>
#include <PmSysGadgetLib.h>
#include <PmSysGadgetLibCommon.h>

#include "Global.h"
#include "AppResources.h"

#define MIN_VERSION  		sysMakeROMVersion(5, 0, 0, sysROMStageRelease, 0)
#define LAUNCH_FLAGS 		(sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)

// Prototypes
extern UInt16 				GetReadOnlySecretMode(void);
extern void 				Alert(Char* MsgType, Char* Msg, Err err);
// extern void 				beep(UInt8 numBeeps);
extern void 				CommonInitGauge(FormPtr pForm) EXTRA_SECTION_ONE;
extern void 				getROMID(UInt8* keyVal);
extern Boolean 				IsPhoneGSM(void);
extern Err 					RomVersionCompatible(UInt32 requiredVersion, UInt16 launchFlags);
 
#ifndef __PHONEUTILS_H__
#define __PHONEUTILS_H__

#endif /* __PHONEUTILS_H__ */

/*
 * PhoneUtils.h
 */