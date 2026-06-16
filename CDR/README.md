# CDR Optimal Partition Finder

This project implements the Contiguous Density Region (CDR) internal
clustering-validation index from:

> Rojas-Thomas and Santos, "New internal clustering validation measure for
> contiguous arbitrary-shape clusters," 2021.

For each candidate partition:

1. A point's local density is its Euclidean distance to its nearest neighbor
   inside the same cluster.
2. Cluster uniformity is the absolute variation of local densities normalized
   by their average.
3. CDR is the cluster-size-weighted sum of cluster uniformities.
4. The selected partition has the lowest consecutive CDR improvement factor
   before the first local minimum, as described in the paper.

The implementation uses OOP classes for CSV loading, distance calculation,
k-means++ clustering, CDR evaluation, optimal partition selection, and output.

## Build

```powershell
g++ -std=c++14 -O2 -Wall -Wextra -pedantic cdr.cpp -o cdr.exe
```

## Run

Input is a headerless CSV containing finite numeric features.

```powershell
.\cdr.exe <data.csv> <max_k> [restarts=10] [seed=42] [assignments.csv]
```

Example:

```powershell
.\cdr.exe sample.csv 6
```

The program evaluates partitions from `k=1` through `max_k`. Partition `k=1`
is required by the paper to calculate the improvement factor for `k=2`.
