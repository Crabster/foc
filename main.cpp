#include <iostream>
#include "FocLexer.h"
#include "src/CodeVisitor.hpp"

int main() {
    std::ifstream stream;
    stream.open("example.foc");
    
    antlr4::ANTLRInputStream input(stream);
    foc::FocLexer lexer(&input);
    antlr4::CommonTokenStream tokens(&lexer);
    foc::FocParser parser(&tokens);    
    foc::FocParser::ProgramContext* tree = parser.program();
    foc::CodeVisitor visitor;
    visitor.visitProgram(tree);
}
