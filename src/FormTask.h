/*
 * FormTask.h
 */
 
#ifndef __FORMTASK_H__
#define __FORMTASK_H__

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
#include "ToDoDB.h"
#include "Tasks.h"
#include "FormRepeat.h"
#include "AppResources.h"

extern UInt16				topRow;
extern UInt16				gCurrentIdx; /* the actual index of the record in the DB */
extern Char*				ctlDateLabel;
extern Char*				ctlTimeLabel;
extern Boolean				bTaskEditing;
extern Char					gGroupsList[];
extern MemHandle			gGroupsH;
extern DateType				gDueDate;
extern Boolean				gHasRepeatInfo;
extern ToDoRepeatInfoType	gToDoRepeatInfo;
extern Boolean				gRepeatAfterCompleted;

// AppMain.c
extern void 				setupAlarm(void);

// Prototypes
extern Boolean 				DisplayNewTaskForm(FormType* pForm) EXTRA_SECTION_ONE;
extern Boolean 				TaskFormHandleEvent(EventType* pEvent) EXTRA_SECTION_ONE;

#endif /* __FORMTASK_H__ */

/*
 * FormTask.h
 */