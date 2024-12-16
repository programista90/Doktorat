#ifndef BASE64_H_
#define BASE64_H_

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
//#include <avr/pgmspace.h>
//#include "../ustring/ustring.h"
//#include "../status.h"


/**
 * Function: base64_encode
 * @data:    Data to be encoded
 * @len:     Length of the data to be encoded
 * Returns:  Encoded data or %NULL on failure
 *
 * Caller is responsible for freeing the returned buffer.
 */
uint8_t * base64_encode(const uint8_t * data, size_t len);
/**
 * Function: base64_decode
 * @data:    Data to be decoded
 * @out_len: Pointer to output length variable
 * Returns:  Decoded data or %NULL on failure
 *
 * Caller is responsible for freeing the returned buffer.
 */
uint8_t * base64_decode(const uint8_t * data, size_t * out_len);


#endif /* BASE64_H_ */
