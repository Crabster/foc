SHELL = /bin/bash

.ONESHELL:

all:
	mkdir -p build
	pushd build
	cmake ..
	make 
	popd 

clean:
	rm -rf build
