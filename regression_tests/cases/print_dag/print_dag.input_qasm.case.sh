INPUT="
OPENQASM 2.0;
include \"qelib1.inc\";

qreg q[15];

cx q[0],q[1];
crz(pi/16) q[0],q[2];
cx q[5],q[6];
crz(pi/16) q[6],q[7];
h q[0];
h q[1];
h q[2];
h q[5];
h q[6];
h q[7];
cx q[7],q[8];
cx q[6],q[8];
"

echo "$INPUT" | lsqecc_slicer -q --printdag input