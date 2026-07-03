INPUT="
OPENQASM 2.0;
include "qelib1.inc";

qreg q[17];

"
echo "$INPUT" | lsqecc_slicer -I qasm -L edpc --condensed 1 --nostagger



