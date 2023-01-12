INPUT="
OPENQASM 2.0;
include \"qelib1.inc\";

qreg q[15];

crz(pi/16) q[0],q[3];
"
echo "$INPUT" | lsqecc_slicer -q --compactlayout