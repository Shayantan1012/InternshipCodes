# BIRCH Clustering Evaluation Guide

This project runs **BIRCH clustering** on the two-emitter radar PDW dataset.

Important:

- BIRCH is the only clustering algorithm used.
- CDR and S_Dbw are used only as validation metrics on the BIRCH output.
- Do not run CDR or S_Dbw as separate clustering programs for this workflow.
- The files inside `CDR/` and `SdbW/` are kept for reference, but the final workflow uses `evaluate_birch_metrics.py`.

## Dataset Files

```text
two_emitter_pdw_dataset.csv
two_emitter_pdw_labeled.csv
```

The labeled file contains:

```text
Emitter_1
Emitter_2
Noise
```

BIRCH clusters the feature data. Ground truth is used only for validation.

## Ubuntu Terminal Commands

Run these commands line by line from Ubuntu or WSL.

```bash
cd "/mnt/c/Users/SHAYANTAN BISWAS/Desktop/IITH_INTERNSHIP/Assignment/InternshipCodes"


python3 prepare_two_emitter_data.py


cd BIRCH_Project


g++ -std=c++14 -O2 -Wall -Wextra -pedantic CF.cpp CFNode.cpp CFTree.cpp Dataset.cpp GlobalClustering.cpp main.cpp Output.cpp Phase2.cpp Phase3.cpp -o birch


./birch


cd ..


python3 validate_clusters.py BIRCH_Project/birch_results.csv prepared_data/two_emitter_pdw_ground_truth.csv


python3 evaluate_birch_metrics.py


python3 generate_birch_analysis.py
```

## What Each Command Does

```bash
python3 prepare_two_emitter_data.py
```

Prepares helper files for validation:

```text
prepared_data/two_emitter_pdw_ground_truth.csv
prepared_data/two_emitter_pdw_labeled_features.csv
prepared_data/two_emitter_pdw_dataset_features.csv
```

```bash
./birch
```

Runs BIRCH clustering and creates:

```text
BIRCH_Project/birch_results.csv
```

```bash
python3 validate_clusters.py BIRCH_Project/birch_results.csv prepared_data/two_emitter_pdw_ground_truth.csv
```

Validates BIRCH cluster labels against ground truth.

```bash
python3 evaluate_birch_metrics.py
```

Calculates CDR and S_Dbw metrics on the BIRCH cluster labels.

This does **not** run k-means.

Output:

```text
metrics/birch_cdr_sdbw_metrics.txt
```

```bash
python3 generate_birch_analysis.py
```

Creates separate SVG analysis plots.

Outputs:

```text
analysis/confusion_matrix.svg
analysis/correct_vs_mismatch.svg
analysis/mismatch_composition.svg
analysis/toa_vs_frequency.svg
analysis/mismatch_breakdown.svg
analysis/summary_statistics.svg
analysis/birch_clustering_summary.txt
```

## BIRCH Configuration

The BIRCH configuration is inside:

```text
BIRCH_Project/main.cpp
```

Current configuration:

```cpp
const string inputFile = "../two_emitter_pdw_labeled.csv";
const bool useGroundTruth = true;
const string outputFile = "birch_results.csv";
const double threshold = 0.5;
const int branchingFactor = 5;
const int phase2MinPoints = 5;
const int finalClusters = 2;
```

## Expected Validation Result

Expected BIRCH validation output:

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

- all `Emitter_1` points are assigned to one BIRCH cluster
- all `Emitter_2` points are assigned to another BIRCH cluster
- noise points are absorbed into the two BIRCH clusters

## Expected BIRCH Metric Output

The metric file:

```text
metrics/birch_cdr_sdbw_metrics.txt
```

contains CDR and S_Dbw values calculated directly on `BIRCH_Project/birch_results.csv`.

Expected values:

```text
CDR: 1370.813796
Scatter: 0.865624
Density between: 0.703448
S_Dbw: 1.569072
```

Again, these metrics are calculated from BIRCH labels only.

No k-means is run.

