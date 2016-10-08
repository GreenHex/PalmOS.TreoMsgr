/*
 * StrUtils.c
 */

#include "StrUtils.h"

// Prototypes
static Char* 						StripTrailingLeadingWhiteSpaces(Char* s);
 
/*
 * GetStrDateTime
 */
void GetStrDateTime(UInt32 TimeSecs, Char* dateStr, Char* timeStr)
{
	DateTimeType 		dtNow;
	
	TimSecondsToDateTime(TimeSecs, &dtNow);
	
	// TimeToAscii(dtNow.hour, dtNow.minute, /* PrefGetPreference(prefTimeFormat) */ tfColon24h , timeStr);
	// DateToAscii(dtNow.month, dtNow.day, dtNow.year, /* PrefGetPreference(prefDateFormat) */ dfDMYWithSlashes, dateStr);
	
	StrPrintF(timeStr, "%02d:%02d", dtNow.hour, dtNow.minute);
	StrPrintF(dateStr, "%02d/%02d/%02d", dtNow.day, dtNow.month, dtNow.year);
		
} // GetStrDateTime

/*
 * GetValidSubString
 */
Boolean GetValidSubString(Char* inStr, Char* outStr, UInt16 outStrSize, Char* startToken, Char* endToken)
{	
	Boolean		retVal = false;
	Char*		pStartLocation = NULL;
	Char*		pEndLocation = NULL;
	UInt16		copyStrLength = 0;
	
	if (!inStr)
		return retVal;
	
	pStartLocation = StrStr(inStr, startToken);
	pEndLocation = StrStr(inStr, endToken);
	
	if (!pEndLocation)
		pEndLocation = inStr + StrLen(inStr);
	
	if ((pStartLocation) && (pEndLocation))
	{
		if ((outStr) && (outStrSize))
		{
			copyStrLength = (((pEndLocation - pStartLocation) - StrLen(startToken) < (outStrSize - 1))
						? (pEndLocation - pStartLocation) - StrLen(startToken) : (outStrSize - 1));
		
			StrNCopy(outStr, (pStartLocation + StrLen(startToken)), copyStrLength);
			outStr[copyStrLength] = chrNull;
			
			StripTrailingLeadingWhiteSpaces(outStr);
		}
		retVal = true;
	}
	
	return (retVal);

} // GetValidSubString

/*
 * StripTrailingLeadingWhiteSpaces
 */
static Char* StripTrailingLeadingWhiteSpaces(Char* s)
{
	Char*	d = s;
	
	while ((*s) && (TxtCharIsSpace(*s++)))
		;
	
	--s; // backup...
		
	while ((*d++ = *s++))
		;
	
	--d;
	
	while (TxtCharIsSpace(*(--d)))
		;
		
	*(++d) = chrNull;
	
	return (s);
		
} // StripTrailingLeadingWhiteSpaces

/*
 * LTrim
 */
Char* LTrim(Char* s)
{
	Char*		d = s;
	UInt16		len = 0;
	
	while (TxtCharIsSpace(*d++));
		;
		
	--d;
	
	len = StrLen(d);
	
	MemMove(s, d, len);
	s[len] = chrNull;
	
	return (s);
		
} // LTrim

/*
 * RTrim
 */
Char* RTrim(Char* s)
{
	return (s);
	
} // RTrim

/*
 * MakeNum
 */
Char* MakeNum(unsigned char *s)
{
	unsigned char c, *d = s;

	if (*s == '+') // first character is a "+"
		*d++ = *s++;
	
	while ((c = *s++))
		if (TxtCharIsDigit(c))
			*d++ = c;
					
	*d = 0;

	return (s);
	
} // MakeNum

/*
 * ReplaceChar
 */
Char* ReplaceChar(unsigned char *s, char orgChr, char newChr)
{
	unsigned char c, *d = s;
	
	while ((c = *s++))
		if (c == orgChr)
			*d++ = newChr;
		else
			*d++ = c;
					
	*d = 0;
	
	return (s);
	
} // ReplaceChar

/*
 * ScrubAddress
 */
Char* ScrubAddress(Char* s)
{
	if (s)
	{
		LTrim(s);
		RTrim(s);
		
		if (!StrChr(s, '@'))
		{
			MakeNum(s);	
		}
	}
	
	return (s);
	
} // ScrubAddress

/*
 * FormatDate
 */
Char* FormatDate(Char* dateStr)
{
	DateFormatType		datePref = (DateFormatType) PrefGetPreference(prefDateFormat);
	Char				separator[2];
	Char				date[3];
	Char				month[3];
	Char				year[5];

	if (!dateStr)
		return (dateStr);

	MemMove(date, dateStr, 2);
	date[2] = chrNull;
	MemMove(month, dateStr + 3, 2);
	month[2] = chrNull;
	MemMove(year, dateStr + 6, 4);
	year[4] = chrNull;
/*		
#define DATE_SEP_DOT		"."
#define DATE_SEP_COMMA		","
#define DATE_SEP_SLASH		"/"
#define DATE_SEP_DASH		"-"
#define DATE_SEP_SPACE		" "
*/
	switch (datePref)
	{
		case dfMDYWithSlashes: 		/* 12/31/95 */
		
			StrCopy(separator, DATE_SEP_SLASH);

			StrCopy(dateStr, month);
			StrCat(dateStr, separator);
			StrCat(dateStr, date);
			StrCat(dateStr, separator);
			StrCat(dateStr, year);
			
			break;
		
		case dfDMYWithSlashes: 		/* 31/12/95 */ 
			
			StrCopy(separator, DATE_SEP_SLASH);

			StrCopy(dateStr, date);
			StrCat(dateStr, separator);
			StrCat(dateStr, month);
			StrCat(dateStr, separator);
			StrCat(dateStr, year);
			
			break;			

		case dfDMYWithDots: 		/* 31.12.95 */
		
			StrCopy(separator, DATE_SEP_DOT);

			StrCopy(dateStr, date);
			StrCat(dateStr, separator);
			StrCat(dateStr, month);
			StrCat(dateStr, separator);
			StrCat(dateStr, year);
			
			break;
			
		case dfDMYWithDashes: 		/* 31-12-95 */

			StrCopy(separator, DATE_SEP_DASH);

			StrCopy(dateStr, date);
			StrCat(dateStr, separator);
			StrCat(dateStr, month);
			StrCat(dateStr, separator);
			StrCat(dateStr, year);

			break;
			
		case dfYMDWithSlashes: 		/* 95/12/31 */ 

			StrCopy(separator, DATE_SEP_SLASH);

			StrCopy(dateStr, year);
			StrCat(dateStr, separator);
			StrCat(dateStr, month);
			StrCat(dateStr, separator);
			StrCat(dateStr, date);

			break;
			
		case dfYMDWithDots: 		/* 95.12.31 */

			StrCopy(separator, DATE_SEP_DOT);

			StrCopy(dateStr, year);
			StrCat(dateStr, separator);
			StrCat(dateStr, month);
			StrCat(dateStr, separator);
			StrCat(dateStr, date);
		
			break;
		
		case dfYMDWithDashes: 		/* 95-12-31 */

			StrCopy(separator, DATE_SEP_DASH);

			StrCopy(dateStr, year);
			StrCat(dateStr, separator);
			StrCat(dateStr, month);
			StrCat(dateStr, separator);
			StrCat(dateStr, date);

			break;
			
//		case dfMDYLongWithComma: 	/* Dec 31, 1995 */
//			break;
			
//		case dfDMYLong: 			/* 31 Dec 1995 */
//			break;			

//		case dfDMYLongWithDot: 		/* 31. Dec 1995 */
//			break;
			
//		case dfDMYLongNoDay: 		/* Dec 1995 */
//			break;
			
//		case dfDMYLongWithComma: 	/* 31 Dec, 1995 */
//			break;
			
		case dfYMDLongWithDot: 		/* 1995.12.31 */

			StrCopy(separator, DATE_SEP_DOT);

			StrCopy(dateStr, year);
			StrCat(dateStr, separator);
			StrCat(dateStr, month);
			StrCat(dateStr, separator);
			StrCat(dateStr, date);			

			break;			

		case dfYMDLongWithSpace: 	/* 1995 Dec 31 */

			StrCopy(separator, DATE_SEP_SPACE);

			StrCopy(dateStr, year);
			StrCat(dateStr, separator);
			StrCat(dateStr, month);
			StrCat(dateStr, separator);
			StrCat(dateStr, date);			

			break;
					
//		case dfMYMed: 				/* Dec '95 */
//			break;
			
//		case dfMYMedNoPost: 		/* Dec 95 */
//			break;
			
		case dfMDYWithDashes: 		/* 12-31-95 */ 

			StrCopy(separator, DATE_SEP_DASH);
	
			StrCat(dateStr, month);
			StrCat(dateStr, separator);
			StrCat(dateStr, date);
			StrCat(dateStr, separator);
			StrCopy(dateStr, year);

			break;
			
		default:
			break;
	} 
	
	return (dateStr);	
	
} // FormatDate

/*
 * FormatTime
 */
Char* FormatTime(Char* timeStr)
{
	TimeFormatType		timePref = (TimeFormatType) PrefGetPreference(prefTimeFormat);
	Char				hourStr[maxStrIToALen];
	Int32				hour = 0;
	Char				AmPmStr[2];
	
	if (!timeStr)
		return (timeStr);
	
	MemMove(hourStr, timeStr, 2);
	hourStr[2] = chrNull;
	
	hour = StrAToI(hourStr);
	
	if (hour >= 12)
	{
		StrCopy(AmPmStr, "p");
	}
	else
	{
		StrCopy(AmPmStr, "a");	
	}
		
	hour = (hour > 12) ? (hour - 12) : hour;
	
	StrPrintF(hourStr, "%02ld", hour);
		
	switch (timePref)
	{		
		case tfColon: 		/* 1:00 */
		case tfColonAMPM: 	/* 1:00 pm */

			MemMove(timeStr, hourStr, 2);
			StrCat(timeStr, AmPmStr);
	
			break;
			
//		case tfColon24h:	/* 13:00 */
//			break;
			
		case tfDot: 		/* 1.00 */
		case tfDotAMPM:		/* 1.00 pm */

			MemMove(timeStr, hourStr, 2);
			StrCat(timeStr, AmPmStr);
			timeStr[2] = '.';
		
			break;
			
		case tfDot24h:		/* 13.00 */

			timeStr[2] = '.';
			
			break;
								 
		case tfHoursAMPM:	/* 1 pm */

			MemMove(timeStr, hourStr, 2);
			StrCat(timeStr, AmPmStr);

			break;
			
//		case tfHours24h:	/* 13 */
//			break;
			
		case tfComma24h:	/* 13,00 */

			timeStr[2] = ',';
			
			break;
					
		default:
			break;
	}
			
	return (timeStr);
	
} // FormatTime

/*
 * StrUtils.c
 */