#!/usr/bin/env bash

cmake -B build
cmake --build build -j

# Run all testcases. 
# You can comment some lines to disable the run of specific examples.
mkdir -p output

for idx in "$@"; do
	if [ -f "testcases/scene${idx}.txt" ]; then
		build/PA4 testcases/scene${idx}.txt output/scene${idx}.bmp
	else
		echo "File testcases/scene${idx}.txt does not exist, skipping."
	fi
done

