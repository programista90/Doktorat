#include <iostream>
#include "xxtea.h"
#include "Timer.h"

#define MX (((z >> 5) ^ (y << 2)) + ((y >> 3) ^ (z << 4))) ^ ((sum ^ y) + (key[(p & 3) ^ e] ^ z))
#define DELTA 0x9e3779b9


static uint32_t * xxtea_to_uint_array(const uint8_t * data, size_t len, int inc_len, size_t * out_len) {
	uint32_t *out;
	size_t i;
	size_t n;

	n = (((len & 3) == 0) ? (len >> 2) : ((len >> 2) + 1));

	if (inc_len) {
		//int fm=getFreeMemory();
		 //Serial.print(("X3b="));
		//Serial.println(fm);
		 //Serial.print(("X3c="));
		//Serial.println((n+1)*sizeof(uint32_t));
	//	displayFreeMemory(1001);
		//return NULL;
		out = (uint32_t *)calloc(n + 1, sizeof(uint32_t));
	//	displayFreeMemory(1002);
		//out = (uint32_t *)malloc((n + 1)*sizeof(uint32_t));
	//	displayFreeMemoryIfCritical(921);
		//Serial.print("X3d=");
	   //Serial.println(n);
		//fm=getFreeMemory();
		//Serial.print(("X3e="));
	   //Serial.println(fm);
		if (!out) return NULL;
	//	displayFreeMemory(1003);
		//Serial.print(("X3f="));
	   //Serial.println(fm);
		out[n] = (uint32_t)len;
		*out_len = n + 1;
	}
	else {
		//Serial.println("X3d");
		out = (uint32_t *)calloc(n, sizeof(uint32_t));
		//out = (uint32_t *)malloc(n*sizeof(uint32_t));
	//	displayFreeMemoryIfCritical(922);
		if (!out) return NULL;
		*out_len = n;
	}
	//for (i = 0; i < len; i++) {
	//    Serial.print(" ,");
	//    Serial.print((char)data[i]);
	//    Serial.print(", ");
	//    Serial.print((uint32_t)data[i]);
	//   Serial.print(", ");
	//}
	//Serial.println("");
	//Serial.println("X3e");
	for (i = 0; i < len; ++i) {
		out[i >> 2] |= (uint32_t)data[i] << ((i & 3) << 3);
	}

	return out;
}

static uint8_t * xxtea_to_ubyte_array(const uint32_t * data, size_t len, int inc_len, size_t * out_len) {
	uint8_t *out;
	size_t i;
	size_t m, n;

	n = len << 2;

	if (inc_len) {
		m = data[len - 1];
		n -= 4;
		if ((m < n - 3) || (m > n)) return NULL;
		n = m;
	}

	out = (uint8_t *)malloc(n + 1);
	//displayFreeMemoryIfCritical(923);

	for (i = 0; i < n; ++i) {
		out[i] = (uint8_t)(data[i >> 2] >> ((i & 3) << 3));
	}

	out[n] = '\0';
	*out_len = n;

	return out;
}

static uint32_t * xxtea_uint_encrypt(uint32_t * data, size_t len, uint32_t * key) {
	uint32_t n = (uint32_t)len - 1;
	uint32_t z = data[n], y, p, q = 6 + 52 / (n + 1), sum = 0, e;

	if (n < 1) return data;

	while (0 < q--) {
		sum += DELTA;
		e = sum >> 2 & 3;

		for (p = 0; p < n; p++) {
			y = data[p + 1];
			z = data[p] += MX;
		}

		y = data[0];
		z = data[n] += MX;
	}

	return data;
}

static uint32_t * xxtea_uint_decrypt(uint32_t * data, size_t len, uint32_t * key) {
	uint32_t n = (uint32_t)len - 1;
	uint32_t z, y = data[0], p, q = 6 + 52 / (n + 1), sum = q * DELTA, e;

	if (n < 1) return data;

	while (sum != 0) {
		e = sum >> 2 & 3;

		for (p = n; p > 0; p--) {
			z = data[p - 1];
			y = data[p] -= MX;
		}

		z = data[n];
		y = data[0] -= MX;
		sum -= DELTA;
	}

	return data;
}

static uint8_t * xxtea_ubyte_encrypt(const uint8_t * data, size_t len, const uint8_t * key, size_t * out_len) {
	uint8_t *out;
	uint32_t *data_array, *key_array;
	size_t data_len, key_len;
	if (!len) return NULL;

	data_array = xxtea_to_uint_array(data, len, 1, &data_len);
	//Serial.println("X3");
	if (!data_array) return NULL;
	//Serial.println(("X4"));
	key_array = xxtea_to_uint_array(key, 16, 0, &key_len);
	if (!key_array) {
		free(data_array);
		return NULL;
	}
	//Serial.println(("X5"));
	out = xxtea_to_ubyte_array(xxtea_uint_encrypt(data_array, data_len, key_array), data_len, 0, out_len);
	//Serial.println(("X6"));
	free(data_array);
	free(key_array);

	return out;
}

static uint8_t * xxtea_ubyte_decrypt(const uint8_t * data, size_t len, const uint8_t * key, size_t * out_len) {
	uint8_t *out;
	uint32_t *data_array, *key_array;
	size_t data_len, key_len;

	if (!len) return NULL;

	data_array = xxtea_to_uint_array(data, len, 0, &data_len);
	if (!data_array) return NULL;

	key_array = xxtea_to_uint_array(key, 16, 0, &key_len);
	if (!key_array) {
		free(data_array);
		return NULL;
	}

	out = xxtea_to_ubyte_array(xxtea_uint_decrypt(data_array, data_len, key_array), data_len, 1, out_len);

	free(data_array);
	free(key_array);

	return out;
}

// public functions

uint8_t * xxtea_encrypt(const uint8_t * data, size_t len, const uint8_t * key, size_t * out_len) {
	size_t i;
	uint8_t fixed_key[16];
	memcpy(fixed_key, key, 16);
//	displayFreeMemoryIfCritical(924);
	//for (i = 0; i < len; i++) {
	//    Serial.print(" _");
	//    Serial.print((char)data[i]);
	//    Serial.print("_ ");
	//}
	
	//Serial.println("");
	for (i = 0; (i < 16) && (fixed_key[i] != 0); ++i);
	for (++i; i < 16; ++i) fixed_key[i] = 0;
	return xxtea_ubyte_encrypt(data, len, fixed_key, out_len);
}

uint8_t * xxtea_decrypt(const uint8_t * data, size_t len, const uint8_t * key, size_t * out_len) {
	size_t i;
	uint8_t fixed_key[16];
	memcpy(fixed_key, key, 16);
	//for (i = 0; i < len; i++) {
	//    Serial.print(" .");
	//    Serial.print((char)data[i]);
	//    Serial.print(". ");
	//}
	//Serial.println("");
	for (i = 0; (i < 16) && (fixed_key[i] != 0); ++i);
	for (++i; i < 16; ++i) fixed_key[i] = 0;
	return xxtea_ubyte_decrypt(data, len, fixed_key, out_len);
}


const uint8_t Base64EncodeChars[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
	'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
	'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
	'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
	'w', 'x', 'y', 'z', '0', '1', '2', '3',
	'4', '5', '6', '7', '8', '9', '+', '/'
};

const char Base64DecodeChars[] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
	52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
	-1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
	-1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
	41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1
};

uint8_t * base64_encode(const uint8_t * data, size_t len) {
	uint8_t *out, *pos;
	const uint8_t *in = data;
	size_t i, quot, rem;
	long c;//int???

	if (!len) return NULL;

	quot = len / 3;
	rem = len % 3;
	out = (uint8_t *)malloc((quot + (rem ? 1 : 0)) * 4 + 1);
	//	displayFreeMemoryIfCritical(421);
	if (!out) return NULL;

	pos = out;

	for (i = 0; i < quot; i++) {
		c = (long)((0x000000ff & (int)*in++)) << 16;
		c |= (long)((0x000000ff & (int)*in++)) << 8;
		c |= 0x000000ffL & *in++;
		//	*pos++ = pgm_read_byte(&(Base64EncodeChars[c >> 18]));
		//	*pos++ = pgm_read_byte(&(Base64EncodeChars[c >> 12 & 0x3f]));
		//	*pos++ = pgm_read_byte(&(Base64EncodeChars[c >> 6 & 0x3f]));
		//	*pos++ = pgm_read_byte(&(Base64EncodeChars[c & 0x3f]));
		*pos++ = (Base64EncodeChars[c >> 18]);
		*pos++ = (Base64EncodeChars[c >> 12 & 0x3f]);
		*pos++ = (Base64EncodeChars[c >> 6 & 0x3f]);
		*pos++ = (Base64EncodeChars[c & 0x3f]);


	}

	if (rem == 1) {
		c = (long)(0x000000ff & (int)*in++);
		//	*pos++ = pgm_read_byte(&(Base64EncodeChars[c >> 2]));
		//	*pos++ = pgm_read_byte(&(Base64EncodeChars[(c & 0x03) << 4]));
		*pos++ = Base64EncodeChars[c >> 2];
		*pos++ = (Base64EncodeChars[(c & 0x03) << 4]);
		*pos++ = '=';
		*pos++ = '=';
	}
	else if (rem == 2) {
		c = (long)((0x000000ff & (int)*in++)) << 8;
		c |= (long)(0x000000ff & (int)*in++);
		//	*pos++ = pgm_read_byte(&(Base64EncodeChars[c >> 10]));
		//	*pos++ = pgm_read_byte(&(Base64EncodeChars[c >> 4 & 0x3f]));
		//	*pos++ = pgm_read_byte(&(Base64EncodeChars[(c & 0x0f) << 2]));
		*pos++ = (Base64EncodeChars[(c & 0x0f) << 2]);
		*pos++ = (Base64EncodeChars[c >> 4 & 0x3f]);
		*pos++ = (Base64EncodeChars[(c & 0x0f) << 2]);
		*pos++ = '=';
	}

	*pos = '\0';

	return out;
}

short int getUnsignedStringLength(uint8_t * string) {
	if (string == NULL)
		return 0;
	for (int i = 0; i < 1000; i++) {//longer strings are "empty" - should never happen
		if (string[i] == '\0')
			return i;
	}
	return 0;
}

uint8_t * base64_decode(const uint8_t * data, size_t * out_len) {
	uint8_t *out, *pos;
	const uint8_t *in = (const uint8_t *)data;
	size_t i, len, quot, rem, paddings = 0;
	long c; //int c;????

	len = getUnsignedStringLength((uint8_t *)data);
	if (!len) return NULL;

	rem = len % 4;
	if (rem) return NULL; // invalid size

	quot = len / 4;
	if (data[len - 2] == '=')
		paddings = 2;
	else if (data[len - 1] == '=')
		paddings = 1;
	out = (uint8_t *)malloc(quot * 3 - paddings + 1);
	//	displayFreeMemoryIfCritical(422);
	if (!out) return NULL;

	pos = out;

	for (i = 0; i < quot; i++) {
		//	c = ((long)pgm_read_byte(&(Base64DecodeChars[(int)*in++]))) << 18; //(int)*in++] << 18;
		//	c += ((long)pgm_read_byte(&(Base64DecodeChars[(int)*in++]))) << 12; //(int)*in++] << 12;
		c = (Base64DecodeChars[(int)*in++]) << 18; //(int)*in++] << 18;
		c += (Base64DecodeChars[(int)*in++]) << 12; //(int)*in++] << 12;
		*pos++ = (c & 0x00ff0000) >> 16;

		if (*in != '=') {
			// c += ((long)pgm_read_byte(&(Base64DecodeChars[(int)*in++]))) << 6;//(int)*in++] << 6;
			c += (Base64DecodeChars[(int)*in++]) << 6;//(int)*in++] << 6;
			*pos++ = (c & 0x0000ff00) >> 8;

			if (*in != '=') {
				//	c += ((long)pgm_read_byte(&(Base64DecodeChars[(int)*in++])));//(int)*in++];
				c += (Base64DecodeChars[(int)*in++]);
				*pos++ = c & 0x000000ff;
			}
		}
	}

	*pos = '\0';
	*out_len = pos - out;

	return out;
}


int main()
{
	Timer timer;
	timer.start();


	uint8_t * text_to_encrypt = (uint8_t *)"Bezpieczne metody dostepu do zasobow w obszarze Internetu Rzeczy, uwzgledniajace limity sprzetowo-programowe";

	uint8_t * encryptionKey = (uint8_t *)"4815162342=108";

	// tu wywołujemy szyfrowanie, metoda przydziela pamięć dla wyniku przez calloc()
	
	size_t inp_len = getUnsignedStringLength(text_to_encrypt);
	size_t out_len;

	uint8_t * xxtea_data = xxtea_encrypt(text_to_encrypt, inp_len, encryptionKey, &out_len);

	size_t s = getUnsignedStringLength(xxtea_data);

	uint8_t * base64_data = base64_encode(xxtea_data, s);

	size_t inp_len_dec = getUnsignedStringLength(xxtea_data);
	size_t out_len_dec;
	uint8_t * xxtea_decrypted_data = xxtea_decrypt(xxtea_data, inp_len_dec, encryptionKey, &out_len_dec);

	int iterationCount = 100000;

	for (int i = 0; i < iterationCount; i++)
	{
		text_to_encrypt = (uint8_t *)"Bezpieczne metody dostepu do zasobow w obszarze Internetu Rzeczy, uwzgledniajace limity sprzetowo-programowe";

		encryptionKey = (uint8_t *)"4815162342=108";

		inp_len = getUnsignedStringLength(text_to_encrypt);
		out_len;

		xxtea_data = xxtea_encrypt(text_to_encrypt, inp_len, encryptionKey, &out_len);

		s = getUnsignedStringLength(xxtea_data);

		base64_data = base64_encode(xxtea_data, s);
		
		inp_len_dec = getUnsignedStringLength(xxtea_data);
		out_len_dec;
		xxtea_decrypted_data = xxtea_decrypt(xxtea_data, inp_len_dec, encryptionKey, &out_len_dec);

	}

	
	timer.stop();

	printf("Liczba iteracji: %d", iterationCount);
	printf("\n");
	printf("Sekundy: %f", timer.elapsedSeconds());
	printf("\n");
	printf("Milisekundy: %f", timer.elapsedMilliseconds());

	//	Serial.println("Sekundy: %f", timer.elapsedSeconds());
	//	Serial.println("Milisekundy: %f", timer.elapsedMilliseconds());
}

// Uruchomienie programu: Ctrl + F5 lub menu Debugowanie > Uruchom bez debugowania
// Debugowanie programu: F5 lub menu Debugowanie > Rozpocznij debugowanie

// Porady dotyczące rozpoczynania pracy:
//   1. Użyj okna Eksploratora rozwiązań, aby dodać pliki i zarządzać nimi
//   2. Użyj okna programu Team Explorer, aby nawiązać połączenie z kontrolą źródła
//   3. Użyj okna Dane wyjściowe, aby sprawdzić dane wyjściowe kompilacji i inne komunikaty
//   4. Użyj okna Lista błędów, aby zobaczyć błędy
//   5. Wybierz pozycję Projekt > Dodaj nowy element, aby utworzyć nowe pliki kodu, lub wybierz pozycję Projekt > Dodaj istniejący element, aby dodać istniejące pliku kodu do projektu
//   6. Aby w przyszłości ponownie otworzyć ten projekt, przejdź do pozycji Plik > Otwórz > Projekt i wybierz plik sln
