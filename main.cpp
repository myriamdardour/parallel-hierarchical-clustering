#include <iostream>
#include <vector>
#include "datacleaning.h"

int main() {
    // 1. Create the parallel structures
    std::vector<Song> original_dataset;
    std::vector<std::vector<double>> math_matrix;

    // 2. Load and clean the data
    std::string filename = "spotify_tracks.csv";
    
    if (loadAndCleanData(filename, original_dataset, math_matrix)) {
        // Just to prove it worked, print the first song!
        std::cout << "\nFirst song loaded: " << original_dataset[0].track_name 
                  << " by " << original_dataset[0].artist_name << std::endl;
        
        std::cout << "Math vector size for first song: " 
                  << math_matrix[0].size() << " features.\n";
    }

    // Step 3 will be normalizing math_matrix
    // Step 4 will be clustering

    return 0;
}