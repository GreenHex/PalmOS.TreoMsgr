/*
 * StrUtils.h
 */

#ifndef __STRUTILS_H__
#define __STRUTILS_H__

#include <PalmOS.h>
#include <PalmTypes.h>

#include "Sections.h"
#include "Global.h"
#include "Prefs.h"
#include "Win.h"

#define DATE_SEP_DOT		"."
#define DATE_SEP_COMMA		","
#define DATE_SEP_SLASH		"/"
#define DATE_SEP_DASH		"-"
#define DATE_SEP_SPACE		" "

// Prototypes
extern void 				GetStrDateTime(UInt32 TimeSecs, Char* dateStr, Char* timeStr) EXTRA_SECTION_ONE;
extern Boolean 				GetValidSubString(Char* inStr, Char* outStr, UInt16 outStrSize, Char* startToken, Char* endToken);
extern Char*				LTrim(Char* s);
extern Char*				RTrim(Char* s);
extern Char* 				MakeNum(unsigned char *s);
extern Char* 				ReplaceChar(unsigned char *s, char orgChr, char newChr);
extern Char*				ScrubAddress(Char* s);
extern Char* 				FormatDate(Char* dateStr) EXTRA_SECTION_ONE;
extern Char* 				FormatTime(Char* timeStr) EXTRA_SECTION_ONE;

#endif /* __STRUTILS_H__ */
/*
 * StrUtils.h
 */
