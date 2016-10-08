/*
 * GroupsList.h
 */
 
#ifndef __GROUPSLIST_H__
#define __GROUPSLIST_H__

#include <Hs.h>
#include <HsAppLaunchCmd.h>
#include <DataMgr.h>

#include "Sections.h"
#include "Global.h"

#define GroupsDbCreator			'TrMe'
#define GroupsDbName			"TreoMsgrGroupsDB"
#define GroupsDbType			'DBGR'

typedef struct
{
	Char 						Name[LEN_GroupName];
	Char						Numbers[LEN_GroupNumbers];
} Groups_t;

// Prototypes
extern UInt16 					GetNumGroupRecords(void);
extern Boolean 					FindGroup(Char* nameStr, UInt16* idxP, Groups_t* groupP);
extern Boolean 					GetGroup(UInt16 idx, Groups_t* groupP);
extern Err 						SaveNewGroup(Groups_t* groupP, Boolean isSecret, UInt16* idxP) EXTRA_SECTION_ONE;
extern Boolean 					ReplaceGroup(UInt16* idxP, Groups_t* groupP, Boolean isSecret) EXTRA_SECTION_ONE;
extern Boolean 					RemoveGroup(UInt16* idxP) EXTRA_SECTION_ONE;
extern Boolean 					LoadGroupsList(ListType* pList, Char* list, MemHandle* listHP) EXTRA_SECTION_ONE;

#endif // __GROUPSLIST_H__

/*
 * GroupsList.h
 */