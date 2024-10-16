#ifndef VARIABLES_H
#define VARIABLES_H

#include "variables.h"

#endif

LWEPrivateKey privateKeyLWE;

using namespace lbcrypto;

std::pair <CryptoContext<DCRTPoly>, KeyPair<lbcrypto::DCRTPoly> > CKKSKeyGen(SecurityLevel sl, uint32_t ringdim) {
    ScalingTechnique scTech = FLEXIBLEAUTO;
    uint32_t multDepth      = 17;
    if (scTech == FLEXIBLEAUTOEXT)
        multDepth += 1;

    uint32_t scaleModSize = 50;
    uint32_t firstModSize = 60;
    uint32_t ringDim      = ringdim;
    uint32_t batchSize    = slots;

    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(multDepth);
    parameters.SetScalingModSize(scaleModSize);
    parameters.SetFirstModSize(firstModSize);
    parameters.SetScalingTechnique(scTech);
    parameters.SetSecurityLevel(sl);
    parameters.SetRingDim(ringDim);
    parameters.SetBatchSize(batchSize);
    parameters.SetSecretKeyDist(UNIFORM_TERNARY);
    parameters.SetKeySwitchTechnique(HYBRID);
    parameters.SetNumLargeDigits(3);

    CryptoContext<DCRTPoly> cc = GenCryptoContext(parameters);

    // Enable the features that you wish to use
    cc->Enable(PKE);
    cc->Enable(KEYSWITCH);
    cc->Enable(LEVELEDSHE);
    cc->Enable(ADVANCEDSHE);
    cc->Enable(SCHEMESWITCH);

    // Generate encryption keys
    auto keys = cc->KeyGen();
    //cc->EvalMultKeyGen(keys.secretKey);

    return std::make_pair(cc, keys);
}

void keyGen(uint32_t securityLevel, std::string DATAFOLDER) {
    // create folder if it does not exist
    if (!std::filesystem::exists(DATAFOLDER)) {
        std::filesystem::create_directory(DATAFOLDER);
    }
    SecurityLevel sl;
    BINFHE_PARAMSET slBin;
    uint32_t ringdim = 1024;
    if(securityLevel == 0){
        sl      = HEStd_NotSet;
        slBin = TOY;
        ringdim = 1024;
    }
    else if(securityLevel == 1){
        sl      = HEStd_128_classic;
        slBin = STD128;
        ringdim = 65536;
    }
    else if(securityLevel == 2){
        sl      = HEStd_192_classic;
        slBin = STD192;
    }
    else if(securityLevel == 3){
        sl      = HEStd_256_classic;
        slBin = STD256;
    }
    else{
        std::cout << "Invalid Security Level" << std::endl;
        return;
    }
    auto [cc, keys] = CKKSKeyGen(sl, ringdim); 
    Serial::SerializeToFile(DATAFOLDER + secKeyLocation, keys.secretKey, SerType::BINARY);
    uint32_t logQ_ccLWE   = 25;
    SchSwchParams params;
    params.SetSecurityLevelCKKS(sl);
    params.SetSecurityLevelFHEW(slBin);
    params.SetCtxtModSizeFHEWLargePrec(logQ_ccLWE);
    params.SetNumSlotsCKKS(slots);
    params.SetNumValues(slots);
    auto privateKeyFHEW = cc->EvalSchemeSwitchingSetup(params);
    privateKeyLWE = privateKeyFHEW;
    auto ccLWE          = cc->GetBinCCForSchemeSwitch();
    ccLWE->BTKeyGen(privateKeyFHEW);
    cc->EvalSchemeSwitchingKeyGen(keys, privateKeyFHEW);
    auto modulus_LWE     = 1 << logQ_ccLWE;
    auto beta            = ccLWE->GetBeta().ConvertToInt();
    auto pLWE2           = modulus_LWE / (2 * beta);  // Large precision
    double scaleSignFHEW = 1.0;
    cc->EvalCompareSwitchPrecompute(pLWE2, scaleSignFHEW);
    Serial::SerializeToFile(DATAFOLDER + LWEKeyLocation, privateKeyFHEW, SerType::BINARY);
    std::vector<double> vec = {0.0};
    Plaintext plain = cc->MakeCKKSPackedPlaintext(vec);
    auto temp = cc->Encrypt(keys.publicKey, plain);
    SchemeSwitchingDataSerializer serializer(cc, keys.publicKey, temp);
    serializer.SetDataDirectory(DATAFOLDER);
    serializer.Serialize();
}
