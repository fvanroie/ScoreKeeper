#include <A4Stuff.h>				//Plug-in environment
#include <folders.h>				//Folder locations
#include <Devices.h>				//Offscreen stuff
#include <QDOffscreen.h>			//Offscreen stuff
#include <Speech.h>					//Speech Manager
#include <Palettes.h>				//Palette Manager

#include <Movies.h>					//QuickTime
#include <ImageCompression.h>

#include "MoreFilesExtras.h"		//More Files stuff

#include "AvaraScoreInterface.h"	//Avara plug-in stuff
#include "ConstantsAndTypes.h"		//My stuff

#include "UtilProc.h"				//Some utility procedures

#define	kExpireDay			31+19	// 19  February 1999
#define kExpireDate			(unsigned long)(2955916800+(unsigned long)((unsigned long)(kExpireDay-242+365+365)*24*60*60))

#define	kIsDemoVersion		false

short						gVerboseErrors = false;

AliasHandle					gLogAlias, gMovAlias;

SpeechChannel				gSpeechChan = NULL;

struct PreferencesType		UserPrefs;

Str255						players[6], longNames[6];

ControlHandle				PrefBtn, SummaryBtn, PopUpBtn, FavoritesBtn, RecentBtn,
							AboutScrollBar, PrefScrollBar;

short						startColumn = 0,
							noColumns = 8,
							noPlayers = 0, noTeams = 0,
							gSelectedPrefs = 0,
							currentPanel = kAboutPanel,
							gSummaryType = kTotalSummary,
							gPlugLevel = kDemo,
							BandWidth,
							minLT, maxLT;

short						gMovieCount, gMovieFrame;

short						gFavoritesCount = 0/*, geneva = 0*/;

long						playerPoints[10][9], teamPoints[10][9],	//[playernr][columnnr]
							ammoTime[10][3], ammoCounted[10][3], 
							ammoFired[10][3], ammoHit[10][3];		//[playernr][ammonr]
							
short						playerJoined[6], playerTeam[6], columnOrder[8],
							sortOrder[6], teamOrder[7], memberCount[7];
							
CIconHandle					ammoIcons[3],movieIcons[4];

WindowPtr					plugWindow = NULL, gameWindow = NULL;

GrafPtr						savePort;

Handle						oldResultsHandle;

Rect						aboutRect, panelRect, optionsRect, aboutPanelRect, PrefIconsRect;
PicHandle					aboutPict;
Boolean						levelBusy = false,
							plugVisible = false,
							gHasSpeechmanager = false,
							gHasQuickTime = false,
							gHasMultiChannel = false,
							gPlugFailure = false,
							gColorsOK = false;
							
CGrafPtr					gMoviePortPtr = nil;
Movie						gMovie = nil;
Track						gMovieTrack;
Media						gMovieMedia;
short						gMovieRefNum;
Handle						gCompressedData = nil;
Ptr							gCompressedDataPtr;
ImageDescriptionHandle		gImageDesc = nil;		

FSSpec						gPlugSpec, gFavFolderSpec;

/* patch stuff */

typedef pascal void (*SetPortProc)(GrafPtr port);
SetPortProc	gOldSetPortAddr;
pascal void setPortPatch(GrafPtr port);
void installSetPortPatch(void);
void unInstallSetPortPatch(void);

// ������������������������������������������������������������������ function prototypes
pascal void	main(ScoreInterfaceRecord *rec);

void analysePicture(ScoreInterfaceRecord *rec);

short findColumn(short whichColumn);
void addPlayerScore(ScoreInterfaceRecord *rec,short theColumn, long plus);
void addPlayerStats(ScoreInterfaceRecord *rec,short theColumn);
void sortScores(void);

void getBandWidth(void);	// Currently not used
void getLatency(void);		// Currently not used

void closePlugIn(ScoreInterfaceRecord *rec);
void initPlugIn(ScoreInterfaceRecord *rec);
void findGameWindow(void);
void drawButtons(ScoreInterfaceRecord *rec);
void drawHeaderRect(struct Rect *theRect);
void drawWindow(ScoreInterfaceRecord *rec);
void showControls(Boolean show);
void switchPanel(ScoreInterfaceRecord *rec);
void drawPanel(ScoreInterfaceRecord *rec);
void drawSharedInterface(void);
void drawSummaryPanel(void);
void drawTitleBar(void);
void drawScore(long *score,long plus);
void drawPercent(short player,short column,short line);
void addPlayerSummary(ScoreInterfaceRecord *rec,short theColumn,long plus);
void addTeamSummary(ScoreInterfaceRecord *rec,short theColumn,long plus);
void drawPlayerPanel();
void drawTeamPanel();
void drawPlayerStatsPanel();
void drawGameStatsPanel();
void reSortTeams(void);
void drawAboutPanel(int howMuch);
void drawShadowRect(Rect theRect);
void doControls(ScoreInterfaceRecord *rec,ControlHandle theCntrl);
void doInContent(ScoreInterfaceRecord *rec);
int trackTitle(Point clickPt,Rect myRect);
void drawCenteredString(Str255 myStr,short x,short y);
void stringTrunc(Str255 theString,short maxLength);
void switchColumns(short col1, short col2);
void changePort(void);
void updateGameScores(void);
void doAboutScrollBar(ControlPartCode,WindowPtr,ControlHandle,Point);
pascal void aboutScrollboxActionProcedure(ControlHandle,ControlPartCode);
void doMoveScrollBox(ControlHandle,SInt16);

void doPanelError(int errorType, int errorNum);
void doErrorAlert(int resID,int itemNum);
void doPrefErrorAlert(int resID,int itemNum);
short doCancelOKAlert(int resID,int itemNum);

void doPreferences(void);
Boolean doPreferencesDialog(short thePanel);
void  doUpdate(EventRecord *eventRecPtr);
pascal void  preferencesUserItems(DialogPtr dialogPtr,SInt16 theItem);
pascal Boolean  eventFilter(DialogPtr dialogPtr,EventRecord *eventRecPtr,SInt16 *itemHit);
void  doInPrefTopics(Point clickPt,short howMuch,struct PreferencesType Prefs,DialogPtr theDialog);
void  doPrefScrollBar(ControlPartCode,WindowPtr,ControlHandle,Point);
void drawPreferencesPanel(int howMuch,int selPrefs);
pascal void  prefScrollBarActionProcedure(ControlHandle controlHdl,ControlPartCode partCode);
void switchPreferencesPanel(DialogPtr theDialog,short newPanel,struct PreferencesType *Prefs);
void defaultPreferences(struct PreferencesType *Prefs,short thePanel);

Boolean askTeamColor(DialogPtr theDialog,short itemHit,short team,struct PreferencesType *TempPrefs);
void adjustTeamColor(DialogPtr theDialog,short itemHit,short team,struct PreferencesType *TempPrefs);
void updateColorPalette(struct PreferencesType *Prefs);
void resetColorPalette(void);
void calcLightColor (RGBColor *theColor,float percent);
void setColor(RGBColor *C, short R, short G, short B);

void checkSoundEnv(void);
void removeClan(Str255 theString);
void speakStringAsync(StringPtr theStrPtr);
void speakTestString(struct PreferencesType *Prefs);
void speakKill(ScoreInterfaceRecord *rec);
void insertVoicesMenu(struct PreferencesType *Prefs);
void changeVoice(struct PreferencesType *Prefs);
void openSpeechChannel(struct PreferencesType *Prefs);
void closeSpeechChannel(void);

Boolean ScoresNotZero(void);
void SelectLogFile(struct PreferencesType *Prefs);
short resolveLogAlias();
void SaveFixedWidthLog(ScoreInterfaceRecord *rec);
void SaveTabDelimitedLog(ScoreInterfaceRecord *rec);
void SaveDatabaseLog(ScoreInterfaceRecord *rec);

void setPlugLevel(void);
OSErr RemoveLicense();
OSErr OpenPref(short *refNum,Str255 filename,signed char perm,short fork);
OSErr FlushPrefVol(void);
void ReadPref(void);
void WritePref(void);
void doAliases(void);

long PopUpSelect(MenuHandle theMenu, Point popPt, short popupItem, Boolean useWFont, Boolean rightAlign);

void enumerateFolder(short vRefNum, long dirID);
void processAFile(CInfoPBRec *filePB);

void doFavorites(void);
void clearFavorites(void);
void doAddFavorite(void);
void doOpenFavorite(short theItem);
short findFavoritesType(short theItem);

void clearRecent(void);
void doRecent(void);
void addRecentFile(short refNum);

void checkMovieEnv(void);
void selectMovieFile(struct PreferencesType *Prefs);
void openMovieFile(struct PreferencesType *Prefs);
void closeMyMovieFile(struct PreferencesType *Prefs);
void addFrame(long frameCount);
Boolean initMovie(void);
// ������������������������������������������������������������������ function prototypes

void addFrame(long frameCount)
{
	OSErr					theErr;
	CGrafPtr				windowPortPtr = NULL;
	GDHandle				deviceHdl;
		
	if(!(LockPixels(gMoviePortPtr->portPixMap))) {
		doErrorAlert(rMovieErrors,kMovQDError); UserPrefs.movEnabled = 0;
		return;
	}
	if (frameCount != 0) {		//skip on initialisation 
		theErr = CompressImage(gMoviePortPtr->portPixMap, &gMoviePortPtr->portRect, 
							   codecMinQuality, 'smc ', gImageDesc,gCompressedDataPtr);
		if (theErr != noErr) {
			doErrorAlert(rMovieErrors,kMovCompressError); UserPrefs.movEnabled = 0;
			return;
		}
		theErr = AddMediaSample(gMovieMedia, gCompressedData, 0, (**gImageDesc).dataSize, 
								frameCount, (SampleDescriptionHandle)gImageDesc,1,0,nil);
		if (theErr != noErr) {
			//doErrorAlert(rMovieErrors,kMovMediaError); UserPrefs.movEnabled = 0; // THE ERROR!!!
			return;
		}
	} // frameCount != 0
	
	//GetGWorld(&windowPortPtr,&deviceHdl);
	windowPortPtr = (CGrafPtr)gameWindow;
	if(!windowPortPtr) {
		doErrorAlert(rMovieErrors,kMovQDError); UserPrefs.movEnabled = 0;
		return;
	}
	if(!(LockPixels(windowPortPtr->portPixMap))) {
		doErrorAlert(rMovieErrors,kMovQDError); UserPrefs.movEnabled = 0;
		return;
	}

	CopyBits(&((GrafPtr) windowPortPtr)->portBits,&((GrafPtr) gMoviePortPtr)->portBits,
			 &windowPortPtr->portRect,&gMoviePortPtr->portRect,srcCopy,NULL);

	UnlockPixels(windowPortPtr->portPixMap);
	
	GetGWorld(&windowPortPtr,&deviceHdl);
	SetGWorld(gMoviePortPtr,NULL);
	
	if (kIsDemoVersion) {
		ForeColor(redColor);
		MoveTo(gMoviePortPtr->portRect.left,gMoviePortPtr->portRect.top);
		LineTo(gMoviePortPtr->portRect.right,gMoviePortPtr->portRect.bottom);
		MoveTo(gMoviePortPtr->portRect.left,gMoviePortPtr->portRect.bottom);
		LineTo(gMoviePortPtr->portRect.right,gMoviePortPtr->portRect.top);
	}
	
	SetGWorld(windowPortPtr,deviceHdl);
	UnlockPixels(gMoviePortPtr->portPixMap);
}

pascal void	main (ScoreInterfaceRecord *rec)
{
	ControlHandle			theCntrl;
	Rect					myRect;
	Point					clickPt;
	short					i, j;
	long					textSize;
	Str255					newLTStr, fragsStr;
	
	EnterCodeResource();

	if (rec->command != ksiInit) {
		if (gPlugFailure) { 
			ExitCodeResource();
			return;
		}
		if (!gColorsOK) { 
			updateColorPalette(&UserPrefs);
			gColorsOK = true;
		}
		SetHandleSize(rec->resultsHandle,0);
	}
		
	switch (rec->command) {
	 	case ksiInit: 
	 		initPlugIn(rec);
			break; // ksiInit
			
	 	case ksiClose:
	 		closePlugIn(rec);
			break; // ksiClose
		
		case ksiLevelLoaded:
			analysePicture(rec);
			break;
			
		case ksiLevelStarted:	//	A level was started (frame 0)
		
			if ((UserPrefs.movEnabled) && (UserPrefs.movFlag == kFileSelected)) {
				gMovieFrame = gMovieCount = 0;
				openMovieFile(&UserPrefs);		
				addFrame(0);
			}
			
			for (i=0; i<10; i++) {
				for (j=0; j<9; j++) {
					playerPoints[i][j] = 0;
					teamPoints[i][j] = 0;
				}
			} // for i
			for (i=0; i<6; i++) {
				playerJoined[i] = playerTeam[i] = 0;
				sortOrder[i] = teamOrder[i] = i;
				for (j=0; j<3; j++) {
					ammoFired[i][j] = 0;
					ammoHit[i][j] = 0;
					ammoTime[i][j] = 0;
					ammoCounted[i][j] = false;
				}
			} // for i
			minLT = 9;
			maxLT = 0;
			getLatency();
		case ksiLevelRestarted:	//	The game was resumed after a pause
			noPlayers = 0;
			for (i=1; i<7; i++) {
				memberCount[i] = 0;
			}
			getBandWidth();
			levelBusy = true;
			if (plugVisible) { 
				changePort();
				TextFace(normal);
				if (UserPrefs.gnlSwitch && (currentPanel != UserPrefs.gnlPanel)) {
					currentPanel = UserPrefs.gnlPanel;				//Switch panels automatically
					SetControlValue(PopUpBtn,currentPanel);
					switchPanel(rec);
				} 
				updateGameScores();
				if (currentPanel != kAboutPanel) drawSummaryPanel();
				SetPort(savePort);
			} //plugVisible
			installSetPortPatch();									//1.2b6 ok
			openSpeechChannel(&UserPrefs);
			break; // ksiLevelRestarted

		case ksiLevelEnded:		//	The game ends (for whatever reason)
			closeMyMovieFile(&UserPrefs);		
			UserPrefs.movEnabled = 0;		// One Movie at a time
			
			if ((UserPrefs.logEnabled) && ScoresNotZero()) {
				if (UserPrefs.logTime == kLogEachGame) {
					SelectLogFile(&UserPrefs);
				}
				if (UserPrefs.logFlag == kFileSelected) {
					switch (UserPrefs.logFormat) {
						case 1 :
							SaveFixedWidthLog(rec);
							break;
						case 2 :
							SaveTabDelimitedLog(rec);
							break;
						case 3 :
							SaveDatabaseLog(rec);
							break;
					}
				}
			}
		case ksiLevelPaused:	//	The game was paused
			closeSpeechChannel();
			unInstallSetPortPatch();					//1.2b6 ok
			levelBusy = false;
			GetPort(&savePort);
			SetPort(plugWindow);
			if (plugVisible) { 
				updateGameScores();
			} //plugVisible
			TextFace(bold);
			SetPort(savePort);
			break; // ksiLevelPaused, ksiLevelEnded

		case ksiPlayerIntro:
			if (rec->playerID == kCompPlayer) {
				ExitCodeResource();
				return;
			} // playerID == kCompPlayer  (computer)
			playerJoined[rec->playerID] = 1;
			playerTeam[rec->playerID] = rec->playerTeam;
			sortOrder[rec->playerID] = noPlayers++;
			memberCount[rec->playerTeam]++;

			noTeams = 0;
			for (i=1; i<=6; i++) noTeams += memberCount[rec->playerTeam]>0;

			textSize = *(rec->playerName) + 1;
			if (textSize > 63) textSize = 63;
			BlockMoveData (rec->playerName,&(players[rec->playerID]), textSize);
			BlockMoveData (rec->playerName,&(longNames[rec->playerID]), textSize);
			TextFont(geneva);     // Truncate long names
			TextSize(9);
			TextFace(normal);
			removeClan(players[rec->playerID]);
			stringTrunc(players[rec->playerID],102);

			if (plugVisible) { 
				changePort();
				TextFace(normal);
				updateGameScores();
				SetPort(savePort);
			} //plugVisible
			break; // ksiPlayerIntro

		case ksiScore:
			if (rec->playerID == kCompPlayer) {
				if (rec->scoreReason == ksiKillBonus) { speakKill(rec); }
				ExitCodeResource();
				return;
			}
			GetPort(&savePort);
			SetPort(plugWindow);
			switch (rec->scoreReason) {
				case ksiShotHit:
					addPlayerScore(rec,kShotColumn,rec->scorePoints);
					addPlayerSummary(rec,kShotColumn,rec->scorePoints);
					addTeamSummary(rec,kShotColumn,rec->scorePoints);
					addPlayerStats(rec,kShotColumn);
					break;
				case ksiGrenadeHit:
					addPlayerScore(rec,kGrenadeColumn,rec->scorePoints);
					addPlayerSummary(rec,kGrenadeColumn,rec->scorePoints);
					addTeamSummary(rec,kGrenadeColumn,rec->scorePoints);
					addPlayerStats(rec,kGrenadeColumn);
					break;
				case ksiMissileHit:
					addPlayerScore(rec,kMissileColumn,rec->scorePoints);
					addPlayerSummary(rec,kMissileColumn,rec->scorePoints);
					addTeamSummary(rec,kMissileColumn,rec->scorePoints);
					addPlayerStats(rec,kMissileColumn);
					break;
				case ksiKillBonus:
					if (rec->scorePoints > 0) { 
						addPlayerScore(rec,kKillColumn,1);
						addPlayerSummary(rec,kKillColumn,1);
						addTeamSummary(rec,kKillColumn,1);
					} // if score > 0
					speakKill(rec);
				case ksiExitBonus:
				case ksiGoodyBonus:
					addPlayerScore(rec,kBonusColumn,rec->scorePoints);
					addPlayerSummary(rec,kBonusColumn,rec->scorePoints);
					addTeamSummary(rec,kBonusColumn,rec->scorePoints);
					break;
				case ksiScoreGoal:
					if (rec->scorePoints > 0) { 
						addPlayerScore(rec,kGoalColumn,1);
						addPlayerSummary(rec,kGoalColumn,1);
						addTeamSummary(rec,kGoalColumn,1);
					}
				case ksiHoldBall:
					addPlayerScore(rec,kBallColumn,rec->scorePoints);
					addPlayerSummary(rec,kBallColumn,rec->scorePoints);
					addTeamSummary(rec,kBallColumn,rec->scorePoints);
					break;
				default:
					addPlayerScore(rec,kDamageColumn,rec->scorePoints);
					addPlayerSummary(rec,kDamageColumn,rec->scorePoints);
					addTeamSummary(rec,kDamageColumn,rec->scorePoints);
					break;
			} // switch scoreReason
			addPlayerScore(rec,kTotalColumn,rec->scorePoints);
			addPlayerSummary(rec,kTotalColumn,rec->scorePoints);
			addTeamSummary(rec,kTotalColumn,rec->scorePoints);
			//sortScores();
			SetPort(savePort);
			break; // ksiScore

		case ksiResultsUpdate:
			plugWindow = rec->resultsWindow;
			changePort();
			TextFace(bold);
			if (!plugVisible) {
				showControls(true);
				plugVisible = true;
			} // !plugVisible
			drawWindow(rec);
			SetPort(savePort);
			break; // ksiResultsUpdate
		
		case ksiResultsShow:		// Custom results page is now visible.
			plugWindow = rec->resultsWindow;
			break; // ksiResultsShow
			
		case ksiResultsHide:		// Custom results page needs to be hidden.
			showControls(false);
			plugWindow = rec->resultsWindow;
			changePort();
			plugVisible = false;
			SetPort(savePort);
			break; // ksiResultsHide
			
		case ksiResultsClick:		//	A mouse click was detected in the custom results page
			plugWindow = rec->resultsWindow;
			changePort();
			TextFace(bold);
			clickPt = rec->theEvent->where;
			GlobalToLocal(&clickPt);
			if (FindControl(clickPt,rec->resultsWindow,&theCntrl)) {	
				doControls(rec,theCntrl);
			} else {
				doInContent(rec);
			}
			SetPort(savePort);
			break; // ksiResultsClick
			
		case ksiConsoleText:
			GetIndString(newLTStr,128,15);
			GetIndString(fragsStr,128,19);			
			if ((cmpStrs(rec->consoleLine,newLTStr,kStartCmp) && (UserPrefs.spkSkipLT)) || 
				(cmpStrs(rec->consoleLine,fragsStr,kStartCmp) && (UserPrefs.spkSkipLT))) {
			} else {
				speakStringAsync(rec->consoleLine);
			}
			break;
	} // switch command

	ExitCodeResource();
} // main

void showControls(Boolean show)
{
	if (show) {
		ShowControl(PrefBtn);
		ShowControl(PopUpBtn);
		ShowControl(FavoritesBtn);
		ShowControl(RecentBtn);
		if (currentPanel == kAboutPanel) {
			ShowControl(AboutScrollBar);
		} else {
			ShowControl(SummaryBtn);
			if (currentPanel == kPlayerStatsPanel) {		// No summary for player stats
				HiliteControl(SummaryBtn,255);
			} else {
				HiliteControl(SummaryBtn,0);
			} // else
		} // if aboutPanel
	} else {
		HideControl(PrefBtn);
		HideControl(SummaryBtn);
		HideControl(PopUpBtn);
		HideControl(FavoritesBtn);
		HideControl(AboutScrollBar);
	}
}

void analysePicture(ScoreInterfaceRecord *rec)
{
	Handle					theHandle;
	Ptr						thePtr;
	long					offset, len;
	unsigned short			opcode, num, size, i, index;
	unsigned char			modOff, modOp, count, skip;
	short					resFile,refNum;
	Str255					theString = "\p", hexOpcode, hexOffset, misTag, lvlTag, adjustString;
	GrafPtr					oldPort;
	WindowPtr				theWindow;
	Rect					theRect;
	Boolean					customColors = false;
	RGBColor				currentColor, arcColor, hiliteColor;
	PaletteHandle			thePalette;
	OSType					theType;
	
	resFile=CurResFile();
	for (index=1;index<=CountResources('LEDI');index++) {
		theHandle = GetIndResource('LEDI',index);
		if (!theHandle) {
			UseResFile(resFile);
			doErrorAlert(rColorStrings,kNoLEDI);
			return;
		}
		HLock(theHandle);
		HNoPurge(theHandle);
		thePtr = *theHandle;
		
		offset = 0;
		len = 4;
		BlockMoveData(thePtr+offset,&theType,len);
		
		refNum = HomeResFile(theHandle);
		UseResFile(refNum);
		HUnlock(theHandle);
		HPurge(theHandle);
		ReleaseResource(theHandle);
		
		if (theType == rec->levelTag) break;		// found LEDI
	}
	
	if (theType != rec->levelTag) {
		UseResFile(resFile);
		doErrorAlert(rColorStrings,kBadLEDI);
		return;
	}
	
	offset += 4;
	len = 2;
	BlockMoveData(thePtr+offset,&size,len);
	offset += 2;
	for (i=0; i<size; i++) {
		len = 4;			// Mission Tag
		BlockMoveData(thePtr+offset,&theType,len);
		offset += 4;
		
		len = 1;			// Mission Name
		BlockMoveData(thePtr+offset,&count,len);
		//len = count+1;
		//BlockMoveData(thePtr+offset,&theString,len);
		offset += count+1;
		offset += offset%2;
		
		len = 1;			// Information length
		BlockMoveData(thePtr+offset,&skip,len);
		offset += 1+skip;	// skip it
		offset += offset%2;
		
		len = 1;			// Picture Name
		BlockMoveData(thePtr+offset,&count,len);
		if (theType == rec->directoryTag) {
			len = count+1;
			BlockMoveData(thePtr+offset,&theString,len);
			//ParamText(theString,"\p","\p","\p");
			//NoteAlert(rErrorAlert,NULL);
			break;
		}
		offset += count+1;
		offset += offset%2;
		
		offset += 2;		// Skip enables required
		/*len = 2;			// File or resource?
		BlockMoveData(thePtr+offset,&num,len);
		if (theType == rec->directoryTag) break;*/		// found Picture
		offset += 2;
		
		offset += 4;		// Skip $00000000 temporary

		len = 2;			// Information length
		BlockMoveData(thePtr+offset,&num,len);
		offset += 2+4*num;
	}
	
	HUnlock(theHandle);
	HPurge(theHandle);
	ReleaseResource(theHandle);

	/*if (num) {		// It's a file
		//doErrorAlert(rColorStrings,kNoPICT);
		SysBeep(20);
		SysBeep(20);
		return;
	}*/

	if (theType != rec->directoryTag) {
		UseResFile(resFile);
		doErrorAlert(rColorStrings,kNoPICT);
		return;
	}
	
	theHandle = GetNamedResource('PICT',theString);
	
	refNum = CurResFile();
	UseResFile(resFile);
	addRecentFile(refNum);
			
	if (!theHandle) {
		doErrorAlert(rColorStrings,kNoPICT);
		return;
	}
	HLock(theHandle);
	HNoPurge(theHandle);
	thePtr = *theHandle;

	updateColorPalette(&UserPrefs);
	GetIndString(adjustString,rColorStrings,kAdjustTeamColor);			// adjust TeamColor
	currentColor.red = currentColor.green = currentColor.blue = 32000;	// medium gray color

	offset = 10;
	for (;1;) {
		len = 2;
		offset += offset % 2;			// 2 byte align
		BlockMoveData(thePtr+offset,&opcode,len);
		offset += 2;					// skip opcode = 2 bytes
		
		if (opcode == 0x00FF) { break; }
		switch (opcode) {
			//case 0x0090: case 0x0091: // experimenteel vastgelegd, ik denk dat het 0 is
			
			case 0x0000: // no opcode
			case 0x0001: // clip (?)
			case 0x0011: // version
			case 0x02FF: // version + end version 1 PICT
			case 0x001C: // hilite mode flag
			case 0x001E: // use default hilite color
			case 0x0038: case 0x0039: case 0x003A: case 0x003B: case 0x003C: // doSameRect
			case 0x0048: case 0x0049: case 0x004A: case 0x004B: case 0x004C: // doSameRRect
			case 0x0058: case 0x0059: case 0x005A: case 0x005B: case 0x005C: // doSameOval
			case 0x0078: case 0x0079: case 0x007A: case 0x007B: case 0x007C: // doSamePoly
			case 0x0088: case 0x0089: case 0x008A: case 0x008B: case 0x008C: // doSameRgn
				break;
				
			case 0x0004: // TxFace, Text Face (byte)
				offset += 1;
				break;
				
			case 0x0003: // text font
			case 0x0005: // text mode
			case 0x0008: // pen mode
			case 0x000D: // text size
			case 0x0015: // fractal pen position
			case 0x0016: // extra for each character
			case 0x0023: // ShortLineFrom
			case 0x00A0: // short comment
				offset += 2;
				break;
				
			case 0x0068: // frameSameArc
				hiliteColor = currentColor;
			case 0x0006: // space extra (fixed point)
			case 0x0007: // pen size (point)
			case 0x000B: // oval size (point)
			case 0x000C: // origin (dh,dv point)
			case 0x000E: // foregroundcolor (long)
			case 0x000F: // backgroundcolor (long)
			case 0x0021: // line from (newPt point)
			case 0x0069: case 0x006A: case 0x006B: case 0x006C: // doSameArc
				offset += 4;
				break;
				
			case 0x001A: // RGBForeColor
				len = 2;
				BlockMoveData(thePtr+offset,&currentColor.red,len);
				len = 2;
				BlockMoveData(thePtr+offset+2,&currentColor.green,len);
				len = 2;
				BlockMoveData(thePtr+offset+4,&currentColor.blue,len);
			case 0x001B: case 0x001D: case 0x001F: // RGB colors
			case 0x0022: // ShortLine, pnLoc (point), dh,dv (-128..127)
				offset += 6;
				break;
				
			case 0x0002: case 0x0009: case 0x000A: // patterns
			case 0x0010: // TxRatio; numer (point), denom (point)
			case 0x0020: // Line; from (point), to (point)
			case 0x0030: case 0x0031: case 0x0032: case 0x0033: case 0x0034: // doRectangle
			case 0x0035: case 0x0036: case 0x0037: // reserved for Apple Use
			case 0x0040: case 0x0041: case 0x0042: case 0x0043: case 0x0044: // doRRectangle
			case 0x0045: case 0x0046: case 0x0047: // reserved for Apple Use
			case 0x0050: case 0x0051: case 0x0052: case 0x0053: case 0x0054: // doOval
			case 0x0055: case 0x0056: case 0x0057: // reserved for Apple Use
				offset += 8;
				break;
				
			case 0x0061:
				arcColor = currentColor;
			case 0x0060: case 0x0062: case 0x0063: case 0x0064: // doArc
				offset += 12;
				break;
				
			case 0x0C00: // HeaderOp
				offset += 24;	// 24 bytes of data
				break;
			
			case 0x0028: offset += 2;	// LongText, txLoc (point), count &data
			case 0x002B: offset += 1;	// DHDVText, dv, dv, count &data
			case 0x0029: case 0x002A:	// DHText and DVText, dh/dv, count & data
				offset += 1;
				len = 1;
				BlockMoveData(thePtr+offset,&count,len);
				count += 1;
				BlockMoveData(thePtr+offset,theString,count);
				offset += count;				// skip data
				
				if (cmpStrs(adjustString,theString,kStartCmp)) {	// adjust TeamColor
					for (i=1; i<=6; i++) {
						count = chrInStr((unsigned char)('0'+i),theString);
						if (!count) continue;
						if (theString[count-1] != 'T') continue;
						customColors = true;
						theWindow = plugWindow;
						while (theWindow != NULL) {
							thePalette = GetPalette(theWindow);
							SetEntryColor(thePalette,i+5,&arcColor);
							SetEntryColor(thePalette,i+11,&hiliteColor);
							ActivatePalette(theWindow);
							theWindow = (WindowPtr)(((WindowPeek)theWindow)->nextWindow);
						} // while
					} // for
				} // if adjust TeamColor
				break;
				
			case 0x00A1: offset += 2;	// kind (word)
			case 0x0024: case 0x0025: case 0x0026: case 0x0027:  // skip 2 + data length
			case 0x002C: case 0x002D: case 0x002E: case 0x002F:  // skip 2 + data length
			case 0x0096: case 0x0097: case 0x009A: case 0x009B:  // skip 2 + data length
			case 0x009C: case 0x009D: case 0x009E: case 0x009F:  // skip 2 + data length
				len = 2;
				BlockMoveData(thePtr+offset,&size,len);
				offset += 2+size;		// skip size = 2 bytes
				break;
				
			default:
				if ((opcode >= 0x0100) && (opcode <=0x7FFF)) {
					size = (long)(opcode/256);
					offset += size*2;	// For opcode $0100-$7FFF: the amount of data for 
					break;				// opcode $nnXX = 2 * nn bytes
				}
				if (((opcode >= 0x00B0) && (opcode <=0x00CF)) ||
					((opcode >= 0x8000) && (opcode <=0x80FF))) {
					break;				// opcode has no data
				}
				/*if (((opcode >= 0x00D0) && (opcode <=0x00FE)) || 
					((opcode >= 0x8100) && (opcode <=0xFFFF))) {*/
					//len = 4;
					//BlockMoveData(thePict+offset,&dataLength,len);
				/*	offset += 4;
					offset += dataLength;				// skip 4 bytes + datalength
					//NoteAlert(rErrorAlert,NULL);
				}*/
				//NumToString(dataLength,tempNumStr);
				
				offset -= 2;
				hexOpcode[0] = hexOffset[0] = 5;
				hexOpcode[1] = hexOffset[1] = (char)'$';
				for (i = 5; i>1; i--) {
					modOff = offset % 16;
					modOp = opcode % 16;
					offset = (long)(offset/16);
					opcode = (long)(opcode/16);
					if (modOp < 10) { hexOpcode[i] = (char)('0' + modOp); } else { hexOpcode[i] = (char)(55 + modOp); }
					if (modOff < 10) { hexOffset[i] = (char)('0' + modOff); } else { hexOffset[i] = (char)(55 + modOff); }
				}
				
				len = 4;
				BlockMoveData(&rec->directoryTag,lvlTag+1,len);
				lvlTag[0] = 4;
				len = 4;
				BlockMoveData(&rec->levelTag,misTag+1,len);
				misTag[0] = 4;
				
				offset += 6;
				ParamText(hexOpcode,hexOffset,misTag,lvlTag);
				if (gVerboseErrors) NoteAlert(rUnknowOpcode,NULL);	//Display parse error
				HUnlock(theHandle);
				HPurge(theHandle);
				ReleaseResource(theHandle);
				updateColorPalette(&UserPrefs);
				return;
		}
	}
	HUnlock(theHandle);
	HPurge(theHandle);
	ReleaseResource(theHandle);

	if (customColors) {
		GetPort(&oldPort);
		theWindow = FrontWindow();
		SetRect(&theRect,0,0,32000,32000);
		theWindow = plugWindow;
		while (theWindow != NULL) {
			SetPort(theWindow);
			InvalRect(&theRect);
			theWindow = (WindowPtr)(((WindowPeek)theWindow)->nextWindow);
		}
		SetPort(oldPort);
	} else {
		updateColorPalette(&UserPrefs);
	}
}

short findColumn(short whichColumn)
{
	short		column;
	
	for (column=0; column<4; column++) {
		if (columnOrder[(startColumn+column)%noColumns] == whichColumn) {
			break;
		} // if
	} // for
	return column;
}

void addPlayerScore(ScoreInterfaceRecord *rec,short theColumn,long plus)
{
	short			i, column, line = 0;
	
	column = findColumn(theColumn);
	if ((plus != 0) && (plugVisible) && (UserPrefs.gnlUpdate)) {
		switch (currentPanel) {
			case kPlayerPanel:
				if (column<4) {
					MoveTo(124+(column+1)*42-3,panelRect.top+kTitleHeight+kTextHeight+sortOrder[rec->playerID]*kRowHeight);
					drawScore(&playerPoints[rec->playerID][theColumn],playerPoints[rec->playerID][theColumn]+plus);
					teamPoints[playerTeam[rec->playerID]][theColumn] += plus;
					return;
				}
				if (theColumn == kTotalColumn) {
					MoveTo(342,panelRect.top+kTitleHeight+kTextHeight+sortOrder[rec->playerID]*kRowHeight);
					drawScore(&playerPoints[rec->playerID][kTotalColumn],playerPoints[rec->playerID][kTotalColumn]+plus);
					teamPoints[playerTeam[rec->playerID]][kTotalColumn] += plus;
					return;
				}
			case kTeamPanel:
				for (i = 0; i < playerTeam[rec->playerID]; i++) {
					line += memberCount[teamOrder[i]];
				}

				if (column<4) {
					MoveTo(124+(column+1)*42-3,panelRect.top+kTitleHeight+kTextHeight+line*kRowHeight);
					drawScore(&teamPoints[playerTeam[rec->playerID]][theColumn],teamPoints[playerTeam[rec->playerID]][theColumn]+plus);
					playerPoints[rec->playerID][theColumn] += plus;
					return;
				}
				if (theColumn == kTotalColumn) {
					MoveTo(342,panelRect.top+kTitleHeight+kTextHeight+line*kRowHeight);
					drawScore(&teamPoints[playerTeam[rec->playerID]][kTotalColumn],teamPoints[playerTeam[rec->playerID]][kTotalColumn]+plus);
					playerPoints[rec->playerID][kTotalColumn] += plus;
					return;
				}
			case kPlayerStatsPanel:
				if (theColumn == kTotalColumn) {
					MoveTo(342,panelRect.top+kTitleHeight+kTextHeight+sortOrder[rec->playerID]*kRowHeight);
					drawScore(&playerPoints[rec->playerID][kTotalColumn],playerPoints[rec->playerID][kTotalColumn]+plus);
					teamPoints[playerTeam[rec->playerID]][kTotalColumn] += plus;
					return;
				}
		}
	}
	playerPoints[rec->playerID][theColumn] += plus;
	teamPoints[playerTeam[rec->playerID]][theColumn] += plus;
}

void addPlayerStats(ScoreInterfaceRecord *rec,short column)
{
	switch (column) {	// ((currentPanel == kPlayerStatsPanel) && (plugVisible)) -> DrawPercent
		case kShotColumn:
			ForeColor(whiteColor);
			drawPercent(rec->playerID, kShotColumn, sortOrder[rec->playerID]);
			ammoFired[rec->playerID][kShotColumn] += 1;
			ammoHit[rec->playerID][kShotColumn] += rec->scorePoints>0;
			ForeColor(blackColor);
			drawPercent(rec->playerID, kShotColumn, sortOrder[rec->playerID]);
			break;
		case kGrenadeColumn:
		case kMissileColumn:
			if ((TickCount()-ammoTime[rec->playerID][column]) <= 15) { //15 ticks between impact
				if ((!ammoCounted[rec->playerID][column]) && (rec->scorePoints>0)) {
					ForeColor(whiteColor);
					drawPercent(rec->playerID, column, sortOrder[rec->playerID]);
					
					ammoHit[rec->playerID][column] += 1;
					ammoCounted[rec->playerID][column] = true;
					
					ForeColor(blackColor);
					drawPercent(rec->playerID, column, sortOrder[rec->playerID]);
				}
			} else {
				ForeColor(whiteColor);
				drawPercent(rec->playerID, column, sortOrder[rec->playerID]);
				
				ammoHit[rec->playerID][column] += rec->scorePoints>0;
				ammoCounted[rec->playerID][column] = rec->scorePoints>0;
				ammoFired[rec->playerID][column] += 1;
				ammoTime[rec->playerID][column] = TickCount();
				
				ForeColor(blackColor);
				drawPercent(rec->playerID, column, sortOrder[rec->playerID]);
			}
			break;
	}
}

void sortScores(void)
{
	short			i, j, temp, loc;
	long			max;
	Boolean			update = false;
	Str255			theScore;
	Rect			theRect;
	short			tempOrder[6];
	
	switch (currentPanel) {
		case kPlayerPanel:
			for (i=0;i<6;i++) tempOrder[i] = sortOrder[i];
			for (i=5;i>0;i--) {
				if (!playerJoined[i]) continue;
				for (j=0;j<i;j++) {
					if (!playerJoined[j]) continue;
					if (playerPoints[sortOrder[i]][kTotalColumn]<playerPoints[sortOrder[j]][kTotalColumn]) continue;
					temp = sortOrder[i];
					sortOrder[i] = sortOrder[j];
					sortOrder[j]= temp;
				}
			}
			for (i=0;i<6;i++) update = update || (tempOrder[i] != sortOrder[i]);
			break;
		case kTeamPanel:
			break;
	}
	if (update) updateGameScores();
	ForeColor(whiteColor);
	SetRect(&theRect,10,10,350,25);
	PaintRect(&theRect);
	ForeColor(blackColor);
	for (i = 0; i<6; i++) {
		MoveTo(50*i+20,20);
		NumToString(sortOrder[i],theScore);
		DrawString(theScore);
		Move(10,0);
		NumToString(playerPoints[i][kTotalColumn],theScore);
		DrawString(theScore);
	}
}

void getBandWidth(void)
{
	MenuHandle			theMenu;
	short				theMark;

	theMenu = GetMHandle(141);
	BandWidth = 0;
	while (true) {
		BandWidth++;
		
		GetItemMark(theMenu,BandWidth,&theMark);
		if (theMark != noMark) break;
		if (BandWidth >= CountMItems(theMenu)) break;
	}	
}

void getLatency(void)
{
	MenuHandle			theMenu;
	short				theMark, LT;

	theMenu = GetMHandle(139);
	LT = 0;
	while (true) {
		LT++;
		if (LT > CountMItems(theMenu)) break;
		
		GetItemMark(theMenu,LT,&theMark);
		if (theMark != noMark) break;
	}
	
	if (LT > maxLT) maxLT = LT;
	if (LT < minLT) minLT = LT;
}

void drawHeaderRect(struct Rect *theRect)
{
	RGBColor				GrayColor;
	
	GrayColor.red = GrayColor.green = GrayColor.blue = 52000;
	RGBForeColor(&GrayColor);
	PaintRect(theRect);
	
	ForeColor(whiteColor);
	MoveTo(theRect->left+1,theRect->bottom-1);
	LineTo(theRect->left+1,theRect->top+1);
	LineTo(theRect->right-1,theRect->top+1);
	
	GrayColor.red = GrayColor.green = GrayColor.blue = 40000;
	RGBForeColor(&GrayColor);
	MoveTo(theRect->right-2,theRect->top+2);
	LineTo(theRect->right-2,theRect->bottom-2);
	LineTo(theRect->left+2,theRect->bottom-2);
	ForeColor(blackColor);
	FrameRect(theRect);
}

void drawWindow(ScoreInterfaceRecord *rec)
{
	Str255						theVersion;
	RGBColor					GrayColor;
	
	GetIndString(theVersion,rPlugStrings,1);
	
	GrayColor.red = GrayColor.green = GrayColor.blue = 40000;
	RGBForeColor(&GrayColor);
	MoveTo(rec->resultsRect.right-1,rec->resultsRect.top+1);	//window outline
	Line(0,rec->resultsRect.bottom-rec->resultsRect.top-2);
	Line(-50,0);
	Move(-60,0);
	Line(-234,0);

	ForeColor(whiteColor);
	MoveTo(7,184);				//window outline
	LineTo(7,7);
	LineTo(351,7);
	
	if (currentPanel == kAboutPanel) {
		drawShadowRect(aboutPanelRect);
	} else {
		drawShadowRect(panelRect);
	}
	
	drawButtons(rec);
	drawPanel(rec);
	
	TextFace(normal);
	PmForeColor(0);
	MoveTo(280-StringWidth(theVersion),28);
	DrawString(theVersion);
	MoveTo(279-StringWidth(theVersion),27);
	PmForeColor(1);
	DrawString(theVersion);
	TextFace(bold);
} // DrawWindow

void switchPanel(ScoreInterfaceRecord *rec)
{
	Rect		myRect;
	
	if (currentPanel == kAboutPanel) {
		HideControl(SummaryBtn);
		drawPanel(rec);
		ShowControl(AboutScrollBar);
	} else {
		HideControl(AboutScrollBar);
		PmBackColor(3);
		SetRect(&myRect,12,150,347,178);
		EraseRect(&myRect);
		FrameRect(&panelRect);
		drawPanel(rec);
		if (currentPanel == kPlayerStatsPanel) {		// No summary for player stats
			HiliteControl(SummaryBtn,255);
		} else {
			HiliteControl(SummaryBtn,0);
		} // else
		ShowControl(SummaryBtn);
	} // if
	BeginUpdate(plugWindow);
	EndUpdate(plugWindow);
	drawButtons(rec);
}

void drawPanel(ScoreInterfaceRecord *rec)
{	
	switch (currentPanel) {
		case kAboutPanel:
			drawAboutPanel(GetCtlValue(AboutScrollBar));
			break;
		case kPlayerPanel:
			drawTitleBar();
			drawSharedInterface();
			drawPlayerPanel();
			drawSummaryPanel();
			break;
		case kTeamPanel:
			drawTitleBar();
			drawSharedInterface();
			drawTeamPanel();
			drawSummaryPanel();
			break;
		case kPlayerStatsPanel:
			drawTitleBar();
			//drawSharedInterface();
			drawPlayerStatsPanel();
			drawSummaryPanel();
			break;
		case kGameStatsPanel:
			drawGameStatsPanel();
			break;
		default:
			break;
	} // switch currentPanel
	ForeColor(blackColor);
	TextFace(bold);
} // DrawPanel

void drawSharedInterface(void)
{
	RGBColor			myRGB;
	Rect				myRect;
	short				i;
	
	ForeColor(whiteColor);
	SetRect(&myRect,panelRect.left+1,panelRect.top+kTitleHeight,panelRect.right-1,panelRect.bottom-1);
	PaintRect(&myRect);
	drawShadowRect(panelRect);

	myRGB.red = myRGB.green = myRGB.blue = 59367;
	RGBForeColor(&myRGB);
	MoveTo(14,panelRect.top+kTitleHeight); Line(330,0);
	for (i=0; i<=4; i++) {
		MoveTo(124+42*i,panelRect.top+kTitleHeight);
		Line(0,6*kRowHeight);
	} // for i
	MoveTo(304,panelRect.top+kTitleHeight); Line(0,6*kRowHeight);
} // DrawSharedInterface

void drawSummaryPanel(void)
{
	RGBColor			myRGB;
	short				i, column;

	ForeColor(whiteColor);
	PaintRect(&optionsRect);	// options Rect
	drawShadowRect(optionsRect);

	myRGB.red = myRGB.green = myRGB.blue = 59367;
	RGBForeColor(&myRGB);
	
	MoveTo(optionsRect.left+1,optionsRect.top+1);
	LineTo(optionsRect.right-2,optionsRect.top+1);
	MoveTo(optionsRect.left+1,optionsRect.bottom-2);
	LineTo(optionsRect.right-2,optionsRect.bottom-2);

	if ((currentPanel!=kPlayerStatsPanel) && (gSummaryType==6)) {
		myRGB.red = myRGB.green = myRGB.blue = 32000;
		RGBForeColor(&myRGB);
		drawCenteredString("\pSummaries are disabled",optionsRect.left+(int)((optionsRect.right-optionsRect.left)/2),optionsRect.top+kTextHeight+1);
		return;
	}
		
	switch (currentPanel) {
		case kPlayerPanel:
			for (i=1; i<=4; i++) {							//Draw divider lines
				MoveTo(124+42*i,optionsRect.top+1);
				LineTo(124+42*i,optionsRect.bottom-2);
			} // for i
			MoveTo(304,optionsRect.top+1);
			LineTo(304,optionsRect.bottom-2);

			if (noPlayers == 0) break;						//Skip summary scores
			for (column=0; column<4; column++) {
				MoveTo(124+(column+1)*42-3,optionsRect.top+kTextHeight+1);
				drawScore(&playerPoints[5+gSummaryType][columnOrder[(startColumn+column)%noColumns]],
						   playerPoints[5+gSummaryType][columnOrder[(startColumn+column)%noColumns]]);
			} // for column
			MoveTo(342,optionsRect.top+kTextHeight+1);
			drawScore(&playerPoints[5+gSummaryType][kTotalColumn],playerPoints[5+gSummaryType][kTotalColumn]);
			break;
		case kTeamPanel:
			for (i=1; i<=4; i++) {							//Draw divider lines
				MoveTo(124+42*i,optionsRect.top+1);
				LineTo(124+42*i,optionsRect.bottom-2);
			} // for i
			MoveTo(303,optionsRect.top+1);
			LineTo(303,optionsRect.bottom-2);

			if (noPlayers == 0) break;						//Skip summary scores
			for (column=0; column<4; column++) {
				MoveTo(124+(column+1)*42-3,optionsRect.top+kTextHeight+1);
				drawScore(&teamPoints[5+gSummaryType][columnOrder[(startColumn+column)%noColumns]],
						   teamPoints[5+gSummaryType][columnOrder[(startColumn+column)%noColumns]]);
			} // for column
			MoveTo(342,optionsRect.top+kTextHeight+1);
			drawScore(&teamPoints[5+gSummaryType][kTotalColumn],playerPoints[5+gSummaryType][kTotalColumn]);
			break;
		case kPlayerStatsPanel:
			/*for (i=1; i<=3; i++) {
				MoveTo(124+56*i,optionsRect.top+1);
				LineTo(124+56*i,optionsRect.bottom-2);
			} // for i
			MoveTo(303,optionsRect.top+1);
			LineTo(303,optionsRect.bottom-2);*/
			myRGB.red = myRGB.green = myRGB.blue = 32000;
			RGBForeColor(&myRGB);
			drawCenteredString("\pSummaries not available",optionsRect.left+(int)((optionsRect.right-optionsRect.left)/2),optionsRect.top+kTextHeight+1);
			break;
		default:
			break;
	} // end switch
}

void drawButtons(ScoreInterfaceRecord *rec)
{	
	//BackColor(whiteColor);				//White background buttons aren't used anymore
	DrawControls(rec->resultsWindow);		//Used to be for 'Enable on-line scoretracking' button
	//PmBackColor(3);
} // DrawButtons

void drawScore(long *score,long newScore)
{
	Str255					theScore;
	
	NumToString(*score,theScore);
	if (*theScore > 6) TextFace(condense);		// compress more than 6 characters
	Move(-StringWidth(theScore),0);
	if (*score != newScore) {
		ForeColor(whiteColor);
		DrawString(theScore);
		*score = newScore;
		NumToString(*score,theScore);
		TextFace(normal);
		if (*theScore > 6) TextFace(condense);	// compress more than 6 characters
		Move(-StringWidth(theScore),0);
	}
	if ((*score < 0) && (UserPrefs.gnlNegRed)) {
		ForeColor(redColor);
	} else {
		ForeColor(blackColor);
	}
	DrawString(theScore);
	TextFace(normal);
}

void drawPercent(short player,short column,short line)
{
	Str255			theStr;
	
	if ((currentPanel != kPlayerStatsPanel) || (!plugVisible)) return;

	if (ammoFired[player][column] != 0) {
		NumToString((long)(100*ammoHit[player][column]/ammoFired[player][column]),theStr);
		theStr[0] += 1;
		theStr[theStr[0]] = (unsigned char)'%';
		drawCenteredString(theStr,124+column*56+17,panelRect.top+kTitleHeight+kTextHeight+line*kRowHeight);
		NumToString((long)ammoFired[player][column],theStr);
		MoveTo(124+(column+1)*56-3-StringWidth(theStr),panelRect.top+kTitleHeight+kTextHeight+line*kRowHeight);
		DrawString(theStr);
	} else {
		GetIndString(theStr,rTitleStrings,9);
		drawCenteredString(theStr,124+column*56+26,panelRect.top+kTitleHeight+kTextHeight+line*kRowHeight);
	}
}

void addPlayerSummary(ScoreInterfaceRecord *rec,short theColumn,long plus)	// KAN BETER!!!!
{
	short			i, column, line = 0, player=rec->playerID;
	long			score[5];
	
	column = findColumn(theColumn);

	score[kTotalSummary] = 0;
	score[kMinimumSummary] = 1000000;
	score[kMaximumSummary] = -999999;
	for (i=0; i<6; i++) {
		if (!playerJoined[i]) continue;
		score[kTotalSummary] += playerPoints[i][theColumn];
		if (playerPoints[i][theColumn] > score[kMaximumSummary]) 
			score[kMaximumSummary] = playerPoints[i][theColumn];
		if (playerPoints[i][theColumn] < score[kMinimumSummary]) 
			score[kMinimumSummary] = playerPoints[i][theColumn];
	}
	score[kAverageSummary] = (long)(score[kTotalSummary]/noPlayers);
	
	if (gSummaryType!=6) {
		if ((column<4) && (plugVisible) && (UserPrefs.gnlUpdate) && (currentPanel==kPlayerPanel)) {
			MoveTo(124+(column+1)*42-3,optionsRect.top+kTextHeight+1);
			drawScore(&playerPoints[5+gSummaryType][theColumn],score[gSummaryType]);
		} //if
			
		if ((column=kTotalColumn) && (plugVisible) && (UserPrefs.gnlUpdate) && (currentPanel==kPlayerPanel)) {
			MoveTo(342,optionsRect.top+kTextHeight+1);
			drawScore(&playerPoints[5+gSummaryType][kTotalColumn],score[gSummaryType]);
		} // if
	}
	
	playerPoints[5+kTotalSummary][theColumn] = score[kTotalSummary];
	playerPoints[5+kMinimumSummary][theColumn] = score[kMinimumSummary];
	playerPoints[5+kMaximumSummary][theColumn] = score[kMaximumSummary];
	playerPoints[5+kAverageSummary][theColumn] = score[kAverageSummary];
}

void addTeamSummary(ScoreInterfaceRecord *rec,short theColumn,long plus)
{
	short			i, column, line = 0, noTeams;
	long			score[5];
	
	column = findColumn(theColumn);
	noTeams = 0;

	score[kTotalSummary] = 0;
	score[kMinimumSummary] = 1000000;
	score[kMaximumSummary] = -999999;
	for (i=0; i<6; i++) {
		if (!memberCount[i]) continue;
		noTeams++;
		score[kTotalSummary] += teamPoints[i][theColumn];
		if (teamPoints[i][theColumn] > score[kMaximumSummary]) 
			score[kMaximumSummary] = teamPoints[i][theColumn];
		if (teamPoints[i][theColumn] < score[kMinimumSummary]) 
			score[kMinimumSummary] = teamPoints[i][theColumn];
	}
	if (noTeams != 0) score[kAverageSummary] = (long)(score[kTotalSummary]/noTeams);
	
	if (gSummaryType!=6) {
		if ((column<4) && (plugVisible) && (UserPrefs.gnlUpdate) && (currentPanel==kTeamPanel)) {
			MoveTo(124+(column+1)*42-3,optionsRect.top+kTextHeight+1);
			drawScore(&teamPoints[5+gSummaryType][theColumn],score[gSummaryType]);
		} //if
			
		if ((column=kTotalColumn) && (plugVisible) && (UserPrefs.gnlUpdate) && (currentPanel==kTeamPanel)) {
			MoveTo(342,optionsRect.top+kTextHeight+1);
			drawScore(&teamPoints[5+gSummaryType][kTotalColumn],score[gSummaryType]);
		} // if
	}
	
	teamPoints[5+kTotalSummary][theColumn] = score[kTotalSummary];
	teamPoints[5+kMinimumSummary][theColumn] = score[kMinimumSummary];
	teamPoints[5+kMaximumSummary][theColumn] = score[kMaximumSummary];
	teamPoints[5+kAverageSummary][theColumn] = score[kAverageSummary];
}

void drawPlayerPanel()
{
	short					i, column;
	Rect					myRect;
	RGBColor				myRGB;
	
	myRGB.red = myRGB.green = myRGB.blue = 59367;

	for (i = 0; i < 6; i++) {
		RGBForeColor(&myRGB);
		MoveTo(14,panelRect.top+kTitleHeight+(i+1)*kRowHeight);
		Line(330,0);
		if (!playerJoined[i]) { continue; }
		
 		PmForeColor(playerTeam[i]+5);
 		SetRect(&myRect,293,panelRect.top+kTitleHeight+1+sortOrder[i]*kRowHeight,
 						304,panelRect.top+kTitleHeight+(sortOrder[i]+1)*kRowHeight);
 		PaintRect(&myRect);

		ForeColor(blackColor);
 		
		MoveTo(18,panelRect.top+kTitleHeight+kTextHeight+sortOrder[i]*kRowHeight);
		DrawString(players[i]);
 		
 		for (column=0; column<4; column++) {
 			MoveTo(124+(column+1)*42-3,panelRect.top+kTitleHeight+kTextHeight+sortOrder[i]*kRowHeight);
			drawScore(&playerPoints[i][columnOrder[(startColumn+column)%noColumns]],
									playerPoints[i][columnOrder[(startColumn+column)%noColumns]]);
		} // for column
		MoveTo(342,panelRect.top+kTitleHeight+kTextHeight+sortOrder[i]*kRowHeight);
		drawScore(&playerPoints[i][kTotalColumn],playerPoints[i][kTotalColumn]);
	} // for i
} // DrawPlayerPanel

void drawTeamPanel()
{
	short					i, j, line, column;
	Rect					myRect;
	RGBColor				myRGB;
	
	myRGB.red = myRGB.green = myRGB.blue = 59367;

	line = 0;
	for (i = 0; i < 6; i++) {
		if (memberCount[teamOrder[i]] == 0) { continue; }
		
 		PmForeColor(teamOrder[i]+5);
 		SetRect(&myRect,293,panelRect.top+kTitleHeight+1+line*kRowHeight,
 						304,panelRect.top+kTitleHeight+(line+memberCount[teamOrder[i]])*kRowHeight);
 		PaintRect(&myRect);
		
 		for (column=0; column<4; column++) {
 			MoveTo(124+(column+1)*42-3,panelRect.top+kTitleHeight+kTextHeight+line*kRowHeight);
			drawScore(&teamPoints[i][columnOrder[(startColumn+column)%noColumns]],
									teamPoints[i][columnOrder[(startColumn+column)%noColumns]]);
		} // for column
		MoveTo(342,panelRect.top+kTitleHeight+kTextHeight+line*kRowHeight);
		drawScore(&teamPoints[i][kTotalColumn],teamPoints[i][kTotalColumn]);
		
		ForeColor(blackColor);
		for (j = 0; j < 6; j++) {
			if ((playerJoined[j]) && (playerTeam[j] == teamOrder[i])) {
				line++;
				MoveTo(18,panelRect.top+kTitleHeight+kTextHeight+(line-1)*kRowHeight);
				DrawString(players[j]);
			}
		} 		
 		RGBForeColor(&myRGB);
		MoveTo(14,panelRect.top+kTitleHeight+line*kRowHeight);
		Line(330,0);
	} // for i
	for (line++; line<7; line++){
 		RGBForeColor(&myRGB);
		MoveTo(14,panelRect.top+kTitleHeight+line*kRowHeight);
		Line(330,0);
	}
} // DrawTeamPanel


void drawGameStatsPanel(void)
{
	Rect					myRect;
	
	ForeColor(whiteColor);
	SetRect(&myRect,14,38,345,139);
	PaintRect(&myRect);
	
	SetRect(&myRect,13,37,134,55);
	drawHeaderRect(&myRect);
	SetRect(&myRect,13,54,134,72);
	drawHeaderRect(&myRect);
	SetRect(&myRect,13,71,134,89);
	drawHeaderRect(&myRect);
	SetRect(&myRect,13,88,134,106);
	drawHeaderRect(&myRect);
	SetRect(&myRect,13,105,134,123);
	drawHeaderRect(&myRect);
	SetRect(&myRect,13,122,134,140);
	drawHeaderRect(&myRect);
}

void drawPlayerStatsPanel(void)
{
	short					i, line, column;
	Rect					myRect;
	long					theTime;
	RGBColor				myRGB;
	
	ForeColor(whiteColor);
	SetRect(&myRect,panelRect.left+1,panelRect.top+kTitleHeight,panelRect.right-1,panelRect.bottom-1);
	PaintRect(&myRect);
	drawShadowRect(panelRect);

	myRGB.red = myRGB.green = myRGB.blue = 59367;
	RGBForeColor(&myRGB);
	MoveTo(14,panelRect.top+kTitleHeight); Line(330,0);
	for (i=0; i<=3; i++) {
		MoveTo(124+56*i,panelRect.top+kTitleHeight);
		Line(0,6*kRowHeight);
	} // for i
	MoveTo(304,panelRect.top+kTitleHeight); Line(0,6*kRowHeight);
	

	line = -1;
	for (i = 0; i < 6; i++) {
		RGBForeColor(&myRGB);
		MoveTo(14,panelRect.top+kTitleHeight+(i+1)*kRowHeight);
		Line(330,0);
		if (!playerJoined[i]) { continue; }
		line++;
		
 		PmForeColor(playerTeam[i]+5);
 		SetRect(&myRect,293,panelRect.top+kTitleHeight+1+line*kRowHeight,
 						304,panelRect.top+kTitleHeight+(line+1)*kRowHeight);
 		PaintRect(&myRect);

		ForeColor(blackColor);
 		
		MoveTo(18,panelRect.top+kTitleHeight+kTextHeight+line*kRowHeight);
		DrawString(players[i]);
 		
 		for (column=0; column<3; column++) {
			drawPercent(i,column,line);
		} // for column 		
 		 		
		MoveTo(342,panelRect.top+kTitleHeight+kTextHeight+line*kRowHeight);
		drawScore(&playerPoints[sortOrder[i]][kTotalColumn],playerPoints[sortOrder[i]][kTotalColumn]);
	} // for i
} // DrawPlayerStatsPanel

void drawShadowRect(Rect theRect)
{
	RGBColor			GrayColor;
	
	GrayColor.red = GrayColor.green = GrayColor.blue = 40000;
	RGBForeColor(&GrayColor);
	MoveTo(theRect.left-1,theRect.bottom-1);
	LineTo(theRect.left-1,theRect.top-1);
	LineTo(theRect.right-1,theRect.top-1);
	ForeColor(whiteColor);
	MoveTo(theRect.right,theRect.top);
	LineTo(theRect.right,theRect.bottom);
	LineTo(theRect.left,theRect.bottom);
	ForeColor(blackColor);
	FrameRect(&theRect);
}

void  checkMovieEnv(void)
{
	OSErr		osErr;
	SInt32		response;

	osErr = Gestalt(gestaltQuickTime,&response);			// OK for QuickTime?
	gHasQuickTime = (osErr == noErr);

	osErr = Gestalt(gestaltCompressionMgr,&response);		// OK for Compression?
	gHasQuickTime = (osErr == noErr) && gHasQuickTime;
}



Boolean initMovie(void)
{
	OSErr					theErr;
	long					maxCompressedSize;
	CGrafPtr				windowPortPtr;
	QDErr					qdErr;
	GDHandle				deviceHdl;
	Rect					movieRect;

	if (!gHasQuickTime) return false;

	if (gImageDesc) DisposeHandle((Handle)gImageDesc);		//delete old data structures
	if (gCompressedData) DisposeHandle(gCompressedData);
	if (gMoviePortPtr) DisposeGWorld(gMoviePortPtr);

	if(!gameWindow) {
		doErrorAlert(rMovieErrors,kMovWindowError); UserPrefs.movEnabled = 0;
		return false;
	} // gameWindow not found

	movieRect.top = movieRect.left = 0;
	movieRect.bottom = (short)((((GrafPtr) gameWindow)->portRect.bottom - ((GrafPtr) gameWindow)->portRect.top)/2);
	movieRect.right = (short)((((GrafPtr) gameWindow)->portRect.right - ((GrafPtr) gameWindow)->portRect.left)/2);

	GetGWorld(&windowPortPtr,&deviceHdl);
	qdErr = NewGWorld(&gMoviePortPtr,8,&movieRect/*((GrafPtr) gameWindow)->portRect*/,NULL,NULL,0);
	if(gMoviePortPtr == NULL || qdErr != noErr) {
		doErrorAlert(rMovieErrors,kMovQDError); UserPrefs.movEnabled = 0;
		return false;
	} // gWorldPort error

	theErr = GetMaxCompressionSize(gMoviePortPtr->portPixMap, &gMoviePortPtr->portRect, 8,
				codecMinQuality, 'smc ', (CompressorComponent)anyCodec, &maxCompressedSize);
	if (theErr != noErr) {
		doErrorAlert(rMovieErrors,kMovDataError); UserPrefs.movEnabled = 0;
		return false;
	}
	
	gCompressedData = NewHandle(maxCompressedSize);
	if (MemError() != noErr) {
		doErrorAlert(rMovieErrors,kMovMemError); UserPrefs.movEnabled = 0;
		return false;
	}
	
	MoveHHi(gCompressedData);
	HLock(gCompressedData);
	gCompressedDataPtr = StripAddress(*gCompressedData);
	gImageDesc = (ImageDescriptionHandle)NewHandle(4);
	if (MemError() != noErr) {
		doErrorAlert(rMovieErrors,kMovMemError); UserPrefs.movEnabled = 0;
		return false;
	}
	return true;
}

void initPlugIn(ScoreInterfaceRecord *rec)
{
	MenuHandle				myMenu;
	short					i,count,index;
	Str255					myStr,myErr1,myErr2,myErr3;
	long					daysLeft;
	unsigned long			currTime;
	OSErr					myErr;
	WindowPtr				myWindow;
	KeyMap					myKeyMap;

	GetKeys(myKeyMap);
	gVerboseErrors = myKeyMap[1] & 0x8000;			// Command-key -> Verbose logging ON
	
	plugWindow = rec->resultsWindow;
	findGameWindow();
	
	setPlugLevel();
	
	if (kIsDemoVersion && !gPlugFailure) {
		GetDateTime (&currTime);		// Check Expired
		daysLeft = (long)((long)(kExpireDate-currTime)/86400);

		if (daysLeft <= 0) {			// 28/02/1998
			gPlugFailure = true;
			GetIndString(myErr1,rErrorStrings,6);
			GetIndString(myErr2,rErrorStrings,8);
			GetIndString(myErr3,rErrorStrings,7);
			ParamText(myErr1,myErr2,myErr3,"\p");
			ShowCursor();
			StopAlert(rErrorAlert,NULL);
		}
		if (daysLeft < 4) {
			GetIndString(myErr1,rErrorStrings,6);
			GetIndString(myErr3,rErrorStrings,7);
			GetIndString(myErr2,rErrorStrings,8+daysLeft);
			ParamText(myErr1,myErr2,myErr3,"\p");
			ShowCursor();
			NoteAlert(rErrorAlert,NULL);
		}
	} // kIsDemoVersion
	
	if (gPlugFailure) return;								//some has gone wrong or expired
	
	for (i=0;i<noColumns;i++) {								//default order
		columnOrder[i] = i;
	} // for i

	checkSoundEnv();
	checkMovieEnv();
	if (gHasQuickTime) {
		myErr = EnterMovies();
		gHasQuickTime = (myErr == noErr);		// application can use QuickTime
	}
	
	rec->resultsChanged = true;   				// force window update
	oldResultsHandle = rec->resultsHandle;		// remember the original scoreboard or message
	rec->resultsHandle = NewHandle(0);			// create a new empty Handle
	
	myMenu = NewMenu(rPopUpMenuID,"\pPop-Up");
	if (myMenu != NULL) {
		for (i=1; i<=kPopUpMenuLength;i++) {
			GetIndString(myStr,rPopUpMenuID,i);
			AppendMenu(myMenu,myStr);
			SetItemStyle(myMenu,i,bold);
		} // for i
		InsertMenu(myMenu,kIsPopUpMenu);
		DrawMenuBar();
	} else {
		SysBeep(20);
	}

	myMenu = NewMenu(rPrefMenuID,"\pPreferences");
	if (myMenu != NULL) {
		for (i=1; i<=kPrefCount;i++) {
			GetIndString(myStr,rPrefTopicStrings,i);
			AppendMenu(myMenu,myStr);
			SetItemStyle(myMenu,i,bold);
		} // for i
		InsertMenu(myMenu,kIsPopUpMenu);
		DrawMenuBar();
	} else {
		SysBeep(20);
	}

	myMenu = NewMenu(rSummaryMenuID,"\pSummary");
	if (myMenu != NULL) {
		for (i=1; i<=kSummaryMenuLength;i++) {
			//doErrorAlert(rSummaryMenuID,i);
			GetIndString(myStr,rSummaryMenuID,i);
			AppendMenu(myMenu,myStr);
			SetItemStyle(myMenu,i,bold);
		} // for i
		InsertMenu(myMenu,kIsPopUpMenu);
		DrawMenuBar();
	} else {
		SysBeep(20);
	}

	aboutPict = GetPicture(rAboutPICT);
	aboutRect = (*aboutPict)->picFrame;
	aboutRect.right -=  aboutRect.left;
	aboutRect.left = 0;
	aboutRect.bottom -= aboutRect.top;
	aboutRect.top = 0;

	SetRect(&panelRect,13,40,346,149);
	SetRect(&optionsRect,124,159,346,177);
	SetRect(&aboutPanelRect,13,40,331,177);
	
	PopUpBtn = GetNewControl(rPopUpButtonID,rec->resultsWindow);
	PrefBtn = GetNewControl(rPrefButtonID,rec->resultsWindow);
	SummaryBtn = GetNewControl(rSummaryButtonID,rec->resultsWindow);
	FavoritesBtn = GetNewControl(rFavoritesButtonID,rec->resultsWindow);
	RecentBtn = GetNewControl(rRecentButtonID,rec->resultsWindow);
  	//HiliteControl(RecentBtn,255);
	AboutScrollBar = GetNewControl(rScrollBarID,rec->resultsWindow);
	if ((aboutRect.bottom-140)>0) {
		SetCtlMax(AboutScrollBar,aboutRect.bottom-aboutRect.top-(aboutPanelRect.bottom-aboutPanelRect.top));
	}
		
	for (i=0; i<3; i++) {
		ammoIcons[i] = GetCIcon(rAmmoIcons+i);
	}
	for (i=0; i<4; i++) {
		movieIcons[i] = GetCIcon(rMovieIcons+i);
	}
		
	for (i = 0; i <= kMoviePrefs; i++) {
		defaultPreferences(&UserPrefs,i);
	}

	if (gPlugLevel == kPayed) ReadPref();
	doAliases();				//create the menus
	updateColorPalette(&UserPrefs);
		
	if ((UserPrefs.logEnabled) && (UserPrefs.logTime == kLogEachSession)) {
		SelectLogFile(&UserPrefs);
	}
	
	UserPrefs.movEnabled = 0;	//QuickTime is disabled by default
	
	myErr = FSpGetFileLocation(HomeResFile(rec->plugIn),&gPlugSpec);	//Reference Alias
/*	if (myErr != noErr) HiliteControl(FavoritesBtn,255);		//Unable to get PlugIn FileSpec
	//myErr = FSpRename(&gPlugSpec,"\pScoreKeeper Test");
	myErr = FSMakeFSSpec(gPlugSpec.vRefNum,gPlugSpec.parID,"\pFavorites",&gFavFolderSpec);
	if (myErr != noErr) HiliteControl(FavoritesBtn,255);		//Unable to get Favorites Folder FileSpec
	//myErr = FSpRename(&gFavFolderSpec,"\pScoreKeeper Test");
/*	myErr = GetDirItems(gFavFolderSpec.vRefNum,gFavFolderSpec.parID,NULL,true,false,
						*items, 100, &count, &index);
*/	/*
	myMenu = NewMenu(rFavoritesButtonID,"\p");	//build new menu
  	AppendMenu(myMenu,"\p<B(Add File�");
  	AppendMenu(myMenu,"\p<B(Clear All�");
  	enumerateFolder(gFavFolderSpec.vRefNum,gFavFolderSpec.parID);
/*
	for (i=0; i<count; i++) {
		SysBeep(20);
  		AppendMenu(myMenu,items[i]->name);
	}
*/
//	InsertMenu(myMenu,kIsPopUpMenu);
//	DrawMenuBar();

//	installSetPortPatch();				//1.2b7 crash

} // InitPlugIn

void enumerateFolder(short vRefNum, long dirID)
{
     Str31      name;
     CInfoPBRec pb;
     OSErr      err;
     short      index = 1;

     //pb.dirInfo.ioNamePtr = name;
     pb.dirInfo.ioVRefNum = vRefNum;

     while (err == noErr) {
         pb.dirInfo.ioFDirIndex = index;
         pb.dirInfo.ioDrDirID = dirID; // gotta set it each time!

         err = PBGetCatInfo(&pb, false);

         if (err == noErr) {
             if ((pb.hFileInfo.ioFlAttrib & 0x10) == 0x10) // folder bit
                 { }//ProcessASubFolder(&pb);
            else {
                 pb.dirInfo.ioDrDirID = dirID; // changed by PBGetCatInfo
                 processAFile(&pb);
             }
         }

         index++;
    }
}

//To get an FSSpec for the file (or subfolder) you could do this:

void processAFile(CInfoPBRec *filePB)
{
    OSErr		err;
    FSSpec		myFSSpec;
    MenuHandle	myMenu;
     	SysBeep(20);

    err = FSMakeFSSpec(filePB->dirInfo.ioVRefNum,
                       filePB->dirInfo.ioDrDirID,
                       filePB->dirInfo.ioNamePtr,
                       &myFSSpec);
	myMenu = GetMHandle(rFavoritesButtonID);	//make sure there is a menu
	if (myMenu) AppendMenu(myMenu,myFSSpec.name);
}


pascal void setPortPatch(GrafPtr port)
{	
	Rect		myRect;
	
	
	EnterCodeResource();
	/* call through to the original SetPort */
	gOldSetPortAddr(port);
		
	if ((port == (GrafPtr)gameWindow) && UserPrefs.movEnabled) {
		if (UserPrefs.movIcon && (movieIcons[gMovieCount] != NULL)) {
			SetRect(&myRect,port->portRect.right-15,port->portRect.top+4,port->portRect.right-4,port->portRect.top+20);
			PlotCIcon(&myRect,movieIcons[gMovieCount]);
		}
		if (levelBusy) {
			gMovieFrame++;
			if (gMovieFrame>=(12-2*UserPrefs.movRate)) {				// delay
				gMovieCount = (gMovieCount + 1) % 4;
				addFrame((short)((12-2*UserPrefs.movRate)*100));		// time * 100
				gMovieFrame = 0;
			}
		}
	}
	ExitCodeResource();
}

void unInstallSetPortPatch(void)
{
	SetToolTrapAddress((UniversalProcPtr)gOldSetPortAddr, _SetPort);
}

void installSetPortPatch(void)
{
	/* patch SetPort */
	gOldSetPortAddr = (SetPortProc)GetToolTrapAddress(_SetPort);
	SetToolTrapAddress((UniversalProcPtr)setPortPatch, _SetPort);
}

void closePlugIn(ScoreInterfaceRecord *rec)
{
	MenuHandle				myMenu;
	short					i;

//	unInstallSetPortPatch();						//1.2b7 crash
	if (gPlugLevel == kPayed) WritePref();
	clearRecent();

	if (gImageDesc) DisposeHandle((Handle)gImageDesc);
	if (gCompressedData) DisposeHandle(gCompressedData);
	if (gMoviePortPtr) DisposeGWorld(gMoviePortPtr); 
	ExitMovies(); 

	if (gLogAlias) DisposeHandle((Handle)(gLogAlias));

	if (rec->resultsHandle) DisposeHandle(rec->resultsHandle);	// Dispose my empty Handle
	rec->resultsHandle = oldResultsHandle;		// reset the original scoreboard or message

	closeSpeechChannel();

	DisposeControl(PrefBtn);
	DisposeControl(SummaryBtn);
	DisposeControl(PopUpBtn);
	DisposeControl(FavoritesBtn);
	DisposeControl(RecentBtn);
	DisposeControl(AboutScrollBar);

	myMenu = GetMHandle(rPopUpMenuID);		//dispose PopUp menu
	if (myMenu) {
		DeleteMenu(rPopUpButtonID);
		DisposeMenu(myMenu);
	}
	myMenu = GetMHandle(rPrefMenuID);		//dispose Preferences menu
	if (myMenu) {
		DeleteMenu(rPrefMenuID);
		DisposeMenu(myMenu);
	}
	myMenu = GetMHandle(rSummaryMenuID);	//dispose Summary menu
	if (myMenu) {
		DeleteMenu(rSummaryMenuID);
		DisposeMenu(myMenu);
	}
	myMenu = GetMHandle(rFavoritesMenuID);	//dispose Favorites menu
	if (myMenu) {
		DeleteMenu(rFavoritesButtonID);
		DisposeMenu(myMenu);
	}
	myMenu = GetMHandle(rRecentMenuID);		//dispose Recent menu
	if (myMenu) {
		DeleteMenu(rRecentMenuID);
		DisposeMenu(myMenu);
	}
	DrawMenuBar();
	
	for (i=0; i<3; i++) {
		DisposeCIcon(ammoIcons[i]);
	}
	for (i=0; i<4; i++) {
		DisposeCIcon(movieIcons[i]);
	}
		
	resetColorPalette();
	rec->resultsChanged = true;    // force window update 
} // ClosePlugIn



void doControls(ScoreInterfaceRecord *rec, ControlHandle theCntrl)
{
	short					partCode, previousPanel, theChoice;
	DialogPtr				theDialog;
	Handle					myHandle;
	Rect					myRect;
	Point					clickPt;

	SetPort(rec->resultsWindow);
	clickPt = rec->theEvent->where;
	GlobalToLocal(&clickPt);

	if (theCntrl == PopUpBtn) {
		previousPanel = currentPanel;
		partCode = TrackControl(theCntrl,rec->theEvent->where,(ControlActionUPP) -1);
		currentPanel = GetControlValue(PopUpBtn);
		if (previousPanel != currentPanel) switchPanel(rec);
		return;
	} // (theCntrl != PopUpBtn)

	if (theCntrl == SummaryBtn) {
		theChoice = gSummaryType;
		partCode = TrackControl(theCntrl,rec->theEvent->where,(ControlActionUPP) -1);
		gSummaryType = GetControlValue(SummaryBtn);
		if (theChoice == gSummaryType) return;
		drawButtons(rec);
		TextFace(normal);
		drawSummaryPanel();
		return;
	} // (theCntrl != PopUpBtn)

	if (theCntrl == AboutScrollBar) {
		if(partCode = FindControl(clickPt,rec->resultsWindow,&theCntrl)) {
			doAboutScrollBar(partCode,rec->resultsWindow,AboutScrollBar,clickPt);
		}
		return;
	} // (theCntrl != ScrollBar)

	if (theCntrl == FavoritesBtn) {
		doFavorites();
		return;
	} // (theCntrl != FavoritesBtn)
	
	if (theCntrl == RecentBtn) {
		doRecent();
		return;
	} // (theCntrl != RecentBtn)

	if (theCntrl == PrefBtn) {
		doPreferences();
		return;
	} // (theCntrl != PrefBtn)
	
	SysBeep(20);
} // DoControls

void doInContent(ScoreInterfaceRecord *rec)
{
	Point					clickPt;
	Rect					myRect;
	short					col1, col2, i, 
							hTop = panelRect.top+1, 				// top of headerrect
							hBottom = panelRect.top+kTitleHeight-1;	// bottom of   "
	long					DragInfo;
	RgnHandle				myRgn,rectRgn1,rectRgn2;
	Str255					myStr255;
	KeyMap					myKeyMap;

	SetPort(rec->resultsWindow);
	clickPt = rec->theEvent->where;
	GlobalToLocal(&clickPt);

	switch (currentPanel) {
		case kPlayerPanel:
		case kTeamPanel:
			SetRect(&myRect,14,hTop,112,hBottom); // Player Title
			if (PtInRect(clickPt,&myRect)) { 
				if (trackTitle(clickPt,myRect)) { 
					currentPanel += 1;
					if (currentPanel>kTeamPanel) {
						currentPanel = kPlayerPanel;
					}
					SetControlValue(PopUpBtn,currentPanel);
					drawPanel(rec);
				}
			}

			SetRect(&myRect,112+1,hTop,124,hBottom); // Title Shift Left
			if (PtInRect(clickPt,&myRect)) { 
				GetKeys(myKeyMap);
				if (trackTitle(clickPt,myRect)) { 
					if(myKeyMap[1] & 0x8000) {
						startColumn = (startColumn + noColumns - 4) % noColumns; 
					} else {
						startColumn = (startColumn + noColumns - 1) % noColumns; 
					} // if KeyMap
					drawPanel(rec);
				} // if TrackTitle
			} // if PtInRect

			myRgn = NewRgn();      		// Enable column dragging
			rectRgn1 = NewRgn();      	// Enable column dragging
			rectRgn2 = NewRgn();      	// Enable column dragging
			for (col1=0; col1<4; col1++) {
				SetRect(&myRect,124+col1*42+1,hTop,124+(col1+1)*42,hBottom);
				if (PtInRect(clickPt,&myRect)) {
					InvertRect(&myRect);
					if (gSummaryType <= kAverageSummary) {
						SetRectRgn(rectRgn1,124+col1*42,panelRect.top,124+(col1+1)*42+1,panelRect.bottom);
						SetRectRgn(rectRgn2,124+col1*42,optionsRect.top,124+(col1+1)*42+1,optionsRect.bottom);
						UnionRgn(rectRgn1,rectRgn2,myRgn);
					} else {
						SetRectRgn(myRgn,124+col1*42,panelRect.top,124+(col1+1)*42+1,panelRect.bottom);
					}
					SetRect(&myRect,124,panelRect.top,292+1,panelRect.bottom);
					DragInfo = DragGrayRgn(myRgn,clickPt,&myRect,&myRect,hAxisOnly,NULL);
					clickPt.h += LoWord(DragInfo);
					col2 = (int)((clickPt.h-124)/42);
					if ((clickPt.h >=124) && (clickPt.h <= 292)) {
						if (col1 < col2) { 
							for (i = col1; i < col2; i++) { switchColumns(i, i+1); }
						}
						if (col1 > col2) { 
							for (i = col1; i > col2; i--) { switchColumns(i, i-1); }
						}
						drawPanel(rec); 
					} else {			// if clickPt OK
						SetRect(&myRect,124+col1*42+1,hTop,124+(col1+1)*42,hBottom);
						InvertRect(&myRect);
					}
				} // if PtInRgn
			} // for col1
			DisposeRgn(myRgn);
			DisposeRgn(rectRgn1);
			DisposeRgn(rectRgn2);
			
			SetRect(&myRect,292+1,hTop,303+1,hBottom); // Title Shift Right
			if (PtInRect(clickPt,&myRect)) { 
				GetKeys(myKeyMap);
				if (trackTitle(clickPt,myRect)) { 
					if(myKeyMap[1] & 0x8000) {
						startColumn = (startColumn + noColumns + 4) % noColumns; 
					} else {
						startColumn = (startColumn + noColumns + 1) % noColumns; 
					} // if KeyMap
					drawPanel(rec);
				} // if TrackTitle
			} // it PtInRect

			break; // kTeamPanel || kPlayerPanel
	} // switch currentPanel
} // DoInContent

int trackTitle(Point clickPt,Rect myRect)
{
	while (StillDown()) {
		GetMouse(&clickPt);
		if (PtInRect(clickPt,&myRect)) {
			InvertRect(&myRect);
			while (StillDown() && PtInRect(clickPt,&myRect)) {
				GetMouse(&clickPt);
			}
			InvertRect(&myRect);
		} // if PtInRect
	} // while StillDown
	return PtInRect(clickPt,&myRect);
} // TrackTitle

void drawTitleBar(void)
{
	Handle					myHandle;
	Rect					myRect;
	Str255					myStr;
	short					i = 0;
	CIconHandle				myIcon;

	TextFace(normal);
	
	SetRect(&myRect,292,panelRect.top,305,panelRect.top+kTitleHeight);	// Color column
	drawHeaderRect(&myRect);

	if (currentPanel == kPlayerStatsPanel) {
		SetRect(&myRect,13,panelRect.top,129,panelRect.top+kTitleHeight);	// Name Column
		drawHeaderRect(&myRect);
		GetIndString(myStr,rTitleStrings,1);
		ForeColor(whiteColor);
		drawCenteredString(myStr,74,panelRect.top+kTextHeight+1);
		ForeColor(blackColor);
		drawCenteredString(myStr,73,panelRect.top+kTextHeight);		
		for (i=0; i<3; i++) {
			SetRect(&myRect,124+56*i,panelRect.top,124+56*(i+1)+1,panelRect.top+kTitleHeight);
			drawHeaderRect(&myRect);
			SetRect(&myRect,124+i*56+28-9,panelRect.top+3,124+i*56+28+9,panelRect.top+15);
			PlotCIcon(&myRect,ammoIcons[i]);
		} // for i
		
	} else {
		SetRect(&myRect,13,panelRect.top,112+1,panelRect.top+kTitleHeight);	// Name Column
		drawHeaderRect(&myRect);
		SetRect(&myRect,112,panelRect.top,112+12+1,panelRect.top+kTitleHeight);	// Left arrow rect
		drawHeaderRect(&myRect);
		for (i=0; i<4; i++) {
			SetRect(&myRect,124+42*i,panelRect.top,124+42*(i+1)+1,panelRect.top+kTitleHeight);
			drawHeaderRect(&myRect);
		} // for i
		switch (currentPanel) {
			case kPlayerPanel:
				GetIndString(myStr,rTitleStrings,1);
				break;
			case kTeamPanel:
				GetIndString(myStr,rTitleStrings,2);
				break;
		} // switch
		ForeColor(whiteColor);
		drawCenteredString(myStr,64,panelRect.top+kTextHeight+1);
		ForeColor(blackColor);
		drawCenteredString(myStr,63,panelRect.top+kTextHeight);
		
		myIcon = GetCIcon(rLeftArrow);
		if (myIcon != NULL) {
			SetRect(&myRect,112+6-4,panelRect.top+3,112+6+4,panelRect.top+15);
			PlotCIcon(&myRect,myIcon);
			DisposeCIcon(myIcon);
		}
		myIcon = GetCIcon(rRightArrow);
		if (myIcon != NULL) {
			SetRect(&myRect,294,panelRect.top+3,294+8,panelRect.top+15);
			PlotCIcon(&myRect,myIcon);
			DisposeCIcon(myIcon);
		}
		
		for (i=0; i<4; i++) {
			switch (columnOrder[(startColumn+i)%noColumns]) {
				case 0:
				case 1:
				case 2:
					SetRect(&myRect,124+i*42+21-9,panelRect.top+3,124+i*42+21+9,panelRect.top+15);
					PlotCIcon(&myRect,ammoIcons[columnOrder[(startColumn+i)%noColumns]]);
					break;
				default:
					GetIndString(myStr,rTitleStrings,columnOrder[(startColumn+i)%noColumns]);
					ForeColor(whiteColor);
					drawCenteredString(myStr,124+i*42+21,panelRect.top+kTextHeight+1);
					ForeColor(blackColor);
					drawCenteredString(myStr,124+i*42+21,panelRect.top+kTextHeight);
			} // switch
		} // for i
	} // if

	SetRect(&myRect,304,panelRect.top,346,panelRect.top+kTitleHeight);	// Total Column
	drawHeaderRect(&myRect);
	GetIndString(myStr,rTitleStrings,8);
	ForeColor(whiteColor);
	drawCenteredString(myStr,325,panelRect.top+kTextHeight+1);
	ForeColor(blackColor);
	drawCenteredString(myStr,324,panelRect.top+kTextHeight);
} // DrawTitleBar

void drawCenteredString(Str255 myStr,short x,short y)
{
	MoveTo(x-(int)(StringWidth(myStr)/2),y);
	DrawString(myStr);
} // DrawCenteredString

void stringTrunc(Str255 theString,short maxLength)
{
	if (StringWidth(theString) > maxLength) {
		while (StringWidth(theString) > (maxLength - StringWidth("\p�"))) {
			theString[0] = (int)(theString[0]-1);
		}
		theString[0] = (int)(theString[0]+1);
		theString[theString[0]] = (char)'�'; //201;
	} // if StringWidth
} // StringTrunc

void switchColumns(short col1, short col2)
{
	short			temp;
	
	temp = columnOrder[(startColumn+col1)%noColumns];
	columnOrder[(startColumn+col1)%noColumns] = columnOrder[(startColumn+col2)%noColumns];
	columnOrder[(startColumn+col2)%noColumns] = temp;
} // SwitchColumns

void changePort(void)
{
	GetPort(&savePort);
	SetPort(plugWindow);
	TextFont(geneva);
	TextSize(9);
} // ChangePort

void updateGameScores(void)
{
	switch (currentPanel) {
		case kPlayerPanel:
			drawSharedInterface();
			drawPlayerPanel();
			drawSummaryPanel();
			break;
		case kTeamPanel:
			drawSharedInterface();
			drawTeamPanel();
			drawSummaryPanel();
			break;
		case kPlayerStatsPanel:
			drawSharedInterface();
			drawPlayerStatsPanel();
			drawSummaryPanel();
			break;
	} // switch currentpanel
} // UpdateGameScores


void drawAboutPanel(int howMuch)
{
	CGrafPtr				windowPortPtr;
	GDHandle				deviceHdl;
	QDErr					qdErr;
	GWorldPtr				gworldPortPtr;
	PixMapHandle			gworldPixMapHdl;
	Boolean					lockPixResult;
	Rect					portRect, myRect;	
	Str255					errorString;

	if (!aboutPict) {
		doPanelError(kPictLoadError,resNotFound);
		return;
	} // NULL Handle

	portRect = aboutPanelRect;
	portRect.right -= portRect.left;
	portRect.left = 0;
	portRect.bottom -= portRect.top;
	portRect.top = 0;
	
	aboutPanelRect.right += 15;
	drawShadowRect(aboutPanelRect);
	aboutPanelRect.right -= 15;
	
	GetGWorld(&windowPortPtr,&deviceHdl);
	BackColor(whiteColor);

	qdErr = NewGWorld(&gworldPortPtr,0,&portRect,NULL,NULL,0);
	if(gworldPortPtr == NULL || qdErr != noErr) {
		doPanelError(kQuickDrawError,qdErr);
		return;
	} // gWorldPort error

	SetGWorld(gworldPortPtr,NULL);

	gworldPixMapHdl = GetGWorldPixMap(gworldPortPtr);
	if(!(lockPixResult = LockPixels(gworldPixMapHdl))) {
		doPanelError(kQuickDrawError,0);
		return;
	} // lockPixels error

	BackColor(whiteColor);		// empty the offscreen graph port
	EraseRect(&(gworldPortPtr->portRect));	

	myRect = aboutRect;
	myRect.top -= howMuch;
	myRect.bottom -= howMuch;
	
	DrawPicture(aboutPict, &myRect);
	
	ForeColor(blackColor);		// frame the offscreen graph port
	FrameRect(&portRect);

	SetGWorld(windowPortPtr,deviceHdl);

	CopyBits(&((GrafPtr) gworldPortPtr)->portBits,&((GrafPtr) windowPortPtr)->portBits,
			 &portRect,&aboutPanelRect,srcCopy,NULL);

	UnlockPixels(gworldPixMapHdl);
	DisposeGWorld(gworldPortPtr);

	if(QDError() != noErr) {
		doPanelError(kQuickDrawError,QDError());
		return;
	} // QuickDraw error

	PmBackColor(3);
}

void doPanelError(int errorType, int errorNum)
{
	Str255				theError;

	SetPort(plugWindow);
	ForeColor(whiteColor);
	PaintRect(&panelRect);
	ForeColor(redColor);
	GetIndString(theError,rErrorStrings,kPanelError);
	drawCenteredString(theError,180,85);
	GetIndString(theError,rErrorStrings,kQuickDrawError);
	drawCenteredString(theError,180,100);
	ForeColor(blackColor);
	FrameRect(&panelRect);
}

void doErrorAlert(int resID,int itemNum)
{
	Str255			theError;
	
	ShowCursor();
	GetIndString(theError,resID,itemNum);
	ParamText(theError,"\p","\p","\p");
	NoteAlert(rErrorAlert,NULL);
}

void doPrefErrorAlert(int resID,int itemNum)
{
	Str255			theError;
	short 			itemHit;
	
	ShowCursor();
	GetIndString(theError,resID,itemNum);
	ParamText(theError,"\p","\p","\p");
	itemHit = NoteAlert(rPrefErrorAlert,NULL);
	if (itemHit == 2) {
		if(!doPreferencesDialog(gSelectedPrefs)) SysBeep(20);
	}
}

short doCancelOKAlert(int resID,int itemNum)
{
	Str255			theError;
	short 			itemHit;
	
	ShowCursor();
	GetIndString(theError,resID,itemNum);
	ParamText(theError,"\p","\p","\p");
	itemHit = CautionAlert(rCancelOKAlert,NULL);
	return itemHit;
}


// �������������������������������������������������������������������� doAboutScrollBar

void  doAboutScrollBar(ControlPartCode partCode,WindowPtr windowPtr,ControlHandle controlHdl,
									 Point mouseXY)
{
	SInt16				oldControlValue;
	SInt16				scrollDistance;

	switch(partCode)
	{
		case kControlIndicatorPart:
			oldControlValue = GetControlValue(controlHdl);
			if(TrackControl(controlHdl,mouseXY,NULL))
			{
				scrollDistance = oldControlValue - GetControlValue(controlHdl);
				if(scrollDistance != 0)
				{
					drawAboutPanel(GetCtlValue(controlHdl));
				}
			}
			break;

		case kControlUpButtonPart:
		case kControlDownButtonPart:
		case kControlPageUpPart:
		case kControlPageDownPart:
			TrackControl(controlHdl,mouseXY,&aboutScrollboxActionProcedure);
			break;
	}
	PmBackColor(3);
}

// ����������������������������������������������������������� aboutScrollboxActionProcedure

pascal void  aboutScrollboxActionProcedure(ControlHandle controlHdl,ControlPartCode partCode)
{
	SInt16				scrollDistance;
	SInt16 				controlValue;
	//RgnHandle			updateRegion;

	if(partCode)
	{

		switch(partCode)
		{
			case kControlUpButtonPart:
			case kControlDownButtonPart:
				scrollDistance = 16;
				break;

			case kControlPageUpPart:
			case kControlPageDownPart:
				scrollDistance = (aboutPanelRect.bottom - aboutPanelRect.top) - 8;
				break;
		}

		if((partCode == kControlDownButtonPart) || (partCode == kControlPageDownPart))
			scrollDistance = -scrollDistance;

		controlValue = GetControlValue(controlHdl);
		if(((controlValue == GetControlMaximum(controlHdl)) && scrollDistance < 0) || 
			 ((controlValue == GetControlMinimum(controlHdl)) && scrollDistance > 0))
			return;

		doMoveScrollBox(controlHdl,scrollDistance);

		drawAboutPanel(GetCtlValue(controlHdl));
	}
}

// ���������������������������������������������������������������������� doMoveScrollBox

void doMoveScrollBox(ControlHandle controlHdl,SInt16 scrollDistance)
{
	SInt16		oldControlValue, controlValue, controlMax;

	oldControlValue = GetControlValue(controlHdl);
	controlMax = GetControlMaximum(controlHdl);

	controlValue = oldControlValue - scrollDistance;
	
	if(controlValue < 0)
		controlValue = 0;
	else if(controlValue > controlMax)
		controlValue = controlMax;

	SetCtlValue(controlHdl,controlValue);
}

// ��������������������������������������������������������������������������������������

Boolean doPreferencesDialog(short thePanel)
{
	DialogPtr					modalDlgPtr;
	SInt16						itemType, itemHit, partCode;
	Handle						itemHdl;
	Rect						itemRect;
	ControlHandle				theCntrl;
	Point						clickPt;
	GrafPtr						oldPort;
	MenuHandle					myMenu;
	Str255						myStr;
	short						i, temp;
	SInt32						finalTicks;
	struct PreferencesType		TempPrefs;
	Boolean						colorsChanged = false;

	if (gPlugLevel != kPayed) {
		doErrorAlert(rPrefErrors,4);
	}
	
	//openSpeechChannel(&UserPrefs);
	TempPrefs = UserPrefs;
	gSelectedPrefs = thePanel;
	
	myMenu = GetMHandle(rPopUpMenuID);
	for (i=1; i<=kPopUpMenuLength;i++) {
		SetItemStyle(myMenu,i,normal);
	} // for i

	myMenu = NewMenu(rLogFormatID,"\pLog format");
	if (myMenu != NULL) {
		for (i=1; i<=kLogFormatLength;i++) {
			GetIndString(myStr,rLogFormatID,i);
			AppendMenu(myMenu,myStr);
		} // for i
		InsertMenu(myMenu,-1);
		DrawMenuBar();
	} else {
		SysBeep(20);
	}

	insertVoicesMenu(&UserPrefs);

	if(!(modalDlgPtr = GetNewDialog(rPrefDialogID,NULL,(WindowPtr) -1)))
		return(false);
	
	PrefScrollBar = GetNewControl(rPrefScrollBarID,(WindowPtr)modalDlgPtr);
	//SetCtlMax(PrefScrollBar,kPrefCount);
	doMoveScrollBox(PrefScrollBar,-gSelectedPrefs);
	
	GetDialogItem(modalDlgPtr,iOKDefault,&itemType,&itemHdl,&itemRect);
	SetDialogItem(modalDlgPtr,iOKDefault,itemType,(Handle) &preferencesUserItems,&itemRect);

	GetDialogItem(modalDlgPtr,iPrefIcons,&itemType,&itemHdl,&PrefIconsRect);
	SetDialogItem(modalDlgPtr,iPrefIcons,itemType,(Handle) &preferencesUserItems,&PrefIconsRect);
	PrefIconsRect.right -= 16;  // scrollbar area
				
	ShowWindow(modalDlgPtr);
	GetPort(&oldPort);
	SetPort(modalDlgPtr);

	switchPreferencesPanel(modalDlgPtr,gSelectedPrefs,&TempPrefs);
	drawPreferencesPanel(GetCtlValue(PrefScrollBar), gSelectedPrefs);

	do
	{
		ModalDialog((ModalFilterUPP) &eventFilter,&itemHit);
		switch (itemHit) {
			case iPrefIcons:
				GetMouse(&clickPt);
				if (FindControl(clickPt,(WindowPtr)modalDlgPtr,&theCntrl)) {
					if (theCntrl == PrefScrollBar) {
						if(partCode = FindControl(clickPt,(WindowPtr)modalDlgPtr,&theCntrl)) {
							doPrefScrollBar(partCode,(WindowPtr)modalDlgPtr,theCntrl,clickPt);
						}
					} // (theCntrl != scrollBar)
				} else { // no control found
					doInPrefTopics(clickPt,GetCtlValue(PrefScrollBar),TempPrefs,modalDlgPtr);
				}
				break;
			case iDefault:
				defaultPreferences(&TempPrefs,gSelectedPrefs);
				switchPreferencesPanel(modalDlgPtr,gSelectedPrefs,&TempPrefs);
				break;
			default :
				GetDialogItem(modalDlgPtr,itemHit,&itemType,&itemHdl,&itemRect);
				switch (itemType) {
					case (ctrlItem+chkCtrl) : 
						SetCtlValue((ControlHandle)itemHdl,!GetControlValue((ControlHandle)itemHdl));
						break;
					case (ctrlItem+radCtrl) : 
						SetCtlValue((ControlHandle)itemHdl,true);
						i = itemHit;
						do {
							i--;
							GetDialogItem(modalDlgPtr,i,&itemType,&itemHdl,&itemRect);
							if (itemType == (ctrlItem+radCtrl)) {
								SetCtlValue((ControlHandle)itemHdl,false);
							}
						} while ((i>4) && (itemType == (ctrlItem+radCtrl)));
						i = itemHit;
						do {
							i++;
							GetDialogItem(modalDlgPtr,i,&itemType,&itemHdl,&itemRect);
							if (itemType == (ctrlItem+radCtrl)) {
								SetCtlValue((ControlHandle)itemHdl,false);
							}
						} while ((i<CountDITL(modalDlgPtr)) &&(itemType == (ctrlItem+radCtrl)));
						break;
				} // switch itemType
				switch (gSelectedPrefs) {
					case kGeneralPrefs :
						GetDialogItem(modalDlgPtr,itemHit,&itemType,&itemHdl,&itemRect);
						switch (itemHit-iPrefIcons) {
							case 1: // gnlNegRed
								TempPrefs.gnlNegRed = GetControlValue((ControlHandle)itemHdl);
								break;
							case 2: // gnlUpdate
								TempPrefs.gnlUpdate = GetControlValue((ControlHandle)itemHdl);
								break;
							case 3: // gnlNoClan
								TempPrefs.gnlRmveClan = GetControlValue((ControlHandle)itemHdl);
								break;
							case 4: // gnlSwitch
								TempPrefs.gnlSwitch = GetControlValue((ControlHandle)itemHdl);
									GetDialogItem(modalDlgPtr,5+iPrefIcons,&itemType,&itemHdl,&itemRect);
									if (TempPrefs.gnlSwitch) {
										HiliteControl((ControlHandle)itemHdl,0);
									} else {
										HiliteControl((ControlHandle)itemHdl,255);
									} // else
								break;
							case 5: // gnlPanel
								TempPrefs.gnlPanel = GetControlValue((ControlHandle)itemHdl);
								break;
						} // switch itemHit
						break;
					case kColorPrefs :
						switch (itemHit-iPrefIcons) {
							case 1: //clrEnabled
								GetDialogItem(modalDlgPtr,itemHit,&itemType,&itemHdl,&itemRect);
								TempPrefs.clrEnabled = GetControlValue((ControlHandle)itemHdl);
								for (i=2; i<=7; i++) {
									GetDialogItem(modalDlgPtr,i+iPrefIcons,&itemType,&itemHdl,&itemRect);
									if (TempPrefs.clrEnabled) {
										HiliteControl((ControlHandle)itemHdl,0);
									} else {
										HiliteControl((ControlHandle)itemHdl,255);
									} // else
								} // for
								break;
							case 2:
							case 3:
							case 4:
							case 5:
							case 6:
							case 7:
							case 8:
							case 9:
								if (askTeamColor(modalDlgPtr,itemHit,itemHit-iPrefIcons-2,&TempPrefs)) {
									colorsChanged = true;
									switchPreferencesPanel(modalDlgPtr,gSelectedPrefs,&TempPrefs);
								}
								break;
						} // switch itemHit
						break;
					//case kSoundPrefs : break;
					case kSpeechPrefs :
						GetDialogItem(modalDlgPtr,itemHit,&itemType,&itemHdl,&itemRect);
						switch (itemHit-iPrefIcons) {
							case 4: // spkSkipLT
								TempPrefs.spkSkipLT = GetControlValue((ControlHandle)itemHdl);
								break;
							case 5: // spkEnabled
								TempPrefs.spkEnabled = GetControlValue((ControlHandle)itemHdl);

								if (TempPrefs.spkEnabled) {
									GetDialogItem(modalDlgPtr,4+iPrefIcons,&itemType,&itemHdl,&itemRect);
									HiliteControl((ControlHandle)itemHdl,0);
									GetDialogItem(modalDlgPtr,2+iPrefIcons,&itemType,&itemHdl,&itemRect);
									HiliteControl((ControlHandle)itemHdl,0);
								} else {
									GetDialogItem(modalDlgPtr,4+iPrefIcons,&itemType,&itemHdl,&itemRect);
									HiliteControl((ControlHandle)itemHdl,255);
									GetDialogItem(modalDlgPtr,2+iPrefIcons,&itemType,&itemHdl,&itemRect);
									HiliteControl((ControlHandle)itemHdl,255);
								}

								if ((TempPrefs.spkEnabled) && (TempPrefs.spkVoice > 2)) {
									GetDialogItem(modalDlgPtr,1+iPrefIcons,&itemType,&itemHdl,&itemRect);
									HiliteControl((ControlHandle)itemHdl,0);
									GetDialogItem(modalDlgPtr,3+iPrefIcons,&itemType,&itemHdl,&itemRect);
									HiliteControl((ControlHandle)itemHdl,0);
								} else {
									GetDialogItem(modalDlgPtr,1+iPrefIcons,&itemType,&itemHdl,&itemRect);
									HiliteControl((ControlHandle)itemHdl,255);
									GetDialogItem(modalDlgPtr,3+iPrefIcons,&itemType,&itemHdl,&itemRect);
									HiliteControl((ControlHandle)itemHdl,255);
								}
								break;
							case 2: // spkVoice
								TempPrefs.spkVoice = GetControlValue((ControlHandle)itemHdl);
								if (TempPrefs.spkVoice > 2) {
									GetDialogItem(modalDlgPtr,1+iPrefIcons,&itemType,&itemHdl,&itemRect);
									HiliteControl((ControlHandle)itemHdl,0);
									GetDialogItem(modalDlgPtr,3+iPrefIcons,&itemType,&itemHdl,&itemRect);
									HiliteControl((ControlHandle)itemHdl,0);
								} else {
									GetDialogItem(modalDlgPtr,1+iPrefIcons,&itemType,&itemHdl,&itemRect);
									HiliteControl((ControlHandle)itemHdl,255);
									GetDialogItem(modalDlgPtr,3+iPrefIcons,&itemType,&itemHdl,&itemRect);
									HiliteControl((ControlHandle)itemHdl,255);
								}
								break;
							case 3: // spkRate
								TempPrefs.spkRate = GetControlValue((ControlHandle)itemHdl);
								break;
							case 1: // Test Voice
								speakTestString(&TempPrefs);
								break;
						} // switch itemHit
						break;
					case kLogPrefs : 
						switch (itemHit-iPrefIcons) {
							case 1 :
								GetDialogItem(modalDlgPtr,itemHit,&itemType,&itemHdl,&itemRect);
								TempPrefs.logEnabled = GetControlValue((ControlHandle)itemHdl);
								for (i=2; i<=6; i++) {
									GetDialogItem(modalDlgPtr,i+iPrefIcons,&itemType,&itemHdl,&itemRect);
									if (TempPrefs.logEnabled) {
										HiliteControl((ControlHandle)itemHdl,0);
									} else {
										HiliteControl((ControlHandle)itemHdl,255);
									}
								}
								// GetDialogItem(modalDlgPtr,kLogOneFile,&itemType,&itemHdl,&itemRect);
								// HiliteControl((ControlHandle)itemHdl,255);
								if (TempPrefs.logTime == kLogEachGame) {
									GetDialogItem(modalDlgPtr,6+iPrefIcons,&itemType,&itemHdl,&itemRect);
									HiliteControl((ControlHandle) itemHdl,255);
								}
								break;
							case 2 :
								GetDialogItem(modalDlgPtr,itemHit,&itemType,&itemHdl,&itemRect);
								TempPrefs.logFormat = GetControlValue((ControlHandle)itemHdl);
								break;
							case kLogEachGame :
								GetDialogItem(modalDlgPtr,6+iPrefIcons,&itemType,&itemHdl,&itemRect);
								HiliteControl((ControlHandle) itemHdl,255);
								TempPrefs.logTime = itemHit-iPrefIcons;
								break;
							case kLogEachSession :
							case kLogOneFile :
								GetDialogItem(modalDlgPtr,6+iPrefIcons,&itemType,&itemHdl,&itemRect);
								HiliteControl((ControlHandle) itemHdl,kControlButtonPart);
								Delay(8,&finalTicks);
								HiliteControl((ControlHandle) itemHdl,0);
								temp = TempPrefs.logTime;
								TempPrefs.logTime = itemHit-iPrefIcons;
							case 6 :
								SelectLogFile(&TempPrefs);

								if ((TempPrefs.logFlag == kFileCancelled) && (itemHit != (6+iPrefIcons))) {
									TempPrefs.logTime = temp;
									for (i=kLogEachGame; i<=kLogOneFile; i++) {
										GetDialogItem(modalDlgPtr,i+iPrefIcons,&itemType,&itemHdl,&itemRect);
										SetCtlValue((ControlHandle)itemHdl,false);
									}
									GetDialogItem(modalDlgPtr,TempPrefs.logTime+iPrefIcons,&itemType,&itemHdl,&itemRect);
									SetCtlValue((ControlHandle)itemHdl,true);
									if (TempPrefs.logTime == kLogEachGame) {
										GetDialogItem(modalDlgPtr,6+iPrefIcons,&itemType,&itemHdl,&itemRect);
										HiliteControl((ControlHandle) itemHdl,255);
									}
								}
								drawPreferencesPanel(GetCtlValue(PrefScrollBar), gSelectedPrefs);
							}		// kLogPrefs	
							break;
					case kMoviePrefs : 
						switch (itemHit-iPrefIcons) {
							case 1 :
								TempPrefs.movEnabled = GetControlValue((ControlHandle)itemHdl);
								GetDialogItem(modalDlgPtr,2+iPrefIcons,&itemType,&itemHdl,&itemRect);
								if (TempPrefs.movEnabled) {
									HiliteControl((ControlHandle)itemHdl,0);
								} else {
									HiliteControl((ControlHandle)itemHdl,255);
								}
								GetDialogItem(modalDlgPtr,4+iPrefIcons,&itemType,&itemHdl,&itemRect);
								if (TempPrefs.movEnabled) {
									HiliteControl((ControlHandle)itemHdl,0);
								} else {
									HiliteControl((ControlHandle)itemHdl,255);
								}
								GetDialogItem(modalDlgPtr,3+iPrefIcons,&itemType,&itemHdl,&itemRect);
								if (TempPrefs.movEnabled) {
									HiliteControl((ControlHandle) itemHdl,kControlButtonPart);
									Delay(8,&finalTicks);
									HiliteControl((ControlHandle) itemHdl,0);
								} else {
									HiliteControl((ControlHandle)itemHdl,255);
								}
								//break;
							case 3 :
								if (TempPrefs.movEnabled) {
									selectMovieFile(&TempPrefs);
									if (TempPrefs.movFlag != kFileSelected) {
										TempPrefs.movEnabled = 0;
										GetDialogItem(modalDlgPtr,1+iPrefIcons,&itemType,&itemHdl,&itemRect);
										SetCtlValue((ControlHandle)itemHdl,false);
										
										GetDialogItem(modalDlgPtr,2+iPrefIcons,&itemType,&itemHdl,&itemRect);
										HiliteControl((ControlHandle)itemHdl,255);
										
										GetDialogItem(modalDlgPtr,4+iPrefIcons,&itemType,&itemHdl,&itemRect);
										HiliteControl((ControlHandle)itemHdl,255);
										
										GetDialogItem(modalDlgPtr,3+iPrefIcons,&itemType,&itemHdl,&itemRect);
										HiliteControl((ControlHandle)itemHdl,255);
									}
								}
								break;
							case 2 :
								TempPrefs.movIcon = GetControlValue((ControlHandle)itemHdl);
								break;
							case 4: // movRate
								TempPrefs.movRate = GetControlValue((ControlHandle)itemHdl);
								break;
						} // kMoviePrefs
				} // switch gSelectedPrefs
				break;
		} // switch itemHit
	} while((itemHit != iOK) && (itemHit != iCancel));

	DisposeDialog(modalDlgPtr);

	if(itemHit == iOK) {
		if (gPlugLevel == kPayed) {
			colorsChanged = colorsChanged || (UserPrefs.clrEnabled != TempPrefs.clrEnabled);
			UserPrefs = TempPrefs;						// Use the new preferences
			if (colorsChanged) updateColorPalette(&UserPrefs);
			changeVoice(&UserPrefs);
		}
	}
	/*if ((UserPrefs.logFlag == kFileCancelled) || (itemHit != iOK)) {
		// Restore the old file settings
	}*/
	
	myMenu = GetMHandle(rPopUpMenuID);
	if (myMenu) {
		for (i=1; i<=kPopUpMenuLength;i++) {
			SetItemStyle(myMenu,i,bold);
		} // for i
	}

	myMenu = GetMHandle(rLogFormatID);
	if (myMenu) {
		DeleteMenu(rLogFormatID);
		DisposeMenu(myMenu);
	}
	
	myMenu = GetMHandle(2022);
	if (myMenu) {
		DeleteMenu(2022);
		DisposeMenu(myMenu);
	}
	
	DrawMenuBar();
		
	SetPort(oldPort);
	closeSpeechChannel();
	return(true);
} // doPreferencesDialog



// ������������������������������������������������������������� preferencesUserItems

pascal void preferencesUserItems(DialogPtr dialogPtr,SInt16 theItem)
{
	WindowPtr		oldPort;
	PenState		oldPenState;
	SInt16			itemType;
	Handle			itemHandle;
	Rect			itemRect;
	SInt8			buttonOval;
	short			i;

	EnterCodeResource();
	GetPort(&oldPort);
	GetPenState(&oldPenState);
	GetDialogItem(dialogPtr,iOK,&itemType,&itemHandle,&itemRect);
	SetPort((*(ControlHandle) itemHandle)->contrlOwner);

	switch(theItem) {
		case iOKDefault : // OK Button Default
			InsetRect(&itemRect,-4,-4);
			buttonOval = (itemRect.bottom - itemRect.top) / 2 + 2;
			PenSize(3,3);
			FrameRoundRect(&itemRect,buttonOval,buttonOval);
			PenSize(1,1);
			break;
		case iPrefIcons:
			drawPreferencesPanel(GetCtlValue(PrefScrollBar), gSelectedPrefs);
			SetRect(&itemRect,105,45,370,185);
			ForeColor(whiteColor);
			PaintRect(&itemRect);
			drawShadowRect(itemRect);
			DrawControls((WindowPtr)dialogPtr);
			break;
	}
	SetPenState(&oldPenState);
	SetPort(oldPort);
	ExitCodeResource();
}

// �������������������������������������������������������������������������� eventFilter

pascal Boolean eventFilter(DialogPtr dialogPtr,EventRecord *eventRecPtr,SInt16 *itemHit)
{
	SInt8			charCode;
	SInt16			itemType;
	Handle			itemHandle;
	Rect			itemRect;
	SInt32			finalTicks;
	SInt16			handledEvent;

	EnterCodeResource();
	handledEvent = false;

	if((eventRecPtr->what == updateEvt) && ((WindowPtr) eventRecPtr->message != dialogPtr))
	{
		doUpdate(eventRecPtr);
	} else {
		switch(eventRecPtr->what) {
			case keyDown:
			case autoKey:
				charCode = eventRecPtr->message & charCodeMask;
				if((charCode == (SInt8) kReturn) || (charCode == (SInt8) kEnter)) {
					GetDialogItem(dialogPtr,iOK,&itemType,&itemHandle,&itemRect);
					HiliteControl((ControlHandle) itemHandle,kControlButtonPart);
					Delay(8,&finalTicks);
					HiliteControl((ControlHandle) itemHandle,0);
					handledEvent = true;
					*itemHit = iOK;
				}
				if((charCode == (SInt8) kEscape) || 
					 ((eventRecPtr->modifiers & cmdKey) && (charCode == (SInt8) kPeriod))) {
					GetDialogItem(dialogPtr,iCancel,&itemType,&itemHandle,&itemRect);
					HiliteControl((ControlHandle) itemHandle,kControlButtonPart);
					Delay(8,&finalTicks);
					HiliteControl((ControlHandle) itemHandle,0);
					handledEvent = true;
					*itemHit = iCancel;
				}
				//	Other keyboard equivalents handled here.
				break;
			// Disk-inserted and other events handled here.
		}
	}

	ExitCodeResource();
	return(handledEvent);
}

// ����������������������������������������������������������������������������� doUpdate

void  doUpdate(EventRecord *eventRecPtr)
{
	WindowPtr		windowPtr;

	windowPtr = (WindowPtr) eventRecPtr->message;

	if(((WindowPeek) windowPtr)->windowKind == dialogKind) {
		BeginUpdate(windowPtr);
		UpdateDialog(windowPtr,windowPtr->visRgn);
		EndUpdate(windowPtr);
	}
}

// ������������������������������������������������������������������������� doInPrefTopics

void  doInPrefTopics(Point clickPt,short howMuch,struct PreferencesType Prefs,DialogPtr theDialog)
{
	short						i;
	Rect						iconRect, nameRect;
	Str255						myStr;
	RGBColor					lightBlueRGB;
	Handle						newItems;
	SInt16						itemType;
	Handle						itemHdl;
	Rect						itemRect;

	short					leapPoints = 67, firstIcon = 60;

	SetPort((WindowPtr)theDialog);
	TextFont(geneva);
	TextSize(9);
	lightBlueRGB.red = lightBlueRGB.green = 52851;
	lightBlueRGB.blue = 65535;
	
	for (i=0; i<2; i++) {
		GetIndString(myStr,rPrefTopicStrings,i+howMuch+1);
		SetRect(&iconRect,30,firstIcon+i*leapPoints,62,firstIcon+32+i*leapPoints);
		SetRect(&nameRect,44-(int)(StringWidth(myStr)/2),firstIcon+35+i*leapPoints,47+(int)(StringWidth(myStr)-StringWidth(myStr)/2),firstIcon+48+i*leapPoints);

		if ((PtInRect(clickPt,&iconRect) || PtInRect(clickPt,&nameRect)) &&
			(gSelectedPrefs != i+howMuch)) {
			while (StillDown()) {
				GetMouse(&clickPt);
				if (PtInRect(clickPt,&iconRect) || PtInRect(clickPt,&nameRect)) {
					InsetRect(&iconRect,-3,-3);
					PenSize(2,2);
					ForeColor(blackColor);
					FrameRect(&iconRect);
					InsetRect(&iconRect,3,3);
					PenSize(1,1);
					//RGBForeColor(&lightBlueRGB);
               		//PaintRect(&nameRect);
					//ForeColor(blackColor);
					//drawCenteredString(myStr,46,firstIcon+44+i*leapPoints);
					LMSetHiliteMode(LMGetHiliteMode() ^ 0x80);
					InvertRect(&nameRect);
					
					while (StillDown() && (PtInRect(clickPt,&iconRect) || PtInRect(clickPt,&nameRect))) {
						GetMouse(&clickPt);
					}
					
					InsetRect(&iconRect,-3,-3);
					PenSize(2,2);
					ForeColor(whiteColor);
					FrameRect(&iconRect);
					InsetRect(&iconRect,3,3);
					PenSize(1,1);
					//PaintRect(&nameRect);
					//ForeColor(blackColor);
					//drawCenteredString(myStr,46,firstIcon+44+i*leapPoints);
					LMSetHiliteMode(LMGetHiliteMode() ^ 0x80);
					InvertRect(&nameRect);
					
				} // if PtInRect
			} // while StillDown
			if ((PtInRect(clickPt,&iconRect) || PtInRect(clickPt,&nameRect)) 
														&& gSelectedPrefs != (i+howMuch)) {
				switchPreferencesPanel(theDialog,i+howMuch,&Prefs);
				drawPreferencesPanel(howMuch, gSelectedPrefs);
			}
			break;
		} // if PtInRect
	} // for i
	TextFont(0);  // sytem font
	TextSize(12);
}

// ������������������������������������������������������������������������� doPrefScrollBar

void  doPrefScrollBar(ControlPartCode partCode,WindowPtr windowPtr,ControlHandle controlHdl,
									 Point mouseXY)
{
	SInt16				oldControlValue;
	SInt16				scrollDistance;
	RgnHandle			updateRegion;
	SInt32				finalTicks;

	switch(partCode)
	{
		case kControlIndicatorPart:
			oldControlValue = GetControlValue(controlHdl);
			if(TrackControl(controlHdl,mouseXY,NULL))
			{
				scrollDistance = oldControlValue - GetControlValue(controlHdl);
				if(scrollDistance != 0)
				{
					drawPreferencesPanel(GetCtlValue(controlHdl), gSelectedPrefs);
					Delay(20,&finalTicks);
				}
			}
			break;

		case kControlUpButtonPart:
		case kControlDownButtonPart:
		case kControlPageUpPart:
		case kControlPageDownPart:
			TrackControl(controlHdl,mouseXY,&prefScrollBarActionProcedure);
			Delay(20,&finalTicks);
			break;
	}
}

// ����������������������������������������������������������� aboutScrollboxActionProcedure

pascal void prefScrollBarActionProcedure(ControlHandle controlHdl,ControlPartCode partCode)
{
	SInt16				scrollDistance;
	SInt16 				controlValue;
	RgnHandle			updateRegion;

	if(partCode)
	{

		switch(partCode)
		{
			case kControlUpButtonPart:
			case kControlDownButtonPart:
			case kControlPageUpPart:
			case kControlPageDownPart:
				scrollDistance = 1;
				break;
		}

		if((partCode == kControlDownButtonPart) || (partCode == kControlPageDownPart))
			scrollDistance = -scrollDistance;

		controlValue = GetControlValue(controlHdl);
		if(((controlValue == GetControlMaximum(controlHdl)) && scrollDistance < 0) || 
			 ((controlValue == GetControlMinimum(controlHdl)) && scrollDistance > 0))
			return;

		doMoveScrollBox(controlHdl,scrollDistance);
		drawPreferencesPanel(GetCtlValue(controlHdl), gSelectedPrefs);
	}
}

void drawPreferencesPanel(int howMuch,int selPrefs)
{
	Rect					myRect;
	CIconHandle				topicIcons[kPrefCount];
	short					i;
	Str255					myStr;
	RGBColor				myRGB;
	
	short					leapPoints = 67, firstIcon = 60;
	
	for (i=0; i<kPrefCount; i++) {
		topicIcons[i] = GetCIcon(rPrefTopicIcons+i);
	}

	ForeColor(whiteColor);
	SetRect(&myRect,11,46,78,184);
	PaintRect(&myRect);
	ForeColor(blackColor);

	TextFont(geneva);
	TextSize(9);
	myRect.right += 16;  // scrollbar area
	InsetRect(&myRect,-1,-1);
	drawShadowRect(myRect);

	BackColor(whiteColor);
	
	for (i=0; i<2; i++) {
		SetRect(&myRect,30,firstIcon+i*leapPoints,62,firstIcon+32+i*leapPoints);
		if (topicIcons[i+howMuch] != NULL) {
			PlotCIcon(&myRect,topicIcons[i+howMuch]);
		} else {
			FrameRect(&myRect);
		}
		GetIndString(myStr,rPrefTopicStrings,i+howMuch+1);
		if ((i+howMuch) == selPrefs) {
			InsetRect(&myRect,-3,-3);
			PenSize(2,2);
			FrameRect(&myRect);
			PenSize(1,1);
			/*myRGB.red = myRGB.green = 52851;
			myRGB.blue = 65535;
			RGBForeColor(&myRGB);*/
			SetRect(&myRect,44-(int)(StringWidth(myStr)/2),firstIcon+35+i*leapPoints,47+(int)(StringWidth(myStr)-StringWidth(myStr)/2),firstIcon+48+i*leapPoints);
			//PaintRect(&myRect);
			LMSetHiliteMode(LMGetHiliteMode() ^ 0x80);
			InvertRect(&myRect);
		}
		ForeColor(blackColor);
		drawCenteredString(myStr,46,firstIcon+44+i*leapPoints);
	}
	
	for (i=0; i<kPrefCount; i++) {
		DisposeCIcon(topicIcons[i]);
	}
	TextFont(0);  // system font
	TextSize(12);
}

void switchPreferencesPanel(DialogPtr theDialog,short newPanel,struct PreferencesType *Prefs)
{
	Handle						itemHdl;
	SInt16						itemType;
	Rect						itemRect;
	short						i;

	itemHdl = GetResource('DITL',rPrefDITLs+newPanel);
	if (itemHdl == NULL) {
		SysBeep(20);
		return;
	}
	
	SetPort((WindowPtr)theDialog);
	TextFont(0);
	TextSize(12);
	
	BackColor(whiteColor);
	ShortenDITL(theDialog,CountDITL(theDialog)-iPrefIcons);
	AppendDITL(theDialog,itemHdl,overlayDITL);
	ReleaseResource(itemHdl);
	
	gSelectedPrefs = newPanel;
	GetDialogItem(theDialog,iDefault,&itemType,&itemHdl,&itemRect);
	switch (newPanel) {
		case kGeneralPrefs : // general preferences
			HiliteControl((ControlHandle)itemHdl,0);

			GetDialogItem(theDialog,1+iPrefIcons,&itemType,&itemHdl,&itemRect);
			SetControlValue((ControlHandle)itemHdl,Prefs->gnlNegRed);

			GetDialogItem(theDialog,2+iPrefIcons,&itemType,&itemHdl,&itemRect);
			SetControlValue((ControlHandle)itemHdl,Prefs->gnlUpdate);

			GetDialogItem(theDialog,3+iPrefIcons,&itemType,&itemHdl,&itemRect);
			SetControlValue((ControlHandle)itemHdl,Prefs->gnlRmveClan);
			
			GetDialogItem(theDialog,4+iPrefIcons,&itemType,&itemHdl,&itemRect);
			SetControlValue((ControlHandle)itemHdl,Prefs->gnlSwitch);

		  	GetDialogItem(theDialog,5+iPrefIcons,&itemType,&itemHdl,&itemRect);
			SetControlValue((ControlHandle)itemHdl,Prefs->gnlPanel);
			if (Prefs->gnlSwitch) {
				HiliteControl((ControlHandle)itemHdl,0);
			} else {
				HiliteControl((ControlHandle)itemHdl,255);
			} // else

			break;
		case kColorPrefs : // color preferences
			HiliteControl((ControlHandle)itemHdl,0);

			GetDialogItem(theDialog,1+iPrefIcons,&itemType,&itemHdl,&itemRect);
			SetControlValue((ControlHandle)itemHdl,Prefs->clrEnabled);

			for (i=0; i<6; i++) {
				adjustTeamColor(theDialog,i+iPrefIcons+2,i,Prefs);
				GetDialogItem(theDialog,i+iPrefIcons+2,&itemType,&itemHdl,&itemRect);
				if (Prefs->clrEnabled) {
					HiliteControl((ControlHandle)itemHdl,0);
				} else {
					HiliteControl((ControlHandle)itemHdl,255);
				} // else
			} // for
			break;
		case kSpeechPrefs : // speech preferences
			if (gHasSpeechmanager) {
				HiliteControl((ControlHandle)itemHdl,0);
			} else {
				HiliteControl((ControlHandle)itemHdl,255);
			}
			
			GetDialogItem(theDialog,2+iPrefIcons,&itemType,&itemHdl,&itemRect);
			SetControlValue((ControlHandle)itemHdl,Prefs->spkVoice);

			GetDialogItem(theDialog,3+iPrefIcons,&itemType,&itemHdl,&itemRect);
			SetControlValue((ControlHandle)itemHdl,Prefs->spkRate);
			
			GetDialogItem(theDialog,4+iPrefIcons,&itemType,&itemHdl,&itemRect);
			SetControlValue((ControlHandle)itemHdl,Prefs->spkSkipLT);
			
			GetDialogItem(theDialog,5+iPrefIcons,&itemType,&itemHdl,&itemRect);
			SetControlValue((ControlHandle)itemHdl,Prefs->spkEnabled);

			if (!gHasSpeechmanager) {
				GetDialogItem(theDialog,5+iPrefIcons,&itemType,&itemHdl,&itemRect);
				HiliteControl((ControlHandle)itemHdl,255);
			}
			if ((!gHasSpeechmanager) || (!Prefs->spkEnabled)) {
				GetDialogItem(theDialog,2+iPrefIcons,&itemType,&itemHdl,&itemRect);
				HiliteControl((ControlHandle)itemHdl,255);
				GetDialogItem(theDialog,4+iPrefIcons,&itemType,&itemHdl,&itemRect);
				HiliteControl((ControlHandle)itemHdl,255);
				/*GetDialogItem(theDialog,4+iPrefIcons,&itemType,&itemHdl,&itemRect);
				HiliteControl((ControlHandle)itemHdl,255);*/
			}
			if ((!gHasSpeechmanager) || (!Prefs->spkEnabled) || (Prefs->spkVoice < 3)) {
				GetDialogItem(theDialog,1+iPrefIcons,&itemType,&itemHdl,&itemRect);
				HiliteControl((ControlHandle)itemHdl,255);
				GetDialogItem(theDialog,3+iPrefIcons,&itemType,&itemHdl,&itemRect);
				HiliteControl((ControlHandle)itemHdl,255);
			}
			break;
		case kLogPrefs : // logging preferences
			HiliteControl((ControlHandle)itemHdl,0);

			GetDialogItem(theDialog,1+iPrefIcons,&itemType,&itemHdl,&itemRect);
			SetControlValue((ControlHandle)itemHdl,Prefs->logEnabled);
			for (i=2; i<=6; i++) {
				GetDialogItem(theDialog,i+iPrefIcons,&itemType,&itemHdl,&itemRect);
				if (Prefs->logEnabled) {
					HiliteControl((ControlHandle)itemHdl,0);
				} else {
					HiliteControl((ControlHandle)itemHdl,255);
				}
			}
			// GetDialogItem(theDialog,kLogOneFile,&itemType,&itemHdl,&itemRect);
			// HiliteControl((ControlHandle)itemHdl,255);

			GetDialogItem(theDialog,2+iPrefIcons,&itemType,&itemHdl,&itemRect);
			SetControlValue((ControlHandle)itemHdl,Prefs->logFormat);
			
			GetDialogItem(theDialog,Prefs->logTime+iPrefIcons,&itemType,&itemHdl,&itemRect);
			SetControlValue((ControlHandle)itemHdl,true);
			if (Prefs->logTime == kLogEachGame) {
				GetDialogItem(theDialog,6+iPrefIcons,&itemType,&itemHdl,&itemRect);
				HiliteControl((ControlHandle)itemHdl,255);
			}
			break;
		case kMoviePrefs : // QuickTime preferences
			if (gHasQuickTime) {
				HiliteControl((ControlHandle)itemHdl,0);
			} else {
				HiliteControl((ControlHandle)itemHdl,255);
			}

			GetDialogItem(theDialog,1+iPrefIcons,&itemType,&itemHdl,&itemRect);	// Save movie
			SetControlValue((ControlHandle)itemHdl,Prefs->movEnabled);
			if (gHasQuickTime) {
				HiliteControl((ControlHandle)itemHdl,0);
			} else {
				HiliteControl((ControlHandle)itemHdl,255);
			}
			
			GetDialogItem(theDialog,2+iPrefIcons,&itemType,&itemHdl,&itemRect);	// Indicator
			SetControlValue((ControlHandle)itemHdl,Prefs->movIcon);
			
			if (gHasQuickTime && Prefs->movEnabled) {
				HiliteControl((ControlHandle)itemHdl,0);
			} else {
				HiliteControl((ControlHandle)itemHdl,255);
			}
			
			GetDialogItem(theDialog,3+iPrefIcons,&itemType,&itemHdl,&itemRect);	// Set� button
			if (gHasQuickTime && Prefs->movEnabled) {
				HiliteControl((ControlHandle)itemHdl,0);
			} else {
				HiliteControl((ControlHandle)itemHdl,255);
			}
			
			GetDialogItem(theDialog,4+iPrefIcons,&itemType,&itemHdl,&itemRect);	// Rate slider
			SetControlValue((ControlHandle)itemHdl,Prefs->movRate);
			
			if (gHasQuickTime && Prefs->movEnabled) {
				HiliteControl((ControlHandle)itemHdl,0);
			} else {
				HiliteControl((ControlHandle)itemHdl,255);
			}
			break;
	} // switch
}

void defaultPreferences(struct PreferencesType *Prefs,short thePanel)
{
	switch (thePanel) {
		case kGeneralPrefs:
			Prefs->gnlRmveClan = Prefs->gnlNegRed = (gPlugLevel == kPayed);
			Prefs->gnlUpdate = true;
			Prefs->gnlSwitch = false;
			Prefs->gnlPanel = kAboutPanel;
			break;
		case kColorPrefs:
			Prefs->clrEnabled = true;
			setColor(&Prefs->teamColor[0],00000,40092,12593);
			setColor(&Prefs->teamColor[1],65535,25443,00000);
			setColor(&Prefs->teamColor[2],49125,17030,20305);
			setColor(&Prefs->teamColor[3],40092,00000,52942);
			setColor(&Prefs->teamColor[4],12593,00000,25443);
			setColor(&Prefs->teamColor[5],00000,00000,61423);
			break;
		/*case kSoundPrefs:
			break;*/
		case kSpeechPrefs:
			Prefs->spkEnabled = false;
			Prefs->spkVoice = 1;
			Prefs->spkRate = 0;
			Prefs->spkSkipLT = false;
			break;
		case kLogPrefs:
			Prefs->logEnabled = false;
			Prefs->logFormat = 1;
			Prefs->logTime = kLogEachGame;
			break;
		case kMoviePrefs:
			Prefs->movEnabled = false;
			Prefs->movIcon = true;
			Prefs->movFlag = kFileNotYetSelected;
			Prefs->movRate = 0;
			break;
		/*case kOnlinePrefs:
			break;*/
	}
}

// ��������������������������������������������������������������������������������������


Boolean askTeamColor(DialogPtr theDialog,short itemHit,short team,struct PreferencesType *TempPrefs)
{
	Point					thePoint;
	RGBColor				theColor;
	Str255					thePrompt;
	SInt16					itemType;
	Handle					itemHdl;
	Rect					itemRect;
	Boolean					colorOK;
	
	GetIndString(thePrompt,rColorStrings,kNewTeamColor);
	SetPt(&thePoint,0,0);		// center on main screen
	theColor = TempPrefs->teamColor[team];
	if (GetColor(thePoint,thePrompt,&TempPrefs->teamColor[team],&theColor)) {
		TempPrefs->teamColor[team] = theColor;
		adjustTeamColor(theDialog,itemHit,team,TempPrefs);
		return true;
	}
	return false;
}

void adjustTeamColor(DialogPtr theDialog,short itemHit,short team,struct PreferencesType *TempPrefs)
{
	SInt16					itemType;
	Handle					itemHdl;
	Rect					itemRect;
	
	GetDialogItem(theDialog,itemHit,&itemType,&itemHdl,&itemRect);
	SetCtlMin((ControlHandle)itemHdl,TempPrefs->teamColor[team].red - 32766);
	SetCtlMax((ControlHandle)itemHdl,TempPrefs->teamColor[team].green - 32766);
	SetCRefCon((ControlHandle)itemHdl,TempPrefs->teamColor[team].blue - 32766);
}

void updateColorPalette(struct PreferencesType *Prefs)
{
	PaletteHandle			thePalette;
	short					i,j;
	WindowPtr				theWindow;
	RGBColor				theColor;
	Rect					theRect;
	GrafPtr					oldPort;
	
	if (UserPrefs.clrEnabled) {
		GetPort(&oldPort);
		theWindow = FrontWindow();
		SetRect(&theRect,0,0,32000,32000);
		while (theWindow != NULL) {
			thePalette = GetPalette(theWindow);
			for (i=0; i<6; i++) {
				theColor = Prefs->teamColor[i];			// don't calcLightColor over original color
				SetEntryColor(thePalette,i+6,&theColor);
				calcLightColor(&theColor,0.7);
				SetEntryColor(thePalette,i+12,&theColor);
			}
			ActivatePalette(theWindow);
			SetPort(theWindow);
			InvalRect(&theRect);
			theWindow = (WindowPtr)(((WindowPeek)theWindow)->nextWindow);
		}
		SetPort(oldPort);
	} else {
		resetColorPalette();
	}
}

void findGameWindow()
{
	WindowPtr				theWindow;
	Str255					theTitle, game = "\pAvara";
	long					len;
	StringPtr				thePtr;
	
	theWindow = FrontWindow();
	while (theWindow != NULL) {
		thePtr = *(((WindowPeek)theWindow)->titleHandle);
		len = (long)(*thePtr);
		len++;
		BlockMoveData(thePtr,theTitle,len);
		if (cmpStrs(theTitle,game,kExactCmp)) {
			gameWindow = theWindow;
		}
		theWindow = (WindowPtr)(((WindowPeek)theWindow)->nextWindow);
	}
}

void resetColorPalette(void)
{
	PaletteHandle			oldPalette;
	short					i;
	WindowPtr				theWindow;
	Rect					theRect;
	GrafPtr					oldPort;
	
	GetPort(&oldPort);
	theWindow = FrontWindow();
	SetRect(&theRect,0,0,32000,32000);
	oldPalette = GetNewPalette(128);
	while (theWindow != NULL) {
		SetPalette(theWindow,oldPalette,true);
		ActivatePalette(theWindow);
		SetPort(theWindow);
		InvalRect(&theRect);
		theWindow = (WindowPtr)(((WindowPeek)theWindow)->nextWindow);
	}
	SetPort(oldPort);
}

void calcLightColor(RGBColor *theColor,float percent)
{
	HSLColor			theHLS;

	RGB2HSL(theColor, &theHLS);
	theHLS.lightness = (short)(percent * 65535); // � 90% of original
	HSL2RGB(&theHLS, theColor);
}

void setColor(RGBColor *C, short R, short G, short B) {
	C->red = R; C->green = G; C->blue = B;
}

// ��������������������������������������������������������������������������������������

void  checkSoundEnv(void)
{
	OSErr		osErr;
	SInt32		response;

	osErr = Gestalt(gestaltSpeechAttr,&response);			// OK for speech?
	gHasSpeechmanager = ((osErr == noErr) && BitTst(&response,31 - gestaltSpeechMgrPresent));

	osErr = Gestalt(gestaltSoundAttr,&response);
	if(osErr == noErr)
		gHasMultiChannel = BitTst(&response,31 - gestaltMultiChannels);		
		//gHasMultiChannel = BitTst(&response,gestaltMultiChannels);		
	else
		gHasMultiChannel = false;
}

void removeClan(Str255 theString)
{
	short			i = 0, j = 0, /*k = 0,*/ loc = 0;
	unsigned char	len = 0;
	long			textSize = 0;
	Str255			skipOpen, skipClose, endStr;
	
	if (!UserPrefs.gnlRmveClan) return;
	
	len = 0;
	textSize = *theString;
	GetIndString(skipOpen,rSpeechStrings,7);
	GetIndString(skipClose,rSpeechStrings,8);
	for (i = 1; i<=textSize; i++) {
		if (loc = chrInStr(theString[i],skipOpen)) {
			for (j=i+1; j<=textSize; j++) {
				if (chrInStr(theString[j],skipClose) == loc) {
					/*for (k=i; k<=j; k++) {
						theString[k] = (char)' ';
					} // for k*/
					i = j+1;
					break;
				} // if close
			} // for j
		} // if open
		endStr[++len] = theString[i];
	} // for i
	endStr[0] = (unsigned char)len;
	BlockMoveData(endStr,theString,len+1);
}

void speakStringAsync(Str255 theStr)
{
	long		textSize = 0;
	short		i, pos;
	Str255		theString, weirdChars, normalChars;
	OSErr		osErr;
	
	removeClan(theStr);
	
	if ((gHasSpeechmanager) && (gSpeechChan != NULL) && (SpeechBusy() == 0) && 
	 		(UserPrefs.spkEnabled)) {
		textSize = *theStr;
		BlockMoveData(theStr,theString,textSize+1);
		
		GetIndString(weirdChars,rSpeechStrings,9);
		GetIndString(normalChars,rSpeechStrings,10);
		
		for (i=1; i<=textSize; i++) {
			if (pos = chrInStr(theString[i],weirdChars)) theString[i] = normalChars[pos];
		} // weirdChars
		osErr = SpeakText(gSpeechChan,(Ptr)theString+1,textSize);
	}
}

void speakTestString(struct PreferencesType *Prefs)
{
	OSErr						myErr;
	VoiceSpec					myVoice;
	VoiceDescription			myInfo;
	short						voiceCount;
	Fixed						myRate;

	changeVoice(Prefs);
	if ((gHasSpeechmanager) && (Prefs->spkEnabled)) {
		myErr = CountVoices(&voiceCount);
		if ((myErr == noErr) && 
				(Prefs->spkVoice-3 >= 0) && (Prefs->spkVoice-3 <= voiceCount)) {
			myErr = GetIndVoice(voiceCount-Prefs->spkVoice+3,&myVoice);
			if (myErr == noErr) {
			  	myErr = GetVoiceDescription(&myVoice,&myInfo,sizeof(myInfo)) ;
				if (myErr == noErr) {
			  		speakStringAsync(myInfo.comment);
			  	} // if GetVoiceDescription
			} // if GetIndVoice
		} // if CountVoices
		if (myErr != noErr) {									// disable on error
			//closeSpeechChannel();
			SysBeep(20);
			//gHasSpeechmanager = false;
		}
	}
}

void speakKill(ScoreInterfaceRecord *rec)
{
	short					textSize, len;
	Str255					theText, theStr;
	
	if (rec->scorePoints != 0) {
		if (rec->playerID == kCompPlayer) {
			if (noPlayers == 1) {
				GetIndString(theText,rSpeechStrings,2);
				textSize = *theText;					// you
				textSize += 1;
			} else {
				textSize = *longNames[rec->scoreID];	// player name
				textSize += 1;
				BlockMoveData(longNames[rec->scoreID],theText,textSize);
			}
			GetIndString(theStr,rSpeechStrings,5);
			len = *theStr;
			BlockMoveData(theStr+1,theText+textSize,len);	// got killed
		} else {
			if (noPlayers == 1) {
				GetIndString(theText,rSpeechStrings,2);
				textSize = *theText;					// you
				textSize += 1;
			} else {
				textSize = *longNames[rec->playerID];	// player name
				textSize += 1;
				BlockMoveData(longNames[rec->playerID],theText,textSize);
			}
			
			if (rec->scoreID == rec->playerID) {		// suicide
				GetIndString(theStr,rSpeechStrings,6);
				len = *theStr;
				BlockMoveData(theStr+1,theText+textSize,len);
			} else {
				if (rec->scoreID == kCompPlayer) {
					if (noPlayers != 1) {
						GetIndString(theStr,rSpeechStrings,5);
						len = *theStr;							// got killed
						BlockMoveData(theStr+1,theText+textSize,len);
					} else {
						GetIndString(theStr,rSpeechStrings,3);	// killed
						len = *theStr;
						BlockMoveData(theStr+1,theText+textSize,len);
						textSize += len;
						GetIndString(theStr,rSpeechStrings,4);	// a computer
						len = *theStr;
						BlockMoveData(theStr+1,theText+textSize,len);
					}
				} else {
					GetIndString(theStr,rSpeechStrings,11+rec->frameNumber%6);		// killed
					len = *theStr;
					BlockMoveData(theStr+1,theText+textSize,len);
					textSize += len;
					len = *longNames[rec->scoreID];				// an opponent
					BlockMoveData(longNames[rec->scoreID]+1,theText+textSize,len);
				}
			}
		} // playerID == kCompPlayer
		textSize += len-1;
		theText[0] = (char)textSize;
		BlockMoveData(theText,rec->consoleLine,textSize+1);
		speakStringAsync(theText);
	}
}

void insertVoicesMenu(struct PreferencesType *Prefs)
{
	OSErr						myErr;
	VoiceSpec					myVoice;
	VoiceDescription			myInfo;
	MenuHandle					myMenu;
	short						i, voiceCount;
	
	myMenu = NewMenu(2022,"\pVoices");
	if (myMenu != NULL) {
		AppendMenu(myMenu,"\pDefault");
		if (gHasSpeechmanager) {
			if (CountVoices(&voiceCount) == noErr) {
			  AppendMenu(myMenu,"\p-");
			  for (i = voiceCount; i >= 0; i--) {
			  	if (GetIndVoice(i,&myVoice) == noErr) {
				  	if (GetVoiceDescription(&myVoice,&myInfo,sizeof(myInfo)) == noErr) {
				  		AppendMenu(myMenu,myInfo.name);
				  	} // if GetVoiceDescription
			  	} // if GetIndVoice
			  } // for i
			} // if CountVoices
		} // if gHasSpeechmanager
		InsertMenu(myMenu,-1);
		DrawMenuBar();
	} else {
		closeSpeechChannel();
		Prefs->spkEnabled = false;
		doErrorAlert(rSpeechStrings,kSpeechError);
	}
}


void changeVoice(struct PreferencesType *Prefs)
{
	closeSpeechChannel();
	openSpeechChannel(Prefs);
}

void openSpeechChannel(struct PreferencesType *Prefs)
{
	OSErr						myErr;
	VoiceSpec					myVoice;
	short						voiceCount;
	Fixed						myRate;
	Handle						myDict;

	if ((gHasSpeechmanager) && (Prefs->spkEnabled)) {
		myErr = CountVoices(&voiceCount);
		if ((myErr == noErr) && 
				(Prefs->spkVoice-3 >= 0) && (Prefs->spkVoice-3 <= voiceCount)) {
			myErr = GetIndVoice(voiceCount-Prefs->spkVoice+3,&myVoice);
			if (myErr == noErr) {
				myErr = NewSpeechChannel(&myVoice,&gSpeechChan);
				if ((myErr == noErr) && (Prefs->spkRate != 0)) {
					GetSpeechRate(gSpeechChan,&myRate);
					if (Prefs->spkRate < 0) {
						myRate = (long)(myRate/(1-Prefs->spkRate/2));
					} else {
						myRate = (long)(myRate*(1+Prefs->spkRate/2));
					}
					SetSpeechRate(gSpeechChan,myRate);
				}
			} // if GetIndVoice
		} // if CountVoices
		if ((myErr != noErr) || 
				(Prefs->spkVoice-3 < 0) || (Prefs->spkVoice-3 > voiceCount)) {
			myErr = NewSpeechChannel(NULL,&gSpeechChan);		// default voice
		}
		if (myErr != noErr) {									// disable on error
			closeSpeechChannel();
			Prefs->spkEnabled = false;
			UserPrefs.spkEnabled = false;
			doErrorAlert(rSpeechStrings,kSpeechError);
		} else {
			myDict = GetResource('dict',1200);
			if ((myDict) && (ResError() == noErr)) {
				myErr = UseDictionary(gSpeechChan,myDict);
				if (myErr == badDictFormat) { SysBeep(20);SysBeep(20);SysBeep(20); }
				if (myErr != noErr) { SysBeep(20); }
				ReleaseResource(myDict);
			} else {
				SysBeep(20);
			}
		}
	}
}

void closeSpeechChannel(void)
{
	if ((gHasSpeechmanager) && (gSpeechChan != NULL)) {
		StopSpeech(gSpeechChan);
		DisposeSpeechChannel(gSpeechChan);
		gSpeechChan = NULL;
	}
}

void selectMovieFile(struct PreferencesType *Prefs)
{
	SFReply					theReply;
	OSErr					theErr;
	Point					loc = {-1,-1};

	Prefs->movFlag = kFileNotYetSelected;
	if (!Prefs->movEnabled) { return; }
	if (!(initMovie())) { return; }
	SFPutFile(loc, "\pEnter movie filename:", "\pMovie File", nil, &theReply);
	if (!theReply.good) {
		Prefs->movFlag = kFileCancelled;	//UserCanceled
		return;
	}
	/*if (Prefs->movFlag == kFileSelected) {	//File exists
		closeMyMovieFile(Prefs);
	}*/
	Prefs->movFlag = kFileSelected;		//File is saved
	FSMakeFSSpec(theReply.vRefNum, 0, theReply.fName, &Prefs->movFile);
}
	
void openMovieFile(struct PreferencesType *Prefs)
{
	OSErr					theErr;
	
	theErr = CreateMovieFile(&Prefs->movFile, kMoovType, smCurrentScript,
							 createMovieFileDeleteCurFile, &gMovieRefNum, &gMovie);
	if (theErr != noErr) {
		doErrorAlert(rMovieErrors,kMovCreateError); Prefs->movFlag = kFileCancelled;
		return;
	}
	gMovieTrack = NewMovieTrack(gMovie, FixRatio(gMoviePortPtr->portRect.right, 1),
			FixRatio(gMoviePortPtr->portRect.bottom, 1), kNoVolume);
	if (GetMoviesError() != noErr) { 
		doErrorAlert(rMovieErrors,kMovTrackError); Prefs->movFlag = kFileCancelled;
		return;
	}
	gMovieMedia = NewTrackMedia (gMovieTrack, VideoMediaType, 7545 /* time*75,45/delay fps*/, nil, 0);
	if (GetMoviesError() != noErr) {
		doErrorAlert(rMovieErrors,kMovTrackError); Prefs->movFlag = kFileCancelled;
		return;
	}
	theErr = BeginMediaEdits(gMovieMedia);
	if (theErr != noErr) {
		doErrorAlert(rMovieErrors,kMovEditError); Prefs->movFlag = kFileCancelled;
		return;
	}
}

void closeMyMovieFile(struct PreferencesType *Prefs)
{
	short					theResID = 0;
	OSErr					theErr;

	if (!UserPrefs.movEnabled) {
		return;
	}
	if (!gHasQuickTime) {
		return;
	}
	if (Prefs->movFlag != kFileSelected) {
		return;
	}
	Prefs->movFlag = kFileNotYetSelected;
	theErr = EndMediaEdits(gMovieMedia);
	if (theErr != noErr) {
		doErrorAlert(rMovieErrors,kMovEditError); Prefs->movEnabled = 0;
		return;
	}
	theErr = InsertMediaIntoTrack(gMovieTrack, 0 , 0, GetMediaDuration(gMovieMedia), 0x00010000);
	if (theErr != noErr) {
		doErrorAlert(rMovieErrors,kMovTrackError); Prefs->movEnabled = 0;
		return;
	}
	theErr = AddMovieResource(gMovie,gMovieRefNum,&theResID,"\pScoreKeeper 1.2.1 �1997-1999 SkareWare");
	if (theErr != noErr) {
		doErrorAlert(rMovieErrors,kMovCreateError); Prefs->movEnabled = 0;
		return;
	}
	if (gMovieRefNum) CloseMovieFile(gMovieRefNum);
	DisposeMovie(gMovie);
	//theErr = FlushPrefVol();
}

Boolean ScoresNotZero(void)
{
	short			i, j;
	Boolean			allZero;
	
	allZero = true;
	for (i=0; i<6; i++) {
		for (j=0; j<8; j++) {
			allZero = allZero && (playerPoints[i][j] == 0);
		}
	}
	return(!allZero);
}

void SelectLogFile(struct PreferencesType *Prefs)
{
	short					fileRefNum;
	Str255					thePrompt, theName;
	OSErr					myErr;
	unsigned long			currTime;
	LongDateTime			longDate;
	StandardFileReply		theReply;
						
	GetDateTime (&currTime);

	GetIndString(thePrompt,rLogFileStrings,1);
	GetIndString(theName,rLogFileStrings,2);
		
	StandardPutFile(thePrompt,theName, &theReply);
	if (!theReply.sfGood) {
		Prefs->logFlag = kFileCancelled;
		return;
	}
	if (theReply.sfReplacing) {
		myErr = FSpOpenDF (&theReply.sfFile, fsWrPerm, &fileRefNum);
		if (myErr != noErr) {
			Prefs->logFlag = kFileCancelled;
			Prefs->logEnabled = false;
			Prefs->logTime = kLogEachGame;
			gSelectedPrefs = kLogPrefs;
			doPrefErrorAlert(rLogFileStrings,5);
			return;
		}
		SetEOF(fileRefNum,0);							// delete all previous contents
		myErr = FSClose (fileRefNum);
	} else {
		myErr = FSpCreate(&theReply.sfFile, 'R*ch', 'TEXT', theReply.sfScript);
		if (myErr != noErr) {
			Prefs->logFlag = kFileCancelled;			// Creation Error
			Prefs->logEnabled = false;
			Prefs->logTime = kLogEachGame;
			gSelectedPrefs = kLogPrefs;
			doPrefErrorAlert(rLogFileStrings,4);
			return;
		}
	}
	if (gLogAlias) DisposeHandle((Handle)(gLogAlias));
	myErr = NewAlias(&gPlugSpec,&theReply.sfFile,&gLogAlias);
	if (myErr != noErr) {
			Prefs->logFlag = kFileCancelled;			// Referencing Error
			Prefs->logEnabled = false;
			Prefs->logTime = kLogEachGame;
			gSelectedPrefs = kLogPrefs;
			doPrefErrorAlert(rLogFileStrings,10);
			return;
	}
	Prefs->logFlag = kFileSelected;
	Prefs->logFile = theReply.sfFile;
	WritePref();
}

void SaveFixedWidthLog(ScoreInterfaceRecord *rec)
{
	char					textBuf[(kLineLength + 1) * 6], title[81]="      Player        C  Shot   Grnd.  Misl. Kill  Bonus Damage  Ball  Goal  Total\r";
	char					cr[1]="\r", at[6]="  at  ", date[9]="   Date: ", mission[9]="Mission: ",
							time[9]="   Time: ", cl[1]=":",nl[1]="0",color[7]=" GYRMPC",
							line[81]="-------------------+-+------+------+------+----+------+------+------+----+------\r";
	short					playerNo, linePos, column, lineCount, lineStart, strSize, fileRefNum;
	Ptr						sourcePtr, destPtr;
	Str255					tempNumStr;
	OSErr					myErr;
	long					textSize, secs;
	unsigned long			currTime;
	unsigned char			wasChanged;
		
	lineCount = -1;
	for (playerNo = 0; playerNo < 6; playerNo++) {
		if (playerJoined[playerNo]) {
			lineCount++;
			lineStart = (kLineLength + 1) * lineCount;
			for (column = 0;column < kLineLength;column++) {
				*(textBuf + lineStart + column) = (char)' ';
			}
			*(textBuf + lineStart + kLineLength) = (char)'\r';
			
			BlockMoveData (longNames[playerNo], tempNumStr, *longNames[playerNo] + 1);
			if (*tempNumStr > 19) {
				strSize = 19;
				*(tempNumStr + 19) = (char)'�';
			} else {
				strSize = *tempNumStr;
			}

			sourcePtr = (char*) tempNumStr + 1;
			destPtr = textBuf + lineStart;
			BlockMoveData (sourcePtr, destPtr, strSize);

			sourcePtr = color + playerTeam[playerNo];
			destPtr = textBuf + lineStart + 20;
			BlockMoveData (sourcePtr, destPtr, 1);

			linePos = 21;					// width of the name and team
			for (column=0; column<9; column++) {
				switch (column) {				// add column width
					case kKillColumn:
					case kGoalColumn:
						linePos += 5;
						break;
					default:
						linePos += 7;
						break;
				}
				NumToString ((long)playerPoints[playerNo][column], tempNumStr);
				sourcePtr = (char*) tempNumStr + 1;
				destPtr = textBuf + lineStart + linePos - *(tempNumStr);
				BlockMoveData (sourcePtr, destPtr, *(tempNumStr));
			}
		}			
	}
	*(textBuf + lineStart + (kLineLength + 1)) = (char)0;
	
	myErr = resolveLogAlias();
	if (myErr == noErr) myErr = FSpOpenDF(&UserPrefs.logFile, fsWrPerm, &fileRefNum);
	
	if (myErr != noErr) {
		UserPrefs.logFlag = kFileCancelled;
		UserPrefs.logEnabled = false;
		UserPrefs.logTime = kLogEachGame;
		gSelectedPrefs = kLogPrefs;
		doPrefErrorAlert(rLogFileStrings,6);		// Open error
		return;
	}

	myErr = SetFPos(fileRefNum, fsFromLEOF, 0);
	if (myErr != noErr) {
		UserPrefs.logFlag = kFileCancelled;
		UserPrefs.logEnabled = false;
		UserPrefs.logTime = kLogEachGame;
		gSelectedPrefs = kLogPrefs;
		doPrefErrorAlert(rLogFileStrings,7);		// append error
		return;
	}

	// ---------------------- Mission Name --------------------------
	textSize = 9;
	/* myErr = */ FSWrite (fileRefNum, &textSize, mission);			// Mission:

	textSize = *(rec->levelName) + 1;
	BlockMoveData (rec->levelName,&tempNumStr, textSize);
	textSize--;
	/* myErr = */ FSWrite (fileRefNum, &textSize, tempNumStr + 1);	// levelName

	textSize = 1;
	/* myErr = */ FSWrite (fileRefNum, &textSize, cr);				// cariage return

	// -------------------- Duration & fps --------------------------
	textSize = 9;
	/* myErr = */ FSWrite (fileRefNum, &textSize, time);			// Time :

	secs = (long)(rec->frameTime * rec->frameNumber / 1000);		// hours
	NumToString ((long)(secs/3600), tempNumStr);
	textSize = *tempNumStr;
	/* myErr = */ FSWrite (fileRefNum, &textSize, tempNumStr + 1);
	
	textSize = 1;
	/* myErr = */ FSWrite (fileRefNum, &textSize, cl);				// collon

	secs = (long)(secs % 3600);										// minutes
	if (((long)(secs/60)) < 10) {
		textSize = 1;
		/* myErr = */ FSWrite (fileRefNum, &textSize, nl);			// extra zero
	}
	NumToString ((long)(secs/60), tempNumStr);
	textSize = *tempNumStr;
	/* myErr = */ FSWrite (fileRefNum, &textSize, tempNumStr + 1);

	textSize = 1;
	/* myErr = */ FSWrite (fileRefNum, &textSize, cl);				// collon

	if (((long)(secs%60)) < 10) {
		textSize = 1;
		/* myErr = */ FSWrite (fileRefNum, &textSize, nl);			// extra zero
	}

	NumToString ((long)(secs % 60), tempNumStr);					// seconds
	textSize = *tempNumStr;
	/* myErr = */ FSWrite (fileRefNum, &textSize, tempNumStr + 1);

	textSize = 1;
	/* myErr = */ FSWrite (fileRefNum, &textSize, cr);				// cariadge return

	// ---------------------- Date & Time ---------------------------
	textSize = 9;
	/* myErr = */ FSWrite (fileRefNum, &textSize, date);			// Date:

	GetDateTime (&currTime);
	DateString (currTime, longDate, tempNumStr, nil);
	textSize = *tempNumStr;
	/* myErr = */ FSWrite (fileRefNum, &textSize, tempNumStr + 1);	// date

	textSize = 6;
	/* myErr = */ FSWrite (fileRefNum, &textSize, at);				//   at

	TimeString (currTime, true, tempNumStr, nil);
	textSize = *tempNumStr;
	/* myErr = */ FSWrite (fileRefNum, &textSize, tempNumStr + 1);	// time

	textSize = 1;
	/* myErr = */ FSWrite (fileRefNum, &textSize, cr);				// cariadge return
	textSize = 1;
	/* myErr = */ FSWrite (fileRefNum, &textSize, cr);				// cariadge return

	textSize = 81;
	/* myErr = */ FSWrite (fileRefNum, &textSize, title);			// score titles

	textSize = 81;
	/* myErr = */ FSWrite (fileRefNum, &textSize, line);			// Seperator line

	textSize = ((noPlayers) * (kLineLength + 1));
	/* myErr = */ FSWrite (fileRefNum, &textSize, textBuf);			// Score buffer

	textSize = 81;
	/* myErr = */ FSWrite (fileRefNum, &textSize, line);			// Seperator line

	textSize = 1;
	/* myErr = */ FSWrite (fileRefNum, &textSize, cr);				// cariadge return

	textSize = 1;
	/* myErr = */ FSWrite (fileRefNum, &textSize, cr);				// cariadge return
	
	myErr = FSClose (fileRefNum);
	rec->consoleLine = "\p\rLog entry complete�";
	rec->consoleJustify = 1;
} // SaveFixedWidthLog

void SaveTabDelimitedLog(ScoreInterfaceRecord *rec)
{
	char					cr[1]="\r", at[6]="  at  ", date[6]="Date:\t", mission[9]="Mission:\t",
							time[6]="Time:\t", cl[1]=":",nl[1]="0",tab[1]="\t",
							green[5]="Green",yellow[6]="Yellow",red[3]="Red",
							magenta[7]="Magenta",purple[6]="Purple",cyan[4]="Cyan",
							title[70]="Player\tColor\tShot\tGrenade\tMissile\tKills\tBonus\tDamage\tBall\tGoals\tTotal\r";
	short					i, j, k, lineStart, strSize, fileRefNum;
	Ptr						sourcePtr, destPtr;
	Str255					tempNumStr;
	OSErr					myErr;
	long					textSize, secs;
	unsigned long			currTime;
	unsigned char			wasChanged;

	myErr = resolveLogAlias();
	if (myErr == noErr) myErr = FSpOpenDF(&UserPrefs.logFile, fsWrPerm, &fileRefNum);
	
	if (myErr != noErr) {
		UserPrefs.logFlag = kFileCancelled;
		UserPrefs.logEnabled = false;
		UserPrefs.logTime = kLogEachGame;
		gSelectedPrefs = kLogPrefs;
		doPrefErrorAlert(rLogFileStrings,6);		// Open error
		return;
	}

	myErr = SetFPos(fileRefNum, fsFromLEOF, 0);
	if (myErr != noErr) {
		UserPrefs.logFlag = kFileCancelled;
		UserPrefs.logEnabled = false;
		UserPrefs.logTime = kLogEachGame;
		gSelectedPrefs = kLogPrefs;
		doPrefErrorAlert(rLogFileStrings,7);		// append error
		return;
	}

	// ---------------------- Mission Name --------------------------
	textSize = 9;
	/* myErr = */ FSWrite (fileRefNum, &textSize, mission);			// Mission:

	textSize = *(rec->levelName) + 1;
	BlockMoveData (rec->levelName,&tempNumStr, textSize);
	textSize--;
	/* myErr = */ FSWrite (fileRefNum, &textSize, tempNumStr + 1);	// levelName

	textSize = 1;
	/* myErr = */ FSWrite (fileRefNum, &textSize, cr);				// cariage return

	// -------------------- Duration & fps --------------------------
	textSize = 6;
	/* myErr = */ FSWrite (fileRefNum, &textSize, time);			// Time :

	secs = (long)(rec->frameTime * rec->frameNumber / 1000);		// hours
	NumToString ((long)(secs/3600), tempNumStr);
	textSize = *tempNumStr;
	/* myErr = */ FSWrite (fileRefNum, &textSize, tempNumStr + 1);
	
	textSize = 1;
	/* myErr = */ FSWrite (fileRefNum, &textSize, cl);				// collon

	secs = (long)(secs % 3600);										// minutes
	if (((long)(secs/60)) < 10) {
		textSize = 1;
		/* myErr = */ FSWrite (fileRefNum, &textSize, nl);			// extra zero
	}
	NumToString ((long)(secs/60), tempNumStr);
	textSize = *tempNumStr;
	/* myErr = */ FSWrite (fileRefNum, &textSize, tempNumStr + 1);

	textSize = 1;
	/* myErr = */ FSWrite (fileRefNum, &textSize, cl);				// collon

	if (((long)(secs%60)) < 10) {
		textSize = 1;
		/* myErr = */ FSWrite (fileRefNum, &textSize, nl);			// extra zero
	}

	NumToString ((long)(secs % 60), tempNumStr);					// seconds
	textSize = *tempNumStr;
	/* myErr = */ FSWrite (fileRefNum, &textSize, tempNumStr + 1);

	textSize = 1;
	/* myErr = */ FSWrite (fileRefNum, &textSize, cr);				// cariadge return

	// ---------------------- Date & Time ---------------------------
	textSize = 6;
	/* myErr = */ FSWrite (fileRefNum, &textSize, date);			// Date:

	GetDateTime (&currTime);
	DateString (currTime, longDate, tempNumStr, nil);
	textSize = *tempNumStr;
	/* myErr = */ FSWrite (fileRefNum, &textSize, tempNumStr + 1);	// date

	textSize = 6;
	/* myErr = */ FSWrite (fileRefNum, &textSize, at);				//   at

	TimeString (currTime, true, tempNumStr, nil);
	textSize = *tempNumStr;
	/* myErr = */ FSWrite (fileRefNum, &textSize, tempNumStr + 1);	// time

	textSize = 1;
	/* myErr = */ FSWrite (fileRefNum, &textSize, cr);				// cariadge return
	textSize = 1;
	/* myErr = */ FSWrite (fileRefNum, &textSize, cr);				// cariadge return

	textSize = 70;
	/* myErr = */ FSWrite (fileRefNum, &textSize, title);			// Scores title

	j = 0;
	for (i = 0; i < 6; i++) {
		if (playerJoined[i]) {
			j++;
			BlockMoveData(longNames[i], tempNumStr, *longNames[i] + 1);
			
			textSize = *tempNumStr;
			/* myErr = */ FSWrite (fileRefNum, &textSize, tempNumStr + 1);		// Player

			textSize = 1;
			/* myErr = */ FSWrite (fileRefNum, &textSize, tab);					// tab

			switch (playerTeam[i]) {
				case 1 : 
					textSize = 5;
					/* myErr = */ FSWrite (fileRefNum, &textSize, green);	// green
					break;
				case 2 : 
					textSize = 6;
					/* myErr = */ FSWrite (fileRefNum, &textSize, yellow);	// yellow
					break;
				case 3 : 
					textSize = 3;
					/* myErr = */ FSWrite (fileRefNum, &textSize, red);		// red
					break;
				case 4 : 
					textSize = 7;
					/* myErr = */ FSWrite (fileRefNum, &textSize, magenta);	// magenta
					break;
				case 5 : 
					textSize = 6;
					/* myErr = */ FSWrite (fileRefNum, &textSize, purple);	// purple
					break;
				case 6 : 
					textSize = 4;
					/* myErr = */ FSWrite (fileRefNum, &textSize, cyan);	// cyan
					break;
			}

			for (k=0; k<9; k++) {
				textSize = 1;
				/* myErr = */ FSWrite (fileRefNum, &textSize, tab);			// tab
				NumToString ((long)playerPoints[i][k], tempNumStr);
				textSize = *tempNumStr;
				/* myErr = */ FSWrite(fileRefNum, &textSize, tempNumStr + 1);
			}
						
			textSize = 1;
			/* myErr = */ FSWrite (fileRefNum, &textSize, cr);				// cariadge return
		}			
	}
						
	textSize = 1;
	/* myErr = */ FSWrite (fileRefNum, &textSize, cr);						// cariadge return
	textSize = 1;
	/* myErr = */ FSWrite (fileRefNum, &textSize, cr);						// cariadge return
	
	myErr = FSClose(fileRefNum);
	rec->consoleLine = "\p\rLog entry complete�";
	rec->consoleJustify = 1;
} // SaveTabDelimitedLog

void SaveDatabaseLog(ScoreInterfaceRecord *rec)
{
	char					cr[1]="\r", at[6]="  at  ", sep[1]="\t", tab[1]="\t",
							cl[1]=":", nl[1]="0"; 							// sep[1]="\035"
	short					i, j, k, lineStart, strSize, fileRefNum;
	Ptr						sourcePtr, destPtr;
	Str255					tempNumStr;
	OSErr					myErr;
	long					textSize, secs;
	unsigned long			currTime;
	unsigned char			wasChanged;

	myErr = resolveLogAlias();
	if (myErr == noErr) myErr = FSpOpenDF(&UserPrefs.logFile, fsWrPerm, &fileRefNum);
	
	if (myErr != noErr) {
		UserPrefs.logFlag = kFileCancelled;
		UserPrefs.logEnabled = false;
		UserPrefs.logTime = kLogEachGame;
		gSelectedPrefs = kLogPrefs;
		doPrefErrorAlert(rLogFileStrings,6);		// open error
		return;
	}

	myErr = SetFPos(fileRefNum, fsFromLEOF, 0);
	if (myErr != noErr) {
		UserPrefs.logFlag = kFileCancelled;
		UserPrefs.logEnabled = false;
		UserPrefs.logTime = kLogEachGame;
		gSelectedPrefs = kLogPrefs;
		doPrefErrorAlert(rLogFileStrings,7);		// append error
		return;
	}

	// ---------------------- Mission Name --------------------------
	textSize = *(rec->levelName) + 1;
	BlockMoveData (rec->levelName,&tempNumStr, textSize);
	textSize--;
	/* myErr = */ FSWrite (fileRefNum, &textSize, tempNumStr + 1);	// levelName

	textSize = 1;
	/* myErr = */ FSWrite (fileRefNum, &textSize, tab);				// tab

	// -------------------- Duration & fps --------------------------
	secs = (long)(rec->frameTime * rec->frameNumber / 1000);		// hours
	NumToString ((long)(secs/3600), tempNumStr);
	textSize = *tempNumStr;
	/* myErr = */ FSWrite (fileRefNum, &textSize, tempNumStr + 1);
	
	textSize = 1;
	/* myErr = */ FSWrite (fileRefNum, &textSize, cl);				// collon

	secs = (long)(secs % 3600);										// minutes
	if (((long)(secs/60)) < 10) {
		textSize = 1;
		/* myErr = */ FSWrite (fileRefNum, &textSize, nl);			// extra zero
	}
	NumToString ((long)(secs/60), tempNumStr);
	textSize = *tempNumStr;
	/* myErr = */ FSWrite (fileRefNum, &textSize, tempNumStr + 1);

	textSize = 1;
	/* myErr = */ FSWrite (fileRefNum, &textSize, cl);				// collon

	if (((long)(secs%60)) < 10) {
		textSize = 1;
		/* myErr = */ FSWrite (fileRefNum, &textSize, nl);			// extra zero
	}

	NumToString ((long)(secs % 60), tempNumStr);					// seconds
	textSize = *tempNumStr;
	/* myErr = */ FSWrite (fileRefNum, &textSize, tempNumStr + 1);

	textSize = 1;
	/* myErr = */ FSWrite (fileRefNum, &textSize, tab);				// tab

	// ---------------------- Date & Time ---------------------------
	GetDateTime (&currTime);
	DateString (currTime, longDate, tempNumStr, nil);
	textSize = *tempNumStr;
	/* myErr = */ FSWrite (fileRefNum, &textSize, tempNumStr + 1);	// date

	textSize = 6;
	/* myErr = */ FSWrite (fileRefNum, &textSize, at);				//   at

	TimeString (currTime, true, tempNumStr, nil);
	textSize = *tempNumStr;
	/* myErr = */ FSWrite (fileRefNum, &textSize, tempNumStr + 1);	// time

	textSize = 1;
	/* myErr = */ FSWrite (fileRefNum, &textSize, tab);				// tab
	// ---------------------- Player Handles ---------------------------
	j = 0;
	for (i = 0; i < 6; i++) {
		if (playerJoined[i]) {
			j++;
			BlockMoveData (longNames[i], tempNumStr, *longNames[i] + 1);
			
			textSize = *tempNumStr;
			/* myErr = */ FSWrite (fileRefNum, &textSize, tempNumStr + 1);		// Player
			
			if (j != 6) {
				textSize = 1;
				/* myErr = */ FSWrite (fileRefNum, &textSize, sep);				// seperator
			}

		}			
	}
	for (j++; j<6; j++) {
		textSize = 1;
		/* myErr = */ FSWrite (fileRefNum, &textSize, sep);						// seperator
	}
	textSize = 1;
	/* myErr = */ FSWrite (fileRefNum, &textSize, tab);							// tab


	// ---------------------- Player Teams ---------------------------
	j = 0;
	for (i = 0; i < 6; i++) {
		if (playerJoined[i]) {
			j++;

			NumToString ((long)playerTeam[i], tempNumStr);
			textSize = *tempNumStr;
			/* myErr = */ FSWrite (fileRefNum, &textSize, tempNumStr + 1);		// Team
			
			if (j != 6) {
				textSize = 1;
				/* myErr = */ FSWrite (fileRefNum, &textSize, sep);				// seperator
			}

		}			
	}
	for (j++; j<6; j++) {
		textSize = 1;
		/* myErr = */ FSWrite (fileRefNum, &textSize, sep);						// seperator
	}
	
	
	for (k=0; k<9; k++) {
		textSize = 1;
		/* myErr = */ FSWrite (fileRefNum, &textSize, tab);						// tab


		// ---------------------- Points ---------------------------
		j = 0;
		for (i = 0; i < 6; i++) {
			if (playerJoined[i]) {
				j++;

				NumToString ((long)playerPoints[i][k], tempNumStr);
				textSize = *tempNumStr;
				/* myErr = */ FSWrite (fileRefNum, &textSize, tempNumStr + 1);	// Points
				
				textSize = 1;
				if (j != 6) /* myErr = */ FSWrite (fileRefNum, &textSize, sep);	// seperator
			}			
		}
		for (j++; j<6; j++) {
			textSize = 1;
			/* myErr = */ FSWrite (fileRefNum, &textSize, sep);					// seperator
		}
	}
	
	textSize = 1;
	/* myErr = */ FSWrite (fileRefNum, &textSize, cr);					// cariadge return

	myErr = FSClose (fileRefNum);
	rec->consoleLine = "\p\rLog entry complete�";
	rec->consoleJustify = 1;
} // SaveDatabaseLog

void setPlugLevel(void)
{
	OSErr		theErr;
	short		refNum, i;
	long		count;
	char		len;
	Str255		license, demo;
	Str255		hack1 = "\pHackUser", hack2 = "\pRegistered",
				hack3 = "\p����", hack4 = "\pJohn Doe", hack5 = "\p=-BOOK-WORM->";

	GetIndString(demo,510,2);
    if (OpenPref(&refNum,"\pAvara License",fsRdPerm,kDataFork) != noErr) {
   		gPlugLevel = kLicenseError;
    	if (gVerboseErrors) doErrorAlert(rErrorStrings,14);
    	return;
    }
    
	count = 1;	//read licensee name length
    if (FSRead(refNum,&count,&len) != noErr) {
 		theErr = FSClose(refNum);
   		gPlugLevel = kLicenseError;
    	if (gVerboseErrors) doErrorAlert(rErrorStrings,13);
    	return;
    }
    
	count = (long)len;
    if (FSRead(refNum,&count,license + 1) != noErr) {
		theErr = FSClose(refNum);
    	gPlugLevel = kLicenseError;
     	if (gVerboseErrors) doErrorAlert(rErrorStrings,15);
 	  	return;
    }
 	license[0] = len;
 	
	if (cmpStrs(license,hack1,kStartCmp) || cmpStrs(license,hack2,kStartCmp) ||
		cmpStrs(license,hack3,kStartCmp) || cmpStrs(license,hack4,kStartCmp) ||
		cmpStrs(license,hack5,kStartCmp)) {
		theErr = FSClose(refNum);
		gPlugLevel = kLicenseError;
     	if (gVerboseErrors) doErrorAlert(rErrorStrings,16);
		RemoveLicense();
	   	return;
	}
 	
	if (!cmpStrs(license,demo,kStartCmp)) {
		theErr = FSClose(refNum);
		gPlugLevel = kPayed;
    	return;
	 }
	theErr = FSClose(refNum);
    gPlugLevel = kDemo;
    if (gVerboseErrors) doErrorAlert(rErrorStrings,12);
	return;
 }

void doPreferences(void)
{
	long			theTicks,theChoice;
	short			theItem;
	Rect			theRect;
	Point			thePoint;
	ControlPtr		thePtr;
	MenuHandle		theMenu;
	
	thePtr = *PrefBtn;
	SetRect(&theRect,thePtr->contrlRect.left,thePtr->contrlRect.top,thePtr->contrlRect.right,thePtr->contrlRect.bottom);
	theTicks = TickCount();
	while (Button() && ((TickCount()-theTicks)<GetDblTime())) {
		GetMouse(&thePoint);
		if (PtInRect(thePoint,&thePtr->contrlRect)) {
			HiliteControl(PrefBtn,1);
		} else {
			theTicks = TickCount();					// restart delay after exit button rect
			HiliteControl(PrefBtn,0);
		}
	}
	if (PtInRect(thePoint,&thePtr->contrlRect)) {	// Inside the button
		if (!Button()) {							//Button released -> quick click
			theItem = gSelectedPrefs;	
		} else {									//Button down -> do pop-up menu
			theMenu = GetMHandle(rPrefButtonID);	//make sure there is a menu
			if (theMenu) {
				for (theItem = kGeneralPrefs; theItem <= kMoviePrefs; theItem++) {
					SetItemMark(theMenu, theItem+1, (unsigned char)'�'*(theItem==gSelectedPrefs));
				}
				SetPt(&thePoint,300,33);			// topright position
				LocalToGlobal(&thePoint);
				theChoice = PopUpSelect(theMenu,thePoint,0,kUseWFont,kRightAlign);
				theItem = LoWord(theChoice) - 1;
			} else {
				theItem = gSelectedPrefs;			//Menu not found, open default panel
			}
		}
		if (theItem >= 0) if(!doPreferencesDialog(theItem)) SysBeep(20);
	}
	
	HiliteControl(PrefBtn,0);
}

//--------------------------------------------------------------------------
	OSErr RemoveLicense()
//--------------------------------------------------------------------------
{
	SysEnvRec theWorld;
	FSSpec myFSSpec;
	short oe,foundVRefNum;
	long foundDirID;
	
	oe = SysEnvirons(1,&theWorld);
	oe = FindFolder(theWorld.sysVRefNum,kPreferencesFolderType,
					false,&foundVRefNum,&foundDirID);
	oe = FSMakeFSSpec(foundVRefNum,foundDirID,"\pAvara License",&myFSSpec);
	oe = FSpDelete(&myFSSpec);
	return oe;
}

//--------------------------------------------------------------------------
	OSErr OpenPref(short *refNum,Str255 filename,signed char perm,short fork)
//--------------------------------------------------------------------------
{
	SysEnvRec theWorld;
	FSSpec myFSSpec;
	short oe,foundVRefNum;
	long foundDirID;
	
	oe = SysEnvirons(1,&theWorld);
	oe = FindFolder(theWorld.sysVRefNum,kPreferencesFolderType,
					false,&foundVRefNum,&foundDirID);
	oe = FSMakeFSSpec(foundVRefNum,foundDirID,filename,&myFSSpec);
	if (fork == kDataFork) {
		if (oe == fnfErr /*&& perm != fsRdPerm*/ )
			oe = FSpCreate(&myFSSpec,kAvaraType,kPrefType,smSystemScript);
			FSpCreateResFile(&myFSSpec,kAvaraType,kPrefType,smSystemScript);
		if (oe == noErr) {
			oe = FSpOpenDF(&myFSSpec,perm,refNum);
			oe = SetFPos(*refNum,fsFromStart,0);
		}
	} else {
		if (oe == fnfErr /*&& perm != fsRdPerm*/ ) {
			oe = FSpCreate(&myFSSpec,kAvaraType,kPrefType,smSystemScript);
			FSpCreateResFile(&myFSSpec,kAvaraType,kPrefType,smSystemScript);
			//oe = ResError();
		}
		
		if (oe == noErr) {
			*refNum = FSpOpenResFile(&myFSSpec,perm);
			oe = ResError();
		}
	}
	
	return oe;
}

//--------------------------------------------------------------------------
	OSErr FlushPrefVol(void)
//--------------------------------------------------------------------------
{
	SysEnvRec		theWorld;
	short			oe,foundVRefNum;
	long			foundDirID;
	
	oe = SysEnvirons(1,&theWorld);
	oe = FindFolder(theWorld.sysVRefNum,kPreferencesFolderType,
					false,&foundVRefNum,&foundDirID);
	oe = FlushVol("\p",foundVRefNum);
	return oe;
}

//--------------------------------------------------------------------------
	void ReadPref(void)
//--------------------------------------------------------------------------
{
	short			prefVer,refNum,oe,i;
	char			len;
	SInt16			scrollPos;
	long			count, dirID;
	int				vRefNum;
	Str255			filename;
	MenuHandle		myMenu;
	FSSpec			myFile;
   		
	GetIndString(filename,rPlugStrings,5);
	if (OpenPref(&refNum,filename,fsRdPerm,kDataFork) != noErr) {
		doPrefErrorAlert(rPrefErrors,kPrefOpenError);
		return;
	}
		
	count = 2;	//read preferences version number
	oe = FSRead(refNum,&count,&prefVer);
		
	if ((prefVer != kPrefVersion)) {
		oe = FSClose(refNum);
		doPrefErrorAlert(rPrefErrors,kPrefVersError);
		return;
	}
			
	count = 2;	//read current summary type
	oe = FSRead(refNum,&count,&gSummaryType);
	SetCtlValue(SummaryBtn,gSummaryType);

	count = 2;	//read current panel position
	oe = FSRead(refNum,&count,&currentPanel);
	SetCtlValue(PopUpBtn,currentPanel);

	count = 2;	//read scrollbar position
	oe = FSRead(refNum,&count,&scrollPos);
	SetCtlValue(AboutScrollBar,scrollPos);

	count = 2;	//read start column position
	oe = FSRead(refNum,&count,&startColumn);

	for (i=0;i<noColumns;i++) {	//read column order
		count = 2;
		oe = FSRead(refNum,&count,&columnOrder[i]);
	} // for i

	count = 2;	//read the current Pref panel number
	oe = FSRead(refNum,&count,&gSelectedPrefs);
		
	count = sizeof(UserPrefs);
	oe = FSRead(refNum,&count,&UserPrefs);

	oe = FSClose(refNum);
	doAliases();
}

//--------------------------------------------------------------------------
	void WritePref(void)
//--------------------------------------------------------------------------
{
	short			resFile,prefVer,refNum,oe,len,i;
	SInt16			scrollPos;
	long			count;
	Str255			filename;
	Handle			theHandle = NULL;
	
	GetIndString(filename,rPlugStrings,5);
	if (OpenPref(&refNum,filename,fsRdWrPerm,kDataFork) != noErr) {
		doErrorAlert(rPrefErrors,kPrefWriteError);
		return;
	}

	//SetEOF(refNum,0);
	
	count = 2;	//write the preferences verison number
	prefVer = kPrefVersion;
	oe = FSWrite(refNum,&count,&prefVer);

	count = 2;	//write the current summary type
	oe = FSWrite(refNum,&count,&gSummaryType);

	count = 2;	//write the current Score panel number
	oe = FSWrite(refNum,&count,&currentPanel);

	count = 2;	//write scroll bar position
	scrollPos = GetCtlValue(AboutScrollBar);
	oe = FSWrite(refNum,&count,&scrollPos);

	count = 2;	//write start column position
	oe = FSWrite(refNum,&count,&startColumn);

	for (i=0;i<noColumns;i++) {	//read column order
		count = 2;
		oe = FSWrite(refNum,&count,&columnOrder[i]);
	} // for i

	count = 2;	//write the current Pref panel number
	oe = FSWrite(refNum,&count,&gSelectedPrefs);
	
	count = sizeof(PreferencesType);
	oe = FSWrite(refNum,&count,&UserPrefs);
	
//	count = 2;	//write the number of favorite files
//	oe = FSWrite(refNum,&count,&gFavoritesCount);

	oe = FSClose(refNum);
	
	resFile=CurResFile();
	if (OpenPref(&refNum,filename,fsRdWrPerm,kResFork) != noErr) {
		doErrorAlert(rPrefErrors,kPrefWriteError);
		return;
	}
	UseResFile(refNum);
	
	HNoPurge((Handle)(gLogAlias));
	theHandle = GetResource(rAliasType, 1000);
	if (theHandle) {
		ChangedResource(theHandle);
		RemoveResource(theHandle);
		UpdateResFile(refNum);
		DisposeHandle(theHandle);
	}
	AddResource((Handle)(gLogAlias),rAliasType,1000,"\p.ScoreKeeper Log");
	UpdateResFile(refNum);
	HPurge((Handle)(gLogAlias));
	
	CloseResFile(refNum);
	UseResFile(resFile);
	oe = FlushPrefVol();
}

long PopUpSelect (MenuHandle theMenu, Point popPt, short popupItem, Boolean useWFont, Boolean rightAlign)
{
    short       item;
    short       itemMark;
    short       oldSysFont, oldWMgrFont, oldCWMgrFont;
    short       oldSysSize, oldWMgrSize, oldCWMgrSize;
    GrafPtr     curPort, wMgrPort;
    CGrafPtr    wMgrCPort;
    SysEnvRec   theWorld;

	oldSysFont = LMGetSysFontFam();
	oldSysSize = LMGetSysFontSize();

    	/* check if we need to use window font */

    GetPort (&curPort);
    if (curPort->txFont == oldSysFont && curPort->txSize == oldSysSize)
        useWFont = false;		/* window font _is_ system font! */

    if (useWFont) {
        /* hack to fix bugs caused by programs that mess up the WindowMgr port(s) */
		/*  (e.g. MacWrite & Word)  - thanks to Leonard Rosenthal for soln */
        GetWMgrPort (&wMgrPort);
        SetPort (wMgrPort);
        oldWMgrFont = wMgrPort->txFont;
        oldWMgrSize = wMgrPort->txSize;
        TextFont (systemFont);
        TextSize (0);
        
        if (SysEnvirons (1, &theWorld) == noErr && theWorld.hasColorQD) {
            GetCWMgrPort (&wMgrCPort);
            SetPort ((GrafPtr)wMgrCPort);
            oldCWMgrFont = wMgrCPort->txFont;
            oldCWMgrSize = wMgrCPort->txSize;
            TextFont (systemFont);
            TextSize (0);
            //TextFont (wMgrCPort->txFont);
            //TextSize (wMgrCPort->txSize);
        } else
            theWorld.hasColorQD = false;
            
            
        SetPort (curPort);

        LMSetSysFontFam(curPort->txFont);
        LMSetSysFontSize(curPort->txSize);
        /* When changing the font of a (popup) menu by hooking the MDEF, LastSPExtra 
		must be set to -1 to clear the font cache both before calling the MDEF 
		and after restoring the font stuff.*/

        LMSetLastSPExtra(-1L);
        
    }

    if (popupItem > 0) {			/* save old mark and check item */
	    GetItemMark (theMenu, popupItem, &itemMark);
        SetItemMark (theMenu, popupItem, useWFont? '*' : checkMark);
    }
    	
    if (rightAlign) {
    	CalcMenuSize(theMenu);
		popPt.h = popPt.h - (*theMenu)->menuWidth;
	}

    item = PopUpMenuSelect (theMenu, popPt.v, popPt.h, (popupItem>0) ? popupItem : 1);

    if (popupItem > 0)				/* restore old item mark */
        SetItemMark (theMenu, popupItem, itemMark);

    if (useWFont) {
        SetPort (wMgrPort);
        TextFont (oldWMgrFont);
        TextSize (oldWMgrSize);
        if (theWorld.hasColorQD) {
            SetPort ((GrafPtr)wMgrCPort);
            TextFont (oldCWMgrFont);
            TextSize (oldCWMgrSize);
			}
        SetPort(curPort);

        LMSetSysFontSize(oldSysSize);
        LMSetSysFontFam(oldSysFont);
        /* When changing the font of a (popup) menu by hooking the MDEF, LastSPExtra 
		must be set to -1 to clear the font cache both before calling the MDEF 
		and after restoring the font stuff.*/
        LMSetLastSPExtra(-1L);
    }

    return item;
}

// The user clicked in the Favorites Icon
// Display the popup menu with the Favorite levels
// Handle the choosen item

void doFavorites(void)
{
	MenuHandle		myMenu;
	long			theChoice;
	int				theItem = 0;
	Point			thePoint;
	Str255			theMenuItem;
  
 	HiliteControl(FavoritesBtn,1);
	myMenu = GetMHandle(rFavoritesButtonID);	// make sure there is a menu
	if (myMenu) {
		SetPt(&thePoint,322,33);				// topright position
		LocalToGlobal(&thePoint);
		theChoice = PopUpSelect(myMenu,thePoint,0,kUseWFont,kRightAlign);
		theItem = LoWord(theChoice);
	} else {
  		doErrorAlert(rFavoritesErrors,kFavUnexpectedError);
  		HiliteControl(FavoritesBtn,255);
  		return;
	}
	
	switch (theItem) {
		case 0: break;							// Canceled; take no action
		case 1:	doAddFavorite();				// Add new favorite
				break;
		case 2: if (doCancelOKAlert(rFavoritesErrors,kFavWarnError) == 2) {
					clearFavorites();
				}
				break;
		default: 
				doOpenFavorite(theItem);		// Send AppleEvent
				break;
	}
		
  	HiliteControl(FavoritesBtn,0);
}

void clearFavorites(void)
{
	OSErr 				myErr;
	Str255				prefName;
	short				refNum,resFile,i,count;
	Handle				theHandle;

	GetIndString(prefName,rPlugStrings,5);		// "ScoreKeeper Preferences"
	resFile=CurResFile();
	if (OpenPref(&refNum,prefName,fsRdWrPerm,kResFork) != noErr) {
		doErrorAlert(rFavoritesErrors,kFavReadError);
		return;
	}
	UseResFile(refNum);
	
	while (CountResources(kKeybType)) {
		theHandle = GetIndResource(kKeybType,1);
		if (theHandle) {
			ChangedResource(theHandle);
			RemoveResource(theHandle);
			DisposeHandle(theHandle);
			UpdateResFile(refNum);
		}
	}
	while (CountResources(kLevlType)) {
		theHandle = GetIndResource(kLevlType,1);
		if (theHandle) {
			ChangedResource(theHandle);
			RemoveResource(theHandle);
			DisposeHandle(theHandle);
			UpdateResFile(refNum);
		}
	}
	while (CountResources(kUserType)) {
		theHandle = GetIndResource(kUserType,1);
		if (theHandle) {
			ChangedResource(theHandle);
			RemoveResource(theHandle);
			DisposeHandle(theHandle);
			UpdateResFile(refNum);
		}
	}
	
	UpdateResFile(refNum);
	CloseResFile(refNum);
	UseResFile(resFile);
	
	doAliases();
}

void clearRecent(void)
{
	OSErr 				myErr;
	//MenuHandle			theMenu;
	Str255				prefName;
	short				refNum,resFile,i,count;
	Handle				theHandle;

	GetIndString(prefName,rPlugStrings,5);		// "ScoreKeeper Preferences"
	resFile=CurResFile();
	if (OpenPref(&refNum,prefName,fsRdWrPerm,kResFork) != noErr) {
		doErrorAlert(rFavoritesErrors,kFavReadError);
		return;
	}
	UseResFile(refNum);
	
	count = CountResources(kRcntType);
	for (i = 1 ; i <= count ; i++) {
		theHandle = GetIndResource(kRcntType,i);
		if (theHandle) {
			ChangedResource(theHandle);
			RemoveResource(theHandle);
			DisposeHandle(theHandle);
		}
	}
	count = CountResources(kRcntType);
	for (i = 1 ; i <= count ; i++) {
		theHandle = GetIndResource(kRcntType,i);
		if (theHandle) {
			ChangedResource(theHandle);
			RemoveResource(theHandle);
			DisposeHandle(theHandle);
		}
	}
	
	UpdateResFile(refNum);
	CloseResFile(refNum);
	UseResFile(resFile);
	
	//doAliases();
}

// Display an OpenFile dialog to choose the file
// Loop to the preferences file and check if it's already there
// If so, replace its FSSpec
// If not, replace its FSSpec

void doAddFavorite(void)
{
	OSErr 				myErr;
	StandardFileReply	theReply;
	SFTypeList 			theTypes;
	MenuHandle			theMenu;
	Str255				prefName,fileName;
	FSSpec				theFile;
	short				refNum,resFile,resID = 0;
	AliasHandle			theAlias;
	FInfo				documentFInfo;
		
		
	theTypes[0] = kKeybType;
	theTypes[1] = kUserType;
	theTypes[2] = kLevlType;
	//theTypes[3] = kPlugType;			//Don't allow quickly loading other plug-ins

	StandardGetFile(nil, 3, theTypes, &theReply);
	if (theReply.sfGood) {
		if (FSpGetFInfo(&theReply.sfFile, &documentFInfo) != noErr) {
			doErrorAlert(rFavoritesErrors,kFavWriteError);
			return;
		}
		theMenu = GetMHandle(rFavoritesButtonID);	//append to favorites menu
		if (theMenu) {
			GetIndString(prefName,rPlugStrings,5);		// "ScoreKeeper Preferences"
			resFile=CurResFile();
			if (OpenPref(&refNum,prefName,fsRdWrPerm,kResFork) != noErr) {
				doErrorAlert(rFavoritesErrors,kFavWriteError);
				return;
			}
			UseResFile(refNum);
			cleanUp(theReply.sfFile.name,fileName);
			theAlias = (AliasHandle)(GetNamedResource(documentFInfo.fdType, fileName));
			if (theAlias) {
				ChangedResource((Handle)(theAlias));
				RemoveResource((Handle)(theAlias));
				UpdateResFile(refNum);
				DisposeHandle((Handle)(theAlias));
			}
			
			myErr = NewAlias(&gPlugSpec,&theReply.sfFile,&theAlias);
			if (myErr != noErr) {
				doErrorAlert(rFavoritesErrors,kFavWriteError);
				CloseResFile(refNum);
				UseResFile(resFile);
			}
			
			while (resID<2000) {
				resID = UniqueID(rAliasType);
			}
			HNoPurge((Handle)(theAlias));
			cleanUp(theReply.sfFile.name,fileName);
			AddResource((Handle)(theAlias),documentFInfo.fdType,resID,fileName);
			UpdateResFile(refNum);
			HPurge((Handle)(gLogAlias));

			CloseResFile(refNum);
			UseResFile(resFile);
	
			doAliases();
		}
	}
}

// Given a name of a level in the menu
// Open the level using the file alis record in the preferences
// Send an 'odoc' AppleEvent to Avara

void doOpenFavorite(short theItem)
{
	OSErr			theErr;
	Str255			prefName;
	short			refNum,resFile,theType;
	FSSpec			theFile;
	AliasHandle		theAlias = NULL;
	unsigned char	wasChanged;
	Str255			theMenuItem;
	MenuHandle		myMenu;
	KeyMap			myKeyMap;

	if (gPlugLevel != kPayed) {
  		doErrorAlert(rFavoritesErrors,kFavDemoError);
   		HiliteControl(FavoritesBtn,0);
 		return;
	}

	GetIndString(prefName,rPlugStrings,5);		// "ScoreKeeper Preferences"
	resFile=CurResFile();
	if (OpenPref(&refNum,prefName,fsRdWrPerm,kResFork) != noErr) {
		doErrorAlert(rFavoritesErrors,kFavReadError);
		return;
	}
	UseResFile(refNum);
	
	myMenu = GetMHandle(rFavoritesButtonID);	//make sure there is a menu
 	GetItem(myMenu,theItem,theMenuItem);
 	theType = findFavoritesType(theItem);
	switch (theType) {
		case 1: theAlias = (AliasHandle)(GetNamedResource(kUserType, theMenuItem));
				break;
		case 2: theAlias = (AliasHandle)(GetNamedResource(kKeybType, theMenuItem));
				break;
		case 3: theAlias = (AliasHandle)(GetNamedResource(kLevlType, theMenuItem));
				break;
		default: return;
	}
	
	GetKeys(myKeyMap);					// Remove files with command key
	if(myKeyMap[1] & 0x8000) {
		ChangedResource((Handle)(theAlias));
		RemoveResource((Handle)(theAlias));
		UpdateResFile(refNum);
		DisposeHandle((Handle)(theAlias));
		CloseResFile(refNum);
		UseResFile(resFile);
		doAliases();
		return;
	}
	
	DetachResource((Handle)(theAlias));
	CloseResFile(refNum);
	UseResFile(resFile);

	theErr = ResolveAlias(&gPlugSpec,theAlias,&theFile,&wasChanged);
	if (theAlias) DisposeHandle((Handle)(theAlias));
	
	if (theErr != noErr) {
		doErrorAlert(rFavoritesErrors,kFavOpenError);
		return;
	}
	theErr = OpenSpecifiedDocument(&theFile);
	if (theErr != noErr) doErrorAlert(rFavoritesErrors,kFavOpenError);
}

short resolveLogAlias()									//renewal of alis record not yet implemented
{
	short				myErr;
	unsigned char		wasChanged;
	
	myErr = ResolveAlias(&gPlugSpec,gLogAlias,&UserPrefs.logFile,&wasChanged);
	if (myErr == fnfErr) {
		myErr = FSpCreate(&UserPrefs.logFile, 'R*ch', 'TEXT', smCurrentScript);
		if (myErr != noErr) {
			UserPrefs.logFlag = kFileCancelled;			// Recreation Error
			UserPrefs.logEnabled = false;
			UserPrefs.logTime = kLogEachGame;
			gSelectedPrefs = kLogPrefs;
			doPrefErrorAlert(rLogFileStrings,12);
		}
	}
	return myErr;
}

void doAliases(void)
{
	short			refNum,resFile,menuCount,i;
	MenuHandle		myMenu;
	FSSpec			myFile;
	Handle			theHandle;
	Str255			filename;

	myMenu = GetMHandle(rFavoritesButtonID);	//dispose of previous menu
	if (myMenu) {
		DeleteMenu(rFavoritesButtonID);
		DisposeMenu(myMenu);
		DrawMenuBar();
	}

	myMenu = NewMenu(rFavoritesButtonID,"\p");	//build new menu
  	AppendMenu(myMenu,"\pAdd File�");
  	AppendMenu(myMenu,"\pClear All�");

	GetIndString(filename,rPlugStrings,5);
	resFile=CurResFile();
	if (OpenPref(&refNum,filename,fsRdWrPerm,kResFork) != noErr) {
		doErrorAlert(rPrefErrors,kPrefWriteError);
		return;
	}
	UseResFile(refNum);
	UpdateResFile(refNum);

	/*resCount = CountResources(rAliasType);
	SetResLoad(false);
	for (i=1; i <= resCount; i++) {
		theHandle = GetIndResource(rAliasType,i);
		if (theHandle) {
			GetResInfo(theHandle,&resID,&theType,filename);
			if (ResError() == noErr) AppendMenu(myMenu,filename);
		}
	}
	SetResLoad(true);*/
	
	if (CountResources(kUserType) > 0 ) {
		AppendMenu(myMenu,"\p-");
		AddResMenu(myMenu,kUserType);
	}
	if (CountResources(kKeybType) > 0 ) {
		AppendMenu(myMenu,"\p-");
		AddResMenu(myMenu,kKeybType);
	}
	if (CountResources(kLevlType) > 0 ) {
		AppendMenu(myMenu,"\p-");
		AddResMenu(myMenu,kLevlType);
	}
	menuCount = CountMItems(myMenu);
	if (menuCount <= 2) DisableItem(myMenu,2);	// Can't clear menu
	for (i=1; i<=menuCount;i++) {
		SetItemStyle(myMenu,i,bold);
	} // for i
	InsertMenu(myMenu,kIsPopUpMenu);
	
	myMenu = GetMHandle(rRecentButtonID);		//dispose of previous menu
	if (myMenu) {
		DeleteMenu(rRecentButtonID);
		DisposeMenu(myMenu);
		DrawMenuBar();
	}
	myMenu = NewMenu(rRecentButtonID,"\p");	//build new menu
	if (CountResources(kRcntType) > 0 ) {
		AddResMenu(myMenu,kRcntType);
	} else {
  		AppendMenu(myMenu,"\p(No recent files");
	}
	menuCount = CountMItems(myMenu);
	for (i=1; i<=menuCount;i++) {
		SetItemStyle(myMenu,i,bold);
	} // for i
	InsertMenu(myMenu,kIsPopUpMenu);
	
	DrawMenuBar();
	//CalcMenuSize(myMenu);*/

	gLogAlias = (AliasHandle)(GetResource(rAliasType, 1000));
	DetachResource((Handle)(gLogAlias));
	CloseResFile(refNum);
	UseResFile(resFile);
}

short findFavoritesType(short theItem)			//preffile MUST be open
{
	theItem = theItem - 2;  // Add & Clear item

	if (CountResources(kUserType) > 0) theItem = theItem - CountResources(kUserType) - 1;
	if (theItem <= 0) return 1;
	
	if (CountResources(kKeybType) > 0) theItem = theItem - CountResources(kKeybType) - 1;
	if (theItem <= 0) return 2;
		
	return 3;
}

void doRecent(void)
{
	MenuHandle		myMenu;
	long			theChoice;
	int				theItem = 0;
	Point			thePoint;
	Str255			theMenuItem;
	OSErr			theErr;
	Str255			prefName;
	short			refNum,resFile,theType;
	FSSpec			theFile;
	AliasHandle		theAlias = NULL;
	unsigned char	wasChanged;
  
 	HiliteControl(RecentBtn,1);
	myMenu = GetMHandle(rRecentButtonID);		//make sure there is a menu
	if (myMenu) {
		SetPt(&thePoint,344,33);				// topright position
		LocalToGlobal(&thePoint);
		theChoice = PopUpSelect(myMenu,thePoint,0,kUseWFont,kRightAlign);
		theItem = LoWord(theChoice);
		if (theItem == 0) {
   			HiliteControl(RecentBtn,0);
			return;
		}
	} else {
  		doErrorAlert(rRecentErrors,kRecUnexpectedError);
  		HiliteControl(RecentBtn,255);
  		return;
	}

	if (gPlugLevel != kPayed) {
  		doErrorAlert(rRecentErrors,kRecDemoError);
   		HiliteControl(RecentBtn,0);
 		return;
	}

	GetIndString(prefName,rPlugStrings,5);		// "ScoreKeeper Preferences"
	resFile=CurResFile();
	if (OpenPref(&refNum,prefName,fsRdWrPerm,kResFork) != noErr) {
		doErrorAlert(rRecentErrors, kRecOpenError);
  		HiliteControl(RecentBtn,255);
		return;
	}
	UseResFile(refNum);
	
	myMenu = GetMHandle(rRecentButtonID);	//make sure there is a menu
 	GetItem(myMenu,theItem,theMenuItem);
	theAlias = (AliasHandle)(GetNamedResource(kRcntType, theMenuItem));
		
	DetachResource((Handle)(theAlias));
	CloseResFile(refNum);
	UseResFile(resFile);

	theErr = ResolveAlias(&gPlugSpec,theAlias,&theFile,&wasChanged);
	if (theAlias) DisposeHandle((Handle)(theAlias));
	
	if (theErr != noErr) {
		doErrorAlert(rRecentErrors,kRecAliasError);
	  	HiliteControl(RecentBtn,0);
		return;
	}
	theErr = OpenSpecifiedDocument(&theFile);
	if (theErr != noErr) doErrorAlert(rRecentErrors,kRecEventError);
		
  	HiliteControl(RecentBtn,0);
}

void addRecentFile(short theRef)
{
	FSSpec				theFile;
	OSErr				theErr;
	MenuHandle			theMenu;
	Str255				prefName,fileName;
	short				refNum,resFile,resID = 0;
	AliasHandle			theAlias;

	if (GetControlValue(RecentBtn) == 255) return;	//Recent menu is diabled due to an error

	if (FSpGetFileLocation(theRef,&theFile) != noErr) {
		doErrorAlert(rRecentErrors,kRecResolveError);
		return;
	}

	GetIndString(prefName,rPlugStrings,5);			// "ScoreKeeper Preferences"
	resFile=CurResFile();
	if (OpenPref(&refNum,prefName,fsRdWrPerm,kResFork) != noErr) {
  		doErrorAlert(rRecentErrors,kRecOpenError);
  		HiliteControl(RecentBtn,255);
		return;
	}
		
	UseResFile(refNum);
	cleanUp(theFile.name,fileName);
	theAlias = (AliasHandle)(GetNamedResource(kRcntType, fileName));
	if (theAlias) {
		ChangedResource((Handle)(theAlias));
		RemoveResource((Handle)(theAlias));
		UpdateResFile(refNum);
		DisposeHandle((Handle)(theAlias));
	}
	
	theErr = NewAlias(&gPlugSpec,&theFile,&theAlias);
	if (theErr != noErr) {
		doErrorAlert(rRecentErrors,kRecWriteError);
		CloseResFile(refNum);
		UseResFile(resFile);
	}
	
	while (resID<2000) {
		resID = UniqueID(kRcntType);
	}
	
	HNoPurge((Handle)(theAlias));
	cleanUp(theFile.name,fileName);
	AddResource((Handle)(theAlias),kRcntType,resID,fileName);
	UpdateResFile(refNum);
	HPurge((Handle)(gLogAlias));

	CloseResFile(refNum);
	UseResFile(resFile);

	doAliases();
}
