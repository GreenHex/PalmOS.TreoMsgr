/*
 * Table.c
 */

#include <PalmOS.h>
#include <PalmTypes.h>
#include <Hs.h>
#include <HsPhone.h>
#include <HsNav.h>
#include <HsExt.h>
#include <palmOneResources.h>
#include <Form.h>
#include <List.h>
#include <AlarmMgr.h>
#include <Progress.h>
#include <DataMgr.h>
#include <List.h> 
#include <Field.h>

#include "Global.h"
#include "MsgLog.h"
#include "Tasks.h"
#include "StrUtils.h"
#include "AppResources.h" 

// Bitmap.c 
extern BitmapType* 			getBitmap(DmResID resID, Coord* widthP, Coord* heightP, MemHandle* bmpHP) EXTRA_SECTION_ONE;
extern void 				releaseBitmap(MemHandle* bmpHP) EXTRA_SECTION_ONE;
 
// AppMain.c
extern UInt16				topRow;

// Prototypes
void 						TableInit(TableType* tableP, TableDisplay_e DispType) EXTRA_SECTION_ONE;

static Err 					LoadTextTableLogItem(void* tableP, Int16 row, Int16 column,
									Boolean editable, MemHandle *dataH,
									Int16 *dataOffset, Int16 *dataSize, FieldType *field) EXTRA_SECTION_ONE;
static void 				TableStatusDrawLogItem(void *tableP, Int16 row, Int16 column, RectangleType *bounds) EXTRA_SECTION_ONE;
static Err 					LoadTextTableTaskItem(void* tableP, Int16 row, Int16 column,
									Boolean editable, MemHandle *dataH,
									Int16 *dataOffset, Int16 *dataSize, FieldType *field) EXTRA_SECTION_ONE;
static void 				TableStatusDrawTaskItem(void *tableP, Int16 row, Int16 column, RectangleType *bounds) EXTRA_SECTION_ONE;
  
/*
 * LoadTextTableLogItem
 */
static Err LoadTextTableLogItem(void* tableP, Int16 row, Int16 column,
					Boolean editable, MemHandle *dataH,
					Int16 *dataOffset, Int16 *dataSize, FieldType *field)
{
	Err				error = errNone;
	MemHandle		mH = NULL;
	Char*			cP = NULL;
	
	log_t			log;
	
	MemSet(&log, sizeof(log_t), 0);
											
	if (SplitLogEntry(row + topRow, &log, NULL))
	{	
		switch (column)
		{
			case colStatus:
	
				mH = MemHandleNew(StrLen(log.Error) + 1);
				cP = MemHandleLock(mH);
				StrCopy(cP, log.Error);
			
				break;
				
			case colDate:
	
				mH = MemHandleNew(StrLen(log.Date) + 1);
				cP = MemHandleLock(mH);
				StrCopy(cP, FormatDate(log.Date));
						
				break;
			
			case colTime:
	
				mH = MemHandleNew(StrLen(log.Time) + 1);
				cP = MemHandleLock(mH);
				StrCopy(cP, FormatTime(log.Time));
			
				break;
				
			case colTask:
	
				mH = MemHandleNew(StrLen(log.Task) + 1);
				cP = MemHandleLock(mH);
				StrCopy(cP, log.Task);
			
				break;
				
			case colName: 
	
				mH = MemHandleNew(StrLen(log.Name) + 1);
				cP = MemHandleLock(mH);
				StrCopy(cP, log.Name);
			
				break;
				
			case colNumber: 
	
				mH = MemHandleNew(StrLen(log.Numbers) + 1);
				cP = MemHandleLock(mH);
				StrCopy(cP, log.Numbers);
			
				break;
				
			case colMsg:
	
				mH = MemHandleNew(StrLen(log.Message) + 1);
				cP = MemHandleLock(mH);
				StrCopy(cP, log.Message);
			
				break;
				
			default:
				
				mH = MemHandleNew(StrLen("INVALID") + 1);
				cP = MemHandleLock(mH);
				StrCopy(cP, "INVALID");
					
				break;
		}
		
		MemHandleUnlock(mH);
		
	}
	else
	{
		mH = MemHandleNew(1);
		cP = MemHandleLock(mH);
		cP[0] = chrNull;	
	}
	
	*dataH = mH;
	*dataOffset = 0;
	*dataSize = MemHandleSize(*dataH);
	
	{
		FieldAttrType		attr;
		
		FldGetAttributes(field, &attr);
		attr.underlined &= noUnderline;
		FldSetAttributes(field, &attr);
	}
	return error;
	
} // LoadTextTableLogItem

/*
 * TableStatusDrawLogItem
 */
static void TableStatusDrawLogItem(void *tableP, Int16 row, Int16 column, RectangleType *bounds)
{
	log_t		log;
	Boolean		isSecret = false;
	
	MemSet(&log, sizeof(log_t), 0);
						
	if ((column == colStatus) && (SplitLogEntry(row + topRow, &log, &isSecret)))
	{
		MemHandle 			bmpH;
		BitmapType* 		pBmp;
			
		if (StrCompare(log.Error, "Message Sent") == 0)
		{
			pBmp = getBitmap(ICON_TICK, NULL, NULL, &bmpH);
		}
		else if (StrCompare(log.Error, "Message Deleted") == 0)
		{
			pBmp = getBitmap(ICON_COMPLETED_DELETED, NULL, NULL, &bmpH);
		}
		else if (StrStr(log.Error, "Messages Sent"))
		{
			pBmp = getBitmap(ICON_SUMMARY, NULL, NULL, &bmpH);
		}
		else // Summary
		{
			pBmp = getBitmap(ICON_CROSS, NULL, NULL, &bmpH);					
		}
		
		WinDrawBitmap(pBmp, bounds->topLeft.x, bounds->topLeft.y);
		
		releaseBitmap(&bmpH);
		
		TblSetRowData(tableP, row, (UInt32) row + topRow);
		
		TblSetRowMasked(tableP, row, (isSecret && (PrefGetPreference(prefShowPrivateRecords) != showPrivateRecords)));
	}
	
} // TableStatusDrawLogItem

/*
 * LoadTextTableTaskItem
 */
static Err LoadTextTableTaskItem(void* tableP, Int16 row, Int16 column,
					Boolean editable, MemHandle *dataH,
					Int16 *dataOffset, Int16 *dataSize, FieldType *field)
{
	Err			error = errNone;
	MemHandle	mH = NULL;
	Char*		cP = NULL;
	
	log_t		log;
	
	{ // remove underline
		FieldAttrType		attr;
		
		FldGetAttributes(field, &attr);
		attr.underlined &= noUnderline;
		FldSetAttributes(field, &attr);
	}
	
	MemSet(&log, sizeof(log_t), 0);
							
	if (SplitTaskEntry(row + topRow, &log, NULL))
	{	
		switch (column)
		{
			case colStatus:
	
				mH = MemHandleNew(StrLen(log.Error) + 1);
				cP = MemHandleLock(mH);
				StrCopy(cP, log.Error);
			
				break;
				
			case colDate:
	
				mH = MemHandleNew(StrLen(log.Date) + 1);
				cP = MemHandleLock(mH);
				StrCopy(cP, FormatDate(log.Date));
						
				break;
			
			case colTime:
	
				mH = MemHandleNew(StrLen(log.Time) + 1);
				cP = MemHandleLock(mH);
				StrCopy(cP, FormatTime(log.Time));
			
				break;
				
			case colTask:
	
				mH = MemHandleNew(StrLen(log.Task) + 1);
				cP = MemHandleLock(mH);
				StrCopy(cP, log.Task);
			
				break;
				
			case colName: 
	
				mH = MemHandleNew(StrLen(log.Name) + 1);
				cP = MemHandleLock(mH);
				StrCopy(cP, log.Name);
			
				break;
				
			case colNumber: 
	
				mH = MemHandleNew(StrLen(log.Numbers) + 1);
				cP = MemHandleLock(mH);
				StrCopy(cP, log.Numbers);
			
				break;
				
			case colMsg:
	
				mH = MemHandleNew(StrLen(log.Message) + 1);
				cP = MemHandleLock(mH);
				StrCopy(cP, log.Message);
			
				break;
				
			default:
				
				mH = MemHandleNew(StrLen("INVALID") + 1);
				cP = MemHandleLock(mH);
				StrCopy(cP, "INVALID");
					
				break;
		}
		
		MemHandleUnlock(mH);
		
	}
	else
	{
		
		mH = MemHandleNew(1);
		cP = MemHandleLock(mH);
		cP[0] = chrNull;	
	}
	
	*dataH = mH;
	*dataOffset = 0;
	*dataSize = MemHandleSize(*dataH);

	return error;
	
} // LoadTextTableTaskItem

/*
 * TableStatusDrawTaskItem
 */
static void TableStatusDrawTaskItem(void *tableP, Int16 row, Int16 column, RectangleType *bounds)
{
	AppUSPrefs_t 		usPrefs;
	
	MemSet(&usPrefs, sizeof(AppUSPrefs_t), 0);
	
	if (column == colStatus)
	{
		MemHandle 			bmpH;
		BitmapType* 		pBmp;
		UInt32				TimeNow = TimGetSeconds();
		
		if (ReadTaskByIdx(row + topRow, &usPrefs, NULL, GetReadOnlySecretMode()))
		{	
			if ((usPrefs.time == 0)
				|| (!StrLen(usPrefs.msg.Numbers))
				|| (!StrLen(usPrefs.msg.Message)))
			{
				pBmp = getBitmap(ICON_INVALID, NULL, NULL, &bmpH);
			}
			else if (usPrefs.isComplete)
			{
				pBmp = getBitmap(ICON_PAST_COMPLETE, NULL, NULL, &bmpH);
			}
			else if (TimeNow > usPrefs.time)
			{
				pBmp = getBitmap(ICON_PAST_INCOMPLETE, NULL, NULL, &bmpH);
			}
			else // success
			{
				pBmp = getBitmap(ICON_FUTURE, NULL, NULL, &bmpH);
			}
			WinDrawBitmap(pBmp, bounds->topLeft.x, bounds->topLeft.y);
			
			releaseBitmap(&bmpH);
			
			TblSetRowData(tableP, row, (UInt32) usPrefs.idx);
			TblSetRowMasked(tableP, row, (usPrefs.isSecret && (PrefGetPreference(prefShowPrivateRecords) && maskPrivateRecords)));
		}
		else
		{
			TblSetRowMasked(tableP, row, false);
		}		
	}
	
} // TableStatusDrawTaskItem

/*
 * TableInit
 */
void TableInit(TableType* tableP, TableDisplay_e DispType)
{ 
	UInt16				numTableRows = 0;
	UInt16				row = 0;
		
	if (!tableP)
		return;
		
	numTableRows = TblGetNumberOfRows(tableP);
	
	for (row = 0; row < numTableRows; row++)
	{		
		TblSetItemStyle(tableP, row, colStatus, customTableItem);
		TblSetItemStyle(tableP, row, colDate, textTableItem);
		TblSetItemStyle(tableP, row, colTime, textTableItem);
		TblSetItemStyle(tableP, row, colTask, textTableItem);
		TblSetItemStyle(tableP, row, colName, textTableItem);
		TblSetItemStyle(tableP, row, colNumber, textTableItem);
		TblSetItemStyle(tableP, row, colMsg, textTableItem);
		
		// TblSetItemFont(tableP, row, colStatus, stdFont);
		TblSetItemFont(tableP, row, colDate, stdFont);
		TblSetItemFont(tableP, row, colTime, stdFont);
		TblSetItemFont(tableP, row, colTask, stdFont);
		TblSetItemFont(tableP, row, colName, stdFont);
		TblSetItemFont(tableP, row, colNumber, stdFont);		
		TblSetItemFont(tableP, row, colMsg, stdFont);
		
		TblSetRowUsable(tableP, row, true);
		TblSetRowMasked(tableP, row, false);
		// TblSetRowSelectable(tableP, row, false);
	} 
	 
	TblSetColumnUsable(tableP, colStatus, true);
	TblSetColumnUsable(tableP, colDate, true);
	TblSetColumnUsable(tableP, colTime, true);
	TblSetColumnUsable(tableP, colTask, true);
	TblSetColumnUsable(tableP, colName, true);
	TblSetColumnUsable(tableP, colNumber, true);
	TblSetColumnUsable(tableP, colMsg, true);
	
	TblSetColumnMasked(tableP, colStatus, false);
	TblSetColumnMasked(tableP, colDate, true);
	TblSetColumnMasked(tableP, colTime, true);
	TblSetColumnMasked(tableP, colTask, true);
	TblSetColumnMasked(tableP, colName, true);
	TblSetColumnMasked(tableP, colNumber, true);
	TblSetColumnMasked(tableP, colMsg, true);
	 
	if (DispType == dispLog) 
	{
		TblSetCustomDrawProcedure(tableP, colStatus, TableStatusDrawLogItem);
		TblSetLoadDataProcedure(tableP, colDate, LoadTextTableLogItem);
		TblSetLoadDataProcedure(tableP, colTime, LoadTextTableLogItem);
		TblSetLoadDataProcedure(tableP, colTask, LoadTextTableLogItem);
		TblSetLoadDataProcedure(tableP, colName, LoadTextTableLogItem);
		TblSetLoadDataProcedure(tableP, colNumber, LoadTextTableLogItem);
		TblSetLoadDataProcedure(tableP, colMsg, LoadTextTableLogItem);
	}
	else // dispTasks
	{
		TblSetCustomDrawProcedure(tableP, colStatus, TableStatusDrawTaskItem);
		TblSetLoadDataProcedure(tableP, colDate, LoadTextTableTaskItem);
		TblSetLoadDataProcedure(tableP, colTime, LoadTextTableTaskItem);
		TblSetLoadDataProcedure(tableP, colTask, LoadTextTableTaskItem);
		TblSetLoadDataProcedure(tableP, colName, LoadTextTableTaskItem);
		TblSetLoadDataProcedure(tableP, colNumber, LoadTextTableTaskItem);
		TblSetLoadDataProcedure(tableP, colMsg, LoadTextTableTaskItem);
	}
	
	TblHasScrollBar(tableP, true); 
	
	TblMarkTableInvalid(tableP);

} // TableInit

/*
 * Table.c
 */
 