rm -rf ./build
rm -rf ./output
mkdir ./build
mkdir ./output
mkdir ./output/compressed
mkdir ./output/decompressed
echo "---Compiling Compressor---"
g++ -O2 ./src/compressor.cpp -o ./build/compressor
if [ $? -eq 0 ]
then
	echo "---Compiling Deompressor---"
	g++ -O2 ./src/decompressor.cpp -o ./build/decompressor
	for trs in 0 1 2 3 4 5 6 7 8 9 10 20 40 80 200 500
	do
		./build/compressor ./testcase/2.hamlet.txt ./output/compressed/2.hamlet.$trs.txt.huff -w $trs > /dev/null
		./build/decompressor ./output/compressed/2.hamlet.$trs.txt.huff ./output/decompressed/2.hamlet.$trs.txt > /dev/null

		diff ./testcase/2.hamlet.txt ./output/decompressed/2.hamlet.$trs.txt > /dev/null
		if [ $? -eq 0 ]
		then
			echo "Threshold $trs Passed"
			python3 src/utils/calc_compress_radio.py ./testcase/2.hamlet.txt ./output/compressed/2.hamlet.$trs.txt.huff
		else
			echo "Threshold $trs Failed"
		fi
	done
	./build/compressor ./testcase/2.hamlet.txt ./output/compressed/2.hamlet.inf.txt.huff > /dev/null
	./build/decompressor ./output/compressed/2.hamlet.inf.txt.huff ./output/decompressed/2.hamlet.inf.txt > /dev/null

	diff ./testcase/2.hamlet.txt ./output/decompressed/2.hamlet.inf.txt > /dev/null
	if [ $? -eq 0 ]
	then
		echo "Threshold inf Passed"
		python3 src/utils/calc_compress_radio.py ./testcase/2.hamlet.txt ./output/compressed/2.hamlet.inf.txt.huff
	else
		echo "Threshold inf Failed"
	fi
else
	echo "Compile error."
fi
