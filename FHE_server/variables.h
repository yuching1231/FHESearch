#ifndef BASE_H
#define BASE_H

#include "openfhe.h"
#include "binfhecontext.h"
#include <time.h>
#include <iostream>
#include <fstream>
#include <string>
#include <utility>
#include <omp.h>
#include <time.h>
#include <filesystem>
#include <stack>
#include <vector>

#include "ciphertext-ser.h"
#include "cryptocontext-ser.h"
#include "key/key-ser.h"
#include "scheme/ckksrns/ckksrns-ser.h"
#include "scheme/ckksrns/schemeswitching-data-serializer.h"

#endif // BASE_H

using namespace lbcrypto;

std::string DATAFOLDER      = "Data\\";
std::string ccLocation      = "cryptocontext.txt";
std::string ccLWELocation   = "ccLWE.txt";
std::string LWEKeyLocation  = "LWEKey.txt";
std::string secKeyLocation  = "key_sec.txt"; 
std::string pubKeyLocation  = "key_pub.txt";  
std::string multKeyLocation = "key_mult.txt";
std::string rotKeyLocation  = "key_rot.txt";
uint32_t slots        = 1;
bool keyLoaded        = false;

CryptoContext<DCRTPoly> cc;
SchemeSwitchingDataDeserializer deserializer;
std::shared_ptr<lbcrypto::BinFHEContext> ccLWE;
KeyPair<lbcrypto::DCRTPoly> keys;

enum CompareType {
    EQUAL,
    GREATER,
    LESS
};
