#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ENTIRE_FILE -1

char xor_tbl[] =
{
  0xC9, 0x95, 0x84, 0xBD, 0x65, 0x15, 0x16, 0x15, 0xD2, 0xE7, 
  0x16, 0xD7, 0xD9, 0x70, 0x3C, 0x00, 0xA0, 0x95, 0x33, 0xF7, 
  0x51, 0xCE, 0xE4, 0xA0, 0x13, 0xCF, 0x33, 0x1E, 0x59, 0x5B, 
  0xEC, 0x42, 0x1E, 0x00, 0x00, 0x00, 0x8F, 0x70, 0xF4, 0xC1, 
  0xB5, 0x3D, 0x7C, 0x60, 0x4C, 0x09, 0xF4, 0x2E, 0x7C, 0x87, 
  0x63, 0x5F, 0xF9, 0x0D, 0xC1, 0x00, 0xC3, 0x74, 0xBE, 0x02, 
  0x09, 0x0A, 0xB0, 0xB1, 0x8E, 0x9B, 0x08, 0xED, 0x3E, 0x1C, 
  0x62, 0x17, 0xCD, 0xF2, 0xE4, 0x14, 0x9D, 0xA6, 0x92, 0x00,

  0x81, 0xE2, 0xB3, 0x96, 0xF1, 0xC2, 0xD3, 0x9B, 0xA1, 0x82, 
  0x73, 0x66, 0x41, 0x12, 0x33, 0x00, 0xAA, 0x66, 0x11, 0x22, 
  0x33, 0x96, 0xA1, 0x2C, 0xC3, 0x56, 0xD1, 0x92, 0x83, 0x56, 
  0xF1, 0xE2, 0xA3, 0x00, 0x00, 0x00, 0x11, 0x12, 0x13, 0x16, 
  0x15, 0x18, 0x19, 0x20, 0x21, 0x22, 0x23, 0x26, 0x31, 0x22, 
  0x43, 0x52, 0x53, 0x32, 0x13, 0x00, 0x8A, 0x76, 0x71, 0x72, 
  0x73, 0x76, 0x71, 0x7C, 0x73, 0x76, 0x71, 0x72, 0x73, 0x76, 
  0x71, 0x72, 0x73, 0x72, 0x73, 0x72, 0x73, 0x72, 0x93, 0x00
};

char xor_tbl_assets[] =
{
    0x5e, 0x02, 0xe3, 0x4d, 0x9e, 0x80, 0x28, 0x50,
    0xa0, 0x81, 0xe4, 0xa5, 0x97, 0xa1, 0x86, 0x36,
    0x70, 0x62, 0x8d, 0x00, 0xf2, 0x43, 0x53, 0x7c,
    0x01, 0xfe, 0xd2, 0x91, 0x40, 0x85, 0xa3, 0xe8,
    0xdf, 0xdb, 0x00, 0x00, 0xfc, 0x03, 0x43, 0xef,
    0xaa, 0x7c, 0xbb, 0x2c, 0x90, 0xcc, 0xce, 0xe8,
    0x3e, 0x00, 0x00, 0x00, 0xaf, 0xf5, 0xf8, 0x50,
    0x07, 0xe6, 0xc7, 0x5e, 0xd7, 0xde, 0xe5, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/* Loads the file given by 'filename' into a new buffer, the 'out_contents'
 * parameter is modified to point at the newly allocated buffer. The caller
 * is responsible for calling free() on the buffer when finished using it. */
static int load_file(const char *filename, int max_len, char **out_contents)
{
    FILE *fp;
    long len;
    char *buf;

    fp = fopen(filename, "r");
    if (!fp) {
        perror("opening input file");
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    len = ftell(fp);
    if (max_len != ENTIRE_FILE && len > max_len) {
        len = max_len;
    }
    rewind(fp);

    *out_contents = malloc(len);
    fread(*out_contents, len, 1, fp);

    fclose(fp);
    return len;
}

int decrypt_500kb(const char *filename)
{
    FILE *fp;
    char *buf, *cur;
    int len;
    int off1 = 0, off2 = 0, off3 = 0, off4 = 0;

    len = load_file(filename, 500000, &buf);
    fp = fopen(filename, "r+b");

    for (cur = buf; cur < buf + len; cur++) {
        *cur ^= xor_tbl[off4 + 56]
             ^  xor_tbl[off3 + 36]
             ^  xor_tbl[off2 + 16]
             ^  xor_tbl[off1     ];

    /*    char v1 = xor_tbl[off1];
        char v2 = xor_tbl[off2 + 16];
        char v3 = xor_tbl[off3 + 36];
        char v4 = xor_tbl[off4 + 56];
        
        char i1 = v1 ^ v2;
        char i2 = *cur ^ i1;
        char i3 = i2 ^ v3;
        *cur = i3 ^ v4; */

        off1 = (off1 + 1) % 15;
        off2 = (off2 + 1) % 17;
        off3 = (off3 + 1) % 19;
        off4 = (off4 + 1) % 23;
    }

    fwrite(buf, len, 1, fp);

    fclose(fp);
    free(buf);

    return 1;
}

int decrypt_asset(const char *filename)
{
    FILE *fp;
    char *buf, *cur;
    int len;
    int off1 = 0, off2 = 0, off3 = 0, off4 = 0;

    len = load_file(filename, ENTIRE_FILE, &buf);
    fp = fopen(filename, "w+b");

    for (cur = buf; cur < buf + len; cur++) {
        *cur ^= xor_tbl_assets[off4 + 52]
             ^  xor_tbl_assets[off3 + 36]
             ^  xor_tbl_assets[off2 + 20]
             ^  xor_tbl_assets[off1     ];

        off4 = (off4 + 1) % 11;
        off3 = (off3 + 1) % 13;
        off2 = (off2 + 1) % 14;
        off1 = (off1 + 1) % 19;
    }

    fwrite(buf, len, 1, fp);

    fclose(fp);
    free(buf);

    return 1;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "usage: %s <mode> <input file>\n", argv[0]);
        return 1;
    }

    if (!strcmp(argv[1], "bootstrap")) {
        decrypt_500kb(argv[2]);
    } else if (!strcmp(argv[1], "asset")) {
        decrypt_asset(argv[2]);
    } else {
        fprintf(stderr, "mode must be 'bootstrap' or 'asset'\n");
        return 1;
    }

    return 0;
}
