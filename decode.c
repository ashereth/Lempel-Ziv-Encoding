#include "io.c"
#include "word.c"
#include "trie.c"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#define OPTIONS "i:o:vh"

char input[100];
char output[100];

int main(int argc, char **argv) {
    int verbose = 0;
    int casei = 0;
    int caseo = 0;
    int opt = 0;
    while ((opt = getopt(argc, argv, OPTIONS)) != -1) {
        switch (opt) {
        case 'i':

            ++casei;
            strcpy(input, optarg); //set input file
            break;
        case 'o':
            ++caseo;
            strcpy(output, optarg); //set ouput file
            break;
        case 'v': ++verbose; break;
        case 'h':
            printf("• -v : Print decompression statistics to stderr.\n\
• -i <input> : Specify input to decompress (stdin by default)\n\
• -o <output> : Specify output of decompressed input (stdout by default)");
            return 0;
        }
    }
    //open files
    //open files
    int infile;
    int outfile;
    if (!casei) {
        infile = dup(STDIN_FILENO);
    } else {
        infile = open(input, O_RDONLY);
    }
    if (!caseo) {
        outfile = dup(STDOUT_FILENO);
    } else {
        outfile = open(output, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    }
    if (infile == -1 || outfile == -1) {
        printf("failed to open file\n");
        return 1;
    }
    FileHeader *header = (FileHeader *) calloc(1, sizeof(FileHeader));
    //read the header
    read_header(infile, header);
    //check magic number
    if (header->magic != MAGIC) {
        printf("wrong magic number");
        return 0;
    }
    WordTable *table = wt_create();
    uint8_t curr_sym = 0;
    uint16_t curr_code = 0;
    uint16_t next_code = START_CODE;
    //main loop from pseudocode
    while (read_pair(infile, &curr_code, &curr_sym, (uint16_t) log2(next_code) + 1)) {
        table[next_code] = word_append_sym(table[curr_code], curr_sym);
        write_word(outfile, table[next_code]);
        next_code = next_code + 1;
        if (next_code == MAX_CODE) {
            wt_reset(table);
            next_code = START_CODE;
        }
    }
    flush_words(outfile);
    //free all memory
    free(header);
    //get the size of the files
    float input_size = lseek(infile, 0, SEEK_END);
    float ouput_size = lseek(outfile, 0, SEEK_END);
    if (verbose) {
        printf("Compressed file size: %.0f bytes\nUncompressed file size: %.0f bytes\n\
Compression ratio: %.2f%%\n",
            ouput_size, input_size, 100 * (1 - (ouput_size / input_size)));
    }
    //close files
    if (casei) {
        close(infile);
    }
    if (caseo) {
        close(outfile);
    }
    //delete the word table
    wt_delete(table);
    return 0;
}
