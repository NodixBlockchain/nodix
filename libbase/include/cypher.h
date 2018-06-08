#ifndef LIBBASE_API
#define LIBBASE_API C_IMPORT
#endif

LIBBASE_API  int  	C_API_FUNC	 RC4(char *key, char *plaintext, size_t len, unsigned char *ciphertext);
LIBBASE_API  int  	C_API_FUNC	 RC4_b(hash_t key, char *plaintext, size_t len, unsigned char *ciphertext);