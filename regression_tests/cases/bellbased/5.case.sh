INPUT="
DeclareLogicalQubitPatches 0,1,2,3,4,5,6
BellPairInit 200 201 6:X,5:Z
Init 100 +
RotateSingleCellPatch 100
HGate 1
HGate 1
RotateSingleCellPatch 1
"
echo "$INPUT" | lsqecc_slicer -L edpc --nostagger --printlli sliced --local



