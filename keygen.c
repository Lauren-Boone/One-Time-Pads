#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

int main(int argc, char* argv[]) {

	if (argc == 2) {
		int length = atoi(argv[1]);
		int i, key, letter = 'A';

		srand(time(NULL));
		//create the random key
		for (i = 0; i < length; ++i) {
			key = rand() % 27;//random number add to ascci A value
			if (key == 26) {
				printf(" ");
			}
			else {
				key = letter + key;
				printf("%c", key);
			}

		}
		printf("\n");

		return 0;
	}
	else {
		exit(1);
	}
}
