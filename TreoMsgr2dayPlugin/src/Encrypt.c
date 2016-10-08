/*
 * Encrypt.c
 */

#include <PalmOS.h>
#include <HsExt.h>
#include <Encrypt.h>

#include "../../TreoMsgr/src/Common.h"

// Prototypes
void 							encBuf(UInt8* io_buffer, UInt16 buf_len, UInt8* key, CryptAction_e action);
Boolean 						encAppPrefs(AppPreferences_t* pPrefs, CryptAction_e action);
Boolean 						VerifyRegistration(Char* RegKey);

static void 					hex2bin(int len, char* hexnum, char* binnum);
// static void 					bin2hex(int len, unsigned char* binnum, char* hexnum);
static Boolean 					hasDES(void);
static void 					getROMID(UInt8* keyVal);

static const char hex[16] =
{
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'a', 'b', 'c', 'd', 'e', 'f',
};

#ifdef _LIBC
# define hexval(c) \
  (c >= '0' && c <= '9'							      \
   ? c - '0'								      \
   : ({	int upp = toupper (c);						      \
	upp >= 'A' && upp <= 'Z' ? upp - 'A' + 10 : -1; }))
#else
static char hexval(char c);
#endif

/*
 * Hex to binary conversion
 */
static void hex2bin(int len, char* hexnum, char* binnum)
{
  int i;

  for (i = 0; i < len; i++)
    *binnum++ = 16 * hexval(hexnum[2 * i]) + hexval(hexnum[2 * i + 1]);
}

/*
 * Binary to hex conversion
 */
 /* NOT USED!!!
static void bin2hex(int len, unsigned char* binnum, char* hexnum)
{
  int i;
  unsigned val;

  for (i = 0; i < len; i++)
    {
      val = binnum[i];
      hexnum[i * 2] = hex[val >> 4];
      hexnum[i * 2 + 1] = hex[val & 0xf];
    }
  hexnum[len * 2] = 0;
}
*/

/*
 * hexval
 */
static char hexval(char c)
{
  if (c >= '0' && c <= '9')
    return (c - '0');
  else if (c >= 'a' && c <= 'z')
    return (c - 'a' + 10);
  else if (c >= 'A' && c <= 'Z')
    return (c - 'A' + 10);
  else
    return -1;
}

/*
 * hasDES
 */
static Boolean hasDES(void)
{
	UInt32 		attributes;
	
	return (FtrGet(sysFtrCreator, sysFtrNumEncryption, &attributes) == errNone)
    			&& ((attributes & sysFtrNumEncryptionMaskDES) != 0);
}

/*
 * encBuf
 */
void encBuf(UInt8* io_buffer, UInt16 buf_len, UInt8* key, CryptAction_e action)
{
	UInt8*	temp_buffer;
	UInt16	i;
	
	if (!hasDES())
		return;
	
	temp_buffer = MemPtrNew(buf_len);
	if (temp_buffer)
	{
		MemSet(temp_buffer, buf_len, 0);
		
		for (i = 0; i < buf_len; i += 8)
		{
			EncDES(&(io_buffer[i]), key, &(temp_buffer[i]), (action == encrypt));
		}
		  	
		MemMove(io_buffer, temp_buffer, buf_len);
	}
}

/*
 * encAppPrefs
 */
Boolean encAppPrefs(AppPreferences_t* pPrefs, CryptAction_e action)
{
	Boolean					retVal = false; 
	UInt8*					iPrefs = NULL; // to store appExpirtyPrefs for crypt operations
	UInt8					key[8];

	iPrefs = MemPtrNew(sizeof(AppPreferences_t));
	if (iPrefs)
	{
		MemSet(iPrefs, sizeof(AppPreferences_t), 0);

		getROMID(key);

		MemMove(iPrefs, pPrefs, sizeof(AppPreferences_t));
		encBuf(iPrefs, sizeof(AppPreferences_t), key, action);
		MemMove(pPrefs, iPrefs, sizeof(AppPreferences_t)); // move back decrypted
		
		MemPtrFree(iPrefs);	
		retVal = true;
	}
	return retVal;
}

/*
 * getROMID
 */
static void getROMID(UInt8* keyVal)
{
	Err 		error;
	
	Char 		bufP[32] = "\0"; 
	UInt16 		bufLen = sizeof(bufP);	
	Char*		altKey = "TreoMsgR";
 
	error = HsGetVersionString(hsVerStrSerialNo, bufP, &bufLen);

	if (error)
		MemMove(keyVal, altKey, 8);
	else
		MemMove(keyVal, bufP + HSSN_PTR_OFFESET, 8); // copy from 5th to 12th location of 12 digit Ser Num.
									  // they are more likely to be different.
		
	return;
	
} // getROMID

/*
 * Encrypt.c
 */
