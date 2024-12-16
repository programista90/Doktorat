// Base64.cpp : Ten plik zawiera funkcję „main”. W nim rozpoczyna się i kończy wykonywanie programu.
//

#include <iostream>

#include "base64.h"
#include "Timer.h"
#include "base64.h"

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
	
	int s = getUnsignedStringLength(text_to_encrypt);

	int iterationCount = 1000;

	for (int i = 0; i < iterationCount; i++)
	{
		text_to_encrypt = (uint8_t *)"Bezpieczne metody dostepu do zasobow w obszarze Internetu Rzeczy, uwzgledniajace limity sprzetowo-programowe";
		
		uint8_t * base64_data = base64_encode(text_to_encrypt, s);
	}

	

//		displayFreeMemoryIfCritical(225);
	//Serial.print("encrypt='");
	//Serial.print((char *)encrypt_data);
	//Serial.println("'");

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
