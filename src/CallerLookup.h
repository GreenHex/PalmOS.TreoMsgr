/*
 * CallerLookup.h
 */

#ifndef __CALLERLOOKUP_H__
#define __CALLERLOOKUP_H__

#include <Hs.h>
#include <palmOneResources.h>
#include <PalmTypes.h>
#include <HsAppLaunchCmd.h>
#include <palmOneCreators.h>

#include "Global.h"
#include "AddrDB.h"

// AddrDB.c
extern Err 				AddrDBGetRecord(DmOpenRef dbP, UInt16 index, AddrDBRecordPtr recordP, MemHandle* recordH);
extern Err 				AddrDBGetDatabase(DmOpenRef* dbPP, UInt16 mode);

// Prototypes.c
extern Boolean 			CallerLookup(char* number, char* formatStr, char* outStr, UInt16 outStrSize);

#endif /* __CALLERLOOKUP_H__ */
 
/*
 * CallerLookup.h
 */