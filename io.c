#include "io.h"
#include "code.h"
#include <stdio.h>
#include <unistd.h>
#include "endian.h"
#include <stdlib.h>
#include <string.h>
//one global buffer for syms and one buffer for bit pairs
static uint8_t bbuffer[BLOCK];
static uint8_t sbuffer[BLOCK];
static int sbuffer_index = 0;
static int sbuffer_end = 0;
static int bbuffer_index = 0;
static int bbuffer_end = 0;

int read_bytes(int infile, uint8_t *buf, int to_read) {
    int batch, total = 0;
    do {
        batch = read(infile, buf + total, to_read - total);
        if (batch > 0) {
            total += batch;
        }
    } while (batch > 0 && total < to_read);
    return total;
}

int write_bytes(int outfile, uint8_t *buf, int to_write) {
    int batch, total = 0;
    do {
        batch = write(outfile, buf + total, to_write - total);
        if (batch > 0) {
            total += batch;
        }
    } while (batch > 0 && total < to_write);
    return total;
}

void read_header(int infile, FileHeader *header) {
    //read from infile to header sizeof fileheader number of bytes
    int bytes = read_bytes(infile, (uint8_t *) header, sizeof(FileHeader));
    //if no bytes were read break
    if (bytes == 0) {
        return;
    }
    //set magic number
    uint32_t magic;
    memcpy(&magic, &header->magic, sizeof(uint32_t));
    //set protection
    uint16_t protection;
    memcpy(&protection, &header->protection, sizeof(uint16_t));
    //check if its big_endian order and swap if so
    if (big_endian()) {
        magic = swap32(magic);
        protection = swap16(protection);
    }
    //set the values for the parts of the struct
    header->magic = magic;
    header->protection = protection;
}

void write_header(int outfile, FileHeader *header) {
    //create a buffer array of size fileheader
    uint8_t buffer[sizeof(FileHeader)];
    //copy magic number and protection from the header
    uint32_t magic = header->magic;
    uint16_t protection = header->protection;
    //check endianess
    if (big_endian()) {
        magic = swap32(magic);
        protection = swap16(protection);
    }
    //copy magic and protection into header array
    memcpy(buffer, &magic, sizeof(uint32_t));
    memcpy(buffer + sizeof(uint32_t), &protection, sizeof(uint16_t));
    //write header to outfile
    write_bytes(outfile, buffer, sizeof(FileHeader));
}

bool read_sym(int infile, uint8_t *sym) {
    //if the index is at the end of buffer
    if (sbuffer_index >= sbuffer_end) {
        //read more bytes into buffer
        int num_read = read_bytes(infile, sbuffer, BLOCK);
        //if no bytes read return false
        if (num_read == 0) {
            return false;
        }
        //reset buffer index
        sbuffer_index = 0;
        //set the end of buffer to the number of bytes read
        sbuffer_end = num_read;
    }
    //get the next symbol from the buffer and increment the buffer position
    *sym = sbuffer[sbuffer_index];
    sbuffer_index++;
    return true;
}

void write_pair(int outfile, uint16_t code, uint8_t sym, int bitlen) {
    //if bitlen is too small exit
    if (bitlen <= 0) {
        return;
    }
    for (int i = 0; i < bitlen; i++) {
        //if buffer if full print it out
        if (bbuffer_index == BLOCK * 8) {
            flush_pairs(outfile);
            //reset all bits of the buffer to 0
            memset(bbuffer, 0, sizeof(bbuffer));
        }
        //set bit at next available index of bbuffer
        //check if the bit is a 1 and if so set the bit in buffer
        if ((code >> (i % 16)) & (uint16_t) 1) {
            //set bit inside bbuffer[index]
            bbuffer[bbuffer_index / 8] = bbuffer[bbuffer_index / 8] | (1 << bbuffer_index % 8);
        }
        bbuffer_index++;
    } //do a similar thing for the sym but its length 8
    for (int i = 0; i < 8; i++) {
        if (bbuffer_index == BLOCK * 8) {
            flush_pairs(outfile);
            //reset all bits of the buffer to 0
            memset(bbuffer, 0, sizeof(bbuffer));
        }
        if ((sym >> (i)) & (uint8_t) 1) {
            bbuffer[bbuffer_index / 8] = bbuffer[bbuffer_index / 8] | (1 << bbuffer_index % 8);
        }
        bbuffer_index++;
    }
}

void flush_pairs(int outfile) {
    //check if bbuffer index is not 0
    if (bbuffer_index > 0) {
        //write out all bytes left in bbuffer up to end of whats left in bbuffer rounded
        write_bytes(outfile, bbuffer, bbuffer_index / 8 + (bbuffer_index % 8 == 0 ? 0 : 1));
        //reset index
        bbuffer_index = 0;
    }
}

bool read_pair(int infile, uint16_t *code, uint8_t *sym, int bitlen) {
    //initialize code and sym to 0
    *code = 0;
    *sym = 0;
    //if buffer is empty read into it
    if (bbuffer_index == 0) {
        bbuffer_end = read_bytes(infile, bbuffer, BLOCK);
    }
    for (int i = 0; i < bitlen; i++) {
        //if buffer has already been read restart at beginning of buffer and read 3 more bytes
        if (bbuffer_index == BLOCK * 8) {
            bbuffer_index = 0;
            bbuffer_end = read_bytes(infile, bbuffer, BLOCK);
            //if nothing was read return false
            if (bbuffer_end == 0) {
                return false;
            }
        }
        //if bit in bbuffer is 1
        if ((bbuffer[bbuffer_index / 8] >> (bbuffer_index % 8)) & (uint8_t) 1) {
            //set bit of code to 1
            *code |= (1 << i);
        }
        bbuffer_index++;
    }
    for (int i = 0; i < 8; i++) {
        //if buffer has already been read restart at beginning of buffer
        if (bbuffer_index == BLOCK * 8) {
            bbuffer_index = 0;
            bbuffer_end = read_bytes(infile, bbuffer, BLOCK);
            //if nothing was read return false
            if (bbuffer_end == 0) {
                return false;
            }
        }
        //if bit in bbuffer is 1
        if ((bbuffer[bbuffer_index / 8] >> (bbuffer_index % 8)) & (uint8_t) 1) {
            //set bit of sym to 1
            *sym |= (1 << i);
        }
        bbuffer_index++;
    }
    //Check if there are more pairs to read
    if (*code != STOP_CODE) {
        return true;
    } else {
        return false;
    }
}

void write_word(int outfile, Word *w) {
    //iterate through all syms in the word
    for (uint32_t i = 0; i < w->len; i++) {
        //if buffer is full reset it and write out the buffer
        if (sbuffer_index == BLOCK) {
            write_bytes(outfile, sbuffer, BLOCK);
            sbuffer_index = 0;
            //reset all bits of the buffer to 0
            memset(sbuffer, 0, sizeof(sbuffer));
        }
        //set value inside buffer to the sym
        sbuffer[sbuffer_index] = w->syms[i];
        //increment sbuffer index
        sbuffer_index++;
    }
}

void flush_words(int outfile) {
    //check if bbuffer index is not 0
    //write out all bytes left in bbuffer up to end of bbuffer
    write_bytes(outfile, sbuffer, sbuffer_index);
    //reset index
    sbuffer_index = 0;
}
