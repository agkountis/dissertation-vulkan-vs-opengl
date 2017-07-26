#!/bin/bash

for file in $(ls $2/*.????)
do
    echo Compiling -- ${file} ...
	$1/glslc --target-env=vulkan -o ${file}.spv ${file}
done
