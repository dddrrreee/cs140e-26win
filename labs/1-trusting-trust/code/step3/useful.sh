# Want to make attack.c into an array in some file
cd ../step1
make quine-gen
cd ../step3
gcc attack-arr-gen.c -o attack-arr-gen # the payload we want to keep replicating
./attack-arr-gen < payload.c > payload-bytes.c

sed -i '' 's/prog/payload/g' payload-bytes.c
# cat payload-bytes.c proto-seed.c > attack-seed.c
cat payload-bytes.c payload.c > attack-seed.c
cat payload.c > attack-seed.c

# manually put payload-bytes.c array at the beginning of the seed

# ../step1/quine-gen < attack-seed.c > attack-quine.c # makes bytes out of trojan-compiler


# gcc trojan-compiler2.c -o trojan-compiler2

# ./compiler compiler.c -o compiler

# ./compiler ../step2/login.c -o login

# ./login









# # Want to make attack.c into an array in some file
# cd ../step1
# make quine-gen
# cd ../step3
# gcc attack-arr-gen.c -o attack-arr-gen # the payload we want to keep replicating
# ./attack-arr-gen < payload.c > payload-bytes.c

# sed -i '' 's/prog/payload/g' payload-bytes.c
# # cat payload-bytes.c proto-seed.c > attack-seed.c
# cat payload-bytes.c > attack-seed.c

# # manually put payload-bytes.c array at the beginning of the seed

# ../step1/quine-gen < attack-seed.c > attack-quine.c # makes bytes out of trojan-compiler


# # gcc trojan-compiler2.c -o trojan-compiler2

# # ./compiler compiler.c -o compiler

# # ./compiler ../step2/login.c -o login

# # ./login




