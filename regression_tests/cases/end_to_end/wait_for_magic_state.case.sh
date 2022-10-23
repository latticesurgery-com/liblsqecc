INPUT="
OPENQASM 2.0;
include \"qelib1.inc\";

qreg q[15];
t q[2];
"
echo "$INPUT" | lsqecc_slicer -q --compactlayout