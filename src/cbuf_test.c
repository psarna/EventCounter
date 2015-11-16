#include <stdio.h>

#include "cbuf.h"

int main()
{
    char c, eof;
    struct cbuf<char, 8> buf;

    for (;;) {
        scanf("%c", &c);
        scanf("%c", &eof);
        switch (c) {
        case 'x':
            if (!buf.empty()) {
                printf("extr[%c]\n", buf.get());
            } else {
                printf("empty\n");
            }
            break;
        default:
            if (!buf.full()) {
                buf.put(c);
            } else {
                printf("full\n");
            }
        }
    }
}

