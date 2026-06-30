import csv
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parent
INPUT = ROOT / "Birch-Implementation" / "birch_results.csv"
OUT_DIR = ROOT / "metrics"
SUMMARY_OUT = OUT_DIR / "birch_cdr_sdbw_metrics.txt"


def load_points(path):
    points = []
    labels = []

    with path.open(newline="") as handle:
        reader = csv.DictReader(handle)
        for row in reader:
            points.append([
                float(row["TOA_ns"]),
                float(row["Freq_MHz"]),
                float(row["PW_ns"]),
                float(row["Az_deg"]),
                float(row["El_deg"]),
            ])
            labels.append(row["Predicted_Cluster"])

    return points, labels


def euclidean(a, b):
    return math.sqrt(sum((x - y) ** 2 for x, y in zip(a, b)))


def mean(points):
    dimensions = len(points[0])
    return [
        sum(point[d] for point in points) / len(points)
        for d in range(dimensions)
    ]


def variance(points, center):
    dimensions = len(points[0])
    return [
        sum((point[d] - center[d]) ** 2 for point in points) / len(points)
        for d in range(dimensions)
    ]


def norm(values):
    return math.sqrt(sum(value * value for value in values))


def group_by_cluster(points, labels):
    clusters = {}
    for point, label in zip(points, labels):
        clusters.setdefault(label, []).append(point)
    return clusters


def normalize(points):
    columns = list(zip(*points))
    minimums = [min(column) for column in columns]
    maximums = [max(column) for column in columns]
    return [
        [
            0.0 if high == low else (value - low) / (high - low)
            for value, low, high in zip(point, minimums, maximums)
        ]
        for point in points
    ]


def calculate_cdr(points, labels):
    clusters = group_by_cluster(points, labels)
    total = len(points)
    weighted_cdr = 0.0
    cluster_uniformities = {}

    for label, members in clusters.items():
        if len(members) <= 1:
            uniformity = 0.0
        else:
            local_densities = []
            for i, point in enumerate(members):
                nearest = min(
                    euclidean(point, other)
                    for j, other in enumerate(members)
                    if i != j
                )
                local_densities.append(nearest)

            average_density = sum(local_densities) / len(local_densities)
            if average_density == 0.0:
                uniformity = 0.0
            else:
                absolute_variation = sum(
                    abs(value - average_density)
                    for value in local_densities
                )
                uniformity = absolute_variation / average_density

        cluster_uniformities[label] = uniformity
        weighted_cdr += uniformity * len(members) / total

    return weighted_cdr, cluster_uniformities


def density(points, center, radius):
    return sum(euclidean(point, center) <= radius for point in points)


def calculate_sdbw(points, labels):
    clusters = group_by_cluster(points, labels)
    cluster_ids = sorted(clusters)
    centers = {
        label: mean(members)
        for label, members in clusters.items()
    }

    overall_center = mean(points)
    overall_variance_norm = norm(variance(points, overall_center))

    cluster_variance_norms = {}
    for label, members in clusters.items():
        cluster_variance_norms[label] = norm(variance(members, centers[label]))

    scatter = 0.0
    if overall_variance_norm > 0.0:
        scatter = sum(
            cluster_variance_norms[label] / overall_variance_norm
            for label in cluster_ids
        ) / len(cluster_ids)

    mean_variance_norm = sum(cluster_variance_norms.values()) / len(cluster_ids)
    density_radius = math.sqrt(mean_variance_norm)

    density_between = 0.0
    pair_count = 0
    for first in cluster_ids:
        for second in cluster_ids:
            if first == second:
                continue

            midpoint = [
                (a + b) / 2.0
                for a, b in zip(centers[first], centers[second])
            ]
            pair_points = clusters[first] + clusters[second]
            middle_density = density(pair_points, midpoint, density_radius)
            first_density = density(clusters[first], centers[first], density_radius)
            second_density = density(clusters[second], centers[second], density_radius)
            denominator = max(first_density, second_density)

            if denominator > 0:
                density_between += middle_density / denominator
            pair_count += 1

    if pair_count > 0:
        density_between /= pair_count

    return {
        "scatter": scatter,
        "density_between": density_between,
        "sdbw": scatter + density_between,
        "density_radius": density_radius,
        "cluster_variance_norms": cluster_variance_norms,
    }


def main():
    points, labels = load_points(INPUT)
    points = normalize(points)
    birch_rows = [
        (point, label)
        for point, label in zip(points, labels)
        if label != "Noise"
    ]
    points = [point for point, _ in birch_rows]
    labels = [label for _, label in birch_rows]
    clusters = group_by_cluster(points, labels)
    cdr_value, cdr_uniformities = calculate_cdr(points, labels)
    sdbw = calculate_sdbw(points, labels)

    OUT_DIR.mkdir(exist_ok=True)

    with SUMMARY_OUT.open("w", encoding="utf-8") as handle:
        handle.write("BIRCH Cluster Validation Metrics\n")
        handle.write("================================\n\n")
        handle.write(f"Input clustering: {INPUT}\n")
        handle.write("No k-means is run in this evaluation.\n")
        handle.write("LOF-predicted Noise rows are excluded.\n")
        handle.write("CDR and S_Dbw are calculated on BIRCH emitter labels only.\n\n")
        handle.write(f"Total points: {len(points)}\n")
        handle.write(f"Number of BIRCH clusters: {len(clusters)}\n\n")

        handle.write("Cluster sizes:\n")
        for label in sorted(clusters):
            handle.write(f"  Cluster {label}: {len(clusters[label])}\n")

        handle.write("\nCDR metric on BIRCH labels:\n")
        handle.write(f"  CDR: {cdr_value:.6f}\n")
        for label in sorted(cdr_uniformities):
            handle.write(
                f"  Cluster {label} uniformity: {cdr_uniformities[label]:.6f}\n"
            )

        handle.write("\nS_Dbw metric on BIRCH labels:\n")
        handle.write(f"  Scatter: {sdbw['scatter']:.6f}\n")
        handle.write(f"  Density between: {sdbw['density_between']:.6f}\n")
        handle.write(f"  S_Dbw: {sdbw['sdbw']:.6f}\n")
        handle.write(f"  Density radius: {sdbw['density_radius']:.6f}\n")

    print(f"Created {SUMMARY_OUT}")


if __name__ == "__main__":
    main()
