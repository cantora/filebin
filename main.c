/*
   Author: Ilan Schnell, 2008-2013
   License: BSD
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define VERSION  "1.0.1"

/* command line options */
int table = 0;
int names = 0;
int full = 0;

/* byte counts and total count */
long long bin[256], n = 0;

#define USAGE  printf("usage: %s [-fnthv] [file] ...\n", argv[0])

void help(void)
{
    printf("\n\
Calculates the frequency of bytes of file(s) or stdin.\n\
Options:\n\
  -f   display full table (including zero counts, implies -t)\n\
  -n   show names when displaying table (implies -t)\n\
  -t   show table\n\
  -h   display this help and exit\n\
  -v   display version number and exit\n");
}

/* changes the global variables: table, names */
void process_options(int argc, char *argv[])
{
    int op;

    while ((op = getopt(argc, argv, "nfthv")) != -1)
        switch(op) {
        case 'h':
            USAGE;
            help();
            exit(0);
        case 'f':
            full = table = 1;
            break;
        case 'n':
            names = table = 1;
            break;
        case 't':
            table = 1;
            break;
        case 'v':
            printf("filebin " VERSION "\n");
            exit(0);
        default:
            USAGE;
            exit(1);
        }
}

const long long MB_SIZE = 1 << 20;
const int PROGRESS_INTERVAL = 1;

void progress() {
    float mbs;

    mbs = n/((float) MB_SIZE);

    printf("\r%.1f MBs", mbs);
    fflush(stdout);
}

void process_fp(FILE *fp)
{
    time_t t, tmp;
    int c;

    t = time(NULL);
    while ((c = fgetc(fp)) != EOF) {
        bin[c]++;
        n++;
        tmp = time(NULL);
        if((tmp - t) >= PROGRESS_INTERVAL) {
          t = tmp;
          progress();
        }
    }
    puts("\n");
}

void process_file(char *fname)
{
    FILE *fp;

    fp = fopen(fname, "rb");
    if (fp == NULL) {
        printf("Error: Can't open `%s' for reading.\n", fname);
        exit(1);
    }
    process_fp(fp);
    fclose(fp);
}

/* Returns the name of character c as a string,
   e.g. "A", "B", "<Space>", "<LF>"
   When the character has no specific name, "." is returned.
*/
char *name(int c)
{
    static char res[8];

    strcpy(res, ".");  /* default */

    if (c >= 33 && c <= 126) {
        res[0] = (char)c;
        res[1] = '\0';
    }
#define M(_c, _name)  else if (c==_c) strcpy(res, "<" _name ">")
    M(0, "Null");
    M(9, "Tab");
    M(10, "LF");
    M(13, "CR");
    M(32, "Space");
    M(127, "Del");
#undef M
    return res;
}

void show_table(void)
{
    int c;

    if (names)
        printf("Char               Count\n"
               "------------------------\n");

    for (c = 0; c < 256; c++)
        if (full || bin[c]) {
            if (names)
                printf("%4d %8s %10lld\n", c, name(c), bin[c]);
            else
                printf("%4d %10lld\n", c, bin[c]);
        }
}

void show_sum(void)
{
    long long a, b1, b2;
    int c;

    a = 0;
    for (c = 32; c <= 126; c++)
        a += bin[c];
    printf("ASCII (32..126)..........  :%12lld\n", a);
    printf("   0 (Null)............... :%12lld\n", bin[0]);
    printf("   9 (Tab)................ :%12lld\n", bin[9]);
    printf("  10 (LF)................. :%12lld\n", bin[10]);
    printf("  13 (CR)................. :%12lld\n", bin[13]);
    printf(" 127 (Del) ............... :%12lld\n", bin[127]);

    b1 = bin[11]+bin[12];
    for (c =  1; c <=  8; c++) b1 += bin[c];
    for (c = 14; c <= 31; c++) b1 += bin[c];
    printf("Others (1..8,11,12,14..31) :%12lld\n", b1);

    b2 = 0;
    for (c = 128; c <= 255; c++) b2 += bin[c];
    printf("Others (128..255) ........ :%12lld\n", b2);

    printf("                             -----------\n");
    printf("Total .................... :%12lld\n", n);
}

int main(int argc, char *argv[])
{
    int c;

    process_options(argc, argv);

    /* Initialize array */
    for (c = 0; c < 256; c++)
        bin[c] = 0;

    if (argc == optind)
        process_fp(stdin);
    else
        while (optind < argc)
            process_file(argv[optind++]);

    if (table)
        show_table();
    else
        show_sum();

    return 0;
}
