INPUT="
DeclareLogicalQubitPatches 0,1,2,3,4,5,6
BellPairInit 200 201 6:X,5:Z
Init 100 +
MultiBodyMeasure 0:X,4:X
MultiBodyMeasure 5:Z,6:Z
MultiBodyMeasure 2:Z,3:Z
RotateSingleCellPatch 100
HGate 1
HGate 1
RotateSingleCellPatch 1
MultiBodyMeasure 0:Z,100:Z
"
echo "$INPUT" | lsqecc_slicer -L edpc --nostagger



