#!/bin/bash

docker run -v `pwd`/$1:/main.foc -v `pwd`:/foc_local foc foc -i /main.foc -o /foc_local/$2
