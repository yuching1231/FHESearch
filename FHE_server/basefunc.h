#ifndef VARIABLES_H
#define VARIABLES_H

#include "variables.h"

#endif

#ifndef LOAD_H
#define LOAD_H

#include "load.h"

#endif

Ciphertext<DCRTPoly> loadciphertext(std::string filename){
    Ciphertext<DCRTPoly> ciphertext;
    if(!Serial::DeserializeFromFile(filename, ciphertext, SerType::BINARY)){
        return 0;
    }
    return ciphertext;
}

int saveciphertext(auto cc, auto ciphertext, std::string filename){
    if(!Serial::SerializeToFile(filename, ciphertext, SerType::BINARY)){
        return 1;
    }
    return 0;
}

auto decrypttool(auto cc, auto seckey, auto ciphertext){
    Plaintext plain;
    cc->Decrypt(seckey, ciphertext, &plain);
    plain->SetLength(slots);   
    return plain->GetCKKSPackedValue()[0].real();
}

auto encrypttool(auto cc, auto pubkey, double d){
    std::vector<double> vec = {d};
    Plaintext plain = cc->MakeCKKSPackedPlaintext(vec);
    auto ciphertext = cc->Encrypt(pubkey, plain);
    return ciphertext;
}

auto Decrypt(std::string cipherfile, std::string KeyFolder){
    Loadcc(KeyFolder, true);
    auto ciphertext = loadciphertext(cipherfile);
    if (!ciphertext) return 0;
    return (int)(decrypttool(cc, keys.secretKey, ciphertext)+0.1);
}

auto Encrypt(double d, std::string KeyFolder, std::string cipherfile){
    Loadcc(KeyFolder, false);
    auto ciphertext = encrypttool(cc, keys.publicKey, d);
    if (!ciphertext) return 0;
    if (saveciphertext(cc, ciphertext, cipherfile)) return 0;
    return 1;
}

auto addtool(Ciphertext<DCRTPoly> first, Ciphertext<DCRTPoly> second){
    return cc->EvalAdd(first, second);
}

auto zeroenc(){
    std::vector<double> vec = {0.0};
    Plaintext plain = cc->MakeCKKSPackedPlaintext(vec);
    auto temp = cc->Encrypt(keys.publicKey, plain);
    return temp;
}