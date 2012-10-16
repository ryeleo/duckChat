#include <stdio.h>

#include "duckchat.h"
#include "raw.h"

int main(int argc, char *argv[]){
    char buffer[32];    
    raw_mode();
    scanf("%s", buffer);
    printf("this my shit: %s\n", buffer);
    cooked_mode();  
}
