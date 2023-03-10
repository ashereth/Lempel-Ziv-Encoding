# Lempel-Ziv Compression

## Description:
This program is used to compress the binary of a file using the Lempel-Ziv compression algorithm so that it is smaller and more manageable. After the file has been compressed/
this program can also decompress it back to the original without loosing any data.

## Building Executables:
To build all executables make sure to first download all files and enter the directory of those files and run\
$ make or $ make all. To make executables for only one program run one of the following options to make the\
corresponding program executable; $ make encode, $ make decrypt. In order to format all files run\
$ make format.

## Cleaning:
To clean the directory of all executable run the command $ make clean.

## Run:
Once you have made the executables that you want to run use the command, $ ./(executable) 'argument(s)'.

## Available Command Line Arguments for Encode Executable:
• -v : Print compression statistics to stderr.\
• -i (input) : Specify input to compress (stdin by default)\
• -o (output) : Specify output of compressed input (stdout by default)

## Available Command Line Arguments for Decode Executable:
• -v : Print decompression statistics to stderr.\
• -i (input) : Specify input to decompress (stdin by default)\
• -o (output) : Specify output of decompressed input (stdout by default)


## Scan-Build False Positives:
Running $scan-build make, generates one error message saying that log2 function in encode.c\
is not defined. However the math library is included in the top of encode.c and it has full funcitonality\
so this error is wrong.

