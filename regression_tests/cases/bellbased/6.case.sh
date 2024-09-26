INPUT="
DeclareLogicalQubitPatches 0,1,2,3,4,5,6
BellPairInit 200 201 0:Z,3:X
Init 100 +
RotateSingleCellPatch 100
HGate 1
HGate 1
RotateSingleCellPatch 1
"
echo "$INPUT" | lsqecc_slicer -l ../examples/core4by4layout.txt --nostagger --printlli sliced --local



