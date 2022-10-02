OPENQASM 2.0;
include "qelib1.inc";

qreg q[8];

h q[1];
h q[2];
s q[3];
s q[4];
x q[5];
s q[6];
cx q[0],q[7];
cx q[0],q[7]; // %ZXWithMBMControlFirst
cx q[0],q[7]; // %ZXWithMBMTargetFirst
cx q[0],q[7]; // %ZXWithMBMControlFirst,AncillaNextToControl
cx q[0],q[7]; // %ZXWithMBMControlFirst,AncillaNextToTarget
cx q[0],q[7]; // %ZXWithMBMTargetFirst,AncillaNextToControl
cx q[0],q[7]; // %ZXWithMBMTargetFirst,AncillaNextToTarget