INPUT="
OPENQASM 2.0;
include "qelib1.inc";

qreg q[2];

h q[1];
h q[1];
h q[1];
h q[1];
h q[1];
h q[1];
h q[1];
h q[1];
h q[1];
h q[1];
h q[1];
h q[1];
t q[1];
t q[1];
"
echo "$INPUT" | lsqecc_slicer -q --nostagger --disttime 11