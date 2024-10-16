#include <iostream>
#include <fstream>
#include <sstream>
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

string plain2cipher(const string& folder, std::vector<std::string>& cipher, const std::vector<std::string>& tokens)
{
    //clock_t start = clock();
    int cipher_number = 0;
    string query = "";
    const string ciphertext = "cipher";
    
    // Clear the cipher vector before use
    cipher.clear();

    for(std::vector<std::string>::size_type i = 0; i < tokens.size(); i++)
    {
        if(tokens[i] == "="  || tokens[i] == ">" || tokens[i] == "<")
        {
            if (i + 1 >= tokens.size()) {
                throw std::runtime_error("Unexpected end of tokens");
            }
            
            cipher_number++;
            cipher.push_back(ciphertext + to_string(cipher_number));
            Encrypt(encodeString(tokens[i+1]), folder, cipher.back());

            if (i > 0) {
                switch (tokens[i-1].front())
                {
                case 'N': query += "P0 "; break;
                case 'I': query += "P1 "; break;
                case 'D': query += "P2 "; break;
                case 'G': break;
                default: break;
                }
            }
            
            query += tokens[i];
            cout << "Now " << tokens[i+1] << " is encoded." << endl;
            query += " c" + to_string(cipher_number-1);
        }
        else if(tokens[i] == ">=" || tokens[i] == "<=" || tokens[i] == "!=")
        {
            if (i + 1 >= tokens.size() || i == 0) {
                throw std::runtime_error("Invalid token sequence");
            }
            
            cipher_number++;
            cipher.push_back(ciphertext + to_string(cipher_number));
            Encrypt(encodeString(tokens[i+1]), folder, cipher.back());
            
            switch (tokens[i-1].front())
            {
            case 'N': query += "( P0 "; break;
            case 'I': query += "( P1 "; break;
            case 'D': query += "( P2 "; break;
            case 'G': break;
            default: break;
            }
            
            if(tokens[i] == "!=")
            {
                query += ">";
            }
            else
            {
                query += tokens[i].front();
            }
            cout << "Now " << tokens[i+1] << " is encoded." << endl;
            query += " c" + to_string(cipher_number-1) + " OR ";

            switch (tokens[i-1].front())
            {
            case 'N': query += "P0 "; break;
            case 'I': query += "P1 "; break;
            case 'D': query += "P2 "; break;
            case 'G': break;
            default: break;
            }

            query += (tokens[i] == "!=") ? "<" : "=";
            query += " c" + to_string(cipher_number-1) + " )";
        }
        else if(tokens[i] == "AND" || tokens[i] == "OR" || tokens[i] == "(" || tokens[i] == ")")
        {
            query += " " + tokens[i] + " ";
        }
    }
    
    //clock_t end = clock();
    cout << "Encryption takes ";
    //running_time(start, end);
    cout << "The modified query:" << endl;
    cout << query << endl;
    return query;
}

int main() {
    const string key_folder = "Keys/";
    const string query_file = "Data/query.txt";
    vector<string> cipher;

    // Read query from file
    ifstream queryfile(query_file);
    string input_query;
    vector<string> query_tokens;

    if (getline(queryfile, input_query)) {
        stringstream ss(input_query);
        string token;
        while (getline(ss, token, ' ')) {
            query_tokens.push_back(token);
        }
    } else {
        cerr << "Error reading query from file." << endl;
        return 1;
    }

    string modified_query = plain2cipher(key_folder, cipher, query_tokens);

    // Write modified query to file
    ofstream outfile("Data/modified_query.txt");
    outfile << modified_query;
    outfile.close();

    // Write cipher to file
    ofstream cipherfile("cipher.txt");
    for (size_t i = 0; i < cipher.size(); ++i) {
        cipherfile << cipher[i];
        if (i < cipher.size() - 1) {
            cipherfile << endl;
        }
    }

    cout << "Encryption completed. Modified query and cipher written to files." << endl;

    return 0;
}
