/*
 * GroupsList.c
 */

#include "GroupsList.h"

// Prototypes
static Err 				DBOpen(DmOpenRef* dbP, UInt16 mode); /* dmModeReadOnly | dmModeReadWrite */
static void 			ReleaseRecord(DmOpenRef* dbP, UInt16* idxP) EXTRA_SECTION_ONE;

/*
 * GetNumGroupRecords
 */
UInt16 GetNumGroupRecords(void)
{
	UInt16				retVal = 0;
	DmOpenRef			dbP = NULL;
	
	DBOpen(&dbP, dmModeReadOnly);
	
	if (dbP)
	{
		retVal = DmNumRecordsInCategory(dbP, dmAllCategories);
		DmCloseDatabase(dbP);
	}
	
	return (retVal);
	
} // GetNumGroupRecords

/*
 * FindGroup
 */
Boolean FindGroup(Char* nameStr, UInt16* idxP, Groups_t* groupP)
{
	Boolean			retVal = false;
	UInt16			numRecords = GetNumGroupRecords();
	Groups_t		group;
	
	UInt16			i = 0;
	
	if (idxP)
	{
		*idxP = dmMaxRecordIndex;
	}
	
	for (i = 0 ; i < numRecords ; i++)
	{
		if (GetGroup(i, &group))
		{		
			if (StrCompare(nameStr, group.Name) == 0)
			{
				retVal = true;
				
				if (idxP)
				{
					*idxP = i;
				}
				
				if (groupP)
				{
					MemMove(groupP, &group, sizeof(Groups_t));	
				}
				
				break;
			}
		}	
	}

	return (retVal);
	
} // FindGroup

/*
 * SaveNewGroup
 */
Err SaveNewGroup(Groups_t* groupP, Boolean isSecret, UInt16* idxP)
{
	Err 				error = errNone;
	DmOpenRef 			dbP = NULL;
	
	DBOpen(&dbP, dmModeReadWrite);
	
	if (dbP)
	{
		MemHandle		recordH;
		Char*			recordP;
		UInt16			idx = dmMaxRecordIndex;
		UInt16			recordAttributes = 0;		
		
		recordH = DmNewRecord(dbP, &idx, sizeof(Groups_t));
		if (recordH)
		{
			recordP = MemHandleLock(recordH);
			
			if (recordP)
			{
				error = DmWrite(recordP, 0, groupP, sizeof(Groups_t));
			}
			
			MemHandleUnlock(recordH);
			recordP = NULL;
		
			DmRecordInfo(dbP, idx, &recordAttributes, NULL, NULL);
			if (isSecret)
			{
				recordAttributes |= dmRecAttrSecret;
			}
			else
			{
				recordAttributes &= ~dmRecAttrSecret;
			}
			DmSetRecordInfo(dbP, idx, &recordAttributes, NULL);
			
			DmReleaseRecord(dbP, idx, false);
			
			if (idxP)
			{
				*idxP = idx;
			}
		}
		
		DmCloseDatabase(dbP);
	}
		
	return error;
	
} // SaveNewGroup

/*
 * ReplaceGroup
 */
Boolean ReplaceGroup(UInt16* idxP, Groups_t* groupP, Boolean isSecret)
{
	Boolean			retVal = false;
	DmOpenRef		dbP = NULL;
	MemHandle		recordH;
	Groups_t*		recordP;
	
	DBOpen(&dbP, dmModeReadWrite);
	
	if (dbP)
	{
		recordH = DmGetRecord(dbP, *idxP);
		if (recordH)
		{
			recordP = (Groups_t*) MemHandleLock(recordH);
			if (recordP)
			{	
				if (!DmWrite(recordP, 0, groupP, sizeof(Groups_t)))
					retVal = true;
			}	
			MemHandleUnlock(recordH);
			recordP = NULL;
		}
		ReleaseRecord(&dbP, idxP);
		
		DmCloseDatabase(dbP);
	}
	
	return (retVal);

} // ReplaceGroup

/*
 * RemoveGroup
 */
Boolean RemoveGroup(UInt16* idxP)
{
	Boolean			retVal = false;
	DmOpenRef		dbP = NULL;
	
	DBOpen(&dbP, dmModeReadWrite);
	
	if (dbP)
	{
		if (DmNumRecords(dbP) > 0)
		{
			if (!DmRemoveRecord(dbP, *idxP))
			{	
				*idxP = ((*idxP) > 0) ? --(*idxP) : 0;
				
				retVal = true;
			}
		}
		
		DmCloseDatabase(dbP);
	}
	return (retVal);
	
} // RemoveGroup

/*
 * GetGroup
 */
Boolean GetGroup(UInt16 idx, Groups_t* groupP)
{
	Boolean			retVal = false;
	DmOpenRef		dbP = NULL;
	MemHandle		recordH = NULL;
	Groups_t*		recordP = NULL;
	
	DBOpen(&dbP, dmModeReadWrite);
	
	if (dbP)
	{
		recordH = DmQueryRecord(dbP, idx);
		
		if (recordH)
		{
			recordP = (Groups_t*) MemHandleLock(recordH);
			if (recordP)
			{
				MemMove(groupP, recordP, sizeof(Groups_t));
				retVal = true;
			}
			
			MemHandleUnlock(recordH);
			recordP = NULL;
		}
		
		DmCloseDatabase(dbP);
	}
	
	return (retVal);

} // GetGroup

/*
 * DBOpen
 */
static Err DBOpen(DmOpenRef* dbP, UInt16 mode)
{
	Err 			error = dmErrCantOpen;
	DmOpenRef 		hdbP = NULL;
	UInt16			dmReadMode = 0;
	
	if (PrefGetPreference(prefShowPrivateRecords) == hidePrivateRecords)
	{
		dmReadMode = mode;
	}
	else
	{
		dmReadMode = mode | dmModeShowSecret;
	}
		
	*dbP = NULL;
	
	hdbP = DmOpenDatabaseByTypeCreator(GroupsDbType, GroupsDbCreator, dmReadMode);
	
	if (!hdbP) // create the database...
	{	
		UInt16		cardNo = 0;
		LocalID  	dbID;
		UInt16		attributes = dmHdrAttrCopyPrevention | dmHdrAttrHidden | dmHdrAttrBackup;
	
		error = DmCreateDatabase(cardNo, GroupsDbName, GroupsDbCreator, GroupsDbType, false); // "true" to create a resource DB
		
		if (error)
			return error;
					
		hdbP = DmOpenDatabaseByTypeCreator(GroupsDbType, GroupsDbCreator, dmReadMode);
		if (!hdbP)
			return (DmGetLastErr());
			
		
		dbID = DmFindDatabase(cardNo, GroupsDbName);
		
		// this returns error, don't know why
		DmSetDatabaseInfo(cardNo, dbID, NULL, &attributes, 
				NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);				
	}

	if (hdbP)
	{
		error = errNone;	
	}

	*dbP = hdbP;
	
	return error;
	
} // DBOpen

/*
 * releaseRecord
 */
static void ReleaseRecord(DmOpenRef* dbP, UInt16* idxP)
{
	UInt16	attrP = 0;
		
	if (!DmRecordInfo(*dbP, *idxP, &attrP, NULL, NULL))
	{
		if (attrP & dmRecAttrBusy)							
			DmReleaseRecord(*dbP, *idxP, false);
	}
	
} // releaseRecord

/*
 * LoadGroupsList
 */
Boolean LoadGroupsList(ListType* pList, Char* list, MemHandle* listHP)
{
	Boolean			retVal = false;
	Char**          listOptions;
	
	if (*listHP != NULL) // unlock... // don't want problems
	{
        MemHandleUnlock(*listHP);
        MemHandleFree(*listHP);
        *listHP = NULL;
    }
    
	if (pList)
	{
		DmOpenRef		dbP;
		
		DBOpen(&dbP, dmModeReadOnly);
		
		if (dbP)	
		{
			MemHandle		recordH;
			Groups_t*		recordP;
			
			char			nameStr[LEN_GroupName];
			UInt16 			nameListSize = 0;
			UInt16			nameStrSize = 0;
			
			UInt16			numItems = DmNumRecordsInCategory(dbP, dmAllCategories);
			UInt16 			i = 0;
			
			if (numItems)
			{
				for (i = 0 ; i < numItems ; i++)
				{
					recordH = DmQueryRecord(dbP, i);
					if (recordH)
					{	
						recordP = (Groups_t*) MemHandleLock(recordH);
						if (recordP)
						{
							StrCopy(nameStr, recordP->Name);
						}
	
						MemHandleUnlock(recordH);
					}
					
					nameStrSize = StrLen(nameStr) + 1;
					MemMove(list + nameListSize, nameStr, nameStrSize);
					nameListSize = nameListSize + nameStrSize;				
				}
	
			
				*listHP = SysFormPointerArrayToStrings(list, numItems);
			    listOptions = MemHandleLock(*listHP); // to be unlocked somewhere...
	
			    LstSetListChoices(pList, listOptions, numItems);
		    
			    retVal = true; 
			}		    
		    else
		    {
		    	LstSetListChoices(pList, NULL, 0);
			}

			DmCloseDatabase(dbP);	
		}
	}
	
	return (retVal);
	
} // LoadGroupsList

/*
 * GroupsList.c
 */