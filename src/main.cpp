
#include <fstream>
#include <iostream>
#include "AST.h"
#include "JSONToAST.h"
#include "Interpreter.h"

using namespace std;

using json = nlohmann::json;


int main(int argc, char** argv)
{
  if (argc != 2){
    cout << "Wrong number of arguments" << endl;
    return 0;
  }

  ifstream file(argv[1]);

  JSONToAST converter;
  AST * ast = nullptr;

  try{
    ast = converter.convert(file);
  }catch(ParserException& e){
    cout << e.what() << endl;
    return 0;
  }

  Interpreter eval;

  try{
    ast->Accept(eval);
  }catch (InterpreterException& e )
  {
    cout << e.what() << endl;
  }catch(exception& e){
    cout << e.what() << endl;
  }
  delete ast;

  return 0;
}
