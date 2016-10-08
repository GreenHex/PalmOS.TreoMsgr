/*
 * FormLog.c
 */

#include "FormLog.h"

// Prototypes
static void 				SetLogFormSecurityButton(FormType* pForm, UInt16 buttonID) EXTRA_SECTION_ONE;
static Boolean 				HandleTableKeyPress(FormType* pForm, UInt16 keyCode) EXTRA_SECTION_ONE;
static void 				SetScrollBar(FormType* pForm, Boolean isInit) EXTRA_SECTION_ONE;
static void 				InitializeLogForm(FormType* pForm) EXTRA_SECTION_ONE;

/*
 * SetLogFormSecurityButton
 */
static void SetLogFormSecurityButton(FormType* pForm, UInt16 buttonID)
{
	if (FrmGetObjectIndex(pForm, buttonID) == frmInvalidObjectId)
		return;
		
	switch (PrefGetPreference(prefShowPrivateRecords))
	{
			case showPrivateRecords:

				CtlSetLabel(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, buttonID)), "Security: Show");
			   
				break;
				
	  		case maskPrivateRecords:
	  		
	  			CtlSetLabel(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, buttonID)), "Security: Mask");
	  			
	  			break;
	  			
	  		case hidePrivateRecords:
	  		
	  			CtlSetLabel(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, buttonID)), "Security: Hide");
	  			
	  			break; 
	  			
	  		default:
	  			
	  			break;
	}
	
} // SetLogFormSecurityButton

/*
 * HandleTableKeyPress
 */
static Boolean HandleTableKeyPress(FormType* pForm, UInt16 keyCode)
{
	Boolean				retVal = false;
	ScrollBarType* 		pSclBar = (ScrollBarType*) FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, LOG_SCROLLBAR));
	Int16				value = 0;
   	Int16 				min = 0;
   	Int16 				max = 0;
  	Int16 				pageSize = 0;
					
	if (keyCode == vchrRockerUp)
	{
		SclGetScrollBar(pSclBar, &value, &min, &max, &pageSize);	
		
		if (value)
		{
			--topRow;
			--value;
			SclSetScrollBar(pSclBar, value, min, max, pageSize);
			
			TblMarkTableInvalid(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, LOG_TABLE)));
			TblRedrawTable(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, LOG_TABLE)));
			
			retVal = true;
		}
	}
	else if (keyCode == vchrRockerDown)
	{
		SclGetScrollBar(pSclBar, &value, &min, &max, &pageSize);
		
		if (value < max)
		{
			++topRow;
			++value;
			SclSetScrollBar(pSclBar, value, min, max, pageSize);
			
			TblMarkTableInvalid(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, LOG_TABLE)));
			TblRedrawTable(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, LOG_TABLE)));
			
			retVal = true;
		}
	}
	
	return (retVal);

} // HandleTableKeyPress

/*
 * SetScrollBar
 */
static void SetScrollBar(FormType* pForm, Boolean isInit)
{
	TableType*			pTable = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, LOG_TABLE));
	ScrollBarType*		pSclBar = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, LOG_SCROLLBAR));
	UInt16				numItems = 0;
	UInt16				pageSize = 0;
	
	FrmSetActiveForm(pForm);
	
	if (currentForm == dispLog)
	{
		numItems = GetNumLogRecords(GetReadOnlySecretMode());
		if (numItems)
		{
			CtlSetEnabled(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, LOG_CLEAR_BUTTON)), true);
		}
		else
		{
			CtlSetEnabled(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, LOG_CLEAR_BUTTON)), false);
		}	
	}
	else // dispTasks
	{
		numItems = GetNumTaskRecords(GetReadOnlySecretMode());
	}

	if (isInit)	
	{
		TableInit(pTable, currentForm);
	}
	
	pageSize = TblGetNumberOfRows(pTable);	
	
	if (numItems > pageSize)
	{
		SclSetScrollBar(pSclBar, numItems - pageSize, 0, numItems - pageSize, pageSize);
		if (isInit)
		{
			topRow = numItems - pageSize;
		}
	}
	else
	{
		SclSetScrollBar(pSclBar, 0, 0, 0, 0);
		if (isInit)
		{
			topRow = 0;
		}
	}

	TblMarkTableInvalid(pTable);
	TblRedrawTable(pTable);

	if (numItems)
	{
		FrmHideObject(pForm, FrmGetObjectIndex(pForm, LOG_NOLOG_LABEL));
	}
	else
	{
		FrmShowObject(pForm, FrmGetObjectIndex(pForm, LOG_NOLOG_LABEL));
	}		
	
} // SetScrollBar

/*
 * InitializeLogForm
 */
static void InitializeLogForm(FormType* pForm)
{	
	SetScrollBar(pForm, true);		
	SetLogFormSecurityButton(pForm, LOG_SECURITY_BUTTON);
	
} // InitializeLogForm

/*
 * DisplayLogForm
 */
Boolean DisplayLogForm(void)
{
	FrmPopupForm(LOG_FORM);
							
	return (true);
	
} // DisplayLogForm

/*
 * LogFormHandleEvent
 */
Boolean LogFormHandleEvent(EventType* pEvent)
{
	static Err						error = errNone;
	
	Boolean 						handled = false;
	FormType* 						pForm = FrmGetActiveForm();
	
	static RectangleType 			rect;
	static RectangleType			rectObscured; /* display box */
	static RectangleType			rectRow; /* highlighted row */
	static WinHandle				winH;
	static Boolean					bWinSaved = false;
	static RectangleType			rEdit; /* edit menu */
	static RectangleType			rSend; /* send menu */
	static RectangleType			rectMenu;
	static WinHandle				winMenuH;
	static Boolean					bWinMenuSaved = false;
	
	static Boolean					bEditInverted = false;
	static Boolean					bSendInverted = false;
	static Boolean 					inEditRect = false;
	static Boolean					inSendRect = false;
	static UInt16					currentRow = dmMaxRecordIndex;
	static log_t					log;

	UInt16							totalNumRecords = 0;
	UInt16							privateNumRecords = 0;
	Char							sNumRecords[8];
	Char							sTitle[32];
	
	switch (pEvent->eType)
	{
		case frmOpenEvent:
		
			if (currentForm == dispLog)
			{			
				totalNumRecords = GetNumLogRecords(dmModeReadOnly | dmModeShowSecret);
				privateNumRecords = totalNumRecords - GetNumLogRecords(dmModeReadOnly);
				
				StrCopy(sTitle, "TreoMsgr Log [");
				StrPrintF(sNumRecords, "%u", privateNumRecords);
				StrCat(sTitle, sNumRecords);
				StrCat(sTitle, "p, ");
				StrPrintF(sNumRecords, "%u", totalNumRecords);
				StrCat(sTitle, sNumRecords);
				StrCat(sTitle, "t]");
				
				FrmSetTitle(pForm, sTitle);
				
				FrmShowObject(pForm, FrmGetObjectIndex(pForm, LOG_CLEAR_BUTTON));
				FrmHideObject(pForm, FrmGetObjectIndex(pForm, LOG_NEW_TASK_BUTTON));							
			}
			else
			{
				totalNumRecords = GetNumTaskRecords(dmModeReadOnly | dmModeShowSecret);
				privateNumRecords = totalNumRecords - GetNumTaskRecords(dmModeReadOnly);

				StrCopy(sTitle, "Messaging Queue [");
				StrPrintF(sNumRecords, "%u", privateNumRecords);
				StrCat(sTitle, sNumRecords);
				StrCat(sTitle, "p, ");
				StrPrintF(sNumRecords, "%u", totalNumRecords);
				StrCat(sTitle, sNumRecords);
				StrCat(sTitle, "t]");
				
				FrmSetTitle(pForm, sTitle);
				FrmHideObject(pForm, FrmGetObjectIndex(pForm, LOG_CLEAR_BUTTON));
				FrmShowObject(pForm, FrmGetObjectIndex(pForm, LOG_NEW_TASK_BUTTON));							
			}
			
			InitializeLogForm(pForm);
			
			FrmDrawForm(pForm);							
		
			handled = true;
			
			break;
/*
		case frmCloseEvent:
			{
				SclSetScrollBar(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, LOG_SCROLLBAR)), 0, 0, 0, 0);
				FrmHideObject(pForm, FrmGetObjectIndex(pForm, LOG_SCROLLBAR));
			}
			break;
*/							
		case keyDownEvent:
		
			handled = HandleTableKeyPress(pForm, pEvent->data.keyDown.keyCode);
			
			break;
			
		case sclRepeatEvent:
// 		case sclExitEvent: 
		
			if (pEvent->data.sclRepeat.newValue != topRow)
			{
				topRow = pEvent->data.sclExit.newValue;
				TblMarkTableInvalid(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, LOG_TABLE)));
				TblRedrawTable(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, LOG_TABLE)));
			}

			break;
		
		case penMoveEvent:
			{
				Int16					x = pEvent->screenX;
				Int16					y = pEvent->screenY;
				
				if (bWinMenuSaved)
				{	
					if (currentForm == dispTasks) /* Only for Tasks */
					{
						inEditRect = (x > rEdit.topLeft.x)
										&& (y > rEdit.topLeft.y)
										&& (x < (rEdit.topLeft.x + rEdit.extent.x))
										&& (y < (rEdit.topLeft.y + rEdit.extent.y));													
									
						if ((!bEditInverted) && (inEditRect))
						{
							WinInvertRectangle(&rEdit, 2);
							bEditInverted = true;		
						}
						else if ((bEditInverted) && (!inEditRect))
						{
							WinInvertRectangle(&rEdit, 2);
							bEditInverted = false;						
						}
					}
					
					inSendRect = (x > rSend.topLeft.x)
									&& (y > rSend.topLeft.y)
									&& (x < (rSend.topLeft.x + rSend.extent.x))
									&& (y < (rSend.topLeft.y + rSend.extent.y));
					
					if ((!bSendInverted) && (inSendRect))
					{
						WinInvertRectangle(&rSend, 2);
						bSendInverted = true;		
					}
					else if ((bSendInverted) && (!inSendRect))
					{
						WinInvertRectangle(&rSend, 2);
						bSendInverted = false;						
					}
				}
			}
			
			break;
			
		case penUpEvent:
		
			if (bWinSaved)
			{	
				WinInvertRectangle(&rectRow, 2); // restore first !!!
				WinRestoreBits(winH, rectObscured.topLeft.x, rectObscured.topLeft.y);
				bWinSaved = false;
			}
			
			if (bWinMenuSaved)
			{	
				WinRestoreBits(winMenuH, rectMenu.topLeft.x, rectMenu.topLeft.y);
				bWinMenuSaved = false;
			}
			
			if (inEditRect)
			{
				FormType*			pTaskForm = NULL;
				
				inEditRect = false;
				
				bTaskEditing = true;
				
				pTaskForm = FrmInitForm(TASK_FORM);
			
				FrmSetActiveForm(pTaskForm);
				FrmSetEventHandler(pTaskForm, TaskFormHandleEvent);
				FrmSetMenu(pTaskForm, MENU_TASK_EDIT);
				FrmSetTitle(pTaskForm, "Edit Message");

				{ // init stuff...
					log_t					log;
					Boolean					isSecret = false;
					ControlType*    		pCtl;
					FieldType*				pField = NULL;
					MemHandle				hText;
					char* 					pText;
					
					// Yucky, global stuff!!!
					gHasRepeatInfo = false;
					MemSet(&gToDoRepeatInfo, sizeof(ToDoRepeatInfoType), 0);
					gRepeatAfterCompleted = false;
					
					SplitTaskEntry(currentRow + topRow, &log, &isSecret);
					
					pCtl = FrmGetObjectPtr(pTaskForm, FrmGetObjectIndex(pTaskForm, TASK_DATE_TRIGGER));
					ctlDateLabel = (char *) CtlGetLabel(pCtl);
					StrCopy(ctlDateLabel, log.Date);	
					CtlSetLabel(pCtl, ctlDateLabel);
											
					pCtl = FrmGetObjectPtr(pTaskForm, FrmGetObjectIndex(pTaskForm, TASK_TIME_TRIGGER));						
					ctlTimeLabel = (char *) CtlGetLabel(pCtl);
					StrCopy(ctlTimeLabel, log.Time);
					CtlSetLabel(pCtl, ctlTimeLabel);
					
					hText = MemHandleNew(StrLen(log.Task) + 1);
					pText = MemHandleLock(hText);
					StrCopy(pText, log.Task);
					pField = FrmGetObjectPtr(pTaskForm, FrmGetObjectIndex(pTaskForm, TASK_TASK_DESCRIPTION_FIELD));
					FldSetTextHandle(pField, NULL);
					FldSetTextHandle(pField, hText);
					MemHandleUnlock(hText);
					FldDrawField(pField);

					hText = MemHandleNew(StrLen(log.Numbers) + 1);
					pText = MemHandleLock(hText);
					StrCopy(pText, log.Numbers);
					pField = FrmGetObjectPtr(pTaskForm, FrmGetObjectIndex(pTaskForm, TASK_PHONE_NUM_FIELD));
					FldSetTextHandle(pField, NULL);
					FldSetTextHandle(pField, hText);
					MemHandleUnlock(hText);
					FldDrawField(pField);

					hText = MemHandleNew(StrLen(log.Message) + 1);
					pText = MemHandleLock(hText);
					StrCopy(pText, log.Message);
					pField = FrmGetObjectPtr(pTaskForm, FrmGetObjectIndex(pTaskForm, TASK_MESSAGE_FIELD));
					FldSetTextHandle(pField, NULL);
					FldSetTextHandle(pField, hText);
					MemHandleUnlock(hText);
					FldDrawField(pField);
					
					pCtl = FrmGetObjectPtr(pTaskForm, FrmGetObjectIndex(pTaskForm, TASK_PRIVATE_CHECKBOX));						
					CtlSetValue(pCtl, isSecret);												
				}
				
				FrmDoDialog(pTaskForm);
				 
				// FrmEraseForm(pTaskForm);
				FrmDeleteForm(pTaskForm);
				
				pTaskForm = NULL;
				
				bTaskEditing = false;
				
				FrmSetActiveForm(pForm);
				SetScrollBar(pForm, false);
			}
			else if (inSendRect)
			{
				AppUSPrefs_t*		usPrefsP = NULL;

				inSendRect = false;

				usPrefsP = MemPtrNew(sizeof(AppUSPrefs_t));
				
				if (!usPrefsP)
				{
					handled = true;
					break;
				}

				MemSet(usPrefsP, sizeof(AppUSPrefs_t), 0);
				
				if (currentForm == dispLog)
				{
					SplitLogEntry(currentRow + topRow, &log, NULL);
				}
				else
				{
					SplitTaskEntry(currentRow + topRow, &log, NULL);					
				}
				
				StrCopy(usPrefsP->Task, log.Task);
				StrCopy(usPrefsP->msg.Numbers, log.Numbers);
				StrNCopy(usPrefsP->msg.DisplayName, log.Numbers, (LEN_CallerID - 1));
				usPrefsP->msg.DisplayName[LEN_CallerID - 1] = chrNull;
				StrCopy(usPrefsP->msg.Message, log.Message);
				usPrefsP->time = TimGetSeconds();
				
				if ((!StrLen(usPrefsP->msg.Numbers)) || (!StrLen(usPrefsP->msg.Message)))
				{
					FrmCustomAlert(WarningOKAlert, "Invalid Entry.", "", "");
				}
				else if (!FrmCustomAlert(ConfirmationOKCancelAlert, "Send Message To: \n", usPrefsP->msg.DisplayName, ""))
				{
					ProgressPtr			pProgress = NULL;
									
					pProgress = PrgStartDialog(APP_NAME, SendMsgCallback, NULL);					
					SplitGroupAndSendMessage(usPrefsP, pProgress, 0);
					PrgStopDialog(pProgress, false);					
				}				
				
				if (usPrefsP)
					MemPtrFree(usPrefsP);
			}

			handled = true;
			
			break;
			
		case tblEnterEvent:
		
			if (pEvent->data.tblEnter.tableID == LOG_TABLE)
			{				
				Boolean 	retVal = false;
				
				bEditInverted = bSendInverted = false;
				inEditRect = inSendRect = false;
				
				currentRow = pEvent->data.tblEnter.row; /* this is used for "edit" and "delete" */
				gCurrentIdx = TblGetRowData(pEvent->data.tblEnter.pTable, currentRow);
				
				if (TblRowMasked(pEvent->data.tblEnter.pTable, currentRow))
				{
					if (SecVerifyPW(showPrivateRecords))
					{
						SetScrollBar(pForm, true);
						SetLogFormSecurityButton(pForm, LOG_SECURITY_BUTTON);
					}
					handled = true;
					break;
				}
				
				MemSet(&log, sizeof(log_t), 0);
				
				if (currentForm == dispLog)
				{
					retVal = SplitLogEntry(pEvent->data.tblEnter.row + topRow, &log, NULL);	
				}
				else // dispTasks
				{
					Char		nameStr[LEN_CallerID];
					
					retVal = SplitTaskEntry(pEvent->data.tblEnter.row + topRow, &log, NULL);
					
					StrNCopy(nameStr, log.Name, (LEN_CallerID - 1)); // save
					nameStr[LEN_CallerID - 1] = chrNull;
					
					StrNCopy(log.Name, log.Numbers, (LEN_CallerID - 1));
					log.Name[LEN_CallerID - 1] = chrNull;
					
					StrNCopy(log.Numbers, nameStr, (LEN_CallerID - 1));
					log.Numbers[LEN_CallerID - 1] = chrNull;
				}
				
				if (retVal)
				{	
				    RGBColorType 			rgb;
				    IndexedColorType		txtColor;				    
					RectangleType			rT; /* table bounds */
					RectangleType			rR; /* row bounds */
						
					TblGetBounds(pEvent->data.tblEnter.pTable, &rT);
					TblGetItemBounds(pEvent->data.tblEnter.pTable, pEvent->data.tblEnter.row, colStatus, &rR);
					
					rectRow.topLeft.x = rT.topLeft.x - 1;
					rectRow.topLeft.y = rR.topLeft.y;
					rectRow.extent.x = RECT_SIZE_X + 2; // rT.extent.x + 2;
					rectRow.extent.y = rR.extent.y;
					
					rect.topLeft.x = RECT_TOP_X;
					if (rectRow.topLeft.y < 60)
					{
						rect.topLeft.y = rectRow.topLeft.y + rectRow.extent.y + 2 + 1; // pEvent->screenY - RECT_SIZE_Y - 4;
					}
					else
					{
						rect.topLeft.y = rectRow.topLeft.y - (1 + 2 + RECT_SIZE_Y); 
					}
					rect.extent.x = RECT_SIZE_X;
					rect.extent.y = RECT_SIZE_Y;
					
					WinGetFramesRectangle(dialogFrame, &rect, &rectObscured);			
					
					winH = WinSaveBits(&rectObscured, &error);
					if (error == errNone)
						bWinSaved = true;					
					
					//WinDrawRectangleFrame(simpleFrame, &rectRow);
					WinInvertRectangle(&rectRow, 2);
					
					if ((currentForm == dispLog) && (StrLen(log.Time)))
					{
						if (StrCompare(log.Error, "Message Sent") == 0) 
						{
							rgb.r=0; rgb.g=153; rgb.b=0; // green?
							txtColor = WinRGBToIndex(&rgb);
						}
						else if (StrStr(log.Error, "Message Deleted")) // deleted
						{
							rgb.r=119; rgb.g=119; rgb.b=119; // grey?
							txtColor = WinRGBToIndex(&rgb);							
						}
						else if (StrStr(log.Error, "Messages Sent")) // summary
						{
							rgb.r=0; rgb.g=51; rgb.b=255; // blue?
							txtColor = WinRGBToIndex(&rgb);							
						}
						else // fail
						{
							rgb.r=255; rgb.g=0; rgb.b=51; // red?
							txtColor = WinRGBToIndex(&rgb);		
						}
												
						rSend.topLeft.x = pEvent->screenX - (MENU_ITEM_SIZE_X / 2);
						
						if (rSend.topLeft.x < 2)
						{
							rSend.topLeft.x = 2;
						}
						
						if (rSend.topLeft.x > (DISP_WIDTH - (MENU_ITEM_SIZE_X + 2)))
						{
							rSend.topLeft.x = (DISP_WIDTH - (MENU_ITEM_SIZE_X + 2));
						}
						
						if (rectRow.topLeft.y > 60)
						{
							rSend.topLeft.y = rectRow.topLeft.y + rectRow.extent.y + 1;
						}
						else
						{
							rSend.topLeft.y = rectRow.topLeft.y - (MENU_ITEM_SIZE_Y + 1);
						}
						
						rSend.extent.x = MENU_ITEM_SIZE_X;
						rSend.extent.y = MENU_ITEM_SIZE_Y;
						
						rectMenu = rSend;
												
						winMenuH = WinSaveBits(&rectMenu, &error);
						if (error == errNone)
							bWinMenuSaved = true;
						
						WinEraseRectangle(&rectMenu, 2);
						DrawEditMenu(NULL, &rSend);
					}
					else // dispTasks
					{
						rgb.r=0; rgb.g=51; rgb.b=255; // blue?
						txtColor = WinRGBToIndex(&rgb);
						
						rEdit.topLeft.x = pEvent->screenX - (MENU_ITEM_SIZE_X + MENU_SPACE);
						
						if (rEdit.topLeft.x < 2)
						{
							rEdit.topLeft.x = 2;
						}
						else if (rEdit.topLeft.x > (DISP_WIDTH - ((MENU_ITEM_SIZE_X * 2) + 5)))
						{
							rEdit.topLeft.x = (DISP_WIDTH - ((MENU_ITEM_SIZE_X * 2) + 5));	
						}
						
						rSend.topLeft.x = rEdit.topLeft.x + MENU_ITEM_SIZE_X + 2 + 1;
						
						if (rectRow.topLeft.y > 60)
						{
							rEdit.topLeft.y = rSend.topLeft.y = rectRow.topLeft.y + rectRow.extent.y + 1;
						}
						else
						{
							rEdit.topLeft.y = rSend.topLeft.y = rectRow.topLeft.y - (MENU_ITEM_SIZE_Y + 1);
						}
						
						rEdit.extent.x = rSend.extent.x = MENU_ITEM_SIZE_X;
						rEdit.extent.y = rSend.extent.y = MENU_ITEM_SIZE_Y;
						
						rectMenu.topLeft.x = rEdit.topLeft.x;
						rectMenu.topLeft.y = rEdit.topLeft.y;
						rectMenu.extent.x = rEdit.extent.x + rSend.extent.x + 2 + 1;
						rectMenu.extent.y = MENU_ITEM_SIZE_Y;
						
						winMenuH = WinSaveBits(&rectMenu, &error);
						
						if (error == errNone)
						{
							bWinMenuSaved = true;
						}
						
						WinEraseRectangle(&rectMenu, 2);
						DrawEditMenu(&rEdit, &rSend);
					}
					
					DispPopupWindow(&log, &rect, &txtColor);
				}
				
				handled = true;
			}
	
			break;
			
		case ctlSelectEvent:
		
			switch (pEvent->data.ctlSelect.controlID)
			{
				case LOG_DONE_BUTTON:
					
					if (currentForm == dispTasks)
					{
						setupAlarm();
					}

					// SclSetScrollBar(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, LOG_SCROLLBAR)), 0, 0, 0, 0);
					// FrmHideObject(pForm, FrmGetObjectIndex(pForm, LOG_SCROLLBAR));

					FrmReturnToForm(0);
					
					handled = true;
					
					break;
					
				case LOG_CLEAR_BUTTON:
				
					if (!FrmCustomAlert(ConfirmationOKCancelAlert, "Clear all log entries?", "", ""))
					{
						DeleteLog();

						FrmReturnToForm(0);
					}

					handled = true;
					
					break;
					
				case LOG_SECURITY_BUTTON:
					{
						privateRecordViewEnum			OldRecordDispStatus = PrefGetPreference(prefShowPrivateRecords);
						
						if (SecSelectViewStatus() != OldRecordDispStatus)
						{							
							SetScrollBar(pForm, true);	
							SetLogFormSecurityButton(pForm, LOG_SECURITY_BUTTON);
						}
					}
			
					handled = true;
									
					break;
					
				case LOG_NEW_TASK_BUTTON:

					handled = DisplayNewTaskForm(pForm);
					
					SetScrollBar(pForm, true);
					
					break;
				
				default:
					break;
			}

			break;
			
		default:
			break;
	}
		
	return handled;
	
} // LogFormHandleEvent

/*
 * FormLog.c
 */