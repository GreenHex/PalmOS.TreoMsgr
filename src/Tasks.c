/*
 * Tasks.c
 */

#include "Tasks.h"

// Prototypes
static Boolean 				GetMessagingInfo(ToDoDBRecordPtr recordP, AppUSPrefs_t* usPrefsP);
static Boolean 				GetPhnNumber(Char* description, Char* phnNumber);
static Boolean 				GetMessageFromNote(Char* note, Char* message);
static Boolean 				GetNextDueDate(ToDoDBRecordPtr recordP, DateType* nextDueDateP);
static Boolean 				GetRepeatInfo(ToDoDBRecordPtr recordP, Char* RepeatInfoStr) EXTRA_SECTION_ONE;

/*
 * CreateCategory
 */
void CreateCategory(void)
{
	DmOpenRef		dbP;
	
	if ((!ToDoGetDatabase(&dbP, dmModeReadWrite)) && (dbP))
	{		
		if (CategoryFind(dbP, APP_NAME) == dmAllCategories)
		{
			UInt16			categoryIdx = 0;
		
			categoryIdx = CategoryFind(dbP, "");
			
			if (categoryIdx != dmRecNumCategories)
			{
				CategorySetName(dbP, categoryIdx, APP_NAME);				
			}
		}
		DmCloseDatabase(dbP);
	}
	
} // CreateCategory

/*
 * DeleteTask
 */
Boolean DeleteTask(UInt16 idx, deleteType_e deleteType)
{
	Boolean			retVal = false;
	DmOpenRef		dbP;
	
	if ((!ToDoGetDatabase(&dbP, dmModeReadWrite  | dmModeShowSecret)) && (dbP))
	{
		UInt16			categoryIdx = 0;
		
		if ((!ToDoChangeSortOrder(dbP, soDueDatePriority))
			&& ((categoryIdx = CategoryFind(dbP, APP_NAME)) != dmAllCategories))
		{
			if (deleteType == archiveRecord)
			{
				retVal = (DmArchiveRecord(dbP, idx) == errNone);
			}
			else
			{
				retVal = (DmDeleteRecord(dbP, idx) == errNone);
			}
			
			if (retVal) // archive or delete successful
			{
				retVal = (DmMoveRecord(dbP, idx, DmNumRecords(dbP)) == errNone);
			}	
		}
		DmCloseDatabase(dbP);
	}
	
	return (retVal);
		
} // DeleteTask

/*
 * SplitTaskEntry
 */
Boolean SplitTaskEntry(UInt16 idx, log_t* log, Boolean* isSecretP)
{
	Boolean 		retVal = false;
	AppUSPrefs_t	usPrefs;
	
	MemSet(&usPrefs, sizeof(AppUSPrefs_t), 0);
	
	if (ReadTaskByIdx(idx, &usPrefs, log->Name, GetReadOnlySecretMode()))
	{
		GetStrDateTime(usPrefs.time, log->Date, log->Time);
		
		StrNCopy((Char*)(log->Error), (Char*)(usPrefs.Task), (LEN_ErrorString - 1));
		log->Error[LEN_ErrorString - 1] = chrNull;
		
		StrCopy(log->Task, usPrefs.Task);
		StrCopy(log->Numbers, usPrefs.msg.Numbers);
		StrCopy(log->Message, usPrefs.msg.Message);
		
		if (isSecretP)
			*isSecretP = usPrefs.isSecret;
		
		retVal = true;
	}
	return (retVal);
	
} // SplitTaskEntry

/*
 * GetNumTaskRecords
 */
UInt16 GetNumTaskRecords(UInt16 dmReadMode)
{
	UInt16				retVal = 0;
	DmOpenRef			dbP = NULL;
	UInt16				categoryIdx = dmAllCategories;
		
	if ((!ToDoGetDatabase(&dbP, dmReadMode)) && (dbP))
	{
		if ((!ToDoChangeSortOrder(dbP, soDueDatePriority))
			&& ((categoryIdx = CategoryFind(dbP, APP_NAME)) != dmAllCategories))
		{	
			retVal = DmNumRecordsInCategory(dbP, categoryIdx);
		}
		DmCloseDatabase(dbP);
	}
	
	return retVal;
	
} // GetNumTaskRecords

/*
 * ReadTaskByIdx
 */
Boolean ReadTaskByIdx(UInt16 idx, AppUSPrefs_t* usPrefsP, Char* RepeatInfoStr, UInt16 dmReadMode)
{
	Boolean 			retVal = false;
	DmOpenRef			dbP = NULL;
	UInt16				categoryIdx = dmAllCategories;
	UInt16				numRecordsInCategory = 0;

	usPrefsP->idx = 0;
	usPrefsP->time = 0;
			
	if ((!ToDoGetDatabase(&dbP, dmReadMode)) && (dbP))
	{
		if ((!ToDoChangeSortOrder(dbP, soDueDatePriority))
			&& ((categoryIdx = CategoryFind(dbP, APP_NAME)) != dmAllCategories)
			&& ((numRecordsInCategory = DmNumRecordsInCategory(dbP, categoryIdx)) > 0))
		{
			MemHandle				recordH = NULL;
			ToDoDBRecordPtr			recordP = NULL;
			UInt16					recordIdx = 0;
			UInt16 					recordAttributes = 0;
			
			Boolean					hasAlarm = false;
			DateType				dueDate = { 0, 0, 0 };
			ToDoAlarmInfoType		alarmInfo;			
			UInt32					almSeconds = 0;

			MemSet(&alarmInfo, sizeof(ToDoAlarmInfoType), 0);
			
			if (!DmSeekRecordInCategory(dbP, &recordIdx, idx, dmSeekForward, categoryIdx))
			{	
				recordH = DmQueryNextInCategory(dbP, &recordIdx, categoryIdx);
					
				if (recordH)
				{
					DmRecordInfo(dbP, recordIdx, &recordAttributes, NULL, NULL);
					usPrefsP->isSecret = (recordAttributes & dmRecAttrSecret);
					
					recordP = (ToDoDBRecordPtr) MemHandleLock(recordH);			
				}
				
				if (recordP)
				{				
					if ((!ToDoDBRecordGetAlarmInfo(dbP, recordIdx, &hasAlarm, &dueDate, &alarmInfo))
							&& (hasAlarm))
					{
						UInt32					numDays = 0;
						
						numDays = DateToDays(dueDate); 
						almSeconds = (UInt32) ((UInt32) numDays * 24 * 60 * 60)
								+ ((((UInt32) alarmInfo.alarmTime.hours * 60)
								+ ((UInt32)	alarmInfo.alarmTime.minutes)) * 60);
					
						usPrefsP->time = almSeconds;						
						
					}
					
					usPrefsP->isComplete = ((recordP->recordFlags) & TODO_RECORD_FLAG_COMPLETE);
					usPrefsP->idx = recordIdx;
					GetMessagingInfo(recordP, usPrefsP);
					
					GetRepeatInfo(recordP, RepeatInfoStr);
					gDueDate = dueDate;
					gDueDate.year -= ZERO_YEAR;
					
					retVal = true;
				}
				
				if (recordH)
				{
					MemHandleUnlock(recordH);
					recordH = NULL;
				}
			}
		}
				
		DmCloseDatabase(dbP);
	}
	
	return retVal;
	
} // ReadTaskByIdx

/*
 * GetMessagingInfo
 */
Boolean GetMessagingInfo(ToDoDBRecordPtr recordP, AppUSPrefs_t* usPrefsP)
{
	Boolean 			retVal = false;

	if (recordP)
	{	
		if (ToDoDBRecordGetFieldPointer(recordP, toDoRecordFieldNote))
		{
			retVal = (GetValidSubString(GetToDoNotePtr(recordP), NULL, 0, NOTE_TOKEN_START, NOTE_TOKEN_END)
					&& GetPhnNumber(GetToDoNotePtr(recordP), usPrefsP->msg.Numbers)
					&& GetMessageFromNote(GetToDoNotePtr(recordP), usPrefsP->msg.Message));
		}

		if (ToDoDBRecordGetFieldPointer(recordP, toDoRecordFieldDescription))
		{
			UInt16	copyLen = (StrLen(GetToDoDescriptionPtr(recordP)) < (LEN_Description - 1)) ?
									StrLen(GetToDoDescriptionPtr(recordP)) : (LEN_Description - 1);

			StrNCopy((Char*)(usPrefsP->Task), (Char*)GetToDoDescriptionPtr(recordP), copyLen);
			usPrefsP->Task[copyLen] = chrNull;
		}
	}

	return (retVal);
	
} // GetMessagingInfo

/*
 * GetPhnNumber
 */
static Boolean GetPhnNumber(Char* note, Char* phnNumber)
{
	return GetValidSubString(note, phnNumber, LEN_PhoneNumbers, TEL_TOKEN_START, TEL_TOKEN_END);
	
} // GetPhnNumber


/*
 * GetMessageFromNote
 */
static Boolean GetMessageFromNote(Char* note, Char* message)
{		
	return GetValidSubString(note, message, LEN_Message, MSG_TOKEN_START, MSG_TOKEN_END);

} // GetMessageFromNote

/*
 * SetCompletionStatus
 * 
 * Returns "true" if the task is marked COMPLETE "false" otherwise
 * 
 */
Boolean SetCompletionStatus(UInt16* recordIdxP) 
{
	Boolean 			retVal = false;
	DmOpenRef			dbP = NULL;
	
	if (!ToDoGetDatabase(&dbP, dmModeReadWrite | dmModeShowSecret) && (dbP))
	{
		MemHandle 			recordH = NULL;
		ToDoDBRecordPtr 	recordP = NULL;
		DateType			nextDueDate;
		Boolean				bNextDueDateValid = false;
		
		recordH = DmQueryRecord(dbP, *recordIdxP);
	
		if (recordH)
			recordP = (ToDoDBRecordPtr) MemHandleLock(recordH);

		if (recordP)
		{	
			bNextDueDateValid = GetNextDueDate(recordP, &nextDueDate);
			
			if (recordH)
			{
				MemHandleUnlock(recordH);
				recordH = NULL;
			}
		
			if (bNextDueDateValid)
			{
				ToDoDBRecordSetDueDate(dbP, *recordIdxP, &nextDueDate);	
			}
			else // mark completed
			{
				UInt16		flag = 0;
				// UInt16		priority = 0;
				
				flag |= TODO_RECORD_FLAG_COMPLETE;	
				
				ToDoChangeRecord(dbP, recordIdxP, 0, 0, toDoRecordFieldComplete, &flag);
				/*
				priority = *(UInt16*) ToDoDBRecordGetFieldPointer(recordP, toDoRecordFieldPriority);
				priority |= 0x80;
				ToDoChangeRecord(dbP, recordIdxP, 0, 0, toDoRecordFieldPriority, &priority);
				*/
				retVal = true;
			}
						
			DmReleaseRecord(dbP, *recordIdxP, false);
		}

		DmCloseDatabase(dbP);		
	}
	
	return retVal;
	
} // SetCompletionStatus

/*
 * GetFirstValidTaskIdx
 */
Boolean GetFirstValidTaskIdx(AppUSPrefs_t* usPrefsP)
{
	Boolean 			retVal = false;
	DmOpenRef			dbP = NULL;
	UInt16				categoryIdx = dmAllCategories;
	UInt16				numRecordsInCategory = 0;

	usPrefsP->idx = 0;
	usPrefsP->time = 0;
	
	if ((!ToDoGetDatabase(&dbP, dmModeReadOnly  | dmModeShowSecret)) && (dbP))
	{
		if ((!ToDoChangeSortOrder(dbP, soDueDatePriority))
			&& ((categoryIdx = CategoryFind(dbP, APP_NAME)) != dmAllCategories)
			&& ((numRecordsInCategory = DmNumRecordsInCategory(dbP, categoryIdx)) > 0))
		{
			MemHandle				recordH = NULL;
			ToDoDBRecordPtr			recordP = NULL;
			UInt16					recordIdx = 0;
			
			Boolean					hasAlarm = false;
			DateType				dueDate = { 0, 0, 0 };
			ToDoAlarmInfoType		alarmInfo;
			
			UInt32					almSeconds = 0;
			UInt32					prevLowAlmSeconds = 0xFFFFFFFF;
			UInt32					timeNow = 0;
			UInt16					i = 0;
			
			AppStats_t				stats;
			
			readStats(&stats);
			stats.numQueued = 0;
			
			MemSet(&alarmInfo, sizeof(ToDoAlarmInfoType), 0);
	
			for ( i = 0 ; i < numRecordsInCategory ; i++ )
			{
				recordH = DmQueryNextInCategory(dbP, &recordIdx, categoryIdx);
				
				if (recordH)
				{				
					recordP = (ToDoDBRecordPtr) MemHandleLock(recordH);
				}
									
				if (recordP)
				{				
					usPrefsP->isComplete = ((recordP->recordFlags) & TODO_RECORD_FLAG_COMPLETE);
	
					if ((!ToDoDBRecordGetAlarmInfo(dbP, recordIdx, &hasAlarm, &dueDate, &alarmInfo))
							&& (hasAlarm))
					{
						UInt32					numDays = 0;

						numDays = DateToDays(dueDate); 
						almSeconds = (UInt32) ((UInt32) numDays * 24 * 60 * 60)
								+ ((((UInt32) alarmInfo.alarmTime.hours * 60)
								+ ((UInt32)	alarmInfo.alarmTime.minutes)) * 60);
								   
						timeNow	= TimGetSeconds();

						if ((!(usPrefsP->isComplete))
								&& ((almSeconds) > timeNow))
						{
							++(stats.numQueued); /* valid future event..., but we don't check messaging info. */
							
							if ((almSeconds < prevLowAlmSeconds)
								&& GetMessagingInfo(recordP, usPrefsP))
							{
								UInt16			recordAttributes = 0;
	
								usPrefsP->idx = recordIdx;
								prevLowAlmSeconds = almSeconds;
								usPrefsP->time = almSeconds;
	
								DmRecordInfo(dbP, recordIdx, &recordAttributes, NULL, NULL);
								usPrefsP->isSecret = (recordAttributes & dmRecAttrSecret);
							
								retVal = true;
							}
						}						
					}
				}

				if (recordH)
				{
					MemHandleUnlock(recordH);
					recordH = NULL;
				}
				++recordIdx;
			}
			
			writeStats(&stats);
		}		
		DmCloseDatabase(dbP);
	}
	
	return retVal;
	
} // GetFirstValidTaskIdx

/*
 * GetNextDueDate
 * 
 * Returns "true" is NextDueDate is valid, "false" othewise
 * 
 */
static Boolean GetNextDueDate(ToDoDBRecordPtr recordP, DateType* nextDueDateP)
{
	Boolean 					retVal = false;
	ToDoRepeatInfoType*			ToDoRepeatInfoP = NULL;
	Boolean						repeatAfterCompleted = false;
	DateType					prevCompletionDate;
	DateType					repeatEndDate;
	DateType					nextDueDate;
	UInt32						daysSinceZero = 0;
	UInt8						repeatFrequency = 0;
							
	// return false if not a repeating task
	if ((!recordP->dataFlags.repeat) || (!recordP->dataFlags.dueDate))
		return (retVal);
	
	repeatAfterCompleted = (recordP->recordFlags & TODO_RECORD_FLAG_REPEAT_ON_COMPLETE);
	
	if ((repeatAfterCompleted) && (recordP->dataFlags.completionDate))
	{
		prevCompletionDate = *(DateType*)ToDoDBRecordGetFieldPointer(recordP, toDoRecordFieldCompletionDate);
	}
	else
	{
		prevCompletionDate = *(DateType*)ToDoDBRecordGetFieldPointer(recordP, toDoRecordFieldDueDate);
	}
	
	ToDoRepeatInfoP = (ToDoRepeatInfoType*) ToDoDBRecordGetFieldPointer(recordP, toDoRecordFieldRepeat);
	
	repeatEndDate = ToDoRepeatInfoP->repeatInfo.repeatEndDate;
	repeatFrequency = ToDoRepeatInfoP->repeatInfo.repeatFrequency;
	
	nextDueDate = prevCompletionDate;
	
	switch (ToDoRepeatInfoP->repeatInfo.repeatType)
	{
	    case repeatDaily:
	    
	    	daysSinceZero = DateToDays(nextDueDate);
	    	daysSinceZero += repeatFrequency;    	
	    	DateDaysToDate(daysSinceZero, &nextDueDate);
	    	
	    	break;
	    	
	    case repeatWeekly:
	    
	    	if (repeatAfterCompleted)
	    	{
	    		daysSinceZero = DateToDays(nextDueDate);
	    		daysSinceZero += (repeatFrequency * 7);
	    		DateDaysToDate(daysSinceZero, &nextDueDate);
	    	}
	    	else
	    	{
	    		UInt8			dow = 0;	    		
	    	
	    		daysSinceZero = DateToDays(nextDueDate);
	    		DateDaysToDate(daysSinceZero, &nextDueDate);
	    		dow = (UInt8) DayOfWeek(nextDueDate.month, nextDueDate.day, nextDueDate.year + ZERO_YEAR);
	    		
	    		if (dow == 6)
	    		{
	    			daysSinceZero += ((repeatFrequency - 1) * 7);
	    		}
	    		
	    		do
	    		{	
	    			DateDaysToDate(++daysSinceZero, &nextDueDate);
	    			dow = (UInt8) DayOfWeek(nextDueDate.month, nextDueDate.day, nextDueDate.year + ZERO_YEAR);
	    			
	    			if (dow == 6)
	    			{
	    				daysSinceZero += ((repeatFrequency - 1) * 7);
	    			}	
	    		}
	    		while (!RepeatOnDOW((&(ToDoRepeatInfoP->repeatInfo)), dow));

	    	}	
	    
	    	break;
	    	
	    case repeatMonthlyByDay:
	    	{
	    		UInt16			numCount = nextDueDate.month + repeatFrequency;
		    	Int16			dom = DayOfMonth(nextDueDate.month, nextDueDate.day, nextDueDate.year);
		    	UInt16			day = 1;
		    	
		    	if (numCount > 12)
		    	{	
		    		nextDueDate.year += (repeatFrequency / 12) + 1;
		    		nextDueDate.month = numCount % 12;
		    	}
		    	else
		    	{
		    		nextDueDate.month = numCount;
		    	}
		    	
		    	for ( day = 1 ; day <= DaysInMonth(nextDueDate.month, nextDueDate.year) ; day++)
		    	{
		    		if (DayOfMonth(nextDueDate.month, day, nextDueDate.year) == dom)
		    		{
		    			nextDueDate.day = day;
		    			break;
		    		}
		    	}
	    	}
	    	break;
	    	
	    case repeatMonthlyByDate: // "Every Month" & "Repeat by: Date"
	    	{
	    		UInt16			numCount = nextDueDate.month + repeatFrequency;
	    		
		    	if (numCount > 12)
		    	{	
		    		nextDueDate.year += (repeatFrequency / 12) + 1;
		    		nextDueDate.month = numCount % 12;
		    	}
		    	else
		    	{
		    		nextDueDate.month = numCount;
		    	}
	    	}	
	    	break;
	    	
	    case repeatYearly:
	    
	    	nextDueDate.year += repeatFrequency;

    		break;
    		
		default: // includes "repeatNone"
		
			break;	
	}

	retVal = (DateTypeCmp(nextDueDate, repeatEndDate) <= 0);
	
	if (nextDueDateP)
		*nextDueDateP = nextDueDate;
	
	return (retVal);
	
} // GetNextDueDate

/*
 * GetRepeatInfo
 */
static Boolean GetRepeatInfo(ToDoDBRecordPtr recordP, Char* RepeatInfoStr)
{
	Boolean 					retVal = false;
	ToDoRepeatInfoType*			ToDoRepeatInfoP = NULL;
	Boolean						repeatAfterCompleted = false;
	UInt8						repeatFrequency = 0;

	if (!recordP->dataFlags.repeat)
	{
		if (RepeatInfoStr)
		{
			StrCopy(RepeatInfoStr, ">> No repeats.");
		}	
		retVal = false;	
	}
	else
	{	
		ToDoRepeatInfoP = (ToDoRepeatInfoType*) ToDoDBRecordGetFieldPointer(recordP, toDoRecordFieldRepeat);
		MemMove(&gToDoRepeatInfo, ToDoRepeatInfoP, sizeof(ToDoRepeatInfoType));
		
		if (RepeatInfoStr)
		{	
			repeatAfterCompleted = (recordP->recordFlags & TODO_RECORD_FLAG_REPEAT_ON_COMPLETE);
			gRepeatAfterCompleted = repeatAfterCompleted;
			
			repeatFrequency = ToDoRepeatInfoP->repeatInfo.repeatFrequency;
			
			switch (ToDoRepeatInfoP->repeatInfo.repeatType)
			{
			    case repeatDaily:
			    
			    	if (repeatFrequency == 1)
				    	StrPrintF(RepeatInfoStr, ">> Every day.");
					else
						StrPrintF(RepeatInfoStr, ">> Every %u days.", repeatFrequency);    
			    	
			    	break;
			    	
			    case repeatWeekly:
			    
			    	if (repeatFrequency == 1)
				    	StrPrintF(RepeatInfoStr, ">> Every week.");
					else
						StrPrintF(RepeatInfoStr, ">> Every %u weeks.", repeatFrequency);    
			    
			    	break;
			    	
			    case repeatMonthlyByDate: // "Every Month" & "Repeat by: Date"	// 20 CHARS MAX.	    	
			    	
			    	if (repeatFrequency == 1)
				    	StrPrintF(RepeatInfoStr, ">> Every month.");
					else
						StrPrintF(RepeatInfoStr, ">> Every %u months.", repeatFrequency);
					
			    	break;
			    	
			    case repeatMonthlyByDay:
			    	
			    	if (repeatFrequency == 1)
				    	StrPrintF(RepeatInfoStr, ">> Every month.");  // 20 CHARS MAX.
					else
						StrPrintF(RepeatInfoStr, ">> Every %u months.", repeatFrequency);
						
			    	break;
			    			    	
			    case repeatYearly:
			    
			    	if (repeatFrequency == 1)
				    	StrPrintF(RepeatInfoStr, ">> Every year.");
					else
						StrPrintF(RepeatInfoStr, ">> Every %u years.", repeatFrequency);    
		    		    		
		    		break;
		    		
				default: // includes "repeatNone"
	
			    	StrCopy(RepeatInfoStr, ">> Invalid repeat.");
			    				
					break;	
			}    
		}
		retVal = true;	
	}
	
	gHasRepeatInfo = retVal;
	
	return retVal;
	
} // GetRepeatInfo

/*
 * Tasks.c
 */