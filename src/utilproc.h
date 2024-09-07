
// String procs
Boolean cmpStrs(Str255 s1, Str255 s2, int cmpType);
void cleanUp(Str255 s1, Str255 s2);
short chrInStr(unsigned char c, Str255 s);

// AppleEvent procs
OSErr OpenSpecifiedDocument(const FSSpec * documentFSSpecPtr);
OSErr SendOpenDocumentEventToProcess(ProcessSerialNumber *targetPSN,const FSSpec * documentFSSpecPtr);
OSErr quitAvara();
