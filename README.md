# BIRCH Implementation: Build, Run, and Evaluate

The active BIRCH code is now in:

```text
Birch-Implementation/
```

`BIRCH_Project/` is the previous implementation and is no longer used by the
root validation, metric, or analysis scripts.

## Pipeline

The active implementation performs:

1. Load the unlabeled PDW feature dataset.
2. Normalize all five input features.
3. Detect outliers with LOF and set them aside as `Noise`.
4. Run BIRCH on the remaining points.
5. Map the two BIRCH clusters to `Emitter_1` and `Emitter_2`.
6. Merge the detected noise points back into the output.
7. Compare predictions with ground truth.
8. Calculate CDR and S_Dbw on the BIRCH emitter clusters only.
9. Generate separate analysis SVG files.

No k-means is run by the root evaluation workflow.

## Ubuntu or WSL Commands

Run these commands line by line:

```bash
cd "/mnt/c/Users/SHAYANTAN BISWAS/Desktop/IITH_INTERNSHIP/InternshipCodes"


python3 prepare_two_emitter_data.py


cd Birch-Implementation


g++ -std=c++17 -O2 -Wall -Wextra -pedantic main.cpp CF.cpp CFNode.cpp CFTree.cpp Dataset.cpp OutlierDetector.cpp Phase2.cpp Phase3.cpp GlobalClustering.cpp Output.cpp Metrics.cpp -o birch


./birch


cd ..


python3 validate_clusters.py Birch-Implementation/birch_results.csv prepared_data/two_emitter_pdw_ground_truth.csv


python3 evaluate_birch_metrics.py


python3 generate_birch_analysis.py
```

## Main Output

The new BIRCH implementation creates:

```text
Birch-Implementation/birch_results.csv
metrics/birch_validation_report.txt
```

To store terminal output in a log, run `./birch > birch_run.log` instead of
`./birch`.

The validation report is recreated automatically every time BIRCH runs. It
contains LOF, BIRCH emitter-clustering, and combined pipeline precision,
recall, F1-score, and accuracy results.

The result CSV uses the original input column names:

```text
TOA_ns,Freq_MHz,PW_ns,Az_deg,El_deg,Ground_Truth,Predicted_Cluster
```

## Built-in Validation Results

The new implementation currently reports:

```text
LOF Precision: 0.820225
LOF Recall: 0.730000
LOF F1-Score: 0.772487

BIRCH non-noise clustering accuracy: 0.991111
Emitter_1 Precision: 1.000000
Emitter_1 Recall: 0.988000
Emitter_1 F1-Score: 0.993964
Emitter_2 Precision: 1.000000
Emitter_2 Recall: 0.995000
Emitter_2 F1-Score: 0.997494
Clustering Macro Precision: 1.000000
Clustering Macro Recall: 0.991500
Clustering Macro F1-Score: 0.995729

Combined pipeline accuracy: 0.977368
Combined Macro Precision: 0.929765
Combined Macro Recall: 0.904333
Combined Macro F1-Score: 0.916152
```

The external ground-truth validator reports:

```text
Rows: 1900
Best mapped accuracy: 97.74%
Adjusted Rand Index: 0.953334
```

Confusion matrix:

```text
cluster,Emitter_1,Emitter_2,Noise
Emitter_1,988,0,10
Emitter_2,0,796,17
Noise,12,4,73
```

## CDR and S_Dbw Metrics

Run:

```bash
python3 evaluate_birch_metrics.py
```

This script:

- reads `Birch-Implementation/birch_results.csv`
- normalizes the raw feature values to match BIRCH's feature space
- excludes rows predicted as `Noise` by LOF
- calculates CDR and S_Dbw on the two BIRCH emitter clusters
- does not run k-means

Output:

```text
metrics/birch_cdr_sdbw_metrics.txt
```

Current values:

```text
CDR: 877.720960
Scatter: 0.874785
Density between: 0.698413
S_Dbw: 1.573197
```

## Separate Analysis Files

Run:

```bash
python3 generate_birch_analysis.py
```

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

## Configuration

The main parameters remain in `Birch-Implementation/main.cpp`:

```cpp
int kNeighbors = 10;
double lofThreshold = 1.5;
double threshold = 0.5;
int branchingFactor = 5;
```
