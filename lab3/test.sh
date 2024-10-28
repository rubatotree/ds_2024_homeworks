rm -rf ./build
rm -rf ./output
mkdir ./build
mkdir ./output
mkdir ./output/compressed
mkdir ./output/decompressed
gcc -O2 ./src/compressor.c -o ./build/compressor
gcc -O2 ./src/decompressor.c -o ./build/decompressor
for file in `ls ./testcase`
do
    ./build/compressor ./testcase/$file ./output/compressed/$file
    ./build/decompressor ./output/compressed/$file ./output/decompressed/$file
    diff ./testcase/$file ./output/decompressed/$file
    if [ $? -eq 0 ]
    then
        echo "$file: Passed"
    else
        echo "$file: Failed"
    fi
done