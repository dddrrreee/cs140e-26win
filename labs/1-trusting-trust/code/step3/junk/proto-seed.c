int i;

// Q: why can't we just print prog twice?
printf("char prog[] = {\n");
for(i = 0; prog[i]; i++)
    printf("\t%d,%c", prog[i], (i+1)%8==0 ? '\n' : ' ');
printf("0 };\n");
printf("%s", prog);
return 0;
