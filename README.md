# BIRCH, CDR, and S_Dbw Clustering Run Guide

This folder contains three clustering-related programs:

- `BIRCH_Project`: BIRCH clustering implementation.
- `CDR`: CDR clustering-validation metric implementation.
- `SdbW`: S_Dbw clustering-validation metric implementation.

The dataset files are:

- `two_emitter_pdw_dataset.csv`: unlabeled PDW feature dataset.
- `two_emitter_pdw_labeled.csv`: labeled PDW dataset with `Ground_Truth`.

The labeled file contains three ground-truth labels:

- `Emitter_1`
- `Emitter_2`
- `Noise`

For two-emitter clustering validation, the programs are run with `k=2`. The
noise points are still included in the validation confusion matrix.

## Requirements

Run all commands from PowerShell.

Required tools:

- Python 3
- `g++` with C++14 support

Go to the project folder:

```powershell
cd "C:\Users\SHAYANTAN BISWAS\Desktop\IITH_INTERNSHIP\Assignment\InternshipCodes"
```

## Step 1: Prepare the Data for CDR and S_Dbw

CDR and S_Dbw need headerless numeric CSV files. Run:

```powershell
python prepare_two_emitter_data.py
```

This creates:

```text
prepared_data\two_emitter_pdw_dataset_features.csv
prepared_data\two_emitter_pdw_labeled_features.csv
prepared_data\two_emitter_pdw_ground_truth.csv
```

Use `two_emitter_pdw_labeled_features.csv` for clustering and
`two_emitter_pdw_ground_truth.csv` for validation.

## Step 2: Run BIRCH

Build BIRCH:

```powershell
cd BIRCH_Project
g++ -std=c++14 -O2 -Wall -Wextra -pedantic CF.cpp CFNode.cpp CFTree.cpp Dataset.cpp GlobalClustering.cpp main.cpp Output.cpp Phase2.cpp Phase3.cpp -o birch.exe
```

Run BIRCH:

```powershell
.\birch.exe
```

Go back to the root folder:

```powershell
cd ..
```

BIRCH output:

```text
BIRCH_Project\birch_results.csv
```

Current BIRCH configuration is inside `BIRCH_Project\main.cpp`:

```cpp
const string inputFile = "../two_emitter_pdw_labeled.csv";
const bool useGroundTruth = true;
const string outputFile = "birch_results.csv";
const double threshold = 0.5;
const int branchingFactor = 5;
const int phase2MinPoints = 5;
const int finalClusters = 2;
```

## Step 3: Run CDR

Build CDR:

```powershell
cd CDR
g++ -std=c++14 -O2 -Wall -Wextra -pedantic cdr.cpp -o cdr.exe
```

Run CDR:

```powershell
.\cdr.exe ..\prepared_data\two_emitter_pdw_labeled_features.csv 6 10 42 cdr_two_emitter_assignments.csv
```

Go back to the root folder:

```powershell
cd ..
```

CDR output:

```text
CDR\cdr_two_emitter_assignments.csv
```

Command format:

```powershell
.\cdr.exe <data.csv> <max_k> [restarts=10] [seed=42] [assignments.csv]
```

For this dataset, CDR selected:

```text
k = 2
```

## Step 4: Run S_Dbw

Build S_Dbw:

```powershell
cd SdbW
g++ -std=c++14 -O2 -Wall -Wextra -pedantic sdbw.cpp -o sdbw.exe
```

Run S_Dbw for two-emitter validation:

```powershell
.\sdbw.exe ..\prepared_data\two_emitter_pdw_labeled_features.csv 2 2 10 42 sdbw_two_emitter_k2_assignments.csv
```

Go back to the root folder:

```powershell
cd ..
```

S_Dbw output:

```text
SdbW\sdbw_two_emitter_k2_assignments.csv
```

Command format:

```powershell
.\sdbw.exe <data.csv> <min_k> <max_k> [restarts=10] [seed=42] [assignments.csv]
```

If you allow S_Dbw to search more clusters, for example `2 6`, it may split
each emitter into smaller subclusters. For direct two-emitter validation, use
`min_k=2` and `max_k=2`.

## Step 5: Validate with Ground Truth

Run these commands from the root folder:

```powershell
python validate_clusters.py BIRCH_Project\birch_results.csv prepared_data\two_emitter_pdw_ground_truth.csv
python validate_clusters.py CDR\cdr_two_emitter_assignments.csv prepared_data\two_emitter_pdw_ground_truth.csv
python validate_clusters.py SdbW\sdbw_two_emitter_k2_assignments.csv prepared_data\two_emitter_pdw_ground_truth.csv
```

The validation script prints:

- number of rows
- best mapped accuracy
- Adjusted Rand Index
- best cluster-to-label mapping
- confusion matrix

Cluster numbers are arbitrary, so validation uses best label mapping before
calculating accuracy.

## Expected Results

For the two-emitter validation configuration, the expected results are:

```text
Rows: 1900
Best mapped accuracy: 94.74%
Adjusted Rand Index: 0.897767
```

Expected confusion matrix:

```text
cluster,Emitter_1,Emitter_2,Noise
0,1000,0,49
1,0,800,51
```

This means:

- all `Emitter_1` points are grouped in one cluster
- all `Emitter_2` points are grouped in another cluster
- noise points are split between the two emitter clusters

## Full Run Order

If starting from the root folder, run:

```powershell
python prepare_two_emitter_data.py

cd BIRCH_Project
g++ -std=c++14 -O2 -Wall -Wextra -pedantic CF.cpp CFNode.cpp CFTree.cpp Dataset.cpp GlobalClustering.cpp main.cpp Output.cpp Phase2.cpp Phase3.cpp -o birch.exe
.\birch.exe
cd ..

cd CDR
g++ -std=c++14 -O2 -Wall -Wextra -pedantic cdr.cpp -o cdr.exe
.\cdr.exe ..\prepared_data\two_emitter_pdw_labeled_features.csv 6 10 42 cdr_two_emitter_assignments.csv
cd ..

cd SdbW
g++ -std=c++14 -O2 -Wall -Wextra -pedantic sdbw.cpp -o sdbw.exe
.\sdbw.exe ..\prepared_data\two_emitter_pdw_labeled_features.csv 2 2 10 42 sdbw_two_emitter_k2_assignments.csv
cd ..

python validate_clusters.py BIRCH_Project\birch_results.csv prepared_data\two_emitter_pdw_ground_truth.csv
python validate_clusters.py CDR\cdr_two_emitter_assignments.csv prepared_data\two_emitter_pdw_ground_truth.csv
python validate_clusters.py SdbW\sdbw_two_emitter_k2_assignments.csv prepared_data\two_emitter_pdw_ground_truth.csv
```

## Copy Paste Commands Line by Line

```powershell
cd "C:\Users\SHAYANTAN BISWAS\Desktop\IITH_INTERNSHIP\Assignment\InternshipCodes"

python prepare_two_emitter_data.py

cd BIRCH_Project

g++ -std=c++14 -O2 -Wall -Wextra -pedantic CF.cpp CFNode.cpp CFTree.cpp Dataset.cpp GlobalClustering.cpp main.cpp Output.cpp Phase2.cpp Phase3.cpp -o birch.exe
.\birch.exe
cd ..
cd CDR
g++ -std=c++14 -O2 -Wall -Wextra -pedantic cdr.cpp -o cdr.exe
.\cdr.exe ..\prepared_data\two_emitter_pdw_labeled_features.csv 6 10 42 cdr_two_emitter_assignments.csv
cd ..
cd SdbW
g++ -std=c++14 -O2 -Wall -Wextra -pedantic sdbw.cpp -o sdbw.exe
.\sdbw.exe ..\prepared_data\two_emitter_pdw_labeled_features.csv 2 2 10 42 sdbw_two_emitter_k2_assignments.csv
cd ..
python validate_clusters.py BIRCH_Project\birch_results.csv prepared_data\two_emitter_pdw_ground_truth.csv
python validate_clusters.py CDR\cdr_two_emitter_assignments.csv prepared_data\two_emitter_pdw_ground_truth.csv
python validate_clusters.py SdbW\sdbw_two_emitter_k2_assignments.csv prepared_data\two_emitter_pdw_ground_truth.csv
```
