INPUT="
OPENQASM 2.0;
include "qelib1.inc";
qreg q[4];
reset q[0];
cx q[0], q[1];
reset q[0];
"
echo "$INPUT" | lsqecc_slicer -q -L compact
