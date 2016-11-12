//
//  crypt.c
//  AirFloat
//
//  Copyright (c) 2013, Kristian Trenskow All rights reserved.
//
//  Redistribution and use in source and binary forms, with or
//  without modification, are permitted provided that the following
//  conditions are met:
//
//  Redistributions of source code must retain the above copyright
//  notice, this list of conditions and the following disclaimer.
//  Redistributions in binary form must reproduce the above
//  copyright notice, this list of conditions and the following
//  disclaimer in the documentation and/or other materials provided
//  with the distribution. THIS SOFTWARE IS PROVIDED BY THE
//  COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
//  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
//  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
//  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
//  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
//  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#if !defined(__APPLE__)

#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/aes.h>
#include <openssl/md5.h>

#include "crypt_key.h"

struct crypt_aes_t {
    AES_KEY key;
    unsigned char iv[16];
};

RSA* _private_key = NULL;

RSA* _crypt_apple_private_key() {
    
    if (_private_key == NULL) {
        BIO* bio = BIO_new(BIO_s_mem());
        BIO_write(bio, AIRPORT_PRIVATE_KEY_PEM, strlen(AIRPORT_PRIVATE_KEY_PEM));
        _private_key = PEM_read_bio_RSAPrivateKey(bio, NULL, NULL, NULL);
        BIO_free(bio);
    }
    
    return _private_key;
    
}

size_t crypt_apple_private_encrypt(void* data, size_t data_size, void* encrypted_data, size_t encrypted_data_size) {
    
    RSA* key = _crypt_apple_private_key();
    
    assert(encrypted_data_size >= RSA_size(key));
    
    return RSA_private_encrypt((int32_t)data_size, data, encrypted_data, key, RSA_NO_PADDING);
    
}

size_t crypt_apple_private_decrypt(void* encrypted_data, size_t encrypted_data_size, void* data, size_t data_size) {
    
    RSA* key = _crypt_apple_private_key();
    
    assert(data_size >= RSA_size(key) - 11);
    
    return RSA_private_decrypt((uint32_t)encrypted_data_size, encrypted_data, data, key, RSA_PKCS1_OAEP_PADDING);
    
}

struct crypt_aes_t* crypt_aes_create(void* key, void* iv, size_t size) {
    
    assert(size == 16);
    
    struct crypt_aes_t* d = (struct crypt_aes_t*)malloc(sizeof(struct crypt_aes_t));
    AES_set_decrypt_key(key, 128, &d->key);
    memcpy(d->iv, iv, 16);
    
    return d;
    
}

void crypt_aes_destroy(struct crypt_aes_t* d) {
    
    free(d);
    
}

size_t crypt_aes_decrypt(struct crypt_aes_t* d, void* encrypted_data, size_t encrypted_data_size, void* data, size_t data_size) {
    
    assert(data_size >= encrypted_data_size);
    
    unsigned char iv[16];
    memcpy(iv, d->iv, 16);
    
    size_t encrypted_len = encrypted_data_size & ~0xf;
    
    unsigned char ret[encrypted_data_size];
    memcpy(ret, encrypted_data, encrypted_data_size);
    
    AES_cbc_encrypt(encrypted_data, ret, encrypted_len, &d->key, iv, AES_DECRYPT);
    
    memcpy(data, ret, encrypted_data_size);
    
    return encrypted_data_size;
    
}

void crypt_md5_hash(const void* content, size_t content_size, void* md5, size_t md5_size) {
    
    assert(md5_size >= 16);
    
    MD5_CTX context;
    MD5_Init(&context);
    MD5_Update(&context, content, content_size);
    MD5_Final(md5, &context);
    
}

#endif
