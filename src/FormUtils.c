/*
 * FormUtils.c
 */
#include "FormUtils.h"

// Prototypes
static Boolean				DisplayPrefsForm(void) EXTRA_SECTION_ONE;
static void 				InitializePrefsForm(FormType* pForm, AppPreferences_t* pPrefs) EXTRA_SECTION_ONE;
static Boolean 				DisplayGroupsForm(void) EXTRA_SECTION_ONE;
static Boolean 				SaveGroup(FormType* pForm, Groups_t* groupP) EXTRA_SECTION_ONE;
#ifdef WITH_REGISTRATION
static Boolean 				DisplayRegisterForm(FormType* pOldForm) EXTRA_SECTION_ONE;
#endif /* WITH_REGISTRATION */
static void 				SetAboutLabels(FormType* pForm, UInt16 labelID, UInt16 regLabelID, Boolean registered) EXTRA_SECTION_ONE;
static void 				SetNextMsgStr(Boolean draw) EXTRA_SECTION_ONE;
static void 				InitializeGroupsForm(FormType* pForm) EXTRA_SECTION_ONE;
static void 				InitializeMainForm(FormType* pForm, AppPreferences_t* prefsP) EXTRA_SECTION_ONE;
static Boolean				DisplayStatisticsForm(FormType* pOldForm) EXTRA_SECTION_ONE;
static void 				InitializeStatisticsForm(FormType* pForm) EXTRA_SECTION_ONE;
static Boolean 				StatsFormHandleEvent(EventType* pEvent) EXTRA_SECTION_ONE;

/*
 * DisplayPrefsForm
 */
static Boolean	DisplayPrefsForm(void)
{
	FrmPopupForm(PREFS_FORM);

	return (true);
	
} // DisplayPrefsForm

/*
 * InitializePrefsForm
 */
static void InitializePrefsForm(FormType* pForm, AppPreferences_t* pPrefs)
{
	ControlType*			pCtl = NULL;
	ListType*				pList = NULL;
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, PREFS_DELETE_COMPLETED_CHECKBOX));
	CtlSetValue(pCtl, pPrefs->bDeleteCompleted);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, PREFS_ASK_FIRST_CHECKBOX));
	CtlSetValue(pCtl, pPrefs->bAskDeleteCompletedFirst);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, PREFS_ARCHIVE_DELETED_CHECKBOK));
	CtlSetValue(pCtl, pPrefs->bArchiveDeletedItems);

	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, PREFS_RESCHEDULE_CHECKBOX));
	CtlSetValue(pCtl, pPrefs->bRetryFailedTasks);

	pList = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, PREFS_RESCHEDULE_LIST));
	LstSetSelection(pList, pPrefs->retryCount);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, PREFS_RESCHEDULE_POPUP));
	CtlSetLabel(pCtl, LstGetSelectionText(pList, LstGetSelection(pList)));
	
} // InitializePrefsForm

/*
 * PrefsFormHandleEvent
 */
Boolean PrefsFormHandleEvent(EventType* pEvent)
{
	Boolean 				handled = false;
	FormType*				pForm = FrmGetActiveForm();
	ControlType* 			pCtl = NULL;
	ListType*				pList = NULL;
	AppPreferences_t		prefs;
	
	MemSet(&prefs, sizeof(AppPreferences_t), 0);
	readPrefs(&prefs);
	
	switch (pEvent->eType)
	{
		case frmOpenEvent:
			
			InitializePrefsForm(pForm, &prefs);
			
			FrmDrawForm(pForm);
			
			handled = true;
			
			break;
		
		case ctlSelectEvent:
			
			switch (pEvent->data.ctlSelect.controlID)
			{
				case PREFS_DONE_BUTTON:
		
					pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, PREFS_DELETE_COMPLETED_CHECKBOX));
					prefs.bDeleteCompleted = (CtlGetValue(pCtl) == 1); 
				
					pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, PREFS_ASK_FIRST_CHECKBOX));
					prefs.bAskDeleteCompletedFirst = (CtlGetValue(pCtl) == 1); 
					
					pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, PREFS_ARCHIVE_DELETED_CHECKBOK));
					prefs.bArchiveDeletedItems = (CtlGetValue(pCtl) == 1);
					
					pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, PREFS_RESCHEDULE_CHECKBOX));
					prefs.bRetryFailedTasks = (CtlGetValue(pCtl) == 1);
				
					pList = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, PREFS_RESCHEDULE_LIST));
					prefs.retryCount = LstGetSelection(pList);
							
					writePrefs(&prefs);
								
					FrmReturnToForm(0);
			
					handled = true;
							
					break;
				
				case PREFS_CANCEL_BUTTON:
									
					FrmReturnToForm(0);
					
					handled = true;
					
					break;
					
				case PREFS_DELETE_COMPLETED_CHECKBOX:				

					pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, PREFS_ASK_FIRST_CHECKBOX));
					
					CtlSetValue(pCtl, (pEvent->data.ctlSelect.on));
					CtlSetEnabled(pCtl, (pEvent->data.ctlSelect.on));
					
					pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, PREFS_ARCHIVE_DELETED_CHECKBOK));
					
					CtlSetValue(pCtl, (pEvent->data.ctlSelect.on));
					CtlSetEnabled(pCtl, (pEvent->data.ctlSelect.on));
					
					handled = true;	

					break;
										
				default:
					break;
			}
			break;
			
		default:
			break;
	}
	
	return (handled);
	
} // PrefsFormHandleEvent

/*
 * DisplayGroupsForm
 */
static Boolean DisplayGroupsForm(void)
{
	FrmPopupForm(GROUPS_FORM);

	return (true);
	
} // DisplayGroupsForm

/*
 * InitializeGroupsForm
 */
static void InitializeGroupsForm(FormType* pForm)
{
	ListType*			pList = NULL;
	
	FrmSetMenu(pForm, MENU_TASK_EDIT);
	
	pList = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, GROUPS_LIST));
	
	if (LoadGroupsList(pList, gGroupsList, &gGroupsH))
	{
		LstSetHeight(pList, 3);
		LstSetSelection(pList, noListSelection);
	}	
/*
 * UNUSED
 * SetLogFormSecurityButton(pForm, GROUPS_SECURITY_BUTTON);
 */
 
} // InitializeGroupsForm

/*
 * GroupsFormHandleEvent
 */
Boolean GroupsFormHandleEvent(EventType* pEvent)
{
	Boolean 				handled = false;
	FormType*				pForm = FrmGetActiveForm();
	ControlType*			pCtl = NULL;
	ListType*				pList = NULL;
	FieldType*				pField = NULL;
	MemHandle				hText = NULL;
	char* 					pText = NULL;
	UInt16					idx = dmMaxRecordIndex;
	Groups_t*				groupP = NULL;
	
	groupP = MemPtrNew(sizeof(Groups_t));
	
	if (!groupP)
		return (handled);
	
	MemSet(groupP, sizeof(Groups_t), 0);
	
	switch (pEvent->eType)
	{							
		case frmOpenEvent:
		
			InitializeGroupsForm(pForm);
			
			FrmDrawForm(pForm);
			
			handled = true;
			
			break;
					
		case lstSelectEvent:
				
			if (pEvent->data.lstSelect.listID == GROUPS_LIST)
			{
				SaveGroup(pForm, groupP);
				
				idx = LstGetSelection(pEvent->data.lstSelect.pList);
				
				if (GetGroup(idx, groupP))
				{
					hText = MemHandleNew(LEN_GroupName);
					pText = MemHandleLock(hText);
					StrCopy(pText, groupP->Name);
					pField = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, GROUPS_NAME_FIELD));
					FldSetTextHandle(pField, NULL);
					FldSetTextHandle(pField, hText);
					MemHandleUnlock(hText);
					FldDrawField(pField);
					
					hText = MemHandleNew(LEN_GroupNumbers);
					pText = MemHandleLock(hText);
					StrCopy(pText, groupP->Numbers);
					pField = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, GROUPS_NUMBER_FIELD));
					FldSetTextHandle(pField, NULL);
					FldSetTextHandle(pField, hText);
					MemHandleUnlock(hText);
					FldDrawField(pField);
					
					pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, GROUPS_PRIVATE_CHECKBOX));
				}
			}	
		
			break;
			
		case ctlSelectEvent:
			
			switch (pEvent->data.ctlSelect.controlID)
			{
				case GROUPS_DONE_BUTTON:
										
					if (SaveGroup(pForm, groupP))	
					{						
						FrmReturnToForm(0);
						
						handled = true;
					}					
										
					break;
/*	UNUSED		
				case GROUPS_SECURITY_BUTTON:
					{
						privateRecordViewEnum			OldRecordDispStatus = PrefGetPreference(prefShowPrivateRecords);
						
						if (SecSelectViewStatus() != OldRecordDispStatus)
						{							
							FrmSetActiveForm(pForm);
							
							pList = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, GROUPS_LIST));
							
							LoadGroupsList(pList, gGroupsList, &gGroupsH);
	
							LstSetSelection(pList, noListSelection);
									
							LstDrawList(pList);	
							
							SetLogFormSecurityButton(pForm, GROUPS_SECURITY_BUTTON);
						}
					}
					handled = true;
					
					break;
*/
					
				case GROUPS_DELETE_BUTTON:
					
					pList = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, GROUPS_LIST));
				
					idx = LstGetSelection(pList);
				
					if ((idx != noListSelection)
							&& (FrmCustomAlert(ConfirmationOKCancelAlert, "Delete Group?\n[", LstGetSelectionText(pList, idx), "]") == 0))
					{
						RemoveGroup(&idx);
						
						LoadGroupsList(pList, gGroupsList, &gGroupsH);
	
						LstSetSelection(pList, noListSelection);
								
						LstDrawList(pList);
											
						pField = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, GROUPS_NAME_FIELD));
						FldDelete(pField, 0, LEN_GroupName - 1);
						FldSetTextHandle(pField, NULL);
						
						pField = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, GROUPS_NUMBER_FIELD));				
						FldDelete(pField, 0, LEN_GroupNumbers - 1);
						FldSetTextHandle(pField, NULL);
					}								
					handled = true;
					
					break;
					
				case GROUPS_SAVE_BUTTON:
					
					SaveGroup(pForm, groupP);					
					
					handled = true;
										
					break;
					
				case GROUPS_LOOKUP_BUTTON:
					{			
						AddrLookupParamsType 	AddrParams;
						
						pField = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, GROUPS_NUMBER_FIELD));
						
						if (pField)
						{
							AddrParams.title = "Lookup";
							AddrParams.pasteButtonText = NULL;
							AddrParams.field1 = addrLookupSortField;
							AddrParams.field2 = addrLookupAnyPhones; // addrLookupMobile;
							AddrParams.field2Optional = false;
							AddrParams.userShouldInteract = true;
							AddrParams.formatStringP = "^smsphone, "; // "^listphone, "; // "^mobile, ";
							PhoneNumberLookupCustom(pField, &AddrParams, true);
						}
					}
					handled = true;
					
					break;
										
				default:
					break;
			}
			break;
			
		default:
			break;
	}
	
	if (groupP)
		MemPtrFree(groupP);
		
	return (handled);
	
} // GroupsFormHandleEvent

/* 
 * SaveGroup
 */
static Boolean SaveGroup(FormType* pForm, Groups_t* groupP)
{ 
	Boolean					retVal = false;
	
	ListType*				pList = NULL;
	FieldType*				pField = NULL;
	ControlType*			pCtl = NULL;
//	MemHandle				hText = NULL;
	char* 					pText = NULL;
	UInt16					idx = dmMaxRecordIndex;
	
	FrmSetActiveForm(pForm);
	
	if ((FldGetTextPtr(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, GROUPS_NAME_FIELD)))
			&& FldGetTextPtr(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, GROUPS_NUMBER_FIELD))))
		&& (StrLen(FldGetTextPtr(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, GROUPS_NAME_FIELD)))) 
			&& StrLen(FldGetTextPtr(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, GROUPS_NUMBER_FIELD)))))
		&& (FldDirty(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, GROUPS_NAME_FIELD)))
			|| FldDirty(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, GROUPS_NUMBER_FIELD)))))
	{
		pField = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, GROUPS_NAME_FIELD));
		pText = FldGetTextPtr(pField);
		
		if (pText)
		{
			StrCopy(groupP->Name, pText);
		}
		
		pField = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, GROUPS_NUMBER_FIELD));				
		pText = FldGetTextPtr(pField);
		
		if (pText)
		{
			StrCopy(groupP->Numbers, pText);
		}
		
		pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, GROUPS_PRIVATE_CHECKBOX));
		
		if (StrLen(groupP->Name) && StrLen(groupP->Numbers))
		{
			if (FindGroup(groupP->Name, &idx, NULL))
			{
				if (FrmCustomAlert(ConfirmationOKCancelAlert, "Replace Group?\n[", groupP->Name, "]") == 0)
				{
					ReplaceGroup(&idx, groupP, false); 
				}
			}
			else
			{
				if (FrmCustomAlert(ConfirmationOKCancelAlert, "Create Group?\n[", groupP->Name, "]") == 0)
				{
					SaveNewGroup(groupP, false, &idx);
				}
			}
			
			FldSetDirty(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, GROUPS_NAME_FIELD)), false);
			FldSetDirty(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, GROUPS_NUMBER_FIELD)), false);
			
			pList = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, GROUPS_LIST));
			
			LoadGroupsList(pList, gGroupsList, &gGroupsH);

			// LstSetSelection(pList, idx);
			
			LstDrawList(pList);
			
			retVal = true;	
		}
		else
		{
			retVal = (FrmCustomAlert(ErrOKCancelAlert, "", "Some required fields \nare blank.", "") != 0);
		}
	}
	else // fields NOT dirty
	{
		retVal = true;
	}	
	
	return (retVal);
	
} // SaveGroup

#ifdef WITH_REGISTRATION
/*
 * DisplayRegisterForm
 */
static Boolean DisplayRegisterForm(FormType* pOldForm)
{
	Boolean					retVal = false;
	AppPreferences_t*		prefsP = NULL;

	prefsP = MemPtrNew(sizeof(AppPreferences_t));
	if (!prefsP)
		return (retVal);
		
	MemSet(prefsP, sizeof(AppPreferences_t), 0);
	readPrefs(prefsP);
	
	if (DisplayRegForm(prefsP->strRegKey) == 0) // empty strRegKey passed in...			
	{
		if (VerifyRegistration(prefsP->strRegKey))
		{
			writePrefs(prefsP);
			
//			FrmHideObject(pOldForm, FrmGetObjectIndex(pOldForm, MAIN_REGISTER_BUTTON));
			FrmCustomAlert(InfoOKAlert, "Thank you for registering!", "", "");
			retVal = true;
		}
		else
		{
			FrmCustomAlert(InfoOKAlert, "Invalid key!", "", "");	
			retVal = false;
		}
	}
	
	if(prefsP)
		MemPtrFree(prefsP);
		
	return (retVal);
	
} // DisplayRegisterForm

#endif /* WITH_REGISTRATION */

/*
 * SetAboutLabels
 */
static void SetAboutLabels(FormType* pForm, UInt16 verLabelID, UInt16 regLabelID, Boolean registered)
{
	ControlType* 		pCtl = NULL;
	MemHandle			strH = NULL;
	Char*				strP = NULL;
	Char*				str = NULL;
	
	strH = DmGet1Resource('tver', 1000);
	
	if (strH)
	{
		strP = MemHandleLock(strH);
		
		if (strP)
		{
			Char				verStr[8];
		 
			StrCopy(verStr, "v");
			StrCat(verStr, strP);
			verStr[StrLen((Char*)FrmGetLabel(pForm, verLabelID))] = chrNull;
			
			FrmCopyLabel(pForm, verLabelID, verStr);
			
			MemHandleUnlock(strH);
		}
	   	
	   	DmReleaseResource(strH);
	}
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, regLabelID));

	str = (Char*) CtlGetLabel(pCtl);
			
	if (registered)
	{
		StrCopy(str, "REGISTERED");
	}
	else
	{
		StrCopy(str, "*UNREGISTERED*");
	}
		
	CtlSetLabel(pCtl, str);

	return;
	
} // SetAboutLabels

/*
 * SetNextMsgStr
 */
static void SetNextMsgStr(Boolean draw)
{
	AppPreferences_t		prefs;
	Char					str[15 + dateStringLength + 1 + timeStringLength + 1];
	MsgColor_e 				color = colorDefault;
	
	readPrefs(&prefs);
	
	if (!prefs.bEnabled)
	{
		StrCopy(str, "TreoMsgr Disabled");
		color = colorRed;
	}
	else
	{
		AppUSPrefs_t			usPrefs;
		Char					dateStr[dateStringLength];
		Char					timeStr[timeStringLength];
		
		readUSPrefs(&usPrefs);
		
		if (usPrefs.time)
		{
			GetStrDateTime(usPrefs.time, dateStr, timeStr);
	
			StrCopy(str, "Next Message: ");
			StrCat(str, FormatDate(dateStr));
			StrCat(str, " ");
			StrCat(str, FormatTime(timeStr));
			color = colorGreen;
		}
		else
		{
			StrCopy(str, "No Scheduled Messages");
			color = colorBlue;
		}
	}
		
	DrawNextMsgStr(str, color, draw);
	
	return;
	
} // SetNextMsgStr

/*
 * InitializeMainForm
 */
static void InitializeMainForm(FormType* pForm, AppPreferences_t* prefsP)
{	
	ControlType*			pCtl = NULL;
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, MAIN_ENABLE_CHECKBOX));
	CtlSetValue(pCtl, prefsP->bEnabled);

/*		
	if (VerifyRegistration(prefsP->strRegKey))
	{
		FrmHideObject(pForm, FrmGetObjectIndex(pForm, MAIN_REGISTER_BUTTON));
	}
*/
	CommonInitGauge(pForm);
	
} // InitializeMainForm

/*
 * MainFormHandleEvent
 */
Boolean MainFormHandleEvent(EventType* pEvent)
{
	Boolean 				handled = false;
	FormType* 				pForm = FrmGetActiveForm();
	AppPreferences_t*		prefsP = NULL;
		
	static RectangleType	rectObscured;
	static WinHandle		winH = NULL;

	prefsP = MemPtrNew(sizeof(AppPreferences_t));
	if (!prefsP)
		return (handled);
		
	MemSet(prefsP, sizeof(AppPreferences_t), 0);
	readPrefs(prefsP);

	switch (pEvent->eType)
	{
		case frmOpenEvent:
	
			InitializeMainForm(pForm, prefsP);
			
			FrmDrawForm(pForm);
					
			handled = true;
			
			break;
			
		case nilEvent:
		
			SetNextMsgStr(true);
			
			break;
			
		case penDownEvent:
			{
				Int16		x = pEvent->screenX;
				Int16		y = pEvent->screenY;
				Boolean 	inRect = false;
							
				inRect = (x > NEXT_MSG_STR_LOC_X)
					&& (y > NEXT_MSG_STR_LOC_Y)
					&& (x < (NEXT_MSG_STR_LOC_X + NEXT_MSG_STR_LENGTH))
					&& (y < (NEXT_MSG_STR_LOC_Y + NEXT_MSG_STR_HEIGHT));
					
				if ((prefsP->bEnabled) && (inRect))
				{
					MainFormDispPopupWindow(pForm, &winH, &rectObscured);
					
					handled = true;	
				}				
			}
			break;
			
		case penUpEvent:
		
			if (winH)
			{
				MainFormRestoreAfterPopup(pForm, &winH, &rectObscured);
			
				handled = true;				
			}
			break;
					
		case ctlSelectEvent:
					
			switch (pEvent->data.ctlSelect.controlID)
			{
				case MAIN_ENABLE_CHECKBOX:

					prefsP->bEnabled = (CtlGetValue(pEvent->data.ctlSelect.pControl) == 1);
					
					writePrefs(prefsP);

					RegisterForNotifications(prefsP->bEnabled);						
					/*
					if (!prefsP->bEnabled)
					{
						PrefSetAppPreferences(appFileCreator, appPrefID, appPrefVersionNum, NULL, 0, false);
					}
					*/				
					handled = true;
					
					break;

				case MAIN_NEW_TASK_BUTTON:

					handled = DisplayNewTaskForm(pForm);
					
					break;
					
				case MAIN_VIEW_MSG_BUTTON:
					
					currentForm = dispTasks;

					handled = DisplayLogForm();
		
					break;
					
				case MAIN_VIEW_LOG_BUTTON:
				
					currentForm = dispLog;

					handled = DisplayLogForm();
												
					break;
					
				case MAIN_GROUPS_BUTTON:
				
					handled = DisplayGroupsForm();
				
					break;						
					
				default:
					break;
			}
						
			break;

		case menuOpenEvent:

#ifdef WITH_REGISTRATION 		
			if (VerifyRegistration(prefsP->strRegKey))
			{
				// MenuHideItem(MENU_MAIN_SEPARATOR);
				MenuHideItem(MENU_MAIN_REGISTER);	
			}
#endif /* WITH_REGISTRATION	*/		

			break;
						
		case menuEvent:
										
			switch (pEvent->data.menu.itemID)
			{
				case MENU_MAIN_NEW_MSG:

					handled = DisplayNewTaskForm(pForm);
										
					break;
				
				case MENU_MAIN_VIEW_MSGS:
				
					currentForm = dispTasks;

					handled = DisplayLogForm();	
					
					break;
						
				case MENU_MAIN_LOG:
				
					currentForm = dispLog;
					
					handled = DisplayLogForm();	
				
					break;
#ifdef WITH_REGISTRATION					
				case MENU_MAIN_REGISTER:

					handled = DisplayRegisterForm(pForm);
					
					break;
#endif /* WITH_REGISTRATION */					
				case MENU_MAIN_GROUPS:
				
					handled = DisplayGroupsForm();
				
					break;

				case MENU_MAIN_STATISTICS:
				
					handled = DisplayStatisticsForm(pForm);
					
					break;
										
				case MENU_MAIN_PREFERENCES:
				
					handled = DisplayPrefsForm();
				
					break;
				
				case MENU_MAIN_ABOUT:
					{
						FormType*		pAboutForm = FrmInitForm(ABOUT_FORM);
#ifdef WITH_REGISTRATION						
						SetAboutLabels(pAboutForm, ABOUT_VERSION_LABEL, ABOUT_REG_LABEL, VerifyRegistration(prefsP->strRegKey));
#else
						SetAboutLabels(pAboutForm, ABOUT_VERSION_LABEL, ABOUT_REG_LABEL, true);					
#endif WITH_REGISTRATION						
						FrmDoDialog(pAboutForm);
						
						FrmDeleteForm(pAboutForm);
										
						handled = true;
					}
					break;
					
				default:
					break;
			}			
			break;

		default:
		
			break;
	}
	
	if (prefsP)
		MemPtrFree(prefsP);

	return (handled);
	
} // MainFormHandleEvent

/*
 * DisplayStatisticsForm
 */
static Boolean	DisplayStatisticsForm(FormType* pOldForm) 
{
	FormType*			pForm = NULL;
	
	pForm = FrmInitForm(STATS_FORM);

	FrmSetActiveForm(pForm);
	FrmSetEventHandler(pForm, StatsFormHandleEvent);
	InitializeStatisticsForm(pForm);
	
	FrmDoDialog(pForm);
				 
	FrmDeleteForm(pForm);
	
	pForm = NULL;
	
	FrmSetActiveForm(pOldForm);
	
	return (true);
	
} // DisplayStatisticsForm

/*
 * InitializeStatisticsForm
 */
static void InitializeStatisticsForm(FormType* pForm)
{
	ControlType*	pCtl = NULL;
	Char*			ctlTextP = NULL;
	AppStats_t		stats;
	UInt16			numSent = 0;
	UInt16			numFailed = 0;
	UInt16			numTotal = 0;
	UInt16			numTasks = GetNumTaskRecords(dmModeReadOnly  | dmModeShowSecret);
	UInt16			numQueued = 0;
	UInt16			numLog = GetNumLogRecords(dmModeReadOnly  | dmModeShowSecret);
	Char			numStr[32];
	
	readStats(&stats);

	numSent = stats.numSent;
	numFailed = stats.numFailed;
	
	numTotal = numSent + numFailed;
	
	numQueued = stats.numQueued;
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, STATS_MSGS_SENT_LABEL));
	ctlTextP = (Char*) CtlGetLabel(pCtl);
	StrPrintF(numStr, "%u", numSent);
	StrCopy(ctlTextP, numStr);
	CtlSetLabel(pCtl, ctlTextP);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, STATS_MSGS_FAILED_LABEL));
	ctlTextP = (Char*) CtlGetLabel(pCtl);
	StrPrintF(numStr, "%u", numFailed);
	StrCopy(ctlTextP, numStr);
	CtlSetLabel(pCtl, ctlTextP);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, STATS_MSGS_TOTAL_LABEL));
	ctlTextP = (Char*) CtlGetLabel(pCtl);
	StrPrintF(numStr, "%u", numTotal);
	StrCopy(ctlTextP, numStr);
	CtlSetLabel(pCtl, ctlTextP);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, STATS_NUM_MSGS_LABEL));
	ctlTextP = (Char*) CtlGetLabel(pCtl);
	StrPrintF(numStr, "%u", numTasks);
	StrCopy(ctlTextP, numStr);
	CtlSetLabel(pCtl, ctlTextP);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, STATS_MSGS_IN_QUEUE_LABEL));
	ctlTextP = (Char*) CtlGetLabel(pCtl);
	StrPrintF(numStr, "%u", numQueued);
	StrCopy(ctlTextP, numStr);
	CtlSetLabel(pCtl, ctlTextP);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, STATS_NUM_LOG_ENTRIES_LABEL));
	ctlTextP = (Char*) CtlGetLabel(pCtl);
	StrPrintF(numStr, "%u", numLog);
	StrCopy(ctlTextP, numStr);
	CtlSetLabel(pCtl, ctlTextP);
	
	EvtWakeup();
	
	return;
	
} // InitializeStatisticsForm

/*
 * StatsFormHandleEvent
 */
static Boolean StatsFormHandleEvent(EventType* pEvent)
{
	Boolean 				handled = false;
	FormType*				pForm = FrmGetActiveForm();
	
	if ((!pEvent) || (pEvent->eType == nilEvent))
	{
		ControlType*			pCtl = NULL;
		Char*					ctlTextP = NULL;
		RGBColorType 			rgb;
		IndexedColorType		txtColor;
		Int16					strWidth = 0;
		RectangleType			rect;

		WinPushDrawState();
		
		FntSetFont(boldFont);
		
		// Sent
		pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, STATS_MSGS_SENT_LABEL));
		ctlTextP = (Char*) CtlGetLabel(pCtl);
		rgb.r=0; rgb.g=153; rgb.b=0; // green?
		txtColor = WinRGBToIndex(&rgb);
	    WinSetTextColor(txtColor);
	    strWidth = FntLineWidth(ctlTextP, StrLen(ctlTextP));
	    FrmGetObjectBounds(pForm, FrmGetObjectIndex(pForm, STATS_MSGS_SENT_LABEL), &rect);
		WinDrawChars(ctlTextP, StrLen(ctlTextP), rect.topLeft.x + STATS_TXT_WIDTH - strWidth, rect.topLeft.y);
		
		// Falied
		pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, STATS_MSGS_FAILED_LABEL));
		ctlTextP = (Char*) CtlGetLabel(pCtl);
		rgb.r=250; rgb.g=0; rgb.b=51; // red?
		txtColor = WinRGBToIndex(&rgb);
		WinSetTextColor(txtColor);
		strWidth = FntLineWidth(ctlTextP, StrLen(ctlTextP));
		FrmGetObjectBounds(pForm, FrmGetObjectIndex(pForm, STATS_MSGS_FAILED_LABEL), &rect);
		WinDrawChars(ctlTextP, StrLen(ctlTextP), rect.topLeft.x + STATS_TXT_WIDTH - strWidth, rect.topLeft.y);
			    
		// Total
		pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, STATS_MSGS_TOTAL_LABEL));
		ctlTextP = (Char*) CtlGetLabel(pCtl);
		rgb.r=0; rgb.g=51; rgb.b=200; // Blue
		txtColor = WinRGBToIndex(&rgb);
		WinSetTextColor(txtColor);
		strWidth = FntLineWidth(ctlTextP, StrLen(ctlTextP));
		FrmGetObjectBounds(pForm, FrmGetObjectIndex(pForm, STATS_MSGS_TOTAL_LABEL), &rect);
		WinDrawChars(ctlTextP, StrLen(ctlTextP), rect.topLeft.x + STATS_TXT_WIDTH - strWidth, rect.topLeft.y);
		
		// Tasks
		pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, STATS_NUM_MSGS_LABEL));
		ctlTextP = (Char*) CtlGetLabel(pCtl);
		rgb.r=0; rgb.g=51; rgb.b=200; // Blue
		txtColor = WinRGBToIndex(&rgb);
		WinSetTextColor(txtColor);
		strWidth = FntLineWidth(ctlTextP, StrLen(ctlTextP));
		FrmGetObjectBounds(pForm, FrmGetObjectIndex(pForm, STATS_NUM_MSGS_LABEL), &rect);
		WinDrawChars(ctlTextP, StrLen(ctlTextP), rect.topLeft.x + STATS_TXT_WIDTH - strWidth, rect.topLeft.y);
		
		// Queued
		pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, STATS_MSGS_IN_QUEUE_LABEL));
		ctlTextP = (Char*) CtlGetLabel(pCtl);
		rgb.r=0; rgb.g=153; rgb.b=0; // green?
		txtColor = WinRGBToIndex(&rgb);
		WinSetTextColor(txtColor);
		strWidth = FntLineWidth(ctlTextP, StrLen(ctlTextP));
		FrmGetObjectBounds(pForm, FrmGetObjectIndex(pForm, STATS_MSGS_IN_QUEUE_LABEL), &rect);
		WinDrawChars(ctlTextP, StrLen(ctlTextP), rect.topLeft.x + STATS_TXT_WIDTH - strWidth, rect.topLeft.y);
		
		// Log
		pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, STATS_NUM_LOG_ENTRIES_LABEL));
		ctlTextP = (Char*) CtlGetLabel(pCtl);
		rgb.r=153; rgb.g=0; rgb.b=153; // purple?
		txtColor = WinRGBToIndex(&rgb);
		WinSetTextColor(txtColor);
		strWidth = FntLineWidth(ctlTextP, StrLen(ctlTextP));
		FrmGetObjectBounds(pForm, FrmGetObjectIndex(pForm, STATS_NUM_LOG_ENTRIES_LABEL), &rect);
		WinDrawChars(ctlTextP, StrLen(ctlTextP), rect.topLeft.x + STATS_TXT_WIDTH - strWidth, rect.topLeft.y);
		
		WinPopDrawState();	
	}
	
	return (handled);
	
} // StatsFormHandleEvent

/*
 * FormUtils.c
 */
