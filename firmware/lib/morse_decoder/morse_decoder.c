#include <string.h>
#include <stdio.h>
#include "morse_decoder.h"

char decode_morse(const char *morse)
{
    printf("Decoding Morse: %s\n", morse);

    // Letters
    if (strcmp(morse, "._") == 0)
        return 'A';
    if (strcmp(morse, "_...") == 0)
        return 'B';
    if (strcmp(morse, "_._.") == 0)
        return 'C';
    if (strcmp(morse, "_..") == 0)
        return 'D';
    if (strcmp(morse, ".") == 0)
        return 'E';
    if (strcmp(morse, ".._.") == 0)
        return 'F';
    if (strcmp(morse, "__.") == 0)
        return 'G';
    if (strcmp(morse, "....") == 0)
        return 'H';
    if (strcmp(morse, "..") == 0)
        return 'I';
    if (strcmp(morse, ".___") == 0)
        return 'J';
    if (strcmp(morse, "_._") == 0)
        return 'K';
    if (strcmp(morse, "._..") == 0)
        return 'L';
    if (strcmp(morse, "__") == 0)
        return 'M';
    if (strcmp(morse, "_.") == 0)
        return 'N';
    if (strcmp(morse, "___") == 0)
        return 'O';
    if (strcmp(morse, ".__.") == 0)
        return 'P';
    if (strcmp(morse, "__._") == 0)
        return 'Q';
    if (strcmp(morse, "._.") == 0)
        return 'R';
    if (strcmp(morse, "...") == 0)
        return 'S';
    if (strcmp(morse, "_") == 0)
        return 'T';
    if (strcmp(morse, ".._") == 0)
        return 'U';
    if (strcmp(morse, "..._") == 0)
        return 'V';
    if (strcmp(morse, ".__") == 0)
        return 'W';
    if (strcmp(morse, "_.._") == 0)
        return 'X';
    if (strcmp(morse, "_.__") == 0)
        return 'Y';
    if (strcmp(morse, "__..") == 0)
        return 'Z';

    // Numbers
    if (strcmp(morse, "_____") == 0)
        return '0';
    if (strcmp(morse, ".____") == 0)
        return '1';
    if (strcmp(morse, "..___") == 0)
        return '2';
    if (strcmp(morse, "...__") == 0)
        return '3';
    if (strcmp(morse, "...._") == 0)
        return '4';
    if (strcmp(morse, ".....") == 0)
        return '5';
    if (strcmp(morse, "_....") == 0)
        return '6';
    if (strcmp(morse, "__...") == 0)
        return '7';
    if (strcmp(morse, "___..") == 0)
        return '8';
    if (strcmp(morse, "____.") == 0)
        return '9';

    // Special Characters
    if (strcmp(morse, "..__") == 0)
        return ' '; // SPACE
    if (strcmp(morse, "._._") == 0)
        return '\n'; // ENTER

    return '?'; // Unknown Morse code
}
