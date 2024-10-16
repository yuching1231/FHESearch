#include <iostream>
#include <string>
#include <stack>
#include <unordered_map>
#include <vector>
#include "headers.h"
#include <time.h>

using namespace std;

int main() {
    string folder = "Keys\\";
    keyGen(0,folder);
    string plain = "200data.csv";

    auto a = readcsv(plain);

    vector<string> cipher = {"cipher1", "cipher2"};
    int pnum = 4;
    Encrypt(30, folder, cipher[0]);
    Encrypt(70, folder, cipher[1]);
    string query = "P0 = c0 and p1 = c1";
    clock_t start, end;
    start = time(NULL);
    searchCSV(folder, query, plain, cipher, "result", pnum);
    end = time(NULL);
    std::cout << "Time taken: " << (double)(end - start) << "s" << std::endl;

    cout << "Done" << endl;
    cout << Decrypt("result", folder) << endl;
    
    query = "(P0 = c0 and p1 = c1)";

    start = time(NULL);
    searchCSV(folder, query, plain, cipher, "result", pnum);
    end = time(NULL);
    std::cout << "Time taken: " << (double)(end - start) << "s" << std::endl;
    
    cout << "Done" << endl;
    cout << Decrypt("result", folder) << endl;


    
    return 0;
}
