#ifndef DISPATCH_H
#define DISPATCH_H


#include <unordered_map>
#include <string>
#include <vector>

#include "AST.h"

using namespace std;


ASTValue * Dispatch(ASTInteger * integer, ASTCallMethod * call, ASTValue * rv);

ASTValue * Dispatch(ASTBoolean * b, ASTCallMethod * call, ASTValue * rv);

ASTValue * Dispatch(ASTNull * nul, ASTCallMethod * call, ASTValue * rv);
#endif