#include <algorithm>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using Point = std::vector<double>;

class Dataset {
public:
    explicit Dataset(std::vector<Point> points) : points_(std::move(points)) {
        validate();
    }

    static Dataset fromCsv(const std::string& path) {
        std::ifstream input(path.c_str());
        if (!input) {
            throw std::runtime_error("Could not open input file: " + path);
        }

        std::vector<Point> points;
        std::string line;
        std::size_t lineNumber = 0;
        while (std::getline(input, line)) {
            ++lineNumber;
            if (line.empty()) {
                continue;
            }

            std::stringstream lineStream(line);
            std::string cell;
            Point point;
            while (std::getline(lineStream, cell, ',')) {
                try {
                    std::size_t parsed = 0;
                    const double value = std::stod(cell, &parsed);
                    const bool hasTrailingText =
                        cell.find_first_not_of(" \t\r\n", parsed) !=
                        std::string::npos;
                    if (hasTrailingText || !std::isfinite(value)) {
                        throw std::invalid_argument("not a finite number");
                    }
                    point.push_back(value);
                } catch (const std::exception&) {
                    throw std::runtime_error(
                        "Invalid numeric value at CSV line " +
                        std::to_string(lineNumber) + ": " + cell);
                }
            }
            points.push_back(std::move(point));
        }
        return Dataset(std::move(points));
    }

    const std::vector<Point>& points() const { return points_; }
    std::size_t size() const { return points_.size(); }
    std::size_t dimensions() const { return points_.front().size(); }

private:
    void validate() const {
        if (points_.empty()) {
            throw std::invalid_argument("The dataset must contain at least one point.");
        }
        const std::size_t dimensions = points_.front().size();
        if (dimensions == 0) {
            throw std::invalid_argument("Points must have at least one dimension.");
        }
        for (const Point& point : points_) {
            if (point.size() != dimensions) {
                throw std::invalid_argument(
                    "All points must have the same number of dimensions.");
            }
        }
    }

    std::vector<Point> points_;
};

class Distance {
public:
    static double squaredEuclidean(const Point& a, const Point& b) {
        double result = 0.0;
        for (std::size_t dimension = 0; dimension < a.size(); ++dimension) {
            const double difference = a[dimension] - b[dimension];
            result += difference * difference;
        }
        return result;
    }

    static double euclidean(const Point& a, const Point& b) {
        return std::sqrt(squaredEuclidean(a, b));
    }

    static double norm(const Point& vector) {
        double squaredNorm = 0.0;
        for (double value : vector) {
            squaredNorm += value * value;
        }
        return std::sqrt(squaredNorm);
    }
};

struct ClusteringResult {
    std::vector<std::size_t> labels;
    std::vector<Point> centers;
    double sumSquaredError = std::numeric_limits<double>::infinity();
};

class KMeans {
public:
    KMeans(std::size_t clusters, std::size_t maxIterations = 300,
           std::size_t restarts = 10, unsigned int seed = 42)
        : clusters_(clusters),
          maxIterations_(maxIterations),
          restarts_(restarts),
          seed_(seed) {
        if (clusters_ < 2 || maxIterations_ == 0 || restarts_ == 0) {
            throw std::invalid_argument(
                "KMeans requires k >= 2, iterations > 0, and restarts > 0.");
        }
    }

    ClusteringResult fit(const Dataset& dataset) const {
        if (clusters_ > dataset.size()) {
            throw std::invalid_argument(
                "The number of clusters cannot exceed the number of points.");
        }

        ClusteringResult best;
        for (std::size_t restart = 0; restart < restarts_; ++restart) {
            std::mt19937 generator(seed_ + static_cast<unsigned int>(restart));
            ClusteringResult candidate = fitOnce(dataset, generator);
            if (candidate.sumSquaredError < best.sumSquaredError) {
                best = std::move(candidate);
            }
        }
        return best;
    }

private:
    std::vector<Point> initializeKMeansPlusPlus(
        const Dataset& dataset, std::mt19937& generator) const {
        const std::vector<Point>& points = dataset.points();
        std::uniform_int_distribution<std::size_t> firstCenter(0, points.size() - 1);
        std::vector<Point> centers(1, points[firstCenter(generator)]);
        std::vector<double> nearestDistance(points.size());

        while (centers.size() < clusters_) {
            double totalDistance = 0.0;
            for (std::size_t i = 0; i < points.size(); ++i) {
                double distance = std::numeric_limits<double>::infinity();
                for (const Point& center : centers) {
                    distance = std::min(
                        distance, Distance::squaredEuclidean(points[i], center));
                }
                nearestDistance[i] = distance;
                totalDistance += distance;
            }

            if (totalDistance == 0.0) {
                for (const Point& point : points) {
                    const bool alreadyCenter = std::find(
                        centers.begin(), centers.end(), point) != centers.end();
                    if (!alreadyCenter) {
                        centers.push_back(point);
                        break;
                    }
                }
                if (centers.size() < clusters_) {
                    centers.push_back(points[centers.size() % points.size()]);
                }
                continue;
            }

            std::uniform_real_distribution<double> choice(0.0, totalDistance);
            const double target = choice(generator);
            double cumulative = 0.0;
            std::size_t selected = points.size() - 1;
            for (std::size_t i = 0; i < points.size(); ++i) {
                cumulative += nearestDistance[i];
                if (cumulative >= target) {
                    selected = i;
                    break;
                }
            }
            centers.push_back(points[selected]);
        }
        return centers;
    }

    static std::size_t nearestCenter(const Point& point,
                                     const std::vector<Point>& centers) {
        std::size_t bestCluster = 0;
        double bestDistance = Distance::squaredEuclidean(point, centers[0]);
        for (std::size_t cluster = 1; cluster < centers.size(); ++cluster) {
            const double distance =
                Distance::squaredEuclidean(point, centers[cluster]);
            if (distance < bestDistance) {
                bestDistance = distance;
                bestCluster = cluster;
            }
        }
        return bestCluster;
    }

    static bool repairEmptyClusters(const Dataset& dataset,
                                    const std::vector<Point>& centers,
                                    std::vector<std::size_t>& labels) {
        std::vector<std::size_t> counts(centers.size(), 0);
        bool repaired = false;
        for (std::size_t label : labels) {
            ++counts[label];
        }

        for (std::size_t empty = 0; empty < centers.size(); ++empty) {
            if (counts[empty] != 0) {
                continue;
            }

            std::size_t farthest = labels.size();
            double farthestDistance = -1.0;
            for (std::size_t i = 0; i < labels.size(); ++i) {
                if (counts[labels[i]] <= 1) {
                    continue;
                }
                const double distance = Distance::squaredEuclidean(
                    dataset.points()[i], centers[labels[i]]);
                if (distance > farthestDistance) {
                    farthestDistance = distance;
                    farthest = i;
                }
            }
            if (farthest == labels.size()) {
                throw std::runtime_error("Unable to repair an empty cluster.");
            }
            --counts[labels[farthest]];
            labels[farthest] = empty;
            ++counts[empty];
            repaired = true;
        }
        return repaired;
    }

    static std::vector<Point> calculateCenters(
        const Dataset& dataset, const std::vector<std::size_t>& labels,
        std::size_t clusters) {
        std::vector<Point> centers(
            clusters, Point(dataset.dimensions(), 0.0));
        std::vector<std::size_t> counts(clusters, 0);
        for (std::size_t i = 0; i < dataset.size(); ++i) {
            ++counts[labels[i]];
            for (std::size_t dimension = 0;
                 dimension < dataset.dimensions(); ++dimension) {
                centers[labels[i]][dimension] += dataset.points()[i][dimension];
            }
        }
        for (std::size_t cluster = 0; cluster < clusters; ++cluster) {
            for (double& coordinate : centers[cluster]) {
                coordinate /= static_cast<double>(counts[cluster]);
            }
        }
        return centers;
    }

    static double calculateSse(const Dataset& dataset,
                               const std::vector<std::size_t>& labels,
                               const std::vector<Point>& centers) {
        double sum = 0.0;
        for (std::size_t i = 0; i < dataset.size(); ++i) {
            sum += Distance::squaredEuclidean(
                dataset.points()[i], centers[labels[i]]);
        }
        return sum;
    }

    ClusteringResult fitOnce(const Dataset& dataset,
                             std::mt19937& generator) const {
        std::vector<Point> centers =
            initializeKMeansPlusPlus(dataset, generator);
        std::vector<std::size_t> labels(dataset.size(), clusters_);

        for (std::size_t iteration = 0; iteration < maxIterations_; ++iteration) {
            bool changed = false;
            for (std::size_t i = 0; i < dataset.size(); ++i) {
                const std::size_t label =
                    nearestCenter(dataset.points()[i], centers);
                changed = changed || label != labels[i];
                labels[i] = label;
            }

            changed = repairEmptyClusters(dataset, centers, labels) || changed;
            const std::vector<Point> newCenters =
                calculateCenters(dataset, labels, clusters_);
            if (!changed || newCenters == centers) {
                centers = newCenters;
                break;
            }
            centers = newCenters;
        }

        ClusteringResult result;
        result.labels = std::move(labels);
        result.centers = std::move(centers);
        result.sumSquaredError =
            calculateSse(dataset, result.labels, result.centers);
        return result;
    }

    std::size_t clusters_;
    std::size_t maxIterations_;
    std::size_t restarts_;
    unsigned int seed_;
};

struct SDbwScore {
    double scatter = 0.0;
    double densityBetween = 0.0;
    double total = 0.0;
};

class SDbwIndex {
public:
    SDbwScore evaluate(const Dataset& dataset,
                       const ClusteringResult& clustering) const {
        validate(dataset, clustering);

        const std::size_t clusters = clustering.centers.size();
        const Point overallVariance =
            variance(dataset.points(), mean(dataset.points()));
        const double overallVarianceNorm = Distance::norm(overallVariance);

        std::vector<double> clusterVarianceNorms;
        clusterVarianceNorms.reserve(clusters);
        for (std::size_t cluster = 0; cluster < clusters; ++cluster) {
            std::vector<Point> points = pointsInCluster(dataset, clustering, cluster);
            Point currentVariance =
                variance(points, clustering.centers[cluster]);
            clusterVarianceNorms.push_back(Distance::norm(currentVariance));
        }

        double scatter = 0.0;
        if (overallVarianceNorm > 0.0) {
            for (double clusterNorm : clusterVarianceNorms) {
                scatter += clusterNorm / overallVarianceNorm;
            }
            scatter /= static_cast<double>(clusters);
        }

        double meanVarianceNorm = 0.0;
        for (double clusterNorm : clusterVarianceNorms) {
            meanVarianceNorm += clusterNorm;
        }
        // ////////////////////////////////////////////
        const double densityRadius =
            std::sqrt(meanVarianceNorm / static_cast<double>(clusters));

        double densityBetween = 0.0;
        for (std::size_t first = 0; first < clusters; ++first) {
            for (std::size_t second = 0; second < clusters; ++second) {
                if (first == second) {
                    continue;
                }
                Point midpoint(dataset.dimensions(), 0.0);
                for (std::size_t dimension = 0;
                     dimension < dataset.dimensions(); ++dimension) {
                    midpoint[dimension] =
                        (clustering.centers[first][dimension] +
                         clustering.centers[second][dimension]) /
                        2.0;
                }

                const std::size_t middleDensity = pairDensity(
                    dataset, clustering, midpoint, densityRadius, first, second);
                const std::size_t firstDensity = clusterDensity(
                    dataset, clustering, clustering.centers[first],
                    densityRadius, first);
                const std::size_t secondDensity = clusterDensity(
                    dataset, clustering, clustering.centers[second],
                    densityRadius, second);
                const std::size_t denominator =
                    std::max(firstDensity, secondDensity);
                if (denominator > 0) {
                    densityBetween += static_cast<double>(middleDensity) /
                                      static_cast<double>(denominator);
                }
            }
        }
        densityBetween /=
            static_cast<double>(clusters * (clusters - 1));

        SDbwScore score;
        score.scatter = scatter;
        score.densityBetween = densityBetween;
        score.total = scatter + densityBetween;
        return score;
    }

private:
    static void validate(const Dataset& dataset,
                         const ClusteringResult& clustering) {
        if (clustering.labels.size() != dataset.size() ||
            clustering.centers.size() < 2) {
            throw std::invalid_argument("Invalid clustering result.");
        }
    }

    static Point mean(const std::vector<Point>& points) {
        Point result(points.front().size(), 0.0);
        for (const Point& point : points) {
            for (std::size_t dimension = 0; dimension < point.size(); ++dimension) {
                result[dimension] += point[dimension];
            }
        }
        for (double& coordinate : result) {
            coordinate /= static_cast<double>(points.size());
        }
        return result;
    }

    static Point variance(const std::vector<Point>& points, const Point& center) {
        Point result(center.size(), 0.0);
        for (const Point& point : points) {
            for (std::size_t dimension = 0; dimension < point.size(); ++dimension) {
                const double difference = point[dimension] - center[dimension];
                result[dimension] += difference * difference;
            }
        }
        for (double& value : result) {
            value /= static_cast<double>(points.size());
        }
        return result;
    }

    static std::vector<Point> pointsInCluster(
        const Dataset& dataset, const ClusteringResult& clustering,
        std::size_t cluster) {
        std::vector<Point> points;
        for (std::size_t i = 0; i < dataset.size(); ++i) {
            if (clustering.labels[i] == cluster) {
                points.push_back(dataset.points()[i]);
            }
        }
        if (points.empty()) {
            throw std::invalid_argument("S_Dbw is undefined for empty clusters.");
        }
        return points;
    }

    static std::size_t pairDensity(const Dataset& dataset,
                                   const ClusteringResult& clustering,
                                   const Point& center, double radius,
                                   std::size_t firstCluster,
                                   std::size_t secondCluster) {
        std::size_t count = 0;
        for (std::size_t i = 0; i < dataset.size(); ++i) {
            if (clustering.labels[i] != firstCluster &&
                clustering.labels[i] != secondCluster) {
                continue;
            }
            if (Distance::euclidean(dataset.points()[i], center) <= radius) {
                ++count;
            }
        }
        return count;
    }

    static std::size_t clusterDensity(const Dataset& dataset,
                                      const ClusteringResult& clustering,
                                      const Point& center, double radius,
                                      std::size_t cluster) {
        std::size_t count = 0;
        for (std::size_t i = 0; i < dataset.size(); ++i) {
            if (clustering.labels[i] == cluster &&
                Distance::euclidean(dataset.points()[i], center) <= radius) {
                ++count;
            }
        }
        return count;
    }
};

struct PartitionCandidate {
    std::size_t clusters = 0;
    ClusteringResult clustering;
    SDbwScore score;
};

class OptimalPartitionFinder {
public:
    OptimalPartitionFinder(std::size_t minClusters, std::size_t maxClusters,
                           std::size_t restarts, unsigned int seed)
        : minClusters_(minClusters),
          maxClusters_(maxClusters),
          restarts_(restarts),
          seed_(seed) {
        if (minClusters_ < 2 || minClusters_ > maxClusters_) {
            throw std::invalid_argument(
                "Expected 2 <= min_k <= max_k.");
        }
    }

    std::vector<PartitionCandidate> evaluate(const Dataset& dataset) const {
        if (maxClusters_ > dataset.size()) {
            throw std::invalid_argument(
                "max_k cannot exceed the number of points.");
        }

        std::vector<PartitionCandidate> candidates;
        SDbwIndex index;
        for (std::size_t clusters = minClusters_; clusters <= maxClusters_;
             ++clusters) {
            KMeans model(clusters, 300, restarts_,
                         seed_ + static_cast<unsigned int>(clusters * 1009));
            PartitionCandidate candidate;
            candidate.clusters = clusters;
            candidate.clustering = model.fit(dataset);
            candidate.score = index.evaluate(dataset, candidate.clustering);
            candidates.push_back(std::move(candidate));
        }
        return candidates;
    }

    static const PartitionCandidate& best(
        const std::vector<PartitionCandidate>& candidates) {
        if (candidates.empty()) {
            throw std::invalid_argument("No partition candidates were evaluated.");
        }
        return *std::min_element(
            candidates.begin(), candidates.end(),
            [](const PartitionCandidate& left, const PartitionCandidate& right) {
                return left.score.total < right.score.total;
            });
    }

private:
    std::size_t minClusters_;
    std::size_t maxClusters_;
    std::size_t restarts_;
    unsigned int seed_;
};

class AssignmentWriter {
public:
    static void write(const std::string& path, const Dataset& dataset,
                      const ClusteringResult& clustering) {
        std::ofstream output(path.c_str());
        if (!output) {
            throw std::runtime_error("Could not open output file: " + path);
        }
        for (std::size_t dimension = 0; dimension < dataset.dimensions();
             ++dimension) {
            output << "feature_" << (dimension + 1) << ',';
        }
        output << "cluster\n";

        output << std::setprecision(15);
        for (std::size_t i = 0; i < dataset.size(); ++i) {
            for (double value : dataset.points()[i]) {
                output << value << ',';
            }
            output << clustering.labels[i] << '\n';
        }
    }
};

static std::size_t parsePositiveInteger(const char* value,
                                        const std::string& name) {
    try {
        std::size_t parsed = 0;
        const unsigned long number = std::stoul(value, &parsed);
        if (parsed != std::string(value).size() || number == 0) {
            throw std::invalid_argument("not positive");
        }
        return static_cast<std::size_t>(number);
    } catch (const std::exception&) {
        throw std::invalid_argument(name + " must be a positive integer.");
    }
}

int main(int argc, char* argv[]) {
    if (argc < 4 || argc > 7) {
        std::cerr
            << "Usage: " << argv[0]
            << " <data.csv> <min_k> <max_k> [restarts=10] [seed=42]"
               " [assignments.csv]\n";
        return 1;
    }

    try {
        const Dataset dataset = Dataset::fromCsv(argv[1]);
        const std::size_t minClusters = parsePositiveInteger(argv[2], "min_k");
        const std::size_t maxClusters = parsePositiveInteger(argv[3], "max_k");
        const std::size_t restarts =
            argc >= 5 ? parsePositiveInteger(argv[4], "restarts") : 10;
        const unsigned int seed = argc >= 6
                                      ? static_cast<unsigned int>(
                                            parsePositiveInteger(argv[5], "seed"))
                                      : 42;
        const std::string outputPath =
            argc >= 7 ? argv[6] : "assignments.csv";

        const OptimalPartitionFinder finder(
            minClusters, maxClusters, restarts, seed);
        const std::vector<PartitionCandidate> candidates =
            finder.evaluate(dataset);

        std::cout << std::fixed << std::setprecision(6);
        std::cout << "k\tScat\tDens_bw\tS_Dbw\tSSE\n";
        for (const PartitionCandidate& candidate : candidates) {
            std::cout << candidate.clusters << '\t'
                      << candidate.score.scatter << '\t'
                      << candidate.score.densityBetween << '\t'
                      << candidate.score.total << '\t'
                      << candidate.clustering.sumSquaredError << '\n';
        }

        const PartitionCandidate& best = OptimalPartitionFinder::best(candidates);
        AssignmentWriter::write(outputPath, dataset, best.clustering);
        std::cout << "\nOptimal partition: k=" << best.clusters
                  << " (minimum S_Dbw=" << best.score.total << ")\n"
                  << "Assignments written to: " << outputPath << '\n';
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
