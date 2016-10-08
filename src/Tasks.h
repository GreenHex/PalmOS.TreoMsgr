/*
 * Tasks.h
 */
 
#ifndef __TASKS_H__
#define __TASKS_H__ 

#include <PalmOS.h>
#include <PalmUtils.h>
#include <Hs.h>
#include <HsExt.h>
#include <palmOneResources.h>
#include <PalmTypes.h>
#include <Category.h>
// #include <TxtGlue.h>

#include "Global.h"
#include "ToDoDB.h"
#include "StrUtils.h"
#include "PhoneUtils.h"

typedef enum {
	deleteRecord = 0,
	archiveRecord = 1
} deleteType_e;

#define	ZERO_YEAR						1904
#define MESSAGING_ADVANCE_TIME			-3

// ToDoDB.c
extern Char* 				ToDoDBRecordGetFieldPointer(ToDoDBRecordPtr recordP, ToDoRecordFieldType field);
extern Int16 				DateTypeCmp(DateType d1, DateType d2);

// Prototypes
extern void 				CreateCategory(void);
extern Boolean 				DeleteTask(UInt16 idx, deleteType_e deleteType);
extern Boolean 				ReadTaskByIdx(UInt16 idx, AppUSPrefs_t* usPrefsP, Char* RepeatInfoStr, UInt16 dmReadMode);
extern Boolean 				SplitTaskEntry(UInt16 idx, log_t* log, Boolean* isSecretP);
extern UInt16				GetNumTaskRecords(UInt16 dmReadMode);
extern Boolean				SetCompletionStatus(UInt16* recordIdxP);
extern Boolean 				GetFirstValidTaskIdx(AppUSPrefs_t* usPrefsP);
extern DateType				gDueDate;
extern Boolean				gHasRepeatInfo;
extern ToDoRepeatInfoType	gToDoRepeatInfo;
extern Boolean				gRepeatAfterCompleted;

#endif /* __TASKS_H__ */

/*
 * Tasks.h
 */
