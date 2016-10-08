/*
 * FormLog.h
 */

#ifndef __FORMLOG_H__
#define __FORMLOG_H__

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
#include <ScrollBar.h>
#include <Category.h>

#include "Sections.h"
#include "Global.h"
#include "Win.h"
#include "Tasks.h"
#include "FormTask.h"
#include "AppResources.h"

extern UInt16				topRow;
extern UInt16				gCurrentIdx; /* the actual index of the record in the DB */
extern TableDisplay_e		currentForm;
extern Char*				ctlDateLabel;
extern Char*				ctlTimeLabel;
extern Boolean				bTaskEditing;
extern Boolean				gHasRepeatInfo;
extern ToDoRepeatInfoType	gToDoRepeatInfo;
extern Boolean				gRepeatAfterCompleted;

// Table.c
extern void 				TableInit(TableType* tableP, TableDisplay_e DispType) EXTRA_SECTION_ONE;

// AppMain.c
extern void 				setupAlarm(void);

// Prototypes
extern Boolean 				DisplayLogForm(void) EXTRA_SECTION_ONE;
extern Boolean 				LogFormHandleEvent(EventType* pEvent) EXTRA_SECTION_ONE;

#endif /* __FORMLOG_H__ */

/*
 * FormLog.h
 */
