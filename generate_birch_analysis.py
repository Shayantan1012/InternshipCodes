import csv
import math
from pathlib import Path


ROOT = Path(__file__).resolve().parent
RESULTS = ROOT / "Birch-Implementation" / "birch_results.csv"
OUT_DIR = ROOT / "analysis"
SUMMARY_OUT = OUT_DIR / "birch_clustering_summary.txt"
PLOT_FILES = {
    "confusion": OUT_DIR / "confusion_matrix.svg",
    "bars": OUT_DIR / "correct_vs_mismatch.svg",
    "pie": OUT_DIR / "mismatch_composition.svg",
    "scatter": OUT_DIR / "toa_vs_frequency.svg",
    "breakdown": OUT_DIR / "mismatch_breakdown.svg",
    "table": OUT_DIR / "summary_statistics.svg",
}


def load_rows(path):
    with path.open(newline="") as handle:
        return list(csv.DictReader(handle))


def best_cluster_mapping(rows):
    if rows and "Predicted_Cluster" in rows[0]:
        return {
            label: label
            for label in sorted({row["Predicted_Cluster"] for row in rows})
        }

    clusters = sorted({row["Cluster"] for row in rows})
    labels = sorted({row["Ground_Truth"] for row in rows if row["Ground_Truth"] != "Noise"})

    if len(clusters) == 2 and labels == ["Emitter_1", "Emitter_2"]:
        options = [
            {clusters[0]: labels[0], clusters[1]: labels[1]},
            {clusters[0]: labels[1], clusters[1]: labels[0]},
        ]
    else:
        options = []
        for cluster in clusters:
            counts = {}
            for row in rows:
                if row["Cluster"] == cluster and row["Ground_Truth"] != "Noise":
                    counts[row["Ground_Truth"]] = counts.get(row["Ground_Truth"], 0) + 1
            if counts:
                options.append({cluster: max(counts, key=counts.get)})

    best = None
    best_correct = -1
    for mapping in options:
        correct = sum(
            row["Ground_Truth"] != "Noise"
            and mapping.get(row["Cluster"]) == row["Ground_Truth"]
            for row in rows
        )
        if correct > best_correct:
            best_correct = correct
            best = mapping

    return best


def scale(value, low, high, start, end):
    if high == low:
        return (start + end) / 2
    return start + (value - low) * (end - start) / (high - low)


def rect(x, y, w, h, fill, stroke="none", sw=1, opacity=1.0):
    return (
        f'<rect x="{x:.2f}" y="{y:.2f}" width="{w:.2f}" height="{h:.2f}" '
        f'fill="{fill}" stroke="{stroke}" stroke-width="{sw}" opacity="{opacity}"/>'
    )


def text(x, y, value, size=14, weight="normal", fill="#111", anchor="middle"):
    escaped = str(value).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")
    return (
        f'<text x="{x:.2f}" y="{y:.2f}" font-family="Arial, sans-serif" '
        f'font-size="{size}" font-weight="{weight}" fill="{fill}" '
        f'text-anchor="{anchor}">{escaped}</text>'
    )


def rotated_text(x, y, value, size=14, weight="normal", fill="#111", anchor="middle", angle=-90):
    escaped = str(value).replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;")
    return (
        f'<text x="{x:.2f}" y="{y:.2f}" font-family="Arial, sans-serif" '
        f'font-size="{size}" font-weight="{weight}" fill="{fill}" '
        f'text-anchor="{anchor}" transform="rotate({angle} {x:.2f} {y:.2f})">'
        f'{escaped}</text>'
    )


def line(x1, y1, x2, y2, stroke="#ddd", sw=1):
    return (
        f'<line x1="{x1:.2f}" y1="{y1:.2f}" x2="{x2:.2f}" y2="{y2:.2f}" '
        f'stroke="{stroke}" stroke-width="{sw}"/>'
    )


def circle(x, y, r, fill, stroke="none", sw=1, opacity=1.0):
    return (
        f'<circle cx="{x:.2f}" cy="{y:.2f}" r="{r:.2f}" fill="{fill}" '
        f'stroke="{stroke}" stroke-width="{sw}" opacity="{opacity}"/>'
    )


def pie_slice(cx, cy, r, start_angle, end_angle, fill):
    start = math.radians(start_angle)
    end = math.radians(end_angle)
    x1 = cx + r * math.cos(start)
    y1 = cy + r * math.sin(start)
    x2 = cx + r * math.cos(end)
    y2 = cy + r * math.sin(end)
    large_arc = 1 if end_angle - start_angle > 180 else 0
    return (
        f'<path d="M {cx:.2f} {cy:.2f} L {x1:.2f} {y1:.2f} '
        f'A {r:.2f} {r:.2f} 0 {large_arc} 1 {x2:.2f} {y2:.2f} Z" '
        f'fill="{fill}" stroke="white" stroke-width="3"/>'
    )


def confusion_matrix(rows, mapping):
    truth_labels = ["Emitter_1", "Emitter_2", "Noise"]
    pred_labels = ["Emitter_1", "Emitter_2", "Noise"]
    matrix = {truth: {pred: 0 for pred in pred_labels} for truth in truth_labels}

    for row in rows:
        truth = row["Ground_Truth"]
        predicted_value = row.get("Predicted_Cluster", row.get("Cluster", ""))
        predicted = mapping.get(predicted_value, predicted_value)
        if predicted in pred_labels and truth in truth_labels:
            matrix[truth][predicted] += 1

    return truth_labels, pred_labels, matrix


def draw_dashboard(rows, mapping, stats):
    truth_labels, pred_labels, matrix = confusion_matrix(rows, mapping)
    width, height = 1500, 980
    parts = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}">',
        rect(0, 0, width, height, "#ffffff"),
        text(width / 2, 45, "BIRCH Clustering Evaluation - Two-Emitter Radar PDW Dataset", 20, "bold"),
    ]

    # Confusion matrix
    x0, y0, cell = 90, 95, 140
    parts.append(text(x0 + cell, y0 - 18, "Confusion Matrix", 15, "bold"))
    max_count = max(matrix[t][p] for t in truth_labels for p in pred_labels)
    for i, truth in enumerate(truth_labels):
        parts.append(text(x0 - 18, y0 + i * cell + cell / 2 + 5, truth, 12, anchor="end"))
        for j, pred in enumerate(pred_labels):
            count = matrix[truth][pred]
            intensity = int(245 - 150 * (count / max_count if max_count else 0))
            fill = f"rgb({intensity},{intensity + 8},{255})" if count else "#f4f8ff"
            if count > 0:
                blue = int(230 - 150 * count / max_count)
                fill = f"rgb({blue},{blue + 35},190)"
            parts.append(rect(x0 + j * cell, y0 + i * cell, cell, cell, fill, "white", 2))
            parts.append(text(x0 + j * cell + cell / 2, y0 + i * cell + cell / 2 + 6, count, 18, "bold", "white" if count > max_count * 0.35 else "#123"))
    for j, pred in enumerate(pred_labels):
        parts.append(text(x0 + j * cell + cell / 2, y0 + len(truth_labels) * cell + 25, pred, 12))
    parts.append(text(x0 + cell, y0 + len(truth_labels) * cell + 55, "Predicted Label", 12, "bold"))
    parts.append(text(x0 - 70, y0 + cell * 1.55, "Ground Truth Label", 12, "bold"))

    # Correct vs mismatch bars
    bx, by, bw, bh = 560, 95, 340, 340
    parts.append(text(bx + bw / 2, by - 18, "Correct vs Mismatch by Group", 15, "bold"))
    groups = [
        ("All Points", stats["correct"], stats["mismatch"], stats["total"]),
        ("Non-Noise", stats["non_noise_correct"], stats["non_noise_mismatch"], stats["non_noise_total"]),
        ("Noise Points", 0, stats["noise_total"], stats["noise_total"]),
    ]
    max_bar = max(g[3] for g in groups)
    for grid in range(0, 5):
        gy = by + bh - grid * bh / 4
        parts.append(line(bx, gy, bx + bw, gy, "#e5e7eb"))
    bar_w = 52
    for idx, (name, correct, mismatch, total) in enumerate(groups):
        base_x = bx + 45 + idx * 115
        correct_h = bh * correct / max_bar if max_bar else 0
        mismatch_h = bh * mismatch / max_bar if max_bar else 0
        parts.append(rect(base_x, by + bh - correct_h, bar_w, correct_h, "#2196f3"))
        parts.append(rect(base_x, by + bh - correct_h - mismatch_h, bar_w, mismatch_h, "#ff5252"))
        if correct:
            parts.append(text(base_x + bar_w / 2, by + bh - correct_h / 2 + 5, correct, 12, "bold", "white"))
        if mismatch:
            parts.append(text(base_x + bar_w / 2, by + bh - correct_h - mismatch_h / 2 + 5, mismatch, 12, "bold", "white"))
        pct = 100 * correct / total if total else 0
        parts.append(text(base_x + bar_w / 2, by + bh - correct_h - mismatch_h - 10, f"{pct:.1f}%", 12, "bold"))
        parts.append(text(base_x + bar_w / 2, by + bh + 22, name, 11))
        parts.append(text(base_x + bar_w / 2, by + bh + 39, f"({total})", 10))
    parts.append(rect(bx + bw - 78, by + 8, 14, 14, "#2196f3"))
    parts.append(text(bx + bw - 58, by + 20, "Correct", 11, anchor="start"))
    parts.append(rect(bx + bw - 78, by + 28, 14, 14, "#ff5252"))
    parts.append(text(bx + bw - 58, by + 40, "Mismatch", 11, anchor="start"))

    # Pie chart
    px, py = 1150, 235
    parts.append(text(px, 75, f"Mismatch Composition", 15, "bold"))
    parts.append(text(px, 93, f"({stats['total']} total)", 13, "bold"))
    slices = [
        ("Correct", stats["correct"], "#4caf50"),
        ("Non-noise mismatches", stats["non_noise_mismatch"], "#ff9800"),
        ("Noise mismatches", stats["noise_total"], "#f44336"),
    ]
    angle = -90
    for _, value, color in slices:
        delta = 360 * value / stats["total"] if stats["total"] else 0
        if delta > 0:
            parts.append(pie_slice(px, py, 145, angle, angle + delta, color))
        angle += delta
    legend_x, legend_y = 1085, 410
    for idx, (label, value, color) in enumerate(slices):
        y = legend_y + idx * 42
        pct = 100 * value / stats["total"] if stats["total"] else 0
        parts.append(rect(legend_x, y, 16, 16, color))
        parts.append(text(legend_x + 24, y + 12, f"{label} ({value})", 11, anchor="start"))
        parts.append(text(legend_x + 24, y + 28, f"{pct:.1f}%", 10, anchor="start"))

    # Scatter plot TOA vs frequency
    sx, sy, sw, sh = 90, 590, 360, 260
    parts.append(text(sx + sw / 2, sy - 30, "TOA vs Frequency", 15, "bold"))
    parts.append(text(sx + sw / 2, sy - 12, "(Coloured by Result)", 12, "bold"))
    parts.append(rect(sx, sy, sw, sh, "#ffffff", "#777"))
    for i in range(1, 5):
        parts.append(line(sx, sy + i * sh / 5, sx + sw, sy + i * sh / 5, "#ececec"))
        parts.append(line(sx + i * sw / 5, sy, sx + i * sw / 5, sy + sh, "#ececec"))
    for row in rows:
        toa = float(row["TOA_ns"])
        freq = float(row["Freq_MHz"])
        truth = row["Ground_Truth"]
        predicted_value = row.get("Predicted_Cluster", row.get("Cluster", ""))
        predicted = mapping.get(predicted_value, predicted_value)
        is_correct = truth != "Noise" and predicted == truth
        x = scale(toa, 0, 1, sx + 8, sx + sw - 8)
        y = scale(freq, 0, 1, sy + sh - 8, sy + 8)
        if truth == "Noise":
            parts.append(text(x, y + 3, "x", 9, "normal", "#ff9800"))
        elif is_correct and truth == "Emitter_1":
            parts.append(circle(x, y, 2.2, "#1e88e5", opacity=0.65))
        elif is_correct and truth == "Emitter_2":
            parts.append(circle(x, y, 2.2, "#43a047", opacity=0.65))
        else:
            parts.append(text(x, y + 3, "x", 10, "bold", "#f44336"))
    parts.append(text(sx + sw / 2, sy + sh + 35, "TOA_ns (normalised)", 12))
    parts.append(text(sx - 48, sy + sh / 2, "Freq_MHz (normalised)", 12))
    legend = [
        ("Emitter_1 correct", "#1e88e5", "circle"),
        ("Emitter_2 correct", "#43a047", "circle"),
        ("Noise pts", "#ff9800", "x"),
        ("Non-noise mismatch", "#f44336", "x"),
    ]
    for idx, (label, color, marker) in enumerate(legend):
        y = sy + 14 + idx * 18
        if marker == "circle":
            parts.append(circle(sx + sw - 115, y - 4, 4, color))
        else:
            parts.append(text(sx + sw - 115, y, "x", 12, "bold", color))
        parts.append(text(sx + sw - 103, y, label, 10, anchor="start"))

    # Mismatch breakdown
    mx, my, mw, mh = 555, 590, 345, 260
    parts.append(text(mx + mw / 2, my - 18, "Mismatch Breakdown by Type", 15, "bold"))
    parts.append(rect(mx, my, mw, mh, "#ffffff", "#777"))
    breakdown = [
        ("Emitter_1 ->\nEmitter_2\n(wrong)", stats["emitter_1_wrong"], "#ff5252"),
        ("Emitter_2 ->\nEmitter_1\n(wrong)", stats["emitter_2_wrong"], "#ff5252"),
        ("Noise ->\nCluster 0\n(absorbed)", stats["noise_by_cluster"].get("0", 0), "#ff9800"),
        ("Noise ->\nCluster 1\n(absorbed)", stats["noise_by_cluster"].get("1", 0), "#ff9800"),
    ]
    max_mismatch = max([value for _, value, _ in breakdown] + [1])
    for idx, (label, value, color) in enumerate(breakdown):
        bar_h = (mh - 45) * value / max_mismatch
        x = mx + 28 + idx * 78
        parts.append(rect(x, my + mh - bar_h - 25, 54, bar_h, color))
        parts.append(text(x + 27, my + mh - bar_h - 35, value, 15, "bold"))
        for line_idx, chunk in enumerate(label.split("\n")):
            parts.append(text(x + 27, my + mh + 12 + line_idx * 13, chunk, 10))

    # Summary table
    tx, ty, tw = 970, 590, 360
    parts.append(text(tx + tw / 2, ty - 18, "Summary Statistics", 15, "bold"))
    rows_table = [
        ("Total Points", f"{stats['total']:,}"),
        ("Cluster 0 -> Emitter_1", f"Majority: {matrix['Emitter_1']['Emitter_1']}/{stats['cluster_counts'].get('0', 0)}"),
        ("Cluster 1 -> Emitter_2", f"Majority: {matrix['Emitter_2']['Emitter_2']}/{stats['cluster_counts'].get('1', 0)}"),
        ("Correctly Classified", f"{stats['correct']:,}"),
        ("Total Mismatches", f"{stats['mismatch']:,}"),
        ("Overall Accuracy", f"{stats['overall_accuracy']:.2f}%"),
        ("Non-Noise Accuracy", f"{stats['non_noise_accuracy']:.2f}%"),
        ("Non-Noise Mismatches", f"{stats['non_noise_mismatch']:,}"),
        ("Noise Mismatches", f"{stats['noise_total']:,} (all noise pts)"),
        ("Noise % of Mismatches", f"{stats['noise_mismatch_pct']:.1f}%"),
    ]
    row_h = 32
    parts.append(rect(tx, ty, tw, row_h, "#1565c0"))
    parts.append(text(tx + 95, ty + 21, "Metric", 12, "bold", "white"))
    parts.append(text(tx + 270, ty + 21, "Value", 12, "bold", "white"))
    for i, (metric, value) in enumerate(rows_table):
        y = ty + row_h * (i + 1)
        parts.append(rect(tx, y, tw, row_h, "#e3f2fd" if i % 2 == 0 else "#f8fbff", "#d0d7de"))
        parts.append(text(tx + 20, y + 21, metric, 11, anchor="start"))
        parts.append(text(tx + 210, y + 21, value, 11, anchor="start"))

    parts.append("</svg>")
    return "\n".join(parts)


def wrap_svg(width, height, title, body):
    return "\n".join([
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}">',
        rect(0, 0, width, height, "#ffffff"),
        text(width / 2, 32, title, 18, "bold"),
        *body,
        "</svg>",
    ])


def draw_confusion_svg(rows, mapping):
    truth_labels, pred_labels, matrix = confusion_matrix(rows, mapping)
    x0, y0, cell = 170, 70, 145
    body = []
    max_count = max(matrix[t][p] for t in truth_labels for p in pred_labels)

    for i, truth in enumerate(truth_labels):
        body.append(text(x0 - 18, y0 + i * cell + cell / 2 + 5, truth, 12, anchor="end"))
        for j, pred in enumerate(pred_labels):
            count = matrix[truth][pred]
            fill = "#f4f8ff"
            if count > 0:
                blue = int(230 - 150 * count / max_count)
                fill = f"rgb({blue},{min(255, blue + 35)},190)"
            body.append(rect(x0 + j * cell, y0 + i * cell, cell, cell, fill, "white", 2))
            body.append(text(
                x0 + j * cell + cell / 2,
                y0 + i * cell + cell / 2 + 6,
                count,
                18,
                "bold",
                "white" if count > max_count * 0.35 else "#123",
            ))

    for j, pred in enumerate(pred_labels):
        body.append(text(x0 + j * cell + cell / 2, y0 + len(truth_labels) * cell + 25, pred, 12))
    body.append(text(x0 + cell, y0 + len(truth_labels) * cell + 55, "Predicted Label", 12, "bold"))
    body.append(rotated_text(32, y0 + cell * 1.5, "Ground Truth Label", 12, "bold"))
    return wrap_svg(700, 600, "Confusion Matrix", body)


def draw_correct_mismatch_svg(stats):
    bx, by, bw, bh = 55, 65, 430, 330
    body = []
    groups = [
        ("All Points", stats["correct"], stats["mismatch"], stats["total"]),
        ("Non-Noise", stats["non_noise_correct"], stats["non_noise_mismatch"], stats["non_noise_total"]),
        ("Noise Points", stats["noise_correct"], stats["noise_mismatch"], stats["noise_total"]),
    ]
    max_bar = max(g[3] for g in groups)

    for grid in range(0, 5):
        gy = by + bh - grid * bh / 4
        body.append(line(bx, gy, bx + bw, gy, "#e5e7eb"))

    bar_w = 62
    for idx, (name, correct, mismatch, total) in enumerate(groups):
        base_x = bx + 60 + idx * 130
        correct_h = bh * correct / max_bar if max_bar else 0
        mismatch_h = bh * mismatch / max_bar if max_bar else 0
        body.append(rect(base_x, by + bh - correct_h, bar_w, correct_h, "#2196f3"))
        body.append(rect(base_x, by + bh - correct_h - mismatch_h, bar_w, mismatch_h, "#ff5252"))
        if correct:
            body.append(text(base_x + bar_w / 2, by + bh - correct_h / 2 + 5, correct, 12, "bold", "white"))
        if mismatch:
            body.append(text(base_x + bar_w / 2, by + bh - correct_h - mismatch_h / 2 + 5, mismatch, 12, "bold", "white"))
        pct = 100 * correct / total if total else 0
        body.append(text(base_x + bar_w / 2, by + bh - correct_h - mismatch_h - 10, f"{pct:.1f}%", 12, "bold"))
        body.append(text(base_x + bar_w / 2, by + bh + 22, name, 11))
        body.append(text(base_x + bar_w / 2, by + bh + 39, f"({total})", 10))

    body.append(rect(370, 62, 14, 14, "#2196f3"))
    body.append(text(390, 74, "Correct", 11, anchor="start"))
    body.append(rect(370, 82, 14, 14, "#ff5252"))
    body.append(text(390, 94, "Mismatch", 11, anchor="start"))
    return wrap_svg(540, 470, "Correct vs Mismatch by Group", body)


def draw_pie_svg(stats):
    cx, cy, r = 250, 210, 145
    body = []
    slices = [
        ("Correct", stats["correct"], "#4caf50"),
        ("Non-noise mismatches", stats["non_noise_mismatch"], "#ff9800"),
        ("Noise mismatches", stats["noise_mismatch"], "#f44336"),
    ]
    angle = -90
    for _, value, color in slices:
        delta = 360 * value / stats["total"] if stats["total"] else 0
        if delta > 0:
            body.append(pie_slice(cx, cy, r, angle, angle + delta, color))
        angle += delta

    body.append(text(cx, 55, f"({stats['total']} total)", 13, "bold"))
    legend_x, legend_y = 135, 380
    for idx, (label, value, color) in enumerate(slices):
        y = legend_y + idx * 38
        pct = 100 * value / stats["total"] if stats["total"] else 0
        body.append(rect(legend_x, y, 16, 16, color))
        body.append(text(legend_x + 24, y + 12, f"{label} ({value}) - {pct:.1f}%", 11, anchor="start"))
    return wrap_svg(520, 520, "Mismatch Composition", body)


def draw_scatter_svg(rows, mapping):
    sx, sy, sw, sh = 95, 75, 450, 320
    body = [rect(sx, sy, sw, sh, "#ffffff", "#777")]
    toa_values = [float(row["TOA_ns"]) for row in rows]
    freq_values = [float(row["Freq_MHz"]) for row in rows]
    toa_min, toa_max = min(toa_values), max(toa_values)
    freq_min, freq_max = min(freq_values), max(freq_values)
    for i in range(1, 5):
        body.append(line(sx, sy + i * sh / 5, sx + sw, sy + i * sh / 5, "#ececec"))
        body.append(line(sx + i * sw / 5, sy, sx + i * sw / 5, sy + sh, "#ececec"))

    for row in rows:
        toa = float(row["TOA_ns"])
        freq = float(row["Freq_MHz"])
        truth = row["Ground_Truth"]
        predicted_value = row.get("Predicted_Cluster", row.get("Cluster", ""))
        predicted = mapping.get(predicted_value, predicted_value)
        is_correct = truth != "Noise" and predicted == truth
        x = scale(toa, toa_min, toa_max, sx + 8, sx + sw - 8)
        y = scale(freq, freq_min, freq_max, sy + sh - 8, sy + 8)
        if truth == "Noise":
            body.append(text(x, y + 3, "x", 9, "normal", "#ff9800"))
        elif is_correct and truth == "Emitter_1":
            body.append(circle(x, y, 2.2, "#1e88e5", opacity=0.65))
        elif is_correct and truth == "Emitter_2":
            body.append(circle(x, y, 2.2, "#43a047", opacity=0.65))
        else:
            body.append(text(x, y + 3, "x", 10, "bold", "#f44336"))

    body.append(text(sx + sw / 2, sy + sh + 35, "TOA_ns", 12))
    body.append(rotated_text(28, sy + sh / 2, "Freq_MHz", 12))
    legend = [
        ("Emitter_1 correct", "#1e88e5", "circle"),
        ("Emitter_2 correct", "#43a047", "circle"),
        ("Noise pts", "#ff9800", "x"),
        ("Non-noise mismatch", "#f44336", "x"),
    ]
    legend_x = sx + sw + 42
    for idx, (label, color, marker) in enumerate(legend):
        y = sy + 22 + idx * 24
        if marker == "circle":
            body.append(circle(legend_x, y - 4, 5, color))
        else:
            body.append(text(legend_x, y, "x", 13, "bold", color))
        body.append(text(legend_x + 18, y, label, 12, anchor="start"))
    return wrap_svg(780, 470, "TOA vs Frequency (Coloured by Result)", body)


def draw_breakdown_svg(stats):
    mx, my, mw, mh = 65, 70, 450, 310
    body = [rect(mx, my, mw, mh, "#ffffff", "#777")]
    breakdown = [
        ("Emitter_1\nmismatches", stats["emitter_1_wrong"], "#ff5252"),
        ("Emitter_2\nmismatches", stats["emitter_2_wrong"], "#ff5252"),
        ("Noise ->\nEmitter_1", stats["noise_by_cluster"].get("Emitter_1", 0), "#ff9800"),
        ("Noise ->\nEmitter_2", stats["noise_by_cluster"].get("Emitter_2", 0), "#ff9800"),
    ]
    max_mismatch = max([value for _, value, _ in breakdown] + [1])
    for idx, (label, value, color) in enumerate(breakdown):
        bar_h = (mh - 55) * value / max_mismatch
        x = mx + 35 + idx * 100
        body.append(rect(x, my + mh - bar_h - 28, 64, bar_h, color))
        body.append(text(x + 32, my + mh - bar_h - 38, value, 15, "bold"))
        for line_idx, chunk in enumerate(label.split("\n")):
            body.append(text(x + 32, my + mh + 12 + line_idx * 13, chunk, 10))
    return wrap_svg(580, 480, "Mismatch Breakdown by Type", body)


def draw_table_svg(stats):
    body = []
    rows_table = [
        ("Total Points", f"{stats['total']:,}"),
        ("Predicted Emitter_1", f"Correct: {stats['cluster_majorities'].get('Emitter_1', '')}"),
        ("Predicted Emitter_2", f"Correct: {stats['cluster_majorities'].get('Emitter_2', '')}"),
        ("Predicted Noise", f"Correct: {stats['cluster_majorities'].get('Noise', '')}"),
        ("Correctly Classified", f"{stats['correct']:,}"),
        ("Total Mismatches", f"{stats['mismatch']:,}"),
        ("Overall Accuracy", f"{stats['overall_accuracy']:.2f}%"),
        ("Non-Noise Accuracy", f"{stats['non_noise_accuracy']:.2f}%"),
        ("Non-Noise Mismatches", f"{stats['non_noise_mismatch']:,}"),
        ("Noise Mismatches", f"{stats['noise_mismatch']:,}"),
        ("Noise % of Mismatches", f"{stats['noise_mismatch_pct']:.1f}%"),
    ]
    tx, ty, tw, row_h = 55, 65, 470, 34
    body.append(rect(tx, ty, tw, row_h, "#1565c0"))
    body.append(text(tx + 130, ty + 22, "Metric", 12, "bold", "white"))
    body.append(text(tx + 345, ty + 22, "Value", 12, "bold", "white"))
    for i, (metric, value) in enumerate(rows_table):
        y = ty + row_h * (i + 1)
        body.append(rect(tx, y, tw, row_h, "#e3f2fd" if i % 2 == 0 else "#f8fbff", "#d0d7de"))
        body.append(text(tx + 20, y + 22, metric, 11, anchor="start"))
        body.append(text(tx + 280, y + 22, value, 11, anchor="start"))
    return wrap_svg(580, 460, "Summary Statistics", body)


def main():
    rows = load_rows(RESULTS)
    OUT_DIR.mkdir(exist_ok=True)

    mapping = best_cluster_mapping(rows)
    cluster_counts = {}
    noise_by_cluster = {}
    correct = 0
    non_noise_total = 0
    non_noise_correct = 0
    emitter_1_wrong = 0
    emitter_2_wrong = 0

    for row in rows:
        cluster = row.get("Predicted_Cluster", row.get("Cluster", ""))
        truth = row["Ground_Truth"]
        predicted = mapping.get(cluster, cluster)
        cluster_counts[cluster] = cluster_counts.get(cluster, 0) + 1

        if truth == "Noise":
            noise_by_cluster[cluster] = noise_by_cluster.get(cluster, 0) + 1
            if predicted == truth:
                correct += 1
            continue

        non_noise_total += 1
        if predicted == truth:
            correct += 1
            non_noise_correct += 1
        elif truth == "Emitter_1":
            emitter_1_wrong += 1
        elif truth == "Emitter_2":
            emitter_2_wrong += 1

    total = len(rows)
    noise_total = sum(noise_by_cluster.values())
    noise_correct = noise_by_cluster.get("Noise", 0)
    noise_mismatch = noise_total - noise_correct
    non_noise_mismatch = non_noise_total - non_noise_correct
    mismatch = total - correct
    stats = {
        "total": total,
        "correct": correct,
        "mismatch": mismatch,
        "non_noise_total": non_noise_total,
        "non_noise_correct": non_noise_correct,
        "non_noise_mismatch": non_noise_mismatch,
        "noise_total": noise_total,
        "noise_correct": noise_correct,
        "noise_mismatch": noise_mismatch,
        "noise_by_cluster": noise_by_cluster,
        "emitter_1_wrong": emitter_1_wrong,
        "emitter_2_wrong": emitter_2_wrong,
        "cluster_counts": cluster_counts,
        "overall_accuracy": 100 * correct / total if total else 0,
        "non_noise_accuracy": 100 * non_noise_correct / non_noise_total if non_noise_total else 0,
        "noise_mismatch_pct": 100 * noise_mismatch / mismatch if mismatch else 0,
    }

    truth_labels, pred_labels, matrix = confusion_matrix(rows, mapping)
    cluster_majorities = {}
    for cluster, label in mapping.items():
        count = sum(
            row.get("Predicted_Cluster", row.get("Cluster", "")) == cluster
            and row["Ground_Truth"] == label
            for row in rows
        )
        cluster_majorities[cluster] = f"{count}/{cluster_counts.get(cluster, 0)}"
    stats["cluster_majorities"] = cluster_majorities

    outputs = {
        PLOT_FILES["confusion"]: draw_confusion_svg(rows, mapping),
        PLOT_FILES["bars"]: draw_correct_mismatch_svg(stats),
        PLOT_FILES["pie"]: draw_pie_svg(stats),
        PLOT_FILES["scatter"]: draw_scatter_svg(rows, mapping),
        PLOT_FILES["breakdown"]: draw_breakdown_svg(stats),
        PLOT_FILES["table"]: draw_table_svg(stats),
    }
    for path, svg in outputs.items():
        path.write_text(svg, encoding="utf-8")

    with SUMMARY_OUT.open("w", encoding="utf-8") as handle:
        handle.write("BIRCH Clustering Evaluation Summary\n")
        handle.write("===================================\n\n")
        handle.write(f"Input: {RESULTS}\n")
        handle.write("Generated SVG files:\n")
        for path in PLOT_FILES.values():
            handle.write(f"- {path}\n")
        handle.write("\n")
        handle.write(f"Cluster mapping: {mapping}\n")
        handle.write(f"Total points: {total}\n")
        handle.write(f"Correctly classified: {correct}\n")
        handle.write(f"Total mismatches: {mismatch}\n")
        handle.write(f"Overall accuracy: {stats['overall_accuracy']:.2f}%\n")
        handle.write(f"Non-noise accuracy: {stats['non_noise_accuracy']:.2f}%\n")
        handle.write(f"Noise points: {noise_total}\n")
        handle.write(f"Correctly detected noise: {noise_correct}\n")
        handle.write(f"Noise mismatches: {noise_mismatch}\n")
        handle.write(f"Noise % of mismatches: {stats['noise_mismatch_pct']:.1f}%\n\n")
        handle.write("Confusion Matrix\n")
        handle.write("," + ",".join(pred_labels) + "\n")
        for truth in truth_labels:
            handle.write(truth + "," + ",".join(str(matrix[truth][pred]) for pred in pred_labels) + "\n")

    for path in PLOT_FILES.values():
        print(f"Created {path}")
    print(f"Created {SUMMARY_OUT}")


if __name__ == "__main__":
    main()
