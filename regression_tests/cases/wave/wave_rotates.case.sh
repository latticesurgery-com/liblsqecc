INPUT="
DeclareLogicalQubitPatches 0,1
RotateSingleCellPatch 0
RotateSingleCellPatch 0
RotateSingleCellPatch 1
"

LAYOUT="
rr
QQ
"

echo "$LAYOUT" > tmp.layout
echo "$INPUT" | lsqecc_slicer -l tmp.layout -P wave
rm tmp.layout