#include "rpi.h"
void undefined_instruction_vector(unsigned pc) { 
    panic("unhandled undefined inst exception: pc=%x [inst=%x]\n", pc, *(uint32_t*)pc);
}
