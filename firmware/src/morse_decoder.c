#include <string.h>
#include <stdio.h>
#include "morse_decoder.h"

char decode_morse(const char *morse)
{
    printf("Decoding Morse: %s\n", morse);

    // Letters
    if (strcmp(morse, "._") == 0)
        return 'a';
    if (strcmp(morse, "_...") == 0)
        return 'b';
    if (strcmp(morse, "_._.") == 0)
        return 'c';
    if (strcmp(morse, "_..") == 0)
        return 'd';
    if (strcmp(morse, ".") == 0)
        return 'e';
    if (strcmp(morse, ".._.") == 0)
        return 'f';
    if (strcmp(morse, "__.") == 0)
        return 'g';
    if (strcmp(morse, "....") == 0)
        return 'h';
    if (strcmp(morse, "..") == 0)
        return 'i';
    if (strcmp(morse, ".___") == 0)
        return 'j';
    if (strcmp(morse, "_._") == 0)
        return 'k';
    if (strcmp(morse, "._..") == 0)
        return 'l';
    if (strcmp(morse, "__") == 0)
        return 'm';
    if (strcmp(morse, "_.") == 0)
        return 'n';
    if (strcmp(morse, "___") == 0)
        return 'o';
    if (strcmp(morse, ".__.") == 0)
        return 'p';
    if (strcmp(morse, "__._") == 0)
        return 'q';
    if (strcmp(morse, "._.") == 0)
        return 'r';
    if (strcmp(morse, "...") == 0)
        return 's';
    if (strcmp(morse, "_") == 0)
        return 't';
    if (strcmp(morse, ".._") == 0)
        return 'u';
    if (strcmp(morse, "..._") == 0)
        return 'v';
    if (strcmp(morse, ".__") == 0)
        return 'w';
    if (strcmp(morse, "_.._") == 0)
        return 'x';
    if (strcmp(morse, "_.__") == 0)
        return 'y';
    if (strcmp(morse, "__..") == 0)
        return 'z';

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
