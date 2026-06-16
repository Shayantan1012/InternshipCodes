# S_Dbw Optimal Partition Finder

This C++ program finds an appropriate number of clusters by:

1. Running k-means with k-means++ initialization for every `k` in a range.
2. Calculating the S_Dbw validity index from the paper.
3. Selecting the partition with the minimum S_Dbw score.

The implementation uses OOP classes for dataset loading, distance calculation,
k-means clustering, S_Dbw evaluation, partition selection, and result writing.

## Build

```powershell
g++ -std=c++14 -O2 -Wall -Wextra -pedantic sdbw.cpp -o sdbw.exe
```

## Run

The input must be a headerless CSV containing only finite numeric features.

```powershell
.\sdbw.exe <data.csv> <min_k> <max_k> [restarts=10] [seed=42] [assignments.csv]
```

Example:

```powershell
.\sdbw.exe sample.csv 2 5
```

The program prints `Scat`, `Dens_bw`, total `S_Dbw`, and SSE for each candidate
partition. It writes the winning partition to `assignments.csv` by default.
