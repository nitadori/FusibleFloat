#ifdef __clang__
    #pragma clang fp contract(off)
#elif defined(__GNUC__)
    #pragma GCC optimize ("no-fast-math")
#endif

#include <cstdio>
#include <cstdlib>
// #define FUSIBLE_FLOAT_FUSE_LVALUE
// #define FUSIBLE_FLOAT_ROUND_LEFT_PRODUCT
// #define FUSIBLE_FLOAT_NO_FUSE_RVALUE
#include "FusibleFloat.hpp"

extern float poly3(float x, float a, float b, float c){
	Ffloat xf = x;
	return a + xf * (b + xf * +c); // +c makes it rvalue
}

int main(int ac, char **av){
	int ai = ac > 1 ? atoi(av[1]) : 3;
	int bi = ac > 2 ? atoi(av[2]) : 7;

	float a = ai / 10.0f;
	float b = bi / 10.0f;
	float c = a * b;
	Ffloat fa = a;
	Ffloat fb = b;
	// Ffloat fc = c;

	puts("test1");
	printf("%e\n", a  * b  - c ); // not fused
	printf("%e\n", fa * fb - c ); // fused
	printf("%e\n", fa * b  - c ); // not fusd by default
	printf("%e\n", fa * +b - c ); // fused if it takes rvalue
	printf("%e\n", a  * fb - c ); // not fusd by default
	printf("%e\n", +a * fb - c ); // fused if it takes rvalue

	puts("test2");
	printf("%e\n", a*b   - fa*fb );
	printf("%e\n", fa*fb - a*b );
#ifdef FUSIBLE_FLOAT_ROUND_LEFT_PRODUCT
	printf("%e\n", fa*fb - fa*fb ); // ambibuity error
#endif
	printf("%e\n", +(fa*fb) - fa*fb );
	printf("%e\n", 0.0f + fa*fb - fa*fb );
}

