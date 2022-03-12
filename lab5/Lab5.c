#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>

#define WRITE_LE(X,F) fwrite_int_little_endian(&X, sizeof(X), F);

struct bmp_header {
    uint16_t header_field;
    uint32_t size;
    uint16_t res1;
    uint16_t res2;
    uint32_t pixel_array_offset;
};


struct bmp_info_header {
    uint32_t size;
    uint32_t bitmap_width;
    uint32_t bitmap_height;
    uint16_t color_planes;
    uint16_t color_depth;
    uint32_t compression;
    uint32_t img_size;
    uint32_t hor_res;
    uint32_t vert_res;
    uint32_t palette_colors;
    uint32_t important_colors;
};



char **empty_img(size_t w, size_t h);
char **read_img(size_t *pw, size_t *ph, FILE *f);
void print_img(char **img, size_t w, size_t h);
void store_img(char **img, size_t w, size_t h, char fname[]);
void free_img(char **img, size_t h);
void fwrite_int_little_endian(void *ptr, size_t size, FILE *f);
void fwrite_bmp_header(struct bmp_header *phdr, FILE *f);
void fwrite_bmp_info_header(struct bmp_info_header *phdr, FILE *f);
int neighbours(char **img, size_t w, size_t h, size_t i, size_t j);
int inside_board(size_t w, size_t h, size_t i, size_t j) {
    return i >= 0 && j >= 0 && i < h && j < w;
}


int neighbours(char **img, size_t w, size_t h, size_t i, size_t j) {
    int res = 0;
    int ds[][2] = {{-1, -1}, {-1, 0}, {-1, +1},
                   {0, -1}, {0, +1},
                   {1, -1}, {1, 0}, {1, +1}};

    for (int nd = 0; nd != 8; ++nd) {
        int ni = i + ds[nd][0];
        int nj = j + ds[nd][1];

        if (inside_board(w, h, ni, nj) && img[ni][nj])
            ++res;
    }

    return res;
}

void testwrite() {
    int w = 32, h = 40;
    char **img = empty_img(w, h);

    int bx = 1, by = 1;
    img[by + 0][bx + 0] = 1;
    img[by + 0][bx + 1] = 1;
    img[by + 1][bx + 0] = 1;
    img[by + 1][bx + 1] = 1;

    int bhx = 5, bhy = 1;
    img[bhy + 0][bhx + 1] = 1;
    img[bhy + 0][bhx + 2] = 1;
    img[bhy + 1][bhx + 0] = 1;
    img[bhy + 1][bhx + 3] = 1;
    img[bhy + 2][bhx + 1] = 1;
    img[bhy + 2][bhx + 2] = 1;

    int osx = 10, osy = 3;
    img[osy + 0][osx + 0] = 1;
    img[osy + 0][osx + 1] = 1;
    img[osy + 0][osx + 2] = 1;

    int gliderx = 1, glidery = 30;
    img[glidery + 0][gliderx + 0] = 1;
    img[glidery + 0][gliderx + 1] = 1;
    img[glidery + 0][gliderx + 2] = 1;
    img[glidery + 1][gliderx + 2] = 1;
    img[glidery + 2][gliderx + 1] = 1;



    print_img(img, w, h);
    store_img(img, w, h, "in5.bmp");
    free_img(img, h);
}


int main(int argc, char **argv) {
    int w = 100, h = 60;
    // char **img = empty_img(w, h);
    // testwrite();
    // return 0;

    int is_input = 0, is_output = 0; // is_max_iter = 0, is_dump_freq = 0;
    char *input_filename;
    char *output_dirname;

    int max_iter = -1;
    int dump_freq = 1;


    static struct option long_options[] = {
        {"input", required_argument, 0,  1 },
        {"output",  required_argument,       0,  2 },
        {"max_iter",  required_argument, 0,  3 },
        {"dump_freq", required_argument,       0,  4 },
        {0,         0,                 0,  0 }
    };

    while (1) {
        int option_index = 0;


        int c = getopt_long(argc, argv, "",
                            long_options, &option_index);

        printf("c: %d\n", c);
        if (c == -1)
            break;

        if (c == '?') {
            printf("missing argument\n");
            exit(EXIT_FAILURE);
        }

        switch (c) {
        case 0:
            printf("option %s", long_options[option_index].name);
            if (optarg)
                printf(" with arg %s", optarg);
            printf("\n");
            break;

        case 1:
//            printf("option %d\n", c);
//            printf("arg: %s\n", optarg);
            is_input = 1;
            input_filename = (char*) malloc(strlen(optarg) + 1);
            strcpy(input_filename, optarg);
            break;

        case 2:
//            printf("option %d\n", c);
//            printf("arg: %s\n", optarg);

            is_output = 1;
            output_dirname = (char*) malloc(strlen(optarg) + 1);
            strcpy(output_dirname, optarg);
            break;

        case 3:
//            printf("option %d\n", c);
//            printf("arg: %s\n", optarg);
            sscanf(optarg, "%d", &max_iter);
            break;


        case 4:
//            printf("option %d\n", c);
//            printf("arg: %s\n", optarg);
            sscanf(optarg, "%d", &dump_freq);
            break;
        case '?':
            break;

        default:
            printf("?? getopt returned character code 0%o ??\n", c);
            printf("error parsing options\n");
            exit(EXIT_FAILURE);

        }
    }

    if (!is_input || !is_output) {
        printf("input file, output directory are mandatory\n");
        exit(EXIT_FAILURE);
    }

    printf("infile: %s\n", input_filename);
    printf("outdir: %s\n", output_dirname);

    printf("maxiter: %d\n", max_iter);
    printf("dump_freq: %d\n", dump_freq);

    // FILE *f = fopen("in.bmp", "rb");
    FILE *f = fopen(input_filename, "rb");
    if (!f) {
        printf("cannot open in file\n");
        exit(EXIT_FAILURE);
    }
    char **img = read_img(&w, &h, f);
    print_img(img, w, h);
    // return 0;

    int iter = 0;
    while(1) {
        if (max_iter != -1 && iter >= max_iter) {
            break;
        }

        if (iter % dump_freq == 0) {
            char buf[1000];

            sprintf(buf, "%s\\%s%d.bmp", output_dirname, "out", iter);
            store_img(img, w, h, buf);
            // print_img(img, w, h);
        }


        char **new_gen = empty_img(w, h);

        for (int i = 0; i != h; ++i) {
            for (int j = 0; j != w; ++j) {
                int neigh = neighbours(img, w, h, i, j);
                if (img[i][j]) {
                    if (neigh == 2 || neigh == 3)
                        new_gen[i][j] = 1;
                    else
                        new_gen[i][j] = 0;
                } else {
                    if (neigh == 3)
                        new_gen[i][j] = 1;
                }

            }
        }

        free_img(img, h);
        img = new_gen;
        ++iter;
    }

    store_img(img, w, h, "out.bmp");

    free_img(img, h);
    return 0;
}

char **read_img(size_t *pw, size_t *ph, FILE *f) {
    // pixel_array_offset
    fseek(f, 0x0a, SEEK_SET);
    unsigned char buf[4];
    fread(buf, 4, 1, f);

    unsigned pixel_array_offset = (unsigned)buf[0] << 0;
    pixel_array_offset += (unsigned)buf[1] << 8;
    pixel_array_offset += (unsigned)buf[2] << 16;
    pixel_array_offset += (unsigned)buf[3] << 24;


    fseek(f, 0x12, SEEK_SET);
    fread(buf, 4, 1, f);

    unsigned w = (unsigned)buf[0] << 0;
    w += (unsigned)buf[1] << 8;
    w += (unsigned)buf[2] << 16;
    w += (unsigned)buf[3] << 24;


    fseek(f, 0x16, SEEK_SET);
    fread(buf, 4, 1, f);

    unsigned h = (unsigned)buf[0] << 0;
    h += (unsigned)buf[1] << 8;
    h += (unsigned)buf[2] << 16;
    h += (unsigned)buf[3] << 24;


    // possible non cross-platform code

    printf("%u\n", pixel_array_offset);
    printf("%d\n", h);
    printf("%d\n", w);

    char **img = empty_img(w, h);

    size_t row_size = ceil(1.0 * w / 32) * 4;


    for (int i = 0; i != h; ++i) {
        size_t row_offset = pixel_array_offset + row_size * i;
        fseek(f, row_offset, SEEK_SET);

        int j;
        for (j = 0; j != w / 8; ++j) {
            char b;
            fread(&b, 1, 1, f);
            for (int bit = 0; bit != 8; ++bit) {
                img[i][j * 8 + bit] = ((unsigned)b >> (7 - bit)) & 1;
            }
        }

        if (w % 8 != 0) {
            char b = 0;
            fread(&b, 1, 1, f);
            for (int bit = 0; bit != w % 8; ++bit) {
                img[i][j * 8 + bit] = ((unsigned)b >> (7 - bit)) & 1;
            }
        }
    }

    *ph = h;
    *pw = w;



    return img;

}

void fwrite_int_little_endian(void *ptr, size_t size, FILE *f) {
    // little-endian
    char x;
    if (size == 2) {
        uint16_t val = *(uint16_t *) ptr;
        x = (char) (val & 0xff);
        fwrite(&x, 1, 1, f);
        x = (char) ((val >> 8) & 0xff);
        fwrite(&x, 1, 1, f);
    } else if (size == 4) {
        uint32_t val = *(uint32_t *) ptr;
        x = (char) ((val >> 0) & 0xff);
        fwrite(&x, 1, 1, f);
        x = (char) ((val >> 8) & 0xff);
        fwrite(&x, 1, 1, f);
        x = (char) ((val >> 16) & 0xff);
        fwrite(&x, 1, 1, f);
        x = (char) ((val >> 24) & 0xff);
        fwrite(&x, 1, 1, f);
    }



}

char **empty_img(size_t w, size_t h) {
    char **img = (char **) malloc(sizeof(char *) * h);
    for (int i = 0; i != h; ++i)
        img[i] = (char *) malloc(w);


    for (int i = 0; i != h; ++i) {
        for (int j = 0; j != w; ++j) {
            img[i][j] = 0;
        }
    }

    /*
    for (int j = 0; j != w; ++j)
        img[j%h][j] = 0;
    */

    return img;
}

void free_img(char **img, size_t h) {
    for (int i = 0; i != h; ++i)
        free(img[i]);
    free(img);
}

void print_img(char **img, size_t w, size_t h) {
    for (int i = h - 1; i >= 0; --i) {
        for (int j = 0; j != w; ++j) {
            printf("%c", img[i][j] ? 'x' : '.');
        }
        puts("");
    }
}


void fwrite_bmp_header(struct bmp_header *phdr, FILE *f) {
    WRITE_LE(phdr->header_field, f);
    WRITE_LE(phdr->size, f);
    WRITE_LE(phdr->res1, f);
    WRITE_LE(phdr->res2, f);
    WRITE_LE(phdr->pixel_array_offset, f);
}


void fwrite_bmp_info_header(struct bmp_info_header *phdr, FILE *f) {
    WRITE_LE(phdr->size, f);
    WRITE_LE(phdr->bitmap_width, f);
    WRITE_LE(phdr->bitmap_height, f);
    WRITE_LE(phdr->color_planes, f);
    WRITE_LE(phdr->color_depth, f);
    WRITE_LE(phdr->compression, f);
    WRITE_LE(phdr->img_size, f);
    WRITE_LE(phdr->hor_res, f);
    WRITE_LE(phdr->vert_res, f);
    WRITE_LE(phdr->palette_colors, f);
    WRITE_LE(phdr->important_colors, f);
}


void store_img(char **img, size_t w, size_t h, char fname[]) {
    struct bmp_header bmp_hdr;
    struct bmp_info_header bmp_info_hdr;
    uint32_t color_table[2];

    size_t row_size = ceil(1.0 * w / 32) * 4;

    char **pixel_data = (char **) malloc(sizeof(char *) * h);
    for (int i = 0; i != h; ++i)
        pixel_data[i] = (char *) malloc(row_size);

    bmp_hdr.header_field = 0x4d42;
    bmp_hdr.res1 = 0;
    bmp_hdr.res2 = 0;

    // todo: bmp_hdr.size
    bmp_hdr.size = 2*3 + 4 * 2
                   + 2*2 + 4*9
                   + sizeof(color_table)
                   + h * row_size;

    // todo: bmp_hdr.pixel_array_offset
    bmp_hdr.pixel_array_offset =
        2*3 + 4*2
        + 2*2 + 4*9
        + sizeof(color_table);

    bmp_info_hdr.size = 40;
    bmp_info_hdr.bitmap_width = w;
    bmp_info_hdr.bitmap_height = h;
    bmp_info_hdr.color_planes = 1;
    bmp_info_hdr.color_depth = 1;
    bmp_info_hdr.compression = 0;
    // bmp_info_hdr.img_size = row_size * h;
    bmp_info_hdr.img_size = 0;  // dummy 0 for BI_RGB

    // https://en.wikipedia.org/wiki/BMP_file_format
    // 72 DPI x 39.3701 inches per metre
    // bmp_info_hdr.hor_res = 0x130b00;
    bmp_info_hdr.hor_res = 2835;
    // bmp_info_hdr.vert_res = 0x130b00;
    bmp_info_hdr.vert_res = 2835;

    bmp_info_hdr.palette_colors = 2;
    bmp_info_hdr.important_colors = 2;

    color_table[0] = 0xffffffff;
    color_table[1] = 0xff000000;


    for (int i = 0; i != h; ++i) {
        int j;
        for (j = 0; j != w / 8; ++j) {
            // printf("j = %d\n", j);
            char b = 0;
            b |= img[i][j * 8];
            for (int bit = 1; bit != 8; ++bit) {
                b <<= 1;
                b |= img[i][j * 8 + bit];
            }
            pixel_data[i][j] = b;
        }

        if (w % 8 != 0) {
            char b = 0;
            b |= img[i][j * 8];
            for (int bit = 1; bit != w % 8; ++bit) {
                b <<= 1;
                b |= img[i][j * 8 + bit];
            }
            b <<= (8 - (w % 8));        // !! byte byte byte [0b110x xxxx]
            pixel_data[i][j] = b;
            ++j;
        }

        while (j % 4 != 0) {
            char b = 0;
            pixel_data[i][j++] = b;
        }
    }

    // printf("ok\n");

    FILE *f = fopen(fname, "wb");
    // fwrite(&bmp_hdr, sizeof(bmp_hdr), 1, f);
    fwrite_bmp_header(&bmp_hdr, f);
    // fwrite(&bmp_info_hdr, sizeof(bmp_info_hdr), 1, f);
    fwrite_bmp_info_header(&bmp_info_hdr, f);
    fwrite(color_table, sizeof(color_table), 1, f);
    // for (int i = 0; i != 2; ++i)
    //    WRITE_LE(color_table[i], f);

    for (int i = 0; i != h; ++i)
        fwrite(pixel_data[i], row_size, 1, f);

    fclose(f);

    for (int i = 0; i != h; ++i)
        free(pixel_data[i]);
    free(pixel_data);

}

