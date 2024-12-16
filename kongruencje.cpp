#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat="
#pragma GCC diagnostic ignored "-Wformat-extra-args"

char * version="Euklides tester v.1.02 07.10.2024 14:30";

long long int ex, ey;
void poszerzony_euklides(long long int a, long long int b)
{
    //printf("Euklides INPUT a=%lld b=%lld\n", a, b);
    if(b!=0){
        poszerzony_euklides(b, a%b);
        long long int pom = ey;
        ey = ex  - a/b*ey;
        ex = pom;
        //printf("Euklides OUTPUT x=%lld y=%lld\n", ex, ey);
    };
}

long long int NWD(long long int a, long long int b)
{
    long long int  pom;

    while(b!=0)
    {
        pom = b;
        b = a%b;
        a = pom;
    }

    return a;
}

//test1
long long int a1=35;
long long int a2=108;
long long int a3=374;
long long int x1=10816;
long long int x2=4815;
long long int x3=823;
long long int expectedResult=3320544483LL;

/*
//test2
long long int a1=35;
long long int a2=108;
long long int a3=374;
long long int x1=623;
long long int x2=815;
long long int x3=823;
long long int expectedResult= 412984243LL;
*/
/*
//test3
long long int a1=35;
long long int a2=32;
long long int a3=374;
long long int x1=623;
long long int x2=428;
long long int x3=823;
long long int expectedResult=38179344LL;
*/
/*
//test4
long long int a1=11;
long long int a2=23;
long long int a3=42;
long long int x1=43;
long long int x2=428;
long long int x3=823;
long long int expectedResult=5486983LL;
*/
/*
//test5
long long int a1=11;
long long int a2=23;
long long int a3=21;
long long int x1=43;
long long int x2=56;
long long int x3=99;
long long int expectedResult=185943LL;
*/

//poniższe zmienne mają wyliczane wartości
long long int nwd1=0;
long long int nwd2=0;
long long int nwd3=0;
long long int n=0;
long long int c1=0;
long long int c2=0;
long long int c3=0;
long long int yy1=1;
long long int yy2=2;
long long int yy3=3;
long long int xxx=0;
long long int xModN=0;

int main(void) {
    printf("%s\n", version);
    printf("sizeof long long int is %d B\n", sizeof(a1));
    //x = 1;
    //y = 0;
    //long long int a, b;
    //a=3962745LL;
    //b=10816LL;
    //a=8901568LL;
    //b=4815LL;
    //a=52079040LL;
    //b=823LL;
    printf("x1=%lld x2=%lld x3=%lld\n", x1, x2, x3);
    printf("a1=%lld a2=%lld a3=%lld\n", a1, a2, a3);
    nwd1=NWD(x1, x2);
    nwd2=NWD(x1, x3);
    nwd3=NWD(x2, x3);
    if (nwd1==1){
        printf("X1 i X2 względnie pierwsze, OK.\n");
    }else{
        printf("Błąd: X1 i X2 nie są względnie pierwsze, koniec algorytmu.\n");
        return 1;
    }
    if (nwd2==1){
        printf("X1 i X3 względnie pierwsze, OK.\n");
    }else{
        printf("Błąd: X1 i X3 nie są względnie pierwsze, koniec algorytmu.\n");
        return 1;
    }
    if (nwd3==1){
        printf("X2 i X3 względnie pierwsze, OK.\n");
    }else{
        printf("Błąd: X2 i X3 nie są względnie pierwsze, koniec algorytmu.\n");
        return 1;
    }
    n=x1*x2*x3;
    printf("N=%lld\n", n);
    c1=n/x1;
    c2=n/x2;
    c3=n/x3;
    printf("C1=%lld C2=%lld C3=%lld\n", c1, c2, c3);
    ex=1;ey=0;poszerzony_euklides(c1,x1);yy1=ex;
    ex=1;ey=0;poszerzony_euklides(c2,x2);yy2=ex;
    ex=1;ey=0;poszerzony_euklides(c3,x3);yy3=ex;
    printf("Y1=%lld Y2=%lld Y3=%lld\n", yy1, yy2, yy3);
    xxx=a1*c1*yy1+a2*c2*yy2+a3*c3*yy3;
    printf("x=%lld\n", xxx);
    xModN=xxx%n;
    if (xModN<0){
        printf("Rezultat ujemny (%lld), konwersja na dodatni\n", xModN);
        while (xModN<0){
            xModN=xModN+n;
        }
    }
    printf("x mod N=%lld\n", xModN);
    if (expectedResult==xModN){
        printf("Rezultat poprawny\n");
    }else{
        printf("Rezultat niepoprawny, wyszło %lld, spodziewano się %lld\n", xModN, expectedResult);
    }

    if (xModN%x1==a1){
        printf("Układ a1-x1 poprawny\n");
    }else{
        printf("Układ a1-x1 niepoprawny, wyszło %lld, spodziewano się %lld\n", xModN%x1, a1);
    }
    if (xModN%x2==a2){
        printf("Układ a2-x2 poprawny\n");
    }else{
        printf("Układ a2-x2 niepoprawny, wyszło %lld, spodziewano się %lld\n", xModN%x2, a2);
    }
    if (xModN%x3==a3){
        printf("Układ a3-x3 poprawny\n");
    }else{
        printf("Układ a3-x3 niepoprawny, wyszło %lld, spodziewano się %lld\n", xModN%x3, a3);
    }
    if (xModN%x1!=a3){
        printf("Układ a3-x1 niepoprawny jak oczekiwano\n");
    }else{
        printf("Układ a3-x1 nieoczekiwanie poprawny\n");
    }
    if (xModN%x2!=a1){
        printf("Układ a1-x2 niepoprawny jak oczekiwano\n");
    }else{
        printf("Układ a1-x2 nieoczekiwanie poprawny\n");
    }

/*
    char * fileName="export.txt";
    printf("\nExporting to file %s\n", fileName);
    exportHuffmanTable(fileName);
    printf("\nProgram finished, thank you\n");
*/
    return EXIT_SUCCESS;
}

#pragma GCC diagnostic pop