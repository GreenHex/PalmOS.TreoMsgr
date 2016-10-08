/*
 * Win.c
 */
 
#include "Win.h"

/*
 * MainFormDispPopupWindow
 */
void MainFormDispPopupWindow(FormType* pForm, WinHandle* winHP, RectangleType* rectObscuredP)
{
	Err						error = errNone;
	RectangleType			rect = { { (160 - RECT_SIZE_X) / 2, NEXT_MSG_STR_LOC_Y - (RECT_SIZE_Y + 4) } , { RECT_SIZE_X , RECT_SIZE_Y } };
	RGBColorType 			rgb;
	IndexedColorType		txtColor;
	AppUSPrefs_t			usPrefs;				    
	log_t					log;
	UInt32					SecsToGo = 0;
	UInt32					DaysToGo = 0;
	UInt32					HoursToGo = 0;
	UInt32					MinutesToGo = 0;
		
	readUSPrefs(&usPrefs);
	
	if ((pForm) && (winHP) && (usPrefs.time)) 
	{
		*winHP = NULL; // preset
		
		if ((!usPrefs.isSecret) || (PrefGetPreference(prefShowPrivateRecords) == showPrivateRecords))
		{
			FrmSetActiveForm(pForm);
			
			WinGetFramesRectangle(dialogFrame, &rect, rectObscuredP);			
			*winHP = WinSaveBits(rectObscuredP, &error);
			
			if (error == errNone)
			{					
				rgb.r=0; rgb.g=153; rgb.b=0; // green?
				txtColor = WinRGBToIndex(&rgb);
				
				MemSet(&log, sizeof(log_t), 0);

				SecsToGo = (usPrefs.time) - TimGetSeconds();

				DaysToGo = (SecsToGo - (SecsToGo % 86400)) / 86400;				
				HoursToGo = (SecsToGo - (DaysToGo * 86400)) / 3600;
				MinutesToGo = (SecsToGo - (DaysToGo * 86400) - (HoursToGo * 3600)) / 60;
							
				StrPrintF(log.Name, "%lud %luh %lum to go", DaysToGo, HoursToGo, MinutesToGo);
									
				StrCopy(log.Error, usPrefs.Task);
				//StrCopy(log.Name, usPrefs.msg.DisplayName);
				StrCopy(log.Numbers, usPrefs.msg.Numbers);
				StrCopy(log.Message, usPrefs.msg.Message);
				
				DispPopupWindow(&log, &rect, &txtColor);
			}
		}
		else
		{
			SecVerifyPW(showPrivateRecords);
		}
	}

} // MainFormDispPopupWindow

/*
 * MainFormRestoreAfterPopup
 */
void MainFormRestoreAfterPopup(FormType* pForm, WinHandle* winHP, RectangleType* rectObscuredP)
{
	if ((pForm) && (*winHP))
	{	
		FrmSetActiveForm(pForm);
		
		WinRestoreBits(*winHP, rectObscuredP->topLeft.x, rectObscuredP->topLeft.y);
		*winHP = NULL;
	}
	
} // MainFormRestoreAfterPopup

/*
 * DrawNextMsgStr
 */
void DrawNextMsgStr(Char* str, MsgColor_e color, Boolean draw)
{
	Int16					strWidth = 0;
	
	RectangleType			rect = { { NEXT_MSG_STR_LOC_X , NEXT_MSG_STR_LOC_Y }, { NEXT_MSG_STR_LENGTH , NEXT_MSG_STR_HEIGHT } };
    RGBColorType 			rgb;
    IndexedColorType		StrColor;
		
	WinPushDrawState();
	
	WinEraseRectangle(&rect, 2); 

	if (draw)
	{
		switch (color)
		{
			case colorGreen:
	
				rgb.r=0; rgb.g=153; rgb.b=0; // Green
			
				break;
				
			case colorRed:
			
				rgb.r=250; rgb.g=0; rgb.b=51; // Red
	
				break;
				
			default:
			
				rgb.r=0; rgb.g=51; rgb.b=200; // Blue
				
				break;		
		}
		
		StrColor = WinRGBToIndex(&rgb);	
	    WinSetTextColor(StrColor);
	    
		FntSetFont(stdFont);
		strWidth = FntLineWidth(str, StrLen(str));
		WinDrawChars(str, StrLen(str), (NEXT_MSG_STR_LENGTH - strWidth) / 2, rect.topLeft.y);
	}
		    
	WinPopDrawState();
	
} // DrawNextMsgStr

/*
 * DrawEditMenu
 */
void DrawEditMenu(RectangleType* pRectEdit, RectangleType* pRectSend)
{
	Int16		strWidth = 0;
	
	WinPushDrawState();

	WinSetForeColor(UIColorGetTableEntryIndex(UIMenuSelectedForeground));
	WinSetBackColor(UIColorGetTableEntryIndex(UIMenuSelectedFill));
	WinSetTextColor(UIColorGetTableEntryIndex(UIMenuSelectedForeground));	
    FntSetFont(stdFont);   
    // WinSetTextColor(colorBlack);

	if (pRectEdit)
	{
		WinEraseRectangle(pRectEdit, 2); // 2nd arg is corner "diameter"!
    	//WinPaintRectangle(pRectEdit, 2);

	    strWidth = FntLineWidth(MENU_TXT_EDIT, StrLen(MENU_TXT_EDIT));    	
	   	WinDrawChars(MENU_TXT_EDIT, StrLen(MENU_TXT_EDIT),
	    	pRectEdit->topLeft.x + ((pRectEdit->extent.x - strWidth) / 2) + 1, 
	    	pRectEdit->topLeft.y);
	}
	
	if (pRectSend)
	{
		WinEraseRectangle(pRectSend, 2); // 2nd arg is corner "diameter"!
    	//WinPaintRectangle(pRectSend, 2);

	    strWidth = FntLineWidth(MENU_TXT_SEND, StrLen(MENU_TXT_SEND));
	    WinDrawChars(MENU_TXT_SEND, StrLen(MENU_TXT_SEND),
	    	pRectSend->topLeft.x + ((pRectSend->extent.x - strWidth) / 2), 
	    	pRectSend->topLeft.y);
	}

	WinPopDrawState();
		
} // DrawEditMenu


/*
 * DispPopupWindow
 */
void DispPopupWindow(log_t* logP, RectangleType *rectP, IndexedColorType *txtColorP)
{
    Int16					strHeight = 0;
    
    RGBColorType 			rgb;
    IndexedColorType		colorWhite;
    IndexedColorType		colorBlack;
	IndexedColorType		colorGrey;
	
	WinPushDrawState();
	
	rgb.r=255; rgb.g=255; rgb.b=255; // white
	colorWhite = WinRGBToIndex(&rgb);	
	
	rgb.r=0; rgb.g=0; rgb.b=0; // black
	colorBlack = WinRGBToIndex(&rgb);
	
	rgb.r=150; rgb.g=150; rgb.b=150; // grey
	colorGrey = WinRGBToIndex(&rgb);
		
	WinSetForeColor(colorGrey);
	WinSetBackColor(colorWhite);
	
    WinEraseRectangleFrame(dialogFrame, rectP); 
    WinEraseRectangle(rectP, 2); // 2nd arg is corner "diameter"!

    WinPaintRectangleFrame(dialogFrame, rectP);
    
    FntSetFont(boldFont);
	strHeight = FntLineHeight();

	WinSetTextColor(*txtColorP);
    WinDrawTruncChars(logP->Error, StrLen(logP->Error),
    	rectP->topLeft.x + TXT_OFF_X, /* + (RECT_SIZE_X - strWidth)/2, */
    	rectP->topLeft.y + TXT_OFF_Y, DISP_WIDTH - (2 * TXT_OFF_X));

    FntSetFont(stdFont);
	strHeight = FntLineHeight();
    
    WinSetTextColor(colorBlack);
    WinDrawTruncChars(logP->Name, StrLen(logP->Name),
    	rectP->topLeft.x + TXT_OFF_X, /* + (RECT_SIZE_X - strWidth)/2, */
    	rectP->topLeft.y + TXT_OFF_Y + strHeight, DISP_WIDTH - (2 * TXT_OFF_X) - 4);
    
    WinSetTextColor(colorGrey);
    WinDrawTruncChars(logP->Numbers, StrLen(logP->Numbers),
    	rectP->topLeft.x + TXT_OFF_X, /* + (RECT_SIZE_X - strWidth)/2, */
    	rectP->topLeft.y + TXT_OFF_Y + (strHeight * 2), DISP_WIDTH - (2 * TXT_OFF_X) - 4);

	WinSetTextColor(colorBlack);
    WinDrawTruncChars(logP->Message, StrLen(logP->Message),
    	rectP->topLeft.x + TXT_OFF_X, /* + (RECT_SIZE_X - strWidth)/2, */
    	rectP->topLeft.y + TXT_OFF_Y + (strHeight * 3), DISP_WIDTH - (2 * TXT_OFF_X) - 4);
    	
    WinPopDrawState();

} // DispPopupWindow

/*
 * Win.c
 */
