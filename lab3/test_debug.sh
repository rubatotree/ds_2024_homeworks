rm -rf ./build
rm -rf ./output
mkdir ./build
mkdir ./output
mkdir ./output/compressed
mkdir ./output/decompressed
gcc -O2 ./src/compressor.c -o ./build/compressor
gcc -O2 ./src/decompressor.c -o ./build/decompressor


echo "---Compressor Output---"
./build/compressor ./testcase/debug.txt ./output/compressed/debug.txt.huff
echo ""
echo "---Decompressor Output---"
./build/decompressor ./output/compressed/debug.txt.huff ./output/decompressed/debug.txt
echo ""
diff ./testcase/debug.txt ./output/decompressed/debug.txt
if [ $? -eq 0 ]
then
	echo "debug.txt: Passed"
else
	echo "debug.txt: Failed"
	echo "---Compressed---"
	cat ./output/compressed/debug.txt.huff
	echo ""
	echo "---Decompressed---"
	cat ./output/decompressed/debug.txt
	echo ""
fi
