/*
 * MsgLog.h
 */

#ifndef __MSGLOG_H__
#define __MSGLOG_H__

#include <Hs.h>
#include <HsAppLaunchCmd.h>
#include <DataMgr.h>

#include "PhoneUtils.h"
#include "Global.h"

// Prototypes
extern Err 							MsgLog(AppUSPrefs_t* usPrefsP, Char* errStr);
extern Boolean 						SplitLogEntry(UInt16 idx, log_t* log, Boolean* isSecretP);
extern UInt16 						GetNumLogRecords(UInt16 dmReadMode);
extern Err 							DeleteLog(void) EXTRA_SECTION_ONE;
	
	
#endif /* __MSGLOG_H__ */
 
/*
 * MsgLog.h
 */
 