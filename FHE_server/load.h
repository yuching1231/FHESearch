#ifndef VARIABLES_H
#define VARIABLES_H

#include "variables.h"

#endif

using namespace lbcrypto;

int Loadcc(std::string DATAFOLDER, bool loadSecKey) {
    if (loadSecKey) {
        if (!Serial::DeserializeFromFile(DATAFOLDER + secKeyLocation, keys.secretKey, SerType::BINARY)) return 1;
        if (!Serial::DeserializeFromFile(DATAFOLDER + ccLocation, cc, SerType::BINARY)) return 1;
    }
    if (keyLoaded) return 0;
    deserializer.SetDataDirectory(DATAFOLDER);
    deserializer.Deserialize();
    cc = deserializer.getCryptoContext();
    keys.publicKey = deserializer.getPublicKey();
    ccLWE = cc->GetBinCCForSchemeSwitch();
    cc->EvalCompareSwitchPrecompute(131072, 1.0);
    keyLoaded = true;
    return 0;
}