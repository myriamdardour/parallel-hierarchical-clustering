#include <iostream>
#include <vector>
#include <chrono>
#include <cassert>
#include <cmath>
#include <iomanip>
#include "hc.h"
#include "datacleaning.h"
#include "normalization.h"

template<typename Func>
double time_ms(Func f) {
    auto t0 = std::chrono::high_resolution_clock::now();
    f();
    auto t1 = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(t1 - t0).count();
}

bool dendrograms_equal(const std::vector<Merge>& a, const std::vector<Merge>& b) {
    if (a.size() != b.size()) return false;
    for (size_t s = 0; s < a.size(); ++s) {
        if (a[s].i != b[s].i || a[s].j != b[s].j)    return false;
        if (std::abs(a[s].dist - b[s].dist) > 1e-9)   return false;
    }
    return true;
}

std::vector<Point> load_spotify_points(int N_limit) {
    std::vector<Song> songs;
    std::vector<std::vector<double>> math_matrix;
    if (!loadAndCleanData("spotify_tracks.csv", songs, math_matrix)) {
        std::cerr << "ERROR: could not load spotify_tracks.csv\n";
        std::exit(1);
    }
    normalizeMatrix(math_matrix);

    if ((int)math_matrix.size() > N_limit)
        math_matrix.resize(N_limit);

    std::vector<Point> pts;
    pts.reserve(math_matrix.size());
    for (const auto& row : math_matrix)
        pts.push_back({row});
    return pts;
}

//  1. Correctness check  (kept on small synthetic points)

std::vector<Point> make_points(int N) {
    std::vector<Point> pts;
    pts.reserve(N);
    for (int i = 0; i < N; ++i) {
        double x = std::sin(i * 0.1) * 100.0;
        double y = std::cos(i * 0.17) * 100.0;
        pts.push_back({{x, y}});
    }
    return pts;
}

void check_correctness() {
    std::cout << "=== 1. Correctness: ptrad_hac vs naive_hac ===\n";
    for (int N : {4, 20, 50}) {
        auto pts = make_points(N);
        for (Linkage l : {Linkage::SINGLE, Linkage::COMPLETE}) {
            for (int P : {1, 2, 4}) {
                auto ref = naive_hac(pts, l);
                auto got = ptrad_hac(pts, l, P);
                bool ok  = dendrograms_equal(ref, got);
                std::cout << "  N=" << std::setw(3) << N
                          << "  P=" << P
                          << "  linkage=" << (l == Linkage::SINGLE ? "single  " : "complete")
                          << "  " << (ok ? "OK" : "MISMATCH") << "\n";
                assert(ok);
            }
        }
    }
    std::cout << "  All correctness checks passed.\n\n";
}

//  2. Experiment A fixed N=1000, sweep P = 1,2,4,8,12

void experiment_A(const std::vector<Point>& pts) {
    std::cout << "=== Experiment A: fixed N=1000, sweep P (Spotify data) ===\n";
    std::cout << std::fixed << std::setprecision(1);

    // baseline
    std::vector<Merge> ref;
    double t_naive = time_ms([&]{ ref = naive_hac(pts, Linkage::SINGLE); });
    std::cout << "  naive baseline: " << t_naive << " ms\n\n";

    std::cout << std::setw(6)  << "P"
              << std::setw(14) << "ptrad(ms)"
              << std::setw(12) << "speedup"
              << std::setw(10) << "correct"
              << "\n";
    std::cout << std::string(44, '-') << "\n";

    for (int P : {1, 2, 4, 8, 12}) {
        std::vector<Merge> got;
        double t_par = time_ms([&]{ got = ptrad_hac(pts, Linkage::SINGLE, P); });
        double speedup = t_naive / t_par;
        bool ok = dendrograms_equal(ref, got);
        std::cout << std::setw(6)  << P
                  << std::setw(14) << t_par
                  << std::setw(11) << speedup << "x"
                  << std::setw(10) << (ok ? "OK" : "MISMATCH")
                  << "\n";
    }
    std::cout << "\n";
}

//  3. Experiment B  fixed P=8, sweep N = 100,200,500,1000,2000


void experiment_B(const std::vector<Point>& all_pts) {
    std::cout << "=== Experiment B: fixed P=8, sweep N (Spotify data) ===\n";
    std::cout << std::fixed << std::setprecision(1);

    std::cout << std::setw(8)  << "N"
              << std::setw(14) << "naive(ms)"
              << std::setw(14) << "ptrad(ms)"
              << std::setw(12) << "speedup"
              << std::setw(10) << "correct"
              << "\n";
    std::cout << std::string(60, '-') << "\n";

    for (int N : {100, 200, 500, 1000, 2000}) {
        std::vector<Point> pts(all_pts.begin(), all_pts.begin() + N);

        std::vector<Merge> ref, got;
        double t_naive = time_ms([&]{ ref = naive_hac(pts, Linkage::SINGLE); });
        double t_par   = time_ms([&]{ got = ptrad_hac(pts, Linkage::SINGLE, 8); });
        double speedup = t_naive / t_par;
        bool ok = dendrograms_equal(ref, got);

        std::cout << std::setw(8)  << N
                  << std::setw(14) << t_naive
                  << std::setw(14) << t_par
                  << std::setw(11) << speedup << "x"
                  << std::setw(10) << (ok ? "OK" : "MISMATCH")
                  << "\n";
    }
    std::cout << "\n";
}

//  4. Experiment C — full grid: N in {100,200,500,1000,2000} x P in {1, 2, 4, 8, 12, 16, 24, 32, 64}

void experiment_C(const std::vector<Point>& all_pts) {
    std::cout << "=== Experiment C: full grid N x P — speedup table (Spotify data) ===\n";
    std::cout << std::fixed << std::setprecision(2);

    std::vector<int> Ns = {100, 200, 500, 1000, 2000};
    std::vector<int> Ps = {1, 2, 4, 8, 12, 16, 24, 32, 64};

    // header
    std::cout << std::setw(8) << "N";
    for (int P : Ps)
        std::cout << std::setw(10) << ("P=" + std::to_string(P));
    std::cout << "\n" << std::string(58, '-') << "\n";

    for (int N : Ns) {
        std::vector<Point> pts(all_pts.begin(), all_pts.begin() + N);

        // naive baseline for this N
        double t_naive = time_ms([&]{ naive_hac(pts, Linkage::SINGLE); });

        std::cout << std::setw(8) << N;
        for (int P : Ps) {
            double t_par = time_ms([&]{ ptrad_hac(pts, Linkage::SINGLE, P); });
            double speedup = t_naive / t_par;
            std::cout << std::setw(9) << speedup << "x";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

int main() {
    //check_correctness();

    std::cout << "Loading Spotify data (up to 2000 songs)...\n";
    auto all_pts = load_spotify_points(2000);
    std::cout << "Loaded " << all_pts.size() << " points, each with "
              << all_pts[0].features.size() << " features.\n\n";

    // Experiment A needs exactly 1000 points
    std::vector<Point> pts_1000(all_pts.begin(), all_pts.begin() + 1000);

    //experiment_A(pts_1000);
    //experiment_B(all_pts);
    experiment_C(all_pts);

    return 0;
}

//run g++ -std=c++17 -O2 -pthread -I include hc_alg.cpp main.cpp datacleaning.cpp normalization.cpp -o main