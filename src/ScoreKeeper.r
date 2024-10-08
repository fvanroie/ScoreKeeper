#include "Types.r"
#include "BalloonTypes.r"
#include "Types.r"
#include "Speech.r"


#define kIconHelpString			1300
#define kPrefHelpString			1301
#define kPopupHelpString		1302
#define kLogFormatHelpString	1303

resource 'dict' (1200, "Speech Dictionary", purgeable)
{
	smRoman, langEnglish, verUS, ThisSecond,	/* Header information. */
	{
		pron, {tx, "LATENCY",		ph, "_2lEYy=1thAXn=1sIY"},
		pron, {tx, "ACHILLES",		ph, "_2AE>h=1kIHl=2IYs"},
		pron, {tx, "LOGIN",			ph, "_2lAOgIHn"},
	},	
};

resource 'hfdr' (-5696, "Finder Help String", purgeable)
{
	HelpMgrVersion, hmDefaultOptions, 0, 0,	/* Header information. */
	{HMSTRResItem {kIconHelpString}}
};

/*
resource 'hdlg' (2020, "Preferences Balloon Help", purgeable)
{
	HelpMgrVersion, 0, hmDefaultOptions, 0,
	0, 							// Balloon variation code
	HMSkipItem {				// no missing items help information
		},
	{
		HMStringResItem {		// Store Help for OK button in 'STR#' 1301 
			{0,0},				// default tip: middle right edge of item rect
			{0,0,0,0},			// default alternate rect: use item rect
			kPrefHelpString,1,	// enabled OK button
			0,0,				// OK button is never dimmed
			0,0,				// no enabled-and-checked stat for button
			0,0					// no other marked states for button
		},
		HMStringResItem {		// Store Help for Cancel button in 'STR#' 1301 
			{0,0},				// default tip: middle right edge of item rect
			{0,0,0,0},			// default alternate rect: use item rect
			kPrefHelpString,2,	// enabled Cancel button
			0,0,				// Cancel button is never dimmed
			0,0,				// no enabled-and-checked stat for button
			0,0					// no other marked states for button
		},
		HMStringResItem {		// Store Help for Default button in 'STR#' 1301 
			{0,0},				// default tip: middle right edge of item rect
			{0,0,0,0},			// default alternate rect: use item rect
			kPrefHelpString,3,	// enabled Default button
			kPrefHelpString,4,	// Default button is dimmed
			0,0,				// no enabled-and-checked stat for button
			0,0					// no other marked states for button
		},
		HMStringResItem {		// Store Help for icons rectangle in 'STR#' 1301 
			{120,80},			// place tip in right bottom corner
			{0,0,0,0},			// default alternate rect: use item rect
			kPrefHelpString,5,	// enabled Icons rectangle
			0,0,				// Icons are never dimmed
			0,0,				// no enabled-and-checked stat for button
			0,0					// no other marked states for button
		}
	}
};

resource 'hmnu' (1999,"Pop-up menu Help",purgeable) 
{
	// Header information
		HelpMgrVersion, hmDefaultOptions, 0,
		0, 							// Balloon variation code
		HMSkipItem {				// no missing items help information
			},
		{
			HMSkipItem {			// no balloon help for title of pop-up menu
				},
			HMStringResItem {		// About ScoreKeeper Item 
				kPopupHelpString,1,	// 'STR#' res id, index when item is enabled
				0,0,				// 'STR#' res id, index when item is dimmed
				0,0,				// 'STR#' res id, command is checked
				kPopupHelpString,2	// 'STR#' res id, command can't be marked
			},
			HMSkipItem {			// no balloon help for dashed item
				},
			HMStringResItem {		// Player Scores Item 
				kPopupHelpString,3,	// 'STR#' res id, index when item is enabled
				0,0,				// 'STR#' res id, index when item is dimmed
				0,0,				// 'STR#' res id, command is checked
				kPopupHelpString,4	// 'STR#' res id, command can't be marked
			},
			HMStringResItem {		// Team Scores Item 
				kPopupHelpString,5,	// 'STR#' res id, index when item is enabled
				0,0,				// 'STR#' res id, index when item is dimmed
				0,0,				// 'STR#' res id, command is checked
				kPopupHelpString,6	// 'STR#' res id, command can't be marked
			},
			HMSkipItem {			// no balloon help for dashed item
				},
			HMStringResItem {		// Player Statistics Item 
				kPopupHelpString,7,	// 'STR#' res id, index when item is enabled
				0,0,				// 'STR#' res id, index when item is dimmed
				0,0,				// 'STR#' res id, command is checked
				kPopupHelpString,8	// 'STR#' res id, command can't be marked
			}
		}
};
		
resource 'hmnu' (2100,"Log format menu Help",purgeable) 
{
	// Header information
		HelpMgrVersion, hmDefaultOptions, 0,
		0, 							// Balloon variation code
		HMSkipItem {				// no missing items help information
			},
		{
			HMSkipItem {			// no balloon help for title of pop-up menu
				},
			HMStringResItem {		// Fixed Width Item 
				kLogFormatHelpString,1,	// 'STR#' res id, index when item is enabled
				0,0,				// 'STR#' res id, index when item is dimmed
				kLogFormatHelpString,2,	// 'STR#' res id, command is checked
				0,0					// 'STR#' res id, command can't be marked
			},
			HMStringResItem {		// Player Scores Item 
				kLogFormatHelpString,3,	// 'STR#' res id, index when item is enabled
				0,0,				// 'STR#' res id, index when item is dimmed
				kLogFormatHelpString,4,	// 'STR#' res id, command is checked
				0,0					// 'STR#' res id, command can't be marked
			},
			HMStringResItem {		// Team Scores Item 
				kLogFormatHelpString,5,	// 'STR#' res id, index when item is enabled
				0,0,				// 'STR#' res id, index when item is dimmed
				kLogFormatHelpString,6,	// 'STR#' res id, command is checked
				0,0					// 'STR#' res id, command can't be marked
			}
		}
};*/