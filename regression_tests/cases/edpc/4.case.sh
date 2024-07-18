INPUT="
OPENQASM 2.0;
include "qelib1.inc";

qreg q[16];

cx q[6],q[0]; // %BellBased
cx q[1],q[3]; // %BellBased
cx q[5], q[13]; // %BellBased
cx q[12],q[7]; // %BellBased
"
echo "$INPUT" | lsqecc_slicer -q -L edpc --nostagger --local -P edpc

