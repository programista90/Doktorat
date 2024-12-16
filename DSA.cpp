#include<stdio.h>
#include<conio.h>
#include<math.h>
#include <iostream>
#include "Timer.h"

long int power(long int a, long int j, long int c)
{
	int f, i;
	f = 1;
	for (i = 1; i <= j; i++)
	{
		f = (f*a) % c;
	}
	f = f % c;
	return f;
}

long int ext_eucledian(long int m, long int b)
{
	int a1 = 1, a2 = 0, a3 = m, b1 = 0, b2 = 1, b3 = b, q, t1, t2, t3;
	while (1)
	{
		if (b3 == 0)
		{
			return 0;
		}
		if (b3 == 1)
		{
			if (b2 < 0)
				b2 += m;
			return b2;
		}
		q = a3 / b3;
		t1 = a1 - (q*b1);
		t2 = a2 - (q*b2);
		t3 = a3 - (q*b3);
		a1 = b1;
		a2 = b2;
		a3 = b3;
		b1 = t1;
		b2 = t2;
		b3 = t3;
	}
}



void main()
{
	Timer timer;
	timer.start();
	
	long int p, q, g, x, hm, k, y, r, s, s1, w, u1, u2, v, v1, v2, v3;
	char input[] = "a";

	char output[] = "";

	// podawanie parametrów
	/*
	printf("enter the value of p:");
	scanf("%ld", &p);
	printf("enter the value of q:");
	scanf("%ld", &q);
	printf("enter the value of g:");
	scanf("%ld", &g);
	printf("enter the value of x:");
	scanf("%ld", &x);
	printf("enter the value of hm:");
	scanf("%ld", &hm);
	printf("enter the value of k:");
	scanf("%ld", &k);
	*/

	p = 283;
	q = 47;
	g = 60;
	x = 19;
	hm = 41;
	k = 22;

	int iterationCount = 550000;

	for (int i = 0; i < iterationCount; i++)
	{
		y = power(g, x, p);
		r = power(g, k, p);
		r = r % q;
		s = ext_eucledian(q, k);
		s1 = (hm + (x*r));
		s = (s*s1) % q;
		w = ext_eucledian(q, s);
		u1 = (hm*w) % q;
		u2 = (r*w) % q;
		v = power(g, u1, p);
		v1 = power(y, u2, p);
		v2 = (v*v1) % p;
		v3 = v2 % q;
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
