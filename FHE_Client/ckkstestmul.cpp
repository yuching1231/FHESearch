#include "openfhe.h"
#include <iostream>

using namespace lbcrypto;

int main() {
    // Step 1: Set up the CKKS crypto system
    CCParams<CryptoContextCKKSRNS> parameters;
    parameters.SetMultiplicativeDepth(20);
    parameters.SetScalingModSize(50);
    parameters.SetBatchSize(8);
    parameters.SetSecurityLevel(SecurityLevel::HEStd_128_classic);  // Set security parameter

    CryptoContext<DCRTPoly> cryptoContext = GenCryptoContext(parameters);
    cryptoContext->Enable(PKE);
    cryptoContext->Enable(KEYSWITCH);
    cryptoContext->Enable(LEVELEDSHE);

    // Step 2: Key generation
    auto keyPair = cryptoContext->KeyGen();
    cryptoContext->EvalMultKeyGen(keyPair.secretKey);

    // Step 3: Encoding and encryption
    std::vector<double> vectorOfInts1(8, 2);  // Vector of 2's
    std::vector<double> vectorOfInts2(8, 3);  // Vector of 3's

    Plaintext plaintext1 = cryptoContext->MakeCKKSPackedPlaintext(vectorOfInts1);
    Plaintext plaintext2 = cryptoContext->MakeCKKSPackedPlaintext(vectorOfInts2);

    auto ciphertext1 = cryptoContext->Encrypt(keyPair.publicKey, plaintext1);
    auto ciphertext2 = cryptoContext->Encrypt(keyPair.publicKey, plaintext2);

    // Step 4: Evaluation (multiplication)
    auto ciphertextResult = ciphertext1;

    for (int i = 0; i < 10; i++) {
        ciphertextResult = cryptoContext->EvalMult(ciphertextResult, ciphertext2);
    }

    // Step 5: Decryption
    Plaintext plaintextResult;
    cryptoContext->Decrypt(keyPair.secretKey, ciphertextResult, &plaintextResult);

    plaintextResult->SetLength(8);

    std::cout << "Result of 100 multiplications: " << plaintextResult << std::endl;

    // The expected result is 2 * (3^100) for each element
    std::cout << "Expected result: " << 2 * pow(3, 100) << std::endl;

    return 0;
}