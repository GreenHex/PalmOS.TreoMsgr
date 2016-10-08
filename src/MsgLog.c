/*
 * MsgLog.c
 */

#include "MsgLog.h"

// Prototypes
static Boolean 						ReadMsgLogByIdx(UInt16 idx, Char** str, Boolean* isSecretP, UInt16 dmReadMode) EXTRA_SECTION_ONE;

/*
 * MsgLog
 */
Err MsgLog(AppUSPrefs_t* usPrefsP, Char* errStr)
{
	Err 				error = errNone;
	DmOpenRef 			dbP = NULL;
	UInt16				attributes = dmHdrAttrCopyPrevention | dmHdrAttrBackup; // dmHdrAttrCopyPrevention | dmHdrAttrHidden | dmHdrAttrBackup;

	Char 				SEPARATOR_STR_START[2];
	Char 				SEPARATOR_STR[3];
	Char				SEPARATOR_STR_END[2];

	DateTimeType 		dtNow;
	Char				timeStr[timeStringLength];
	Char				dateStr[dateStringLength];
	Char*				logStr = NULL;
	UInt16				logStrLen = 0;
	
	SEPARATOR_STR_START[0] = SEPARATOR_CHAR_START;
	SEPARATOR_STR_START[1] = chrNull;
	
	SEPARATOR_STR[0] = SEPARATOR_CHAR_END;
	SEPARATOR_STR[1] = SEPARATOR_CHAR_START;
	SEPARATOR_STR[2] = chrNull;

	SEPARATOR_STR_END[0] = SEPARATOR_CHAR_END;
	SEPARATOR_STR_END[1] = chrNull;

	logStrLen = StrLen(SEPARATOR_STR_START)
					+ dateStringLength + StrLen(SEPARATOR_STR)
					+ timeStringLength + StrLen(SEPARATOR_STR)	
					+ StrLen(errStr) + StrLen(SEPARATOR_STR)
					+ StrLen(usPrefsP->Task) + StrLen(SEPARATOR_STR)
					+ StrLen(usPrefsP->msg.DisplayName) + StrLen(SEPARATOR_STR)
					+ StrLen(usPrefsP->msg.Numbers) + StrLen(SEPARATOR_STR)
					+ StrLen(usPrefsP->msg.Message) + StrLen(SEPARATOR_STR)
					+ StrLen(SEPARATOR_STR_END)
					+ 1; 
	
	logStr = MemPtrNew(logStrLen);

	if (!logStr)
		return (1);
	
	MemSet(logStr, logStrLen, 0);
			
	TimSecondsToDateTime(usPrefsP->time, &dtNow);

	// TimeToAscii(dtNow.hour, dtNow.minute, /* PrefGetPreference(prefTimeFormat) */ tfColon24h, timeStr);
	// DateToAscii(dtNow.month, dtNow.day, dtNow.year, PrefGetPreference(prefDateFormat), dateStr);
	
	StrPrintF(timeStr, "%02d:%02d", dtNow.hour, dtNow.minute);
	StrPrintF(dateStr, "%02d/%02d/%02d", dtNow.day, dtNow.month, dtNow.year);
	
	StrCopy(logStr, SEPARATOR_STR_START);
	StrCat(logStr, dateStr);
	StrCat(logStr, SEPARATOR_STR);
	StrCat(logStr, timeStr);
	StrCat(logStr, SEPARATOR_STR);
	StrCat(logStr, errStr);		
	StrCat(logStr, SEPARATOR_STR);
	StrCat(logStr, usPrefsP->Task);		
	StrCat(logStr, SEPARATOR_STR);
	StrCat(logStr, usPrefsP->msg.DisplayName);
	StrCat(logStr, SEPARATOR_STR);
	StrCat(logStr, usPrefsP->msg.Numbers);
	StrCat(logStr, SEPARATOR_STR);
	StrCat(logStr, usPrefsP->msg.Message);
	StrCat(logStr, SEPARATOR_STR_END);
	
	logStr[logStrLen] = chrNull;
	
	dbP = DmOpenDatabaseByTypeCreator(LogDbType, LogDbCreator, dmModeReadWrite | dmModeShowSecret);
	
	if (!dbP) // create the database...
	{	
		UInt16			cardNo = 0;
		LocalID  		dbID;
	
		error = DmCreateDatabase(cardNo, LogDbName, LogDbCreator, LogDbType, false); // "true" to create a resource DB
		
		if (error)
			return error;
					
		dbP = DmOpenDatabaseByTypeCreator(LogDbType, LogDbCreator, dmModeReadWrite | dmModeShowSecret);
		if (!dbP)
			return (DmGetLastErr());
			
		dbID = DmFindDatabase(cardNo, LogDbName);
		
		// this returns error, don't know why
		DmSetDatabaseInfo(cardNo, dbID, NULL, &attributes, 
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);				
	}

	if (dbP)
	{
		MemHandle		recordH;
		Char*			recordP;
		UInt16			idx = dmMaxRecordIndex;
		UInt16			recordAttributes = 0;		
		
		recordH = DmNewRecord(dbP, &idx, StrLen(logStr) + 1);
		if (recordH)
		{
			recordP = MemHandleLock(recordH);
			
			if (recordP)
			{
				error = DmWrite(recordP, 0, logStr, StrLen(logStr) + 1);
			}
			
			MemHandleUnlock(recordH);
			recordP = NULL;
		
			DmRecordInfo(dbP, idx, &recordAttributes, NULL, NULL);
			if (usPrefsP->isSecret)
			{
				recordAttributes |= dmRecAttrSecret;
			}
			else
			{
				recordAttributes &= ~dmRecAttrSecret;
			}
			DmSetRecordInfo(dbP, idx, &recordAttributes, NULL);
			
			DmReleaseRecord(dbP, idx, false);
		}
		
		DmCloseDatabase(dbP);
	}
	
	if (logStr)
		MemPtrFree(logStr);
		
	return error;
	
} // MsgLog

/*
 * SplitLogEntry
 */
Boolean SplitLogEntry(UInt16 idx, log_t* log, Boolean* isSecretP)
{
	Boolean 			retVal = false;
	Char* 				str = NULL;
	
	Char 				SEPARATOR_STR_START[2];
	Char				SEPARATOR_STR_END[2];
		
	Char*				tmpStrP;
	
	Char*				pStartLocation = NULL;
	Char*				pEndLocation = NULL;
	UInt16				copyStrLength = 0;	
	
	SEPARATOR_STR_START[0] = SEPARATOR_CHAR_START;
	SEPARATOR_STR_START[1] = chrNull;

	SEPARATOR_STR_END[0] = SEPARATOR_CHAR_END;
	SEPARATOR_STR_END[1] = chrNull;
	
	if ((retVal = ReadMsgLogByIdx(idx, &str, isSecretP, GetReadOnlySecretMode())))
	{	
		// date
		tmpStrP = str;
		
		pStartLocation = StrStr(tmpStrP, SEPARATOR_STR_START) + StrLen(SEPARATOR_STR_START);
		pEndLocation = StrStr(tmpStrP, SEPARATOR_STR_END);
		
		copyStrLength = pEndLocation - pStartLocation;
		
		StrNCopy(log->Date, pStartLocation, copyStrLength);
		log->Date[copyStrLength] = chrNull;
		
		// time
		tmpStrP = pEndLocation + 1;
		
		pStartLocation = StrStr(tmpStrP, SEPARATOR_STR_START) + StrLen(SEPARATOR_STR_START);
		pEndLocation = StrStr(tmpStrP, SEPARATOR_STR_END);
		
		copyStrLength = pEndLocation - pStartLocation;
	
		StrNCopy(log->Time, pStartLocation, copyStrLength);
		log->Time[copyStrLength] = chrNull;
		
		// error
		tmpStrP = pEndLocation + 1;
		
		pStartLocation = StrStr(tmpStrP, SEPARATOR_STR_START) + StrLen(SEPARATOR_STR_START);
		pEndLocation = StrStr(tmpStrP, SEPARATOR_STR_END);
		
		copyStrLength = pEndLocation - pStartLocation;
	
		StrNCopy(log->Error, pStartLocation, copyStrLength);
		log->Error[copyStrLength] = chrNull;

		// task
		tmpStrP = pEndLocation + 1;
		
		pStartLocation = StrStr(tmpStrP, SEPARATOR_STR_START) + StrLen(SEPARATOR_STR_START);
		pEndLocation = StrStr(tmpStrP, SEPARATOR_STR_END);
		
		copyStrLength = pEndLocation - pStartLocation;
	
		StrNCopy(log->Task, pStartLocation, copyStrLength);
		log->Task[copyStrLength] = chrNull;
	
		// name
		tmpStrP = pEndLocation + 1;
		
		pStartLocation = StrStr(tmpStrP, SEPARATOR_STR_START) + StrLen(SEPARATOR_STR_START);
		pEndLocation = StrStr(tmpStrP, SEPARATOR_STR_END);
		
		copyStrLength = pEndLocation - pStartLocation;
	
		StrNCopy(log->Name, pStartLocation, copyStrLength);
		log->Name[copyStrLength] = chrNull;
		
		// number
		tmpStrP = pEndLocation + 1;
		
		pStartLocation = StrStr(tmpStrP, SEPARATOR_STR_START) + StrLen(SEPARATOR_STR_START);
		pEndLocation = StrStr(tmpStrP, SEPARATOR_STR_END);
		
		copyStrLength = pEndLocation - pStartLocation;
	
		StrNCopy(log->Numbers, pStartLocation, copyStrLength);
		log->Numbers[copyStrLength] = chrNull;
		
		// msg
		tmpStrP = pEndLocation + 1;
		
		pStartLocation = StrStr(tmpStrP, SEPARATOR_STR_START) + StrLen(SEPARATOR_STR_START);
		pEndLocation = StrStr(tmpStrP, SEPARATOR_STR_END);
		
		copyStrLength = pEndLocation - pStartLocation;
	
		StrNCopy(log->Message, pStartLocation, copyStrLength);
		log->Message[copyStrLength] = chrNull;
		
		if (str)
			MemPtrFree(str);
		
		str = NULL;
		tmpStrP = NULL;
	}
			
	return (retVal);
	
} // SplitLogEntry
	
/*
 * ReadMsgLogByIdx
 */
static Boolean ReadMsgLogByIdx(UInt16 idx, Char** strP, Boolean* isSecretP, UInt16 dmReadMode)
{
	Boolean 			retVal = false;
	DmOpenRef 			dbP = NULL;

	dbP = DmOpenDatabaseByTypeCreator(LogDbType, LogDbCreator, dmReadMode);
	
	if (!dbP) // create the database...
	{	
		return retVal;
	}
	
	if (dbP)
	{
		MemHandle		recordH;
		Char*			recordP;
		UInt16			recordIdx = 0;
		
		if (!DmSeekRecordInCategory(dbP, &recordIdx, idx, dmSeekForward, dmAllCategories))
		{
			recordH = DmQueryNextInCategory(dbP, &recordIdx, dmAllCategories);
			if (recordH)
			{
				if (isSecretP)
				{
					UInt16			recordAttributes = 0;		
		
					DmRecordInfo(dbP, recordIdx, &recordAttributes, NULL, NULL);
					*isSecretP = (recordAttributes & dmRecAttrSecret);
				}			
				recordP = MemHandleLock(recordH);
				
				if (recordP)
				{
					*strP = MemPtrNew(StrLen(recordP) + 1);
					StrCopy(*strP, recordP);
										
					retVal = true;
				}
				
				MemHandleUnlock(recordH);
				recordP = NULL;
				DmReleaseRecord(dbP, recordIdx, false);
			}
		}	
		DmCloseDatabase(dbP);
	}
			
	return (retVal);
	
} // ReadMsgLogByIdx

/*
 * GetNumLogRecords
 */
UInt16 GetNumLogRecords(UInt16 dmReadMode)
{
	UInt16				retVal = 0;
	DmOpenRef			dbP = NULL;
	
	dbP = DmOpenDatabaseByTypeCreator(LogDbType, LogDbCreator, dmReadMode);
	
	if (dbP)
	{
		retVal = DmNumRecordsInCategory(dbP, dmAllCategories);
		DmCloseDatabase(dbP);
	}
	
	return (retVal);
	
} // GetNumLogRecords

/*
 * DeleteLog
 */
Err DeleteLog(void)
{
	Err					error = errNone;
	
	DmOpenRef			dbP = NULL;
	
	dbP = DmOpenDatabaseByTypeCreator(LogDbType, LogDbCreator, dmModeReadWrite | dmModeShowSecret);
	
	if (dbP) // delete the database
	{
		UInt16				cardNo = 0;
		LocalID  			dbID;
		UInt16				attributes = 0;

		error = DmOpenDatabaseInfo(dbP, &dbID, NULL, NULL, &cardNo, NULL);

		error = DmDatabaseProtect(cardNo, dbID, false); // unprotect, if required
		
		error = DmDatabaseInfo(cardNo, dbID, NULL, &attributes,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

		attributes = attributes & (~dmHdrAttrReadOnly);
		
		error = DmSetDatabaseInfo(cardNo, dbID, NULL, &attributes,
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

		error = DmCloseDatabase(dbP);

		error = DmDeleteDatabase(cardNo, dbID);
		
		dbP = NULL;
	}
	
	return (error);
	
} // DeleteLog

/*
 * MsgLog.c
 */