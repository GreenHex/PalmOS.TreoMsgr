/*
 * SMSUtils.c
 */
 
#include "SMSUtils.h"

// Prototypes
static Err 				SendOneMessage(Message_t* msgP, ProgressPtr pProgress, Int16 retryCount, Char* errStr);

/*
 * SplitGroupAndSendMessage
 */
Err SplitGroupAndSendMessage(AppUSPrefs_t* usPrefsP, ProgressPtr pProgress, Int16 retryCount)
{
	Err				error = errNone;
	Err				errSend = errNone;
	UInt16			numValidAddresses = 0;
	UInt16			numSentMessages = 0;	
  	Groups_t		group;
	UInt16			grpIdx = dmMaxRecordIndex;
  	Char			numbersStr[LEN_PhoneNumbers]; /* temp string */
  	Char			numbersList[LEN_PhoneNumbers];
  	Char			errStr[LEN_ErrorString];
	Char			number[LEN_PhoneNum];
	Char*			pStartLocation = NULL; 
	Char*			pEndLocation = NULL;
	UInt16			inStrLen = 0; 
	UInt16			copyStrLength = 0;
	
	MemSet(&group, sizeof(Groups_t), 0);

	StrCopy(numbersStr, usPrefsP->msg.Numbers);
	
	if (FindGroup(usPrefsP->msg.Numbers, &grpIdx, &group))
	{
		StrCopy(numbersList, group.Numbers);		
	}
	else
	{
		StrCopy(numbersList, usPrefsP->msg.Numbers); /* save the list */
	}	

	ReplaceChar(numbersList, ';', ',');
	LTrim(numbersList);
	
	pStartLocation = numbersList;
	inStrLen = StrLen(numbersList);
	
	while (pStartLocation < ((numbersList) + inStrLen))
	{
		pEndLocation = StrStr(pStartLocation, SEPARATOR_STR);
		
		if (!pEndLocation)
			pEndLocation = pStartLocation + StrLen(pStartLocation);
	
		copyStrLength = pEndLocation - pStartLocation;

		if (copyStrLength > 3)
		{
			StrNCopy(number, pStartLocation, copyStrLength);
			number[copyStrLength] = chrNull;	
			
			ScrubAddress(number);
			
			if (StrLen(number))
			{				
				++numValidAddresses;

				StrCopy(usPrefsP->msg.Numbers, number);
						
				errSend = SendOneMessage(&(usPrefsP->msg), pProgress, retryCount, errStr);
					
				error |= errSend;
					
				MsgLog(usPrefsP, errStr); 	
				
				if (errSend == errNone)
				{
					++numSentMessages;
				}
			}
		}

		pStartLocation = pEndLocation + StrLen(SEPARATOR_STR);		
	}

  	if ((pProgress) && (numValidAddresses > 1))
	{
		StrPrintF(errStr, "%i of %i messages sent.", numSentMessages, numValidAddresses);
		PrgUpdateDialog(pProgress, 0, 3, errStr, true);
		PrgHandleEvent(pProgress, NULL);
		
		StrNCopy(usPrefsP->msg.DisplayName, numbersStr, (LEN_CallerID - 1));
		usPrefsP->msg.DisplayName[LEN_CallerID - 1] = chrNull;
		StrCopy(usPrefsP->msg.Numbers, numbersList);
		StrPrintF(errStr, "%i of %i Messages Sent", numSentMessages, numValidAddresses);
		MsgLog(usPrefsP, errStr); 
	}

	return (error);
	
} // SplitGroupAndSendMessage

/*
 * SendOneMessage
 * msgP->Numbers should contain only ONE address
 */
static Err SendOneMessage(Message_t* msgP, ProgressPtr pProgress, Int16 retryCount, Char* errStr)
{
	Err					error = phnErrUnknownError;
	Char				errorStr[LEN_ErrorString] = ERR_STR_SMS_ERR;
	UInt32 				msgID = 0;
	PhnAddressList 		addList;
  	PhnAddressHandle 	hAddress;
  	UInt16				PhoneLibRefNum = 0;
  	DmOpenRef 			smsRefNum = 0;      // CDMA???
//  	UInt32				msgFlags = 0;
  	AppStats_t			stats;
//  	EventType			event;
//  	SMSMessageStatus	status = kNone;
  	
  	if (pProgress)
	{
		PrgUpdateDialog(pProgress, 0, 2, msgP->DisplayName, true);
		PrgHandleEvent(pProgress, NULL);
	}
	
	if (HsGetPhoneLibrary(&PhoneLibRefNum))
		return false;

	if (PhnLibOpen(PhoneLibRefNum))
		return false;
		
	if (PhnLibModulePowered(PhoneLibRefNum) != phnPowerOn)
	{
		StrCopy(errorStr, ERR_STR_PHN_OFF);
	  	goto CloseAndRelease;
	}
	
	if (!PhnLibRegistered(PhoneLibRefNum))
	{
		StrCopy(errorStr, ERR_STR_NETWORK_ERR);
	  	goto CloseAndRelease;
	}
	
	if (!IsPhoneGSM())
	{
		smsRefNum = PhnLibGetDBRef(PhoneLibRefNum);    // CDMA???
	}
	
	msgID = PhnLibNewMessage(PhoneLibRefNum, kMTOutgoing);
  	if (!msgID)
  	{
  		StrCopy(errorStr, ERR_STR_SMS_CREATE_ERR);
		goto SendMessage_CloseAndRelease;
  	}
  	
  	PhnLibSetOwner(PhoneLibRefNum, msgID, appFileCreator);
	PhnLibSetDate( PhoneLibRefNum, msgID, TimGetSeconds());
  	PhnLibSetText(PhoneLibRefNum, msgID, msgP->Message, (short)StrLen(msgP->Message));
/*
  	PhnLibGetFlags(PhoneLibRefNum, msgID, &msgFlags);
	msgFlags |= kAutoDelete;
  	PhnLibSetFlags(PhoneLibRefNum, msgID, msgFlags);	
*/
	// fill in the address
  	addList = PhnLibNewAddressList(PhoneLibRefNum);
  	if (!addList)
  	{
  		StrCopy(errorStr, ERR_STR_ADDRESS_LIST_ERR);
		goto SendMessage_CloseAndRelease;
  	}
  	
	hAddress = PhnLibNewAddress(PhoneLibRefNum, msgP->Numbers, phnLibUnknownID);
	
	if (!hAddress)
	{		
		StrCopy(errorStr, ERR_STR_ADDRESS_ERR);
		goto AddressErr_FreeAndRelease;				
	}

	if ((error = PhnLibAddAddress(PhoneLibRefNum, addList, hAddress)))
	{
		StrCopy(errorStr, ERR_STR_INVALID_ADDRESS_ERR);
		goto AddressErr_FreeAndRelease;
	}
	
	MemHandleFree(hAddress);
		
  	if ((error = PhnLibSetAddresses(PhoneLibRefNum, msgID, addList)))
	{
		StrCopy(errorStr, ERR_STR_ADDRESS_LIST_ERR);
		goto AddressErr_FreeAndRelease;				
	}
  			
	if (pProgress)
	{
		if (!CallerLookup(msgP->Numbers, ID_DISPLAY_FORMAT_STRING, msgP->DisplayName, LEN_CallerID))
		{
			StrNCopy(msgP->DisplayName, msgP->Numbers, (LEN_CallerID - 1));
			msgP->DisplayName[LEN_CallerID - 1] = chrNull;
		}
		PrgUpdateDialog(pProgress, 0, 0, msgP->DisplayName, true);
		PrgHandleEvent(pProgress, NULL);					
	}
	
	do
	{
//		writePhnStatus(kNone);
				
		error = PhnLibSendMessage(PhoneLibRefNum, msgID, true);

/*
		do
		{
			EvtGetEvent(&event, evtWaitForever);
			
			if (SysHandleEvent(&event))
				continue;
				
			PhnLibGetStatus(PhoneLibRefNum, msgID, &status);
			
			if (pProgress)
			{
				PrgUpdateDialog(pProgress, 0, 0, "Waiting!", true);
				PrgHandleEvent(pProgress, NULL);					
			}
		
		} while (status != kSent);

		do
		{
			EvtGetEvent(&event, evtWaitForever);
			
			if (SysHandleEvent(&event))
				continue;
				
			if (pProgress)
			{
				PrgUpdateDialog(pProgress, 0, 0, "Something wrong!", true);
				PrgHandleEvent(pProgress, NULL);					
			}
		}
		while (readPhnStatus() != kSent);
*/

	}
	while ((error != errNone) && (retryCount--));
	
  	if (error)
  	{
		StrCopy(errorStr, ERR_STR_SMS_ERR);	
	}
	else
	{
		StrCopy(errorStr, ERR_STR_SMS_SENT);
	}
/*
	if ((!error) && (PhnLibGetStatus(PhoneLibRefNum, msgID, &status) == errNone) && (status == kSent))
	{		
		PhnLibDeleteMessage(PhoneLibRefNum, msgID, false);
	}
*/	
AddressErr_FreeAndRelease:

	PhnLibDisposeAddressList(PhoneLibRefNum, addList);
	
SendMessage_CloseAndRelease:

	if (!IsPhoneGSM())
	{
		PhnLibReleaseDBRef(PhoneLibRefNum, smsRefNum); // CDMA???
	}
	
CloseAndRelease:
	
	(void)PhnLibClose(PhoneLibRefNum);
	
	if (pProgress)
	{
		if (error == errNone)
		{
			PrgUpdateDialog(pProgress, 0, 1, msgP->DisplayName, true); // it's done	
		}
		else
		{
			char 		errStr[8];
  		
	  		StrPrintF(errStr, "0x%x", error);
	  		
	  		StrCat(errorStr, " [");
			StrCat(errorStr, errStr);
			StrCat(errorStr, "]");
	
			PrgUpdateDialog(pProgress, 0, 4, errorStr, true); // some error
		}
		PrgHandleEvent(pProgress, NULL);		
	}
	
	readStats(&stats);
	if (error)
	{
		++(stats.numFailed);		
	}
	else
	{
		++(stats.numSent);		
	}
	writeStats(&stats);
	
	if ((errStr) && (errorStr))
		StrCopy(errStr, errorStr);
	
	return (error);
	
} // SendOneMessage

/*
 * SendMsgCallback
 */
Boolean SendMsgCallback(PrgCallbackDataPtr pCallbackData)
{	
	pCallbackData->bitmapId = MESSAGING_BITMAP;
	pCallbackData->canceled = true;
		
	if (pCallbackData->stage == 2)
	{
		StrCopy(pCallbackData->textP, "Configuring SMS...");
	}
	else if (pCallbackData->stage == 0)
	{
		StrCopy(pCallbackData->textP, "Sending message...\n");
		StrCat(pCallbackData->textP, pCallbackData->message);
	}	
	else if (pCallbackData->stage == 1)
	{	
		StrCopy(pCallbackData->textP, "Message sent.\n");
		StrCat(pCallbackData->textP, pCallbackData->message);
		pCallbackData->delay = true;
	}
	else if (pCallbackData->stage == 3)
	{
		StrCopy(pCallbackData->textP, "Messaging report:\n");
		StrCat(pCallbackData->textP, pCallbackData->message);
		pCallbackData->delay = true;
	}
	else if (pCallbackData->stage > 3)
	{
		StrCopy(pCallbackData->textP, "Message not sent.\n");
		StrCat(pCallbackData->textP, pCallbackData->message);
	}

	pCallbackData->textChanged = true;

	return true; 

} // SendMsgCallback

/*
 * SMSUtils.c
 */
