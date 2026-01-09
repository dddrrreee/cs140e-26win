gcc attack-char-gen.c -o attack-char-gen # exe that makes bytes

./attack-char-gen < trojan-compiler2.c > attack-arr.c # makes bytes out of trojan-compiler

cp trojan-compiler2.c compiler.c # Now can compiler trojan compiler (since it includes attack-arr.c)

gcc compiler.c -o compiler

./compiler compiler.c -o compiler

./compiler ../step2/login.c -o login

./login




