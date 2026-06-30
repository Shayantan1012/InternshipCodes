Running the Project

Requirements

Before running the project, make sure the following tools are installed:


C++ compiler (G++ recommended)
Git
Terminal / Command Prompt


The project was developed and tested using:

C++17
G++ Compiler
Windows/Linux Environment


Step 1: Clone the Repository

Clone the GitHub repository:

bashgit clone https://github.com/Bavadharani-R/Birch-Implementation.git

Move into the project directory:

bashcd Birch-Implementation

The project directory should contain:

Birch-Implementation
│
├── main.cpp
├── CF.cpp
├── CF.h
├── CFNode.cpp
├── CFNode.h
├── CFTree.cpp
├── CFTree.h
├── Dataset.cpp
├── Dataset.h
├── OutlierDetector.cpp
├── OutlierDetector.h
├── Phase2.cpp
├── Phase2.h
├── Phase3.cpp
├── Phase3.h
├── GlobalClustering.cpp
├── GlobalClustering.h
├── Output.cpp
├── Output.h
├── Metrics.cpp
├── Metrics.h
│
├── two_emitter_pdw_dataset.csv
└── two_emitter_pdw_labeled.csv


Step 2: Dataset Placement

Place both dataset files inside the project folder:


two_emitter_pdw_dataset.csv — raw input data (read by the program)
two_emitter_pdw_labeled.csv — ground truth reference file, used for comparison and metrics


The dataset contains five input features:


TOA_ns
Freq_MHz
PW_ns
Az_deg
El_deg


The ground truth file additionally contains a Ground_Truth column with values Emitter_1, Emitter_2, or Noise.


Step 3: Compile the Program

Compile all C++ source files together:

bashg++ main.cpp CF.cpp CFNode.cpp CFTree.cpp Dataset.cpp OutlierDetector.cpp Phase2.cpp Phase3.cpp GlobalClustering.cpp Output.cpp Metrics.cpp -o birch

If compilation is successful, an executable file will be generated:

Windows: birch.exe
Linux: birch


Step 4: Run the Program

Windows:

bashbirch.exe

Linux / macOS:

bash./birch


Step 5: Execution Flow

Input CSV Dataset
        |
        v
Dataset Loading
        |
        v
Feature Normalization
        |
        v
Outlier Detection (LOF)
        |
        v
Noise Points Set Aside
        |
        v
Phase 1: CF Tree Construction (clean points only)
        |
        v
Incremental Point Insertion
        |
        v
CF Merge / New CF Creation
        |
        v
Node Overflow Detection
        |
        v
CF Tree Splitting
        |
        v
Phase 2: CF Tree Condensation
        |
        v
Removal of Small Micro Clusters
        |
        v
Phase 3: Global Clustering
        |
        v
Final Cluster Assignment
        |
        v
Map Clusters to Emitter Names
        |
        v
Merge Noise Points Back as "Noise"
        |
        v
Compare with Ground Truth
        |
        v
Result CSV Generation
        |
        v
Validation Metrics (Outlier / Clustering / Combined)


Step 6: Configuration Parameters

The main BIRCH parameters can be modified in main.cpp:

cppdouble threshold = 0.5;
int branchingFactor = 5;

Threshold — controls the maximum distance allowed for merging a new point into an existing CF.
If distance <= threshold, the point is merged; otherwise a new CF is created.

Branching Factor — controls the maximum number of CF entries stored in one node.
When the node exceeds this limit, a node overflow is detected and the node is split.

The outlier detection parameters can also be modified in main.cpp:

cppint kNeighbors = 10;
double lofThreshold = 1.5;

kNeighbors — number of neighbors used by LOF to estimate local density.
lofThreshold — LOF score above which a point is flagged as noise.


Step 7: Program Output

During execution, the terminal displays:

Outlier Detection Output

Total points: 1900
Noise points removed: 89
Clean points remaining: 1811

Phase 1 Output

Distance = 0.0417
Merged into existing CF

or

Distance = 0.5339
Created new CF

On overflow:

Node Overflow detected
Splitting root node...

Phase 2 Output

Removing small CF N=1
Phase 2 Remaining CFs = 2

Phase 3 Output

Micro Cluster 0 belongs to Cluster 0
Micro Cluster 1 belongs to Cluster 1

Validation Metrics Output

===== OUTLIER DETECTION METRICS (LOF) =====
Precision = 0.82
Recall = 0.73
F1-Score = 0.77

===== CLUSTERING METRICS (BIRCH only, Emitter_1 vs Emitter_2) =====
Clustering Accuracy = 0.991

===== COMBINED PIPELINE METRICS (Outlier + Clustering) =====
Overall Accuracy = 0.977

The program also reports per-class and macro-averaged classification metrics:

```text
===== CLUSTERING METRICS (BIRCH only, Emitter_1 vs Emitter_2) =====
Emitter_1 Precision=1 Recall=0.988 F1-Score=0.993964
Emitter_2 Precision=1 Recall=0.995 F1-Score=0.997494
Macro Precision = 1
Macro Recall = 0.9915
Macro F1-Score = 0.995729

===== COMBINED PIPELINE METRICS (Outlier + Clustering) =====
Emitter_1 Precision=0.98998 Recall=0.988 F1-Score=0.988989
Emitter_2 Precision=0.97909 Recall=0.995 F1-Score=0.986981
Noise Precision=0.820225 Recall=0.73 F1-Score=0.772487
Macro Precision = 0.929765
Macro Recall = 0.904333
Macro F1-Score = 0.916152
```

The same validation output is saved automatically to:

```text
../metrics/birch_validation_report.txt
```


Step 8: Generated Output File

After successful execution, the labeled clustering results are stored in:

birch_results.csv

The output contains the original raw features, the ground truth label, and the predicted label:

TOA_ns,Freq_MHz,PW_ns,Az_deg,El_deg,Ground_Truth,Predicted_Cluster
20000,5900,4000,20.1219,7.688,Emitter_1,Emitter_1
55000,6100,4000,-25.1808,14.8002,Emitter_2,Noise
...


Troubleshooting

Compilation Error

Make sure all .cpp files are included during compilation:

bashg++ main.cpp CF.cpp CFNode.cpp CFTree.cpp Dataset.cpp OutlierDetector.cpp Phase2.cpp Phase3.cpp GlobalClustering.cpp Output.cpp Metrics.cpp -o birch

Dataset Not Found

If the program displays a file loading error, check that both
two_emitter_pdw_dataset.csv and two_emitter_pdw_labeled.csv
are located in the same folder as the executable.

Permission Error (Linux)

Give execution permission:

bashchmod +x birch

Then run:

bash./birch


Complete BIRCH + LOF Execution Summary

The implementation performs:


Load PDW dataset
Normalize features
Run LOF outlier detection on all points (preprocessing, before clustering)
Set aside noise points; pass only clean points to BIRCH
Construct CF Tree (Phase 1)
Incrementally insert points
Merge points into existing CFs
Create new CFs when threshold is exceeded
Split overflowing CF Nodes
Condense CF Tree (Phase 2) — remove insignificant micro clusters
Perform global clustering (Phase 3)
Generate final cluster labels
Map clusters to Emitter_1 / Emitter_2 by comparing average frequency
Re-merge noise points back into the result as "Noise"
Compare predictions against ground truth
Save labeled results to CSV
Compute validation metrics: outlier detection (Precision/Recall/F1), clustering (Accuracy), and combined pipeline (Overall Accuracy)


The complete pipeline — outlier detection, BIRCH clustering, and validation — can therefore be executed using a single compilation and run command.
