import csv
from pathlib import Path


ROOT = Path(__file__).resolve().parent
OUT_DIR = ROOT / "prepared_data"


def read_numeric_rows(path, labeled):
    rows = []
    truth = []

    with path.open(newline="") as handle:
        reader = csv.DictReader(handle)
        for record in reader:
            if labeled:
                rows.append([
                    float(record["TOA_ns"]),
                    float(record["Freq_MHz"]),
                    float(record["PW_ns"]),
                    float(record["Az_deg"]),
                    float(record["El_deg"]),
                ])
                truth.append(record["Ground_Truth"])
            else:
                rows.append([
                    float(record["TOA_ns"]),
                    float(record["Freq_MHz"]),
                    float(record["PW_ns"]),
                    float(record["Az_deg"]),
                    float(record["El_deg"]),
                ])

    return rows, truth


def normalize(rows):
    columns = list(zip(*rows))
    mins = [min(column) for column in columns]
    maxs = [max(column) for column in columns]

    normalized = []
    for row in rows:
        normalized_row = []
        for value, low, high in zip(row, mins, maxs):
            normalized_row.append(0.0 if high == low else (value - low) / (high - low))
        normalized.append(normalized_row)

    return normalized


def write_rows(path, rows):
    with path.open("w", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerows(rows)


def write_truth(path, truth):
    with path.open("w", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow(["Ground_Truth"])
        for label in truth:
            writer.writerow([label])


def main():
    OUT_DIR.mkdir(exist_ok=True)

    unlabeled_rows, _ = read_numeric_rows(ROOT / "two_emitter_pdw_dataset.csv", False)
    labeled_rows, truth = read_numeric_rows(ROOT / "two_emitter_pdw_labeled.csv", True)

    write_rows(OUT_DIR / "two_emitter_pdw_dataset_features.csv", normalize(unlabeled_rows))
    write_rows(OUT_DIR / "two_emitter_pdw_labeled_features.csv", normalize(labeled_rows))
    write_truth(OUT_DIR / "two_emitter_pdw_ground_truth.csv", truth)

    print("Prepared files:")
    print(OUT_DIR / "two_emitter_pdw_dataset_features.csv")
    print(OUT_DIR / "two_emitter_pdw_labeled_features.csv")
    print(OUT_DIR / "two_emitter_pdw_ground_truth.csv")


if __name__ == "__main__":
    main()
