# cygwin based - need to handle DOS as well. - research that.
#  Generally this is called from within a cygwin environment....
# Notes
# o set.c   is the library being developed
#   o It contains some UNIT_TEST code (via main()) included via -DUNIT_TEST
#     o This code (1) if no cmmd-line-args passed, executes a test suite
#     o This code (2) if    cmmd-line-args passed, performs per the command-line
#       o Some cmmd-line testCases are also within this Makefile as target 'testCases'
#   o It has a cuTest test code included via -DCU_TEST
#     o In this case set.c, is a true library (no main()), and allTests.c is the testHarness
# o Targets are also available for evaluating test coverage with 'gcov'.
# o Other, various targets are available for static analysis, viz., lint, qa-c, etc., etc.
# o setMisra.c is an alternative version of this library, coded to obey MISRA rules.
# o 
# Ideas
#   o gcc -Wl,--heap,1024,--stack,4096 -o foo foo.c 
# o 

BAT_FROM_CYGWIN=$(cygpath ${COMSPEC})
LIN_BAT=${HOME}/dev/tools/LIN.BAT
LIN_BAT=D:/home/mattharg/dev/tools/LIN.BAT 
CMD=/cygdrive/c/WINNT/system32/cmd.exe 


SOURCES=set.c 
HEADERS=set.h


UNIT_TEST=set
UNIT_TEST=set.exe


CFLAGS=-g -Wall
C_GCOV_FLAGS=-fprofile-arcs -ftest-coverage
LIB_NEON=-I /usr/include/neon
LIB_CUTEST= -I ${HOME}/dev/cutest
LIBS=
DEFINES_UNIT_TEST=-DUNIT_TEST
DEFINES_CUTEST=-DCUTEST

# D E F A U L T      T A R G E T    Put in a std. bld target here..
help:  # Print Out All The Targets
	awk '/^[^ ]*:/ {print $$1}'         Makefile

${UNIT_TEST}: Makefile ${SOURCES} ${HEADERS}
	${CC} ${CFLAGS} ${DEFINES_UNIT_TEST} ${LIBS} ${SOURCES} -o ${UNIT_TEST}

lint:    # Lint Eg.
	${CMD} /c ${LIN_BAT} --editor --gcc   ${DEFINES_UNIT_TEST} set.c  | ${PAGER}

misra:
	${CMD} /c ${LIN_BAT} --editor --misra ${DEFINES_UNIT_TEST} set.c  | ${PAGER}

misraGcc:
	${CMD} /c ${LIN_BAT} --gcc    --misra ${DEFINES_UNIT_TEST} set.c  | ${PAGER}

splint:
	splint.exe set.c set.h | ${PAGER}

splintMatt:
	splint.exe -boolops set.c set.h | ${PAGER}

splintQuiet:
	splint.exe -boolops -exportlocal -predboolint -predboolothers set.c | ${PAGER}

qac:
	run_qac.bat set.c | ${PAGER}

cleanAll:
	rm *.da *.bb *.exe

allTests.exe:	allTests.c Makefile ${SOURCES} ${HEADERS}
	${CC} ${CFLAGS} ${DEFINES_CUTEST} ${LIBS} ${LIB_CUTEST} allTests.c ${SOURCES} -o allTests.exe

cuTest: allTests.exe
	allTests.exe

allTests.bb: Makefile ${SOURCES} ${HEADERS}
	${CC} ${C_GCOV_FLAGS} ${CFLAGS} ${DEFINES_CUTEST} ${LIBS} ${LIB_CUTEST} allTests.c ${SOURCES} -o allTests.exe

allTests.da: allTests.bb Makefile ${SOURCES} ${HEADERS}
	./allTests.exe

allTests.c.gcov: allTests.da Makefile ${SOURCES} ${HEADERS}
	gcov -f set.c
	gcov -f allTests.c

allTests.c.Maxgcov: allTests.da Makefile ${SOURCES} ${HEADERS}
	gcov -f -bc      set.c
	gcov -f -bc allTests.c
	mv      set.c.gcov      set.c.Maxgcov
	mv allTests.c.gcov allTests.c.Maxgcov

cuTestGcov: allTests.c.gcov 
	less set.c.gcov  allTests.c.gcov

cuTestGcovMax: allTests.c.Maxgcov 
	less set.c.Maxgcov  allTests.c.Maxgcov



set.bb: Makefile ${SOURCES} ${HEADERS}
	${CC} ${C_GCOV_FLAGS} ${CFLAGS} ${DEFINES_UNIT_TEST} ${LIBS} ${SOURCES} -o ${UNIT_TEST}

set.da: set.bb Makefile ${SOURCES} ${HEADERS}
	./set.exe

set.c.gcov: set.da Makefile ${SOURCES} ${HEADERS}
	gcov -f set.c

set.c.Maxgcov: set.da Makefile ${SOURCES} ${HEADERS}
	gcov -f -bc set.c
	mv set.c.gcov set.c.Maxgcov

gcov: set.c.gcov 
	less set.c.gcov

gcovMax: set.c.Maxgcov 
	less set.c.Maxgcov


set_misra.exe: set_misra.c set_misra.h Makefile
	$(CC) $(CFLAGS) ${DEFINES_UNIT_TEST} -I . set_misra.c -o set_misra.exe

# This test stuff would need checking of result values - but heh we use RTRT instead anyway.
# Simple 1st pass test cases - needs work ...
QUIET=-n
# ...{setOK=0, setEmptySet, setBadSet, setBadCardinal, setBadMember, setMemberNotFound, setNotEqual} setStatus;
#        0         1             2           3              4                5               6
testCases:
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -s \"alice fred john\");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -s \"fred john\");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -s \"alice\");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -s \"\");
	(./expectReturn.sh ${QUIET} 2   ./${UNIT_TEST} -s 0);
	(./expectReturn.sh ${QUIET} 3   ./${UNIT_TEST} -c \"alice fred john\");
	(./expectReturn.sh ${QUIET} 2   ./${UNIT_TEST} -c \"fred john\");
	(./expectReturn.sh ${QUIET} 1   ./${UNIT_TEST} -c \"alice\");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -c \"\");
	(./expectReturn.sh ${QUIET} 255  ./${UNIT_TEST} -c 0);
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -n \"fred alice john\");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -n \"fred fred john fred john\");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -n \"alice\");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -n \"f f a f e d d c b a f f a \");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -n \"\");
	(./expectReturn.sh ${QUIET} 2   ./${UNIT_TEST} -n 0);
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -m \"alice\" \"alice fred john\");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -m \"fred\"  \"alice fred john\");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -m \"john\"  \"alice fred john\");
	(./expectReturn.sh ${QUIET} 5   ./${UNIT_TEST} -m \"alice\" \"fred john\");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -m \"fred john\" \"fred john\");
	(./expectReturn.sh ${QUIET} 5   ./${UNIT_TEST} -m \"alice and fred \"  \"alice\");
	(./expectReturn.sh ${QUIET} 1   ./${UNIT_TEST} -m \"alice\" \"\");
	(./expectReturn.sh ${QUIET} 2   ./${UNIT_TEST} -m \"alice\" 0);
	(./expectReturn.sh ${QUIET} 4   ./${UNIT_TEST} -m \"\" \"fred john\");
	(./expectReturn.sh ${QUIET} 4   ./${UNIT_TEST} -m 0 \"fred john\");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -e \"alice fred john\" \"alice fred john\");
	(./expectReturn.sh ${QUIET} 6   ./${UNIT_TEST} -e \"fred\"  \"alice fred john\");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -e \"john\"  \"john\");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -e \"alice fred\" \"fred alice\");
	(./expectReturn.sh ${QUIET} 6   ./${UNIT_TEST} -e \"alice\" \"\");
	(./expectReturn.sh ${QUIET} 2   ./${UNIT_TEST} -e \"alice\" 0);
	(./expectReturn.sh ${QUIET} 6   ./${UNIT_TEST} -e \"\" \"fred john\");
	(./expectReturn.sh ${QUIET} 2   ./${UNIT_TEST} -e 0  \"fred john\");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -u \"alice fred john\" \"alice fred john\");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -u \"joseph\"  \"alice fred john\");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -u \"john\"  \"johnathan\");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -u \"alice fred\" \"fred ziggy\");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -u \"alice\" \"\");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -u \"alice john\" \"\");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -u \"alice\" 0);
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -u \"\" \"fred john\");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -u 0  \"fred john\");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -i \"alice fred john\" \"alice fred john\");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -i \"joseph\"  \"alice fred john\");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -i \"john\"  \"johnathan\");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -i \"alice fred\" \"fred ziggy\");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -i \"alice\" \"\");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -i \"alice john\" \"\");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -i \"alice\" 0);
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -i \"\" \"fred john\");
	(./expectReturn.sh ${QUIET} 0   ./${UNIT_TEST} -i 0  \"fred john\");




