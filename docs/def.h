

#define LEN_PhoneNum					20
#define LEN_PhoneNumbers				256
#define LEN_CallerID					64
#define LEN_Message						160
#define LEN_Description					64
#define LEN_RepeatString				64
#define LEN_ErrorString					64
//
#define LEN_GroupName					LEN_CallerID
#define LEN_GroupNumbers				LEN_PhoneNum

//
#define LEN_GroupsList					LEN_GroupNumbers * 20
	
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
typedef struct
{
	Char 								Address[LEN_PhoneNumbers];
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

typedef struct {
	Char								Date[dateStringLength];
	Char								Time[timeStringLength];
	Char								Error[LEN_ErrorString];
	Char								Task[LEN_Description];
	Char								Name[LEN_CallerID];
	Char								Numbers[LEN_PhoneNumbers];
	Char 								Message[LEN_Message];
} log_t;