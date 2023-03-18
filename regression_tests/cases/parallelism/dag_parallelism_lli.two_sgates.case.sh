INPUT="
DeclareLogicalQubitPatches 0,1
SGate 0
SGate 1
HGate 0
HGate 0
"

LAYOUT="
rrrr
QrrQ
"

echo "$LAYOUT" > tmp.layout
echo "$INPUT" | lsqecc_slicer -l tmp.layout -P dag
rm tmp.layout