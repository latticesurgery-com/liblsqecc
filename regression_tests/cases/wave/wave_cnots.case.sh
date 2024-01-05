INPUT="
OPENQASM 2.0;
include "qelib1.inc";

qreg q[6];

cx q[0],q[1]; // %BellBased
cx q[0],q[2]; // %BellBased
cx q[4],q[5]; // %BellBased
"

echo "$INPUT" | lsqecc_slicer -q -L edpc -P wave --local
