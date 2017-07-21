#!/bin/bash

for file in $(ls $1/*.????)
do
	/home/gordath/VulkanSDK/1.0.51.0/x86_64/bin/glslangValidator -V -o $file.spv $file
done

