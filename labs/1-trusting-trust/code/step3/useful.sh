gcc attack-char-gen.c -o attack-char-gen # exe that makes bytes

./attack-char-gen < trojan-compiler2.c > attack-arr.c # makes bytes out of trojan-compiler

gcc trojan-compiler2.c -o compiler # Now can compiler trojan compiler (since it includes attack-arr.c)

gcc compiler.c -o compiler

./compiler compiler.c -o compiler
./compiler compiler.c -o compiler
./compiler compiler.c -o compiler
./compiler compiler.c -o compiler
./compiler compiler.c -o compiler
./compiler compiler.c -o compiler




