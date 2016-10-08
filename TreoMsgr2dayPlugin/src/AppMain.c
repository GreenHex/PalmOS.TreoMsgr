/*
 * AppMain.c
 */

#include <PalmOS.h>
#include <PalmTypes.h>

#include "../../TreoMsgr/src/Common.h"
#include "AppResources.h"

// Defines
#define pluginFileCreator		'Tm2d'	// register your own at http://www.palmos.com/dev/creatorid/
#define pluginFileType			'PlGn'
#define	pluginFileName			"TreoMsgr2dayPlugin"
#define appFileCreator			'TrMe'
#define appVersionNum			0x01
#define appPrefID				0x00
#define appPrefVersionNum		0x01

typedef   struct {
    UInt16  					cmd;
    UInt32  					dwRequestForTime;
    char    					szResult[501];
} todayPluginStruct;

// Prototypes
static Boolean 					ReadPrefs(AppPreferences_t* prefsP);
static Boolean 					ReadUSPrefs(AppUSPrefs_t* usPrefsP);
static void 					GetNextMsgStr(AppUSPrefs_t* usPrefsP, Char* str);
static void 					HandlePlugin(MemPtr cmdPBP);

UInt32 							PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags);

Boolean ReadPrefs(AppPreferences_t* prefsP)
{	
	UInt16 prefSize = sizeof(AppPreferences_t);
	
	if (PrefGetAppPreferences(appFileCreator, appPrefID, prefsP, &prefSize, true) == noPreferenceFound)
	{
		return (false);
	}

	encAppPrefs(prefsP, decrypt); // prefsP encrypted, so decrypt it...
	return (true);

} // readPrefs

/*
 * ReadUSPrefs
 */
static Boolean ReadUSPrefs(AppUSPrefs_t* usPrefsP)
{	
	UInt16 			usPrefSize = sizeof(AppUSPrefs_t);
	
	if (PrefGetAppPreferences(appFileCreator, appPrefID, usPrefsP, &usPrefSize, false) == noPreferenceFound)
		return (false);
		
	return (true);
	
} // readUSPrefs

/*
 * GetStrDateTime
 */
static void GetStrDateTime(UInt32 TimeSecs, Char* dateStr, Char* timeStr)
{
	DateTimeType 		dtNow;
	
	TimSecondsToDateTime(TimeSecs, &dtNow);
	
	// DateToAscii(dtNow.month, dtNow.day, dtNow.year, PrefGetPreference(prefDateFormat), dateStr);
	// TimeToAscii(dtNow.hour, dtNow.minute, PrefGetPreference(prefTimeFormat), timeStr);
	
	StrPrintF(dateStr, "%02d/%02d/%02d", dtNow.day, dtNow.month, dtNow.year);
	StrPrintF(timeStr, "%02d:%02d", dtNow.hour, dtNow.minute);
		
} // GetStrDateTime


/*
 * HandlePlugin
 */
static void HandlePlugin(MemPtr cmdPBP)
{
    todayPluginStruct* 		pp = (todayPluginStruct*)cmdPBP;
    AppPreferences_t		prefs;
	DmSearchStateType 		stateInfo;
	UInt16					cardNo;
	LocalID					dbID = 0;
    UInt32					almTime = 0;
    UInt32					ref = 0;
    
    if (pp)
	{			
		if (DmGetNextDatabaseByTypeCreator(true, &stateInfo, sysFileTApplication, appFileCreator, true, &cardNo, &dbID))
		{
			StrCopy(pp->szResult, "<I>2000<L><CR><T>TreoMsgr Not Installed</T></C>");
		}
		else if (!ReadPrefs(&prefs))
		{
			StrCopy(pp->szResult, "<I>2000<L><CR><T>TreoMsgr Not Setup</T></C>");
		}
		else if (!prefs.bEnabled)
		{	
			StrCopy(pp->szResult, "<I>2000<L><CR><T>TreoMsgr Disabled</T></C>");
		}
		else
		{
			AppUSPrefs_t			usPrefs;
			
			MemSet(&usPrefs, sizeof(AppUSPrefs_t), 0);
			
			ReadUSPrefs(&usPrefs);
			
			if (usPrefs.time)
			{
				Char					dateStr[dateStringLength];
				Char					timeStr[timeStringLength];
	
				GetStrDateTime(usPrefs.time, dateStr, timeStr);
		
				StrCopy(pp->szResult, "<I>2000<L><T>Next Msg: </T><CG><T>");
				StrCat(pp->szResult, dateStr);
				StrCat(pp->szResult, " ");
				StrCat(pp->szResult, timeStr);
				StrCat(pp->szResult, "</T></C>");
			}
			else 
			{
				StrCopy(pp->szResult, "<I>2000<L><CB><T>No Scheduled Messages</T></C>");
			}
			
		}			
	}    
    return;

} // HandlePlugin

/*
 * PilotMain
 */
UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	Err error = errNone;

	switch (cmd) {
		
		// This is the custom launch code for 2day
		case sysAppLaunchCmdCustomBase:
			HandlePlugin(cmdPBP);
			break;
		
		case sysAppLaunchCmdNormalLaunch:
			{
				DmSearchStateType 		stateInfo;
				UInt16					cardNo;
				LocalID					dbID;
    			UInt16					cmd = sysAppLaunchCmdNormalLaunch;
			
				if (!DmGetNextDatabaseByTypeCreator(true, &stateInfo, sysFileTApplication, appFileCreator, true, &cardNo, &dbID))
				{
					error =	SysUIAppSwitch(cardNo, dbID, cmd, NULL);
				}
			}
			break;

		default:
		
			break;
	}
	
	return error;
	
} // PilotMain


/*
 * AppMain.c
 */