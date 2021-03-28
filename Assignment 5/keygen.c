#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

int main (int argc, char **argv) {
    time_t t;
    int length = atoi(argv[1]);
    char key[length + 1];

    // Use current time as seed for random generator.  
    srand((unsigned) time(&t));

    // Generating random capital letters and spaces.
    for (int x = 0; x < length; x++) {
        int random = rand() % 27; // Random number between 0-26.

        if (random < 26) // If the random number is between 0-25, it's a letter.
            key[x] = (char) (random + 65);
        else if (random == 26) // If the random number is 26, it's a space.
            key[x] = ' ';
    }
    
    key[length] = '\n';

    fprintf(stdout, "%s", key);

    return 0;
}