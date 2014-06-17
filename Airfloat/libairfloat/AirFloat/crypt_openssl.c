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

#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/aes.h>
#include <openssl/md5.h>

#define AIRPORT_PRIVATE_KEY \
"-----BEGIN RSA PRIVATE KEY-----\n" \
"MIIEpQIBAAKCAQEA59dE8qLieItsH1WgjrcFRKj6eUWqi+bGLOX1HL3U3GhC/j0Qg90u3sG/1CUt\n" \
"wC5vOYvfDmFI6oSFXi5ELabWJmT2dKHzBJKa3k9ok+8t9ucRqMd6DZHJ2YCCLlDRKSKv6kDqnw4U\n" \
"wPdpOMXziC/AMj3Z/lUVX1G7WSHCAWKf1zNS1eLvqr+boEjXuBOitnZ/bDzPHrTOZz0Dew0uowxf\n" \
"/+sG+NCK3eQJVxqcaJ/vEHKIVd2M+5qL71yJQ+87X6oV3eaYvt3zWZYD6z5vYTcrtij2VZ9Zmni/\n" \
"UAaHqn9JdsBWLUEpVviYnhimNVvYFZeCXg/IdTQ+x4IRdiXNv5hEewIDAQABAoIBAQDl8Axy9XfW\n" \
"BLmkzkEiqoSwF0PsmVrPzH9KsnwLGH+QZlvjWd8SWYGN7u1507HvhF5N3drJoVU3O14nDY4TFQAa\n" \
"LlJ9VM35AApXaLyY1ERrN7u9ALKd2LUwYhM7Km539O4yUFYikE2nIPscEsA5ltpxOgUGCY7b7ez5\n" \
"NtD6nL1ZKauw7aNXmVAvmJTcuPxWmoktF3gDJKK2wxZuNGcJE0uFQEG4Z3BrWP7yoNuSK3dii2jm\n" \
"lpPHr0O/KnPQtzI3eguhe0TwUem/eYSdyzMyVx/YpwkzwtYL3sR5k0o9rKQLtvLzfAqdBxBurciz\n" \
"aaA/L0HIgAmOit1GJA2saMxTVPNhAoGBAPfgv1oeZxgxmotiCcMXFEQEWflzhWYTsXrhUIuz5jFu\n" \
"a39GLS99ZEErhLdrwj8rDDViRVJ5skOp9zFvlYAHs0xh92ji1E7V/ysnKBfsMrPkk5KSKPrnjndM\n" \
"oPdevWnVkgJ5jxFuNgxkOLMuG9i53B4yMvDTCRiIPMQ++N2iLDaRAoGBAO9v//mU8eVkQaoANf0Z\n" \
"oMjW8CN4xwWA2cSEIHkd9AfFkftuv8oyLDCG3ZAf0vrhrrtkrfa7ef+AUb69DNggq4mHQAYBp7L+\n" \
"k5DKzJrKuO0r+R0YbY9pZD1+/g9dVt91d6LQNepUE/yY2PP5CNoFmjedpLHMOPFdVgqDzDFxU8hL\n" \
"AoGBANDrr7xAJbqBjHVwIzQ4To9pb4BNeqDndk5Qe7fT3+/H1njGaC0/rXE0Qb7q5ySgnsCb3DvA\n" \
"cJyRM9SJ7OKlGt0FMSdJD5KG0XPIpAVNwgpXXH5MDJg09KHeh0kXo+QA6viFBi21y340NonnEfdf\n" \
"54PX4ZGS/Xac1UK+pLkBB+zRAoGAf0AY3H3qKS2lMEI4bzEFoHeK3G895pDaK3TFBVmD7fV0Zhov\n" \
"17fegFPMwOII8MisYm9ZfT2Z0s5Ro3s5rkt+nvLAdfC/PYPKzTLalpGSwomSNYJcB9HNMlmhkGzc\n" \
"1JnLYT4iyUyx6pcZBmCd8bD0iwY/FzcgNDaUmbX9+XDvRA0CgYEAkE7pIPlE71qvfJQgoA9em0gI\n" \
"LAuE4Pu13aKiJnfft7hIjbK+5kyb3TysZvoyDnb3HOKvInK7vXbKuU4ISgxB2bB3HcYzQMGsz1qJ\n" \
"2gG0N5hvJpzwwhbhXqFKA4zaaSrw622wDniAK5MlIE0tIAKKP4yxNGjoD2QYjhBGuhvkWKaXTyY=\n" \
"-----END RSA PRIVATE KEY-----"

struct crypt_aes_t {
    AES_KEY key;
    unsigned char iv[16];
};

RSA* _private_key = NULL;

RSA* _crypt_apple_private_key() {
    
    if (_private_key == NULL) {
        BIO* bio = BIO_new(BIO_s_mem());
        BIO_write(bio, AIRPORT_PRIVATE_KEY, strlen(AIRPORT_PRIVATE_KEY));
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

void crypt_hex_encode(const void* content, size_t content_size, void* hex, size_t hex_size) {
    
    assert(hex_size >= content_size * 2);
    
    static char hex_values[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
    
    for (int i = 0 ; i < content_size ; i++) {
        ((char*)hex)[i*2] = hex_values[(((char*)content)[i] >> 4) & 0xF];
        ((char*)hex)[(i*2)+1] = hex_values[((char*)content)[i] & 0xF];
    }
    
}
