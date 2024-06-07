









RequestMagicState 25 0;BellBasedCNOT 0 25 26 27 [ExtendSplit (6,6),(5,6);BellPrepare (4,5),(5,5)];
BellBasedCNOT 0 25 26 27 [BellMeasure (5,5),(5,6);MergeContract (4,6),(4,5)];MeasureSinglePatch 25 Z;RequestYState 28 0 [PrepareY (7,6)];
MultiBodyMeasure 28:Z,0:Z [TwoPatchMeasure (7,6),(6,6)];MeasureSinglePatch 28 X;
ZGate 0;RequestMagicState 29 1;BellBasedCNOT 1 29 30 31 [ExtendSplit (6,8),(5,8);BellPrepare (4,9),(5,9)];
BellBasedCNOT 1 29 30 31 [BellMeasure (5,9),(5,8);MergeContract (4,10),(4,9)];MeasureSinglePatch 29 Z;RequestYState 32 1 [PrepareY (7,8)];
MultiBodyMeasure 32:Z,1:Z [TwoPatchMeasure (7,8),(6,8)];MeasureSinglePatch 32 X;
ZGate 1;RequestMagicState 33 2;BellBasedCNOT 2 33 34 35 [ExtendSplit (6,10),(5,10);BellPrepare (4,13),(5,13);BellPrepare (5,12),(5,11)];
BellBasedCNOT 2 33 34 35 [BellMeasure (5,13),(5,12);BellMeasure (5,11),(5,10);MergeContract (4,14),(4,13)];MeasureSinglePatch 33 Z;RequestYState 36 2 [PrepareY (7,10)];
MultiBodyMeasure 36:Z,2:Z [TwoPatchMeasure (7,10),(6,10)];MeasureSinglePatch 36 X;
ZGate 2;RequestMagicState 37 3;BellBasedCNOT 3 37 38 39 [ExtendSplit (6,12),(7,12);BellPrepare (8,15),(7,15);BellPrepare (7,14),(7,13)];
BellBasedCNOT 3 37 38 39 [BellMeasure (7,15),(7,14);BellMeasure (7,13),(7,12);MergeContract (8,16),(8,15)];MeasureSinglePatch 37 Z;RequestYState 40 3 [PrepareY (5,12)];
MultiBodyMeasure 40:Z,3:Z [TwoPatchMeasure (5,12),(6,12)];MeasureSinglePatch 40 X;
ZGate 3;RequestMagicState 41 4;BellBasedCNOT 4 41 42 43 [ExtendSplit (6,14),(5,14);BellPrepare (4,13),(5,13)];
BellBasedCNOT 4 41 42 43 [BellMeasure (5,13),(5,14);MergeContract (4,14),(4,13)];MeasureSinglePatch 41 Z;RequestYState 44 4 [PrepareY (7,14)];
MultiBodyMeasure 44:Z,4:Z [TwoPatchMeasure (7,14),(6,14)];MeasureSinglePatch 44 X;
ZGate 4;RequestMagicState 45 5;BellBasedCNOT 5 45 46 47 [ExtendSplit (8,6),(7,6);BellPrepare (8,5),(7,5)];
BellBasedCNOT 5 45 46 47 [BellMeasure (7,5),(7,6);MergeContract (8,4),(8,5)];MeasureSinglePatch 45 Z;RequestYState 48 5 [PrepareY (9,6)];
MultiBodyMeasure 48:Z,5:Z [TwoPatchMeasure (9,6),(8,6)];MeasureSinglePatch 48 X;
ZGate 5;RequestMagicState 49 6;BellBasedCNOT 6 49 50 51 [ExtendSplit (8,8),(7,8);BellPrepare (4,7),(5,7);BellPrepare (6,7),(7,7)];
BellBasedCNOT 6 49 50 51 [BellMeasure (5,7),(6,7);BellMeasure (7,7),(7,8);MergeContract (4,6),(4,7)];MeasureSinglePatch 49 Z;RequestYState 52 6 [PrepareY (9,8)];
MultiBodyMeasure 52:Z,6:Z [TwoPatchMeasure (9,8),(8,8)];MeasureSinglePatch 52 X;
ZGate 6;RequestMagicState 53 7;BellBasedCNOT 7 53 54 55 [ExtendSplit (8,10),(7,10);BellPrepare (4,11),(5,11);BellPrepare (6,11),(7,11)];
BellBasedCNOT 7 53 54 55 [BellMeasure (5,11),(6,11);BellMeasure (7,11),(7,10);MergeContract (4,10),(4,11)];MeasureSinglePatch 53 Z;RequestYState 56 7 [PrepareY (9,10)];
MultiBodyMeasure 56:Z,7:Z [TwoPatchMeasure (9,10),(8,10)];MeasureSinglePatch 56 X;
ZGate 7;RequestMagicState 57 8;BellBasedCNOT 8 57 58 59 [ExtendSplit (8,12),(7,12);BellPrepare (8,15),(7,15);BellPrepare (7,14),(7,13)];
BellBasedCNOT 8 57 58 59 [BellMeasure (7,15),(7,14);BellMeasure (7,13),(7,12);MergeContract (8,16),(8,15)];MeasureSinglePatch 57 Z;RequestYState 60 8 [PrepareY (9,12)];
MultiBodyMeasure 60:Z,8:Z [TwoPatchMeasure (9,12),(8,12)];MeasureSinglePatch 60 X;
ZGate 8;RequestMagicState 61 9;BellBasedCNOT 9 61 62 63 [ExtendSplit (8,14),(7,14);BellPrepare (4,15),(5,15);BellPrepare (6,15),(7,15)];
BellBasedCNOT 9 61 62 63 [BellMeasure (5,15),(6,15);BellMeasure (7,15),(7,14);MergeContract (4,14),(4,15)];MeasureSinglePatch 61 Z;RequestYState 64 9 [PrepareY (9,14)];
MultiBodyMeasure 64:Z,9:Z [TwoPatchMeasure (9,14),(8,14)];MeasureSinglePatch 64 X;
ZGate 9;RequestMagicState 65 10;BellBasedCNOT 10 65 66 67 [ExtendSplit (10,6),(9,6);BellPrepare (8,5),(9,5)];
BellBasedCNOT 10 65 66 67 [BellMeasure (9,5),(9,6);MergeContract (8,4),(8,5)];MeasureSinglePatch 65 Z;RequestYState 68 10 [PrepareY (11,6)];
MultiBodyMeasure 68:Z,10:Z [TwoPatchMeasure (11,6),(10,6)];MeasureSinglePatch 68 X;
ZGate 10;RequestMagicState 69 11;BellBasedCNOT 11 69 70 71 [ExtendSplit (10,8),(11,8);BellPrepare (12,5),(11,5);BellPrepare (11,6),(11,7)];
BellBasedCNOT 11 69 70 71 [BellMeasure (11,5),(11,6);BellMeasure (11,7),(11,8);MergeContract (12,4),(12,5)];MeasureSinglePatch 69 Z;RequestYState 72 11 [PrepareY (9,8)];
MultiBodyMeasure 72:Z,11:Z [TwoPatchMeasure (9,8),(10,8)];MeasureSinglePatch 72 X;
ZGate 11;RequestMagicState 73 12;BellBasedCNOT 12 73 74 75 [ExtendSplit (10,10),(9,10);BellPrepare (4,9),(5,9);BellPrepare (6,9),(7,9);BellPrepare (8,9),(9,9)];
BellBasedCNOT 12 73 74 75 [BellMeasure (5,9),(6,9);BellMeasure (7,9),(8,9);BellMeasure (9,9),(9,10);MergeContract (4,10),(4,9)];MeasureSinglePatch 73 Z;RequestYState 76 12 [PrepareY (11,10)];
MultiBodyMeasure 76:Z,12:Z [TwoPatchMeasure (11,10),(10,10)];MeasureSinglePatch 76 X;
ZGate 12;RequestMagicState 77 13;BellBasedCNOT 13 77 78 79 [ExtendSplit (10,12),(9,12);BellPrepare (8,15),(9,15);BellPrepare (9,14),(9,13)];
BellBasedCNOT 13 77 78 79 [BellMeasure (9,15),(9,14);BellMeasure (9,13),(9,12);MergeContract (8,16),(8,15)];MeasureSinglePatch 77 Z;RequestYState 80 13 [PrepareY (11,12)];
MultiBodyMeasure 80:Z,13:Z [TwoPatchMeasure (11,12),(10,12)];MeasureSinglePatch 80 X;
ZGate 13;RequestMagicState 81 14;BellBasedCNOT 14 81 82 83 [ExtendSplit (10,14),(11,14);BellPrepare (12,15),(11,15)];
BellBasedCNOT 14 81 82 83 [BellMeasure (11,15),(11,14);MergeContract (12,16),(12,15)];MeasureSinglePatch 81 Z;RequestYState 84 14 [PrepareY (9,14)];
MultiBodyMeasure 84:Z,14:Z [TwoPatchMeasure (9,14),(10,14)];MeasureSinglePatch 84 X;
ZGate 14;RequestMagicState 85 15;BellBasedCNOT 15 85 86 87 [ExtendSplit (12,6),(11,6);BellPrepare (12,5),(11,5)];
BellBasedCNOT 15 85 86 87 [BellMeasure (11,5),(11,6);MergeContract (12,4),(12,5)];MeasureSinglePatch 85 Z;RequestYState 88 15 [PrepareY (13,6)];
MultiBodyMeasure 88:Z,15:Z [TwoPatchMeasure (13,6),(12,6)];MeasureSinglePatch 88 X;
ZGate 15;

