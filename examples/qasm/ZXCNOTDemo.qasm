OPENQASM 2.0;
include "qelib1.inc";

qreg q[15];

cx q[0],q[10]; // %ZXWithMBMControlFirst,AncillaFreePlacement
cx q[0],q[4]; // %ZXWithMBMControlFirst,AncillaFreePlacement
cx q[0],q[12]; // %ZXWithMBMTargetFirst,AncillaFreePlacement
cx q[0],q[14]; // %ZXWithMBMTargetFirst,AncillaFreePlacement
