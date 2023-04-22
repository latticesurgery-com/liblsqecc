INPUT="
OPENQASM 2.0;
include "qelib1.inc";

qreg q[25];

cx q[0],q[1]; // %BellBased
cx q[2],q[3]; // %BellBased
cx q[4],q[5]; // %BellBased
cx q[6],q[7]; // %BellBased
"
echo "$INPUT" | lsqecc_slicer -q -L edpc --nostagger -P dag --local