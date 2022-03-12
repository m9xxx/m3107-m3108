#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <unistd.h>
#include <getopt.h>
#include <errno.h>




// 10 bytes
struct Id3Header {
    char identifier[3];
    char version[2];
    char flags;
    int8_t size[4];
};

struct Id3FrameHeader {
    char id[4];
    uint8_t size[4];
    char flags[2];
};

struct Id3Frame {
    struct Id3FrameHeader hdr;
    size_t data_len; // redundant!
    char *data;
};

struct FileImage {
    struct Id3Frame *frames;
    int nframes;

    char *mpeg;
    int szmpeg;
};

void deleteFileImage(struct FileImage *fi);

void simpleId3Header(struct Id3Header *phdr, size_t size);
int getNumberOfFrames(FILE *in, size_t tag_size, size_t *ppad_size);
struct Id3Frame* get_frames(FILE *in, int nframes);
struct FileImage* read_file(char fname[]);


void deleteFileImage(struct FileImage *fi) {
    for (int i = 0; i != fi->nframes; ++i) {
        free(fi->frames[i].data);
    }
    free(fi->frames);
    free(fi);
}

void option_get(char fname[], char frame_id[]) {
    // printf("get!\n");

    struct FileImage *fi = read_file(fname);

    // printf("total frames: %d\n", fi->nframes);

    int found = 0;
    char buf[10];


    int i;
    for (i = 0; i != fi->nframes; ++i) {
        strncpy(buf, fi->frames[i].hdr.id, 4);
        buf[4] = 0;

        if (strcmp(buf, frame_id) == 0) {
            found = 1;
            break;
        }
    }

    if (found) {
        if (buf[0] != 'T') {
            printf("not a text frame %s\n", buf);
        } else if (fi->frames[i].data[0] != 0) {
            printf("not a Unicode text frame, not supported so far\n");
        } else {
            printf("%s=", buf);
            for (int j = 1; j != fi->frames[i].data_len; ++j) {
                printf("%c", fi->frames[i].data[j]);
            }
        }

        puts("");
    } else {
        printf("frame not found\n");
    }




    deleteFileImage(fi);
}

void option_set(char fname[], char set[], char value[]) {
    printf("set!\n");

    struct FileImage *fi = read_file(fname);

    // FILE *out = fopen("fname.mp3", "wb");
    //FILE *out = fopen("out.mp3", "wb");
    FILE *out = fopen(fname, "wb");

    if (!out) {
        fprintf(stderr, "cannot open file [%s]\n", fname);
        printf("Error %d \n", errno);

        exit(EXIT_FAILURE);
    }

    int found = 0;
    int i = 0;
    for (i = 0; i != fi->nframes; ++i) {
        char buf[5];
        strncpy(buf, fi->frames[i].hdr.id, 4);
        buf[4] = 0;

        if (strcmp(buf, set) == 0) {
            found = 1;
            break;
        }
    }


    struct Id3FrameHeader newhdr;
    memcpy(newhdr.id, set, 4);
    newhdr.flags[0] = 0;
    newhdr.flags[1] = 0;

    char *data = (char *) malloc(strlen(value) + 2);
    data[0] = 0;
    strcpy(data + 1, value);

    int sz = strlen(value) + 2;
    newhdr.size[0] = (sz >> 24) & 0xff;
    newhdr.size[1] = (sz >> 16) & 0xff;
    newhdr.size[2] = (sz >> 8) & 0xff;
    newhdr.size[3] = (sz >> 0) & 0xff;

    if (found == 1) {
        fi->frames[i].hdr = newhdr;
        free(fi->frames[i].data);
        fi->frames[i].data = data;
        fi->frames[i].data_len = sz;
    }




    size_t id3tag_size = 0;
    for (int i = 0; i != fi->nframes; ++i) {
        id3tag_size += fi->frames[i].data_len;
        id3tag_size += 10;
    }

    struct Id3Header hdr;
    simpleId3Header(&hdr, id3tag_size);
    fwrite(&hdr, sizeof(struct Id3Header), 1, out);
    for (int i = 0; i != fi->nframes; ++i) {
        fwrite(&fi->frames[i].hdr, 10, 1, out);
        fwrite(fi->frames[i].data, fi->frames[i].data_len, 1, out);
    }
    fwrite(fi->mpeg, 1, fi->szmpeg, out);
    fclose(out);
    free(fi->mpeg);

    deleteFileImage(fi);

}

void option_show(char fname[]) {
    printf("show!\n");

    struct FileImage *fi = read_file(fname);

    printf("total frames: %d\n", fi->nframes);

    for (int i = 0; i != fi->nframes; ++i) {
        printf("%d\n", i);
        printf("frame id: %c%c%c%c\n",
               fi->frames[i].hdr.id[0],
               fi->frames[i].hdr.id[1],
               fi->frames[i].hdr.id[2],
               fi->frames[i].hdr.id[3]);
        // printf("size: %lu\n", frame_size);

        char buf[10];
        strncpy(buf, fi->frames[i].hdr.id, 4);
        buf[4] = 0;

        if (strcmp(buf, "APIC") == 0) {
            printf("apic => image type frame\n");
            continue;
        }

        if (buf[0] != 'T') {
            printf("not a text frame\n");
            continue;
        }

        if (fi->frames[i].data[0] != 0) {
            printf("not a Unicode text frame, not supported so far\n");
            continue;
        }


        for (int j = 1; j != fi->frames[i].data_len; ++j) {
            printf("%c", fi->frames[i].data[j]);
        }

        puts("");
    }


    deleteFileImage(fi);
}

struct FileImage* read_file(char filename[]) {

    struct FileImage *res = (struct FileImage *)
                            malloc(sizeof(struct FileImage));

    FILE *in = fopen(filename, "rb");

    fseek(in, 0, SEEK_END);
    long sz = ftell(in);
    fseek(in, 0, SEEK_SET);

    struct Id3Header hdr;

    // must return 10
    fread(&hdr, sizeof(struct Id3Header), 1, in);

    /*
    printf("id3 header:\n");
    printf("identifier: \"%c%c%c\"\n",
           hdr.identifier[0], hdr.identifier[1], hdr.identifier[2]);
    printf("version: %d %d\n", hdr.version[0], hdr.version[1]);
    printf("flags: %02x\n", (int) hdr.flags);
    */

    unsigned long size = 0;
    size |= hdr.size[0];
    size <<= 7;
    size |= hdr.size[1];
    size <<= 7;
    size |= hdr.size[2];
    size <<= 7;
    size |= hdr.size[3];
    // printf("size: %lu %08lx\n", size, size);

    size_t pad_size;
    int nframes = getNumberOfFrames(in, size, &pad_size);
    // printf("total frames: %d\n", nframes);

    long pos = ftell(in);

    res->nframes = nframes;
    res->frames = get_frames(in, nframes);

    pos = ftell(in);
    fseek(in, (long) pad_size, SEEK_CUR);
    pos = ftell(in);

    long to_read = sz - pos;

    // printf("to_read (mpeg): %lu\n", (unsigned long) to_read);

    char *mpeg = (char *) malloc(to_read);
    fread(mpeg, 1, to_read, in);

    res->mpeg = mpeg;
    res->szmpeg = to_read;

    fclose(in);

    return res;

};

void test1() {
    printf("Hello world!\n");
    char filename[] = "in3.mp3";

    FILE *in = fopen(filename, "rb");

    fseek(in, 0, SEEK_END);
    long sz = ftell(in);
    fseek(in, 0, SEEK_SET);

    printf("size: %ld\n", sz);

    struct Id3Header hdr;

    // must return 10
    fread(&hdr, sizeof(struct Id3Header), 1, in);

    printf("id3 header:\n");
    printf("identifier: \"%c%c%c\"\n",
           hdr.identifier[0], hdr.identifier[1], hdr.identifier[2]);
    printf("version: %d %d\n", hdr.version[0], hdr.version[1]);
    printf("flags: %02x\n", (int) hdr.flags);
    unsigned long size = 0;
    size |= hdr.size[0];
    size <<= 7;
    size |= hdr.size[1];
    size <<= 7;
    size |= hdr.size[2];
    size <<= 7;
    size |= hdr.size[3];
    printf("size: %lu %08lx\n", size, size);

    size_t pad_size;
    int nframes = getNumberOfFrames(in, size, &pad_size);
    printf("total frames: %d\n", nframes);

    long pos = ftell(in);
    struct Id3Frame *frames = get_frames(in, nframes);
    pos = ftell(in);
    fseek(in, (long) pad_size, SEEK_CUR);
    pos = ftell(in);
    long to_read = sz - pos;
    printf("to_read (mpeg): %lu\n", (unsigned long) to_read);

    char *mpeg = (char *) malloc(to_read);
    fread(mpeg, 1, to_read, in);

    fclose(in);


    for (int i = 0; i != nframes; ++i) {
        printf("%d\n", i);
        printf("frame id: %c%c%c%c\n",
               frames[i].hdr.id[0], frames[i].hdr.id[1],frames[i].hdr.id[2], frames[i].hdr.id[3]);
        // printf("size: %lu\n", frame_size);
        // printf("flags: %02x\n", (int) ((frames[i].hdr.flags[0] << 8) + frames[i].hdr.flags[1]));

        char buf[10];
        strncpy(buf, frames[i].hdr.id, 4);
        buf[4] = 0;

        if (strcmp(buf, "APIC") == 0) {
            continue;
        }

        for (int j = 0; j != frames[i].data_len; ++j) {
            printf("%c", frames[i].data[j]);
        }

        puts("");

    }


    FILE *out = fopen("out.mp3", "wb");


    size_t id3tag_size = 0;
    for (int i = 0; i != nframes; ++i) {
        id3tag_size += frames[i].data_len;
        id3tag_size += 10;
    }
    simpleId3Header(&hdr, id3tag_size);
    fwrite(&hdr, sizeof(struct Id3Header), 1, out);
    for (int i = 0; i != nframes; ++i) {
        fwrite(&frames[i].hdr, 10, 1, out);
        fwrite(frames[i].data, frames[i].data_len, 1, out);
    }
    fwrite(mpeg, 1, to_read, out);
    fclose(out);
    free(mpeg);

}

enum CMD {SHOW, SET, VALUE, GET, FILEPATH};

int main(int argc, char *argv[]) {

    int c;
    int digit_optind = 0;

    int is_show = 0, is_set = 0, is_value = 0;
    int is_get = 0, is_filepath = 0;

    char *filepath = 0;
    char *frame_id = 0;
    char *set = 0;
    char *value = 0;

    while (1) {
        int this_option_optind = optind ? optind : 1;
        int option_index = 0;
        static struct option long_options[] = {
            {"show",     no_argument, 0,  SHOW },
            {"set",  required_argument,       0,  SET },
            {"value",  required_argument, 0,  VALUE },
            {"get", required_argument,       0,  GET },
            {"filepath",  required_argument, 0, FILEPATH},
            {0,         0,                 0,  0 }
        };

        c = getopt_long(argc, argv, "",
                        long_options, &option_index);
        if (c == -1)
            break;




        switch (c) {
        case SHOW:
            printf("SHOW\n");
            is_show = 1;
            break;
        case SET:
            printf("SET %s\n", optarg);
            is_set = 1;
            set = (char *) malloc(strlen(optarg) + 1);
            strcpy(set, optarg);
            break;
        case VALUE:
            printf("VALUE %s\n", optarg);
            is_value = 1;
            value = (char *) malloc(strlen(optarg) + 1);
            strcpy(value, optarg);
            break;
        case GET:
            // printf("GET %s\n", optarg);
            is_get = 1;
            frame_id = (char *) malloc(strlen(optarg) + 1);
            strcpy(frame_id, optarg);
            break;
        case FILEPATH:
            // printf("FILEPATH %s\n", optarg);
            is_filepath = 1;
            filepath = (char *) malloc(strlen(optarg) + 1);
            strcpy(filepath, optarg);
            break;
        default:
            printf("?? getopt returned character code 0%o ??\n", c);
        }
    }

    if (optind < argc) {
        printf("non-option ARGV-elements: ");
        while (optind < argc)
            printf("%s ", argv[optind++]);
        printf("\n");
    }


    if (!is_filepath) {
        fprintf(stderr, "filepath is required\n");
        exit(EXIT_FAILURE);
    }

    if (is_show) {
        option_show(filepath);
    }

    if (is_get) {
        option_get(filepath, frame_id);
    }

    if (is_set) {
        if (!is_value) {
            fprintf(stderr, "value is required\n");
            exit(EXIT_FAILURE);
        }

        option_set(filepath, set, value);
    }


    return 0;
}


void simpleId3Header(struct Id3Header *phdr, size_t size) {
    phdr->identifier[0] = 'I';
    phdr->identifier[1] = 'D';
    phdr->identifier[2] = '3';

    phdr->version[0] = 3;
    phdr->version[1] = 0;

    phdr->flags = 0;

    for (int i = 3; i >= 0; --i) {
        phdr->size[i] = (size & 0x7f);
        size >>= 7;
    }
}


int getNumberOfFrames(FILE *in, size_t tag_size, size_t *ppad_size) {

    fseek(in, 10, SEEK_SET);

    size_t pad_bytes = 0;

    int nframes = 0;
    long to_read = tag_size;
    while (to_read > 0) {
        struct Id3FrameHeader frame_hdr;

        fread(&frame_hdr, 1, 1, in);
        if (* (char*) &frame_hdr == 0) {
            ++pad_bytes;
            --to_read;
            continue;
        }
        fread(1 + (char *) &frame_hdr, sizeof(struct Id3FrameHeader) - 1, 1, in);
        to_read -= 10;

        ++nframes;

        unsigned long frame_size = 0;
        frame_size += frame_hdr.size[0] << 24;
        frame_size += frame_hdr.size[1] << 16;
        frame_size += frame_hdr.size[2] << 8;
        frame_size += frame_hdr.size[3] << 0;


        /*
        printf("frame id: %c%c%c%c\n",
               frame_hdr.id[0], frame_hdr.id[1],frame_hdr.id[2], frame_hdr.id[3]);
        printf("size: %lu\n", frame_size);
        printf("flags: %02x\n", (int) ((frame_hdr.flags[0] << 8) + frame_hdr.flags[1]));
        */

        /*
        char *bytes = (char *) malloc(frame_size);
        fread(bytes, 1, frame_size, in);
        to_read -= frame_size;
        */
        fseek(in, frame_size, SEEK_CUR);
        to_read -= frame_size;

        // free(bytes);
    }
    // save all frames

    // printf("padding bytes: %lu\n", (unsigned long) pad_bytes);

    *ppad_size = pad_bytes;

    return nframes;

}



struct Id3Frame* get_frames(FILE *in, int nframes) {
    fseek(in, 10, SEEK_SET);

    struct Id3Frame *frames = (struct Id3Frame *)
                              malloc(sizeof(struct Id3Frame) * nframes);

    for (int i = 0; i != nframes; ++i) {
        fread(&frames[i].hdr, sizeof(struct Id3FrameHeader), 1, in);

        unsigned long frame_size = 0;
        frame_size += frames[i].hdr.size[0] << 24;
        frame_size += frames[i].hdr.size[1] << 16;
        frame_size += frames[i].hdr.size[2] << 8;
        frame_size += frames[i].hdr.size[3] << 0;

        /*
        printf("frame id: %c%c%c%c\n",
               frames[i].hdr.id[0], frames[i].hdr.id[1],frames[i].hdr.id[2], frames[i].hdr.id[3]);
        printf("size: %lu\n", frame_size);
        printf("flags: %02x\n", (int) ((frames[i].hdr.flags[0] << 8) + frames[i].hdr.flags[1]));
        */


        frames[i].data_len = frame_size;
        frames[i].data = (char *) malloc(frame_size);
        fread(frames[i].data, 1, frame_size, in);

    }

    return frames;
}
