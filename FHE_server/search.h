#ifndef VARIABLES_H
#define VARIABLES_H

#include "variables.h"

#endif

#ifndef LOAD_H
#define LOAD_H

#include "load.h"

#endif

#pragma omp declare reduction(addtool : Ciphertext<DCRTPoly> : omp_out = addtool(omp_out, omp_in)) initializer(omp_priv = zeroenc()) 




Ciphertext<DCRTPoly> CKKSnot (auto a) {
    auto cipherOne = encrypttool(cc, keys.publicKey, 1.0);
    return cc->EvalSub(cipherOne, a);
}

Ciphertext<DCRTPoly> CKKSand (auto a, auto b) {
    return cc->EvalMult(a, b);
}

Ciphertext<DCRTPoly> CKKSor (auto a, auto b) {
    return cc->EvalSub(cc->EvalAdd(a, b), cc->EvalMult(a, b));
}
std::vector<std::vector<double>> readcsv(const std::string& filename) {
    std::vector<std::vector<double>> data;
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return data;
    }

    std::string line;
    size_t numColumns = 0;
    bool firstLine = true;

    while (std::getline(file, line)) {
        std::vector<double> row;
        std::stringstream ss(line);
        std::string item;

        while (std::getline(ss, item, ',')) {
            try {
                double value = std::stod(item);
                row.push_back(value);
            } catch (const std::invalid_argument& e) {
                std::cerr << "Invalid number: " << item << " in line: " << line << std::endl;
                data.clear();
                return data;
            } catch (const std::out_of_range& e) {
                std::cerr << "Number out of range: " << item << " in line: " << line << std::endl;
                data.clear();
                return data;
            }
        }

        if (firstLine) {
            numColumns = row.size();
            firstLine = false;
        } else {
            if (row.size() != numColumns) {
                std::cerr << "Error: inconsistent number of columns in row: " << line << std::endl;
                data.clear();
                return data;
            }
        }

        data.push_back(row);
    }

    file.close();
    return data;
}

Ciphertext<DCRTPoly> comparetool(auto first, auto second, char op){

    auto cipherOne = encrypttool(cc, keys.publicKey, 1.0);

    auto m1 = cc->EvalCompareSchemeSwitching(first, second, slots, slots);
    auto m2 = cc->EvalCompareSchemeSwitching(second, first, slots, slots);

    Ciphertext<DCRTPoly> m1NOT = CKKSnot(m1);
    Ciphertext<DCRTPoly> m2NOT = CKKSnot(m2);

    if (op == '=') return CKKSand(m1NOT, m2NOT);
    if (op == '>') return CKKSand(m2, m1NOT);
    if (op == '<') return CKKSand(m1, m2NOT);
    return 0;
}

Ciphertext<DCRTPoly> optool(char op, auto a, auto b) {
    switch (op) {
        case '&': return CKKSand(a, b);
        case '|': return CKKSor(a, b);
    }
    return 0;
}

std::string stringpreload(const std::string &expression) {
    std::string result;
    for (size_t i = 0; i < expression.length(); i++) {
        if (expression.substr(i, 3) == "and" || expression.substr(i, 3) == "AND") {
            result += '&';
            i += 2;
        } else if (expression.substr(i, 2) == "or" || expression.substr(i, 2) == "OR") {
            result += '|';
            i += 1;
        } else if (expression[i] == 'P') {
            result += 'p';
        } else if (expression[i] == 'C') {
            result += 'c';
        } else if (!isspace(expression[i]) || 
                   (i > 0 && (isdigit(expression[i-1]) && (expression[i] == '<' || expression[i] == '>' || expression[i] == '='))) ||
                   (i < expression.length() - 1 && (isdigit(expression[i+1]) && (expression[i] == '<' || expression[i] == '>' || expression[i] == '='))) ) {
            result += expression[i];
        } 
    }
    return result;
}



Ciphertext<DCRTPoly> evaluate(const std::string &tokens, const std::vector<Ciphertext<DCRTPoly>> &vecC,const std::vector<Ciphertext<DCRTPoly>> &vecP) {
    std::stack<Ciphertext<DCRTPoly>> valuesStack;
    std::stack<char> opsStack;
    for (size_t i = 0; i < tokens.length(); i++) {
        if (tokens[i] == ' ') continue;

        if (tokens[i] == '(') {
            opsStack.push(tokens[i]);
        } else if ((tokens[i] == 'p' || tokens[i] == 'c') && (i+2 < tokens.length()) && (tokens[i+2] == '<' || tokens[i+2] == '>' || tokens[i+2] == '=')) {
            auto first = tokens[i] == 'p' ? vecP[tokens[i+1]-'0'] : vecC[tokens[i+1]-'0'];
            char op = tokens[i+2];
            auto second = tokens[i+3] == 'p' ? vecP[tokens[i+4]-'0'] : vecC[tokens[i+4]-'0'];
            valuesStack.push(comparetool(first, second, op));
            i += 4; 
        } else if (tokens[i] == ')') {
            while (!opsStack.empty() && opsStack.top() != '(') {
                char op = opsStack.top(); opsStack.pop();
                auto val2 = valuesStack.top(); valuesStack.pop();
                auto val1 = valuesStack.top(); valuesStack.pop();
                valuesStack.push(optool(op, val1, val2));
            }
            opsStack.pop();
        } else {
            while (!opsStack.empty() && opsStack.top() != '(') {
                char op = opsStack.top(); opsStack.pop();
                auto val2 = valuesStack.top(); valuesStack.pop();
                auto val1 = valuesStack.top(); valuesStack.pop();
                valuesStack.push(optool(op, val1, val2));
            }
            opsStack.push(tokens[i]);
        }
    }
    while (!opsStack.empty()) {
        char op = opsStack.top(); opsStack.pop();
        auto val2 = valuesStack.top(); valuesStack.pop();
        auto val1 = valuesStack.top(); valuesStack.pop();
        valuesStack.push(optool(op, val1, val2));
    }

    return valuesStack.top();
}


int searchCSV(std::string KeyFolder, std::string originalquery, std::string plaintfile, std::vector<std::string> cipherfile, std::string result, int pnum) {
    Loadcc(KeyFolder, false);
    std::vector<Ciphertext<DCRTPoly>> vecC;
    auto query = stringpreload(originalquery);
    for(auto c : cipherfile){
        auto ciphertext = loadciphertext(c);
        if (!ciphertext) return 0;
        vecC.push_back(ciphertext);
    }
    
    auto plaintext = readcsv(plaintfile);
    //if (!plaintext) return 0;

    auto ciphercount = zeroenc();

    //MP execute for plaintext, pnum is the number of threads
    int size_per_thread = plaintext.size()/pnum;
    #pragma omp parallel num_threads(pnum) reduction(addtool: ciphercount)
    {
        int thread_id = omp_get_thread_num();
        int start = thread_id * size_per_thread;
        int end = (thread_id == pnum - 1) ? plaintext.size() : (thread_id + 1) * size_per_thread;
        for (int i = start; i < end; i++) {
            std::vector<Ciphertext<DCRTPoly>> vecP;
            for(auto p : plaintext[i]){
                auto ciphertext = encrypttool(cc, keys.publicKey, p);
                //if (!ciphertext) return 0;
                vecP.push_back(ciphertext);
            }
            
            ciphercount = addtool(ciphercount, evaluate(query, vecC, vecP));
            //std::cout << decrypttool(cc, keys.secretKey, cc->EvalFHEWtoCKKS(result, slots, slots, 2)) << std::endl<< std::endl;
        }
    }
    if (saveciphertext(cc, ciphercount, result)) return 0;
    return 1;
}