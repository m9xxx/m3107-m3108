#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

typedef struct {
	char data[128];	// 128 = 1024 // 8
} uint1024_t;

// implementation functions

int get(const uint1024_t *px, size_t n);
void set(uint1024_t *px, size_t n, int value);

// interface

uint1024_t from_uint(unsigned int x);
uint1024_t add_op(uint1024_t x, uint1024_t y);
uint1024_t sub_op(uint1024_t x, uint1024_t y);
uint1024_t mul_op(uint1024_t x, uint1024_t y);
void printf_value(uint1024_t x);
void scanf_value(uint1024_t* x);

void test_mul();
void test_sub();
void test_create();
void test_add();

int main(void) {

	FILE *empty = fopen("empty", "w");
	fclose(empty);

	freopen("in1.txt", "r", stdin);

	puts("!!!Hello World!!!");

	// test_create();
	// test_add();
	// test_sub();
	// test_mul();

	uint1024_t x;
	scanf_value(&x);


	// uint1024_t y = mul_op(x, from_uint(1));
	uint1024_t y = mul_op(x, from_uint(3));
	printf_value(y);


	return EXIT_SUCCESS;
}

uint1024_t from_uint(unsigned int x){
    uint1024_t res;

    for (size_t i = 0; i != 128; ++i)
    	res.data[i] = 0;

    size_t i = 0;
    while (x != 0) {
    	set(&res, i, x & 1);
    	x >>= 1; //shift a bit to the right
    	++i;
    }

    return res;
}
uint1024_t add_op(uint1024_t x, uint1024_t y){
    uint1024_t res;

    int carry = 0;

    for (int i = 0; i != 1024; ++i) {
    	int s = get(&x, i) + get(&y, i) + carry;
    	set(&res, i, s % 2);
    	carry = s / 2;
    }

    return res;
}
uint1024_t sub_op(uint1024_t x, uint1024_t y){
    uint1024_t res;

    // ~y (1's complement)
    for (int i = 0; i != 128; ++i) {
    	y.data[i] = ~y.data[i];
    }

    // 2's complement
    y = add_op(y, from_uint(1));

    res = add_op(x, y);

    return res;
}
uint1024_t mul_op(uint1024_t x, uint1024_t y){
    uint1024_t res = from_uint(0);

    for (int i = 0; i != 1024; ++i) {
    	if (get(&y, i))
    		res = add_op(x, res);
    	x = add_op(x, x);
    }

    return res;
}
void printf_value(uint1024_t x){
	for (int i = 1024 - 1; i >= 0; --i) {
		printf("%d", get(&x, i));
		if (0 && i % 128 == 0)
			puts("");
	}
	puts("");
}
void scanf_value(uint1024_t* x){
	char buf[500];		// 500 will be sufficient
	fgets(buf, 500, stdin);
	buf[strlen(buf) - 1] = 0;

	// 123

	uint1024_t tenten = from_uint(10);

	size_t n = strlen(buf);
	uint1024_t acc = from_uint(0);
	for (size_t i = 0; i != n; ++i) {
		acc = mul_op(acc, tenten);
		acc = add_op(acc, from_uint(buf[i] - '0'));
	}

	// printf("acc: %d\n", acc);
	// printf_value(acc);

	for (size_t i = 0; i != 128; ++i)
		x->data[i] = acc.data[i];

}


int get(const uint1024_t *px, size_t n) {
	int byte = n / 8;
	int bit = n % 8;

	return (px->data[byte] >> bit) & 1;
}

void set(uint1024_t *px, size_t n, int value) {
	int byte = n / 8;
	int bit = n % 8;

	if (value) {
		char mask = 1 << bit;
		px->data[byte] |= mask;
	} else {
		char mask = ~(1 << bit);
		px->data[byte] &= mask;
	}
}


void test_create() {
	puts("zero");
		uint1024_t zero = from_uint(0);
		printf_value(zero);

		puts("one");
		uint1024_t one = from_uint(1);
		printf_value(one);

		puts("nine");
		uint1024_t nine = from_uint(9);
		printf_value(nine);

		puts("nine x3");
		uint1024_t nine3 = from_uint(999);
		printf_value(nine3);

		puts("uintmax");
		uint1024_t uintmax = from_uint(UINT_MAX);
		printf_value(uintmax);

		puts("237985474");
		uint1024_t r1 = from_uint(237985474);
		printf_value(r1);


		printf("sizeof(unsigned) = %d\n", (int) sizeof(unsigned));

}


void test_add() {
		uint1024_t zero = from_uint(0);
		uint1024_t one = from_uint(1);
		uint1024_t nine = from_uint(9);
		uint1024_t nine3 = from_uint(999);
		uint1024_t uintmax = from_uint(UINT_MAX);
		uint1024_t r1 = from_uint(237985474);

		printf("0 + 1\n");
		printf_value(add_op(zero, one));



		printf("9 + 9\n");
		printf_value(add_op(nine, nine));
		printf("999 + 999\n");
		printf_value(add_op(nine3, nine3));
		printf("999 + r1\n");
		printf_value(add_op(nine3, r1));
		printf("uintmax + 1\n");
		printf_value(add_op(uintmax, one));

		uint1024_t x = one;
		for (int i = 0; i != 512; ++i)
			x = add_op(x, x); 		// x <= x + x = 2x
		printf("2^512\n");
		printf_value(x);

		x = one;		// 2^0
		for (int i = 0; i != 1023; ++i)
			x = add_op(x, x); 		// x <= x + x = 2x
		printf("2^1023\n");
		printf_value(x);
}


void test_sub() {
		uint1024_t zero = from_uint(0);
		uint1024_t one = from_uint(1);
		uint1024_t nine = from_uint(9);
		uint1024_t nine3 = from_uint(999);
		uint1024_t uintmax = from_uint(UINT_MAX);
		uint1024_t r1 = from_uint(237985474);

		printf("1 - 0\n");
		printf_value(sub_op(one, zero));
		printf("1 - 1\n");
		printf_value(sub_op(one, one));
		printf("999 - 9\n");
		printf_value(sub_op(nine3, nine));
		printf("r1 - 999\n");
		printf_value(sub_op(r1, nine3));
		printf("uintmax - r1\n");
		printf_value(sub_op(uintmax, r1));

		uint1024_t y = one;
		for (int i = 0; i != 512; ++i)
			y = add_op(y, y);

		uint1024_t x = one;		// 2^0
		for (int i = 0; i != 1023; ++i)
			x = add_op(x, x); 		// x <= x + x = 2x

		printf("2^1023 - 2^512\n");
		printf_value(sub_op(x, y));


		printf("!!! 0 - 1\n");
		printf_value(sub_op(zero, one));
}



void test_mul() {
		uint1024_t zero = from_uint(0);
		uint1024_t one = from_uint(1);
		uint1024_t nine = from_uint(9);
		uint1024_t nine3 = from_uint(999);
		uint1024_t uintmax = from_uint(UINT_MAX);
		uint1024_t r1 = from_uint(237985474);

		printf("0 * 1\n");
		printf_value(mul_op(zero, one));

		printf("9 * 9\n");
		printf_value(mul_op(nine, nine));
		printf("999 * 999\n");
		printf_value(mul_op(nine3, nine3));
		printf("999 * r1\n");
		printf_value(mul_op(nine3, r1));
		printf("uintmax * uintmax\n");
		printf_value(mul_op(uintmax, uintmax));
		printf("uintmax * r1\n");
		printf_value(mul_op(uintmax, r1));

		uint1024_t x = one;
		for (int i = 0; i != 512; ++i)
			x = add_op(x, x); 		// x <= x + x = 2x
		// printf("2^512\n");
		// printf_value(x);
}



