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
            printf("• -v : Print compression statistics to stderr.\n\
• -i <input> : Specify input to compress (stdin by default)\n\
• -o <output> : Specify output of compressed input (stdout by default)");
            return 0;
        }
    }
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
    //michael helped a lot with figuring out how to get and set header values
    // Get and set header for file
    struct stat file_stats;
    fstat(outfile, &file_stats);
    // Allocate memory for header
    FileHeader *header = (FileHeader *) calloc(1, sizeof(FileHeader));
    // Set protection field in header
    //if stats is larger than a 16 bit integer than shrink it down to fit protection
    if ((file_stats.st_mode & 0777) > 65535) {
        header->protection = (uint16_t) (file_stats.st_mode & 0xffff);
    } else {
        header->protection = (uint16_t) (file_stats.st_mode & 0777);
    }
    //set magic number in header
    header->magic = 0xBAADBAAC;
    // Write header to file
    write_header(outfile, header);
    //set all needed variables
    TrieNode *root = trie_create();
    TrieNode *curr_node = root;
    TrieNode *prev_node = NULL;
    uint8_t curr_sym = 0;
    uint8_t prev_sym = 0;
    uint16_t next_code = START_CODE;
    //main loop based of pseudocode
    while (read_sym(infile, &curr_sym)) {
        TrieNode *next_node = trie_step(curr_node, curr_sym);
        if (next_node != NULL) {
            prev_node = curr_node;
            curr_node = next_node;
        } else {
            write_pair(outfile, curr_node->code, curr_sym, (uint16_t) log2(next_code) + 1);
            curr_node->children[curr_sym] = trie_node_create(next_code);
            curr_node = root;
            next_code = next_code + 1;
        }
        if (next_code == MAX_CODE) {
            trie_reset(root);
            curr_node = root;
            next_code = START_CODE;
        }
        prev_sym = curr_sym;
    }
    if (curr_node != root) {
        write_pair(outfile, prev_node->code, prev_sym, (uint16_t) log2(next_code) + 1);
        next_code = (next_code + 1) % MAX_CODE;
    }
    write_pair(outfile, STOP_CODE, 0, (uint16_t) log2(next_code) + 1);
    flush_pairs(outfile);
    //close files
    trie_delete(root);
    free(header);
    //get the size of the files
    float input_size = lseek(infile, 0, SEEK_END);
    float ouput_size = lseek(outfile, 0, SEEK_END);
    if (verbose) {
        printf("Compressed file size: %.0f bytes\nUncompressed file size: %.0f bytes\n\
Compression ratio: %.2f%%\n",
            ouput_size, input_size, 100 * (1 - (ouput_size / input_size)));
    }
    if (casei) {
        close(infile);
    }
    if (caseo) {
        close(outfile);
    }
    return 0;
}
