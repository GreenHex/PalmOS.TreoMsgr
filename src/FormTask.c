/*
 * FormTask.c
 */

#include "FormTask.h"

// Prototypes
static Boolean 				CreateTask(FormType* pForm) EXTRA_SECTION_ONE;
static void 				InitializeTaskForm(FormType* pForm) EXTRA_SECTION_ONE;

/*
 * CreateTask
 */
static Boolean CreateTask(FormType* pForm)
{
	Boolean 			retVal = false;
	ControlType*		pCtl = NULL;
	DmOpenRef			dbP = NULL;
	UInt16				categoryIdx = dmAllCategories;
	ToDoItemType 		item;

	FieldType*			pTaskField = NULL;
	FieldType*			pMsgField = NULL;
	FieldType*			pNumField = NULL;

	Char				tmpDescStr[LEN_Description];
	Char				tmpNoteStr[StrLen(NOTE_TOKEN_START) + StrLen(NOTE_TOKEN_END)
									+ StrLen(TEL_TOKEN_START) + StrLen(TEL_TOKEN_END)  
									+ StrLen(MSG_TOKEN_START) + StrLen(MSG_TOKEN_END) 
									+ LEN_Message + LEN_PhoneNumbers + 1];

/*
#define NOTE_TOKEN_START					"<tm>"
#define NOTE_TOKEN_END						"</tm>"
#define TEL_TOKEN_START						"<tel>"
#define TEL_TOKEN_END						"</tel>"
#define MSG_TOKEN_START						"<msg>"
#define MSG_TOKEN_END						"</msg>"
*/					

	MemSet(&item, sizeof(ToDoItemType), 0);
	
	item.dataFlags.alarm = false;
	item.dataFlags.completionDate = false;
	item.dataFlags.description = false;
	item.dataFlags.dueDate = false;
	item.dataFlags.note = false;
	item.dataFlags.repeat = false;

	item.recordFlags &= ~TODO_RECORD_FLAG_COMPLETE;
	
	if (gRepeatAfterCompleted)
	{
		item.recordFlags |= TODO_RECORD_FLAG_REPEAT_ON_COMPLETE;
	}
	else
	{
		item.recordFlags &= ~TODO_RECORD_FLAG_REPEAT_ON_COMPLETE;
	}
	
	item.priority = 3;
		
	{	
		Char*					ctlLabel = NULL;		
		Char					strMonth[3];
		Char					strDay[3];
		Char					strYear[5];
					
		pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, TASK_DATE_TRIGGER));
						
		ctlLabel = (char *) CtlGetLabel(pCtl);

		strDay[0] = ctlLabel[0]; strDay[1] = ctlLabel[1]; strDay[2] = chrNull;						
		strMonth[0] = ctlLabel[3]; strMonth[1] = ctlLabel[4]; strMonth[2] = chrNull;
		strYear[0] = ctlLabel[6]; strYear[1] = ctlLabel[7]; strYear[2] = ctlLabel[8];
		strYear[3] = ctlLabel[9]; strYear[4] = chrNull;
 
		item.dueDate.month = (Int16) StrAToI(strMonth);						
		item.dueDate.day = (Int16) StrAToI(strDay);
		item.dueDate.year = (Int16) StrAToI(strYear) + 16;
		
		item.repeatInfo.repeatStartDate = item.dueDate; 
				
		item.dataFlags.dueDate = true;
	}
				
	{
		Char*					ctlLabel = NULL;
		
		Char					strHours[3];
		Char					strMins[3];
		
		pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, TASK_TIME_TRIGGER));
		
		ctlLabel = (char *) CtlGetLabel(pCtl);
		
		strHours[0] = ctlLabel[0]; strHours[1] = ctlLabel[1]; strHours[2] = chrNull;
		strMins[0] = ctlLabel[3]; strMins[1] = ctlLabel[4]; strMins[2] = chrNull;

		item.alarmInfo.alarmTime.hours = (UInt16) StrAToI(strHours);
		item.alarmInfo.alarmTime.minutes = (UInt16) StrAToI(strMins);

		item.alarmInfo.alarmAdvanceDays = 0;
		item.dataFlags.alarm = true;						
	}

	pTaskField = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, TASK_TASK_DESCRIPTION_FIELD));			
	if (FldGetTextPtr(pTaskField) && StrLen(FldGetTextPtr(pTaskField)))
	{
		StrCopy(tmpDescStr, FldGetTextPtr(pTaskField));
		item.descriptionP = tmpDescStr;
		item.dataFlags.description = true;
	}

	pMsgField = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, TASK_MESSAGE_FIELD));
	pNumField = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, TASK_PHONE_NUM_FIELD));
					
	if (FldGetTextPtr(pMsgField)
			&& StrLen(FldGetTextPtr(pMsgField))
			&& FldGetTextPtr(pNumField)
			&& StrLen(FldGetTextPtr(pNumField)))
	{
		StrCopy(tmpNoteStr, NOTE_TOKEN_START);
		StrCat(tmpNoteStr, TEL_TOKEN_START);
		StrCat(tmpNoteStr, FldGetTextPtr(pNumField));
		StrCat(tmpNoteStr, TEL_TOKEN_END);
		StrCat(tmpNoteStr, MSG_TOKEN_START);
		StrCat(tmpNoteStr, FldGetTextPtr(pMsgField));
		StrCat(tmpNoteStr, MSG_TOKEN_END);
		StrCat(tmpNoteStr, NOTE_TOKEN_END);
		item.noteP = tmpNoteStr;
		item.dataFlags.note = true;
	}

	if (gHasRepeatInfo)
	{
		item.repeatInfo = gToDoRepeatInfo;
		if ((!gRepeatAfterCompleted) && (gToDoRepeatInfo.repeatInfo.repeatType == repeatMonthlyByDay))
		{
			item.repeatInfo.repeatInfo.repeatOn = DayOfMonth(item.dueDate.month, item.dueDate.day, item.dueDate.year + ZERO_YEAR); 
		}
		
		item.repeatInfo.repeatStartDate = item.dueDate; 

		item.dataFlags.repeat = true;
	}
	
	if ((item.dataFlags.description) && (item.dataFlags.note))
	{
		if (bTaskEditing)
		{	
			UInt16				idx = gCurrentIdx;				

			DeleteTask(idx, deleteRecord);
		}
		
		if ((!ToDoGetDatabase(&dbP, dmModeReadWrite  | dmModeShowSecret)) && (dbP))
		{
			if ((categoryIdx = CategoryFind(dbP, APP_NAME)) != dmAllCategories)
			{
				Err					error = errNone; 
				UInt16				idx = dmMaxRecordIndex;
				
				error = ToDoNewRecord(dbP, &item, categoryIdx, 0, 0, &idx);
				
				// TEMPORARY FIX!!! for illegal record length (4 bytes extra)!!!
				ToDoChangeRecord(dbP, &idx, 0, 0, toDoRecordFieldNote, item.noteP);
				
				{
					Boolean 		isSecret = false;
					UInt16			recordAttributes = 0;
					
					pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, TASK_PRIVATE_CHECKBOX));
					isSecret = (CtlGetValue(pCtl) == 1);
					
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
				}
/*
				if (bTaskEditing)
				{
					DmReleaseRecord(dbP, idx, false);
				}
*/
				if (error)
				{
					FrmCustomAlert(ErrOKAlert, "Unable to create task.", "", "");
				}
				else
				{
					AppPreferences_t		prefs;
					
					readPrefs(&prefs);
					if (prefs.bEnabled)
					{
						setupAlarm();
						
						{
							UInt32		timeNow	= TimGetSeconds();
							UInt32		numDays = DateToDays(item.dueDate); 
							UInt32		almSeconds = (UInt32) ((UInt32) numDays * 24 * 60 * 60)
									+ ((((UInt32) item.alarmInfo.alarmTime.hours * 60)
									+ ((UInt32)	item.alarmInfo.alarmTime.minutes)) * 60);
						
							if (almSeconds > timeNow)
							{ 			   
								FrmCustomAlert(InfoOKAlert, "Message queued.", "", "");
							}
							else
							{
								FrmCustomAlert(WarningOKAlert, "Past scheduled time.", "", "");
							}
						}
					}
					else
					{
						FrmCustomAlert(WarningOKAlert, "TreoMsgr is Disabled.", "", "");
					}
				}
			}
			else
			{
				FrmCustomAlert(ErrOKAlert, "", "Please create 'TreoMsgr' category in Tasks.", "");
			}
			DmCloseDatabase(dbP);
		}
		
		retVal = true;
	}
	else
	{
		FrmCustomAlert(ErrOKAlert, "", "Some required fields \nare blank.", "");		
	}
	
	return (retVal);
	
} // CreateTask

/*
 * DisplayNewTaskForm
 */
Boolean DisplayNewTaskForm(FormType* pForm)
{
	FormType*			pTaskForm = FrmInitForm(TASK_FORM);

	// Yucky, global stuff...
	gHasRepeatInfo = false;
	MemSet(&gToDoRepeatInfo, sizeof(ToDoRepeatInfoType), 0);
	gRepeatAfterCompleted = false;
					
	FrmSetActiveForm(pTaskForm);
	FrmSetEventHandler(pTaskForm, TaskFormHandleEvent);
	InitializeTaskForm(pTaskForm);
	
	FrmDoDialog(pTaskForm);
				 
	// FrmEraseForm(pTaskForm);
	FrmDeleteForm(pTaskForm);
	
	pTaskForm = NULL;
	
	FrmSetActiveForm(pForm);
	
	return (true);
	
} // DisplayNewTaskForm

/*
 * InitializeTaskForm
 */
static void InitializeTaskForm(FormType* pForm)
{	
	ControlType*    pCtl;	
	DateTimeType	dt;
	
	TimSecondsToDateTime(TimGetSeconds(), &dt);
	
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, TASK_DATE_TRIGGER));
	ctlDateLabel = (char *) CtlGetLabel(pCtl);	
	StrPrintF(ctlDateLabel, "%02d/%02d/%02d", dt.day, dt.month, dt.year);
	CtlSetLabel(pCtl, ctlDateLabel);
	
	gDueDate.month = dt.month;
	gDueDate.day = dt.day;
	gDueDate.year = dt.year - ZERO_YEAR;
								
	pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, TASK_TIME_TRIGGER));						
	ctlTimeLabel = (char *) CtlGetLabel(pCtl);
	StrPrintF(ctlTimeLabel, "%02d:%02d", dt.hour, dt.minute);
	CtlSetLabel(pCtl, ctlTimeLabel);

	FrmHideObject(pForm, FrmGetObjectIndex(pForm, TASK_DELETE_BUTTON));						

	FrmSetMenu(pForm, MENU_TASK_EDIT);
		
} // InitializeTaskForm

/*
 * TaskFormHandleEvent
 */
Boolean TaskFormHandleEvent(EventType* pEvent)
{
	Boolean 				handled = false;
	FormType* 				pForm = FrmGetActiveForm();
	ControlType*    		pCtl = NULL;
	ListType*				pList = NULL;
	MemHandle				hText = NULL;
	Char* 					pText = NULL;
	FieldType*				pField = NULL;
	
	switch (pEvent->eType)
	{
		case frmLoadEvent:
			
			InitializeTaskForm(pForm);

			FrmDrawForm(pForm);
			
			handled = true;
			
			break;
			
		case ctlSelectEvent:
			
			switch (pEvent->data.ctlSelect.controlID)
			{
				case TASK_GROUP_LOOKUP_BUTTON:
					{
						pList = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, TASK_GROUP_LIST));
	
						if (LoadGroupsList(pList, gGroupsList, &gGroupsH))
						{
							Int16		selIdx = -1;
							Char*		listStr = NULL;
							
							LstSetHeight(pList, (LstGetNumberOfItems(pList) > 7) ? 7 : LstGetNumberOfItems(pList));
							LstSetSelection(pList, noListSelection);
							selIdx = LstPopupList(pList);
							
							if (selIdx > -1)
							{
								listStr = LstGetSelectionText(pList, selIdx);
								
								{
									FieldType* 		pNameField = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, TASK_TASK_DESCRIPTION_FIELD));			
								
									if (!(FldGetTextPtr(pNameField) && StrLen(FldGetTextPtr(pNameField))))
									{
										hText = MemHandleNew(StrLen(listStr) + 1);
										pText = MemHandleLock(hText);
										StrCopy(pText, listStr);
										FldSetTextHandle(pNameField, NULL);
										FldSetTextHandle(pNameField, hText);
										MemHandleUnlock(hText);
										FldDrawField(pNameField);
									}
								}
				
								hText = MemHandleNew(StrLen(listStr) + 1);
								pText = MemHandleLock(hText);
								StrCopy(pText, listStr);
								pField = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, TASK_PHONE_NUM_FIELD));
								FldSetTextHandle(pField, NULL);
								FldSetTextHandle(pField, hText);
								MemHandleUnlock(hText);
								FldDrawField(pField);
							}
						}
						else
						{
							FrmCustomAlert(InfoOKAlert, "No Groups Defined.", "", "");
						}
					}
				
					handled = true;
					
					break;
					
				case TASK_LOOKUP_BUTTON:
					{			
						FieldType*				pField = NULL;
						AddrLookupParamsType 	AddrParams;
						
						pField = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, TASK_PHONE_NUM_FIELD));
						
						if (pField)
						{
							AddrParams.title = "Lookup";
							AddrParams.pasteButtonText = NULL;
							AddrParams.field1 = addrLookupSortField;
							AddrParams.field2 = addrLookupAnyPhones;
							AddrParams.field2Optional = false;
							AddrParams.userShouldInteract = true;  
							AddrParams.formatStringP = "^smsphone, "; // "^listphone, ";
							PhoneNumberLookupCustom(pField, &AddrParams, true);
						}
						
						handled = true;	
					}
					break;
				
				case TASK_DATE_TRIGGER:
					{	
						Int16					month = 0;
						Int16					day = 0;
						Int16					year = 0;
						
						Char					strMonth[3];
						Char					strDay[3];
						Char					strYear[5];
					
						pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, pEvent->data.ctlSelect.controlID));
						
						ctlDateLabel = (char *) CtlGetLabel(pCtl);

						strDay[0] = ctlDateLabel[0]; strDay[1] = ctlDateLabel[1]; strDay[2] = chrNull;						
						strMonth[0] = ctlDateLabel[3]; strMonth[1] = ctlDateLabel[4]; strMonth[2] = chrNull;
						strYear[0] = ctlDateLabel[6]; strYear[1] = ctlDateLabel[7]; strYear[2] = ctlDateLabel[8];
						strYear[3] = ctlDateLabel[9]; strYear[4] = chrNull;

						month = (Int16) StrAToI(strMonth);						
						day = (Int16) StrAToI(strDay);
						year = (Int16) StrAToI(strYear);
						
						// month = 11;
						// day = 30;
						// year = 2007;
						
						if (SelectDay(selectDayByDay, &month, &day, &year, "Select Date"))
						{
							StrPrintF(ctlDateLabel, "%02d/%02d/%02d", day, month, year);
							CtlSetLabel(pCtl, ctlDateLabel);	
							
							gDueDate.month = month;
							gDueDate.day = day;
							gDueDate.year = year - ZERO_YEAR;	
						}
					
						handled = true;	
					}
					break;
					
				case TASK_TIME_TRIGGER:
					{
						UInt16					TimeHours = 0;
						UInt16					TimeMins = 0;
						
						Char					strHours[3];
						Char					strMins[3];
						
						pCtl = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, pEvent->data.ctlSelect.controlID));
						
						ctlTimeLabel = (char *) CtlGetLabel(pCtl);
						
						strHours[0] = ctlTimeLabel[0]; strHours[1] = ctlTimeLabel[1]; strHours[2] = chrNull;
						strMins[0] = ctlTimeLabel[3]; strMins[1] = ctlTimeLabel[4]; strMins[2] = chrNull;

						TimeHours = (UInt16) StrAToI(strHours);
						TimeMins = (UInt16) StrAToI(strMins);
	
						if (SelectOneTime(&TimeHours, &TimeMins, "Select Time"))
						{	
							StrPrintF(ctlTimeLabel, "%02d:%02d", TimeHours, TimeMins);
							CtlSetLabel(pCtl, ctlTimeLabel);
						}
					
						handled = true;
					}
					break;
					
				case TASK_REPEAT_BUTTON:
					{						
						handled = DisplayRepeatForm(pForm);
					}			
					break;

				case TASK_CREATE_BUTTON:
					
					if (CreateTask(pForm))
					{														
						FrmReturnToForm(0);
					}
					
					handled = true;
							
					break;
					
				case TASK_CANCEL_BUTTON:
				
					FrmReturnToForm(0);
				
					handled = true;
						
					break;
				
				case TASK_DELETE_BUTTON:

					if (FrmCustomAlert(ConfirmationOKCancelAlert, "Delete scheduled message?", "", "") == 0)
					{
						DeleteTask(gCurrentIdx, deleteRecord);
	
						gCurrentIdx = dmMaxRecordIndex;
	
						if (topRow)
							--topRow;
						
						FrmReturnToForm(0);	
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
	
	return handled;
	
} // TaskFormHandleEvent

/*
 * FormTask.c
 */