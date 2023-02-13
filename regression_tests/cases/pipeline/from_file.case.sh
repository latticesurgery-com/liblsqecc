INPUT="
DeclareLogicalQubitPatches 0,1
LogicalPauli 1 X
"
echo "$INPUT" > input_file
lsqecc_slicer -i input_file -L compact
rm input_file