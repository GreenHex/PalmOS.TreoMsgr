/*
 * Prefs.c
 */
 
#include "Prefs.h"

/*
 * writePrefs
 */
void writePrefs(AppPreferences_t* prefsP)
{
#ifdef WITH_REGISTRATION
	encAppPrefs(prefsP, encrypt);
#endif /* WITH_REGISTRATION */

	PrefSetAppPreferences(appFileCreator, appPrefID, appPrefVersionNum, prefsP, sizeof(AppPreferences_t), true);

#ifdef WITH_REGISTRATION
	encAppPrefs(prefsP, decrypt); // decrypt immediately...
#endif /* WITH_REGISTRATION */

} // writePrefs

/*
 * readPrefs
 */
void readPrefs(AppPreferences_t* prefsP)
{	
	UInt16 prefSize = sizeof(AppPreferences_t);
	
	if (PrefGetAppPreferences(appFileCreator, appPrefID, prefsP, &prefSize, true) == noPreferenceFound)
	{	
		// default application preference values
		prefsP->bEnabled = PREFS_ENABLED;
		MemMove(prefsP->strRegKey, PREFS_REG_KEY, StrLen(prefsP->strRegKey));
		prefsP->bDeleteCompleted = PREFS_DELETE_COMPLETED;
		prefsP->bAskDeleteCompletedFirst = PREFS_ASK_DELETE_FIRST;
		prefsP->bArchiveDeletedItems = PREFS_ARCHIVE_DELETED;
		prefsP->bRetryFailedTasks = PREFS_RETRY_FAILED_TASKS;
		prefsP->retryCount = PREFS_RETRY_COUNT;
		writePrefs(prefsP);
	}
	
#ifdef WITH_REGISTRATION
	else
	{
		encAppPrefs(prefsP, decrypt); // prefsP encrypted, so decrypt it...
	}
#endif /* WITH_REGISTRATION*/

} // readPrefs

/*
 * writeUSPrefs
 */
void writeUSPrefs(AppUSPrefs_t* usPrefsP)
{
	PrefSetAppPreferences(appFileCreator, appPrefID, appPrefVersionNum, usPrefsP, sizeof(AppUSPrefs_t), false);

} // writeUSPrefs

/*
 * readUSPrefs
 */
void readUSPrefs(AppUSPrefs_t* usPrefsP)
{	
	UInt16 usPrefSize = sizeof(AppUSPrefs_t);
	
	if (PrefGetAppPreferences(appFileCreator, appPrefID, usPrefsP, &usPrefSize, false) == noPreferenceFound)
	{	
		usPrefsP->idx = 0;
		usPrefsP->time = 0;
		MemSet(usPrefsP->Task, LEN_Description, 0);
		MemSet(&(usPrefsP->msg), sizeof(Message_t), 0);
		usPrefsP->isComplete = false;
		usPrefsP->isSecret = false;
				
		writeUSPrefs(usPrefsP);
	}
	
} // readUSPrefs

/*
 * writeStats
 */
void writeStats(AppStats_t* statsP)
{
	PrefSetAppPreferences(appFileCreator, appStatsID, appPrefVersionNum, statsP, sizeof(AppStats_t), true);

} // writeStats

/*
 * readStats
 */
void readStats(AppStats_t* statsP)
{
	UInt16 AppStatsSize = sizeof(AppStats_t);
	
	if (PrefGetAppPreferences(appFileCreator, appStatsID, statsP, &AppStatsSize, true) == noPreferenceFound)
	{	
		statsP->numSent = 0;
		statsP->numFailed = 0;
		statsP->numQueued = 0;
		
		writeStats(statsP);
	}
	
} // readStats

/*
 * writePhnStatus
 */
/*
void writePhnStatus(SMSMessageStatus status)
{
	PhnStatus_t			phnStatus;
	
	phnStatus.status = status;
	
	PrefSetAppPreferences(appFileCreator, appPhnStatusID, appPrefVersionNum, &phnStatus, sizeof(PhnStatus_t), false);

} */ // writePhnStatus

/*
 * readPhnStatus
 */
/*
SMSMessageStatus readPhnStatus(void)
{	
	PhnStatus_t			phnStatus;
	UInt16 				phnStatusSize = sizeof(PhnStatus_t);
	
	if (PrefGetAppPreferences(appFileCreator, appPhnStatusID, &phnStatus, &phnStatusSize, false) == noPreferenceFound)
	{	
		phnStatus.status = kNone;

		writePhnStatus(phnStatus.status);
	}
	
	return (phnStatus.status);
	
} */ // readPhnStatus

/*
 * writeDeferredData
 */
/*
void writeDeferredData(AppUSPrefs_t* usPrefsP)
{
	PrefSetAppPreferences(appFileCreator, appDeferedEvtID, appPrefVersionNum, usPrefsP, sizeof(AppUSPrefs_t), false);

} */ // writeDeferredData

/*
 * readDeferredData
 */
/*
Boolean readDeferredData(AppUSPrefs_t* usPrefsP)
{	
	Boolean				retVal = true;
	UInt16 				usPrefSize = sizeof(AppUSPrefs_t);
	
	if (PrefGetAppPreferences(appFileCreator, appDeferedEvtID, usPrefsP, &usPrefSize, false) == noPreferenceFound)
	{	
		retVal = false;	
	}
	
	// delete preferences
	PrefSetAppPreferences(appFileCreator, appDeferedEvtID, appPrefVersionNum, usPrefsP, 0, false);
	
	return (retVal);
	
} */ // readDeferredData

/*
 * Prefs.c
 */