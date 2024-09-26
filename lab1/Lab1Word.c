#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <ctype.h>

// getopt usage taken from
// Mark Mitchell "Advanced Linux Programming"

const char* program_name;

void print_usage(FILE* stream, int exit_code) {
    fprintf(stream, "Usage: %s [OPTION] [filename]\n", program_name); //How to use
    fprintf(stream, "\t-l, --lines\tprint # of lines\n"); //Functions
    fprintf(stream, "\t-c, --bytes\tprint # of bytes\n");
    fprintf(stream, "\t-w, --words\tprint # of words\n");
    exit(exit_code);
}

int main(int argc, char *argv[]) {
    int next_option;

    char short_options[] = "lcw"; //String of every possible options

    const struct option long_options[] = {  //structure describes a single long option name for the sake of getopt_long
        {            "lines", 0, NULL, 'l'        }, //lines - long option name, 0 - not taking any argument, NULL - pointer on nothing, l - short option name
        {            "bytes", 0, NULL, 'c'        },
        {            "words", 0, NULL, 'w'        },
        {            NULL, 0, NULL, 0        }
    };

    const char* input_filename = NULL;
    int l = 0, c = 0, w = 0; // counter of options

    program_name = argv[0]; //firstly write program.exe in console\powershell etc

    do {
        next_option = getopt_long(argc, argv, short_options, long_options, NULL); //long option could be anything we mentioned earlier

        switch (next_option) {
        case 'l':
            l = 1;
            break;
        case 'c':
            c = 1;
            break;
        case 'w':
            w = 1;
            break;
        case '?':
            print_usage(stderr, 1);
        case -1:
            break;
        default:
            abort();
        }

    } while (next_option != -1); //checking case if user typed invalid options

    if (!l && !c && !w)
        l = c = w = 1;

    if (optind != argc) {
        input_filename = argv[optind];
    }

    FILE *in = stdin;
    if (input_filename != NULL) { //checking case if file doesn't exist
        in = fopen(input_filename, "r");
        if (!in) {
            fprintf(stderr, "cannot open file %s\n", input_filename);
            abort();
        }
    }

    int lc = 0, cc = 0, wc = 0; // lines\characters\words counter

    int is_word = 0;

    int ch;
    while ((ch = getc(in)) != EOF) { //while we getting any characters
        // printf("[%c]\n", ch);
        if (ch == '\n')  //if \n we adding new line and character
            ++lc;
        ++cc;

        if (!is_word && !isspace(ch)) {
            is_word = 1;
            ++wc;
        } else if (is_word && isspace(ch)) {
            is_word = 0;
        }

    }

    if (l)
        printf("\t%d\n\tAlso, don't press <enter> after your final line, it will cause miscalculation\n", ++lc); //if user will press enter after last word, it will create empty line
    if (w)
        printf("\t%d", wc);
    if (c)
        printf("\t%d", cc);
    puts("");

    return 0;
}
