INPUT="
OPENQASM 2.0;
include "qelib1.inc";

qreg q[16];

cx q[0],q[8]; // %BellBased
cx q[1],q[9]; // %BellBased

"
echo "$INPUT" | lsqecc_slicer -q -L edpc --nostagger --local -P edpc

