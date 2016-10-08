/*
 * Bitmap.c
 */

#include <Hs.h>
#include <HsPhone.h>
#include <HsNav.h>
#include <HsExt.h>
#include <palmOneResources.h>
#include <PalmTypes.h>

#include "Sections.h"

// Prototypes
BitmapType* 			getBitmap(DmResID resID, Coord* widthP, Coord* heightP, MemHandle* bmpHP) EXTRA_SECTION_ONE;
void 					releaseBitmap(MemHandle* bmpHP) EXTRA_SECTION_ONE;

/*
 * getBitmap
 */
BitmapType* getBitmap(DmResID resID, Coord* widthP, Coord* heightP, MemHandle* bmpHP)
{
	BitmapType* 		pBmp = NULL;
	
	*bmpHP = DmGetResource(bitmapRsc, resID); 
	pBmp = MemHandleLock(*bmpHP);

	BmpGetDimensions(pBmp, widthP, heightP, NULL);
		
	return pBmp;
	
} // getBitmap

/*
 * releaseBitmap
 */
void releaseBitmap(MemHandle* bmpHP)
{
	MemHandleUnlock(*bmpHP);
	DmReleaseResource(*bmpHP);
	
} // releaseBitmmap

/*
 * Bitmap.c
 */