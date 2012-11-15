/* ************************************************************ 
 * File:               set.c
 * 
 * Description:    Some set stuff - a quick test to exercise unit testing under RTRT
 * o A set is like set algebra, a member is a lexical token or name (contains no whitespace (WS))
 * o A set is implemented as a single C string, with members delimetered by WS
 *   I'd  like to hide this implementation.
 * o A set can be represented in "normalised form" - delimitted by single space, no duplicates
 * o Implementation details are represented in a single struct, 'setImp'.
 * o 
 * o DEPRECIATED - A member *can* have an int value. Otherwise int value is zero.
 *
 *
 *  T o     D o 
 * o Introduce some cutest test cases.
 * o Create alternative versions of this file, which
 *   o 0 - Is FULLY MISRA compliant
 *   o 1 - Is BE_MAX compliant
 *   o 2 - Is BE_mid compliant
 *   o 3 - Is BE_min compliant
 * o This whole thing of being able to NOT (dynamically) create your set is a bit sick
 *   o Better introduce propper OO techniques for the set object - alla cuTest's "suite"
 * o
 * o
 *
 *
 *
  ************************************************************** */

#include "set.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

/* #include "lintTidy.h" */

/* What's the best palce to store constants in C  -- Discussion -- ?
   In the .H they must be a #define, but what if you want to inspect them
	 with a debugger - You want a var., so therefore they must live in an obj file,
	 but then they loose compile time visibility, and it's assoc static analysis advantages...
	 Ah well  - here we can check them statically, and view them with a debugger, the only
	            problem is that set.c uses the variable, and client's of set.h must use the
							#defines. so we loose connectedness with the var.
	 o In unit testing, we want to reduce the values in order that reasonably sized
	   test cases can be constructed to perform boundary case tests. - removing the 'const' from the 
		 definition allows this to be overriden in the unit test code.
	 */
CONST_UNLESS_TESTING struct setImplementationStruct setImp = { 
	SET_MAX_MEMBERS_VALUE,
	SET_MAX_CHARS_VALUE,
	SET_MEMBER_SEPARATOR,
	SET_WHITESPACE_CHARS 
};  

// Implementation Type - cStr - Is the chosen implementation of 'set'.  It's a C string.
//    Strictly, a cStr is a 'set' iff it is 'normalised'.
typedef unsigned char *cStr;             /* Have to turn OFF MISRA rule re. absense of signed specifier - cStr are PLAIN */
typedef const char *cStrConst;  /* Have to turn OFF MISRA rule re. absense of signed specifier - cStr are PLAIN */
static const cStr setModName="setModule";

// MISRA-Friendly Local Function Declarations
static myBool      isWhiteSpace(unsigned char c) ;                 // return !=0 if c is whitespace
static void        swapCharPtr(unsigned char **a, unsigned char **b) ;      // Swaps the two char pointers pointed at by params 1 & 2
static setStatus   isBadLength(setConst setA) ;           // returns !=0 if setA has a bad length ( > Max allowed )
static setStatus   isBadIntermediaryLength(setConst setA) ; // returns !=0 if setA has a bad length ( > Max allowed )
static setStatus   listDuplicates(cStr cStrA) ;            // ReWrites setA so that it contains *only* the duplicates
static cStr        listJoin(setConst setA, setConst setB);// Returns new C string that is the concatenation of params
static setStatus   listNormaliseOrDuplicates(set setA, myBool Normalise) ; // Either normalises or listsDuplicates

static void inline swapCharPtr(unsigned char **a, unsigned char **b) {
	unsigned char *t;         t=*a;  *a=*b;  *b=t;
}

static myBool inline isWhiteSpace(unsigned char c) { // FIXME:  Consider pre-processor macro fn() instead for performance.....
	return (myBool) ((c == ' ') || (c == '\t') || (c == '\n'));
}

static setStatus inline isBadLength(setConst setA) {
	if (!setA) return setBadSet;
	if (strlen(setA) >= (size_t)setImp.setMaxChars) return setBadSet;
	return setOK;
}

static setStatus inline isBadIntermediaryLength(setConst setA) {
	if (!setA) return setBadSet;
	if (strlen(setA) >= (size_t) 2 * setImp.setMaxChars) return setBadSet;
	return setOK;
}

static int inline myStrCmp(const char * s1, const char *s2) { // Safer Ver of string.h's strcmp

	if (!s1) return -1;
	if (!s2) return +1;
	return strcmp(s1, s2);
}

/* Prototypes of Module Private functions for MISRA C */

const unsigned char *setStatus2Str(setStatus stat) {
	static const unsigned char *Mssg[]={
		"setOK", "setEmptySet", "setBadSet", "setBadCardinal", "setBadMember", "setMemberNotFound", "setNotEqual"
	};

	static const unsigned char *MssgUnknown="Unknown Status";

	if ((setOK <= stat) && (stat <= setNotEqual))
		return Mssg[stat];
	else
		return MssgUnknown; 
}


setStatus setShow(setConst setA) {
		   if (!setA) {  
		//fprintf(stderr, "%s: setShow: Error: NIL Supplied to setShow\n", setModName);
		return(setBadSet);  
		// FIXME: Check for oversize sets etc.
	} else if (!*setA) {
		//fprintf(stderr, "%s: setShow: Info: Empty Set supplied to setShow\n", setModName);
		return(setOK); // Empty set is a valid set - no error here thanks.
	} else { /* Assume we have a set here */
		/* Stops Further Testing... assert(('a' <= *setA) && (*setA <= 'Z'));  */
		fprintf(stderr, "%s", setA);
	}
	return setOK;
}

sl setCardinal(setConst setA) {
	if (!setA) {   
		//fprintf(stderr, "%s: setCardinal: Error: NIL Supplied to setCardinal\n", setModName);
		return((sl) -1);  /* Cant give a sensible cardinal of a bad set */
	} else if (!*setA) {
		//fprintf(stderr, "%s: setCardinal: Info: Empty Set supplied to setCardinal\n", setModName);
		return(0);
	} else { /* Processing A Set: Assume we have a set here */
		/* Stops Further Testing... assert(('a' <= *setA) && (*setA <= 'Z'));  */
		signed long numMembers=0; /* FIXME: This should be ZERO!! */
		/* Scan the string, counting occurances of WS -> Non-WS */
		/* WHY NOT USE strings.h::strtok here?? */
		while (*setA) {               /* Danger Str-OverRun */
			if (!isWhiteSpace(*setA)) {
				/* At Token or NUL */
				if (!*setA) return(numMembers);
				/* At Token */
				numMembers++;   /* Danger int-OverRun */
				if (numMembers >=setImp.setMaxMembers) {
					/* Some nice exceptional handling here would be nice */
					return((sl) -1);
				}
				/* At start of token: Scan to WS or NUL */
				while (!isWhiteSpace(*setA) && *setA) ++setA; 
				if (!*setA) return (numMembers);
				/* At WS */
			} 
			/* At WS or NUL */
			while (isWhiteSpace(*setA) && *setA) ++setA;
			if (!*setA) return (numMembers);
			/* At Start Of Token: Loop */
		}
		/* At NUL */
		return (numMembers);
	} /* Processing A Set */
}

setStatus      setIsNotMember(memStr aMem, setConst setA) {
/* Assuming the implementation of a set is a str with memberes separated  by WS */
	unsigned char *test;
	size_t	lenMem;
	unsigned char trailChar;

	if (!setA)                return setBadSet;
	if (!aMem)                return setBadMember;
	if (!*setA)               return setEmptySet;
	if (!*aMem)               return setBadMember;

	lenMem=strlen(aMem);    
	if (lenMem >= setImp.setMaxChars)  return setBadMember;
	if (setCardinal((set) aMem) !=1)   return setBadMember;
	test=strstr(setA, aMem);  

	if ( (test!=NULL) ) {  /* Possible Match - Problem with wd boundaries .... */
		/* Check For Trailing non-WS */
		trailChar=*(test + lenMem);  
		if ((!trailChar) || (isWhiteSpace(trailChar))) {  /* No Trailing Problems */
			if ((test==(const unsigned char*)setA) || isWhiteSpace(*(test - 1))) { /* No Leading Problems */
				return setOK;
				}
		} 
	} 
	return setMemberNotFound;
}

static setStatus        listNormaliseOrDuplicates(set setA, myBool Normalise) {
// spec
//  o IN-PLACE normalisation (or listDuplicates) of a set - ie I will write to *setA !
//  o Either 'normalise' functionality -or- reports duplicates, dependent on myBool Normalise
// spec - Normalise
//   (1) Order, (2) Reduce WS to single char, (3) remove duplicates, 
// spec - Duplicates
//   (1) Order, (2) Reduce WS to single char, (3) report only duplicates ((single) entry *only* for those duplicated)
// Design
//   o Take a temporary copy of the supplied string, into local array.
//   o Create local array of ptrs of posn of each token (member) within the copy of the string.
//   o Sort the array of ptrs; 
//   o Generate new string(normalised or duplicates) at location pointed to by original string (formal param),
// Boundary Analysis
//   o Allow supply of double-sized set, and creation of single-sized set.
//     This is because setUnion concats its params and calls normalise.
	signed long MAX_MEMBERS_X_2=2 * setImp.setMaxMembers;
// #define	MAX_MEMBERS_X_2  (2 * setImp.setMaxMembers)

	unsigned char *tT[MAX_MEMBERS_X_2]; // token Table of ptrs to start of each token
	int tTindex=-1;                  // Index into tT
	int numTokens;                   // Number of tokens discovered

#define sCopySize	(1 + 2 * setImp.setMaxChars)
	unsigned char sCopy[sCopySize]; // Copy of supplied set. Use of strtok damages the supplied string.
	unsigned char *aMember;                   // a token       

	int pass;                        // Multiple passes within the sort function...

	// Deal with some exceptional conditions
	if (!setA)                               return setBadSet;  // No set supplied
	if (!*setA)                              return setOK;      // Empty set supplied
	if (isBadIntermediaryLength(setA))       return setBadSet;  // Set TOO large. 
	if (setCardinal(setA) > MAX_MEMBERS_X_2) return setBadSet;  // Set TOO many members THIS DONE IN LOGIC BELOW.....

	// Generate tT - Create this table of ptrs 
	(void) strncpy(sCopy, setA, sCopySize);        
	do {
		aMember=strtok((tTindex == -1) ? sCopy : NULL, setImp.setWhiteSpaceChars);
		if (aMember) {
			if (tTindex == MAX_MEMBERS_X_2 -1) 
				return setBadSet;
			else 
				tT[++tTindex]=aMember;   // Got one 
			}
	} while (aMember);

	// Really poor sort algorithm - but heh it's not production.
	// Could use stdlib's qsort.....
	numTokens=tTindex;
	for (pass=numTokens; pass >=0; pass--) {
		for (tTindex=numTokens; tTindex; tTindex--) {
			if (myStrCmp(tT[tTindex], tT[tTindex -1]) < 0) {
				swapCharPtr(&(tT[tTindex]), &(tT[tTindex -1]));
			}
		}
	}

	*setA=(unsigned char)0; // Bang !! - Sometimes you cannot write to here - unless its a TRUE set that I made....
	for (tTindex=0; tTindex <=numTokens; tTindex++) {
			// WHY? Get MISRA slap for side-effect ON RHS of &&
		if (Normalise != (tTindex && !myStrCmp(tT[tTindex], tT[tTindex -1])))  { 
			if (*setA) setA=strcat(setA, " ");  // Only pre-pend space on non-initial members
			setA=strcat(setA, tT[tTindex]); //zzz
		}
	}
	if (setCardinal(setA) > setImp.setMaxMembers) return setBadSet;  // Set TOO many members
 	if (isBadLength(setA))                        return setBadSet;  // Set TOO large
	return setOK;
};


setStatus        setNormalise(set setA) {
	setStatus s;
	sl card;

	s=listNormaliseOrDuplicates(setA, 1);
	if (s) return s;
	card=setCardinal(setA);
	if (card > setImp.setMaxMembers) return setBadSet;
	return s;
}

static setStatus        listDuplicates(cStr cStrA) {
	return listNormaliseOrDuplicates(cStrA, 0);
}

setStatus  setIsNotEqual(set setA, set setB) {
	// normalise both and then single string compare
	// SIDE-EFFECTS !! - The supplied sets get normalised.  No harm there though iff TRUE sets.
	// TODO: Be defensive - check ret codes of setNormalise...
	setStatus nA, nB;
	nA=setNormalise(setA);
	nB=setNormalise(setB);
	if (nA) return nA;
	if (nB) return nB;
	return (myStrCmp(setA, setB) ? setNotEqual : setOK);
}
 
static cStr listJoin(setConst setA, setConst setB) {
	// concatenate the two 
	cStr j;  

	if (!setA || !setB)    return (cStr) NULL;
	//if (!*setA)            return (cStr) setB;  // FIXME FIXME Problem Here listJoin should be CREATING a new cStr
	//if (!*setB)            return (cStr) setA;  // FIXME       Need to Malloc a copy......
	if (isBadLength(setA)) return (cStr) NULL;  // Set TOO large
	if (isBadLength(setB)) return (cStr) NULL;  // Set TOO large

	j=malloc(1 + setImp.setMaxChars *2 * sizeof(char));
	// FIXME: Check return of malloc here .....
	if (j==NULL) return (cStr) NULL;
	*j=(unsigned char) 0;
	(void) strcat(j, setA);
	(void) strcat(j, " ");
	(void) strcat(j, setB);
	realloc(j, 2 + strlen(j));
	if (isBadIntermediaryLength(j)) return (cStr) NULL;  // Set TOO large
	return j;
}

set            setUnion(setConst setA, setConst setB) {
	setStatus s;

	set j=listJoin(setA, setB);
	s=setNormalise(j); // OK if j is NULL, setNormalise will handle it.
	// FIXME: Check size of j in chars here.....
	// FIXME: realloc appropriately
	if (s) return (set) NULL;

	return j;
}

set     setIntersection(setConst setA, setConst setB) {
	setStatus s;
	// FIXME: testing return status here....
	cStr j=listJoin(setA, setB);
	s=listDuplicates(j);
	if (s) return (set) NULL;
	s=setNormalise(j);
	if (s) return (set) NULL;
	// FIXME: realloc appropriately
	return  (set) j;
}


setStatus  setDelete(set setA) {          // Delete this set - iff created by union or intersection
	free(setA);
	return setOK;
}


#ifdef CUTEST

#include <CuTest.h>
#include <CuTest.c>
                        
                                        
                                        
//    X      XXX             XXX     XXX  
//  XXX     X   X           X   X   X   X 
//    X     X   X               X   X   X 
//    X     X   X               X   X   X 
//    X     X   X             XX    X   X 
//    X     X   X               X   X   X 
//    X     X   X               X   X   X 
//    X     X   X           X   X   X   X 
//  XXXXX    XXX      X      XXX     XXX  
//                   X                    
                                        
void testSetNormalise(CuTest *tc) {
	char* input;
	char* expected;
	setStatus ss;

	// Note: Boundary Case - Input 2 * Max number of chars; Output Max number of chars
	// Note: Boundary Case - Max number of members

	// Boundary Case - 29 chars - Should be OK
	input = strdup("a i a i a i a e a i a a i a i");   expected = "a e i"; 
	ss = setNormalise(input); CuAssertStrEquals(tc, expected, input); CuAssertIntEquals(tc, setOK, ss);

	// Boundary Case - 29 chars - Should be OK
	input = strdup("1234567890 234567890 a3456789");   expected = "1234567890 234567890 a3456789";  // 
	ss = setNormalise(input); CuAssertStrEquals(tc, expected, input); CuAssertIntEquals(tc, setOK, ss);

	// Boundary Case - 30 chars - ON THE LIMIT - Should return setBadSet as resulting set is OVER/ON LIMIT
	input = strdup("1234567890 234567890 a34567890");   expected = "1234567890 234567890 a34567890";  // 
	ss = setNormalise(input); CuAssertStrEquals(tc, expected, input); CuAssertIntEquals(tc, setBadSet, ss);

	// Boundary Case - 31 chars - OVER THE LIMIT - Should return setBadSet
	input = strdup("1234567890 234567890 a345678901");   expected = "1234567890 234567890 a345678901";  // 
	ss = setNormalise(input); CuAssertStrEquals(tc, expected, input); CuAssertIntEquals(tc, setBadSet, ss);

	// Boundary Case - 32 chars - Input is NOT OVER THE 2*MAX LIMIT and RESULT NOT over the LIMIT - Should return setOK
	input = strdup("1234567890 234567890 z z z z z z");   expected = "1234567890 234567890 z";  // 
	ss = setNormalise(input); CuAssertStrEquals(tc, expected, input); CuAssertIntEquals(tc, setOK, ss);

	// Boundary Case - 9 members - Should be OK
	input = strdup("j i h g e d c b a");   expected = "a b c d e g h i j"; 
	ss = setNormalise(input); CuAssertStrEquals(tc, expected, input); CuAssertIntEquals(tc, setOK, ss);

	// Boundary Case - 10 RESULTING members - Should return setBadSet
	input = strdup("j i h g e d c b a z");   expected = "a b c d e g h i j z"; 
	ss = setNormalise(input); CuAssertStrEquals(tc, expected, input); CuAssertIntEquals(tc, setBadSet, ss);

	// Boundary Case - 19 Supplied members - Should return setOK
	input = strdup("a 1 a 1 a 1 a 1 a 1 a 1 a 1 a 1 a 1 a");   expected = "1 a"; 
	ss = setNormalise(input); CuAssertStrEquals(tc, expected, input); CuAssertIntEquals(tc, setOK, ss);

	// Boundary Case - 20 Supplied members - Should return setOK
	input = strdup("a 1 a 1 a 1 a 1 a 1 a 1 a 1 a 1 a 1 a 1");   expected = "1 a"; 
	ss = setNormalise(input); CuAssertStrEquals(tc, expected, input); CuAssertIntEquals(tc, setOK, ss);

	// Boundary Case - 21 Supplied members - Should return setBadSet
	input = strdup("a 1 a 1 a 1 a 1 a 1 a 1 a 1 a 1 a 1 a 1 a");   expected = "a 1 a 1 a 1 a 1 a 1 a 1 a 1 a 1 a 1 a 1 a"; 
	ss = setNormalise(input); CuAssertStrEquals(tc, expected, input); CuAssertIntEquals(tc, setBadSet, ss);

	input = strdup("a i a i h g f e d c b a i a i");   expected = "a b c d e f g h i"; ss = setNormalise(input);
	CuAssertStrEquals(tc, expected, input); CuAssertIntEquals(tc, setOK, ss);

	// General Case - duplicates and ordering
	input = strdup("i h g f e d c b a");               expected ="a b c d e f g h i";ss = setNormalise(input);
	CuAssertStrEquals(tc, expected, input); CuAssertIntEquals(tc, setOK, ss);

	// General Case - duplicates and ordering
	input = strdup("c c c c c c d a");                 expected = "a c d";           ss = setNormalise(input);
	CuAssertStrEquals(tc, expected, input); CuAssertIntEquals(tc, setOK, ss);

	// General Case - duplicates and ordering
	input = strdup("beta alpha beta gamma alpha");     expected = "alpha beta gamma"; ss = setNormalise(input);
	CuAssertStrEquals(tc, expected, input); CuAssertIntEquals(tc, setOK, ss);

	// Boundary Case - empty set supplied
	input = strdup("");                                expected = "";                 ss = setNormalise(input);
	CuAssertStrEquals(tc, expected, input); CuAssertIntEquals(tc, setOK, ss);

	// Exceptional Case - NULL supplied
	input = NULL;                                      expected = NULL;               ss = setNormalise(input);
	CuAssertStrEquals(tc, expected, input); CuAssertIntEquals(tc, setBadSet, ss);
} //testSetNormalise


void testSetUnion(CuTest *tc) {
	char* input1;
	char* input2;
	char* expected;
	set setU;

	input1 = strdup("");  input2=strdup("a c b");  expected = "a b c"; 
	setU = setUnion(input1, input2); CuAssertStrEquals(tc, expected, setU); 

	input1 = strdup("a b c");  input2=strdup("");  expected = "a b c"; 
	setU = setUnion(input1, input2); CuAssertStrEquals(tc, expected, setU); 

	input1 = strdup("a b c");  input2=NULL;  expected = NULL; 
	setU = setUnion(input1, input2); CuAssertStrEquals(tc, expected, setU); 

	input1 = NULL;  input2=strdup("c b a");  expected = NULL; 
	setU = setUnion(input1, input2); CuAssertStrEquals(tc, expected, setU); 

	// Boundary case - 9 members in result - should be OK
	input1 = strdup("i h g f p a z");  input2=strdup("i h g f e d");  expected = "a d e f g h i p z"; 
	setU = setUnion(input1, input2); CuAssertStrEquals(tc, expected, setU); 

	// Boundary case - 10 members in result which is ON THE LIMIT, and therefore should return NULL
	input1 = strdup("i h g f p a z x");  input2=strdup("i h g f e d");  expected = NULL;
	setU = setUnion(input1, input2); CuAssertStrEquals(tc, expected, setU); 

	// Boundary case - 12 members in result which is OVER THE LIMIT, and therefore should return NULL
	input1 = strdup("i h g f p a z x");  input2=strdup("i h g f e d w y");  expected = NULL; //"a d e f g h i p w x y z"; 
	setU = setUnion(input1, input2); CuAssertStrEquals(tc, expected, setU); 

	// Boundary case - 30 chars in RESULT which should be OK
	input1 = strdup("a234567890 c34");  input2=strdup("b234567890 d34");  expected = "a234567890 b234567890 c34 d34";
	setU = setUnion(input1, input2); CuAssertStrEquals(tc, expected, setU); 

	// Boundary case - 31 chars in RESULT which should return NULL
	input1 = strdup("a234567890 c34");  input2=strdup("b234567890 d345");  expected = NULL;
	setU = setUnion(input1, input2); CuAssertStrEquals(tc, expected, setU); 

	// Boundary case - 58 chars in INTERMEDIARY which should be OK
	input1 = strdup("1234567890 234567890 a3456789");  input2=strdup("1234567890 234567890 a3456789");  expected = "1234567890 234567890 a3456789";
	setU = setUnion(input1, input2); CuAssertStrEquals(tc, expected, setU); 

	// Boundary case - 60 chars in INTERMEDIARY which  THE LIMIT, Should return NULL
	input1 = strdup("1234567890 234567890 a34567890");  input2=strdup("1234567890 234567890 a34567890");  expected = NULL;
	setU = setUnion(input1, input2); CuAssertStrEquals(tc, expected, setU); 

} //testSetUnion

CuSuite* SetUtilGetSuite() {
	CuSuite* suite = CuSuiteNew();

	setImp.setMaxMembers=10;  // These are the implementation values for the tests
	setImp.setMaxChars=30;

	SUITE_ADD_TEST(suite, testSetUnion);
	SUITE_ADD_TEST(suite, testSetNormalise);

	return suite;
}

#endif






#ifdef UNIT_TEST

static cStr mk0Null(cStr s) {
	if (!s)               return s;  // strcmp Can't cope with NULLs
	if (! strcmp("0", s)) return NULL; else return s;
}

static int setTestDriver(int argc, char **argv) {
	const unsigned char me[]="setTestDriver"; 
	 /* Lets keep complaints about not using argc/v quiet... */
	if (!argc) { 
		fprintf(stderr, "%s: No Args: .\n", me);
	} else {
		int i;
		unsigned char *mem;
		unsigned char *otherSet;
		unsigned char *uSet;
		unsigned char newSet[setImp.setMaxChars];
		unsigned char newSet_B[setImp.setMaxChars];
		myBool isMem;
		while (--argc > 0 && (*++argv)[0] == '-')
			switch ((*argv)[1]) {
			case 's':    
				--argc; argv++; i=setShow(mk0Null(*argv)); fprintf(stderr, "\n");
				fprintf(stderr, "%s: Show:          :%-15s: Result:           %3d\n", "set", *argv, i);
				return i;
			case 'c':
				--argc; argv++; i=setCardinal(mk0Null(*argv));
				fprintf(stderr, "%s: Cardinal:      :%-15s:                : Result:%3d \n", "set", *argv, i);
				fprintf(stdout, "%d\n", i);
				return i;
			case 'n':
				--argc; argv++; 
				// !! setNormalise writes to what's pointed at by its params. 
				//    That MUST be writable.  I don't think **argv is  is....
				if (*argv) strcpy(newSet, *argv); else newSet[0]=0;   // strcpy cant cope with NULLs.....
				i=setNormalise(mk0Null(newSet));
				fprintf(stderr, "%s: Normalise:     :%-15s:%-15s : Status:%3d (%s)\n", "set", *argv, newSet, i, setStatus2Str(i) );
				fprintf(stdout, "%s\n", newSet);
				return i;
			case 'm':
				--argc; argv++; mem=mk0Null(*argv);
				--argc; argv++; isMem=setIsNotMember(mem, mk0Null(*argv));
				fprintf(stderr, "%s: isNotMember:   :%-15s:%-15s : Result:%3d (%s)\n", "set", mem, *argv, isMem, setStatus2Str(isMem));
				fprintf(stdout, "%d\n", isMem);
				return isMem;
			case 'e': // Note - setIsNotEqual calls normalise, so *argv won't do......
				--argc; argv++; 
				otherSet=mk0Null(*argv);
				if (*argv) strcpy(newSet, otherSet); else newSet[0]=0;   // strcpy cant cope with NULLs.....
				--argc; argv++; 
				if (mk0Null(*argv)) strcpy(newSet_B, mk0Null(*argv)); else newSet_B[0]=0;   // strcpy cant cope with NULLs.....
				isMem=setIsNotEqual(newSet, newSet_B);
				fprintf(stderr, "%s: setIsNotEqual: :%-15s, %-15s: Result:%3d (%s)\n", "set", otherSet, *argv, isMem, setStatus2Str(isMem));
				fprintf(stdout, "%d\n", isMem);
				return isMem;
			case 'u':
				--argc; argv++; otherSet=mk0Null(*argv);
				--argc; argv++; uSet=setUnion(otherSet, mk0Null(*argv));
				fprintf(stderr, "%s: setUnion:      :%-15s, %-15s: Result: %-15s\n", "set", otherSet, *argv, uSet);
				fprintf(stdout, "%s\n", uSet);
				//fprintf(stderr, "%s: : setUnion: Doing The CleanUp\n", me);
				setDelete(uSet);
				//fprintf(stderr, "%s: : setUnion: CleanUp Done\n", me);
				// Like to return a decent value here
				break;
			case 'i':
				--argc; argv++; otherSet=mk0Null(*argv);
				--argc; argv++; uSet=setIntersection(otherSet, mk0Null(*argv));
				fprintf(stderr, "%s: setIntersection:%-15s, %-15s: Result: %-15s\n", "set", otherSet, mk0Null(*argv), uSet);
				fprintf(stdout, "%s\n", uSet);
				//fprintf(stderr, "%s: : setIntersection: Doing The CleanUp\n", me);
				setDelete(uSet);
				//fprintf(stderr, "%s: : setIntersection: CleanUp Done\n", me);
				// Like to return a decent value here
				break;
			default:
				fprintf(stderr, "%s: : ERROR UNKNOWN: %c:\n", "set", (*argv)[1] );
			}
	}
	//fprintf(stderr, "%s: Complete. No Status Value returned (only 0)\n", me);
	return 0; // What we return if we can't return anythingelse
}


int main(int argc, char **argv) {
		// I use these for testing cases
		setImp.setMaxMembers= 8; //SET_MAX_MEMBERS_VALUE,
		setImp.setMaxChars  =25; //SET_MAX_CHARS_VALUE,

	if (argc >1) { // If command-line contains test cases, use that
		// I use these for command-line usage - megga values....
		setImp.setMaxMembers= 18000; //SET_MAX_MEMBERS_VALUE,
		setImp.setMaxChars  =250000; //SET_MAX_CHARS_VALUE,

		fprintf(stderr, "\nSet: Implementation Bounds: setMaxMembers:%d, setMaxChars:%d\n\n", setImp.setMaxMembers, setImp.setMaxChars);
		return setTestDriver(argc, argv);
	} else {    // Otherwise, exercise a basic set coded here.
                                                                                
 /*                                                                               
 XXXXXXX                                            X                           
  X    X                                   X                                    
  X                                        X                                    
  X  X   XXX XXX  XXXXX   XXXXX  XXXXXX   XXXX    XXX     XXXXX  XX XX    
  XXXX    X   X  X     X X     X  X    X   X        X    X     X  XX  X  
  X  X     XXX   X       XXXXXXX  X    X   X        X    X     X  X   X 
  X        XXX   X       X        X    X   X        X    X     X  X   X
  X    X  X   X  X     X X     X  X    X   X  X     X    X     X  X   X 
 XXXXXXX XXX XXX  XXXXX   XXXXX   XXXXX     XX    XXXXX   XXXXX  XXX XXX 
                                  X                                             
                                 XXX                                            
                                        
                                        
   XXXX                                 
  X    X                                
 X                                      
 X        XXXX    XXXXX   XXXXX   XXXXX 
 X            X  X     X X     X X     X
 X        XXXXX   XXX    XXXXXXX  XXX   
 X       X    X      XX  X           XX 
  X    X X    X  X     X X     X X     X
   XXXX   XXXX X  XXXXX   XXXXX   XXXXX 
                                        
*/                                      
		char  *av1[]  ={"progName", "-c", "a b c ", NULL};
		char  *av1_1[]={"progName", "-c", "abbey",  NULL};
		char  *av1_2[]={"progName", "-c", "",       NULL};
		char  *av1_3[]={"progName", "-c", NULL,     NULL};

		char  *av2[]  ={"progName", "-n", "a b d b c",       NULL};
		char  *av2_1[]={"progName", "-n", "ddd d a b b c b", NULL};
		char  *av2_2[]={"progName", "-n", "",                NULL};
		char  *av2_3[]={"progName", "-n", NULL,              NULL};

		char  *av3[]  ={"progName", "-m", "fred",     "alf frederick",NULL};
		char  *av3_1[]={"progName", "-m", "fred",     "fred alf john",NULL};
		char  *av3_10[]={"progName", "-m", "alf",      "fred alf john",NULL};
		char  *av3_11[]={"progName", "-m", "john",     "fred alf john",NULL};
		char  *av3_2[]={"progName", "-m", "fred joe", "fred joe",     NULL};
		char  *av3_3[]={"progName", "-m", "fred joe", "fred",         NULL};
		char  *av3_4[]={"progName", "-m", "fred",     "",             NULL};
		char  *av3_5[]={"progName", "-m", "",         "alf fred",     NULL};
		char  *av3_6[]={"progName", "-m", "",         "",             NULL};
		char  *av3_7[]={"progName", "-m", "",         NULL,           NULL};
		char  *av3_8[]={"progName", "-m", NULL,       "alf fred",     NULL};
		char  *av3_9[]={"progName", "-m", NULL,       NULL,           NULL};

		char  *av4[]  ={"progName", "-i", "a b c ", "c b e",          NULL};
		char  *av4_1[]={"progName", "-i", "a b c ", "k l m ",         NULL};
		char  *av4_2[]={"progName", "-i", "a b c ", "k l a",          NULL};
		char  *av4_3[]={"progName", "-i", "a b c ", "",               NULL};
		char  *av4_4[]={"progName", "-i", "",       "c b e",          NULL};
		char  *av4_5[]={"progName", "-i", "a b c ", NULL,             NULL};
		char  *av4_6[]={"progName", "-i", NULL,     "c b e",          NULL};
		char  *av4_7[]={"progName", "-i", "",       NULL,             NULL};
		char  *av4_8[]={"progName", "-i", NULL,      NULL,            NULL};

		char  *av5[]  ={"progName", "-u", "a b c ", "c b e",          NULL};
		char  *av5_1[]={"progName", "-u", "a b c ", "k l m ",         NULL};
		char  *av5_2[]={"progName", "-u", "a b c ", "k l a",          NULL};
		char  *av5_3[]={"progName", "-u", "a b c ", "",               NULL};
		char  *av5_4[]={"progName", "-u", "",       "c b e",          NULL};
		char  *av5_5[]={"progName", "-u", "a b c ", NULL,             NULL};
		char  *av5_6[]={"progName", "-u", NULL,     "c b e",          NULL};
		char  *av5_7[]={"progName", "-u", "",       NULL,             NULL};
		char  *av5_8[]={"progName", "-u", NULL,      NULL,            NULL};

		char  *av6[]  ={"progName", "-e", "a b c ", "a b c",          NULL};
		char  *av6_1[]={"progName", "-e", "a b c ", "c b   a",        NULL};
		char  *av6_2[]={"progName", "-e", "a b c ", "k l a",          NULL};
		char  *av6_3[]={"progName", "-e", "a b c ", "",               NULL};
		char  *av6_4[]={"progName", "-e", "",       "",               NULL};
		char  *av6_5[]={"progName", "-e", "a b c ", NULL,             NULL};
		char  *av6_6[]={"progName", "-e", NULL,     "c b e",          NULL};
		char  *av6_7[]={"progName", "-e", "",       NULL,             NULL};
		char  *av6_8[]={"progName", "-e", NULL,      NULL,            NULL};


		fprintf(stderr, "Test 1    "); (void) setTestDriver(3, av1);
		fprintf(stderr, "Test 1_1  "); (void) setTestDriver(3, av1_1);
		fprintf(stderr, "Test 1_2  "); (void) setTestDriver(3, av1_2);
		fprintf(stderr, "Test 1_3  "); (void) setTestDriver(3, av1_3);

		fprintf(stderr, "Test 2    "); (void) setTestDriver(3, av2);
		fprintf(stderr, "Test 2_1  "); (void) setTestDriver(3, av2_1);
		fprintf(stderr, "Test 2_2  "); (void) setTestDriver(3, av2_2);
		fprintf(stderr, "Test 2_3  "); (void) setTestDriver(3, av2_3);

		fprintf(stderr, "Test 3    "); (void) setTestDriver(4, av3);
		fprintf(stderr, "Test 3_1  "); (void) setTestDriver(4, av3_1);
		fprintf(stderr, "Test 3_2  "); (void) setTestDriver(4, av3_2);
		fprintf(stderr, "Test 3_3  "); (void) setTestDriver(4, av3_3);
		fprintf(stderr, "Test 3_4  "); (void) setTestDriver(4, av3_4);
		fprintf(stderr, "Test 3_5  "); (void) setTestDriver(4, av3_5);
		fprintf(stderr, "Test 3_6  "); (void) setTestDriver(4, av3_6);
		fprintf(stderr, "Test 3_7  "); (void) setTestDriver(4, av3_7);
		fprintf(stderr, "Test 3_8  "); (void) setTestDriver(4, av3_8);
		fprintf(stderr, "Test 3_9  "); (void) setTestDriver(4, av3_9);
		fprintf(stderr, "Test 3_10 "); (void) setTestDriver(4, av3_10);
		fprintf(stderr, "Test 3_11 "); (void) setTestDriver(4, av3_11);

		fprintf(stderr, "Test 4    "); (void) setTestDriver(4, av4);
		fprintf(stderr, "Test 4_1  "); (void) setTestDriver(4, av4_1);
		fprintf(stderr, "Test 4_2  "); (void) setTestDriver(4, av4_2);
		fprintf(stderr, "Test 4_3  "); (void) setTestDriver(4, av4_3);
		fprintf(stderr, "Test 4_4  "); (void) setTestDriver(4, av4_4);
		fprintf(stderr, "Test 4_5  "); (void) setTestDriver(4, av4_5);
		fprintf(stderr, "Test 4_6  "); (void) setTestDriver(4, av4_6);
		fprintf(stderr, "Test 4_7  "); (void) setTestDriver(4, av4_7);
		fprintf(stderr, "Test 4_8  "); (void) setTestDriver(4, av4_8);

		fprintf(stderr, "Test 5    "); (void) setTestDriver(4, av5);
		fprintf(stderr, "Test 5_1  "); (void) setTestDriver(4, av5_1);
		fprintf(stderr, "Test 5_2  "); (void) setTestDriver(4, av5_2);
		fprintf(stderr, "Test 5_3  "); (void) setTestDriver(4, av5_3);
		fprintf(stderr, "Test 5_4  "); (void) setTestDriver(4, av5_4);
		fprintf(stderr, "Test 5_5  "); (void) setTestDriver(4, av5_5);
		fprintf(stderr, "Test 5_6  "); (void) setTestDriver(4, av5_6);
		fprintf(stderr, "Test 5_7  "); (void) setTestDriver(4, av5_7);
		fprintf(stderr, "Test 5_8  "); (void) setTestDriver(4, av5_8);

		fprintf(stderr, "Test 6    "); (void) setTestDriver(4, av6);
		fprintf(stderr, "Test 6_1  "); (void) setTestDriver(4, av6_1);
		fprintf(stderr, "Test 6_2  "); (void) setTestDriver(4, av6_2);
		fprintf(stderr, "Test 6_3  "); (void) setTestDriver(4, av6_3);
		fprintf(stderr, "Test 6_4  "); (void) setTestDriver(4, av6_4);
		fprintf(stderr, "Test 6_5  "); (void) setTestDriver(4, av6_5);
		fprintf(stderr, "Test 6_6  "); (void) setTestDriver(4, av6_6);
		fprintf(stderr, "Test 6_7  "); (void) setTestDriver(4, av6_7);
		fprintf(stderr, "Test 6_8  "); (void) setTestDriver(4, av6_8);
/*                                                                
                                                                
 XXXXXX                              XX                         
  X    X                              X                         
  X    X                              X                         
  X    X  XXXXX  XX  XX  XX XX    XXXXX   XXXX   XXX XX  XXX XXX
  XXXXX  X     X  X   X   XX  X  X    X       X    XX  X  X   X 
  X    X X     X  X   X   X   X  X    X   XXXXX    X      X   X 
  X    X X     X  X   X   X   X  X    X  X    X    X       X X  
  X    X X     X  X  XX   X   X  X    X  X    X    X       X X  
 XXXXXX   XXXXX    XX XX XXX XXX  XXXXXX  XXXX X XXXXX      X   
                                                            X   
                                                          XX    
                                        
                                        
   XXXX                                 
  X    X                                
 X                                      
 X        XXXX    XXXXX   XXXXX   XXXXX 
 X            X  X     X X     X X     X
 X        XXXXX   XXX    XXXXXXX  XXX   
 X       X    X      XX  X           XX 
  X    X X    X  X     X X     X X     X
   XXXX   XXXX X  XXXXX   XXXXX   XXXXX 
                                        
*/                                        
// How to do the boundary test cases?
// Largely this will be 
//  o The upper bound on the cardinality of the set
//  o The upper bound on the size of the C string.
//  o 
//  o 
//  o 
//  o 
//  o 
//		setImp.setMaxMembers= 3; //SET_MAX_MEMBERS_VALUE,
//		setImp.setMaxChars  =10; //SET_MAX_CHARS_VALUE,

	fprintf(stderr, "\nBoundary Testing with setMaxMembers:%d, setMaxChars:%d\n\n", setImp.setMaxMembers, setImp.setMaxChars);

		char  *av1_10[]={"progName", "-n", "a23456789012345678901234",  NULL};
		char  *av1_11[]={"progName", "-n", "a234567890123456789012345", NULL};
		char  *av1_12[]={"progName", "-n", "a 2 3 4 5 6 7", NULL};
		char  *av1_13[]={"progName", "-n", "a 2 3 4 5 6 7 8 9 0 1 2 3", NULL};
		char  *av1_14[]={"progName", "-c", "    g f e d c b a", NULL};
		char  *av1_15[]={"progName", "-c", "  h g f e d c b a", NULL};
		char  *av1_16[]={"progName", "-c", "i h g f e d c b a", NULL};
		char  *av1_17[]={"progName", "-i", "i h g f e d", "i h g f e d", NULL};
		char  *av1_18[]={"progName", "-u", "i h g f p a", "i h g f e d", NULL};
		char  *av1_19[]={"progName", "-u", "i h s f p a", "i h g f e d", NULL};
		char  *av1_20[]={"progName", "-i", "iii hhh ggg fff eee ", "fff eee ddd ccc bbb aaa", NULL};
		char  *av1_21[]={"progName", "-u", "iii hhh ggg fff eee ", "fff eee ddd ccc bbb aaa", NULL};
		fprintf(stderr, "Test 1_10  "); (void) setTestDriver(3, av1_10);
		fprintf(stderr, "Test 1_11  "); (void) setTestDriver(3, av1_11);
		fprintf(stderr, "Test 1_12  "); (void) setTestDriver(3, av1_12);
		fprintf(stderr, "Test 1_13  "); (void) setTestDriver(3, av1_13);
		fprintf(stderr, "Test 1_14  "); (void) setTestDriver(3, av1_14);
		fprintf(stderr, "Test 1_15  "); (void) setTestDriver(3, av1_15);
		fprintf(stderr, "Test 1_16  "); (void) setTestDriver(3, av1_16);
		fprintf(stderr, "Test 1_17  "); (void) setTestDriver(3, av1_17);
		fprintf(stderr, "Test 1_18  "); (void) setTestDriver(3, av1_18);
		fprintf(stderr, "Test 1_19  "); (void) setTestDriver(3, av1_19);
		fprintf(stderr, "Test 1_20  "); (void) setTestDriver(3, av1_20);
		fprintf(stderr, "Test 1_21  "); (void) setTestDriver(3, av1_21);



	}
	return 0;
}
#endif
