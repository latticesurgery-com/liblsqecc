INPUT="
OPENQASM 2.0;
include "qelib1.inc";

qreg q[25];

t q[0];
"
echo "$INPUT" | lsqecc_slicer -q -L edpc --nostagger --local --notwists



