
#include <stdlib.h>
#include <stdio.h>
#include "../include/pmc-get-row.h"

int main(int argc, char *argv[])
{
    char default_label[32] = "na";
    char* label;
    if (argc == 2) {
        label = argv[1];
    } else if (argc == 1) {
        label=&default_label[0];
    } else {
        printf("Usage: ./bin/pmc-get-pmcs [label]");
        exit(-1);
    }
    pmc_get_row(label);
    return 0;
}
