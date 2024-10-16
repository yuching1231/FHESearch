#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "headers.h"

using namespace std;

int encodeString(const std::string& input) {
    try {
        return std::stoi(input);
    } catch (const std::invalid_argument&) {
        // If not an integer, proceed with encoding
    }

    // Use a simple hash function
    uint32_t hash = 0;
    for (char c : input) {
        hash = (hash * 31) + static_cast<unsigned char>(c);
    }

    // Ensure the result is a positive integer within int range
    return static_cast<int>(hash & 0x7FFFFFFF);
}

void encodeCSV(const std::string& inputFile, const std::string& outputFile) {
    std::ifstream inFile(inputFile);
    std::ofstream outFile(outputFile);
    cout << inputFile << endl;

    if (!inFile.is_open() || !outFile.is_open()) {
        throw std::runtime_error("Failed to open input or output file");
    }

    std::string line;
    bool firstLine = true;

    while (std::getline(inFile, line)) {
        if (!firstLine) {
            outFile << "\n";
        }

        std::istringstream iss(line);
        std::string item;
        bool firstItem = true;

        while (std::getline(iss, item, ',')) {
            if (!firstItem) {
                outFile << ",";
            }
            outFile << encodeString(item);
            firstItem = false;
        }

        firstLine = false;
    }

    inFile.close();
    outFile.close();
}

int main(int argc, char **argv) {
    const string key_folder = "Keys/";
    const string data_folder = "Data/";
    string csvfile, encoded_csvfile;
    string modified_query;
    vector<string> cipher;
    int pnum = 40;

    try {
        // Read input CSV file name

            encoded_csvfile = data_folder + "encoded_" + argv[1];
            csvfile = data_folder + argv[1];
            cout << csvfile << "->" << encoded_csvfile << endl;

        // Encode CSV file
        encodeCSV(csvfile, encoded_csvfile);
        cout << "CSV file encoded successfully." << endl;

        // Read modified query
        ifstream query_file("Data/modified_query.txt");
        if (!getline(query_file, modified_query)) {
            throw runtime_error("Error reading modified query.");
        }
        cout << "Modified query read successfully." << endl;

        // Read cipher
        ifstream cipher_file("cipher.txt");
        string cipher_line;
        while (getline(cipher_file, cipher_line)) {
            cipher.push_back(cipher_line);
        }
        cout << "Cipher read successfully. Number of cipher elements: " << cipher.size() << endl;

        // Perform search
        cout << "Starting search operation..." << endl;
        searchCSV(key_folder, modified_query, encoded_csvfile, cipher, "result", pnum);
        cout << "Search completed successfully." << endl;

    } catch (const exception& e) {
        cerr << "Error during search operation: " << e.what() << endl;
        return 1;
    }

    cout << "Search completed. Results stored in 'result' file." << endl;
    return 0;
}
