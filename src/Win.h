/*
 * Win.h
 */

#ifndef __WIN_H__
#define __WIN_H__

#include <PalmOS.h>
#include <PalmTypes.h>
#include <Hs.h>
#include <HsNav.h>
#include <HsExt.h>
#include <HsPhone.h>

#include "Global.h"
#include "GroupsList.h"
#include "SMSUtils.h"
#include "Prefs.h"

// Defines and Globals
#define DISP_WIDTH							156
#define DISP_HEIGHT							160

#define DISP_CEN_X							DISP_WIDTH / 2
#define DISP_CEN_Y							DISP_HEIGHT / 2

#define RECT_SIZE_X							150
#define RECT_SIZE_Y							14 + (11 * 3) 

#define RECT_TOP_X							(DISP_WIDTH - RECT_SIZE_X) / 2
#define RECT_TOP_Y							(DISP_HEIGHT - RECT_SIZE_Y) / 2

#define TXT_OFF_X							4
#define TXT_OFF_Y							1

#define MENU_ITEM_SIZE_X					32
#define MENU_ITEM_SIZE_Y					11
#define MENU_SPACE							1
#define MENU_ITEM_TEXT_OFFSET_Y				1

#define MENU_TXT_EDIT						"Edit"
#define MENU_TXT_SEND						"Send"

#define NEXT_MSG_STR_LOC_X					0
#define NEXT_MSG_STR_LOC_Y					145 /* 128 */
#define NEXT_MSG_STR_LENGTH					160 // pixels, width of display...
#define NEXT_MSG_STR_HEIGHT					12

typedef enum {
	colorDefault							= 0,
	colorBlue 								= 0,
	colorGreen 								= 1,
	colorRed								= 2
} MsgColor_e;

/*
#define LEN_DisplayString					36 // characters that can be displayed

typedef struct { 
	Char									L1[LEN_DisplayString];
	Char									L2[LEN_DisplayString];
	Char									L3[LEN_DisplayString];
	Char									L4[LEN_DisplayString];
} Display_t;
*/

// Prototypes
extern void 			MainFormDispPopupWindow(FormType* pForm, WinHandle* winHP, RectangleType* rectObscuredP) EXTRA_SECTION_ONE;
extern void 			MainFormRestoreAfterPopup(FormType* pForm, WinHandle* winHP, RectangleType* rectObscuredP) EXTRA_SECTION_ONE;
extern void				DrawNextMsgStr(Char* str, MsgColor_e color, Boolean draw) EXTRA_SECTION_ONE;
extern void 			DrawEditMenu(RectangleType* pRectEdit, RectangleType* pRectSend) EXTRA_SECTION_ONE;
extern void 			DispPopupWindow(log_t* logP, RectangleType *rectP, IndexedColorType *txtColorP) EXTRA_SECTION_ONE;

#endif /* __WIN_H__ */

/*
 * Win.h
 */