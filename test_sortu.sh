#!/bin/sh

TEST1=sortu_test1.t
TEST2=sortu_test2.t
EXPECTED=sortu_exp.t
OUTPUT=sortu_out.t

rm -f $TEST1 $TEST2 $EXPECTED $OUTPUT

check () {
    if [ $? -ne 0 ]; then
	exit 1
    fi
    
    diff -q --ignore-all-space $OUTPUT $EXPECTED
    if [ $? -ne 0 ]; then
	echo "ERROR: output from test not expected: $NAME"
	exit 2
    fi
    return 0
}

###############################################################################
# Basic testing
###############################################################################

NAME="basic alpha already sorted"

cat > $TEST1 <<EOF
1
2
3
4
EOF

cat > $EXPECTED <<EOF
1 1
1 2
1 3
1 4
EOF

sortu $TEST1 > $OUTPUT
check

########################################

NAME="basic alpha not sorted"

cat > $TEST1 <<EOF
3
1
2
4
EOF

cat > $EXPECTED <<EOF
1 1
1 2
1 3
1 4
EOF

check

########################################

NAME="basic alpha reverse sorted"

cat > $TEST1 <<EOF
4
3
2
1
EOF

cat > $EXPECTED <<EOF
1 1
1 2
1 3
1 4
EOF

sortu $TEST1 > $OUTPUT
check

########################################

NAME="basic count sort"

cat > $TEST1 <<EOF
4
3
2
1
3
EOF

cat > $EXPECTED <<EOF
1 1
1 2
1 4
2 3
EOF

sortu $TEST1 > $OUTPUT
check

########################################

NAME="case match"

cat > $TEST1 <<EOF
D
c
B
a
EOF

cat > $EXPECTED <<EOF
1 B
1 D
1 a
1 c
EOF

sortu $TEST1 > $OUTPUT
check

########################################

NAME="alpha not number sort"

cat > $TEST1 <<EOF
1  a
10 b
3  c
EOF

cat > $EXPECTED <<EOF
1 1  a
1 10 b
1 3  c
EOF

sortu $TEST1 > $OUTPUT
check

###############################################################################
# Argument testing
###############################################################################

NAME="blank ignore argument"

cat > $TEST1 <<EOF
1

2

3
4
EOF

cat > $EXPECTED <<EOF
1 1
1 2
1 3
1 4
EOF

sortu -b $TEST1 > $OUTPUT
check

########################################

NAME="cumulative numbers argument"

cat > $TEST1 <<EOF
2
9
4
8
EOF

cat > $EXPECTED <<EOF
1 1 2
1 2 4
1 3 8
1 4 9
EOF

sortu -c $TEST1 > $OUTPUT
check

########################################

NAME="no count argument"

cat > $TEST1 <<EOF
4
2
1
3
EOF

cat > $EXPECTED <<EOF
1
2
3
4
EOF

sortu -C $TEST1 > $OUTPUT
check

########################################

NAME="field delimiter and number arguments"

cat > $TEST1 <<EOF
4,d,D,
2,b,B,
1,a,A,
3,c,C,
EOF

cat > $EXPECTED <<EOF
1 a
1 b
1 c
1 d
EOF

sortu -d=, -f 2  $TEST1 > $OUTPUT
check

########################################

NAME="key sort argument"

cat > $TEST1 <<EOF
4,d,D,
2,b,B,
1,a,A,
3,c,C,
EOF

cat > $EXPECTED <<EOF
1 a
1 b
1 c
1 d
EOF

sortu -d=, -f 2  $TEST1 > $OUTPUT
check

########################################

NAME="key sort argument"

cat > $TEST1 <<EOF
1
2
1
3
1
2
EOF

cat > $EXPECTED <<EOF
3 1
2 2
1 3
EOF

sortu -r $TEST1 > $OUTPUT
check

########################################

NAME="loose field argument"

cat > $TEST1 <<EOF
1  3
2 1
3   2
EOF

cat > $EXPECTED <<EOF
1 1
1 2
1 3
EOF

sortu -f 2 -l $TEST1 > $OUTPUT
check

########################################

NAME="minimum match argument"

cat > $TEST1 <<EOF
1
2
3
1
EOF

cat > $EXPECTED <<EOF
2 1
EOF

sortu -m 2 $TEST1 > $OUTPUT
check

########################################

NAME="maximum match argument"

cat > $TEST1 <<EOF
1
2
3
1
EOF

cat > $EXPECTED <<EOF
1 2
1 3
EOF

sortu -M 1 $TEST1 > $OUTPUT
check

########################################

NAME="case insensitive argument"

cat > $TEST1 <<EOF
D
c
B
a
EOF

cat > $EXPECTED <<EOF
1 a
1 b
1 c
1 d
EOF

sortu -i $TEST1 > $OUTPUT
check

########################################

NAME="number argument"

cat > $TEST1 <<EOF
1  a
10 b
3  c
EOF

cat > $EXPECTED <<EOF
1 1
1 3
1 10
EOF

sortu -n $TEST1 > $OUTPUT
check

########################################
# ERROR
#
NAME="number float argument"
echo "WARNING: test fails: $NAME"

#cat > $TEST1 <<EOF
#1.1 a
#10  b
#1   c
#EOF
#
#cat > $EXPECTED <<EOF
#1 1.00
#1 1.10
#1 10.00
#EOF

#sortu -N $TEST1 > $OUTPUT
#check

########################################

NAME="order keep argument"

cat > $TEST1 <<EOF
3
1
2
EOF

cat > $EXPECTED <<EOF
1 3
1 1
1 2
EOF

sortu -o $TEST1 > $OUTPUT
check

########################################

NAME="percentage show argument"

cat > $TEST1 <<EOF
3
1
2
1
EOF

cat > $EXPECTED <<EOF
1 25% 2
1 25% 3
2 50% 1
EOF

sortu -p $TEST1 > $OUTPUT
check

########################################

NAME="reverse sort argument"

cat > $TEST1 <<EOF
4
1
3
2
EOF

cat > $EXPECTED <<EOF
1 4
1 3
1 2
1 1
EOF

sortu -r $TEST1 > $OUTPUT
check

########################################

NAME="start offset argument"

cat > $TEST1 <<EOF
a4
b1
c3
d2
EOF

cat > $EXPECTED <<EOF
1 1
1 2
1 3
1 4
EOF

sortu -s 1 $TEST1 > $OUTPUT
check

########################################

NAME="start offset argument"

cat > $TEST1 <<EOF
a4
b1
a3
b2
EOF

cat > $EXPECTED <<EOF
2 a
2 b
EOF

sortu -S 1 $TEST1 > $OUTPUT
check

########################################

NAME="verbose argument"

cat > $TEST1 <<EOF
4
1
3
2
EOF

cat > $EXPECTED <<EOF
Count: Data:     
---------- ----------
1 1
1 2
1 3
1 4
---------- ----------
4 Total     
EOF

sortu -v $TEST1 > $OUTPUT
check

###############################################################################

rm -f $TEST1 $TEST2 $EXPECTED $OUTPUT
