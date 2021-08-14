# Functionally Oriented C

### Compilation

For compilation with docker it should be sufficient to run

```bash
$ docker build . -t foc
```

To compile any file in your `pwd`, you can use script `util/foc.sh`:

```bash
# Usage: ./util/foc.sh source destination
$ ./util/foc.sh examples/hello_world.foc hello_world
```

### GRUN

To install grun for debugging syntax trees, run commands below:

```bash
$ cd /usr/local/lib
$ sudo curl -O https://www.antlr.org/download/antlr-4.9.1-complete.jar
$ export CLASSPATH=".:/usr/local/lib/antlr-4.9.1-complete.jar:$CLASSPATH"
$ alias antlr4='java -jar /usr/local/lib/antlr-4.9.1-complete.jar'
$ alias grun='java org.antlr.v4.gui.TestRig'
```

