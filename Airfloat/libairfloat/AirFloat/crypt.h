//
//  crypt.h
//  AirFloat
//
//  Created by Kristian Trenskow on 2/26/13.
//
//

#ifndef _crypt_h
#define _crypt_h

typedef struct crypt_t *crypt_aes_p;

size_t crypt_apple_private_encrypt(void* data, size_t data_size, void* encrypted_data, size_t encrypted_data_size);
size_t crypt_apple_private_decrypt(void* encrypted_data, size_t encrypted_data_size, void* data, size_t data_size);

crypt_aes_p crypt_aes_create(void* key, void* iv, size_t size);
void crypt_aes_destroy(crypt_aes_p d);
size_t crypt_aes_decrypt(crypt_aes_p d, void* encrypted_data, size_t encrypted_data_size, void* data, size_t data_size);

void crypt_md5_hash(const void* content, size_t content_size, void* md5, size_t md5_size);
void crypt_hex_encode(const void* content, size_t content_size, void* hex, size_t hex_size);

#endif
