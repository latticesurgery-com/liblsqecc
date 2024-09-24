









RequestMagicState 25 0;BellBasedCNOT 0 25 26 27 [ExtendSplit (2,2),(1,2);BellPrepare (0,3),(1,3)];
BellBasedCNOT 0 25 26 27 [MergeContract (0,2),(0,3);BellMeasure (1,3),(1,2)];MeasureSinglePatch 25 Z;RequestYState 28 0 [PrepareY (3,2)];
MultiBodyMeasure 28:Z,0:Z [TwoPatchMeasure (3,2),(2,2)];MeasureSinglePatch 28 X;
ZGate 0;RequestMagicState 29 1;BellBasedCNOT 1 29 30 31 [ExtendSplit (2,4),(1,4);BellPrepare (0,5),(1,5)];
BellBasedCNOT 1 29 30 31 [MergeContract (0,4),(0,5);BellMeasure (1,5),(1,4)];MeasureSinglePatch 29 Z;RequestYState 32 1 [PrepareY (3,4)];
MultiBodyMeasure 32:Z,1:Z [TwoPatchMeasure (3,4),(2,4)];MeasureSinglePatch 32 X;
ZGate 1;RequestMagicState 33 2;BellBasedCNOT 2 33 34 35 [ExtendSplit (2,6),(1,6);BellPrepare (0,7),(1,7)];
BellBasedCNOT 2 33 34 35 [MergeContract (0,6),(0,7);BellMeasure (1,7),(1,6)];MeasureSinglePatch 33 Z;RequestYState 36 2 [PrepareY (3,6)];
MultiBodyMeasure 36:Z,2:Z [TwoPatchMeasure (3,6),(2,6)];MeasureSinglePatch 36 X;
ZGate 2;RequestMagicState 37 3;BellBasedCNOT 3 37 38 39 [ExtendSplit (2,8),(1,8);BellPrepare (0,9),(1,9)];
BellBasedCNOT 3 37 38 39 [MergeContract (0,8),(0,9);BellMeasure (1,9),(1,8)];MeasureSinglePatch 37 Z;RequestYState 40 3 [PrepareY (3,8)];
MultiBodyMeasure 40:Z,3:Z [TwoPatchMeasure (3,8),(2,8)];MeasureSinglePatch 40 X;
ZGate 3;RequestMagicState 41 4;BellBasedCNOT 4 41 42 43 [ExtendSplit (2,10),(1,10);BellPrepare (0,11),(1,11)];
BellBasedCNOT 4 41 42 43 [MergeContract (0,10),(0,11);BellMeasure (1,11),(1,10)];MeasureSinglePatch 41 Z;RequestYState 44 4 [PrepareY (3,10)];
MultiBodyMeasure 44:Z,4:Z [TwoPatchMeasure (3,10),(2,10)];MeasureSinglePatch 44 X;
ZGate 4;RequestMagicState 45 5;BellBasedCNOT 5 45 46 47 [ExtendSplit (4,2),(3,2);BellPrepare (4,1),(3,1)];
BellBasedCNOT 5 45 46 47 [MergeContract (4,0),(4,1);BellMeasure (3,1),(3,2)];MeasureSinglePatch 45 Z;RequestYState 48 5 [PrepareY (5,2)];
MultiBodyMeasure 48:Z,5:Z [TwoPatchMeasure (5,2),(4,2)];MeasureSinglePatch 48 X;
ZGate 5;RequestMagicState 49 6;BellBasedCNOT 6 49 50 51 [ExtendSplit (4,4),(3,4);BellPrepare (0,5),(1,5);BellPrepare (2,5),(3,5)];
BellBasedCNOT 6 49 50 51 [MergeContract (0,4),(0,5);BellMeasure (1,5),(2,5);BellMeasure (3,5),(3,4)];MeasureSinglePatch 49 Z;RequestYState 52 6 [PrepareY (5,4)];
MultiBodyMeasure 52:Z,6:Z [TwoPatchMeasure (5,4),(4,4)];MeasureSinglePatch 52 X;
ZGate 6;RequestMagicState 53 7;BellBasedCNOT 7 53 54 55 [ExtendSplit (4,6),(3,6);BellPrepare (0,7),(1,7);BellPrepare (2,7),(3,7)];
BellBasedCNOT 7 53 54 55 [MergeContract (0,6),(0,7);BellMeasure (1,7),(2,7);BellMeasure (3,7),(3,6)];MeasureSinglePatch 53 Z;RequestYState 56 7 [PrepareY (5,6)];
MultiBodyMeasure 56:Z,7:Z [TwoPatchMeasure (5,6),(4,6)];MeasureSinglePatch 56 X;
ZGate 7;RequestMagicState 57 8;BellBasedCNOT 8 57 58 59 [ExtendSplit (4,8),(3,8);BellPrepare (0,9),(1,9);BellPrepare (2,9),(3,9)];
BellBasedCNOT 8 57 58 59 [MergeContract (0,8),(0,9);BellMeasure (1,9),(2,9);BellMeasure (3,9),(3,8)];MeasureSinglePatch 57 Z;RequestYState 60 8 [PrepareY (5,8)];
MultiBodyMeasure 60:Z,8:Z [TwoPatchMeasure (5,8),(4,8)];MeasureSinglePatch 60 X;
ZGate 8;RequestMagicState 61 9;BellBasedCNOT 9 61 62 63 [ExtendSplit (4,10),(5,10);BellPrepare (4,11),(5,11)];
BellBasedCNOT 9 61 62 63 [MergeContract (4,12),(4,11);BellMeasure (5,11),(5,10)];MeasureSinglePatch 61 Z;RequestYState 64 9 [PrepareY (3,10)];
MultiBodyMeasure 64:Z,9:Z [TwoPatchMeasure (3,10),(4,10)];MeasureSinglePatch 64 X;
ZGate 9;RequestMagicState 65 10;BellBasedCNOT 10 65 66 67 [ExtendSplit (6,2),(5,2);BellPrepare (6,1),(5,1)];
BellBasedCNOT 10 65 66 67 [MergeContract (6,0),(6,1);BellMeasure (5,1),(5,2)];MeasureSinglePatch 65 Z;RequestYState 68 10 [PrepareY (7,2)];
MultiBodyMeasure 68:Z,10:Z [TwoPatchMeasure (7,2),(6,2)];MeasureSinglePatch 68 X;
ZGate 10;RequestMagicState 69 11;BellBasedCNOT 11 69 70 71 [ExtendSplit (6,4),(5,4);BellPrepare (0,5),(1,5);BellPrepare (2,5),(3,5);BellPrepare (4,5),(5,5)];
BellBasedCNOT 11 69 70 71 [MergeContract (0,4),(0,5);BellMeasure (1,5),(2,5);BellMeasure (3,5),(4,5);BellMeasure (5,5),(5,4)];MeasureSinglePatch 69 Z;RequestYState 72 11 [PrepareY (7,4)];
MultiBodyMeasure 72:Z,11:Z [TwoPatchMeasure (7,4),(6,4)];MeasureSinglePatch 72 X;
ZGate 11;RequestMagicState 73 12;BellBasedCNOT 12 73 74 75 [ExtendSplit (6,6),(5,6);BellPrepare (0,5),(1,5);BellPrepare (2,5),(3,5);BellPrepare (4,5),(5,5)];
BellBasedCNOT 12 73 74 75 [MergeContract (0,6),(0,5);BellMeasure (1,5),(2,5);BellMeasure (3,5),(4,5);BellMeasure (5,5),(5,6)];MeasureSinglePatch 73 Z;RequestYState 76 12 [PrepareY (7,6)];
MultiBodyMeasure 76:Z,12:Z [TwoPatchMeasure (7,6),(6,6)];MeasureSinglePatch 76 X;
ZGate 12;RequestMagicState 77 13;BellBasedCNOT 13 77 78 79 [ExtendSplit (6,8),(5,8);BellPrepare (6,11),(5,11);BellPrepare (5,10),(5,9)];
BellBasedCNOT 13 77 78 79 [MergeContract (6,12),(6,11);BellMeasure (5,11),(5,10);BellMeasure (5,9),(5,8)];MeasureSinglePatch 77 Z;RequestYState 80 13 [PrepareY (7,8)];
MultiBodyMeasure 80:Z,13:Z [TwoPatchMeasure (7,8),(6,8)];MeasureSinglePatch 80 X;
ZGate 13;RequestMagicState 81 14;BellBasedCNOT 14 81 82 83 [ExtendSplit (6,10),(5,10);BellPrepare (4,11),(5,11)];
BellBasedCNOT 14 81 82 83 [MergeContract (4,12),(4,11);BellMeasure (5,11),(5,10)];MeasureSinglePatch 81 Z;RequestYState 84 14 [PrepareY (7,10)];
MultiBodyMeasure 84:Z,14:Z [TwoPatchMeasure (7,10),(6,10)];MeasureSinglePatch 84 X;
ZGate 14;RequestMagicState 85 15;BellBasedCNOT 15 85 86 87 [ExtendSplit (8,2),(7,2);BellPrepare (8,1),(7,1)];
BellBasedCNOT 15 85 86 87 [MergeContract (8,0),(8,1);BellMeasure (7,1),(7,2)];MeasureSinglePatch 85 Z;RequestYState 88 15 [PrepareY (9,2)];
MultiBodyMeasure 88:Z,15:Z [TwoPatchMeasure (9,2),(8,2)];MeasureSinglePatch 88 X;
ZGate 15;

