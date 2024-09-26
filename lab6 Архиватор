#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <ctype.h>

#define SIZE (1024*1024) //so we are not going to go far from our standart size

// getopt usage taken from
// Mark Mitchell "Advanced Linux Programming"

const char* program_name; //no explanations needed

struct header_record {
    long int header_sz; // title
    long int sz; //inner information
};
long int get_size(char filename[]); //name of file
void extract(const char* arc_filename); //name of file in archive
void list(const char* arc_filename); // same thing

void create(const char* arc_filename, //info in archive
            char *files[], //pointer to files, headers
            int n);


void print_usage(FILE* stream, int exit_code) { //literally instructions
    fprintf(stream, "Usage: %s --file FILE --create|--extract||--list FILENAMES\n", program_name);
    fprintf(stream, "\t--file FILE\tarc file name\n");
    fprintf(stream, "\t--create\tcreate a new archive file\n");
    fprintf(stream, "\t--extract\textracts\n");
    fprintf(stream, "\t--list\tlist files in an archive file\n");
    exit(exit_code);
}

int main(int argc, char *argv[]) {
    int next_option;

    char short_options[] = ""; //described in long_options

    const struct option long_options[] = {  //our useful functions
        {            "file", 1, NULL, 'f'        }, //1 because file takes file name as an argument
        {            "create", 0, NULL, 'c'        },
        {            "extract", 0, NULL, 'e'        },
        {            "list", 0, NULL, 'l'        },
        {            NULL, 0, NULL, 0        }
    };

    const char* arc_filename = NULL; //temporarely making archive empty, user will name it later
    int c = 0, e = 0, l = 0; //before user will give it

    program_name = argv[0]; //first argument - main.exe

    do {
        next_option = getopt_long(argc, argv, short_options, long_options, NULL); //getting an option

        switch (next_option) { //calculating every outcome depending on function
        case 'f':
            arc_filename = optarg;
            break;
        case 'c':
            c = 1;
            break;
        case 'e':
            e = 1;
            break;
        case 'l':
            l = 1;
            break;
        case '?':
            print_usage(stderr, 1);
        case -1:
            break;
        default:
            abort();
        }

    } while (next_option != -1);

    if (!arc_filename) { //potential outcome when user skipped archive's name
        fprintf(stderr, "arc_filename was not provided\n");
        print_usage(stderr, 1);
    }

    if (c + e + l != 1) {
        fprintf(stderr, "only one of the options (create/extract/list)");
        fprintf(stderr, " must be provided\n");
        print_usage(stderr, 1);
    }

    if (c) {
        // create
        create(arc_filename, argv + optind, argc - optind);
    } else if (e) {
        // extract
        extract(arc_filename);
    } else {
        // list
        list(arc_filename);
    }

    return 0;
}


void create(const char* arc_filename,
            char *files[],
            int n) {
    FILE *af = fopen(arc_filename, "wb"); //archive's file name (af)
    if (!af) {
        fprintf(stderr, "cannot create the archive file: %s",
                arc_filename);
        exit(1);
    }

    struct header_record *hs = (struct header_record *) //size of header, giving it a little bit of memory
                               malloc(sizeof(struct header_record) * n);

    if (!hs) { //no memory for header which is very sad
        fprintf(stderr, "cannot allocate memory for hs\n");
        exit(1);
    }

    for (int i = 0; i != n; ++i) { //getting size of files that going to archive
        hs[i].header_sz = strlen(files[i]); //file name's lenght
        hs[i].sz = get_size(files[i]);
    }

    for (int i = 0; i != n; ++i) {
        printf("%d. %s: ", i, files[i]); // number of file, its name
        printf("%ld %ld\n", hs[i].header_sz, hs[i].sz); //size of file's name, file's size itself
    }

    fwrite((void *) &n, sizeof(int), 1, af); //number of bytes coming from file
    fwrite((void *) hs, sizeof(struct header_record), n, af); //*straight in the archive's header

    for (int i = 0; i != n; ++i) {
        FILE *f = fopen(files[i], "rb");
        if (!f) {
            fprintf(stderr, "cannot create the input file: %s", files[i]);
            exit(1);
        }

        fwrite(files[i], hs[i].header_sz, 1, af); //copying file's info into archive

        // https://stackoverflow.com/questions/14150431/copying-a-file-in-c-with-fwrite?rq=1

        char buffer[SIZE]; //declaring buffer with defined size
        size_t bytes;

        while (0 < (bytes = fread(buffer, 1, sizeof(buffer), f))) //reading buffer, writing it in, repeat until we copied a whole file
            fwrite(buffer, 1, bytes, af);

        fclose(f);
    }

    free(hs);
    fclose(af); //put all files in arc

}

void extract(const char* arc_filename) {
     FILE *af = fopen(arc_filename, "rb");
    if (!af) {
        fprintf(stderr, "cannot open the archive file: %s",
                arc_filename);
        exit(1);
    }

    int n;
    size_t res;
    res = fread((void *) &n, sizeof(int), 1, af);
    if (res != 1) {
            fprintf(stderr, "file corrupted: n, %ld bytes read\n", res);
            exit(1);
        }

    struct header_record *hs = (struct header_record *)
                               malloc(sizeof(struct header_record) * n);

    if (!hs) {
        fprintf(stderr, "cannot allocate memory for hs\n");
        exit(1);
    }

    if (fread((void *) hs, sizeof(struct header_record), n, af) !=
        n) {
            fprintf(stderr, "file corrupted\n");
            exit(1);
        }

    size_t offset = sizeof(int) * 1 +
        sizeof(struct header_record) * n;

    for (int i = 0; i != n; ++i) {
        char *buf = malloc(hs[i].header_sz + 1);

        if (fread((void *) buf, 1, hs[i].header_sz, af) !=
            hs[i].header_sz) {
            fprintf(stderr, "file corrupted\n");
            exit(1);
        }
        buf[hs[i].header_sz] = 0;
        printf("%s (%ld bytes)\n", buf, hs[i].sz);

        FILE *f = fopen(buf, "wb");
        if (!f) {
            fprintf(stderr, "cannot create the input file: %s",
                    buf);
            exit(1);
        }

        char buffer[SIZE];

        while (0 < (res = fread(buffer, 1, hs[i].sz, af))) {
            fwrite(buffer, 1, res, f);
            hs[i].sz -= res;
        }

        fclose(f);
        free(buf);
    }

    free(hs);
    fclose(af);

}

void list(const char* arc_filename) {
    FILE *af = fopen(arc_filename, "rb");
    if (!af) {
        fprintf(stderr, "cannot open the archive file: %s",
                arc_filename);
        exit(1);
    }

    int n;
    size_t res;
    res = fread((void *) &n, sizeof(int), 1, af);
    if (res != 1) {
            fprintf(stderr, "file corrupted: n, %ld bytes read\n", res);
            exit(1);
        }

    struct header_record *hs = (struct header_record *)
                               malloc(sizeof(struct header_record) * n);

    if (!hs) {
        fprintf(stderr, "cannot allocate memory for hs\n");
        exit(1);
    }

    if (fread((void *) hs, sizeof(struct header_record), n, af) !=
        n) {
            fprintf(stderr, "file corrupted\n");
            exit(1);
        }

    size_t offset = sizeof(int) * 1 +
        sizeof(struct header_record) * n;  //file's offset from collecting binary info and info from header

    for (int i = 0; i != n; ++i) {
        char *buf = malloc(hs[i].header_sz + 1);

        if (fread((void *) buf, 1, hs[i].header_sz, af) !=
            hs[i].header_sz) {
            fprintf(stderr, "file corrupted\n");
            exit(1);
        }
        buf[hs[i].header_sz] = 0;
        printf("%s (%ld bytes)\n", buf, hs[i].sz); //file's name and bytes
        fseek(af, hs[i].sz, SEEK_CUR);
        free(buf);
    }

    free(hs);
    fclose(af);

}


// https://www.cplusplus.com/reference/cstdio/ftell/

long int get_size(char filename[]) { //function for files in archive (including function create)
    long int size;
    FILE *pFile = fopen (filename,"rb");
    if (pFile==NULL) {
        perror ("Error opening file");
    } else {
        fseek (pFile, 0, SEEK_END);   // non-portable
        size=ftell (pFile);
        fclose (pFile);
        printf ("Size of %s: %ld bytes.\n", filename, size);
    }
    return size;
}



