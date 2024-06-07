









<<<<<<< HEAD
RequestMagicState 25 0;BellBasedCNOT 0 25 26 27 [ExtendSplit (2,2),(2,1);BellPrepare (1,2),(1,1)];
BellBasedCNOT 0 25 26 27 [BellMeasure (1,1),(2,1);MergeContract (0,2),(1,2)];MeasureSinglePatch 25 Z;RequestYState 28 0;
BellBasedCNOT 0 28 29 30 [ExtendSplit (2,2),(2,1);BellPrepare (1,0),(1,1)];
BellBasedCNOT 0 28 29 30 [BellMeasure (1,1),(2,1);MergeContract (0,0),(1,0)];
HGate 28;RotateSingleCellPatch 28;
BusyRegion (0,0),(0,1),StepsToClear(2);
BusyRegion (0,0),(0,1),StepsToClear(1);
BusyRegion (0,0),(0,1),StepsToClear(0);BellBasedCNOT 0 28 31 32 [ExtendSplit (2,2),(2,1);BellPrepare (1,0),(1,1)];
BellBasedCNOT 0 28 31 32 [BellMeasure (1,1),(2,1);MergeContract (0,0),(1,0)];
HGate 28;RotateSingleCellPatch 28;
BusyRegion (0,0),(0,1),StepsToClear(2);
BusyRegion (0,0),(0,1),StepsToClear(1);
BusyRegion (0,0),(0,1),StepsToClear(0);RequestYState 28 0;RequestMagicState 33 1;BellBasedCNOT 1 33 34 35 [ExtendSplit (2,4),(2,3);BellPrepare (1,2),(1,3)];
BellBasedCNOT 1 33 34 35 [BellMeasure (1,3),(2,3);MergeContract (0,2),(1,2)];MeasureSinglePatch 33 Z;RequestYState 36 1;BellBasedCNOT 1 36 37 38 [ExtendSplit (2,4),(2,5);BellPrepare (1,4),(1,5)];
BellBasedCNOT 1 36 37 38 [BellMeasure (1,5),(2,5);MergeContract (0,4),(1,4)];
HGate 36;RotateSingleCellPatch 36;
BusyRegion (0,4),(0,5),StepsToClear(2);
BusyRegion (0,4),(0,5),StepsToClear(1);
BusyRegion (0,4),(0,5),StepsToClear(0);BellBasedCNOT 1 36 39 40 [ExtendSplit (2,4),(2,3);BellPrepare (1,4),(1,3)];
BellBasedCNOT 1 36 39 40 [BellMeasure (1,3),(2,3);MergeContract (0,4),(1,4)];
HGate 36;RotateSingleCellPatch 36;
BusyRegion (0,4),(0,5),StepsToClear(2);
BusyRegion (0,4),(0,5),StepsToClear(1);
BusyRegion (0,4),(0,5),StepsToClear(0);RequestYState 36 1;RequestMagicState 41 2;BellBasedCNOT 2 41 42 43 [ExtendSplit (2,6),(2,5);BellPrepare (1,6),(1,5)];
BellBasedCNOT 2 41 42 43 [BellMeasure (1,5),(2,5);MergeContract (0,6),(1,6)];MeasureSinglePatch 41 Z;RequestYState 44 2;BellBasedCNOT 2 44 45 46 [ExtendSplit (2,6),(2,7);BellPrepare (1,4),(1,3);BellPrepare (2,3),(3,3);BellPrepare (3,4),(3,5);BellPrepare (3,6),(3,7)];
BellBasedCNOT 2 44 45 46 [BellMeasure (1,3),(2,3);BellMeasure (3,3),(3,4);BellMeasure (3,5),(3,6);BellMeasure (3,7),(2,7);MergeContract (0,4),(1,4)];
HGate 44;RotateSingleCellPatch 44;
BusyRegion (0,4),(0,5),StepsToClear(2);
BusyRegion (0,4),(0,5),StepsToClear(1);
BusyRegion (0,4),(0,5),StepsToClear(0);BellBasedCNOT 2 44 47 48 [ExtendSplit (2,6),(2,5);BellPrepare (1,4),(1,5)];
BellBasedCNOT 2 44 47 48 [BellMeasure (1,5),(2,5);MergeContract (0,4),(1,4)];
HGate 44;RotateSingleCellPatch 44;
BusyRegion (0,4),(0,5),StepsToClear(2);
BusyRegion (0,4),(0,5),StepsToClear(1);
BusyRegion (0,4),(0,5),StepsToClear(0);RequestYState 44 2;RequestMagicState 49 3;BellBasedCNOT 3 49 50 51 [ExtendSplit (2,8),(2,9);BellPrepare (1,10),(1,9)];
BellBasedCNOT 3 49 50 51 [BellMeasure (1,9),(2,9);MergeContract (0,10),(1,10)];MeasureSinglePatch 49 Z;RequestYState 52 3;BellBasedCNOT 3 52 53 54 [ExtendSplit (2,8),(2,7);BellPrepare (1,8),(1,7)];
BellBasedCNOT 3 52 53 54 [BellMeasure (1,7),(2,7);MergeContract (0,8),(1,8)];
HGate 52;RotateSingleCellPatch 52;
BusyRegion (0,8),(0,9),StepsToClear(2);
BusyRegion (0,8),(0,9),StepsToClear(1);
BusyRegion (0,8),(0,9),StepsToClear(0);BellBasedCNOT 3 52 55 56 [ExtendSplit (2,8),(2,7);BellPrepare (1,8),(1,7)];
BellBasedCNOT 3 52 55 56 [BellMeasure (1,7),(2,7);MergeContract (0,8),(1,8)];
HGate 52;RotateSingleCellPatch 52;
BusyRegion (0,8),(0,9),StepsToClear(2);
BusyRegion (0,8),(0,9),StepsToClear(1);
BusyRegion (0,8),(0,9),StepsToClear(0);RequestYState 52 3;RequestMagicState 57 4;BellBasedCNOT 4 57 58 59 [ExtendSplit (2,10),(2,11);BellPrepare (3,12),(3,11)];
BellBasedCNOT 4 57 58 59 [BellMeasure (3,11),(2,11);MergeContract (2,12),(3,12)];MeasureSinglePatch 57 Z;RequestYState 60 4;BellBasedCNOT 4 60 61 62 [ExtendSplit (2,10),(2,9);BellPrepare (1,8),(1,9)];
BellBasedCNOT 4 60 61 62 [BellMeasure (1,9),(2,9);MergeContract (0,8),(1,8)];
HGate 60;RotateSingleCellPatch 60;
BusyRegion (0,8),(0,9),StepsToClear(2);
BusyRegion (0,8),(0,9),StepsToClear(1);
BusyRegion (0,8),(0,9),StepsToClear(0);BellBasedCNOT 4 60 63 64 [ExtendSplit (2,10),(2,9);BellPrepare (1,8),(1,9)];
BellBasedCNOT 4 60 63 64 [BellMeasure (1,9),(2,9);MergeContract (0,8),(1,8)];
HGate 60;RotateSingleCellPatch 60;
BusyRegion (0,8),(0,9),StepsToClear(2);
BusyRegion (0,8),(0,9),StepsToClear(1);
BusyRegion (0,8),(0,9),StepsToClear(0);RequestYState 60 4;RequestMagicState 65 5;BellBasedCNOT 5 65 66 67 [ExtendSplit (4,2),(4,1);BellPrepare (1,2),(1,1);BellPrepare (2,1),(3,1)];
BellBasedCNOT 5 65 66 67 [BellMeasure (1,1),(2,1);BellMeasure (3,1),(4,1);MergeContract (0,2),(1,2)];MeasureSinglePatch 65 Z;RequestYState 68 5;BellBasedCNOT 5 68 69 70 [ExtendSplit (4,2),(4,3);BellPrepare (5,0),(5,1);BellPrepare (5,2),(5,3)];
BellBasedCNOT 5 68 69 70 [BellMeasure (5,1),(5,2);BellMeasure (5,3),(4,3);MergeContract (4,0),(5,0)];
=======
RequestMagicState 25 0;BellBasedCNOT 0 25 26 27 [ExtendSplit (6,6),(5,6);BellPrepare (4,7),(5,7)];
BellBasedCNOT 0 25 26 27 [BellMeasure (5,7),(5,6);MergeContract (4,6),(4,7)];MeasureSinglePatch 25 Z;RequestYState 28 0;BellBasedCNOT 0 28 29 30 [ExtendSplit (6,6),(7,6);BellPrepare (4,5),(5,5);BellPrepare (6,5),(7,5)];
BellBasedCNOT 0 28 29 30 [BellMeasure (5,5),(6,5);BellMeasure (7,5),(7,6);MergeContract (4,4),(4,5)];
HGate 28;RotateSingleCellPatch 28;
BusyRegion (4,4),(4,5),StepsToClear(2);
BusyRegion (4,4),(4,5),StepsToClear(1);
BusyRegion (4,4),(4,5),StepsToClear(0);BellBasedCNOT 0 28 31 32 [ExtendSplit (6,6),(5,6);BellPrepare (4,5),(5,5)];
BellBasedCNOT 0 28 31 32 [BellMeasure (5,5),(5,6);MergeContract (4,4),(4,5)];
HGate 28;RotateSingleCellPatch 28;
BusyRegion (4,4),(4,5),StepsToClear(2);
BusyRegion (4,4),(4,5),StepsToClear(1);
BusyRegion (4,4),(4,5),StepsToClear(0);RequestYState 28 0;RequestMagicState 33 1;BellBasedCNOT 1 33 34 35 [ExtendSplit (6,8),(5,8);BellPrepare (4,7),(5,7)];
BellBasedCNOT 1 33 34 35 [BellMeasure (5,7),(5,8);MergeContract (4,6),(4,7)];MeasureSinglePatch 33 Z;RequestYState 36 1;BellBasedCNOT 1 36 37 38 [ExtendSplit (6,8),(7,8);BellPrepare (4,9),(5,9);BellPrepare (6,9),(7,9)];
BellBasedCNOT 1 36 37 38 [BellMeasure (5,9),(6,9);BellMeasure (7,9),(7,8);MergeContract (4,8),(4,9)];
HGate 36;RotateSingleCellPatch 36;
BusyRegion (4,8),(4,9),StepsToClear(2);
BusyRegion (4,8),(4,9),StepsToClear(1);
BusyRegion (4,8),(4,9),StepsToClear(0);BellBasedCNOT 1 36 39 40 [ExtendSplit (6,8),(5,8);BellPrepare (4,9),(5,9)];
BellBasedCNOT 1 36 39 40 [BellMeasure (5,9),(5,8);MergeContract (4,8),(4,9)];
HGate 36;RotateSingleCellPatch 36;
BusyRegion (4,8),(4,9),StepsToClear(2);
BusyRegion (4,8),(4,9),StepsToClear(1);
BusyRegion (4,8),(4,9),StepsToClear(0);RequestYState 36 1;RequestMagicState 41 2;BellBasedCNOT 2 41 42 43 [ExtendSplit (6,10),(5,10);BellPrepare (4,11),(5,11)];
BellBasedCNOT 2 41 42 43 [BellMeasure (5,11),(5,10);MergeContract (4,10),(4,11)];MeasureSinglePatch 41 Z;RequestYState 44 2;BellBasedCNOT 2 44 45 46 [ExtendSplit (6,10),(7,10);BellPrepare (4,9),(5,9);BellPrepare (6,9),(7,9)];
BellBasedCNOT 2 44 45 46 [BellMeasure (5,9),(6,9);BellMeasure (7,9),(7,10);MergeContract (4,8),(4,9)];
HGate 44;RotateSingleCellPatch 44;
BusyRegion (4,8),(4,9),StepsToClear(2);
BusyRegion (4,8),(4,9),StepsToClear(1);
BusyRegion (4,8),(4,9),StepsToClear(0);BellBasedCNOT 2 44 47 48 [ExtendSplit (6,10),(5,10);BellPrepare (4,9),(5,9)];
BellBasedCNOT 2 44 47 48 [BellMeasure (5,9),(5,10);MergeContract (4,8),(4,9)];
HGate 44;RotateSingleCellPatch 44;
BusyRegion (4,8),(4,9),StepsToClear(2);
BusyRegion (4,8),(4,9),StepsToClear(1);
BusyRegion (4,8),(4,9),StepsToClear(0);RequestYState 44 2;RequestMagicState 49 3;BellBasedCNOT 3 49 50 51 [ExtendSplit (6,12),(5,12);BellPrepare (4,11),(5,11)];
BellBasedCNOT 3 49 50 51 [BellMeasure (5,11),(5,12);MergeContract (4,10),(4,11)];MeasureSinglePatch 49 Z;RequestYState 52 3;BellBasedCNOT 3 52 53 54 [ExtendSplit (6,12),(7,12);BellPrepare (4,13),(5,13);BellPrepare (6,13),(7,13)];
BellBasedCNOT 3 52 53 54 [BellMeasure (5,13),(6,13);BellMeasure (7,13),(7,12);MergeContract (4,12),(4,13)];
HGate 52;RotateSingleCellPatch 52;
BusyRegion (4,12),(4,13),StepsToClear(2);
BusyRegion (4,12),(4,13),StepsToClear(1);
BusyRegion (4,12),(4,13),StepsToClear(0);BellBasedCNOT 3 52 55 56 [ExtendSplit (6,12),(5,12);BellPrepare (4,13),(5,13)];
BellBasedCNOT 3 52 55 56 [BellMeasure (5,13),(5,12);MergeContract (4,12),(4,13)];
HGate 52;RotateSingleCellPatch 52;
BusyRegion (4,12),(4,13),StepsToClear(2);
BusyRegion (4,12),(4,13),StepsToClear(1);
BusyRegion (4,12),(4,13),StepsToClear(0);RequestYState 52 3;RequestMagicState 57 4;BellBasedCNOT 4 57 58 59 [ExtendSplit (6,14),(5,14);BellPrepare (4,15),(5,15)];
BellBasedCNOT 4 57 58 59 [BellMeasure (5,15),(5,14);MergeContract (4,14),(4,15)];MeasureSinglePatch 57 Z;RequestYState 60 4;BellBasedCNOT 4 60 61 62 [ExtendSplit (6,14),(7,14);BellPrepare (4,13),(5,13);BellPrepare (6,13),(7,13)];
BellBasedCNOT 4 60 61 62 [BellMeasure (5,13),(6,13);BellMeasure (7,13),(7,14);MergeContract (4,12),(4,13)];
HGate 60;RotateSingleCellPatch 60;
BusyRegion (4,12),(4,13),StepsToClear(2);
BusyRegion (4,12),(4,13),StepsToClear(1);
BusyRegion (4,12),(4,13),StepsToClear(0);BellBasedCNOT 4 60 63 64 [ExtendSplit (6,14),(5,14);BellPrepare (4,13),(5,13)];
BellBasedCNOT 4 60 63 64 [BellMeasure (5,13),(5,14);MergeContract (4,12),(4,13)];
HGate 60;RotateSingleCellPatch 60;
BusyRegion (4,12),(4,13),StepsToClear(2);
BusyRegion (4,12),(4,13),StepsToClear(1);
BusyRegion (4,12),(4,13),StepsToClear(0);RequestYState 60 4;RequestMagicState 65 5;BellBasedCNOT 5 65 66 67 [ExtendSplit (8,6),(7,6);BellPrepare (4,7),(5,7);BellPrepare (6,7),(7,7)];
BellBasedCNOT 5 65 66 67 [BellMeasure (5,7),(6,7);BellMeasure (7,7),(7,6);MergeContract (4,6),(4,7)];MeasureSinglePatch 65 Z;RequestYState 68 5;BellBasedCNOT 5 68 69 70 [ExtendSplit (8,6),(9,6);BellPrepare (8,5),(9,5)];
BellBasedCNOT 5 68 69 70 [BellMeasure (9,5),(9,6);MergeContract (8,4),(8,5)];
>>>>>>> 7db7bb4 (Fixed test cases.)
HGate 68;RotateSingleCellPatch 68;
<<<<<<< HEAD
BusyRegion (4,0),(4,1),StepsToClear(2);
BusyRegion (4,0),(4,1),StepsToClear(1);
BusyRegion (4,0),(4,1),StepsToClear(0);BellBasedCNOT 5 68 71 72 [ExtendSplit (4,2),(4,1);BellPrepare (5,0),(5,1)];
BellBasedCNOT 5 68 71 72 [BellMeasure (5,1),(4,1);MergeContract (4,0),(5,0)];
=======
BusyRegion (8,4),(8,5),StepsToClear(2);
BusyRegion (8,4),(8,5),StepsToClear(1);
<<<<<<< HEAD
BusyRegion (8,4),(8,5),StepsToClear(0);BellBasedCNOT 5 68 71 72 [ExtendSplit (8,6),(8,5);BellPrepare (7,4),(7,5)];
BellBasedCNOT 5 68 71 72 [BellMeasure (7,5),(8,5);MergeContract (8,4),(7,4)];
>>>>>>> 4dcc26c (Added twist-based Y state preparation on one tile and updated S gate teleportation protocol.)
HGate 68;RotateSingleCellPatch 68;
BusyRegion (4,0),(4,1),StepsToClear(2);
BusyRegion (4,0),(4,1),StepsToClear(1);
BusyRegion (4,0),(4,1),StepsToClear(0);RequestYState 68 5;RequestMagicState 73 6;BellBasedCNOT 6 73 74 75 [ExtendSplit (4,4),(4,5);BellPrepare (1,6),(1,5);BellPrepare (2,5),(3,5)];
BellBasedCNOT 6 73 74 75 [BellMeasure (1,5),(2,5);BellMeasure (3,5),(4,5);MergeContract (0,6),(1,6)];MeasureSinglePatch 73 Z;RequestYState 76 6;BellBasedCNOT 6 76 77 78 [ExtendSplit (4,4),(4,3);BellPrepare (1,4),(1,3);BellPrepare (2,3),(3,3)];
BellBasedCNOT 6 76 77 78 [BellMeasure (1,3),(2,3);BellMeasure (3,3),(4,3);MergeContract (0,4),(1,4)];
HGate 76;RotateSingleCellPatch 76;
BusyRegion (0,4),(0,5),StepsToClear(2);
BusyRegion (0,4),(0,5),StepsToClear(1);
BusyRegion (0,4),(0,5),StepsToClear(0);BellBasedCNOT 6 76 79 80 [ExtendSplit (4,4),(4,3);BellPrepare (1,4),(1,3);BellPrepare (2,3),(3,3)];
BellBasedCNOT 6 76 79 80 [BellMeasure (1,3),(2,3);BellMeasure (3,3),(4,3);MergeContract (0,4),(1,4)];
HGate 76;RotateSingleCellPatch 76;
BusyRegion (0,4),(0,5),StepsToClear(2);
BusyRegion (0,4),(0,5),StepsToClear(1);
BusyRegion (0,4),(0,5),StepsToClear(0);RequestYState 76 6;RequestMagicState 81 7;BellBasedCNOT 7 81 82 83 [ExtendSplit (4,6),(4,5);BellPrepare (1,2),(1,3);BellPrepare (2,3),(3,3);BellPrepare (3,4),(3,5)];
BellBasedCNOT 7 81 82 83 [BellMeasure (1,3),(2,3);BellMeasure (3,3),(3,4);BellMeasure (3,5),(4,5);MergeContract (0,2),(1,2)];MeasureSinglePatch 81 Z;RequestYState 84 7;BellBasedCNOT 7 84 85 86 [ExtendSplit (4,6),(4,7);BellPrepare (1,4),(1,5);BellPrepare (1,6),(1,7);BellPrepare (2,7),(3,7)];
BellBasedCNOT 7 84 85 86 [BellMeasure (1,5),(1,6);BellMeasure (1,7),(2,7);BellMeasure (3,7),(4,7);MergeContract (0,4),(1,4)];
HGate 84;RotateSingleCellPatch 84;
BusyRegion (0,4),(0,5),StepsToClear(2);
BusyRegion (0,4),(0,5),StepsToClear(1);
BusyRegion (0,4),(0,5),StepsToClear(0);BellBasedCNOT 7 84 87 88 [ExtendSplit (4,6),(4,5);BellPrepare (1,4),(1,5);BellPrepare (2,5),(3,5)];
BellBasedCNOT 7 84 87 88 [BellMeasure (1,5),(2,5);BellMeasure (3,5),(4,5);MergeContract (0,4),(1,4)];
HGate 84;RotateSingleCellPatch 84;
BusyRegion (0,4),(0,5),StepsToClear(2);
BusyRegion (0,4),(0,5),StepsToClear(1);
BusyRegion (0,4),(0,5),StepsToClear(0);RequestYState 84 7;RequestMagicState 89 8;BellBasedCNOT 8 89 90 91 [ExtendSplit (4,8),(4,7);BellPrepare (1,6),(1,7);BellPrepare (2,7),(3,7)];
BellBasedCNOT 8 89 90 91 [BellMeasure (1,7),(2,7);BellMeasure (3,7),(4,7);MergeContract (0,6),(1,6)];MeasureSinglePatch 89 Z;RequestYState 92 8;BellBasedCNOT 8 92 93 94 [ExtendSplit (4,8),(4,9);BellPrepare (1,8),(1,9);BellPrepare (2,9),(3,9)];
BellBasedCNOT 8 92 93 94 [BellMeasure (1,9),(2,9);BellMeasure (3,9),(4,9);MergeContract (0,8),(1,8)];
HGate 92;RotateSingleCellPatch 92;
BusyRegion (0,8),(0,9),StepsToClear(2);
BusyRegion (0,8),(0,9),StepsToClear(1);
BusyRegion (0,8),(0,9),StepsToClear(0);BellBasedCNOT 8 92 95 96 [ExtendSplit (4,8),(4,7);BellPrepare (1,8),(1,7);BellPrepare (2,7),(3,7)];
BellBasedCNOT 8 92 95 96 [BellMeasure (1,7),(2,7);BellMeasure (3,7),(4,7);MergeContract (0,8),(1,8)];
HGate 92;RotateSingleCellPatch 92;
BusyRegion (0,8),(0,9),StepsToClear(2);
BusyRegion (0,8),(0,9),StepsToClear(1);
BusyRegion (0,8),(0,9),StepsToClear(0);RequestYState 92 8;RequestMagicState 97 9;BellBasedCNOT 9 97 98 99 [ExtendSplit (4,10),(4,9);BellPrepare (1,10),(1,9);BellPrepare (2,9),(3,9)];
BellBasedCNOT 9 97 98 99 [BellMeasure (1,9),(2,9);BellMeasure (3,9),(4,9);MergeContract (0,10),(1,10)];MeasureSinglePatch 97 Z;RequestYState 100 9;BellBasedCNOT 9 100 101 102 [ExtendSplit (4,10),(4,11);BellPrepare (3,12),(3,11)];
BellBasedCNOT 9 100 101 102 [BellMeasure (3,11),(4,11);MergeContract (4,12),(3,12)];
HGate 100;RotateSingleCellPatch 100;
BusyRegion (4,12),(4,11),StepsToClear(2);
BusyRegion (4,12),(4,11),StepsToClear(1);
BusyRegion (4,12),(4,11),StepsToClear(0);BellBasedCNOT 9 100 103 104 [ExtendSplit (4,10),(4,11);BellPrepare (5,12),(5,11)];
BellBasedCNOT 9 100 103 104 [BellMeasure (5,11),(4,11);MergeContract (4,12),(5,12)];
=======
BusyRegion (8,4),(8,5),StepsToClear(0);BellBasedCNOT 5 68 71 72 [ExtendSplit (8,6),(7,6);BellPrepare (8,5),(7,5)];
BellBasedCNOT 5 68 71 72 [BellMeasure (7,5),(7,6);MergeContract (8,4),(8,5)];
HGate 68;RotateSingleCellPatch 68;
BusyRegion (8,4),(8,5),StepsToClear(2);
BusyRegion (8,4),(8,5),StepsToClear(1);
BusyRegion (8,4),(8,5),StepsToClear(0);RequestYState 68 5;RequestMagicState 73 6;BellBasedCNOT 6 73 74 75 [ExtendSplit (8,8),(7,8);BellPrepare (4,7),(5,7);BellPrepare (6,7),(7,7)];
BellBasedCNOT 6 73 74 75 [BellMeasure (5,7),(6,7);BellMeasure (7,7),(7,8);MergeContract (4,6),(4,7)];MeasureSinglePatch 73 Z;RequestYState 76 6;BellBasedCNOT 6 76 77 78 [ExtendSplit (8,8),(9,8);BellPrepare (4,9),(5,9);BellPrepare (6,9),(7,9);BellPrepare (8,9),(9,9)];
BellBasedCNOT 6 76 77 78 [BellMeasure (5,9),(6,9);BellMeasure (7,9),(8,9);BellMeasure (9,9),(9,8);MergeContract (4,8),(4,9)];
HGate 76;RotateSingleCellPatch 76;
BusyRegion (4,8),(4,9),StepsToClear(2);
BusyRegion (4,8),(4,9),StepsToClear(1);
BusyRegion (4,8),(4,9),StepsToClear(0);BellBasedCNOT 6 76 79 80 [ExtendSplit (8,8),(7,8);BellPrepare (4,9),(5,9);BellPrepare (6,9),(7,9)];
BellBasedCNOT 6 76 79 80 [BellMeasure (5,9),(6,9);BellMeasure (7,9),(7,8);MergeContract (4,8),(4,9)];
HGate 76;RotateSingleCellPatch 76;
BusyRegion (4,8),(4,9),StepsToClear(2);
BusyRegion (4,8),(4,9),StepsToClear(1);
BusyRegion (4,8),(4,9),StepsToClear(0);RequestYState 76 6;RequestMagicState 81 7;BellBasedCNOT 7 81 82 83 [ExtendSplit (8,10),(7,10);BellPrepare (4,11),(5,11);BellPrepare (6,11),(7,11)];
BellBasedCNOT 7 81 82 83 [BellMeasure (5,11),(6,11);BellMeasure (7,11),(7,10);MergeContract (4,10),(4,11)];MeasureSinglePatch 81 Z;RequestYState 84 7;BellBasedCNOT 7 84 85 86 [ExtendSplit (8,10),(9,10);BellPrepare (4,9),(5,9);BellPrepare (6,9),(7,9);BellPrepare (8,9),(9,9)];
BellBasedCNOT 7 84 85 86 [BellMeasure (5,9),(6,9);BellMeasure (7,9),(8,9);BellMeasure (9,9),(9,10);MergeContract (4,8),(4,9)];
HGate 84;RotateSingleCellPatch 84;
BusyRegion (4,8),(4,9),StepsToClear(2);
BusyRegion (4,8),(4,9),StepsToClear(1);
BusyRegion (4,8),(4,9),StepsToClear(0);BellBasedCNOT 7 84 87 88 [ExtendSplit (8,10),(7,10);BellPrepare (4,9),(5,9);BellPrepare (6,9),(7,9)];
BellBasedCNOT 7 84 87 88 [BellMeasure (5,9),(6,9);BellMeasure (7,9),(7,10);MergeContract (4,8),(4,9)];
HGate 84;RotateSingleCellPatch 84;
BusyRegion (4,8),(4,9),StepsToClear(2);
BusyRegion (4,8),(4,9),StepsToClear(1);
BusyRegion (4,8),(4,9),StepsToClear(0);RequestYState 84 7;RequestMagicState 89 8;BellBasedCNOT 8 89 90 91 [ExtendSplit (8,12),(7,12);BellPrepare (4,11),(5,11);BellPrepare (6,11),(7,11)];
BellBasedCNOT 8 89 90 91 [BellMeasure (5,11),(6,11);BellMeasure (7,11),(7,12);MergeContract (4,10),(4,11)];MeasureSinglePatch 89 Z;RequestYState 92 8;BellBasedCNOT 8 92 93 94 [ExtendSplit (8,12),(9,12);BellPrepare (4,13),(5,13);BellPrepare (6,13),(7,13);BellPrepare (8,13),(9,13)];
BellBasedCNOT 8 92 93 94 [BellMeasure (5,13),(6,13);BellMeasure (7,13),(8,13);BellMeasure (9,13),(9,12);MergeContract (4,12),(4,13)];
HGate 92;RotateSingleCellPatch 92;
BusyRegion (4,12),(4,13),StepsToClear(2);
BusyRegion (4,12),(4,13),StepsToClear(1);
BusyRegion (4,12),(4,13),StepsToClear(0);BellBasedCNOT 8 92 95 96 [ExtendSplit (8,12),(7,12);BellPrepare (4,13),(5,13);BellPrepare (6,13),(7,13)];
BellBasedCNOT 8 92 95 96 [BellMeasure (5,13),(6,13);BellMeasure (7,13),(7,12);MergeContract (4,12),(4,13)];
HGate 92;RotateSingleCellPatch 92;
BusyRegion (4,12),(4,13),StepsToClear(2);
BusyRegion (4,12),(4,13),StepsToClear(1);
BusyRegion (4,12),(4,13),StepsToClear(0);RequestYState 92 8;RequestMagicState 97 9;BellBasedCNOT 9 97 98 99 [ExtendSplit (8,14),(7,14);BellPrepare (4,15),(5,15);BellPrepare (6,15),(7,15)];
BellBasedCNOT 9 97 98 99 [BellMeasure (5,15),(6,15);BellMeasure (7,15),(7,14);MergeContract (4,14),(4,15)];MeasureSinglePatch 97 Z;RequestYState 100 9;BellBasedCNOT 9 100 101 102 [ExtendSplit (8,14),(9,14);BellPrepare (8,15),(9,15)];
BellBasedCNOT 9 100 101 102 [BellMeasure (9,15),(9,14);MergeContract (8,16),(8,15)];
HGate 100;RotateSingleCellPatch 100;
BusyRegion (8,16),(8,15),StepsToClear(2);
BusyRegion (8,16),(8,15),StepsToClear(1);
BusyRegion (8,16),(8,15),StepsToClear(0);BellBasedCNOT 9 100 103 104 [ExtendSplit (8,14),(9,14);BellPrepare (8,15),(9,15)];
BellBasedCNOT 9 100 103 104 [BellMeasure (9,15),(9,14);MergeContract (8,16),(8,15)];
>>>>>>> 7db7bb4 (Fixed test cases.)
HGate 100;RotateSingleCellPatch 100;
<<<<<<< HEAD
BusyRegion (4,12),(4,11),StepsToClear(2);
BusyRegion (4,12),(4,11),StepsToClear(1);
BusyRegion (4,12),(4,11),StepsToClear(0);RequestYState 100 9;RequestMagicState 105 10;BellBasedCNOT 10 105 106 107 [ExtendSplit (6,2),(6,1);BellPrepare (7,0),(7,1)];
BellBasedCNOT 10 105 106 107 [BellMeasure (7,1),(6,1);MergeContract (6,0),(7,0)];MeasureSinglePatch 105 Z;RequestYState 108 10;BellBasedCNOT 10 108 109 110 [ExtendSplit (6,2),(6,3);BellPrepare (5,0),(5,1);BellPrepare (5,2),(5,3)];
BellBasedCNOT 10 108 109 110 [BellMeasure (5,1),(5,2);BellMeasure (5,3),(6,3);MergeContract (4,0),(5,0)];
=======
BusyRegion (8,16),(8,15),StepsToClear(2);
BusyRegion (8,16),(8,15),StepsToClear(1);
<<<<<<< HEAD
BusyRegion (8,16),(8,15),StepsToClear(0);RequestYState 100 9;RequestMagicState 105 10;BellBasedCNOT 10 105 106 107 [ExtendSplit (10,6),(10,5);BellPrepare (9,4),(9,5)];
BellBasedCNOT 10 105 106 107 [BellMeasure (9,5),(10,5);MergeContract (10,4),(9,4)];MeasureSinglePatch 105 Z;RequestYState 108 10;BellBasedCNOT 10 108 109 110 [ExtendSplit (10,6),(10,7);BellPrepare (7,4),(7,5);BellPrepare (7,6),(7,7);BellPrepare (8,7),(9,7)];
BellBasedCNOT 10 108 109 110 [BellMeasure (7,5),(7,6);BellMeasure (7,7),(8,7);BellMeasure (9,7),(10,7);MergeContract (8,4),(7,4)];
>>>>>>> 4dcc26c (Added twist-based Y state preparation on one tile and updated S gate teleportation protocol.)
HGate 108;RotateSingleCellPatch 108;
BusyRegion (4,0),(4,1),StepsToClear(2);
BusyRegion (4,0),(4,1),StepsToClear(1);
BusyRegion (4,0),(4,1),StepsToClear(0);BellBasedCNOT 10 108 111 112 [ExtendSplit (6,2),(6,1);BellPrepare (5,0),(5,1)];
BellBasedCNOT 10 108 111 112 [BellMeasure (5,1),(6,1);MergeContract (4,0),(5,0)];
=======
BusyRegion (8,16),(8,15),StepsToClear(0);RequestYState 100 9;RequestMagicState 105 10;BellBasedCNOT 10 105 106 107 [ExtendSplit (10,6),(9,6);BellPrepare (10,5),(9,5)];
BellBasedCNOT 10 105 106 107 [BellMeasure (9,5),(9,6);MergeContract (10,4),(10,5)];MeasureSinglePatch 105 Z;RequestYState 108 10;BellBasedCNOT 10 108 109 110 [ExtendSplit (10,6),(11,6);BellPrepare (8,5),(7,5);BellPrepare (7,6),(7,7);BellPrepare (8,7),(9,7);BellPrepare (10,7),(11,7)];
BellBasedCNOT 10 108 109 110 [BellMeasure (7,5),(7,6);BellMeasure (7,7),(8,7);BellMeasure (9,7),(10,7);BellMeasure (11,7),(11,6);MergeContract (8,4),(8,5)];
HGate 108;RotateSingleCellPatch 108;
BusyRegion (8,4),(8,5),StepsToClear(2);
BusyRegion (8,4),(8,5),StepsToClear(1);
BusyRegion (8,4),(8,5),StepsToClear(0);BellBasedCNOT 10 108 111 112 [ExtendSplit (10,6),(9,6);BellPrepare (8,5),(9,5)];
BellBasedCNOT 10 108 111 112 [BellMeasure (9,5),(9,6);MergeContract (8,4),(8,5)];
>>>>>>> 7db7bb4 (Fixed test cases.)
HGate 108;RotateSingleCellPatch 108;
<<<<<<< HEAD
BusyRegion (4,0),(4,1),StepsToClear(2);
BusyRegion (4,0),(4,1),StepsToClear(1);
BusyRegion (4,0),(4,1),StepsToClear(0);RequestYState 108 10;RequestMagicState 113 11;BellBasedCNOT 11 113 114 115 [ExtendSplit (6,4),(6,3);BellPrepare (1,2),(1,3);BellPrepare (2,3),(3,3);BellPrepare (4,3),(5,3)];
BellBasedCNOT 11 113 114 115 [BellMeasure (1,3),(2,3);BellMeasure (3,3),(4,3);BellMeasure (5,3),(6,3);MergeContract (0,2),(1,2)];MeasureSinglePatch 113 Z;RequestYState 116 11;BellBasedCNOT 11 116 117 118 [ExtendSplit (6,4),(6,5);BellPrepare (1,4),(1,5);BellPrepare (2,5),(3,5);BellPrepare (4,5),(5,5)];
BellBasedCNOT 11 116 117 118 [BellMeasure (1,5),(2,5);BellMeasure (3,5),(4,5);BellMeasure (5,5),(6,5);MergeContract (0,4),(1,4)];
=======
BusyRegion (8,4),(8,5),StepsToClear(2);
BusyRegion (8,4),(8,5),StepsToClear(1);
<<<<<<< HEAD
BusyRegion (8,4),(8,5),StepsToClear(0);RequestYState 108 10;RequestMagicState 113 11;BellBasedCNOT 11 113 114 115 [ExtendSplit (10,8),(10,7);BellPrepare (9,4),(9,5);BellPrepare (9,6),(9,7)];
BellBasedCNOT 11 113 114 115 [BellMeasure (9,5),(9,6);BellMeasure (9,7),(10,7);MergeContract (10,4),(9,4)];MeasureSinglePatch 113 Z;RequestYState 116 11;BellBasedCNOT 11 116 117 118 [ExtendSplit (10,8),(10,9);BellPrepare (5,8),(5,9);BellPrepare (6,9),(7,9);BellPrepare (8,9),(9,9)];
BellBasedCNOT 11 116 117 118 [BellMeasure (5,9),(6,9);BellMeasure (7,9),(8,9);BellMeasure (9,9),(10,9);MergeContract (4,8),(5,8)];
>>>>>>> 4dcc26c (Added twist-based Y state preparation on one tile and updated S gate teleportation protocol.)
HGate 116;RotateSingleCellPatch 116;
BusyRegion (0,4),(0,5),StepsToClear(2);
BusyRegion (0,4),(0,5),StepsToClear(1);
BusyRegion (0,4),(0,5),StepsToClear(0);BellBasedCNOT 11 116 119 120 [ExtendSplit (6,4),(6,3);BellPrepare (1,4),(1,3);BellPrepare (2,3),(3,3);BellPrepare (4,3),(5,3)];
BellBasedCNOT 11 116 119 120 [BellMeasure (1,3),(2,3);BellMeasure (3,3),(4,3);BellMeasure (5,3),(6,3);MergeContract (0,4),(1,4)];
=======
BusyRegion (8,4),(8,5),StepsToClear(0);RequestYState 108 10;RequestMagicState 113 11;BellBasedCNOT 11 113 114 115 [ExtendSplit (10,8),(9,8);BellPrepare (10,5),(9,5);BellPrepare (9,6),(9,7)];
BellBasedCNOT 11 113 114 115 [BellMeasure (9,5),(9,6);BellMeasure (9,7),(9,8);MergeContract (10,4),(10,5)];MeasureSinglePatch 113 Z;RequestYState 116 11;BellBasedCNOT 11 116 117 118 [ExtendSplit (10,8),(11,8);BellPrepare (4,9),(5,9);BellPrepare (6,9),(7,9);BellPrepare (8,9),(9,9);BellPrepare (10,9),(11,9)];
BellBasedCNOT 11 116 117 118 [BellMeasure (5,9),(6,9);BellMeasure (7,9),(8,9);BellMeasure (9,9),(10,9);BellMeasure (11,9),(11,8);MergeContract (4,8),(4,9)];
HGate 116;RotateSingleCellPatch 116;
BusyRegion (4,8),(4,9),StepsToClear(2);
BusyRegion (4,8),(4,9),StepsToClear(1);
BusyRegion (4,8),(4,9),StepsToClear(0);BellBasedCNOT 11 116 119 120 [ExtendSplit (10,8),(9,8);BellPrepare (4,9),(5,9);BellPrepare (6,9),(7,9);BellPrepare (8,9),(9,9)];
BellBasedCNOT 11 116 119 120 [BellMeasure (5,9),(6,9);BellMeasure (7,9),(8,9);BellMeasure (9,9),(9,8);MergeContract (4,8),(4,9)];
>>>>>>> 7db7bb4 (Fixed test cases.)
HGate 116;RotateSingleCellPatch 116;
<<<<<<< HEAD
BusyRegion (0,4),(0,5),StepsToClear(2);
BusyRegion (0,4),(0,5),StepsToClear(1);
BusyRegion (0,4),(0,5),StepsToClear(0);RequestYState 116 11;RequestMagicState 121 12;BellBasedCNOT 12 121 122 123 [ExtendSplit (6,6),(6,7);BellPrepare (1,6),(1,7);BellPrepare (2,7),(3,7);BellPrepare (4,7),(5,7)];
BellBasedCNOT 12 121 122 123 [BellMeasure (1,7),(2,7);BellMeasure (3,7),(4,7);BellMeasure (5,7),(6,7);MergeContract (0,6),(1,6)];MeasureSinglePatch 121 Z;RequestYState 124 12;BellBasedCNOT 12 124 125 126 [ExtendSplit (6,6),(6,5);BellPrepare (1,4),(1,5);BellPrepare (2,5),(3,5);BellPrepare (4,5),(5,5)];
BellBasedCNOT 12 124 125 126 [BellMeasure (1,5),(2,5);BellMeasure (3,5),(4,5);BellMeasure (5,5),(6,5);MergeContract (0,4),(1,4)];
=======
BusyRegion (4,8),(4,9),StepsToClear(2);
BusyRegion (4,8),(4,9),StepsToClear(1);
<<<<<<< HEAD
BusyRegion (4,8),(4,9),StepsToClear(0);RequestYState 116 11;RequestMagicState 121 12;BellBasedCNOT 12 121 122 123 [ExtendSplit (10,10),(10,9);BellPrepare (5,10),(5,9);BellPrepare (6,9),(7,9);BellPrepare (8,9),(9,9)];
BellBasedCNOT 12 121 122 123 [BellMeasure (5,9),(6,9);BellMeasure (7,9),(8,9);BellMeasure (9,9),(10,9);MergeContract (4,10),(5,10)];MeasureSinglePatch 121 Z;RequestYState 124 12;BellBasedCNOT 12 124 125 126 [ExtendSplit (10,10),(10,11);BellPrepare (5,8),(5,7);BellPrepare (6,7),(7,7);BellPrepare (8,7),(9,7);BellPrepare (10,7),(11,7);BellPrepare (11,8),(11,9);BellPrepare (11,10),(11,11)];
BellBasedCNOT 12 124 125 126 [BellMeasure (5,7),(6,7);BellMeasure (7,7),(8,7);BellMeasure (9,7),(10,7);BellMeasure (11,7),(11,8);BellMeasure (11,9),(11,10);BellMeasure (11,11),(10,11);MergeContract (4,8),(5,8)];
>>>>>>> 4dcc26c (Added twist-based Y state preparation on one tile and updated S gate teleportation protocol.)
HGate 124;RotateSingleCellPatch 124;
BusyRegion (0,4),(0,5),StepsToClear(2);
BusyRegion (0,4),(0,5),StepsToClear(1);
BusyRegion (0,4),(0,5),StepsToClear(0);BellBasedCNOT 12 124 127 128 [ExtendSplit (6,6),(6,5);BellPrepare (1,4),(1,5);BellPrepare (2,5),(3,5);BellPrepare (4,5),(5,5)];
BellBasedCNOT 12 124 127 128 [BellMeasure (1,5),(2,5);BellMeasure (3,5),(4,5);BellMeasure (5,5),(6,5);MergeContract (0,4),(1,4)];
HGate 124;RotateSingleCellPatch 124;
BusyRegion (0,4),(0,5),StepsToClear(2);
BusyRegion (0,4),(0,5),StepsToClear(1);
BusyRegion (0,4),(0,5),StepsToClear(0);RequestYState 124 12;RequestMagicState 129 13;BellBasedCNOT 13 129 130 131 [ExtendSplit (6,8),(6,9);BellPrepare (5,12),(5,11);BellPrepare (5,10),(5,9)];
BellBasedCNOT 13 129 130 131 [BellMeasure (5,11),(5,10);BellMeasure (5,9),(6,9);MergeContract (6,12),(5,12)];MeasureSinglePatch 129 Z;RequestYState 132 13;BellBasedCNOT 13 132 133 134 [ExtendSplit (6,8),(6,7);BellPrepare (1,8),(1,7);BellPrepare (2,7),(3,7);BellPrepare (4,7),(5,7)];
BellBasedCNOT 13 132 133 134 [BellMeasure (1,7),(2,7);BellMeasure (3,7),(4,7);BellMeasure (5,7),(6,7);MergeContract (0,8),(1,8)];
=======
BusyRegion (4,8),(4,9),StepsToClear(0);RequestYState 116 11;RequestMagicState 121 12;BellBasedCNOT 12 121 122 123 [ExtendSplit (10,10),(9,10);BellPrepare (4,9),(5,9);BellPrepare (6,9),(7,9);BellPrepare (8,9),(9,9)];
BellBasedCNOT 12 121 122 123 [BellMeasure (5,9),(6,9);BellMeasure (7,9),(8,9);BellMeasure (9,9),(9,10);MergeContract (4,10),(4,9)];MeasureSinglePatch 121 Z;RequestYState 124 12;BellBasedCNOT 12 124 125 126 [ExtendSplit (10,10),(11,10);BellPrepare (4,7),(5,7);BellPrepare (6,7),(7,7);BellPrepare (8,7),(9,7);BellPrepare (10,7),(11,7);BellPrepare (11,8),(11,9)];
BellBasedCNOT 12 124 125 126 [BellMeasure (5,7),(6,7);BellMeasure (7,7),(8,7);BellMeasure (9,7),(10,7);BellMeasure (11,7),(11,8);BellMeasure (11,9),(11,10);MergeContract (4,8),(4,7)];
HGate 124;RotateSingleCellPatch 124;
BusyRegion (4,8),(4,9),StepsToClear(2);
BusyRegion (4,8),(4,9),StepsToClear(1);
BusyRegion (4,8),(4,9),StepsToClear(0);BellBasedCNOT 12 124 127 128 [ExtendSplit (10,10),(9,10);BellPrepare (4,9),(5,9);BellPrepare (6,9),(7,9);BellPrepare (8,9),(9,9)];
BellBasedCNOT 12 124 127 128 [BellMeasure (5,9),(6,9);BellMeasure (7,9),(8,9);BellMeasure (9,9),(9,10);MergeContract (4,8),(4,9)];
HGate 124;RotateSingleCellPatch 124;
BusyRegion (4,8),(4,9),StepsToClear(2);
BusyRegion (4,8),(4,9),StepsToClear(1);
BusyRegion (4,8),(4,9),StepsToClear(0);RequestYState 124 12;RequestMagicState 129 13;BellBasedCNOT 13 129 130 131 [ExtendSplit (10,12),(9,12);BellPrepare (10,15),(9,15);BellPrepare (9,14),(9,13)];
BellBasedCNOT 13 129 130 131 [BellMeasure (9,15),(9,14);BellMeasure (9,13),(9,12);MergeContract (10,16),(10,15)];MeasureSinglePatch 129 Z;RequestYState 132 13;BellBasedCNOT 13 132 133 134 [ExtendSplit (10,12),(11,12);BellPrepare (4,11),(5,11);BellPrepare (6,11),(7,11);BellPrepare (8,11),(9,11);BellPrepare (10,11),(11,11)];
BellBasedCNOT 13 132 133 134 [BellMeasure (5,11),(6,11);BellMeasure (7,11),(8,11);BellMeasure (9,11),(10,11);BellMeasure (11,11),(11,12);MergeContract (4,12),(4,11)];
>>>>>>> 7db7bb4 (Fixed test cases.)
HGate 132;RotateSingleCellPatch 132;
<<<<<<< HEAD
BusyRegion (0,8),(0,9),StepsToClear(2);
BusyRegion (0,8),(0,9),StepsToClear(1);
BusyRegion (0,8),(0,9),StepsToClear(0);BellBasedCNOT 13 132 135 136 [ExtendSplit (6,8),(6,9);BellPrepare (1,8),(1,9);BellPrepare (2,9),(3,9);BellPrepare (4,9),(5,9)];
BellBasedCNOT 13 132 135 136 [BellMeasure (1,9),(2,9);BellMeasure (3,9),(4,9);BellMeasure (5,9),(6,9);MergeContract (0,8),(1,8)];
=======
BusyRegion (4,12),(4,13),StepsToClear(2);
BusyRegion (4,12),(4,13),StepsToClear(1);
<<<<<<< HEAD
BusyRegion (4,12),(4,13),StepsToClear(0);BellBasedCNOT 13 132 135 136 [ExtendSplit (10,12),(10,11);BellPrepare (5,12),(5,11);BellPrepare (6,11),(7,11);BellPrepare (8,11),(9,11)];
BellBasedCNOT 13 132 135 136 [BellMeasure (5,11),(6,11);BellMeasure (7,11),(8,11);BellMeasure (9,11),(10,11);MergeContract (4,12),(5,12)];
>>>>>>> 4dcc26c (Added twist-based Y state preparation on one tile and updated S gate teleportation protocol.)
HGate 132;RotateSingleCellPatch 132;
BusyRegion (0,8),(0,9),StepsToClear(2);
BusyRegion (0,8),(0,9),StepsToClear(1);
BusyRegion (0,8),(0,9),StepsToClear(0);RequestYState 132 13;RequestMagicState 137 14;BellBasedCNOT 14 137 138 139 [ExtendSplit (6,10),(6,9);BellPrepare (1,10),(1,9);BellPrepare (2,9),(3,9);BellPrepare (4,9),(5,9)];
BellBasedCNOT 14 137 138 139 [BellMeasure (1,9),(2,9);BellMeasure (3,9),(4,9);BellMeasure (5,9),(6,9);MergeContract (0,10),(1,10)];MeasureSinglePatch 137 Z;RequestYState 140 14;BellBasedCNOT 14 140 141 142 [ExtendSplit (6,10),(6,11);BellPrepare (5,12),(5,11)];
BellBasedCNOT 14 140 141 142 [BellMeasure (5,11),(6,11);MergeContract (4,12),(5,12)];
HGate 140;RotateSingleCellPatch 140;
BusyRegion (4,12),(4,11),StepsToClear(2);
BusyRegion (4,12),(4,11),StepsToClear(1);
BusyRegion (4,12),(4,11),StepsToClear(0);BellBasedCNOT 14 140 143 144 [ExtendSplit (6,10),(6,11);BellPrepare (5,12),(5,11)];
BellBasedCNOT 14 140 143 144 [BellMeasure (5,11),(6,11);MergeContract (4,12),(5,12)];
HGate 140;RotateSingleCellPatch 140;
BusyRegion (4,12),(4,11),StepsToClear(2);
BusyRegion (4,12),(4,11),StepsToClear(1);
BusyRegion (4,12),(4,11),StepsToClear(0);RequestYState 140 14;RequestMagicState 145 15;BellBasedCNOT 15 145 146 147 [ExtendSplit (8,2),(8,1);BellPrepare (7,0),(7,1)];
BellBasedCNOT 15 145 146 147 [BellMeasure (7,1),(8,1);MergeContract (6,0),(7,0)];MeasureSinglePatch 145 Z;RequestYState 148 15;BellBasedCNOT 15 148 149 150 [ExtendSplit (8,2),(8,3);BellPrepare (9,0),(9,1);BellPrepare (9,2),(9,3)];
BellBasedCNOT 15 148 149 150 [BellMeasure (9,1),(9,2);BellMeasure (9,3),(8,3);MergeContract (8,0),(9,0)];
=======
BusyRegion (4,12),(4,13),StepsToClear(0);BellBasedCNOT 13 132 135 136 [ExtendSplit (10,12),(9,12);BellPrepare (4,13),(5,13);BellPrepare (6,13),(7,13);BellPrepare (8,13),(9,13)];
BellBasedCNOT 13 132 135 136 [BellMeasure (5,13),(6,13);BellMeasure (7,13),(8,13);BellMeasure (9,13),(9,12);MergeContract (4,12),(4,13)];
HGate 132;RotateSingleCellPatch 132;
BusyRegion (4,12),(4,13),StepsToClear(2);
BusyRegion (4,12),(4,13),StepsToClear(1);
BusyRegion (4,12),(4,13),StepsToClear(0);RequestYState 132 13;RequestMagicState 137 14;BellBasedCNOT 14 137 138 139 [ExtendSplit (10,14),(11,14);BellPrepare (10,15),(11,15)];
BellBasedCNOT 14 137 138 139 [BellMeasure (11,15),(11,14);MergeContract (10,16),(10,15)];MeasureSinglePatch 137 Z;RequestYState 140 14;BellBasedCNOT 14 140 141 142 [ExtendSplit (10,14),(9,14);BellPrepare (8,15),(9,15)];
BellBasedCNOT 14 140 141 142 [BellMeasure (9,15),(9,14);MergeContract (8,16),(8,15)];
HGate 140;RotateSingleCellPatch 140;
BusyRegion (8,16),(8,15),StepsToClear(2);
BusyRegion (8,16),(8,15),StepsToClear(1);
BusyRegion (8,16),(8,15),StepsToClear(0);BellBasedCNOT 14 140 143 144 [ExtendSplit (10,14),(9,14);BellPrepare (8,15),(9,15)];
BellBasedCNOT 14 140 143 144 [BellMeasure (9,15),(9,14);MergeContract (8,16),(8,15)];
HGate 140;RotateSingleCellPatch 140;
BusyRegion (8,16),(8,15),StepsToClear(2);
BusyRegion (8,16),(8,15),StepsToClear(1);
BusyRegion (8,16),(8,15),StepsToClear(0);RequestYState 140 14;RequestMagicState 145 15;BellBasedCNOT 15 145 146 147 [ExtendSplit (12,6),(11,6);BellPrepare (10,5),(11,5)];
BellBasedCNOT 15 145 146 147 [BellMeasure (11,5),(11,6);MergeContract (10,4),(10,5)];MeasureSinglePatch 145 Z;RequestYState 148 15;BellBasedCNOT 15 148 149 150 [ExtendSplit (12,6),(13,6);BellPrepare (12,5),(13,5)];
BellBasedCNOT 15 148 149 150 [BellMeasure (13,5),(13,6);MergeContract (12,4),(12,5)];
>>>>>>> 7db7bb4 (Fixed test cases.)
HGate 148;RotateSingleCellPatch 148;
<<<<<<< HEAD
BusyRegion (8,0),(8,1),StepsToClear(2);
BusyRegion (8,0),(8,1),StepsToClear(1);
BusyRegion (8,0),(8,1),StepsToClear(0);BellBasedCNOT 15 148 151 152 [ExtendSplit (8,2),(8,1);BellPrepare (9,0),(9,1)];
BellBasedCNOT 15 148 151 152 [BellMeasure (9,1),(8,1);MergeContract (8,0),(9,0)];
=======
BusyRegion (12,4),(12,5),StepsToClear(2);
BusyRegion (12,4),(12,5),StepsToClear(1);
<<<<<<< HEAD
BusyRegion (12,4),(12,5),StepsToClear(0);BellBasedCNOT 15 148 151 152 [ExtendSplit (12,6),(12,5);BellPrepare (11,4),(11,5)];
BellBasedCNOT 15 148 151 152 [BellMeasure (11,5),(12,5);MergeContract (12,4),(11,4)];
>>>>>>> 4dcc26c (Added twist-based Y state preparation on one tile and updated S gate teleportation protocol.)
=======
BusyRegion (12,4),(12,5),StepsToClear(0);BellBasedCNOT 15 148 151 152 [ExtendSplit (12,6),(11,6);BellPrepare (12,5),(11,5)];
BellBasedCNOT 15 148 151 152 [BellMeasure (11,5),(11,6);MergeContract (12,4),(12,5)];
>>>>>>> 7db7bb4 (Fixed test cases.)
HGate 148;RotateSingleCellPatch 148;
BusyRegion (8,0),(8,1),StepsToClear(2);
BusyRegion (8,0),(8,1),StepsToClear(1);
BusyRegion (8,0),(8,1),StepsToClear(0);RequestYState 148 15;

