; file ps.am
.entry LIST
.extern W
MAIN: add r3, LIST
LOOP: prn #48
lea STR, r6
inc r6
mov r3, W
sub r1, r4
bne END
cmp val1, #-6
bne END[r15]
dec K
.entry MAIN
sub LOOP[r10] ,r14
END: stop
STR: .string "abcd"
LIST: .data 6, -9
.data -100
.entry cmp
r1: .data 34
mov: .data 3,4
K: .data 31
.extern val1