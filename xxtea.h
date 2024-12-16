#ifndef XXTEA_H_
#define XXTEA_H_
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
//#include <arduino.h>
//#include "../ustring/ustring.h"
//#include "../status.h"


/**
 * Function: xxtea_encrypt
 * @data:    Data to be encrypted
 * @len:     Length of the data to be encrypted
 * @key:     Symmetric key
 * @out_len: Pointer to output length variable
 * Returns:  Encrypted data or %NULL on failure
 *
 * Caller is responsible for freeing the returned buffer.
 */
uint8_t * xxtea_encrypt(const uint8_t * data, size_t len, const uint8_t * key, size_t * out_len);

/**
 * Function: xxtea_decrypt
 * @data:    Data to be decrypted
 * @len:     Length of the data to be decrypted
 * @key:     Symmetric key
 * @out_len: Pointer to output length variable
 * Returns:  Decrypted data or %NULL on failure
 *
 * Caller is responsible for freeing the returned buffer.
 */
uint8_t * xxtea_decrypt(const uint8_t * data, size_t len, const uint8_t * key, size_t * out_len);


#endif /* XXTEA_H_ */
