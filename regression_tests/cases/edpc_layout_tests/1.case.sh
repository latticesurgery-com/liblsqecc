INPUT="
OPENQASM 2.0;
include "qelib1.inc";

qreg q[4];

h q[0];
s q[0];
t q[0];
cx q[0],q[1]; // %BellBased
"
echo "$INPUT" | lsqecc_slicer -q -L edpc --nostagger --local

