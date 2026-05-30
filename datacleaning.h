#ifndef DATACLEANING_H
#define DATACLEANING_H

#include <string>
#include <vector>

// This struct holds ONLY the human-readable info.
// The math matrix will hold the numbers.
struct Song {
    std::string track_id;
    std::string track_name;
    std::string artist_name;
    std::string album_name;
    std::string genre;
    
    int cluster_label = -1; // To be filled in later by the clustering algorithm
};

// Parses a single line of CSV, ignoring commas inside quotes
std::vector<std::string> parseCSVLine(const std::string& line);

// Reads the file, populates the structs, and builds the math matrix
bool loadAndCleanData(const std::string& filename, 
                      std::vector<Song>& out_metadata, 
                      std::vector<std::vector<double>>& out_math_matrix);

#endif // DATACLEANING_H