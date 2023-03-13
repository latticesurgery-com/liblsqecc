INPUT="
DeclareLogicalQubitPatches 0,1,2,3,4,5,6
RotateSingleCellPatch 0
RotateSingleCellPatch 2
"
echo "$INPUT" | lsqecc_slicer -l ../examples/core4by4layout.txt --printlli sliced -P dag