#define kAvaraType		   	'AVAR'
#define kPrefType		   	'AVAP'
#define kLevlType			'AVAL'
#define kPlugType			'AVSI'
#define kKeybType			'AVKB'
#define kUserType			'AVAG'
#define kMoovType			'TVOD'
#define kRcntType			'rcnt'

// RESOURCES 
#define rPlugStrings		1000
#define rAboutPICT			1202
#define rTitleStrings		1900
#define rSummaryButtonID	2001
#define rSummaryMenuID		2001
#define rPopUpButtonID		1999
#define rPopUpMenuID		1999
#define rPrefButtonID		2000
#define rPrefMenuID			2000
#define rScrollBarID		2002
#define rFavoritesButtonID	2003
#define rFavoritesMenuID	2003
#define rRecentButtonID		2004
#define rRecentMenuID		2004
#define rPrefDialogID		2020
#define rPrefScrollBarID	2020
#define rPrefTopicStrings	2020
#define rPrefDITLs			2021
#define rLogFormatID		2100

//#define rTagArrows			2000    // green = 2000 -> Blue = 2006
#define rLeftArrow			2008
#define rRightArrow			2009
#define rAmmoIcons			2010    // shot = 2010, grenade = 2011, missle = 2012
#define rMovieIcons			2015
#define rPrefTopicIcons		2020

// STRINGS
#define rErrorAlert			1010
#define rErrorStrings		1010
#define kPanelError			1
#define	kPictLoadError		2
#define kQuickDrawError		3
#define kPopUpLoadError		4
#define	kPopUpUnavailable	5

#define rPrefErrorAlert		1011
#define rPrefErrors			1011
#define rUnknowOpcode		1022
#define kPrefWriteError		1
#define	kPrefOpenError		2
#define	kPrefVersError		3

#define rCancelOKAlert		1012
#define rFavoritesErrors	1012
#define kFavDemoError		1
#define kFavOpenError		2
#define kFavWarnError		3
#define kFavUnexpectedError	4
#define kFavWriteError		5
#define kFavReadError		6

#define rMovieErrors		1013
#define kMovQDError			1
#define kMovDataError		2
#define kMovMemError		3
#define kMovMediaError		4
#define kMovCompressError	5
#define kMovEditError		6
#define kMovTrackError		7
#define kMovCreateError		8
#define kMovWindowError		9

#define rRecentErrors		1014
#define kRecResolveError	1
#define kRecOpenError		2
#define kRecWriteError		3
#define kRecDemoError		4
#define kRecAliasError		5
#define kRecEventError		6
#define kRecUnexpectedError	7

#define rColorStrings		2022
#define kNewTeamColor		1
#define kNoLEDI				2
#define kBadLEDI			3
#define kNoPICT				4
#define kAdjustTeamColor	5

#define rSpeechStrings		2024
#define kSpeechError		1

#define rLogFileStrings		2025

// LENGTHS
#define kPopUpMenuLength	6  // 7 with Game Statistics
#define kSummaryMenuLength	6  // Total, Min, Max, Average, -, Disabled
#define kLogFormatLength	3

#define kLineLength			80
#define kTitleHeight		17
#define kTextHeight			11
#define kRowHeight			15		

#define kReturn					(SInt8) 0x0D
#define kEnter					(SInt8) 0x03
#define kEscape					(SInt8) 0x1B
#define kPeriod					(SInt8) 0x2E

#define iOK						1
#define iCancel					2
#define iDefault				3
#define iOKDefault				4
#define iPrefHeader				5
#define iPrefIcons				6
//#define iUserRect				5

#define kPrefCount				5		// CNTL 2020 needs to be adjusted too!!!
//#define iPrefScrollBar		5

#define mEdit					130
#define iCut					3
#define iCopy					4
#define iPaste					5
#define iClear					6

#define kPrefVersion			1207	// 1.2.0 revison 7   -> ScoreKeeper 1.2 GM

#define kCompPlayer				-1

#define kIsPopUpMenu			-1
#define kUseWFont				true
#define kRightAlign				true

struct PreferencesType {
	short					gnlRmveClan, gnlUpdate, gnlNegRed, gnlSwitch, gnlPanel;
	short					logEnabled, logFormat, logTime, logFlag; FSSpec logFile;
	short					spkSkipLT, spkEnabled, spkVoice, spkRate;
	short					clrEnabled; RGBColor teamColor[6];
	short					movEnabled, movIcon, movFlag, movRate; FSSpec movFile;
} PreferencesType;

typedef enum {
	kAboutPanel = 1,
	kPlayerPanel = 3,
	kTeamPanel = 4,
	kPlayerStatsPanel = 6,
	kGameStatsPanel = 7
} ScorePanel;

typedef enum {
	kTotalSummary = 1,
	kMinimumSummary = 2,
	kMaximumSummary = 3,
	kAverageSummary = 4
} SummaryType;

typedef enum {
	kExactCmp = 1,
	kStartCmp = 2,
} StringComparisons;

typedef enum {
	kPayed = 0,
	kDemo = 1,
	kLicenseError = 2
} PlugLevels;

typedef enum {
	kGeneralPrefs = 0,
	kColorPrefs = 1,
	//kSoundPrefs = 2,
	kSpeechPrefs = 2,
	kLogPrefs = 3,
	kMoviePrefs = 4
	//kOnlinePrefs = 5
} PrefPanel;

typedef enum {
	kLogEachGame = 3,
	kLogEachSession = 4,
	kLogOneFile = 5
} logSaveTimes;

typedef enum {
	kShotColumn = 0,
	kGrenadeColumn = 1,
	kMissileColumn = 2,
	kKillColumn = 3,
	kBonusColumn = 4,
	kDamageColumn = 5,
	kBallColumn = 6,
	kGoalColumn = 7,
	kTotalColumn = 8
} ColumnTypes;

typedef enum {
	kCompTeam = 0,
	kGreenTeam = 1,
	kYellowTeam = 2,
	kRedTeam = 3,
	kPinkTeam = 4,
	kPurpleTeam = 5,
	kBlueTeam = 6
} PlayerTeam;

typedef enum {
	kFileNotYetSelected,
	kFileSelected,
	kFileCancelled
} fileHandlingStatus;

typedef enum {
	kDataFork = 0,
	kResFork = 1
} FileForks;
