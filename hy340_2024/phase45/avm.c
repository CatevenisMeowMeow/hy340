#include "avm.h"


int main(){
    avm_initialize();
    avm_load_instructions("target_code");
    printf("Starting avm execution...\n\n");
    unsigned int cycle_count = 0; //for debugging
    while(executionFinished != 1){
            printf("Cycle #%u: ",cycle_count);//for debugging
           // print_curr_instr(); //for debugging
            execute_cycle();
            cycle_count++;//for debugging
    }
    printf("\n\nexecution finished!\n");
    return 1;
}