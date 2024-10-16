#include <iostream>
#include <fstream>
#include <string>
#include "headers.h"

using namespace std;
using namespace lbcrypto;

int main() {
    const string key_folder = "Keys/";
    string security_level_file = "Data/security_level.txt";
    int32_t security_level = 0;

    // Read security level from file
    ifstream secfile(security_level_file);
    if (secfile >> security_level) {
        cout << "Security Level is " << security_level << endl;
    } else {
        cerr << "Error reading security level from file." << endl;
        return 1;
    }

    cout << "Generating Keys..." << endl;
    clock_t start = clock();

    try {
        keyGen(security_level, key_folder);

        cout << "Keys generated and saved successfully." << endl;
    } catch (const exception& e) {
        cerr << "Error during key generation: " << e.what() << endl;
        return 1;
    }

    clock_t end = clock();

    cout << "Key Generation takes ";
    int time_taken = int(end - start) / CLOCKS_PER_SEC;
    cout << time_taken / 60 << " minutes and " << int(time_taken) % 60 << " seconds." << endl;

    return 0;
}
