// convert the contents of stdin to their ASCII values (e.g., 
// '\n' = 10) and spit out the <prog> array used in Figure 1 in
// Thompson's paper.
#include <stdio.h>

char buf[500];

int main(void) { 

    // Beginning of array
    printf("char compile_attack[] = {\n");

    char ch = getchar();
    int read_index = 0;


    while (ch != EOF) {
        buf[read_index] = ch;

        // Need to print with 8 characters on a line
        // printf("\t%d,\n", ch);
        printf("\t%d,%c", ch, (read_index+1)%8==0 ? '\n' : ' ');
        ch = getchar();
        read_index++;
    }
    printf("0 };\n");

    // Index located at max index now
    int print_index = 0;
    while (print_index < read_index) {
        printf("%c", buf[print_index]);
        print_index++;
    }
    

	return 0;
}
