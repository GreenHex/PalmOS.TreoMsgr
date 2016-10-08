/*
 * FormRepeat.c
 */
 
#include "FormRepeat.h"

// Prototypes
static void 			LoadRepeatInfo(FormType* pForm) EXTRA_SECTION_TWO;
static void 			SaveRepeatInfo(FormType* pForm) EXTRA_SECTION_TWO;
static void 			ShowSomeControls(FormType* pForm) EXTRA_SECTION_TWO;
static void 			ShowWeekControls(FormType* pForm) EXTRA_SECTION_TWO;
static void 			ShowMonthControls(FormType* pForm) EXTRA_SECTION_TWO;
static void 			HideRepeatControls(FormType* pForm) EXTRA_SECTION_TWO;
static void 			InitializeRepeatForm(FormType* pForm) EXTRA_SECTION_TWO;
static Boolean 			RepeatFormHandleEvent(EventType* pEvent) EXTRA_SECTION_TWO;

/*
 * LoadRepeatInfo
 */
static void LoadRepeatInfo(FormType* pForm)
{					
	FieldType*					pField = NULL;
	MemHandle					hText = NULL;
	Char*						pText = NULL;
	Char*						ctlDateLabel = NULL;
		
	HideRepeatControls(pForm);
	
	if (gHasRepeatInfo)
	{
		UInt8						repeatFrequency = 0;
		Char						freqStr[maxStrIToALen];
		DateType					repeatEndDate;
		
		UInt8						dow = 0;
		
		ShowSomeControls(pForm);
		
		// Set Frequency
		repeatFrequency = gToDoRepeatInfo.repeatInfo.repeatFrequency;
		repeatFrequency = (repeatFrequency) ? repeatFrequency : 1;
		StrPrintF(freqStr, "%u", repeatFrequency);
		
		pField = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_INTERVAL_FIELD));
		hText = MemHandleNew(maxStrIToALen);
		pText = MemHandleLock(hText);
		StrCopy(pText, freqStr);
		FldSetTextHandle(pField, NULL);
		FldSetTextHandle(pField, hText);
		MemHandleUnlock(hText);
		
		repeatEndDate = gToDoRepeatInfo.repeatInfo.repeatEndDate;
		
		// Set Repeat Type
		if (gRepeatAfterCompleted)
		{
			CtlSetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_FIXED_SCHEDULE)), 0);
			CtlSetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_AFTER_COMPLETION)), 1);
		}
		else
		{
			CtlSetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_FIXED_SCHEDULE)), 1);
			CtlSetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_AFTER_COMPLETION)), 0);
		}
		
		// Set End Date...
		if (DateToInt(repeatEndDate) == -1)
		{
			LstSetSelection(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_END_DATE_LIST)), 0);
	
			ctlDateLabel = (char *) CtlGetLabel(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_END_DATE_POPUP)));
			StrCopy(ctlDateLabel, LABEL_NO_DUE_DATE);					
			
			CtlSetLabel(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_END_DATE_POPUP)), ctlDateLabel);
		}
		else
		{
			LstSetSelection(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_END_DATE_LIST)), 1);
	
			ctlDateLabel = (char *) CtlGetLabel(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_END_DATE_POPUP)));
			StrPrintF(ctlDateLabel, "%02d/%02d/%02d", repeatEndDate.day, repeatEndDate.month, repeatEndDate.year + ZERO_YEAR);					
			
			CtlSetLabel(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_END_DATE_POPUP)), ctlDateLabel);
		}
					
		// Set Repeat Type
		switch (gToDoRepeatInfo.repeatInfo.repeatType)
		{
		    case repeatDaily:
		    
		    	CtlSetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_DAY_PB)), 1);
		    	CtlSetLabel(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_INTERVAL_TYPE_LABEL)), "Day(s)");
		    	
		    	break;
		    	
		    case repeatWeekly:
		    
		    	CtlSetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_WEEK_PB)), 1);	
		    	CtlSetLabel(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_INTERVAL_TYPE_LABEL)), "Week(s)");
					
		    	ShowWeekControls(pForm);
		    	
		    	for (dow = 0 ; dow < 7 ; dow++)
		    	{
		    		if (RepeatOnDOW(&(gToDoRepeatInfo.repeatInfo), dow))
		    		{
		    			CtlSetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_SUN + (dow * 10))), 1);	
		    		}
		    		else
		    		{
		    			CtlSetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_SUN + (dow * 10))), 0);	
		    		}
		    	}
		    	break;
		    	
		    case repeatMonthlyByDay:
		    	
				CtlSetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_MONTH_PB)), 1);
				CtlSetLabel(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_INTERVAL_TYPE_LABEL)), "Month(s)");
				
				CtlSetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_BY_DAY_PB)), 1);
				
				ShowMonthControls(pForm);
				
		    	break;
		    	
		    case repeatMonthlyByDate: // "Every Month" & "Repeat by: Date"

		    	CtlSetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_MONTH_PB)), 1);
				CtlSetLabel(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_INTERVAL_TYPE_LABEL)), "Month(s)");
				
				CtlSetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_BY_DATE_PB)), 1);
				
				ShowMonthControls(pForm);
				
		    	break;
		    	
		    case repeatYearly:
		    
		    	CtlSetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_YEAR_PB)), 1);
				CtlSetLabel(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_INTERVAL_TYPE_LABEL)), "Year(s)");
				
	    		break;
	    		
			default: // includes "repeatNone"
			
				CtlSetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_NONE_PB)), 1);
				
				HideRepeatControls(pForm);
				
				break;
		}
	}
	else
	{
		CtlSetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_NONE_PB)), 1);
		
		// Schedule
		CtlSetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_FIXED_SCHEDULE)), 1);
		CtlSetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_AFTER_COMPLETION)), 0);
			
		// Frequency
		pField = FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_INTERVAL_FIELD));
		hText = MemHandleNew(maxStrIToALen);
		pText = MemHandleLock(hText);
		StrCopy(pText, "1");
		FldSetTextHandle(pField, NULL);
		FldSetTextHandle(pField, hText);
		MemHandleUnlock(hText);
		
		// End Date
		LstSetSelection(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_END_DATE_LIST)), 0);
		ctlDateLabel = (char *) CtlGetLabel(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_END_DATE_POPUP)));
		StrCopy(ctlDateLabel, LABEL_NO_DUE_DATE);					
		CtlSetLabel(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_END_DATE_POPUP)), ctlDateLabel);
		
		// Repeat Type
		CtlSetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_BY_DATE_PB)), 1);
		
		HideRepeatControls(pForm);
	}
	
	return;
	
} // LoadRepeatInfo

/*
 * SaveRepeatInfo
 */
static void SaveRepeatInfo(FormType* pForm)
{
	RepeatType					repeatType = repeatNone;
	UInt8						repeatFrequency = 0;
	Boolean						hasEndDate = false;
	DateType					EndDate;
	UInt8       				repeatOn = 0;           /**< monthlyByDay and repeatWeekly only */
    UInt8       				repeatStartOfWeek = 0;
		
	if (CtlGetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_NONE_PB))) == 1)
	{
		gHasRepeatInfo = false;
	}
	else
	{
		Char*						freqStr = NULL;
		// DateType					repeatEndDate;
		
		gHasRepeatInfo = true;
		gRepeatAfterCompleted = (CtlGetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_AFTER_COMPLETION))) == 1);
		
		freqStr = FldGetTextPtr(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_INTERVAL_FIELD)));
		if (freqStr)
		{
			repeatFrequency = (UInt8) StrAToI(freqStr);
			
			repeatFrequency = (repeatFrequency) ? repeatFrequency : 1;
			
			repeatFrequency = (repeatFrequency > 99) ? 99 : repeatFrequency;
		}
		else
		{
			repeatFrequency = 1;
		}
		
		 // End Date
		hasEndDate = (LstGetSelection(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_END_DATE_LIST))) == 1);
		DateToInt(EndDate) = toDoNoDueDate;

		if (hasEndDate)
		{
			Char*					ctlLabel = NULL;		
			Char					strMonth[3];
			Char					strDay[3];
			Char					strYear[5];
		
			ctlLabel = (char *) CtlGetLabel(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_END_DATE_POPUP)));

			strDay[0] = ctlLabel[0]; strDay[1] = ctlLabel[1]; strDay[2] = chrNull;						
			strMonth[0] = ctlLabel[3]; strMonth[1] = ctlLabel[4]; strMonth[2] = chrNull;
			strYear[0] = ctlLabel[6]; strYear[1] = ctlLabel[7]; strYear[2] = ctlLabel[8];
			strYear[3] = ctlLabel[9]; strYear[4] = chrNull;
	 
			EndDate.month = (Int16) StrAToI(strMonth);						
			EndDate.day = (Int16) StrAToI(strDay);
			EndDate.year = (Int16) StrAToI(strYear) + 16;							
		}
				
		// The reset of the stuff
		if (CtlGetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_DAY_PB))) == 1)
		{ // DAY
			
			repeatType = repeatDaily;
		}
		else if (CtlGetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_WEEK_PB))) == 1)
		{ // WEEK
			
			repeatType = repeatWeekly;
			
			if (!gRepeatAfterCompleted)
			{
				UInt8		dow = 0;
				
				for ( dow = 0 ; dow < 7 ; dow++ )
				{
					if (CtlGetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_SUN + (dow * 10)))) == 1)
					{
						repeatOn |= 1 << dow;
					}
				}
			}
		}
		else if (CtlGetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_MONTH_PB))) == 1)
		{ // MONTH
			
			if (gRepeatAfterCompleted)
			{
				repeatType = repeatMonthlyByDate;
			}
			else
			{
				if (CtlGetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_BY_DAY_PB))) == 1)
				{
					repeatType = repeatMonthlyByDay;
					repeatOn = DayOfMonth(gDueDate.month, gDueDate.day, gDueDate.year + ZERO_YEAR);
				}
				else
				{
					repeatType = repeatMonthlyByDate;
				}
			}
		}
		else if (CtlGetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_YEAR_PB))) == 1)
		{ // YEAR
			
			repeatType = repeatYearly;
		}
		else
		{ // something wrong
			
			repeatType = repeatNone;
			gHasRepeatInfo = false;
			// nothing	
		}
	}
	
	gToDoRepeatInfo.repeatInfo.repeatType = repeatType;
	gToDoRepeatInfo.repeatInfo.repeatFrequency = repeatFrequency;
	gToDoRepeatInfo.repeatInfo.repeatEndDate = EndDate;
	gToDoRepeatInfo.repeatInfo.repeatStartOfWeek = repeatStartOfWeek;
	gToDoRepeatInfo.repeatInfo.repeatOn = repeatOn;
	
	return;
	
} // SaveRepeatInfo

/*
 * ShowSomeControls
 */
static void ShowSomeControls(FormType* pForm)
{
	FrmShowObject(pForm, FrmGetObjectIndex(pForm, REPEAT_FIXED_SCHEDULE));
	FrmShowObject(pForm, FrmGetObjectIndex(pForm, REPEAT_AFTER_COMPLETION));
	FrmShowObject(pForm, FrmGetObjectIndex(pForm, REPEAT_INTERVAL_LABEL));
	FrmShowObject(pForm, FrmGetObjectIndex(pForm, REPEAT_INTERVAL_FIELD));
	FrmShowObject(pForm, FrmGetObjectIndex(pForm, REPEAT_INTERVAL_TYPE_LABEL));
	FrmShowObject(pForm, FrmGetObjectIndex(pForm, REPEAT_END_DATE_LABEL));
	FrmShowObject(pForm, FrmGetObjectIndex(pForm, REPEAT_END_DATE_POPUP));

	return;
	
} // ShowSomeControls

/*
 * ShowDayControls
 */
static void ShowWeekControls(FormType* pForm)
{
	if (gRepeatAfterCompleted)
	{
		FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_LABEL));
		FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_SUN));
		FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_MON));
		FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_TUE));
		FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_WED));
		FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_THU));
		FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_FRI));
		FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_SAT));
	}
	else
	{
		FrmShowObject(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_LABEL));
		FrmShowObject(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_SUN));
		FrmShowObject(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_MON));
		FrmShowObject(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_TUE));
		FrmShowObject(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_WED));
		FrmShowObject(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_THU));
		FrmShowObject(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_FRI));
		FrmShowObject(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_SAT));
	}
	
	{
		UInt8		dow = 0;
		Boolean		isAllDaysBlank = true;
			
		for (dow = 0 ; dow < 7 ; dow++)
		{
			isAllDaysBlank &= (CtlGetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_SUN + (dow * 10)))) == 0);
						
		}
		
		if (isAllDaysBlank)
		{
			dow = (UInt8) DayOfWeek(gDueDate.month, gDueDate.day, gDueDate.year + ZERO_YEAR);
			if (dow < 7)
			{	
				CtlSetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_SUN + (dow * 10))), 1);
			}		
		}
	}
	return;
	
} // ShowDayControls

/*
 * ShowMonthControls
 */
static void ShowMonthControls(FormType* pForm)
{
	if (gRepeatAfterCompleted)
	{
		FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_BY_LABEL));
		FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_BY_DAY_PB));
		FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_BY_DATE_PB));
	}
	else
	{
		FrmShowObject(pForm, FrmGetObjectIndex(pForm, REPEAT_BY_LABEL));
		FrmShowObject(pForm, FrmGetObjectIndex(pForm, REPEAT_BY_DAY_PB));
		FrmShowObject(pForm, FrmGetObjectIndex(pForm, REPEAT_BY_DATE_PB));
	}
	
	if (CtlGetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_BY_DAY_PB))) == 0)
	{
		CtlSetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_BY_DATE_PB)), 1);
	}
	else
	{
		CtlSetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_BY_DATE_PB)), 0);
	}
	return;	

} // ShowMonthControls

/*
 * HideRepeatControls
 */
static void HideRepeatControls(FormType* pForm)
{
	FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_FIXED_SCHEDULE));
	FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_AFTER_COMPLETION));
	FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_SAT));
	FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_INTERVAL_LABEL));
	FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_INTERVAL_FIELD));
	FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_INTERVAL_TYPE_LABEL));
	FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_END_DATE_LABEL));
	FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_END_DATE_POPUP));
	FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_END_DATE_LIST));
	FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_BY_LABEL));
	FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_BY_DAY_PB));
	FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_BY_DATE_PB));
	FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_LABEL));
	FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_SUN));
	FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_MON));
	FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_TUE));
	FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_WED));
	FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_THU));
	FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_FRI));
	FrmHideObject(pForm, FrmGetObjectIndex(pForm, REPEAT_ON_SAT));
	
	return;
	
} // HideRepeatControls

/*
 * DisplayRepeatForm
 */
Boolean DisplayRepeatForm(FormType* pForm)
{
	FormType*		pRepeatForm = FrmInitForm(REPEAT_FORM);
		
	FrmSetActiveForm(pRepeatForm);
	
	FrmSetEventHandler(pRepeatForm, RepeatFormHandleEvent);
	
	InitializeRepeatForm(pRepeatForm);
	
	FrmDoDialog(pRepeatForm);
	
	// SaveRepeatInfo(pRepeatForm);
	
	FrmDeleteForm(pRepeatForm);
	
	pRepeatForm = NULL;
	
	FrmSetActiveForm(pForm);
	
	return (true);
	
} // DisplayRepeatForm

/*
 * 	InitializeRepeatForm(pRepeatForm)
 */
static void InitializeRepeatForm(FormType* pForm)
{								
	LoadRepeatInfo(pForm); // Do stuff!!!	
	
	return;
	
} // InitializeRepeatForm


/*
 * RepeatFormHandleEvent
 */
static Boolean RepeatFormHandleEvent(EventType* pEvent)
{
	Boolean 				handled = false;
	FormType*				pForm = FrmGetActiveForm();
	
	switch (pEvent->eType)
	{
		case frmCloseEvent:
				    			
			break;
			
		case ctlSelectEvent:
		
			switch (pEvent->data.ctlSelect.controlID)
			{
				case REPEAT_END_DATE_POPUP:
					{
						Char**          	listOptions;
						Int16 				listIdx = -1;
						Char*				ctlDateLabel = NULL;
						
						Int16				month = 0;
						Int16				day = 0;
						Int16				year = 0;
						
						Char				strMonth[3];
						Char				strDay[3];
						Char				strYear[5];

						Char				listStr[LIST_STR_LENGTH];
						MemHandle			listH = NULL;
							
						ctlDateLabel = (char *) CtlGetLabel(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_END_DATE_POPUP)));
						
						strDay[0] = ctlDateLabel[0]; strDay[1] = ctlDateLabel[1]; strDay[2] = chrNull;						
						strMonth[0] = ctlDateLabel[3]; strMonth[1] = ctlDateLabel[4]; strMonth[2] = chrNull;
						strYear[0] = ctlDateLabel[6]; strYear[1] = ctlDateLabel[7]; strYear[2] = ctlDateLabel[8];
						strYear[3] = ctlDateLabel[9]; strYear[4] = chrNull;
						
						if (TxtCharIsDigit(strDay[0]) && TxtCharIsDigit(strDay[1])
							&& TxtCharIsDigit(strMonth[0]) && TxtCharIsDigit(strMonth[1])
							&& TxtCharIsDigit(strYear[0]) && TxtCharIsDigit(strYear[1])
							&& TxtCharIsDigit(strYear[2]) && TxtCharIsDigit(strYear[3]))
						{
							month = (Int16) StrAToI(strMonth);						
							day = (Int16) StrAToI(strDay);
							year = (Int16) StrAToI(strYear);
						}
						else
						{
							DateTimeType	dt;

							TimSecondsToDateTime(TimGetSeconds(), &dt);
							
							month = dt.month;
							day = dt.day;
							year = dt.year;								
						}
						
						if (listH) // unlock... // don't want problems
						{
					        MemHandleUnlock(listH);
					        MemHandleFree(listH);
					        listH = NULL;
					    }
					    
					    MemSet(listStr, LIST_STR_LENGTH, 0);
					    StrCopy(listStr, LABEL_NO_DUE_DATE);
					    StrCopy(listStr + StrLen(LABEL_NO_DUE_DATE) + 1, LABEL_SELECT_DUE_DATE);
					    
						listH = SysFormPointerArrayToStrings(listStr, 2);
		    			listOptions = MemHandleLock(listH); // to be unlocked somewhere...

					    LstSetListChoices(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_END_DATE_LIST)), listOptions, 2);
						
						listIdx = LstPopupList(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_END_DATE_LIST)));
						
						if (listIdx == 1)
						{				
							if (SelectDay(selectDayByDay, &month, &day, &year, "Select End Date"))
							{	
								// DateType 		selDate;
								
								// selDate.month = month;
								// selDate.day = day;
								// selDate.year = year - ZERO_YEAR;
								
								// if (DateTypeCmp(selDate, gDueDate) > 0)
								// {																													
									ctlDateLabel = (char *) CtlGetLabel(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_END_DATE_POPUP)));
	
									StrPrintF(ctlDateLabel, "%02d/%02d/%02d", day, month, year);
	
									CtlSetLabel(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_END_DATE_POPUP)), ctlDateLabel);						
								// }
								// else
								// {
								// 	FrmCustomAlert(ErrOKAlert, "", "Task end date is \nbefore due date.", "");
								// }
							}
						}
						else if (listIdx == 0)
						{
							CtlSetLabel(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_END_DATE_POPUP)),
								LstGetSelectionText(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_END_DATE_LIST)), listIdx));
						}
						
						if (listH) // unlock... // don't want problems
						{
					        MemHandleUnlock(listH);
					        MemHandleFree(listH);
					        listH = NULL;
					    }
		
					}
																		
					break;
					
				case REPEAT_NONE_PB:
				
					HideRepeatControls(pForm);
					
					break;

				case REPEAT_DAY_PB:

					HideRepeatControls(pForm);

					CtlSetLabel(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_INTERVAL_TYPE_LABEL)), "Day(s)");

					ShowSomeControls(pForm);					
									
					break;
				
				case REPEAT_WEEK_PB:
				
					HideRepeatControls(pForm);

					CtlSetLabel(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_INTERVAL_TYPE_LABEL)), "Week(s)");
					
					ShowSomeControls(pForm);

					ShowWeekControls(pForm);						
																			
					break;
					
				case REPEAT_MONTH_PB:
				
					HideRepeatControls(pForm);
					
					CtlSetLabel(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_INTERVAL_TYPE_LABEL)), "Month(s)");

					ShowSomeControls(pForm);
	
					ShowMonthControls(pForm);
										
					break;
					
				case REPEAT_YEAR_PB:

					HideRepeatControls(pForm);

					CtlSetLabel(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_INTERVAL_TYPE_LABEL)), "Year(s)");

					ShowSomeControls(pForm);

					break;
					
				case REPEAT_FIXED_SCHEDULE:

					gRepeatAfterCompleted = false;
					
					if (CtlGetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_WEEK_PB))) == 1)
					{
						ShowWeekControls(pForm);	
					}
					
					if (CtlGetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_MONTH_PB))) == 1)
					{
						ShowMonthControls(pForm);	
					}
											
					break;
					
				case REPEAT_AFTER_COMPLETION:

					gRepeatAfterCompleted = true;

					if (CtlGetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_WEEK_PB))) == 1)
					{
						ShowWeekControls(pForm);	
					}
					
					if (CtlGetValue(FrmGetObjectPtr(pForm, FrmGetObjectIndex(pForm, REPEAT_MONTH_PB))) == 1)
					{
						ShowMonthControls(pForm);	
					}
					
					break;
										
				case REPEAT_DONE_BUTTON:
								
					SaveRepeatInfo(pForm);
						
					break;
					
				case REPEAT_CANCEL_BUTTON:
									
					handled = false;
				
					break;
					
				default:
					break;
			}
			
			break;
			
		default:
			break;
	
	}
			
	return (handled);
	
} // RepeatFormHandleEvent

/*
 * FormRepeat.c
 */