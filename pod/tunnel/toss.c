#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

static char *
read_stdin (void)
{
    size_t cap = 4096, /* Initial capacity for the char buffer */
           len =    0; /* Current offset of the buffer */
    char *buffer = malloc(cap * sizeof (char));
    int c;

    /* Read char by char, breaking if we reach EOF or a newline */
    while ((c = fgetc(stdin)) != '\n' && !feof(stdin)) {
        buffer[len] = c;

        /* When cap == len, we need to resize the buffer
         * so that we don't overwrite any bytes
         */
        if (++len == cap)
            /* Make the output buffer twice its current size */
            buffer = realloc(buffer, (cap *= 2) * sizeof (char));
    }

    /* Trim off any unused bytes from the buffer */
    buffer = realloc(buffer, (len + 1) * sizeof (char));

    /* Pad the last byte so we don't overread the buffer in the future */
    buffer[len] = '\0';

    return buffer;
}

int
main (int argc, char **argv)
{
    char *input = read_stdin();

    ssize_t dist = (ssize_t) atoi(input);

    int fd = open ( argv[1], O_WRONLY );

    write ( fd, &dist, sizeof (dist));

    free(input); /* Don't forget to free the memory allocated with malloc */

    close (fd);

    return 0;
}

