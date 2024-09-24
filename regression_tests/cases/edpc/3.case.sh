INPUT="
OPENQASM 2.0;
include "qelib1.inc";

qreg q[16];

cx q[0],q[7]; // %BellBased
h q[1];
cx q[1],q[11]; // %BellBased
h q[8];
h q[7];
cx q[3], q[15]; // %BellBased
cx q[6],q[2]; // %BellBased
"
echo "$INPUT" | lsqecc_slicer -q -L edpc --nostagger --local -P edpc

