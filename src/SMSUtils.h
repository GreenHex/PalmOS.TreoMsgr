/*
 * SMSUtils.h
 */

#ifndef __SMSUTILS_H__
#define __SMSUTILS_H__

#include <PalmOS.h>
#include <Hs.h>
#include <HsNav.h>
#include <HsExt.h>
#include <HsPhone.h>
#include <Progress.h>

#include "Global.h"
#include "CallerLookup.h"
#include "GroupsList.h"
#include "StrUtils.h"
#include "MsgLog.h"
#include "AppResources.h"

// defines
#define ERR_STR_PHN_OFF					"Radio Off"
#define ERR_STR_NETWORK_ERR				"Network Error"
#define ERR_STR_SMS_CREATE_ERR			"Message Create Error"
#define ERR_STR_ADDRESS_LIST_ERR		"Address List Error"
#define ERR_STR_INVALID_ADDRESS_ERR		"Address Add Error"
#define ERR_STR_ADDRESS_ERR				"Address Error"
#define ERR_STR_SMS_ERR					"SMS Error"
#define ERR_STR_SMS_SENT				"Message Sent"
//
#define SEPARATOR_STR					","

// Trim.c
// extern char*			trim(char *str);

// Prototypes
extern Err				SplitGroupAndSendMessage(AppUSPrefs_t* usPrefsP, ProgressPtr pProgress, Int16 retryCount);
extern Boolean 			SendMsgCallback(PrgCallbackDataPtr pCallbackData);

#endif /* __SMSUTILS_H__ */

/*
 * SMSUtils.h
 */
 