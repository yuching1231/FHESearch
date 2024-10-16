#include <iostream>
#include <fstream>
#include <string>
#include "headers.h"

using namespace std;

int main(int argc, char **argv) {
    const string filename = argv[1];
    const string key_folder = "Keys/";
    const string result_file = "Data/" + filename;
    const string decrypted_file = "Data/decrypted_result.txt";

    // Decrypt the result
    string decrypted_result = to_string(Decrypt(result_file, key_folder));

    // Write decrypted result to file
    ofstream outfile(decrypted_file);
    outfile << decrypted_result << endl;
    outfile.close();
    cout << "Decrypted result: " << decrypted_result << endl;
    cout << "Decryption completed. Results written to " << decrypted_file << endl;

    return 0;
}
