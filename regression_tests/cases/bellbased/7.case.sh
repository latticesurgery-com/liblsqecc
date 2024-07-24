INPUT="
DeclareLogicalQubitPatches 0,1,2,3,4,5,6
BellPairInit 200 201 6:Z,2:Z
"
echo "$INPUT" | lsqecc_slicer -L edpc --nostagger --local



