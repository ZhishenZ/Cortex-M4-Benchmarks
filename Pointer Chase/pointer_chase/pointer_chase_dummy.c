#include <stdlib.h>
#include <stdio.h>

// for (i = 0; < n; += step) {     // device pointers 32-bit on gt200.     

// h_a[i] = ((unsigned int)(uintptr_t)d_a) + ((i + stride) % n)*sizeof(unsigned int); }
const void * const pointers[10] = {&pointers[1], &pointers[2], &pointers[3], &pointers[4], &pointers[5], &pointers[6], &pointers[7], &pointers[8], &pointers[9], NULL };
//const void ** const p0 = &pointers[0];
const void * const p0 = &pointers[0];
int main(){


void **this_pp = p0;// this is a pointer of a pointer 

// while(this_pointer){
//     this_pointer = &pointers[i];
// }
while(this_pp){
    this_pp = *this_pp;  
}

return 0;

}