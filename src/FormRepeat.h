/*
 * FormRepeat.h
 */
 
#ifndef __FORMREPEAT_H__
#define __FORMREPEAT_H__

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
#include "PhoneUtils.h"
#include "ToDoDB.h"
#include "Tasks.h"
#include "AppResources.h"

extern DateType					gDueDate;
extern Boolean					gHasRepeatInfo;
extern ToDoRepeatInfoType		gToDoRepeatInfo;
extern Boolean					gRepeatAfterCompleted;

#define LIST_STR_LENGTH			100
#define LABEL_NO_DUE_DATE		"No End Date"
#define LABEL_SELECT_DUE_DATE	"Select End Date..."

// Prototypes
extern Boolean 					DisplayRepeatForm(FormType* pForm) EXTRA_SECTION_TWO;

#endif /* __FORMREPEAT_H__ */

/*
 * FormRepeat.h
 */