/* ***********************
File:               set.h
 Whoop's - lost the file - here's a rebuild - needs checking.
*/

#ifndef SET_H
#define SET_H
typedef signed long sl;
typedef int myBool;           /* Probably attach this to environment's boolean type */

typedef const char *memStr;       /* Std 'C' NULL-terminated str */
typedef unsigned char *set;                /* I might change it, BUT, Don't peek at this implementation... */
typedef const unsigned char *setConst;     /* Per 'set' but I contract not to alter it */

// There has to be a better way...
struct setImplementationStruct {  /* Singleton Blob of all the implementation details */
	unsigned int setMaxMembers;        /* Maximum members allowed in a set. However intermediary sets constructed with twice
	this  */
#define SET_MAX_MEMBERS_VALUE 10
	unsigned int setMaxChars;          /* Maximum buffer size (in chars) for the set. However intermediary sets constructed
	with twice this*/
#define SET_MAX_CHARS_VALUE 100
	unsigned char setMemberSeparator;  /* Single char to separate set members within a C string */
#define SET_MEMBER_SEPARATOR ' '
	unsigned char *setWhiteSpaceChars;  /* permitted separated within a string - but NOT normal form */
#define SET_WHITESPACE_CHARS " \t\n"
};
#if  defined(UNIT_TEST) || defined(CUTEST)
#define CONST_UNLESS_TESTING
#else
#define CONST_UNLESS_TESTING const
#endif
extern CONST_UNLESS_TESTING struct setImplementationStruct setImp;  // Users can use this for run-time visibility of the implementation


typedef enum setStatusEnum {setOK=0, setEmptySet, setBadSet, setBadCardinal, setBadMember, setMemberNotFound, setNotEqual} setStatus;
const unsigned char  *setStatus2Str(setStatus stat); 

setStatus          setShow(setConst setA);              // Show's or displays the set - not very useful
sl             setCardinal(setConst setA);              // Returns the Cardinality of the set - number of members
setStatus   setIsNotMember(memStr aMem, setConst setA); // Returns setOK when member found, !=0 otherwise

 // Note: These following functions generally will alter the value of 'set' - by Normalisation which
 //       only reduces the size of the supplied string.  Consumer shouldn't be peeking anyway.
 // When the fn() return value is a 'set', 
 //   o It is created by the (union/intersect) function 
 //   o Use setDelete to delete
setStatus setIsNotEqual(set setA, set setB); // Retruns setOK when the sets are not equal, !=0 otherwise
setStatus  setNormalise(set setA);           

set            setUnion(setConst setA, setConst setB); 
set     setIntersection(setConst setA, setConst setB); 
setStatus     setDelete(set setA);           // Delete this set - iff created by union or intersection

// F u t u r e    D e v e l o p m e n t     S e t     C o n s t r u c t i o n    . . . . . . . . . .
// char *set2String(setConst setA);         // I guess I need to malloc here......
// set setCreateEmptySet();                 // I guess I need to malloc here......
// set setAddMember(memStr memA, set setA); // I guess I need to malloc here......
// set  setRmMember(memStr memA, set setA); // I guess I need to RE-malloc here......
//
//
//

#endif /* SET_H */
