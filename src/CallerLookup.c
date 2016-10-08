/*
 * CallerLookup.c
 */
 
#include "CallerLookup.h"

// Prototypes
static Boolean 		CallerLookupCallbackFunc(void* ref);

/*
 * CallerLookupCallbackFunc
 */
static Boolean CallerLookupCallbackFunc(void* ref)
{
    return false;
}

/*
 * CallerLookup
 */
Boolean CallerLookup(char* number, char* formatStr, char* outStr, UInt16 outStrSize)
{
    Boolean 					retVal = false;
    SysNotifyParamType* 		pNotifyParams;
    AddrCallerIDParamsType* 	pLookupParams;
    
    if (!outStr)
    	return retVal;
    
    pNotifyParams = MemPtrNew(sizeof(SysNotifyParamType));
    pLookupParams = MemPtrNew(sizeof(AddrCallerIDParamsType));
    
    if (pNotifyParams && pLookupParams)
	{
		Err						error = errNone;
		UInt32					result;
		UInt16					cardNo;
		LocalID					dbID;
		DmSearchStateType 		stateInfo;
	
		UInt32 					plainVal = 0;
	    
	    MemSet(pNotifyParams, sizeof(SysNotifyParamType), 0);
	    MemSet(pLookupParams, sizeof(AddrCallerIDParamsType), 0);

	    pNotifyParams->notifyType = addrAppNotificationCmdCallerID;
	    pNotifyParams->notifyDetailsP = pLookupParams;
	
	    pLookupParams->lookupString = number;
	    pLookupParams->formatString = formatStr;
	    pLookupParams->callback = CallerLookupCallbackFunc;
	    pLookupParams->ref = (void*)&plainVal;
	
	 	if(DmGetNextDatabaseByTypeCreator(true, &stateInfo, sysFileTApplication, kPalmOneCreatorIDContacts, true, &cardNo, &dbID))
	 		return retVal;
		error = SysAppLaunch(cardNo, dbID, 0, sysAppLaunchCmdNotify, pNotifyParams, &result);	    
	    // AppCallWithCommand(kPalmOneCreatorIDContacts, sysAppLaunchCmdNotify, pNotifyParams);
	    
	    if((!error) && (pLookupParams->resultString))
	    {
	    	StrNCopy(outStr, pLookupParams->resultString, (outStrSize - 1));
	    	outStr[outStrSize-1] = chrNull;
	    	
	    	retVal = true;
	    }
	}

	if (pLookupParams)
		MemPtrFree(pLookupParams);

	if (pNotifyParams)
		MemPtrFree(pNotifyParams);

    return retVal;
    
} /* CallerLookup*/

/*
 * CallerLookup.c
 */
