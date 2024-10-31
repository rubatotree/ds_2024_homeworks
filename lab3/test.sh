rm -rf ./build
rm -rf ./output
mkdir ./build
mkdir ./output
mkdir ./output/compressed
mkdir ./output/decompressed
g++ -O2 ./src/compressor.cpp -o ./build/compressor
g++ -O2 ./src/decompressor.cpp -o ./build/decompressor
for file in `ls ./testcase`
do
    ./build/compressor ./testcase/$file ./output/compressed/$file.huff > /dev/null
    ./build/decompressor ./output/compressed/$file.huff ./output/decompressed/$file > /dev/null
    diff ./testcase/$file ./output/decompressed/$file > /dev/null
    if [ $? -eq 0 ]
    then
        echo "$file: Passed"
    else
        echo "$file: Failed"
	fi
    python3 src/utils/calc_compress_radio.py ./testcase/$file ./output/compressed/$file.huff 
done
