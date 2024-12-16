/*
 ============================================================================
 Name        : huffman.c
 Author      : J. Rykowski
 Version     : 1.01
 Copyright   : (c) PUE/DIT 2023
 Description : Generation of Huffman/LZW tree
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;

char * version="Short-data encryption tester v.1.02 25.05.2023 11:30";

//data structures
//alphabet table
uint8_t alphabetNoNoise[]={'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.', '+', '-'};
uint8_t noiseOnly[]=      {'@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '=', '!', '~', ' '}; //the last space will not be used for adding noise
uint8_t alphabetWithNoise[26];
//no of elements of the set of all possible chars must be equal to alphabet + noise - exactly!
#define ALPHABET_SIZE ( sizeof(alphabetWithNoise) )
#define ALPHABET_NO_NOISE_SIZE ( sizeof(alphabetNoNoise) )
#define NOISE_SIZE ( sizeof(noiseOnly)-1 )
#define MAX_SUBPART_SIZE ( 2 )

#define MAX_STRING_LENGTH 4

#if MAX_SUBPART_SIZE == 3
#define LZW_TABLE_LENGTH ( (ALPHABET_SIZE+NOISE_SIZE)*(ALPHABET_SIZE+NOISE_SIZE)*(ALPHABET_SIZE+NOISE_SIZE) )
//(int)(pow(ALPHABET_SIZE+NOISE_SIZE, MAX_SUBPART_SIZE)) )
//power() function cannot be used as constant above!
#define HUFFMAN_TABLE_LENGTH ( LZW_TABLE_LENGTH + 1 )
#endif
#if MAX_SUBPART_SIZE == 2
#define LZW_TABLE_LENGTH ( (ALPHABET_SIZE+NOISE_SIZE)*(ALPHABET_SIZE+NOISE_SIZE) )
//(int)(pow(ALPHABET_SIZE+NOISE_SIZE, MAX_SUBPART_SIZE)) )
//power() function cannot be used as constant above!
#define HUFFMAN_TABLE_LENGTH ( LZW_TABLE_LENGTH + 1 )
#endif

//parameters for random generators
#define MAX_OCCURRENCE 6
#define SHUFFLE_COUNTER LZW_TABLE_LENGTH

//parameters for printing tables
//#define PRINT_TABLE_INITIAL true
//#define PRINT_TABLE_SHUFFLED true
//#define PRINT_TABLE_MIDDLE_SHUFFLED true
//#define PRINT_TABLE_INSERTED true
//#define PRINT_TABLE_FINAL true

typedef  struct{
	uint32_t stringAsInteger;
	uint8_t stringAsString[MAX_STRING_LENGTH+1]; //max 4 characters per string
	uint32_t noOfOccurrences;
	bool alreadyProcessed;
} LZW_record;

//LZW temporal table
LZW_record LZW_table[LZW_TABLE_LENGTH];
int LZW_noOf1CharItems=0;
int LZW_noOf2CharItems=0;
int LZW_noOf3CharItems=0;
int LZW_noOfAllItems=0;
int LZW_maxOccurrence=0;

typedef  struct{
	int leftChild;
	int rightChild;
	int parent;
	uint32_t stringAsInteger;
	uint8_t stringAsString[MAX_STRING_LENGTH+1]; //max 4 characters per string
	uint32_t noOfOccurrences;
	bool used;
} HUFFMAN_record;


//Huffman final table, including LZW items
int HUFFMAN_noOfAllItems=0;
HUFFMAN_record HUFFMAN_table[HUFFMAN_TABLE_LENGTH];
HUFFMAN_record HUFFMAN_temporalTable[HUFFMAN_TABLE_LENGTH];
int HUFFMAN_root=-1;

double HUFFMAN_minLevelWeighted=1000.0;
double HUFFMAN_maxLevelWeighted=0.0;
double HUFFMAN_averageLevelWeighted=0.0;
int HUFFMAN_minLevelAbsolute=1000;
int HUFFMAN_maxLevelAbsolute=0;
int HUFFMAN_averageLevelAbsolute=0;
double HUFFMAN_compressionFactor=0.0;

//TODO: the string-operating methods do not check memory overflow !
void prepareStringWithNoise(uint8_t * stringNoNoise, uint8_t * stringWithNoise){
	int pos=0;
	stringWithNoise[pos]='\0';
	int noisePositions[21];
	int strl=strlen((char *)stringNoNoise);
	int noOfNoiseChars=(strl*2)/6;//adding 33% of noise, but no more than 20 characters
		//selecting random positions the noise is to be added
	if (noOfNoiseChars>=20)
		noOfNoiseChars=19;//to not overflow the buffer
	if (noOfNoiseChars<=0)
		noOfNoiseChars=1;
	//int totalChars=strl+noOfNoiseChars;
	//if ((totalChars/2)*2 != totalChars)
	//	noOfNoiseChars++;//always in pairs to avoid single chars
	for (int i=0; i<noOfNoiseChars; i++){
		noisePositions[i]=rand() % strl;
	}
	noisePositions[0]=0;//to be sure noise begins the message
	for (int i=0;i<strl;i++){
		for (int n=0;n<noOfNoiseChars; n++){
			if (noisePositions[n]==i){
				stringWithNoise[pos++]=noiseOnly[rand()%(NOISE_SIZE)];
			}
		}
		stringWithNoise[pos++]=stringNoNoise[i];
		stringWithNoise[pos]='\0';
	}
}
bool isNoiseCharacter(uint8_t c){
	for (int i=0; i<NOISE_SIZE+1; i++){
		if (noiseOnly[i]==c)
			return true;
	}
	return false;
}
void prepareStringWithNoNoise(uint8_t * stringWithNoise, uint8_t * stringNoNoise){
	int pos=0;
	stringNoNoise[pos]='\0';
	int strl=strlen((char *)stringWithNoise);
	for (int i=0;i<strl;i++){
		if (!isNoiseCharacter(stringWithNoise[i])){
			stringNoNoise[pos++]=stringWithNoise[i];
			stringNoNoise[pos]='\0';
		}
	}
}

int getFirstPositionWithEqualCharacters(uint8_t * stringInput, int minNoOfCharactersToBeEqual){
	if (stringInput[0]=='\0')//should never happen, this is the very last element of Huffman tree (empty string)
		return -1;
	for (int i=0; i<HUFFMAN_noOfAllItems;i++){
		uint8_t * huffmanString=HUFFMAN_table[i].stringAsString;
		switch (minNoOfCharactersToBeEqual){
			case 1:
				if (stringInput[0]==huffmanString[0] && huffmanString[1]=='\0'){
					//printf("found c1 of '%s' at index %d '%s'\n", stringInput, i, huffmanString);
					return i;
				}
				break;
			case 2:
				if (stringInput[0]==huffmanString[0] &&
					stringInput[1]==huffmanString[1] && huffmanString[2]=='\0'){
					//printf("found c2 of '%s' at index %d '%s'\n", stringInput, i, huffmanString);
					return i;
				}
				break;
			case 3:
				if (stringInput[0]==huffmanString[0] &&
					stringInput[1]==huffmanString[1] &&
					stringInput[2]==huffmanString[2] && huffmanString[3]=='\0'){
					//printf("found c3 of '%s' at index %d '%s'\n", stringInput, i, huffmanString);
					return i;
				}
				break;
			case 4:
				if (stringInput[0]==huffmanString[0] &&
					stringInput[1]==huffmanString[1] &&
					stringInput[2]==huffmanString[2] &&
					stringInput[3]==huffmanString[3] && huffmanString[4]=='\0'){
					//printf("found c4 of '%s' at index %d '%s'\n", stringInput, i, huffmanString);
					return i;
				}
				break;
		}//switch
	}
	//printf("ERROR: bad character in input string: '%s'\n", stringInput);
	return -1; //should never happen - this means that input string contains a not valid character
}

int getLengthToRoot(int itemNo){
	if (itemNo<0)
		return -1;
	HUFFMAN_record item=HUFFMAN_table[itemNo];
	if (item.parent==itemNo)
		return 1;
	return (1+getLengthToRoot(item.parent));
}
void addItemsToRoot(int itemNo, uint8_t * result){
	if (itemNo<0)
		return;
	HUFFMAN_record item=HUFFMAN_table[itemNo];
	if (item.parent==itemNo)
		return;
	HUFFMAN_record parent=HUFFMAN_table[item.parent];
	if (itemNo==parent.leftChild){
		strcat((char *)result, "0");
	}else{
		strcat((char *)result, "1");
	}
	addItemsToRoot(item.parent, result);
}
void reverseString(uint8_t *s){
	if (strlen((char *)s)<=1){
		return;
	}else if (strlen((char *)s)==2){
		uint8_t t=s[0];
		s[0]=s[1];
		s[1]=t;
		return;
	}else{
		uint8_t buff[128];
		int pos=0;
		for (int i=strlen((char *)s)-1; i>=0; i--){
			buff[pos++]=s[i];
		}
		buff[pos]='\0';
		for (int i=0; i<=strlen((char *)s); i++){
			s[i]=buff[i];
		}
	}
}
void complimentStringWithEndingZeros(uint8_t * stringOutput){
	int strl=strlen((char *)stringOutput);
	if (strl<=0)
		return;
	int missingZeros=strl%8;
	if (missingZeros==0)
		return;
	missingZeros=8-missingZeros;
	for (int i=0; i<missingZeros;i++){
		strcat((char *)stringOutput,(char *)"0");
	}
}
int encodeStringMax2Bytes(uint8_t * stringInput, uint8_t * stringOutput){
	//printf("Inspecting encoding 2B '%s'\n", stringInput);
	//for (int i=0; i<strlen((char *)stringInput)+1;i++){
	//	stringOutput[i]=stringInput[i];
	//}
	//returning number of characters to advance, 0 in case of the error (no substring detected)
	//for a single character - just finding one element and exiting with its index
	if (strlen((char *)stringInput)==0){
		//printf("ERROR: empty input string\n");
		return 0;
	}
	int ret=1;
	uint8_t stringBuffer[128];
	if (strlen((char *)stringInput)==1){
		int pos1=getFirstPositionWithEqualCharacters(stringInput, 1);
		stringBuffer[0]='\0';
		addItemsToRoot(pos1, stringBuffer);
		reverseString(stringBuffer);
		strcat((char *)stringOutput, (char *)stringBuffer);
	}else {
		int pos1=getFirstPositionWithEqualCharacters(stringInput, 1);
		int pos2=getFirstPositionWithEqualCharacters(stringInput, 2);
		//int weight1=getLengthToRoot(pos1);
		//int weight2=getLengthToRoot(pos2);
		if (pos1>=0 && pos2>=0){
			pos1=pos2;//longer string always better
			ret=2;
		}else if (pos1<0){
			pos1=pos2;
			ret=2;
		}//if 2 is less than zero, no problem here
		//here pos1 contains valid and "the best" index
		if (pos1>=0){//must be like this here - just to be sure
			stringBuffer[0]='\0';
			addItemsToRoot(pos1, stringBuffer);
			reverseString(stringBuffer);
			strcat((char *)stringOutput, (char *)stringBuffer);
			//printf("Add item: %d %d '%s'->'%s'\n", pos1, ret, stringInput, stringOutput);
		}else{
			printf("ERROR: bad character in input string ???: '%s'\n", stringInput);
			ret=0; //snh
		}

	}
	//printf("Compressing '%s' of len %d to '%s'\n", stringInput, ret, stringOutput);
	return encodeStringMax2Bytes(stringInput+ret, stringOutput);
}

void decodeString(uint8_t * stringInput, uint8_t * stringOutput){
	//for (int i=0; i<strlen((char *)stringInput)+1;i++){
	//	stringOutput[i]=stringInput[i];
	//}
	//printf("Inspecting decoding input='%s'\n", stringInput);
	int pos=0;
	int strl=strlen((char *)stringInput);
	if (strl==0)
		return;
	if (strl<=7){
		//possibly ending zeros, checking
		int noOfOnes=0;
		for (int i=0; i<strl; i++){
			if (stringInput[i]=='1')
				noOfOnes++;
		}
		if (noOfOnes==0){
			//ending zeroes, truncating and exiting
			return;
		}
	}
	uint8_t c=stringInput[pos];
	bool finished=false;
	int currentItem=HUFFMAN_root;
	int posE=strlen((char *)stringOutput);
	while (!finished){
		//if (c=='\0')
		///	finished=true;
		//else{
			HUFFMAN_record item=HUFFMAN_table[currentItem];
			if (item.leftChild<0 && item.rightChild<0){
				//end item, getting string and putting to the output
				if (item.stringAsString[0]!='\0'){
					stringOutput[posE++]=item.stringAsString[0];
				}
				if (item.stringAsString[1]!='\0'){
					stringOutput[posE++]=item.stringAsString[1];
				}
				if (item.stringAsString[2]!='\0'){
					stringOutput[posE++]=item.stringAsString[2];
				}
				if (item.stringAsString[3]!='\0'){
					stringOutput[posE++]=item.stringAsString[3];
				}
				stringOutput[posE]='\0';
				finished=true;
			}else{
				//going further, intermediate node only
				if (c=='0'){
					currentItem=item.leftChild;
				}else{
					currentItem=item.rightChild;
				}
				pos++;
				c=stringInput[pos];
			}
		//}
	}
	if (c!='\0'){
		decodeString(stringInput+pos, stringOutput);
	}
}

void processSingleString(uint8_t * stringToSend){


	struct timeval moment_begin, moment_end;
	clock_t momentBegin, momentEnd;
	double cpu_time_used;

	momentBegin = clock();
    gettimeofday(&moment_begin, 0);

	//encoding and decoding sample strings of characters
	printf("\nString at the input of len %dB = '%s'\n", strlen((char *)stringToSend), stringToSend);
	uint8_t stringWithNoise[128];//assuming here maximum length of input string in bytes with noise added (50% extra)
	uint8_t stringEncoded[100000];//assuming here maximum length of input string in bits ('0'/'1' char representation for testing purposes)
	prepareStringWithNoise(stringToSend, stringWithNoise);
	printf("String with noise of len %dB = '%s'\n", strlen((char *)stringWithNoise), stringWithNoise);
	stringEncoded[0]='\0';
	encodeStringMax2Bytes(stringWithNoise, stringEncoded);
	complimentStringWithEndingZeros(stringEncoded);
	printf("String encoded of len %dB = '%s'\n", (strlen((char *)stringEncoded)+7)/8, stringEncoded );
	uint8_t stringDecodedWithNoise[128];
	stringDecodedWithNoise[0]='\0';
	decodeString(stringEncoded, stringDecodedWithNoise);
	printf("String decoded with noise of len %dB = '%s'\n", strlen((char *)stringDecodedWithNoise), stringDecodedWithNoise);
	uint8_t stringDecodedNoNoise[128];
	prepareStringWithNoNoise(stringDecodedWithNoise, stringDecodedNoNoise);
	printf("String decoded no noise of len %dB = '%s'\n", strlen((char *)stringDecodedNoNoise), stringDecodedNoNoise);

	momentEnd = clock();
    gettimeofday(&moment_end, 0);
	cpu_time_used = ((double) (momentEnd - momentBegin)) / CLOCKS_PER_SEC;
	long time_used = moment_end.tv_usec - moment_begin.tv_usec;
	if (cpu_time_used>0.0000001)
		printf("Timing = %f s\n", (float)cpu_time_used);
	if (time_used>0L)
		printf("Timing = %d us\n", (int)time_used);
	//In general the above timing functions are not working for Eclipse C/C++ | MinGC - CPU is too fast
}

void prepareAlphabet(){
	printf("Init alphabet with some noise\n");
	int currentLetter=0;
	for (int i=0; i<ALPHABET_NO_NOISE_SIZE; i++){
		alphabetWithNoise[currentLetter++]=alphabetNoNoise[i];
	}
	for (int i=0; i<NOISE_SIZE; i++){
		alphabetWithNoise[currentLetter++]=noiseOnly[i];
	}
	printf("Alphabet with some noise ready to use\n");
}
void prepareLZW(){
	printf("Preparing LZW data structure\n");
	//the below code is for subpart 2 or 3 chars maximum
#if MAX_SUBPART_SIZE >= 2
	printf("Max size of LZW record - 2 bytes\n");
	int i_currentLZWRecord=0;
	int maxLevelLZW=2000; //maximum level of non-single items
	//first generating single symbols
	for (int i_firstChar=0; i_firstChar<ALPHABET_SIZE; i_firstChar++){
		uint8_t firstChar=alphabetWithNoise[i_firstChar];
		uint8_t secondChar='\0';
		uint8_t thirdChar='\0';
		uint8_t fourthChar='\0';
		//LZW_record currentLZWRecord=LZW_table[i_currentLZWRecord];
		LZW_table[i_currentLZWRecord].stringAsString[0]=firstChar;
		LZW_table[i_currentLZWRecord].stringAsString[1]=secondChar;
		LZW_table[i_currentLZWRecord].stringAsString[2]=thirdChar;
		LZW_table[i_currentLZWRecord].stringAsString[3]=fourthChar;
		LZW_table[i_currentLZWRecord].stringAsInteger=fourthChar+256*thirdChar+256*256*secondChar+256*256*256*firstChar;
		LZW_table[i_currentLZWRecord].noOfOccurrences=(rand() % MAX_OCCURRENCE);//0; //1-char subparts are an exception //rand() % MAX_OCCURRENCE;
		LZW_table[i_currentLZWRecord].alreadyProcessed=false;
#ifdef PRINT_TABLE_INITIAL
		printf("Record 1-char #%d: '%s'->0x%x, no=%d\n",
			i_currentLZWRecord,
			(char *)(LZW_table[i_currentLZWRecord].stringAsString),
			(unsigned int)LZW_table[i_currentLZWRecord].stringAsInteger,
			(int)LZW_table[i_currentLZWRecord].noOfOccurrences);
#endif
		//the below not really needed here (always zero occurrences), but who knows?
		if (LZW_maxOccurrence<LZW_table[i_currentLZWRecord].noOfOccurrences){
			LZW_maxOccurrence=LZW_table[i_currentLZWRecord].noOfOccurrences;
			//printf("New max occurrence 1-char = %d\n", LZW_maxOccurrence);
		}
		i_currentLZWRecord++;
		LZW_noOf1CharItems++;
	}
	//middle - generating 2-char long symbols
	for (int i_firstChar=0; i_firstChar<ALPHABET_SIZE; i_firstChar++){
		uint8_t firstChar=alphabetWithNoise[i_firstChar];
		for (int i_secondChar=0; i_secondChar<ALPHABET_SIZE; i_secondChar++){
			uint8_t secondChar=alphabetWithNoise[i_secondChar];
			uint8_t thirdChar='\0';
			uint8_t fourthChar='\0';
			//LZW_record currentLZWRecord=LZW_table[i_currentLZWRecord];
			LZW_table[i_currentLZWRecord].stringAsString[0]=firstChar;
			LZW_table[i_currentLZWRecord].stringAsString[1]=secondChar;
			LZW_table[i_currentLZWRecord].stringAsString[2]=thirdChar;
			LZW_table[i_currentLZWRecord].stringAsString[3]=fourthChar;
			LZW_table[i_currentLZWRecord].stringAsInteger=fourthChar+256*thirdChar+256*256*secondChar+256*256*256*firstChar;
			LZW_table[i_currentLZWRecord].noOfOccurrences=(rand() % MAX_OCCURRENCE); //most common values
			if (LZW_table[i_currentLZWRecord].noOfOccurrences==0){
				LZW_table[i_currentLZWRecord].noOfOccurrences=2; //just to be sure it is "better" than a single char
			}
			LZW_table[i_currentLZWRecord].alreadyProcessed=false;
#ifdef PRINT_TABLE_INITIAL
			printf("Record 2-char #%d: '%s'->0x%x, no=%d\n",
				i_currentLZWRecord,
				(char *)(LZW_table[i_currentLZWRecord].stringAsString),
				(unsigned int)LZW_table[i_currentLZWRecord].stringAsInteger,
				(int)LZW_table[i_currentLZWRecord].noOfOccurrences);
#endif
			if (LZW_maxOccurrence<((int)(LZW_table[i_currentLZWRecord].noOfOccurrences))){
				LZW_maxOccurrence=(int)(LZW_table[i_currentLZWRecord].noOfOccurrences);
				//printf("New max occurrence 2-char = %d\n", LZW_maxOccurrence);
			}
			i_currentLZWRecord++;
			LZW_noOf2CharItems++;
			if (LZW_noOf2CharItems>maxLevelLZW){
				i_firstChar=ALPHABET_SIZE;
				i_secondChar=ALPHABET_SIZE;//finishing both loops to not overload tree size
			}
		}
	}
#if MAX_SUBPART_SIZE >= 3
/*	//last - generating 3-char long symbols
	for (int i_firstChar=0; i_firstChar<ALPHABET_SIZE; i_firstChar++){
		uint8_t firstChar=alphabetWithNoise[i_firstChar];
		for (int i_secondChar=0; i_secondChar<ALPHABET_SIZE; i_secondChar++){
			uint8_t secondChar=alphabetWithNoise[i_secondChar];
			for (int i_thirdChar=0; i_thirdChar<ALPHABET_SIZE; i_thirdChar++){
				uint8_t thirdChar=alphabetWithNoise[i_thirdChar];
				uint8_t fourthChar='\0';
				//LZW_record currentLZWRecord=LZW_table[i_currentLZWRecord];
				LZW_table[i_currentLZWRecord].stringAsString[0]=firstChar;
				LZW_table[i_currentLZWRecord].stringAsString[1]=secondChar;
				LZW_table[i_currentLZWRecord].stringAsString[2]=thirdChar;
				LZW_table[i_currentLZWRecord].stringAsString[3]=fourthChar;
				LZW_table[i_currentLZWRecord].stringAsInteger=fourthChar+256*thirdChar+256*256*secondChar+256*256*256*firstChar;
				LZW_table[i_currentLZWRecord].noOfOccurrences=(rand() % MAX_OCCURRENCE)/3; //not so common values
				if (LZW_table[i_currentLZWRecord].noOfOccurrences==0){
					LZW_table[i_currentLZWRecord].noOfOccurrences=1; //just to be sure it is "better" than a single char
				}
				LZW_table[i_currentLZWRecord].alreadyProcessed=false;
#ifdef PRINT_TABLE_INITIAL
				printf("Record 3-char #%d: '%s'->0x%x, no=%d\n",
						i_currentLZWRecord,
						(char *)(LZW_table[i_currentLZWRecord].stringAsString),
						(unsigned int)LZW_table[i_currentLZWRecord].stringAsInteger,
						(int)LZW_table[i_currentLZWRecord].noOfOccurrences);
#endif
				if (LZW_maxOccurrence<LZW_table[i_currentLZWRecord].noOfOccurrences){
					LZW_maxOccurrence=LZW_table[i_currentLZWRecord].noOfOccurrences;
					printf("New max occurrence 3-char = %d\n", LZW_maxOccurrence);
				}
				i_currentLZWRecord++;
				LZW_noOf3CharItems++;
			}
		}
	}*/
#endif

#endif
	LZW_noOfAllItems=LZW_noOf1CharItems+LZW_noOf2CharItems+LZW_noOf3CharItems;
	printf("LZW table prepared, no of items=%d, max occurrence=%d\n", LZW_noOfAllItems, LZW_maxOccurrence);

	printf("Shuffle the table to get real random locations\n");
	int shuffleCnt=0;
	for (int i=0; i<LZW_noOfAllItems; i++){
		int firstIndex=rand() % i_currentLZWRecord;
		int secondIndex=rand() % i_currentLZWRecord;
		if (firstIndex!=secondIndex){
			LZW_record temp;
			//LZW_record firstLZWRecord=LZW_table[firstIndex];
			//LZW_record secondLZWRecord=LZW_table[secondIndex];
#ifdef PRINT_TABLE_MIDDLE_SHUFFLED
				printf("Record shuffling #%d->%d\n",firstIndex, secondIndex);
#endif
			temp.noOfOccurrences=LZW_table[firstIndex].noOfOccurrences;
			temp.stringAsInteger=LZW_table[firstIndex].stringAsInteger;
			for (int s=0; s<MAX_STRING_LENGTH+1;s++){
				temp.stringAsString[s]=LZW_table[firstIndex].stringAsString[s];
			}
#ifdef PRINT_TABLE_MIDDLE_SHUFFLED
			printf("Record temp-shuffle #%d: '%s'->0x%x, no=%d\n",
					i,
					(char *)(temp.stringAsString),
					(unsigned int)temp.stringAsInteger,
					(int)temp.noOfOccurrences);
#endif
#ifdef PRINT_TABLE_MIDDLE_SHUFFLED
			printf("Record first-shuffle #%d: '%s'->0x%x, no=%d\n",
					i,
					(char *)(LZW_table[firstIndex].stringAsString),
					(unsigned int)LZW_table[firstIndex].stringAsInteger,
					(int)LZW_table[firstIndex].noOfOccurrences);
#endif
#ifdef PRINT_TABLE_MIDDLE_SHUFFLED
			printf("Record second-shuffle #%d: '%s'->0x%x, no=%d\n",
					i,
					(char *)(LZW_table[secondIndex].stringAsString),
					(unsigned int)LZW_table[secondIndex].stringAsInteger,
					(int)LZW_table[secondIndex].noOfOccurrences);
#endif
			LZW_table[firstIndex].noOfOccurrences=LZW_table[secondIndex].noOfOccurrences;
			LZW_table[firstIndex].stringAsInteger=LZW_table[secondIndex].stringAsInteger;
			for (int s=0; s<MAX_STRING_LENGTH+1;s++){
				LZW_table[firstIndex].stringAsString[s]=LZW_table[secondIndex].stringAsString[s];
			}
			LZW_table[secondIndex].noOfOccurrences=temp.noOfOccurrences;
			LZW_table[secondIndex].stringAsInteger=temp.stringAsInteger;
			for (int s=0; s<MAX_STRING_LENGTH+1;s++){
				LZW_table[secondIndex].stringAsString[s]=temp.stringAsString[s];
			}
			shuffleCnt++;
		}
	}
	printf("Table shuffled %d times\n", shuffleCnt);

	//printing shuffled table
#ifdef PRINT_TABLE_SHUFFLED
	for (int i=0; i<i_currentLZWRecord; i++){
		LZW_record currentLZWRecord=LZW_table[i];
		printf("Record shuffled #%d: '%s'->0x%x, no=%d\n",
			i,
			(char *)(currentLZWRecord.stringAsString),
			(unsigned int)currentLZWRecord.stringAsInteger,
			(int)currentLZWRecord.noOfOccurrences);
	}
#endif
	//sorting elements according to number of occurrences to build the Huffmann graph
	//idea: more frequent elements comes to be upper parts of the graph, the "equal" elements are put in the random order

	printf("Filling LZW table with random data finished\n");
}

int addToTemporaryTable(int indexLeft, int indexRight, int noOfOccurrences){
	if (indexLeft>=0 && indexRight>=0){
		//find first element in temporary table free of use
		for (int i=0; i<HUFFMAN_TABLE_LENGTH; i++){
			if (!HUFFMAN_temporalTable[i].used){
				HUFFMAN_temporalTable[i].used=true;
				HUFFMAN_temporalTable[i].leftChild=indexLeft;
				HUFFMAN_temporalTable[i].rightChild=indexRight;
				HUFFMAN_temporalTable[i].noOfOccurrences=noOfOccurrences;
				HUFFMAN_temporalTable[i].stringAsString[0]='\0';
				HUFFMAN_temporalTable[i].stringAsString[1]='\0';
				HUFFMAN_temporalTable[i].stringAsString[2]='\0';
				HUFFMAN_temporalTable[i].stringAsString[3]='\0';
				HUFFMAN_temporalTable[i].stringAsString[4]='\0';
				//printf("New temporary element %d with indices %d, %d\n", i, indexLeft, indexRight);
				return i;
			}
		}
		printf("ERROR: no place for new temporary element with indices %d, %d\n", indexLeft, indexRight);
	}else{
		//snh
		printf("ERROR: adding non existing LZW element(s) to temporary table: %d, %d\n", indexLeft, indexRight);
		return -1;
	}
	return -1; //snh
}

int addToTemporaryTableFinalNode(int indexLZW){
	if (indexLZW>=0 && indexLZW<=LZW_TABLE_LENGTH){
		LZW_record itemLZW=LZW_table[indexLZW];
		//find first empty index in the temporary table
		for (int i=0; i<HUFFMAN_TABLE_LENGTH; i++){
			if (!HUFFMAN_temporalTable[i].used){
				HUFFMAN_temporalTable[i].used=true;
				HUFFMAN_temporalTable[i].leftChild=-1;
				HUFFMAN_temporalTable[i].rightChild=-1;
				HUFFMAN_temporalTable[i].noOfOccurrences=itemLZW.noOfOccurrences;
				HUFFMAN_temporalTable[i].stringAsInteger=itemLZW.stringAsInteger;
				HUFFMAN_temporalTable[i].stringAsString[0]=itemLZW.stringAsString[0];
				HUFFMAN_temporalTable[i].stringAsString[1]=itemLZW.stringAsString[1];
				HUFFMAN_temporalTable[i].stringAsString[2]=itemLZW.stringAsString[2];
				HUFFMAN_temporalTable[i].stringAsString[3]=itemLZW.stringAsString[3];
				HUFFMAN_temporalTable[i].stringAsString[4]=itemLZW.stringAsString[4];
				//printf("New temporary final element %d for '%s'\n", i, HUFFMAN_temporalTable[i].stringAsString);
				return i;
			}
		}
		printf("ERROR: no place for new temporary element with index %d\n", indexLZW);
	}else{
		//snh
		printf("ERROR: adding non existing LZW element(s) to temporary table: %d\n", indexLZW);
		return -1;
	}
	return -1; //snh
}

int moveToHuffmanTable(int indexInTemporaryTable){
	if (indexInTemporaryTable>=0){
		HUFFMAN_record itemTemp=HUFFMAN_temporalTable[indexInTemporaryTable];
		HUFFMAN_table[HUFFMAN_noOfAllItems].leftChild=itemTemp.leftChild;
		HUFFMAN_table[HUFFMAN_noOfAllItems].rightChild=itemTemp.rightChild;
		HUFFMAN_table[HUFFMAN_noOfAllItems].noOfOccurrences=itemTemp.noOfOccurrences;
		HUFFMAN_table[HUFFMAN_noOfAllItems].stringAsString[0]=itemTemp.stringAsString[0];
		HUFFMAN_table[HUFFMAN_noOfAllItems].stringAsString[1]=itemTemp.stringAsString[1];
		HUFFMAN_table[HUFFMAN_noOfAllItems].stringAsString[2]=itemTemp.stringAsString[2];
		HUFFMAN_table[HUFFMAN_noOfAllItems].stringAsString[3]=itemTemp.stringAsString[3];
		HUFFMAN_table[HUFFMAN_noOfAllItems].stringAsString[4]=itemTemp.stringAsString[4];
		//strings are of no use here - that's intermediate tree node
		HUFFMAN_temporalTable[indexInTemporaryTable].used=false; //now removing this item from temporary table
		//printf("Moved temporary element %d with indices %d, %d\n", HUFFMAN_noOfAllItems, HUFFMAN_table[HUFFMAN_noOfAllItems].leftChild, HUFFMAN_table[HUFFMAN_noOfAllItems].rightChild);

		//updating parent info if any children
		if (HUFFMAN_table[HUFFMAN_noOfAllItems].leftChild>=0){
			HUFFMAN_table[HUFFMAN_table[HUFFMAN_noOfAllItems].leftChild].parent=HUFFMAN_noOfAllItems;
		}
		if (HUFFMAN_table[HUFFMAN_noOfAllItems].rightChild>=0){
			HUFFMAN_table[HUFFMAN_table[HUFFMAN_noOfAllItems].rightChild].parent=HUFFMAN_noOfAllItems;
		}

		HUFFMAN_noOfAllItems++;
		return HUFFMAN_noOfAllItems-1;
	}else{
		//snh
		printf("ERROR: adding non existing LZW element to Huffman table: %d\n", indexInTemporaryTable);
		return -1;
	}
	return -1;//snh
}

void prepareHUFFMAN(){
	printf("Preparing Huffman binary tree\n");
	HUFFMAN_noOfAllItems=0;
	//preparing temporal table - setting all items to non-existing by making the indices negative
	for (int i=0; i<HUFFMAN_TABLE_LENGTH; i++){
		HUFFMAN_temporalTable[i].leftChild=-1;
		HUFFMAN_temporalTable[i].rightChild=-1;
		HUFFMAN_temporalTable[i].parent=-1;
		HUFFMAN_temporalTable[i].used=false;
		HUFFMAN_table[i].leftChild=-1;
		HUFFMAN_table[i].rightChild=-1;//the two latter probably not needed, but who knows?
		HUFFMAN_table[i].used=false;
	}
	//moving all elements from LZW to temporary table, stating "no children" and weighted occurrence
	for (int i=0; i<LZW_noOfAllItems; i++){
		//int i1=
				addToTemporaryTableFinalNode(i);
		//printf("Added final node to temporary table %d-> %d\n", i, i1);
	}
	//linking pairs one by one with lowest-occurrence elements
	HUFFMAN_root=-1;//so far we had no candidate for the root, but surely we'll get it below
	// we link elements into pairs, starting from the low-occurrence (weighted) ones, trying to minimize the pair occurrence
	bool atLeastOneSuperPairFound=true;
	while (atLeastOneSuperPairFound){
		atLeastOneSuperPairFound=false;
		for (int i=0; i<HUFFMAN_TABLE_LENGTH; i++){
			//finding low-occurrence candidate
			int lowestOccurrence=10000;
			int bestCandidate=-1;
			for (int i=0; i<HUFFMAN_TABLE_LENGTH;i++){
				HUFFMAN_record itemTemp=HUFFMAN_temporalTable[i];
				if (itemTemp.used && itemTemp.noOfOccurrences<=lowestOccurrence){
					bestCandidate=i;
					lowestOccurrence=itemTemp.noOfOccurrences;
				}
			}
			if (bestCandidate>=0){
				int lowestOccurrenceBrother=10000;
				int bestBrother=-1;
				for (int j=0; j<HUFFMAN_TABLE_LENGTH; j++){
					HUFFMAN_record itemBrother=HUFFMAN_temporalTable[j];
					if (j!=bestCandidate && itemBrother.used && itemBrother.noOfOccurrences<=lowestOccurrenceBrother){
						bestBrother=j;
						lowestOccurrenceBrother=itemBrother.noOfOccurrences;
					}
				}//searching all items, not only to the first check
				if (bestBrother>=0){
					//still at least two elements, merging
					int i1=moveToHuffmanTable(bestCandidate);
					int i2=moveToHuffmanTable(bestBrother);
					//int i3=
							addToTemporaryTable(i1, i2, HUFFMAN_table[i1].noOfOccurrences+HUFFMAN_table[i2].noOfOccurrences);
					//printf("Added temporary node (3) %d\n", i3);
					atLeastOneSuperPairFound=true;
				}
				else{
					//only one available element - root
					int i1=moveToHuffmanTable(bestCandidate);
					printf("Added root node (3) %d\n", i1);
					HUFFMAN_table[i1].parent=i1;//root points to itself
					HUFFMAN_root=i1;
					atLeastOneSuperPairFound=false;
					i=HUFFMAN_TABLE_LENGTH;
				}
			}
			else{
				//snh - eliminated while searching for a pair
				printf("Error: cannot find best candidate for root???\n");
			}
		}//for all temporary items
	}//while any temporary item to be linked into a pair

	//printing Huffman table
#ifdef PRINT_TABLE_FINAL
	for (int i=0; i<HUFFMAN_noOfAllItems; i++){
		HUFFMAN_record currentHUFFMANRecord=HUFFMAN_table[i];
		char rootYesNo='F';
		if (currentHUFFMANRecord.parent==i)
			rootYesNo='T';
		printf("Huffman final tree #%d: '%s', no=%d, left=%d, right=%d parent=%d root=%c\n",
			i,
			(char *)(currentHUFFMANRecord.stringAsString),
			(int)currentHUFFMANRecord.noOfOccurrences,
			(int)currentHUFFMANRecord.leftChild,
			(int)currentHUFFMANRecord.rightChild,
			(int)currentHUFFMANRecord.parent,
			rootYesNo);
	}
#endif

	printf("Huffman binary tree prepared, no of elements = %d\n", HUFFMAN_noOfAllItems);
}

void computeHuffmanLevels(){
	HUFFMAN_minLevelWeighted=1000.0;
	HUFFMAN_maxLevelWeighted=0.0;
	HUFFMAN_minLevelAbsolute=1000;
	HUFFMAN_maxLevelAbsolute=0;
	HUFFMAN_averageLevelAbsolute=0;
	double averageLevel=0.0;
	int noOfLeaves=0;
	for (int i=0; i<HUFFMAN_noOfAllItems; i++){
		HUFFMAN_record currentHUFFMANRecord=HUFFMAN_table[i];
		if (currentHUFFMANRecord.leftChild<0 && currentHUFFMANRecord.rightChild<0){
			//this is leaf node, may compute the level
			int level=getLengthToRoot(i);
			double realLevel=(double)level;
			int levelLength=0;
			if (currentHUFFMANRecord.stringAsString[0]!='\0')
				levelLength++;
			if (currentHUFFMANRecord.stringAsString[1]!='\0')
				levelLength++;
			if (currentHUFFMANRecord.stringAsString[2]!='\0')
				levelLength++;
			if (currentHUFFMANRecord.stringAsString[3]!='\0')
				levelLength++;
			if (levelLength>0){//should be always like this
				realLevel=realLevel/levelLength;
				averageLevel=averageLevel+realLevel;
				HUFFMAN_averageLevelAbsolute=HUFFMAN_averageLevelAbsolute+level;
				noOfLeaves++;
				if (realLevel<HUFFMAN_minLevelWeighted){
					HUFFMAN_minLevelWeighted=realLevel;
				}
				if (realLevel>HUFFMAN_maxLevelWeighted){
					HUFFMAN_maxLevelWeighted=realLevel;
				}
				if (level<HUFFMAN_minLevelAbsolute){
					HUFFMAN_minLevelAbsolute=level;
				}
				if (level>HUFFMAN_maxLevelAbsolute){
					HUFFMAN_maxLevelAbsolute=level;
				}
			}else{
				printf("ERROR: empty string detected in table: %d\n", i);
			}
		}
	}
	if (noOfLeaves>0){//must be like this here
		HUFFMAN_averageLevelWeighted=averageLevel/((double)noOfLeaves);
		HUFFMAN_averageLevelAbsolute=HUFFMAN_averageLevelAbsolute/noOfLeaves;
	}

	HUFFMAN_compressionFactor=(HUFFMAN_averageLevelWeighted)/8.0;

}

int findItemEncodedWithZeroesOnly(){
	bool reached=false;
	int currentNode=HUFFMAN_root;
	while (!reached){
		int leftChild=HUFFMAN_table[currentNode].leftChild;
		if (leftChild<0)
			return currentNode;
		currentNode=leftChild;
	}
	return -1; //snh
}
void exportHuffmanTable(char * fileName){
	//exporting Huffman data (table+description variables) to a C code, in the form:
	//#define HUFFMAN_TABLE_LENGTH ( XXX )
	//HUFFMAN_record HUFFMAN_table[2]={
	//		{1,2,3,4,"55",6,true},
	//		{7,8,9,0,"11",2,false}
	//};
	//typedef  struct{
	//	int leftChild;
	//	int rightChild;
	//	int parent;
	//	uint32_t stringAsInteger;
	//	uint8_t stringAsString[MAX_STRING_LENGTH+1]; //max 4 characters per string
	//	uint32_t noOfOccurrences;
	//	bool used;
	//} HUFFMAN_record;
	//int HUFFMAN_noOfAllItems=XXX;
	//int HUFFMAN_root=XXX;
	//double HUFFMAN_minLevelWeighted=XXX;
	//double HUFFMAN_maxLevelWeighted=XXX;
	//double HUFFMAN_averageLevelWeighted=XXX;
	//int HUFFMAN_minLevelAbsolute=XXX;
	//int HUFFMAN_maxLevelAbsolute=XXX;
	//int HUFFMAN_averageLevelAbsolute=XXX;
	//double HUFFMAN_compressionFactor=XXX;

	FILE *fp;
	if ((fp=fopen(fileName, "w"))==NULL) {
	     printf ("Nie mogę otworzyć pliku eksportu do zapisu!\n");
	     exit(1);
	}
	fprintf(fp, "//%s\n", version);
	fprintf(fp, "#define HUFFMAN_TABLE_LENGTH ( %d )\n", HUFFMAN_noOfAllItems);
	fprintf(fp, "#define MAX_STRING_LENGTH ( 4 )\n");
	fprintf(fp, "int HUFFMAN_noOfAllItems=%d;\n", HUFFMAN_noOfAllItems);
	fprintf(fp, "int HUFFMAN_root=%d;\n", HUFFMAN_root);
	fprintf(fp, "int HUFFMAN_minLevelAbsolute=%d;\n", HUFFMAN_minLevelAbsolute);
	fprintf(fp, "int HUFFMAN_maxLevelAbsolute=%d;\n", HUFFMAN_maxLevelAbsolute);
	fprintf(fp, "int HUFFMAN_averageLevelAbsolute=%d;\n", HUFFMAN_averageLevelAbsolute);
	fprintf(fp, "double HUFFMAN_minLevelWeighted=%f;\n", (float)HUFFMAN_minLevelWeighted);
	fprintf(fp, "double HUFFMAN_maxLevelWeighted=%f;\n", (float)HUFFMAN_maxLevelWeighted);
	fprintf(fp, "double HUFFMAN_averageLevelWeighted=%f;\n", (float)HUFFMAN_averageLevelWeighted);
	fprintf(fp, "double HUFFMAN_compressionFactor=%f;\n", (float)HUFFMAN_compressionFactor);
	fprintf(fp, "typedef  struct{\n");
	fprintf(fp, "	int leftChild;\n");
	fprintf(fp, "	int rightChild;\n");
	fprintf(fp, "	int parent;\n");
	fprintf(fp, "	uint32_t stringAsInteger;\n");
	fprintf(fp, "	uint8_t stringAsString[MAX_STRING_LENGTH+1]; \n");
	fprintf(fp, "	uint32_t noOfOccurrences;\n");
	fprintf(fp, "	bool used;\n");
	fprintf(fp, "} HUFFMAN_record;\n");
	fprintf(fp, "HUFFMAN_record HUFFMAN_table[]={\n");
	for (int i=0;i<HUFFMAN_noOfAllItems; i++){
		fprintf(fp, "{");
		fprintf(fp, "%d", HUFFMAN_table[i].leftChild);
		fprintf(fp, ", %d", HUFFMAN_table[i].rightChild);
		fprintf(fp, ", %d", HUFFMAN_table[i].parent);
		fprintf(fp, ", %d", (int)HUFFMAN_table[i].stringAsInteger);
		fprintf(fp, ", \"%s\"", (char *)HUFFMAN_table[i].stringAsString);
		fprintf(fp, ", %d", (int)HUFFMAN_table[i].noOfOccurrences);
		fprintf(fp, ", true}");
		if (i!=(HUFFMAN_noOfAllItems-1))
			fprintf(fp, ",");
		fprintf(fp, "		//item no %d", i);
		if (i==HUFFMAN_root)
			fprintf(fp, " (ROOT)");
		fprintf(fp, "\n");
	}
	fprintf(fp, "};\n");
	//storing all table elements as table declarations, one by one, but only for real-used ones
	fclose (fp);
}
int main(void) {
	printf("%s\n", version);

	printf("Init random generator\n");
	srand(time(NULL));   // Initialization, should only be called once
	//srand(1);//for testing only (comment the upper line if using this fixed way)

	prepareAlphabet();

	//prepare weighted table with substrings of the alphabet
	prepareLZW();

	//preparing Huffman binary tree
	prepareHUFFMAN();
	computeHuffmanLevels();

	printf("Huffman binary tree prepared, no of elements = %d, root = %d\n", HUFFMAN_noOfAllItems, HUFFMAN_root);
	printf("Absolute: min. level=%d, max. level=%d, av. level=%d\n", HUFFMAN_minLevelAbsolute, HUFFMAN_maxLevelAbsolute, HUFFMAN_averageLevelAbsolute);
	printf("Weighted: min. level=%.2f, max. level=%.2f, av. level=%.2f compression=%.0f%c\n",
			(float)HUFFMAN_minLevelWeighted,
			(float)HUFFMAN_maxLevelWeighted,
			(float)HUFFMAN_averageLevelWeighted,
			(float)HUFFMAN_compressionFactor*100.0,
			'%');

	int allZeroItem=findItemEncodedWithZeroesOnly();
	int allZeroItemLevel=getLengthToRoot(allZeroItem);
	uint8_t * allZeroString=HUFFMAN_table[allZeroItem].stringAsString;
	printf("All-zero element: %d '%s' of level %d and weight=%d\n", allZeroItem, allZeroString, allZeroItemLevel, (int)HUFFMAN_table[allZeroItem].noOfOccurrences);


	processSingleString((uint8_t *)"1234.5678");
	processSingleString((uint8_t *)"1234.5678");
	processSingleString((uint8_t *)"1234.5678");
	processSingleString((uint8_t *)"123.456");
	processSingleString((uint8_t *)"123.456");
	processSingleString((uint8_t *)"123.456");
	processSingleString((uint8_t *)"12.34");
	processSingleString((uint8_t *)"12.34");
	processSingleString((uint8_t *)"12.34");
	processSingleString((uint8_t *)"1.2");
	processSingleString((uint8_t *)"1.2");
	processSingleString((uint8_t *)"1.2");
	processSingleString((uint8_t *)"12");
	processSingleString((uint8_t *)"12");
	processSingleString((uint8_t *)"12");
	processSingleString((uint8_t *)"1");
	processSingleString((uint8_t *)"1");
	processSingleString((uint8_t *)"1");

	printf("\nProgram finished, thank you\n");

	char * fileName="export.txt";
	printf("\nExporting to file %s\n", fileName);
	exportHuffmanTable(fileName);
	printf("\nExport finished, thank you\n");

	return EXIT_SUCCESS;
}
