#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
	FILE *empty = fopen("empty", "w");
	fclose(empty);

	// freopen("in1.txt", "r", stdin);


	FILE *in = fopen("access_log_Jul95", "r");
	//FILE *in = fopen("access_log_10000", "r");
	// FILE *in = fopen("access_log_15", "r");

	int maxn = 0;
	int max_line_length = 0;
	int ch;
	int cur_line_length = 0;
	while((ch = getc(in)) != EOF) {
		if (ch == '\n') {
			if (cur_line_length > max_line_length)
				max_line_length = cur_line_length;
			cur_line_length = 0;
			++maxn;
		}
		++cur_line_length;
	}

	if (cur_line_length > max_line_length)
		max_line_length = cur_line_length;
	++maxn;


	int error5xx = 0;

	printf("cur_line_length = %d\n", cur_line_length);
	rewind(in);

	char *buf = (char *) malloc(max_line_length + 1);
	while(fgets(buf, max_line_length + 1, in)) {
		// puts(buf);
		char *ptr = strchr(buf, '"');
		if (!ptr)
			continue;
		ptr = strchr(ptr + 1, '"');
		if (!ptr)
			continue;
		// puts(ptr);
		int status;
		sscanf(ptr + 1, "%d", &status);

		if (500 <= status && status <= 599)
			++error5xx;

	}



	fclose(in);

	printf("maxn = %d\n", maxn);
	printf("max_line_length = %d\n", max_line_length);
	printf("5xx errors: %d\n", error5xx);

	return EXIT_SUCCESS;
}
