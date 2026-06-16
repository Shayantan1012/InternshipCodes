import argparse
import csv
from itertools import permutations
from math import comb


def load_clusters(path):
    with open(path, newline="") as handle:
        reader = csv.DictReader(handle)
        if "cluster" in reader.fieldnames:
            column = "cluster"
        elif "Cluster" in reader.fieldnames:
            column = "Cluster"
        else:
            raise ValueError("Assignment CSV must contain a cluster or Cluster column.")

        return [record[column] for record in reader]


def load_truth(path):
    with open(path, newline="") as handle:
        reader = csv.DictReader(handle)
        if "Ground_Truth" not in reader.fieldnames:
            raise ValueError("Truth CSV must contain a Ground_Truth column.")
        return [record["Ground_Truth"] for record in reader]


def best_mapped_accuracy(clusters, truth):
    cluster_ids = sorted(set(clusters))
    truth_ids = sorted(set(truth))
    best_correct = 0
    best_mapping = {}

    for ordering in permutations(truth_ids):
        mapping = dict(zip(cluster_ids, ordering))
        correct = sum(mapping.get(cluster) == label for cluster, label in zip(clusters, truth))
        if correct > best_correct:
            best_correct = correct
            best_mapping = mapping

    return best_correct / len(truth), best_mapping


def adjusted_rand_index(clusters, truth):
    contingency = {}
    cluster_counts = {}
    truth_counts = {}

    for cluster, label in zip(clusters, truth):
        contingency[(cluster, label)] = contingency.get((cluster, label), 0) + 1
        cluster_counts[cluster] = cluster_counts.get(cluster, 0) + 1
        truth_counts[label] = truth_counts.get(label, 0) + 1

    sum_comb = sum(comb(count, 2) for count in contingency.values())
    sum_cluster = sum(comb(count, 2) for count in cluster_counts.values())
    sum_truth = sum(comb(count, 2) for count in truth_counts.values())
    total_pairs = comb(len(clusters), 2)

    expected = sum_cluster * sum_truth / total_pairs if total_pairs else 0.0
    maximum = 0.5 * (sum_cluster + sum_truth)
    denominator = maximum - expected

    return 0.0 if denominator == 0.0 else (sum_comb - expected) / denominator


def confusion_rows(clusters, truth):
    cluster_ids = sorted(set(clusters))
    truth_ids = sorted(set(truth))
    rows = []

    for cluster in cluster_ids:
        row = [cluster]
        for label in truth_ids:
            row.append(sum(c == cluster and t == label for c, t in zip(clusters, truth)))
        rows.append(row)

    return ["cluster"] + truth_ids, rows


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("assignments")
    parser.add_argument("truth")
    args = parser.parse_args()

    clusters = load_clusters(args.assignments)
    truth = load_truth(args.truth)

    if len(clusters) != len(truth):
        raise ValueError(
            f"Row mismatch: {len(clusters)} assignments, {len(truth)} truth labels."
        )

    accuracy, mapping = best_mapped_accuracy(clusters, truth)
    ari = adjusted_rand_index(clusters, truth)
    header, rows = confusion_rows(clusters, truth)

    print(f"Rows: {len(clusters)}")
    print(f"Best mapped accuracy: {accuracy * 100:.2f}%")
    print(f"Adjusted Rand Index: {ari:.6f}")
    print("Best cluster mapping:")
    for cluster, label in mapping.items():
        print(f"  {cluster} -> {label}")
    print("Confusion matrix:")
    print(",".join(header))
    for row in rows:
        print(",".join(str(value) for value in row))


if __name__ == "__main__":
    main()
