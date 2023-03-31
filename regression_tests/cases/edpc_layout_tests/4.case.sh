INPUT="
OPENQASM 2.0;
include "qelib1.inc";

qreg q[7];

cx q[0],q[1];
cx q[2],q[3];
cx q[4],q[5];
cx q[6],q[2];
"
echo "$INPUT" | lsqecc_slicer -q -L edpc --nostagger



