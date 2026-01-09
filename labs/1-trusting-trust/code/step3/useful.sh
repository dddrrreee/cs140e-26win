gcc quine-gen.c -o quine-gen # exe that makes bytes

./quine-gen < attack-seed.c > trojan-compiler2.c # makes bytes out of trojan-compiler


gcc trojan-compiler2.c -o trojan-compiler2

./compiler compiler.c -o compiler

./compiler ../step2/login.c -o login

./login




