#define _XOPEN_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(void) {
	FILE *empty = fopen("empty", "w");
	fclose(empty);

	// freopen("in1.txt", "r", stdin);
	// freopen("out.txt", "w", stdout);

	// window in seconds
	int w = 15;

	// FILE *in = fopen("access_log_Jul95", "r");
	FILE *in = fopen("access_log_10000", "r");
	// FILE *in = fopen("access_log_15", "r");

	int maxn = 0;
	int max_line_length = 0;
	int ch;
	int cur_line_length = 0;

	int nlines = 0;
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

	int *timestamps;
	timestamps = (int *) malloc (sizeof(int) * maxn);

	int n = 0;
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

		// timestamp

		ptr = strchr(buf, '[');
		if (!ptr)
			continue;

		int day;
		int h, m, s;
		sscanf(ptr + 1, "%d", &day);

		ptr = strchr(ptr + 1, ':');
		if (!ptr)
			continue;

		sscanf(ptr + 1, "%d", &h);

		ptr = strchr(ptr + 1, ':');
		if (!ptr)
			continue;

		sscanf(ptr + 1, "%d", &m);

		ptr = strchr(ptr + 1, ':');
		if (!ptr)
			continue;

		sscanf(ptr + 1, "%d", &s);


		int ts = (((((day-1) * 24) + h) * 60) + m) * 60 + s;

		// printf("%d, %d:%d:%d = %d\n", day, h, m, s, ts);
		// printf("[%s]\n", ptr);

		timestamps[n++] = ts;
	}

	printf("n = %d\n", n);
	fclose(in);

	for (int i = 0; i != n; ++i) {
		// printf("%d: %d\n", i, timestamps[i]);
	}

	// max window search

	// [0, 0) = empty interval
	int candidate_window_i = 0, candidate_window_j = 0;

	int i = 0;
	int j = 0;

	while (i != n) {
		// window = [i, j)
		while (j != n && timestamps[j] - timestamps[i] < w) {
			++j;
		}

		// printf("%d: [%d, %d)\n", timestamps[i], i, j);

		if (j - i > candidate_window_j - candidate_window_i) {
			candidate_window_i = i;
			candidate_window_j = j;
			if (0)
				printf("\tcandidate: [%d, %d)\n",
						candidate_window_i,
						candidate_window_j);
		}
		++i;
	}

	printf("maxn = %d\n", maxn);
	printf("max_line_length = %d\n", max_line_length);
	printf("5xx errors: %d\n", error5xx);

	printf("w: [%d, %d)\n", candidate_window_i, candidate_window_j);

	// int ts = (((((day-1) * 24) + h) * 60) + m) * 60 + s;
	int day, h, m, s;
	int x = timestamps[candidate_window_i];

	s = x % 60;
	x /= 60;
	m = x % 60;
	x /= 60;
	h = x % 24;
	x /= 24;
	day = x + 1;

	printf("%02d/Jul/1995 %02d:%02d:%02d", day, h, m, s);

	// timestamp of the last query IN the window
	x = timestamps[candidate_window_j - 1];

	s = x % 60;
	x /= 60;
	m = x % 60;
	x /= 60;
	h = x % 24;
	x /= 24;
	day = x + 1;

	printf("\t%02d/Jul/1995 %02d:%02d:%02d\n", day, h, m, s);



	printf("length of w: %d\n",
			candidate_window_j - candidate_window_i);

	return EXIT_SUCCESS;
}
