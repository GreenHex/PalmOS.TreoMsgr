/*
 * Common.h
 */
 
#ifndef __COMMON_H__
#define __COMMON_H__
 
/***/

#ifdef dateStringLength
#undef dateStringLength
#endif

#ifdef timeStringLength
#undef timeStringLength
#endif
 
#define dateStringLength				12 /* 00/00/0000 */
#define timeStringLength				6 /* 00:00 */

/***/

#define MIN_VERSION  					sysMakeROMVersion(5, 0, 0, sysROMStageRelease, 0)
#define LAUNCH_FLAGS 					(sysAppLaunchFlagNewGlobals | sysAppLaunchFlagUIApp)

#define PWD_LENGTH						80 	// should be multiple of 8
#define KEY_LENGTH						8 	// should be multiple of 8
#define HSSN_PTR_OFFESET				4 //  offset for copying key from

// the following for registration...
#define HSSN_LENGTH						12
#define KEY_LENGTH						8
#define HEX_KEY_LENGTH					KEY_LENGTH * 2

typedef struct {
	UInt8								pwd[PWD_LENGTH];
	UInt8								key[KEY_LENGTH];
} AppSecurityType;

typedef enum {
	decrypt 							= 0,
	encrypt 							= 1
} CryptAction_e;

#define appExpiryFileCreator			'phnM'

#define EXPIRY_SECOND					0
#define EXPIRY_MINUTE					0
#define EXPIRY_HOUR						0
#define EXPIRY_DATE						31
#define EXPIRY_MONTH					3
#define EXPIRY_YEAR						2007

// 25 calls per day * 30 days * 3 months * 10 notifications per call = ~22500
#define MAX_RUNS 						2000

typedef struct {
	UInt32								expiryDate;
	UInt32								timeLastUsed;
	UInt32								maxRuns;
	UInt32								numRuns;
} AppExpiryPrefs_t;

#define VENDOR_NAME						"www.swCP3.com"
// #define VENDOR_NAME						"www.MobiHand.com"
// #define VENDOR_NAME						"www.MyTreo.net"
// #define VENDOR_NAME						"www.Handango.com"
// #define VENDOR_NAME						"www.PalmGear.com"
// #define VENDOR_NAME						"www.Softonic.com"	

/* Compiler options **************************************************/
// #define ERROR_CHECK_FULL 			// something...
// #define WITH_REGISTRATION
// build with logging
#define MSG_LOG
/***********************************************************************/

#define ALERT_MEMORY_ERROR				"Memory Error"
#define ALERT_PHONE_ERROR				"Phone Error"

// beeps, etc...
#define SND1_FREQUENCY					900 // 1760
#define SND1_DURATION					80
#define SND2_FREQUENCY					900 // 1760
#define SND2_DURATION					80
#define PAUSE_FREQUENCY					1 // 0 doesn't play any sound // we don't use this anyway
#define PAUSE_DURATION					80

#define NOTE_TOKEN_START				"<tm>"
#define NOTE_TOKEN_END					"</tm>"
#define TEL_TOKEN_START					"<tel>"
#define TEL_TOKEN_END					"</tel>"
#define MSG_TOKEN_START					"<msg>"
#define MSG_TOKEN_END					"</msg>"

#define LEN_PhoneNum					20
#define LEN_PhoneNumbers				190
#define LEN_CallerID					64
#define LEN_Message						160
#define LEN_Description					64
#define LEN_RepeatString				64
#define LEN_ErrorString					64
//
#define LEN_GroupName					LEN_CallerID
#define LEN_GroupNumbers				LEN_PhoneNumbers
#define LEN_GroupsList					LEN_GroupName * 20
	
/*
#define PHONE_NUM_LIST_SIZE				256
#define MESSAGE_SIZE					160
#define CALLER_ID_SIZE					64
#define TASK_DESCRIPTION_SIZE			64
#define ERROR_STRING_SIZE				64
#define REPEAT_STRING_SIZE				ERROR_STRING_SIZE
#define GROUP_NAME_SIZE					CALLER_ID_SIZE
#define GROUP_NUM_LIST_SIZE				PHONE_NUM_LIST_SIZE
#define GROUPS_LIST_SIZE				GROUP_NUM_LIST_SIZE * 25
*/

/*
#define PHN_NUMBER_LENGTH				256
#define MESSAGE_LENGTH					160
#define CLID_NAME_LENGTH				64
#define ERROR_STRING_LENGTH				64
#define TASK_FIELD_LENGTH				256
#define GROUPS_NUMBER_LENGTH			256
*/
#define ID_DISPLAY_FORMAT_STRING		"^friendlyname (^label)"

#define LogDbCreator					'TrMe'
#define LogDbName						"TreoMsgrLogDB"
#define LogDbType						'DATA'

#define SEPARATOR_CHAR_START			0x008B
#define SEPARATOR_CHAR_END				0x009B

/*
 * Miscellaneous stuff
 */
typedef struct { 
	Boolean								bEnabled; // 1
	Boolean								strRegKey[HEX_KEY_LENGTH + 1]; // 17
	Boolean								bDeleteCompleted; // 1
	Boolean								bAskDeleteCompletedFirst; // 1
	Boolean								bArchiveDeletedItems; // 1
	Boolean								bRetryFailedTasks; // 1
	Int16								retryCount; // 2
	UInt8								junk[8]; // Total 24 + 8 bytes
} AppPreferences_t;

#define PREFS_ENABLED					true
#define PREFS_REG_KEY					"*UNREGISTERED*"
#define PREFS_DELETE_COMPLETED			false
#define PREFS_ASK_DELETE_FIRST			false
#define PREFS_ARCHIVE_DELETED			false
#define PREFS_RETRY_FAILED_TASKS		false
#define PREFS_RETRY_COUNT				0

typedef struct
{
	Char 								Numbers[LEN_PhoneNumbers];
	Char								DisplayName[LEN_CallerID];
	Char 								Message[LEN_Message];
} Message_t;

typedef struct {
	UInt16								idx;
	UInt32								time;
	Char								Task[LEN_Description];
	Message_t							msg;
	Boolean								isComplete;
	Boolean								isSecret;
} AppUSPrefs_t;

#define TABLE_NUM_COLUMNS				5
#define TABLE_NUM_ROWS					8

typedef enum {
	dispLog 							= 0,
	dispTasks 							= 1
} TableDisplay_e;

typedef struct {
	Char								Date[dateStringLength];
	Char								Time[timeStringLength];
	Char								Error[LEN_ErrorString];
	Char								Task[LEN_Description];
	Char								Name[LEN_CallerID];
	Char								Numbers[LEN_PhoneNumbers];
	Char 								Message[LEN_Message];
} log_t;

typedef enum {
	colStatus							= 0,
	colDate								= 1,
	colTime								= 2,
	colTask								= 3,
	colName								= 4,
	colNumber							= 5,
	colMsg								= 6
} column_e;

#endif // __COMMON_H__

/*
 * Common.h
 */
