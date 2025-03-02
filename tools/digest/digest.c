#include <stdio.h>
#include <string.h>

unsigned int lrotl(unsigned int value, int shift) {
    return (value << shift) | (value >> (sizeof(unsigned int) * 8 - shift));
}

int main(int argc, char **argv) {
    if (argc != 2) {
	printf("syntax : digest string\n");
	return(1);
    }
    int l = strlen(argv[1]);
    int crc = 0;
    int n;
    for (n=0; n<l/4; n++)
	crc = lrotl(crc,1) + *((int*)&argv[1][n*4]);
    int left = l-n*4;
    if (left) {
	unsigned int buff = 0;
	memcpy(&buff,&argv[1][n*4],left);
	crc = lrotl(crc,1) + buff;
    }
    printf("%x\n",crc);

    return 0;
}

