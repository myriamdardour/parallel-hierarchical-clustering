//decide how to normalize data: zscore (if normally distrib), squeeze to [0,1], use other distance?

#include "datacleaning.h"
#include <fstream>
#include <iostream>

std::vector<std::string> parseCSVLine(const std::string& line) {
    std::vector<std::string> result;
    std::string current_field;
    bool in_quotes = false;

    for (size_t i = 0; i < line.length(); ++i) {
        char c = line[i];
        
        if (c == '\"') {
            // Toggle the in_quotes state when we see a quotation mark
            in_quotes = !in_quotes; 
        } else if (c == ',' && !in_quotes) {
            // If we see a comma AND we aren't inside quotes, split the field
            result.push_back(current_field);
            current_field.clear();
        } else {
            // Otherwise, add the character to the current field
            current_field += c;
        }
    }
    // Don't forget to add the very last field!
    result.push_back(current_field); 
    
    return result;
}

bool loadAndCleanData(const std::string& filename, 
                      std::vector<Song>& out_metadata, 
                      std::vector<std::vector<double>>& out_math_matrix) {
                          
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filename << std::endl;
        return false;
    }

    std::string line;
    
    // 1. Read and ignore the first line (the column headers)
    if (!std::getline(file, line)) {
        std::cerr << "Error: File is empty." << std::endl;
        return false; 
    }

    // 2. Loop through every subsequent line
    while (std::getline(file, line)) {
        
        // Handle a quirk where Windows saves CSVs with a hidden '\r' at the end of lines
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        // Split the line into columns
        std::vector<std::string> fields = parseCSVLine(line);

        // Based on your CSV, we strictly expect 21 columns.
        if (fields.size() != 21) {
            continue; // Skip this row if it is broken or misaligned
        }

        try {
            // --- Step A: Build the Metadata Struct ---
            Song song;
            song.track_id    = fields[0];
            song.track_name  = fields[1];
            song.artist_name = fields[2];
            song.album_name  = fields[3];
            song.genre       = fields[5];

            // --- Step B: Build the Math Vector ---
            std::vector<double> math_row;
            
            // Extract numeric fields based on their column index
            math_row.push_back(std::stod(fields[4])); // release_year
            math_row.push_back(std::stod(fields[6])); // popularity
            math_row.push_back(std::stod(fields[7])); // duration_ms
            
            // Convert 'explicit' text ("True" / "False") into 1.0 or 0.0
            double is_explicit = (fields[8] == "True" || fields[8] == "true") ? 1.0 : 0.0;
            math_row.push_back(is_explicit);

            // The remaining columns (Indices 9 to 20) are all the audio features
            // (danceability, energy, loudness, etc.)
            for (int i = 9; i <= 20; ++i) {
                math_row.push_back(std::stod(fields[i]));
            }

            // --- Step C: Save to our main data structures ---
            out_metadata.push_back(song);
            out_math_matrix.push_back(math_row);

        } catch (const std::exception& e) {
            // If std::stod() encounters text instead of a number, it lands here.
            // We just ignore the broken row and continue to the next one.
            continue;
        }
    }

    file.close();
    std::cout << "Successfully loaded " << out_metadata.size() << " valid songs." << std::endl;
    return true;
}