AUTHOR
Matthew Hargreaves

BACKGROUND
This library of C functions was written on a whim to use as a basis for demonstrating the use of a unit test framework - Rational's Test Real-Time (RTRT), and then later traditional C unit testing - placing the unit tests within the source code file under conditional compilation.

CONCEPT
A library of C functions that operate on C strings providing the functionality of set algebra - union, intersection etc.
So if a="apple pear egg" and b="apple sun", then intersection(a, b)="apple" and union(a, b)="apple pear egg sun".
It turns out to implement this you need a few support funcitons such as normalForm etc.
The library contains the source code and the unit tests (the reason for writing the library in the first place).
The makeFile is for many other testing frameworks as well.

COMMAND-LINE
A command-line interface is also provided.  I have used the command-line in scripts to manipulate strings where I want the intersection.  Quite Handy - but see also xgrep which has must of the functionality anyway.

LICENSE
Public Domain, though, like any artist, I would like attribution.

TEST RESULTS
Test 1    set: Cardinal:      :a b c          :                : Result:  3 
Test 1_1  set: Cardinal:      :abbey          :                : Result:  1 
Test 1_2  set: Cardinal:      :               :                : Result:  0 
Test 1_3  set: Cardinal:      :(null)         :                : Result: -1 
Test 2    set: Normalise:     :a b d b c      :a b c d         : Status:  0 (setOK)
Test 2_1  set: Normalise:     :ddd d a b b c b:a b c d ddd     : Status:  0 (setOK)
Test 2_2  set: Normalise:     :               :                : Status:  0 (setOK)
Test 2_3  set: Normalise:     :(null)         :                : Status:  0 (setOK)
Test 3    set: isNotMember:   :fred           :alf frederick   : Result:  5 (setMemberNotFound)
Test 3_1  set: isNotMember:   :fred           :fred alf john   : Result:  0 (setOK)
Test 3_2  set: isNotMember:   :fred joe       :fred joe        : Result:  4 (setBadMember)
Test 3_3  set: isNotMember:   :fred joe       :fred            : Result:  4 (setBadMember)
Test 3_4  set: isNotMember:   :fred           :                : Result:  1 (setEmptySet)
Test 3_5  set: isNotMember:   :               :alf fred        : Result:  4 (setBadMember)
Test 3_6  set: isNotMember:   :               :                : Result:  1 (setEmptySet)
Test 3_7  set: isNotMember:   :               :(null)          : Result:  2 (setBadSet)
Test 3_8  set: isNotMember:   :(null)         :alf fred        : Result:  4 (setBadMember)
Test 3_9  set: isNotMember:   :(null)         :(null)          : Result:  2 (setBadSet)
Test 3_10 set: isNotMember:   :alf            :fred alf john   : Result:  0 (setOK)
Test 3_11 set: isNotMember:   :john           :fred alf john   : Result:  0 (setOK)
Test 4    set: setIntersection:a b c          , c b e          : Result: b c            
Test 4_1  set: setIntersection:a b c          , k l m          : Result:                
Test 4_2  set: setIntersection:a b c          , k l a          : Result: a              
Test 4_3  set: setIntersection:a b c          ,                : Result:                
Test 4_4  set: setIntersection:               , c b e          : Result:                
Test 4_5  set: setIntersection:a b c          , (null)         : Result: (null)         
Test 4_6  set: setIntersection:(null)         , c b e          : Result: (null)         
Test 4_7  set: setIntersection:               , (null)         : Result: (null)         
Test 4_8  set: setIntersection:(null)         , (null)         : Result: (null)         
Test 5    set: setUnion:      :a b c          , c b e          : Result: a b c e        
Test 5_1  set: setUnion:      :a b c          , k l m          : Result: a b c k l m    
Test 5_2  set: setUnion:      :a b c          , k l a          : Result: a b c k l      
Test 5_3  set: setUnion:      :a b c          ,                : Result: a b c          
Test 5_4  set: setUnion:      :               , c b e          : Result: b c e          
Test 5_5  set: setUnion:      :a b c          , (null)         : Result: (null)         
Test 5_6  set: setUnion:      :(null)         , c b e          : Result: (null)         
Test 5_7  set: setUnion:      :               , (null)         : Result: (null)         
Test 5_8  set: setUnion:      :(null)         , (null)         : Result: (null)         
Test 6    set: setIsNotEqual: :a b c          , a b c          : Result:  0 (setOK)
Test 6_1  set: setIsNotEqual: :a b c          , c b   a        : Result:  0 (setOK)
Test 6_2  set: setIsNotEqual: :a b c          , k l a          : Result:  6 (setNotEqual)
Test 6_3  set: setIsNotEqual: :a b c          ,                : Result:  6 (setNotEqual)
Test 6_4  set: setIsNotEqual: :               ,                : Result:  0 (setOK)
Test 6_5  set: setIsNotEqual: :a b c          , (null)         : Result:  6 (setNotEqual)
Test 6_6  set: setIsNotEqual: :(null)         , c b e          : Result:  6 (setNotEqual)
Test 6_7  set: setIsNotEqual: :               , (null)         : Result:  0 (setOK)
Test 6_8  set: setIsNotEqual: :(null)         , (null)         : Result:  0 (setOK)

Boundary Testing with setMaxMembers:8, setMaxChars:25

Test 1_10  set: Normalise:     :a23456789012345678901234:a23456789012345678901234 : Status:  0 (setOK)
Test 1_11  set: Normalise:     :a234567890123456789012345:a234567890123456789012345 : Status:  2 (setBadSet)
Test 1_12  set: Normalise:     :a 2 3 4 5 6 7  :2 3 4 5 6 7 a   : Status:  0 (setOK)
Test 1_13  set: Normalise:     :a 2 3 4 5 6 7 8 9 0 1 2 3:0 1 2 3 4 5 6 7 8 9 a : Status:  0 (setOK)
Test 1_14  set: Cardinal:      :    g f e d c b a:                : Result:  7 
Test 1_15  set: Cardinal:      :  h g f e d c b a:                : Result: -1 
Test 1_16  set: Cardinal:      :i h g f e d c b a:                : Result: -1 
Test 1_17  set: setIntersection:i h g f e d    , i h g f e d    : Result: d e f g h i    
Test 1_18  set: setUnion:      :i h g f p a    , i h g f e d    : Result: a d e f g h i p
Test 1_19  set: setUnion:      :i h s f p a    , i h g f e d    : Result: a d e f g h i p s
Test 1_20  set: setIntersection:iii hhh ggg fff eee , fff eee ddd ccc bbb aaa: Result: eee fff        
Test 1_21  set: setUnion:      :iii hhh ggg fff eee , fff eee ddd ccc bbb aaa: Result: (null)         
3
1
0
-1
a b c d
a b c d ddd


5
0
4
4
1
4
1
2
4
2
0
0
b c

a


(null)
(null)
(null)
(null)
a b c e
a b c k l m
a b c k l
a b c
b c e
(null)
(null)
(null)
(null)
0
0
6
6
0
6
6
0
0
a23456789012345678901234
a234567890123456789012345
2 3 4 5 6 7 a
0 1 2 3 4 5 6 7 8 9 a
7
-1
-1
d e f g h i
a d e f g h i p
a d e f g h i p s
eee fff
(null)
