#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include<time.h>

#include "hmac_sha256.h"

//#include <cassert.h>
//#include <iomanip.h>
//#include <iostream.h>
//#include <sstream.h>
//#include <tuple.h>
//#include <vector.h>

#define SHA256_HASH_SIZE 16
//128bits

//all strings assumed to be of max. length 255, does not matter real memory usage

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat="
#pragma GCC diagnostic ignored "-Wformat-extra-args"

char * version="HMAC (podział sekretu) tester v.1.02 20.11.2024 21:30";

typedef  struct{
    unsigned char key[255];
    unsigned char data[255];
    unsigned char hmac[255];
} test_record;

char KEY[255] = "1234567890123456"; //160bits
long VALIDATION_PERIOD_SECS = 60L;

//boja
typedef struct {
    double random;
    double fixed;        //Lagrange value computed by CA for X=random aX2+bX+c
    char uuid[255];        // UUID counterpart
    char major[255];        // MAJOR counterpart
    char minor[255];        // MINOR counterpart
    char VALUE_HMAC[255]; //20B to transmit, standard beacon uuid=16 + major=2 + minor=2 = 20B to transmit)
} beacon_data;
#define BEACON_SECRET_VALUE_X 2
char GROUP_ID[255] = "d2c8440c-490a-4393-9369-e827b20f631d";
char MINOR[255] = "11";
char MAJOR[255] = "22";//values just for the imagination of a real beacon

//klient
typedef struct{
    double random;
    double fixed;        //Lagrange value computed by CA for X=random aX2+bX+c
    char uuid[255];        // UUID counterpart
    char VALUE_HMAC[255];
} user_data;
#define USER_SECRET_VALUE_X 4
char USER_ID[255] = "9c6557a4-0d8d-44bc-b78c-882ac929eac7";

//3rd party, CA
typedef struct
{
    double a;
    double b;
    double c; //Lagrange parameters;
    double ownX;
    double ownY;
    char VALUE_HMAC1[255];    //from beacon
    char VALUE_HMAC2[255];     //from user
    char VALUE_HMAC3[255];  //from myself
} ca_data;
#define CA_SECRET_VALUE_X 5

//data to be sent from user to CA after detecting a beacon
typedef struct
{
    double beacon_random;
    char beacon_uuid[255];
    char beacon_major[255];
    char beacon_minor[255];
    char beacon_hmac[255];
    double user_random;
    char user_uuid[255];
    char user_hmac[255];
} tbs_data;

/*
test_record test0 = {
    "super-secret-key",
    "Hello World!",
    "4b393abced1c497f8048860ba1ede46a23f1ff5209b18e9c428bddfbb690aad8"
};

test_record test1 = {
    "\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b",
    "\x48\x69\x20\x54\x68\x65\x72\x65",
    "b0344c61d8db38535ca8afceaf0bf12b881dc200c9833da726e9376c2e32cff7"
};

// Test with a key shorter than the length of the HMAC output
test_record test2 = {
    "\x4a\x65\x66\x65",
    "\x77\x68\x61\x74\x20\x64\x6f\x20\x79\x61\x20\x77\x61\x6e\x74\x20\x66\x6f\x72\x20\x6e\x6f\x74\x68\x69\x6e\x67\x3f",
    "5bdcc146bf60754e6a042426089575c75a003f089d2739839dec58b964ec3843"
};

//Test with a combined length of key and data that is larger than 64 bytes (= block-size of SHA-224 and SHA-256)
test_record test3 = {
    "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa",
    "\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd\xdd",
    "773ea91e36800e46854db8ebd09181a72959098b3ef8c122d9635514ced565fe"
};

//Test with a combined length of key and data that is larger than 64
//bytes (= block-size of SHA-224 and SHA-256)
test_record test4 = {
    "\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19",
    "\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd\xcd",
    "82558a389a443c0ea4cc819899f2083a85f0faa3e578f8077a2e3ff46729665b"
};

//Test with a truncation of output to 128 bits
test_record test5 = {
    "\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c\x0c",
    "\x54\x65\x73\x74\x20\x57\x69\x74\x68\x20\x54\x72\x75\x6e\x63\x61\x74\x69\x6f\x6e",
    "a3b6167473100ee06e0c796c2955552b"
};

//Test with a key larger than 128 bytes (= block-size of SHA-384 and SHA-512)
test_record test6 = {
    "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa",
    "\x54\x65\x73\x74\x20\x55\x73\x69\x6e\x67\x20\x4c\x61\x72\x67\x65\x72"
        "\x20\x54\x68\x61\x6e\x20\x42\x6c\x6f\x63\x6b\x2d\x53\x69\x7a\x65\x20"
        "\x4b\x65\x79\x20\x2d\x20\x48\x61\x73\x68\x20\x4b\x65\x79\x20\x46\x69"
        "\x72\x73\x74",
    "60e431591ee0b67f0d8a26aacbf5b77f8e0bc6213728c5140546040f0ee37f54"
};

//Test with a key and data that is larger than 128 bytes (= block-size of SHA-384 and SHA-512)
test_record test7 = {
    "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa"
        "\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa\xaa",
    "\x54\x68\x69\x73\x20\x69\x73\x20\x61\x20\x74\x65\x73\x74\x20\x75\x73"
        "\x69\x6e\x67\x20\x61\x20\x6c\x61\x72\x67\x65\x72\x20\x74\x68\x61\x6e"
        "\x20\x62\x6c\x6f\x63\x6b\x2d\x73\x69\x7a\x65\x20\x6b\x65\x79\x20\x61"
        "\x6e\x64\x20\x61\x20\x6c\x61\x72\x67\x65\x72\x20\x74\x68\x61\x6e\x20"
        "\x62\x6c\x6f\x63\x6b\x2d\x73\x69\x7a\x65\x20\x64\x61\x74\x61\x2e\x20"
        "\x54\x68\x65\x20\x6b\x65\x79\x20\x6e\x65\x65\x64\x73\x20\x74\x6f\x20"
        "\x62\x65\x20\x68\x61\x73\x68\x65\x64\x20\x62\x65\x66\x6f\x72\x65\x20"
        "\x62\x65\x69\x6e\x67\x20\x75\x73\x65\x64\x20\x62\x79\x20\x74\x68\x65"
        "\x20\x48\x4d\x41\x43\x20\x61\x6c\x67\x6f\x72\x69\x74\x68\x6d\x2e",
    "9b09ffa71b942fcb27635fbcd5b0e944bfdc63644f0713938a7f51535c3a35e2"
};

*/
/*static void verify_result(unsigned char * pattern, unsigned char * result){
    if (strcmp((char *)result, (char *)pattern)!=0){
        printf("Test failed, result='%s', expected='%s'\n", (char *)result, (char *)pattern);
    }else{
        printf("Test succeeded for '%s'\n", (char *)result);
    }
}*/

static void convertCharToHEX(unsigned char c, unsigned char hexTable[3]){
    int cH=c/16;
    if (cH>=10){
        hexTable[0]=cH+'a'-10;
    }else{
        hexTable[0]=cH+'0';
    }
    int cL=c%16;
    if (cL>=10){
        hexTable[1]=cL+'a'-10;
    }else{
        hexTable[1]=cL+'0';
    }
    hexTable[3]='\0';
    //printf("char int=0x%x hex=0x%s\n", (int)c, hexTable);
}

static void convert_result_toChars(unsigned char * result, int len, unsigned char * conv){
    strcpy((char *)conv, (char *)"");
    for (int i=0; i<len; i++){
        unsigned char c=result[i];
        unsigned char hex[3];
        convertCharToHEX(c, hex);
        strcat((char *)conv, (char *)hex);
    }
    //printf("converted string='%s'\n", conv);
}

/*static void perform_test(test_record * testData){
    unsigned char out[255];
    hmac_sha256(
            testData->key, strlen((char *)testData->key),
            testData->data, strlen((char *)testData->data),
            out,  strlen((char *)testData->hmac)/2);
    unsigned char converted[255*3];
    convert_result_toChars(out, strlen((char *)testData->hmac)/2, converted);
    verify_result(testData->hmac, converted);
}

*/

/*static char * getHMACasString(char * key, int keyLen, char * data, int dataLen, char * outAsString, int outLen){
    unsigned char out[255];
    hmac_sha256(
            key, keyLen,
            data, dataLen,
            out, outLen);
    unsigned char converted[255*3];
    convert_result_toChars(out, outLen, converted);
    strcpy((char *)outAsString, (char *)converted);
    return outAsString;
}*/

static char * getHMACfromDoubles(char * key, int keyLen, double data1, double data2, char * outAsString, int outLen){
    unsigned char out[255];
    double params[2];
    params[0]=data1;
    params[1]=data2;
    hmac_sha256(
            key, keyLen,
            params, 16,
            out, outLen);
    unsigned char converted[255*3];
    convert_result_toChars(out, outLen, converted);
    strcpy((char *)outAsString, (char *)converted);
    return outAsString;
}

//określanie momentu czasowego
//timeMS klasycznie jako long-int
//kowersja na łańcuch znaków: YYYY-MM-DD HH:MM:SS

time_t rawtime;
struct tm * timeinfo;

static void initTime(){
    time(&rawtime);
    timeinfo = localtime(&rawtime);
}

static void getCurrentTime(char * out){
    sprintf(out, "%04d-%02d-%02d %02d:%02d:%02d",
            timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
            timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
}
//static time_t getRawCurrentTime(){
//    return rawtime;
//}
static char * getFutureTime(char * out, long secSinceNow){
    time_t rawtime1=time(NULL) + secSinceNow;
    struct tm * timeinfo1=localtime(&rawtime1);
    sprintf(out, "%04d-%02d-%02d %02d:%02d:%02d",
            timeinfo1->tm_year + 1900, timeinfo1->tm_mon + 1, timeinfo1->tm_mday,
            timeinfo1->tm_hour, timeinfo1->tm_min, timeinfo1->tm_sec);
    return out;
}

double getRandomValue4B(){
    int random=rand();
    return (double)random;
}

//Lagrange function
static void getLagrangeParameters(
        double x0, //double y_beacon,
        double x1, //double y_user,
        double x2, //double y_ca,
        //the above values are never pair-equal
        double* a,
        double* b,
        double* c){
    //a=1/( (x1-x0)(x1-x2) )
    //b=-(x0+x2)/( (x1-x0)(x1-x2) )
    //c=x0*x2/( (x1-x0)(x1-x2) )
    //first - common dividor
    double dividor=(x1-x0)*(x1-x2);
    *a=1.0/dividor;
    *b=-(x0+x2)/dividor;
    *c=(x0*x2)/dividor;
}

static void getLagrangeFunction(
        double x0, double y0,
        double x1, double y1,
        double x2, double y2,
        //the above values are never pair-equal
        double* a,
        double* b,
        double* c){
    double a0, b0, c0;
    getLagrangeParameters(x1, x0, x2, &a0, &b0, &c0);
    printf("Lagrange a0=%f b0=%f c0=%f\n", a0, b0, c0);
    double a1, b1, c1;
    getLagrangeParameters(x0, x1, x2, &a1, &b1, &c1);
    printf("Lagrange a1=%f b1=%f c1=%f\n", a1, b1, c1);
    double a2, b2, c2;
    getLagrangeParameters(x0, x2, x1, &a2, &b2, &c2);
    printf("Lagrange a2=%f b2=%f c2=%f\n", a2, b2, c2);
    *a=y0*a0+y1*a1+y2*a2;
    *b=y0*b0+y1*b1+y2*b2;
    *c=y0*c0+y1*c1+y2*c2;
}

double getLagrangeValue(double a, double b, double c, double x){
    return (a*x*x + b*x + c);
}

//beacon operations
void prepare_CA(ca_data * ca, double a, double b, double c){
    ca->a=a;
    ca->b=b;
    ca->c=c;
    ca->ownX=CA_SECRET_VALUE_X;
    ca->ownY=getLagrangeValue(a, b, c, CA_SECRET_VALUE_X);
    getHMACfromDoubles(KEY, strlen(KEY), 0.0, ca->ownY, ca->VALUE_HMAC3, SHA256_HASH_SIZE);//just for fun
}

void prepare_beacon(beacon_data * beacon,  double fixed){
    beacon->fixed=fixed;
    strcpy(beacon->uuid, GROUP_ID);
    strcpy(beacon->minor, MINOR);
    strcpy(beacon->major, MAJOR);
}

void prepare_user(user_data * user, double fixed){
    user->fixed=fixed;
    strcpy(user->uuid, USER_ID);
}

int main(void) {
    printf("%s\n", version);
    long long int a1=0;
    printf("sizeof long long int is %d B\n", sizeof(a1));
    unsigned char a2=0;
    printf("sizeof u-char is %d B\n", sizeof(a2));
    float a3=0.1;
    printf("sizeof float is %d B\n", sizeof(a3));             //4B
    double a4=0.1;
    printf("sizeof double is %d B\n", sizeof(a4));            //8B
    long double a5=0.1;
    printf("sizeof long-double is %d B\n", sizeof(a5));        //12B

    initTime();
    char timeBuffer[255];
    getCurrentTime(timeBuffer);
    printf("Current time is '%s'\n", timeBuffer);
    getFutureTime(timeBuffer, VALIDATION_PERIOD_SECS);
    printf("Validation time is '%s'\n\n", timeBuffer);

    /*perform_test(&test0);
    perform_test(&test1);
    perform_test(&test2);
    perform_test(&test3);
    perform_test(&test4);
    perform_test(&test5);
    perform_test(&test6);
    perform_test(&test7);

    char result[255];

    char * key="super-secret-key";
    char * data="Hello World!";
    char * hmac="4b393abced1c497f8048860ba1ede46a23f1ff5209b18e9c428bddfbb690aad8";

    getHMACasString(key, strlen(key), data, strlen(data), result, 32);
    printf("\nTest key='%s' data='%s' result='%s'\n", key, data, result);
    verify_result((unsigned char *)hmac, (unsigned char *)result);
    */

    double a=94, b=166, c=1234;

    //install CA
    ca_data ca;
    prepare_CA(&ca, a, b, c);
    printf("CA:\n  a=%f b=%f c=%f x=%f y=%f\n  HMAC(y+0.0)='%s'\n\n",
            ca.a,
            ca.b,
            ca.c,
            ca.ownX,
            ca.ownY,
            ca.VALUE_HMAC3
            );

    //install beacon
    beacon_data beacon;
    prepare_beacon(&beacon,  getLagrangeValue(a, b, c, BEACON_SECRET_VALUE_X));
    beacon.random=getRandomValue4B();
    getHMACfromDoubles(
            KEY, strlen(KEY),
            beacon.random, beacon.fixed,
            beacon.VALUE_HMAC, SHA256_HASH_SIZE);//this value is to be broadcasted

    printf("BEACON:\n  UUID='%s' MAJOR='%s' MINOR='%s'\n  random=%f fixed=%f HMAC='%s'\n\n",
            beacon.uuid,
            beacon.major,
            beacon.minor,
            beacon.random,
            beacon.fixed,
            beacon.VALUE_HMAC);

    //install user

    user_data user;
    prepare_user(&user, getLagrangeValue(a, b, c, USER_SECRET_VALUE_X));
    user.random=getRandomValue4B();
    getHMACfromDoubles(KEY, strlen(KEY), user.random, user.fixed, user.VALUE_HMAC, SHA256_HASH_SIZE);//this value is to be presented to CA

    printf("USER:\n  UUID='%s' \n  random=%f fixed=%f HMAC='%s'\n\n",
            user.uuid,
            user.random,
            user.fixed,
            user.VALUE_HMAC);

    //get data from user based on info just got from near-by beacon

    tbs_data tbs;
    strcpy(tbs.beacon_uuid, beacon.uuid);
    strcpy(tbs.beacon_major, beacon.major);
    strcpy(tbs.beacon_minor, beacon.minor);
    strcpy(tbs.beacon_hmac, beacon.VALUE_HMAC);
    strcpy(tbs.user_uuid, user.uuid);
    strcpy(tbs.user_hmac, user.VALUE_HMAC);
    tbs.beacon_random=beacon.random;
    tbs.user_random=user.random;

    //CA test after receiving data
    getHMACfromDoubles(KEY, strlen(KEY), beacon.random, getLagrangeValue(a, b, c, BEACON_SECRET_VALUE_X), ca.VALUE_HMAC1, SHA256_HASH_SIZE);
    getHMACfromDoubles(KEY, strlen(KEY), user.random, getLagrangeValue(a, b, c, USER_SECRET_VALUE_X), ca.VALUE_HMAC2, SHA256_HASH_SIZE);

    //beacon authorization - HMAC values must match
    if (strcmp(ca.VALUE_HMAC1, tbs.beacon_hmac) != 0){
        //Beacon not authorized
        printf("Beacon not authorized:\n  UUID='%s',\n  HMAC received='%s',\n  HMAC computed='%s'\n\n",
                tbs.beacon_uuid, tbs.beacon_hmac, ca.VALUE_HMAC1);
    }else{
        printf("Beacon authorized:\n  UUID='%s',\n  HMAC received='%s',\n  HMAC computed='%s'\n\n",
                tbs.beacon_uuid, tbs.beacon_hmac, ca.VALUE_HMAC1);
    }

    //user authorization - HMAC values must match
    if (strcmp(ca.VALUE_HMAC2, tbs.user_hmac) != 0){
        //User not authorized
        printf("User not authorized:\n  UUID='%s',\n  HMAC received='%s',\n  HMAC computed='%s'\n\n",
                tbs.user_uuid, tbs.user_hmac, ca.VALUE_HMAC2);
    }else{
        printf("User authorized:\n  UUID='%s',\n  HMAC received='%s',\n  HMAC computed='%s'\n\n",
                tbs.user_uuid, tbs.user_hmac, ca.VALUE_HMAC2);
    }

    //check rights of the user to the beacon - secrets must match

    double a_computed, b_computed, c_computed;
    double x0=BEACON_SECRET_VALUE_X;     double y0=getLagrangeValue(a, b, c, x0);
    double x1=USER_SECRET_VALUE_X;         double y1=getLagrangeValue(a, b, c, x1);
    double x2=CA_SECRET_VALUE_X;         double y2=getLagrangeValue(a, b, c, x2);

    getLagrangeFunction(x0, y0, x1, y1, x2, y2, &a_computed, &b_computed, &c_computed);
    printf("Lagrange parameters as assumed  a=%f b=%f c=%f\n\n", a, b, c);
    printf("Lagrange parameters as computed a=%f b=%f c=%f\n\n", a_computed, b_computed, c_computed);

    double error_a=a-a_computed;
    double error_b=b-b_computed;
    double error_c=c-c_computed;
    if (error_a<0) error_a=-error_a;
    if (error_b<0) error_b=-error_b;
    if (error_c<0) error_c=-error_c;
    double tolerance=0.0000001;
    printf("Error a=%f\n", error_a);
    printf("Error b=%f\n", error_b);
    printf("Error c=%f\n", error_c);

    if (error_a>tolerance){
        printf("User and beacon do not match, a=%f != a_computed=%f\n", a, a_computed);
    }else{
        printf("User and beacon match, a=%f == a_computed=%f\n", a, a_computed);
    }
    if (error_b>tolerance){
        printf("User and beacon do not match, b=%f != b_computed=%f\n", b, b_computed);
    }else{
        printf("User and beacon match, b=%f == b_computed=%f\n", b, b_computed);
    }
    if (error_c>tolerance){
        printf("User and beacon do not match, c=%f != c_computed=%f\n", c, c_computed);
    }else{
        printf("User and beacon match, c=%f == c_computed=%f\n", c, c_computed);
    }

    return EXIT_SUCCESS;
}

#pragma GCC diagnostic pop