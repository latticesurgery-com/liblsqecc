INPUT="
OPENQASM 2.0;
include "qelib1.inc";

qreg q[25];
cx q[0],q[1]; // %ZXWithMBMControlFirst,AncillaNextToControl
cx q[2],q[3]; // %ZXWithMBMTargetFirst,AncillaNextToControl
cx q[4],q[5]; // %ZXWithMBMControlFirst,AncillaNextToTarget
cx q[6],q[7]; // %ZXWithMBMTargetFirst,AncillaNextToTarget
"
echo "$INPUT" | lsqecc_slicer -q -L edpc --nostagger



