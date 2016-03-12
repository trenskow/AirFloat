//
//  crypt_cc.c
//  libairfloat
//
//  Created by Kristian Trenskow on 13/08/15.
//
//

#include <Security/Security.h>
#import <CommonCrypto/CommonCryptor.h>
#import <CommonCrypto/CommonKeyDerivation.h>

#include <stdint.h>
#include <string.h>

#include "base64.h"

#include "crypt_key.h"
#include "crypt.h"

struct crypt_aes_t {
    uint8_t key[16];
    uint8_t iv[16];
    CCCryptorRef cryptor;
};

SecKeyRef _crypt_get_private_key() {
    
    uint8_t keyData[strlen(AIRPORT_PRIVATE_KEY_P12)];
    size_t keyDataLen = base64_decode(AIRPORT_PRIVATE_KEY_P12, keyData);
    
    CFDataRef key = CFDataCreate(NULL, keyData, keyDataLen);
    
    CFMutableDictionaryRef options = CFDictionaryCreateMutable(NULL, 1, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDictionarySetValue(options, kSecImportExportPassphrase, CFSTR(""));
    
    CFArrayRef items = NULL;
    
    OSStatus error = SecPKCS12Import(key, options, &items);
    
    SecKeyRef privateKey = NULL;
    
    if (error == noErr && CFArrayGetCount(items) > 0) {
        CFDictionaryRef identityDict = CFArrayGetValueAtIndex(items, 0);
        SecIdentityRef identity = (SecIdentityRef)CFDictionaryGetValue(identityDict, kSecImportItemIdentity);
        error = SecIdentityCopyPrivateKey(identity, &privateKey);
        if (error != noErr)
            privateKey = NULL;
    }
    
    CFRelease(items);
    CFRelease(options);
    
    return privateKey;
    
}

size_t crypt_apple_private_encrypt(void* data, size_t data_size, void* encrypted_data, size_t encrypted_data_size) {
    
    SecKeyRef privateKey = _crypt_get_private_key();
    
    size_t len = 0;
    
    if (privateKey != NULL) {
        
        len = encrypted_data_size;
        
        OSStatus err = SecKeyRawSign(privateKey, kSecPaddingNone, data, data_size, encrypted_data, &len);
        if (err != noErr)
            len = 0;
        
        CFRelease(privateKey);
        
    }
    
    return len;
    
}

size_t crypt_apple_private_decrypt(void* encrypted_data, size_t encrypted_data_size, void* data, size_t data_size) {
    
    SecKeyRef privateKey = _crypt_get_private_key();
    
    size_t len = 0;
    
    if (privateKey != NULL) {
        
        len = data_size;
        
        OSStatus err = SecKeyDecrypt(privateKey, kSecPaddingOAEP, encrypted_data, encrypted_data_size, data, &len);
        if (err != noErr)
            len = 0;
        
        CFRelease(privateKey);
        
    }
    
    return len;
    
}

crypt_aes_p crypt_aes_create(void* key, void* iv, size_t size) {
    
    assert(size == 16);
    
    struct crypt_aes_t *d = (struct crypt_aes_t*)malloc(sizeof(struct crypt_aes_t));
    memcpy(d->key, key, 16);
    memcpy(d->iv, iv, 16);
    
    OSStatus err = CCCryptorCreate(kCCDecrypt, kCCAlgorithmAES128, kCCOptionPKCS7Padding, key, 16, iv, &d->cryptor);
    
    if (err != noErr)
        return NULL;
    
    return d;
    
}

void crypt_aes_destroy(crypt_aes_p d) {
    
    CCCryptorRelease(d->cryptor);
    
    free(d);
    
}

size_t crypt_aes_decrypt(crypt_aes_p d, void* encrypted_data, size_t encrypted_data_size, void* data, size_t data_size) {
    
    size_t data_moved = 0;
    
    CCCryptorUpdate(d->cryptor, encrypted_data, encrypted_data_size, data, data_size, &data_moved);
    
    return data_moved;
    
}

void crypt_md5_hash(const void* content, size_t content_size, void* md5, size_t md5_size) {
    
    assert(md5_size >= 16);
    
    CC_MD5_CTX context;
    CC_MD5_Init(&context);
    CC_MD5_Update(&context, content, (CC_LONG)content_size);
    CC_MD5_Final(md5, &context);
    
}
