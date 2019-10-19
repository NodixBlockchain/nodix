#ifndef LIBEC_API
#define LIBEC_API C_IMPORT
#endif

#define crypto_hash_sha512_BYTES 64
#define crypto_sign_ed25519_SECRETKEYBYTES 64U	
#define crypto_sign_ed25519_PUBLICKEYBYTES 32U	 	
#define crypto_sign_ed25519_BYTES 64U
#define crypto_sign_BYTES crypto_sign_ed25519_BYTES

typedef unsigned char dh_key_t[64];

typedef int				C_API_FUNC crypto_extract_key_func	(dh_key_t pk, const dh_key_t sk);
typedef int 			C_API_FUNC crypto_sign_open_func(const struct string *sign, const hash_t msgh, const struct string *pk);
typedef int 			C_API_FUNC compress_pubkey_func		(dh_key_t pk, dh_key_t cpk);
typedef int				C_API_FUNC derive_key_func			(dh_key_t public_key, dh_key_t private_key, hash_t secret);

typedef int				C_API_FUNC crypto_get_pub_func		(const dh_key_t sk, dh_key_t pk);

typedef crypto_sign_open_func	*crypto_sign_open_func_ptr;
typedef crypto_extract_key_func *crypto_extract_key_func_ptr;
typedef compress_pubkey_func	*compress_pubkey_func_ptr;
typedef derive_key_func			*derive_key_func_ptr;
typedef crypto_get_pub_func		*crypto_get_pub_func_ptr;

#ifdef FORWARD_CRYPTO
	typedef struct string	C_API_FUNC crypto_sign_func(const hash_t msgh, const dh_key_t sk);
	typedef crypto_sign_func		*crypto_sign_func_ptr;
#endif 



