/*
 * AppMain.c
 */
 
#include <PalmOS.h>
#include <PalmTypes.h>
#include <Hs.h>
#include <HsPhone.h>
#include <HsNav.h>
#include <HsExt.h> 
#include <palmOneCreators.h>
#include <palmOneResources.h>
#include <Form.h>
#include <List.h>
#include <AlarmMgr.h>
#include <Progress.h>
#include <DataMgr.h>
#include <PrivateRecords.h>
#include <TonesLibTypes.h>
#include <TonesLib.h>

#include "Global.h"
#include "ToDoDB.h"
#include "Win.h" 
#include "Tasks.h"
#include "GroupsList.h"
#include "SMSUtils.h"
#include "Prefs.h"
#include "FormUtils.h" 
#include "StrUtils.h" 
#include "MsgLog.h"
#include "Tasks.h" 
#include "PhoneUtils.h"
#include "AppResources.h"

#ifdef WITH_REGISTRATION
// Expire.c
extern Boolean 				isExpired(void);
extern UInt16 				DisplayRegForm(Char* StrKey) EXTRA_SECTION_ONE;

// Encrypt.c
extern Boolean 				VerifyRegistration(Char* RegKey);

#endif /* WITH_REGISTRATION */

// Prototypes...
static Err 					AppStart(void);
static void 				AppStop(void);
static Boolean 				AppHandleEvent(EventType* pEvent);
static void 				AppEventLoop(void);

void 						RegisterForNotifications(Boolean bRegister);
void 						setupAlarm(void);

UInt32 						PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags);

// Globals
UInt16						topRow; /* top row of the table */
UInt16						gCurrentIdx; /* the actual index of the record in the DB */
TableDisplay_e				currentForm;
Char*						ctlDateLabel;
Char*						ctlTimeLabel;
Boolean						bTaskEditing; /* task is being edited, NOT created */
Char						gGroupsList[LEN_GroupsList];
MemHandle					gGroupsH;
DateType					gDueDate;
Boolean						gHasRepeatInfo;
ToDoRepeatInfoType			gToDoRepeatInfo;
Boolean						gRepeatAfterCompleted;


/*
 * RegisterForNotifications
 */
void RegisterForNotifications(Boolean bRegister)
{
	UInt16 				cardNo; 
	LocalID  			dbID;
	
	SysCurAppDatabase(&cardNo, &dbID);
		
	if (bRegister)
	{
		SysNotifyRegister(cardNo, dbID, sysNotifyAppQuittingEvent, NULL, sysNotifyNormalPriority, NULL);
		SysNotifyRegister(cardNo, dbID, sysNotifyTimeChangeEvent, NULL, sysNotifyNormalPriority, NULL);
		setupAlarm();	
	}
	else
	{
		SysNotifyUnregister(cardNo, dbID, sysNotifyAppQuittingEvent, sysNotifyNormalPriority);
		SysNotifyUnregister(cardNo, dbID, sysNotifyTimeChangeEvent, sysNotifyNormalPriority);
		AlmSetAlarm(cardNo, dbID, 0, 0, true);	
	}
				
} // RegisterForNotifications

/*
 * AppStart
 */
static Err AppStart(void)
{
	gGroupsH = NULL;
	bTaskEditing = false;
	
	FrmGotoForm(MAIN_FORM);
	return errNone;

} // AppStart

/*
 * AppStop
 */
static void AppStop(void)
{	
	if (gGroupsH != NULL) // unlock... // don't want problems
	{
        MemHandleUnlock(gGroupsH);
        MemHandleFree(gGroupsH);
        gGroupsH = NULL;
    }
    
    FrmCloseAllForms();	
        
} // AppStop

/*
 * AppHandleEvent
 */
static Boolean AppHandleEvent(EventType* pEvent)
{
	UInt16 		formID;
	FormType* 	pForm = NULL;
	Boolean		handled = false;
	
	if (pEvent->eType == frmLoadEvent)
	{
		formID = pEvent->data.frmLoad.formID;
		
		pForm = FrmInitForm(formID);
		FrmSetActiveForm(pForm);
		
		if (formID == MAIN_FORM)
			FrmSetEventHandler(pForm, MainFormHandleEvent);
/*		
		if (formID == TASK_FORM)
			FrmSetEventHandler(pForm, TaskFormHandleEvent);
*/					
		if (formID == LOG_FORM)
			FrmSetEventHandler(pForm, LogFormHandleEvent);
		
		if (formID == GROUPS_FORM)
			FrmSetEventHandler(pForm, GroupsFormHandleEvent);
			
		if (formID == PREFS_FORM)
			FrmSetEventHandler(pForm, PrefsFormHandleEvent);
	
		handled = true;
	}
	
	return handled;
	
} // AppHandleEvent

/*
 * AppEventLoop
 */
static void AppEventLoop(void)
{
	Err			error = errNone;
	EventType	event;

	do {
		EvtGetEvent(&event, evtWaitForever);

		if (SysHandleEvent(&event))
			continue;
		
		if (MenuHandleEvent(0, &event, &error))
			continue;
			
		if (AppHandleEvent(&event))
			continue;

		FrmDispatchEvent(&event);

	} while (event.eType != appStopEvent);

} // AppEventLoop

/*
 * setupAlarm
 */
void setupAlarm(void)
{
	UInt16				cardNo = 0;
	LocalID				dbID;
	AppUSPrefs_t*		usPrefsP = NULL;

	usPrefsP = MemPtrNew(sizeof(AppUSPrefs_t));
	if (!usPrefsP) return;

	MemSet(usPrefsP, sizeof(AppUSPrefs_t), 0);

	SysCurAppDatabase(&cardNo, &dbID);
	
	if (GetFirstValidTaskIdx(usPrefsP))	
	{
		// StrNCopy((Char*)(usPrefsP->msg.DisplayName), (Char*)(usPrefsP->msg.Numbers), (LEN_CallerID - 1));
		// usPrefsP->msg.DisplayName[LEN_CallerID - 1] = chrNull;

		AlmSetAlarm(cardNo, dbID, 0, usPrefsP->time + MESSAGING_ADVANCE_TIME, true);
	}
	else
	{
		AlmSetAlarm(cardNo, dbID, 0, 0, true);
	}

	writeUSPrefs(usPrefsP);

	MemPtrFree(usPrefsP);

	return;
	
} // setupAlarm

/*
 * PilotMain
 */
UInt32 PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
	Err 						error = errNone;
	AppPreferences_t* 			prefsP = NULL;

	switch (cmd)
	{
		case sysAppLaunchCmdNormalLaunch:
	
			prefsP = MemPtrNew(sizeof(AppPreferences_t));
			if (!prefsP)
			{
				error = phnErrMemory;
				break;	
			}
			MemSet(prefsP, sizeof(AppPreferences_t), 0);
		  	readPrefs(prefsP);
  		
			gGroupsH = NULL; /* !? */
			  				
			// Check device
			if ((error = RomVersionCompatible(MIN_VERSION, launchFlags)))
			{
				break;
			}
			
#ifdef WITH_REGISTRATION 			
			// exit if expired.
			if (isExpired())
			{
				if (!VerifyRegistration(prefsP->strRegKey))
				{
					if (DisplayRegForm(prefsP->strRegKey) == 0)					
					{
						if (VerifyRegistration(prefsP->strRegKey))
						{
							writePrefs(prefsP);
							
							FrmCustomAlert(InfoOKAlert, "Thank you for registering!", "", "");
						}
						else
						{
							prefsP->bEnabled = false; // disable TreoFlex...
							writePrefs(prefsP);
							
							FrmCustomAlert(InfoOKAlert, "Invalid key, exiting!", "", "");	
							
							break;
						}
					}
					else
					{
						prefsP->bEnabled = false; // disable TreoFlex...
						writePrefs(prefsP);

						break;
					}
				}
			}
#endif /* WITH_REGISTRATION */	
			
			if ((error = AppStart()) == 0)
			{
				privateRecordViewEnum 		securityStatus = 0;
				
				securityStatus = PrefGetPreference(prefShowPrivateRecords);
				
				CreateCategory();
				
				AppEventLoop();
			
				readPrefs(prefsP);
				RegisterForNotifications(prefsP->bEnabled);

				if ((securityStatus != showPrivateRecords)
						&& (PrefGetPreference(prefShowPrivateRecords) == showPrivateRecords))
				{
					PrefSetPreference(prefShowPrivateRecords, securityStatus);
				}

				AppStop();
			}

			break;
			
		case sysAppLaunchCmdSystemReset:
		
			prefsP = MemPtrNew(sizeof(AppPreferences_t));
			if (!prefsP)
			{
				error = phnErrMemory;
				break;	
			}
			MemSet(prefsP, sizeof(AppPreferences_t), 0);
		  	readPrefs(prefsP);
		  	
			// Check device
			if (!(error = RomVersionCompatible(MIN_VERSION, launchFlags)))
			{
				RegisterForNotifications(prefsP->bEnabled);
			}		
			break;
			
		case sysAppLaunchCmdNotify:
			{
				SysNotifyParamType* 		notifyParam = (SysNotifyParamType*)cmdPBP;
				
				prefsP = MemPtrNew(sizeof(AppPreferences_t));
				if (!prefsP)
				{
					error = phnErrMemory;
					break;	
				}
				MemSet(prefsP, sizeof(AppPreferences_t), 0);
			  	readPrefs(prefsP);

#ifdef WITH_REGISTRATION		  	
				// exit if expired.
				if (!VerifyRegistration(prefsP->strRegKey))
				{
					if (isExpired())
					{
						break;
					}
				}
#endif /* WITH_REGISTRATION */		
				if (notifyParam->notifyType == sysNotifyAppQuittingEvent)
				{
					SysNotifyAppLaunchOrQuitType* 		notifyDetailsP = (SysNotifyAppLaunchOrQuitType*)notifyParam->notifyDetailsP;
					UInt16								cardNo;
					LocalID								dbID;
					UInt16								cardNo2Day;
					LocalID								dbID2Day;
//					UInt16								cardNoDateBk5;
//					LocalID								dbIDDateBk5;
					UInt16								cardNoDateBk6;
					LocalID								dbIDDateBk6;
					UInt16								cardNoAgendus;
					LocalID								dbIDAgendus;
					DmSearchStateType 					stateInfo;

					DmGetNextDatabaseByTypeCreator(true, &stateInfo,
							'appl', 'StHr', true, &cardNo2Day, &dbID2Day);

//					DmGetNextDatabaseByTypeCreator(true, &stateInfo,
//							'appl', 'CESF', true, &cardNoDateBk5, &dbIDDateBk5);

					DmGetNextDatabaseByTypeCreator(true, &stateInfo,
							'appl', 'CESF', true, &cardNoDateBk6, &dbIDDateBk6);

					DmGetNextDatabaseByTypeCreator(true, &stateInfo,
							'appl', 'Actn', true, &cardNoAgendus, &dbIDAgendus);
					
					if (!DmGetNextDatabaseByTypeCreator(true, &stateInfo,
							'appl', kPalmOneCreatorIDTasks, true, &cardNo, &dbID))
					{
						if ((notifyDetailsP->dbID == dbID)
								|| (notifyDetailsP->dbID == dbID2Day)
						/*		|| (notifyDetailsP->dbID == dbIDDateBk5) */
								|| (notifyDetailsP->dbID == dbIDDateBk6)
								|| (notifyDetailsP->dbID == dbIDAgendus))
						{
							setupAlarm();
						}
					}
					((SysNotifyParamType*)cmdPBP)->handled = true;
				}
				else if (notifyParam->notifyType == sysNotifyTimeChangeEvent)
				{
					setupAlarm();
					((SysNotifyParamType*)cmdPBP)->handled = true;	
				}
			}
			break;
		
		case sysAppLaunchCmdAlarmTriggered:
	
			((SysAlarmTriggeredParamType*)cmdPBP)->purgeAlarm = false;
			
			break;
			
		case sysAppLaunchCmdDisplayAlarm:
		
			if (!((SysDisplayAlarmParamType*)cmdPBP)->ref)
			{				
				UInt16				idx = 0;			
				AppUSPrefs_t*		usPrefsP = NULL;
				ProgressPtr			pProgress;
				
				((SysDisplayAlarmParamType*)cmdPBP)->ref += 1;
				
				prefsP = MemPtrNew(sizeof(AppPreferences_t));
				if (!prefsP)
				{
					error = phnErrMemory;
					break;	
				}
				MemSet(prefsP, sizeof(AppPreferences_t), 0);
			  	readPrefs(prefsP);
				
				usPrefsP = MemPtrNew(sizeof(AppUSPrefs_t));
				if (!usPrefsP)
				{
					error = phnErrMemory;
					break;
				}
				MemSet(usPrefsP, sizeof(AppUSPrefs_t), 0);
				readUSPrefs(usPrefsP);
				
				if (usPrefsP->time) // is valid entry...
				{
					Int16		retryCount = (prefsP->bRetryFailedTasks) ? prefsP->retryCount : 0;
					
					idx = usPrefsP->idx;
					
					pProgress = PrgStartDialog(APP_NAME, SendMsgCallback, NULL);
					if (SplitGroupAndSendMessage(usPrefsP, pProgress, retryCount) == errNone)
					{
						SndPlaySystemSound( sndClick );						
					
						if ((SetCompletionStatus(&idx)) && (prefsP->bDeleteCompleted))
						{
							Boolean 	bDeleteTask = false;
						
							if (prefsP->bAskDeleteCompletedFirst)
							{
								if (!FrmCustomAlert(ConfirmationOKCancelAlert, "Delete Message To:\n", usPrefsP->msg.DisplayName, "?"))
								{
									bDeleteTask = true;
								}
							}
							else
							{
								bDeleteTask = true;
							}
							
							if (bDeleteTask)
							{
								Boolean 	retVal = false;
								
								if (prefsP->bArchiveDeletedItems)
								{
									retVal = DeleteTask(idx, archiveRecord);
								}
								else
								{
									retVal = DeleteTask(idx, deleteRecord);
								}
								
								if (retVal)
								{
									MsgLog(usPrefsP, "Message Deleted");
								}
							}
						}
					}
					else
					{
						SndPlaySystemSound( sndError );
					}
					
					setupAlarm();
					
					PrgStopDialog(pProgress, false);			
				}
				
				if (usPrefsP)
				{
					MemPtrFree(usPrefsP);
				}
					
				usPrefsP = NULL;
			}
			break;
			
		case phnLibLaunchCmdEvent:
			{
				PhnEventPtr 	eventP = (PhnEventPtr)cmdPBP;
				
				if (eventP->eventType == phnEvtMessageStat)
				{
//					writePhnStatus(eventP->data.params.newStatus);
					
					eventP->acknowledge = true;				
				}
			}
			break;
			
		default:
			break;
	}

	if (prefsP)
		MemPtrFree(prefsP);

	return (error);
	
} // PilotMain

/*
 * AppMain.c
 */