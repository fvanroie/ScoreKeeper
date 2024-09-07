#include <Processes.h>				//OpenDocument Stuff
#include <AppleEvents.h>			//OpenDocument Stuff

#include "utilproc.h"				//Some utility procedures

Boolean cmpStrs(Str255 s1, Str255 s2, int cmpType)
{
	short				i;
	
	if ((*s1 != *s2) && (cmpType == 1)) return false;		// kExactComp
	
	i = 0;
	while ((i < *s1) && (i < *s2)) {
		i++;
		if (s1[i] != s2[i]) return false;
	}
	return true;
}

void cleanUp(Str255 s1, Str255 s2)
{
	unsigned char	i, j;
		
	i = 1; j = 0;
	while (i < *s1) {
		if ((s1[i] != ' ') && (s1[i] != '-')) break; i++; 
	}
	i--;
	while (i < *s1) {
		  i++; j++; s2[j] = s1[i];
	}
	while (j > 0) {
		if ((s2[j] != ' ') && (s2[j] != '-')) break; j--; 
	}

	s2[0] = j;
}

short chrInStr(unsigned char c, Str255 s)
{
	short			i;
	
	for (i=1; i<= *s; i++) {
		if ((unsigned char)(s[i]) == c) return i;
	}
	return 0;
}
// given an application's serial number and a document, 
// SendOpenDocumentEventToProcess passes 
// the application an OpenDocuments event for the document

OSErr SendOpenDocumentEventToProcess(ProcessSerialNumber *targetPSN,
	const FSSpec * documentFSSpecPtr)
{
	OSErr retCode;
	AppleEvent theAppleEvent, theReplyEvent;
	AEDesc targetAddrDesc, docDesc;
	AEDescList docDescList;
	AliasHandle docAlias;
	
	// to simplify cleanup, ensure that handles are nil to start
	theAppleEvent.dataHandle = nil;
	docDescList.dataHandle = nil;
	docDesc.dataHandle = nil;
	docAlias = nil;
	
	// create an address descriptor based on the serial number of
	// the target process
	
	retCode = AECreateDesc(typeProcessSerialNumber, (Ptr) targetPSN,
		sizeof(ProcessSerialNumber), &targetAddrDesc);
	if (retCode != noErr) goto Bail;
	
	// make a descriptor list containing just a descriptor with an
	// alias to the document
	
	retCode = AECreateList(nil, 0, false, &docDescList);
	if (retCode != noErr) goto Bail;
	
	retCode = NewAlias(nil, documentFSSpecPtr, &docAlias);
	if (retCode != noErr) goto Bail;
	
	HLock((Handle) docAlias);
	retCode = AECreateDesc(typeAlias, (Ptr) *docAlias, 
		GetHandleSize((Handle) docAlias), &docDesc);
	HUnlock((Handle) docAlias);
	if (retCode != noErr) goto Bail;
	
	retCode = AEPutDesc(&docDescList, 0, &docDesc);
	if (retCode != noErr) goto Bail;
	
	// now make the 'odoc' AppleEvent descriptor and insert the 
	// document descriptor list as the direct object
	
	retCode = AECreateAppleEvent(kCoreEventClass, kAEOpenDocuments,
		&targetAddrDesc, kAutoGenerateReturnID, kAnyTransactionID,
		&theAppleEvent);
	if (retCode != noErr) goto Bail;
	
	retCode = AEPutParamDesc(&theAppleEvent, keyDirectObject, &docDescList);
	if (retCode != noErr) goto Bail;
	
	// finally, send the Apple event
	retCode = AESend(&theAppleEvent, &theReplyEvent, kAENoReply, 
		kAEHighPriority, kAEDefaultTimeout, nil, nil);
	
Bail:
	// dispose of everything that was allocated
	
	if (theAppleEvent.dataHandle != nil)  (void) AEDisposeDesc(&theAppleEvent);
	if (docDescList.dataHandle != nil)  (void) AEDisposeDesc(&docDescList);
	if (docDesc.dataHandle != nil)  (void) AEDisposeDesc(&docDesc);
	if (docAlias != nil)  DisposeHandle((Handle) docAlias);
	
	return retCode;
}

// OpenSpecifiedDocument searches to see if the application which
// created the document is already running.  If so, it sends
// an OpenSpecifiedDocuments Apple event to the target application
// (remember that, because of puppet strings, this works even
// if the target application is not Apple event-aware.)

OSErr OpenSpecifiedDocument(const FSSpec * documentFSSpecPtr)
{
	OSErr retCode;
	ProcessSerialNumber currPSN;
	ProcessInfoRec currProcessInfo;
	FSSpec applicationSpec;
	FInfo documentFInfo;
	Boolean foundRunningProcessFlag;
	
	// verify the document file exists and get its creator type
	
	retCode = FSpGetFInfo(documentFSSpecPtr, &documentFInfo);
	if (retCode != noErr) {
		goto Bail;
	}
	
	// check the current processes to see if the creator app is already
	// running, and get its process serial number (as currPSN)
	
	currPSN.lowLongOfPSN = kNoProcess;
	currPSN.highLongOfPSN = 0;
	
	currProcessInfo.processInfoLength = sizeof(ProcessInfoRec);
	currProcessInfo.processName = nil;
	currProcessInfo.processAppSpec = &applicationSpec;
	
	foundRunningProcessFlag = false;
	while (GetNextProcess(&currPSN) == noErr) {
		if (GetProcessInformation(&currPSN, &currProcessInfo) == noErr) {
			if (currProcessInfo.processSignature == documentFInfo.fdCreator) {
				foundRunningProcessFlag = true;
				break;
			}
		}
	}
	
	// if the creator is running, send it an OpenDocuments Apple event
	// since there is no need to launch it
	
	if (foundRunningProcessFlag)
		retCode = SendOpenDocumentEventToProcess(&currPSN, documentFSSpecPtr);
	
	// else if the creator is not running, find it on disk and launch
	// it with the OpenDocuments event included as a part of the
	// launch parameters
	
	/*else {
		retCode = FindApplicationFromDocument(documentFSSpecPtr, &applicationSpec);
		
		if (retCode == noErr)
		
			retCode = LaunchApplicationWithDocument(&applicationSpec,
				documentFSSpecPtr);
	}*/
	
Bail:
	return retCode;
}

OSErr quitAvara()
{
	OSErr retCode;
	ProcessSerialNumber currPSN;
	ProcessInfoRec currProcessInfo;
	FSSpec applicationSpec;
	FInfo documentFInfo;
	Boolean foundRunningProcessFlag;
	AppleEvent theAppleEvent, theReplyEvent;
	AEDesc targetAddrDesc;
	
	// to simplify cleanup, ensure that handles are nil to start
	theAppleEvent.dataHandle = nil;
		
	// check the current processes to see if the creator app is already
	// running, and get its process serial number (as currPSN)
	
	currPSN.lowLongOfPSN = kNoProcess;
	currPSN.highLongOfPSN = 0;
	
	currProcessInfo.processInfoLength = sizeof(ProcessInfoRec);
	currProcessInfo.processName = nil;
	currProcessInfo.processAppSpec = &applicationSpec;
	
	foundRunningProcessFlag = false;
	while (GetNextProcess(&currPSN) == noErr) {
		if (GetProcessInformation(&currPSN, &currProcessInfo) == noErr) {
			if (currProcessInfo.processSignature == 'AVAR') {
				foundRunningProcessFlag = true;
				break;
			}
		}
	}
	
	// if the creator is running, send it an OpenDocuments Apple event
	// since there is no need to launch it
	
	if (!foundRunningProcessFlag) goto Bail;
			
	// create an address descriptor based on the serial number of
	// the target process
	
	retCode = AECreateDesc(typeProcessSerialNumber, (Ptr) &currPSN,
		sizeof(ProcessSerialNumber), &targetAddrDesc);
	if (retCode != noErr) goto Bail;
	
	// now make the 'odoc' AppleEvent descriptor and insert the 
	// document descriptor list as the direct object
	
	retCode = AECreateAppleEvent(kCoreEventClass, kAEQuitApplication,
		&targetAddrDesc, kAutoGenerateReturnID, kAnyTransactionID,
		&theAppleEvent);
	if (retCode != noErr) goto Bail;
	
	// finally, send the Apple event
	retCode = AESend(&theAppleEvent, &theReplyEvent, kAENoReply, 
		kAEHighPriority, kAEDefaultTimeout, nil, nil);
	
Bail:
	// dispose of everything that was allocated
	
	if (theAppleEvent.dataHandle != nil)  (void) AEDisposeDesc(&theAppleEvent);	
	return retCode;
}