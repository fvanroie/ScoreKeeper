/*
	File:		Speech.r

	Contains:	Rez definitions for App-defined Pronunciation Dictionaries

	Copyright:	� 1992-1994 by Apple Computer, Inc., all rights reserved.

*/

#include "SysTypes.r"


// 'dict' - User dictionary resource

type 'dict' {
	longint = DictEnd >> 3;	// total byte length
	longint = 'dict';		// Dictionary atom
	longint = 1;			// format version
	integer Script;			// script system (e.g. smRoman)
	integer Language;		// language code (e.g. langEnglish)
	integer Region;			// region code 	 (e.g. verUS)
	unsigned longint;		// date last modified (seconds since Jan 1, 1904)
	longint = 0;			// reserved 1
	longint = 0;			// reserved 2
	longint = 0;			// reserved 3
	longint = 0;			// reserved 4
	longint = $$CountOf(EntryArray);	// entry count
	array EntryArray {
EntryBegin:
		integer = (EntryEnd[$$ArrayIndex(EntryArray)] -
				   EntryBegin[$$ArrayIndex(EntryArray)])
				  >> 3;								// entry byte length
		integer null,pron=0x21,abbr=0x22;			// entry type 
		integer = $$CountOf(FieldList);				// field count
		wide array FieldList {
FieldBegin:
			integer = (FieldEnd[$$ArrayIndex(EntryArray),$$ArrayIndex(FieldList)] -
					   FieldBegin[$$ArrayIndex(EntryArray),$$ArrayIndex(FieldList)])
					  >> 3;							// field byte length (excluding padding)
			integer null,tx=0x21,ph=0x22,pos=0x23;	// field type
			string;									// field data
FieldEnd:
			align word;
		};
EntryEnd:
	};
	
DictEnd:					// end of dictionary resource
};

// Note, the ThisSecond value in the following resource can be 
// easily defined when Rez is invoked.  For example:
//		rez -o Sample.rsrc -d "ThisSecond=`date -n`" DictSample.r

#ifndef ThisSecond
#define	ThisSecond	0
#endif

/* Here's a sample dictionary to demonstrate the proper format:

resource 'dict' (1, "TestDict") {
	smRoman, langEnglish, verUS, ThisSecond,
	{
		pron, {tx, "SARAH", 		ph, "_m2AY_s1IHst2AXr"},
		pron, {tx, "ROOSEVELT", 	ph, "_1EHf_d1IY_1AAr"},
		pron, {tx, "SAMUEL", 		ph, "_s1AEm_AY_2AEm"},
		pron, {tx, "SHAKESPEARE", 	ph, "_D2UX_b1AArd_2UXv_1AEv2AAn"},
		pron, {tx, "OHIO", 			ph, "_1OW%%h1AY%%1OW"},
		pron, {tx, "ALEXANDER", 	ph, "_1AEl_D2UX_p2AEl"},
		pron, {tx, "CHICAGO", 		ph, "_st1IHNk2IHN_1UXhny2IHn"},
		pron, {tx, "BEARD", 		ph, "_mUXstAES"},
		pron, {tx, "DAVID", 		ph, "_gAAl1AYAETs_f1OW"},
		pron, {tx, "DEMOCRATS", 	ph, "_r2IHp1UXblIHk2IHn"},
		pron, {tx, "YANKEES", 		ph, "_r1EHd_s1AAks"},
		pron, {tx, "RACHEL", 		ph, "_m2AY_n1IYs"},
		pron, {tx, "NAKED", 		ph, "_ny1UWd"},
		pron, {tx, "OCTOBER", 		ph, "_h1AAl2OW_w1IYn"},
		pron, {tx, "MASSACHUSETTS", ph, "_t1AEks_2UX_C1UWsEHts"},
		pron, {tx, "MISSISSIPPI", 	ph, "_fl1UXd_k1UXnCr2IY"},
		pron, {tx, "HANOVER", 		ph, "_d1AArtmUXT_t1AWn"},
		pron, {tx, "GREENWICH", 	ph, "_h1OWm_2UXv_D2UX_r1IHC"},
		pron, {tx, "FELIX", 		ph, "_f1IYl2IHks_D2UX_k1AEt"},
		pron, {tx, "WEDNESDAY", 	ph, "_m1IHd_w1IYk"},
	},
};

*/