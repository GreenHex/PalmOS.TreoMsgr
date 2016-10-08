/*
 * AddrDB.c
 */
 
#include <Hs.h>
#include <HsAppLaunchCmd.h>
#include <DataMgr.h>

#include "AddrDB.h"

#define BitAtPosition(pos)                ((UInt32)1 << (pos))
#define GetBitMacro(bitfield, index)      ((bitfield) & BitAtPosition(index))

// Prototypes
Err 			AddrDBGetRecord(DmOpenRef dbP, UInt16 index, AddrDBRecordPtr recordP, MemHandle* recordH);
Err 			AddrDBGetDatabase(DmOpenRef* dbPP, UInt16 mode);
static void 	PrvAddrDBUnpack(PrvAddrPackedDBRecord* src, AddrDBRecordPtr dest);

/*
 * PrvAddrDBUnpack
 */
void PrvAddrDBUnpack(PrvAddrPackedDBRecord *src, AddrDBRecordPtr dest)
{
	Int16   	index;
	UInt32 		flags;
	char*		p;

	dest->options = src->options;
	flags = src->flags.allBits;
	p = &src->firstField;

	for (index = firstAddressField; index < addressFieldsCount; index++)
	{
		// If the flag is set point to the string else NULL
		if (GetBitMacro(flags, index) != 0)
		{
			dest->fields[index] = p;
			p += StrLen(p) + 1;
		}
		else
			dest->fields[index] = NULL;
	}
}

/*
 * AddrDBGetRecord
 */
Err AddrDBGetRecord(DmOpenRef dbP, UInt16 index, AddrDBRecordPtr recordP, MemHandle *recordH)
{
	PrvAddrPackedDBRecord*		src;

	*recordH = DmQueryRecord(dbP, index);
	src = (PrvAddrPackedDBRecord *) MemHandleLock(*recordH);
	if (src == NULL)
		return dmErrIndexOutOfRange;

	PrvAddrDBUnpack(src, recordP);

	return 0;
}

/*
 * AddrDBGetDatabase
 */
Err AddrDBGetDatabase(DmOpenRef *dbPP, UInt16 mode)
{
	Err error = 0;
	DmOpenRef dbP;
//	UInt16 cardNo;
//	LocalID dbID;

	*dbPP = NULL;

	// Find the application's data file.  If it doesn't exist create it.
	dbP = DmOpenDatabaseByTypeCreator(addrDBType, sysFileCAddress, mode);
	if (!dbP)
	{
		error = DmCreateDatabase(0, addrDBName, sysFileCAddress, addrDBType, false);
		if (error)
			return error;

		dbP = DmOpenDatabaseByTypeCreator(addrDBType, sysFileCAddress, mode);
		if (!dbP)
			return (1);
	}

	*dbPP = dbP;
	return 0;
}
