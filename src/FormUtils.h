/*
 * FormUtils.h
 */

#ifndef __FORMUTILS_H__
#define __FORMUTILS_H__

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
#include "GroupsList.h"
#include "Win.h"
#include "Prefs.h"
#include "FormLog.h"
#include "AppResources.h"

#define STATS_TXT_WIDTH		24

extern Char					gGroupsList[];
extern MemHandle			gGroupsH;

// Expire.c
extern UInt16 				DisplayRegForm(Char* StrKey) EXTRA_SECTION_ONE;

// Encrypt.c
extern Boolean 				VerifyRegistration(Char* RegKey);

// AppMain.c
extern void 				RegisterForNotifications(Boolean bRegister);
extern void 				setupAlarm(void);

// Prototypes
extern Boolean 				MainFormHandleEvent(EventType* pEvent) EXTRA_SECTION_ONE;
extern Boolean 				GroupsFormHandleEvent(EventType* pEvent) EXTRA_SECTION_ONE;
extern Boolean				PrefsFormHandleEvent(EventType* pEvent) EXTRA_SECTION_ONE;

#endif /* __FORMUTILS_H__ */

/*
 * FormUtils.h
 */