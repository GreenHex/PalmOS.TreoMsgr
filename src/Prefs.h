/*
 * Prefs.h
 */
 
#ifndef __PREFS_H__
#define __PREFS_H__

#include <PalmOS.h>
#include <HsExt.h>
#include <HsPhoneSMSTypes.h>

#include "Global.h"
#include "AppResources.h"

typedef struct {
	UInt16								numSent;
	UInt16								numFailed;
	UInt16								numQueued;
} AppStats_t;

typedef struct {
	SMSMessageStatus					status;
} PhnStatus_t;

#ifdef WITH_REGISTRATION
// Encrypt.c
extern void 				encBuf(UInt8* io_buffer, UInt16 buf_len, UInt8* key, CryptAction_e action);
extern Boolean 				encAppPrefs(AppPreferences_t* prefsP, CryptAction_e action);

#endif /* WITH_REGISTRATION */

// Prototypes 
extern void 				writePrefs(AppPreferences_t* prefsP);
extern void 				readPrefs(AppPreferences_t* prefsP);
extern void 				writeUSPrefs(AppUSPrefs_t* usPrefsP);
extern void 				readUSPrefs(AppUSPrefs_t* usPrefsP);
extern void					readStats(AppStats_t* statsP);
extern void					writeStats(AppStats_t* statsP);
// extern void 				writePhnStatus(SMSMessageStatus status);
// extern SMSMessageStatus 	readPhnStatus(void);
// extern void					writeDeferredData(AppUSPrefs_t* usPrefsP);
// extern Boolean				readDeferredData(AppUSPrefs_t* usPrefsP);

#endif /* __PREFS_H__ */

/*
 * Prefs.h
 */