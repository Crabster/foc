#!/bin/bash

antlr4() {
    java -jar /usr/local/lib/antlr-4.9.1-complete.jar "$@"
}
grun() {
    java org.antlr.v4.gui.TestRig "$@"
}

mkdir -p build/grun
antlr4 Foc.g4 -o build/grun
pushd build/grun > /dev/null
env | grep CLASSPATH
javac Foc*.java 
cat | grun Foc program -gui
popd > /dev/null
