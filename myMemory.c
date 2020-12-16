#include "myMemory.h"


char* b_start;
char* b_end;

void getBuffer(){
    b_start = (char *)malloc(3*1024*1024);
    b_end = b_start + 3*1024*1024;
}