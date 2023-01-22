#include "includes.hpp"

std::string aes_encrypt(const std::string& plaintext, const std::string& key) {
    EVP_CIPHER_CTX* ctx;
    int len;
    int ciphertext_len;
    std::string ciphertext;

    /* Create and initialise the context */
    if(!(ctx = EVP_CIPHER_CTX_new())) return "";

    /* Initialise the encryption operation. */
    if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, (const unsigned char*)key.c_str(), (const unsigned char*)key.c_str())) return "";

    /* Provide the message to be encrypted, and obtain the encrypted output. */
    /* EVP_EncryptUpdate can be called multiple times if necessary */
    if(1 != EVP_EncryptUpdate(ctx, (unsigned char*)ciphertext.data(), &len, (const unsigned char*)plaintext.c_str(), plaintext.size())) return "";
    ciphertext_len = len;

    /* Finalise the encryption. */
    if(1 != EVP_EncryptFinal_ex(ctx, (unsigned char*)ciphertext.data() + len, &len)) return "";
    ciphertext_len += len;

    /* Clean up */
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext;
}
