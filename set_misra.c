/* ************************************************************ 
 * File:               set_misra.c
 * 
 * Description:    Some set stuff - a quick test to exercise unit testing under RTRT
 * o A set is like set algebra, a member is a lexical token or name (contains no whitespace (WS))
 * o A set is implemented as a single C string, with members delimetered by WS
 *   I'd  like to hide this implementation.
 * o A set can be represented in "normalised form" - delimitted by single space, no duplicates
 * o Implementation details are represented in a single struct, 'setImp'.
 * o 
 * o A member *can* have an int value. Otherwise int value is zero.
 *
 *
 *  T o     D o 
 * o Introduce some cutest test cases.
 * o Create alternative versions of this file, which
 *   o 0 - Is FULLY MISRA compliant
 *   o 1 - Is BE_MAX compliant
 *   o 2 - Is BE_mid compliant
 *   o 3 - Is BE_min compliant
 * o
 * o
 * o
 *
 *
 *
  ************************************************************** */

#include <set_misra.h> /* MISRA - Non-std filename */
#include <stdio.h>     /* MISRA - Not Misra compliant include file */
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
typedef UC8 *cStr;             /* MISRA - Have to turn OFF MISRA rule re. absense of Signed specifier - cStr are PLAIN */
typedef const UC8 *cStrConst;  /* MISRA - Have to turn OFF MISRA rule re. absense of Signed specifier - cStr are PLAIN */
static const cStr setModName="setModule";

// MISRA-Friendly Local Function Declarations
static myBool      isWhiteSpace(UC8 c) ;                 // return !=0 if c is whitespace
static void        swapCharPtr(UC8 **a, UC8 **b) ;      // Swaps the two char pointers pointed at by params 1 & 2
static setStatus   isBadLength(setConst setA) ;           // returns !=0 if setA has a bad length ( > Max allowed )
static setStatus   listDuplicates(cStr cStrA) ;            // ReWrites setA so that it contains *only* the duplicates
static cStr        listJoin(setConst setA, setConst setB);// Returns new C string that is the concatenation of params
static setStatus   listNormaliseOrDuplicates(set setA, myBool Normalise) ; // Either normalises or listsDuplicates

static void swapCharPtr(UC8 **a, UC8 **b) {
	UC8 *t;         t=*a;  *a=*b;  *b=t;
}

static myBool isWhiteSpace(UC8 c) { // FIXME:  Consider INLINE - or - pre-processor macro fn() instead for performance.....
	return (myBool) ((c == ' ') || (c == '\t') || (c == '\n'));
}

static setStatus isBadLength(setConst setA) {
	if (!setA) { return setBadSet; } 
	if (strlen(setA) >= (size_t)setImp.setMaxChars) { return setBadSet; } 
	return setOK;
}

static SI16 myStrCmp(const UC8 * s1, const UC8 *s2) { // Safer Ver of string.h's strcmp

	if (!s1) {return -1; } 
	if (!s2) {return +1; } 
	return strcmp(s1, s2);
}

/* Prototypes of Module Private functions for MISRA C */

const UC8 *setStatus2Str(setStatus stat) {
	static const UC8 *Mssg[]={
		"setOK", "setEmptySet", "setBadSet", "setBadCardinal", "setBadMember", "setMemberNotFound", "setNotEqual"
	};

	static const UC8 *MssgUnknown="Unknown Status";

	if ((setOK <= stat) && (stat <= setNotEqual)) {
		return Mssg[stat];
	} else { /* MISRA - QAC6 reckons this is unreachable - but then again it */
	         /*         trusts the dynamic range of params typed to enums, when truly they're int's */
			 /*         I prefer a more DEFENSIVE approach */
		return MssgUnknown; 
    }
}


setStatus setShow(setConst setA) {
		   if (!setA) {  
		//printf("%s: setShow: Error: NIL Supplied to setShow\n", setModName);
		return(setBadSet);  
		// FIXME: Check for oversize sets etc.
	} else if (!*setA) {
		//printf("%s: setShow: Info: Empty Set supplied to setShow\n", setModName);
		return(setOK); // Empty set is a valid set - no error here thanks.
	} else { /* Assume we have a set here */
		/* Stops Further Testing... assert(('a' <= *setA) && (*setA <= 'Z'));  */
		(void) printf("%s", setA);
	}
	return setOK;
}

sl setCardinal(setConst setA) {
	if (!setA) {   
		//printf("%s: setCardinal: Error: NIL Supplied to setCardinal\n", setModName);
		return((sl) -1);  /* Cant give a sensible cardinal of a bad set */
	} else if (!*setA) {
		//printf("%s: setCardinal: Info: Empty Set supplied to setCardinal\n", setModName);
		return(0);
	} else { /* Processing A Set: Assume we have a set here */
		/* Stops Further Testing... assert(('a' <= *setA) && (*setA <= 'Z'));  */
		SL64 numMembers=0; /* FIXME: This should be ZERO!! */
		/* Scan the string, counting occurances of WS -> Non-WS */
		/* WHY NOT USE strings.h::strtok here?? */
		while (*setA) {               /* Danger Str-OverRun */
			if (!isWhiteSpace(*setA)) {
				/* At Token or NUL */
				if (!*setA) { return(numMembers); }
				/* At Token */
				numMembers++;   /* FIXME - Danger int-OverRun  Fit It.*/
				if (numMembers >=setImp.setMaxMembers) {
					/* Some BETTER exceptional handling here would be nice */
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
	UC8 *test;
	size_t	lenMem;
	UC8 trailChar;

	if (!setA)                return setBadSet;
	if (!aMem)                return setBadMember;
	if (!*setA)               return setEmptySet;
	if (!*aMem)               return setBadMember;

	lenMem=strlen(aMem);    
	if (lenMem >= setImp.setMaxChars)  return setBadMember;
	if (setCardinal((setConst) aMem) !=1)   return setBadMember;
	test=strstr(setA, aMem);  

	if ( (test!=NULL) ) {  /* Possible Match - Problem with wd boundaries .... */
		/* Check For Trailing non-WS */
		trailChar=*(test + lenMem);  
		if ((!trailChar) || (isWhiteSpace(trailChar))) {  /* No Trailing Problems */
			if ((test==(const UC8*)setA) || isWhiteSpace(*(test - 1))) { /* No Leading Problems */
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

	SL64 MAX_MEMBERS_X_2=2 * setImp.setMaxMembers;  
// #define	MAX_MEMBERS_X_2  (2 * setImp.setMaxMembers)

	UC8 *tT[MAX_MEMBERS_X_2]; // token Table of ptrs to start of each token
	int tTindex=-1;                  // Index into tT
	int numTokens;                   // Number of tokens discovered

	UC8 sCopy[setImp.setMaxChars]; // Copy of supplied set. Use of strtok damages the supplied string.
	UC8 *aMember;                   // a token       

	int pass;                        // Multiple passes within the sort function...

	// Deal with some exceptional conditions
	if (!setA)             return setBadSet;  // No set supplied
	if (!*setA)            return setOK;      // Empty set supplied
	if (isBadLength(setA)) return setBadSet;  // Set TOO large
	if (setCardinal(setA) > MAX_MEMBERS_X_2) return setBadSet;  // Set TOO many members

	// Generate tT - Create this table of ptrs 
	(void) strncpy(sCopy, setA, setImp.setMaxChars);        
	do {
		aMember=strtok((tTindex == -1) ? sCopy : NULL, setImp.setWhiteSpaceChars);
		if (aMember) {
			if (tTindex == MAX_MEMBERS_X_2) 
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

	*setA=(UC8)0; // Bang !! - Sometimes you cannot write to here - unless its a TRUE set that I made....
	for (tTindex=0; tTindex <=numTokens; tTindex++) {
			// WHY? Get MISRA slap for side-effect ON RHS of &&
		if (Normalise != (tTindex && !myStrCmp(tT[tTindex], tT[tTindex -1])))  { 
			if (*setA) setA=strcat(setA, " ");  // Only pre-pend space on non-initial members
			setA=strcat(setA, tT[tTindex]); //zzz
		}
	}
	if (setCardinal(setA) > setImp.setMaxMembers) return setBadSet;  // Set TOO many members
	return setOK;
}

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

	j=malloc(1 + setImp.setMaxChars *2 * sizeof(UC8));
	// FIXME: Check return of malloc here .....
	if (j==NULL) return (cStr) NULL;
	*j=(UC8) 0;
	(void) strcat(j, setA);
	(void) strcat(j, " ");
	(void) strcat(j, setB);
	realloc(j, 2 + strlen(j)); /* FIXME - test results of the re-alloc */
	if (isBadLength(j)) return (cStr) NULL;  // Set TOO large
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

	// TODO: Boundary Case - Max number of chars
	// TODO: Boundary Case - Max number of members

	// Boundary Case - 29 chars - Should be OK
	input = strdup("a i a i a i a e a i a a i a i");   expected = "a e i"; 
	ss = setNormalise(input); CuAssertStrEquals(tc, expected, input); CuAssertIntEquals(tc, setOK, ss);

	// Boundary Case - 30 chars - ON THE LIMIT - Should return setBadSet
	input = strdup("a i a i a i a e a i a a i a ii");   expected = "a i a i a i a e a i a a i a ii";  // "a e i ii"; 
	ss = setNormalise(input); CuAssertStrEquals(tc, expected, input); CuAssertIntEquals(tc, setBadSet, ss);

	// Boundary Case - 9 members - Should be OK
	input = strdup("j i h g e d c b a");   expected = "a b c d e g h i j"; 
	ss = setNormalise(input); CuAssertStrEquals(tc, expected, input); CuAssertIntEquals(tc, setOK, ss);

	// Boundary Case - 10 members - Should return setBadSet
	input = strdup("j i h g e d c b a z");   expected = "a b c d e g h i j z"; 
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

static int setTestDriver(int argc, UC8 **argv) {  
	const UC8 me[]="setTestDriver"; 
	 /* Lets keep complaints about not using argc/v quiet... */
	if (!argc) { 
		printf("%s: No Args: .\n", me);
	} else {
		int i;
		UC8 *mem;
		UC8 *otherSet;
		UC8 *uSet;
		UC8 newSet[300];
		UC8 newSet_B[300];
		myBool isMem;
		while (--argc > 0 && (*++argv)[0] == '-')
			switch ((*argv)[1]) {
			case 's':  
				--argc; argv++; i=setShow(mk0Null(*argv)); printf("\n");
				printf("%s: Show:          :%-15s: Result:           %3d\n", "set", *argv, i);
				return i;
			case 'c':
				--argc; argv++; i=setCardinal(mk0Null(*argv));
				printf("%s: Cardinal:      :%-15s:                : Result:%3d \n", "set", *argv, i);
				return i;
			case 'n':
				--argc; argv++; 
				// !! setNormalise writes to what's pointed at by its params. 
				//    That MUST be writable.  I don't think **argv is  is....
				if (*argv) strcpy(newSet, *argv); else newSet[0]=0;   // strcpy cant cope with NULLs.....
				i=setNormalise(mk0Null(newSet));
				printf("%s: Normalise:     :%-15s:%-15s : Status:%3d (%s)\n", "set", *argv, newSet, i, setStatus2Str(i) );
				return i;
			case 'm':
				--argc; argv++; mem=mk0Null(*argv);
				--argc; argv++; isMem=setIsNotMember(mem, mk0Null(*argv));
				printf("%s: isNotMember:   :%-15s:%-15s : Result:%3d (%s)\n", "set", mem, *argv, isMem, setStatus2Str(isMem));
				return isMem;
			case 'e': // Note - setIsNotEqual calls normalise, so *argv won't do......
				--argc; argv++; 
				otherSet=mk0Null(*argv);
				if (*argv) strcpy(newSet, otherSet); else newSet[0]=0;   // strcpy cant cope with NULLs.....
				--argc; argv++; 
				if (mk0Null(*argv)) strcpy(newSet_B, mk0Null(*argv)); else newSet_B[0]=0;   // strcpy cant cope with NULLs.....
				isMem=setIsNotEqual(newSet, newSet_B);
				printf("%s: setIsNotEqual: :%-15s, %-15s: Result:%3d (%s)\n", "set", otherSet, *argv, isMem, setStatus2Str(isMem));
				return isMem;
			case 'u':
				--argc; argv++; otherSet=mk0Null(*argv);
				--argc; argv++; uSet=setUnion(otherSet, mk0Null(*argv));
				printf("%s: setUnion:      :%-15s, %-15s: Result: %-15s\n", "set", otherSet, *argv, uSet);
				//printf("%s: : setUnion: Doing The CleanUp\n", me);
				setDelete(uSet);
				//printf("%s: : setUnion: CleanUp Done\n", me);
				// Like to return a decent value here
				break;
			case 'i':
				--argc; argv++; otherSet=mk0Null(*argv);
				--argc; argv++; uSet=setIntersection(otherSet, mk0Null(*argv));
				printf("%s: setIntersection:%-15s, %-15s: Result: %-15s\n", "set", otherSet, mk0Null(*argv), uSet);
				//printf("%s: : setIntersection: Doing The CleanUp\n", me);
				setDelete(uSet);
				//printf("%s: : setIntersection: CleanUp Done\n", me);
				// Like to return a decent value here
				break;
			default:
				printf("%s: : ERROR UNKNOWN: %c:\n", "set", (*argv)[1] );
			}
	}
	//printf("%s: Complete. No Status Value returned (only 0)\n", me);
	return 0; // What we return if we can't return anythingelse
}


int main(int argc, char **argv) {
		setImp.setMaxMembers= 8; //SET_MAX_MEMBERS_VALUE,
		setImp.setMaxChars  =25; //SET_MAX_CHARS_VALUE,

	if (argc >1) { // If command-line contains test cases, use that
		printf("\nSet: Implementation Bounds: setMaxMembers:%d, setMaxChars:%d\n\n", setImp.setMaxMembers, setImp.setMaxChars);
		return setTestDriver(argc, (UC8 **) argv);
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
		UC8  *av1[]  ={"progName", "-c", "a b c ", NULL};
		UC8  *av1_1[]={"progName", "-c", "abbey",  NULL};
		UC8  *av1_2[]={"progName", "-c", "",       NULL};
		UC8  *av1_3[]={"progName", "-c", NULL,     NULL};

		UC8  *av2[]  ={"progName", "-n", "a b d b c",       NULL};
		UC8  *av2_1[]={"progName", "-n", "ddd d a b b c b", NULL};
		UC8  *av2_2[]={"progName", "-n", "",                NULL};
		UC8  *av2_3[]={"progName", "-n", NULL,              NULL};

		UC8  *av3[]  ={"progName", "-m", "fred",     "alf frederick",NULL};
		UC8  *av3_1[]={"progName", "-m", "fred",     "fred alf john",NULL};
		UC8  *av3_10[]={"progName", "-m", "alf",      "fred alf john",NULL};
		UC8  *av3_11[]={"progName", "-m", "john",     "fred alf john",NULL};
		UC8  *av3_2[]={"progName", "-m", "fred joe", "fred joe",     NULL};
		UC8  *av3_3[]={"progName", "-m", "fred joe", "fred",         NULL};
		UC8  *av3_4[]={"progName", "-m", "fred",     "",             NULL};
		UC8  *av3_5[]={"progName", "-m", "",         "alf fred",     NULL};
		UC8  *av3_6[]={"progName", "-m", "",         "",             NULL};
		UC8  *av3_7[]={"progName", "-m", "",         NULL,           NULL};
		UC8  *av3_8[]={"progName", "-m", NULL,       "alf fred",     NULL};
		UC8  *av3_9[]={"progName", "-m", NULL,       NULL,           NULL};

		UC8  *av4[]  ={"progName", "-i", "a b c ", "c b e",          NULL};
		UC8  *av4_1[]={"progName", "-i", "a b c ", "k l m ",         NULL};
		UC8  *av4_2[]={"progName", "-i", "a b c ", "k l a",          NULL};
		UC8  *av4_3[]={"progName", "-i", "a b c ", "",               NULL};
		UC8  *av4_4[]={"progName", "-i", "",       "c b e",          NULL};
		UC8  *av4_5[]={"progName", "-i", "a b c ", NULL,             NULL};
		UC8  *av4_6[]={"progName", "-i", NULL,     "c b e",          NULL};
		UC8  *av4_7[]={"progName", "-i", "",       NULL,             NULL};
		UC8  *av4_8[]={"progName", "-i", NULL,      NULL,            NULL};

		UC8  *av5[]  ={"progName", "-u", "a b c ", "c b e",          NULL};
		UC8  *av5_1[]={"progName", "-u", "a b c ", "k l m ",         NULL};
		UC8  *av5_2[]={"progName", "-u", "a b c ", "k l a",          NULL};
		UC8  *av5_3[]={"progName", "-u", "a b c ", "",               NULL};
		UC8  *av5_4[]={"progName", "-u", "",       "c b e",          NULL};
		UC8  *av5_5[]={"progName", "-u", "a b c ", NULL,             NULL};
		UC8  *av5_6[]={"progName", "-u", NULL,     "c b e",          NULL};
		UC8  *av5_7[]={"progName", "-u", "",       NULL,             NULL};
		UC8  *av5_8[]={"progName", "-u", NULL,      NULL,            NULL};

		UC8  *av6[]  ={"progName", "-e", "a b c ", "a b c",          NULL};
		UC8  *av6_1[]={"progName", "-e", "a b c ", "c b   a",        NULL};
		UC8  *av6_2[]={"progName", "-e", "a b c ", "k l a",          NULL};
		UC8  *av6_3[]={"progName", "-e", "a b c ", "",               NULL};
		UC8  *av6_4[]={"progName", "-e", "",       "",               NULL};
		UC8  *av6_5[]={"progName", "-e", "a b c ", NULL,             NULL};
		UC8  *av6_6[]={"progName", "-e", NULL,     "c b e",          NULL};
		UC8  *av6_7[]={"progName", "-e", "",       NULL,             NULL};
		UC8  *av6_8[]={"progName", "-e", NULL,      NULL,            NULL};


		printf("Test 1    "); (void) setTestDriver(3, av1);
		printf("Test 1_1  "); (void) setTestDriver(3, av1_1);
		printf("Test 1_2  "); (void) setTestDriver(3, av1_2);
		printf("Test 1_3  "); (void) setTestDriver(3, av1_3);

		printf("Test 2    "); (void) setTestDriver(3, av2);
		printf("Test 2_1  "); (void) setTestDriver(3, av2_1);
		printf("Test 2_2  "); (void) setTestDriver(3, av2_2);
		printf("Test 2_3  "); (void) setTestDriver(3, av2_3);

		printf("Test 3    "); (void) setTestDriver(4, av3);
		printf("Test 3_1  "); (void) setTestDriver(4, av3_1);
		printf("Test 3_2  "); (void) setTestDriver(4, av3_2);
		printf("Test 3_3  "); (void) setTestDriver(4, av3_3);
		printf("Test 3_4  "); (void) setTestDriver(4, av3_4);
		printf("Test 3_5  "); (void) setTestDriver(4, av3_5);
		printf("Test 3_6  "); (void) setTestDriver(4, av3_6);
		printf("Test 3_7  "); (void) setTestDriver(4, av3_7);
		printf("Test 3_8  "); (void) setTestDriver(4, av3_8);
		printf("Test 3_9  "); (void) setTestDriver(4, av3_9);
		printf("Test 3_10 "); (void) setTestDriver(4, av3_10);
		printf("Test 3_11 "); (void) setTestDriver(4, av3_11);

		printf("Test 4    "); (void) setTestDriver(4, av4);
		printf("Test 4_1  "); (void) setTestDriver(4, av4_1);
		printf("Test 4_2  "); (void) setTestDriver(4, av4_2);
		printf("Test 4_3  "); (void) setTestDriver(4, av4_3);
		printf("Test 4_4  "); (void) setTestDriver(4, av4_4);
		printf("Test 4_5  "); (void) setTestDriver(4, av4_5);
		printf("Test 4_6  "); (void) setTestDriver(4, av4_6);
		printf("Test 4_7  "); (void) setTestDriver(4, av4_7);
		printf("Test 4_8  "); (void) setTestDriver(4, av4_8);

		printf("Test 5    "); (void) setTestDriver(4, av5);
		printf("Test 5_1  "); (void) setTestDriver(4, av5_1);
		printf("Test 5_2  "); (void) setTestDriver(4, av5_2);
		printf("Test 5_3  "); (void) setTestDriver(4, av5_3);
		printf("Test 5_4  "); (void) setTestDriver(4, av5_4);
		printf("Test 5_5  "); (void) setTestDriver(4, av5_5);
		printf("Test 5_6  "); (void) setTestDriver(4, av5_6);
		printf("Test 5_7  "); (void) setTestDriver(4, av5_7);
		printf("Test 5_8  "); (void) setTestDriver(4, av5_8);

		printf("Test 6    "); (void) setTestDriver(4, av6);
		printf("Test 6_1  "); (void) setTestDriver(4, av6_1);
		printf("Test 6_2  "); (void) setTestDriver(4, av6_2);
		printf("Test 6_3  "); (void) setTestDriver(4, av6_3);
		printf("Test 6_4  "); (void) setTestDriver(4, av6_4);
		printf("Test 6_5  "); (void) setTestDriver(4, av6_5);
		printf("Test 6_6  "); (void) setTestDriver(4, av6_6);
		printf("Test 6_7  "); (void) setTestDriver(4, av6_7);
		printf("Test 6_8  "); (void) setTestDriver(4, av6_8);
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

	printf("\nBoundary Testing with setMaxMembers:%d, setMaxChars:%d\n\n", setImp.setMaxMembers, setImp.setMaxChars);

		UC8  *av1_10[]={"progName", "-n", "a23456789012345678901234",  NULL};
		UC8  *av1_11[]={"progName", "-n", "a234567890123456789012345", NULL};
		UC8  *av1_12[]={"progName", "-n", "a 2 3 4 5 6 7", NULL};
		UC8  *av1_13[]={"progName", "-n", "a 2 3 4 5 6 7 8 9 0 1 2 3", NULL};
		UC8  *av1_14[]={"progName", "-c", "    g f e d c b a", NULL};
		UC8  *av1_15[]={"progName", "-c", "  h g f e d c b a", NULL};
		UC8  *av1_16[]={"progName", "-c", "i h g f e d c b a", NULL};
		UC8  *av1_17[]={"progName", "-i", "i h g f e d", "i h g f e d", NULL};
		UC8  *av1_18[]={"progName", "-u", "i h g f p a", "i h g f e d", NULL};
		UC8  *av1_19[]={"progName", "-u", "i h s f p a", "i h g f e d", NULL};
		UC8  *av1_20[]={"progName", "-i", "iii hhh ggg fff eee ", "fff eee ddd ccc bbb aaa", NULL};
		UC8  *av1_21[]={"progName", "-u", "iii hhh ggg fff eee ", "fff eee ddd ccc bbb aaa", NULL};
		printf("Test 1_10  "); (void) setTestDriver(3, av1_10);
		printf("Test 1_11  "); (void) setTestDriver(3, av1_11);
		printf("Test 1_12  "); (void) setTestDriver(3, av1_12);
		printf("Test 1_13  "); (void) setTestDriver(3, av1_13);
		printf("Test 1_14  "); (void) setTestDriver(3, av1_14);
		printf("Test 1_15  "); (void) setTestDriver(3, av1_15);
		printf("Test 1_16  "); (void) setTestDriver(3, av1_16);
		printf("Test 1_17  "); (void) setTestDriver(3, av1_17);
		printf("Test 1_18  "); (void) setTestDriver(3, av1_18);
		printf("Test 1_19  "); (void) setTestDriver(3, av1_19);
		printf("Test 1_20  "); (void) setTestDriver(3, av1_20);
		printf("Test 1_21  "); (void) setTestDriver(3, av1_21);



	}
	return 0;
}
#endif
