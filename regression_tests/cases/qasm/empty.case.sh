INPUT="
OPENQASM 2.0;
include \"qelib1.inc\";

qreg q[15];
"
echo "$INPUT" | lsqecc_slicer --compactlayout
