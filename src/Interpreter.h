#ifndef INTERPRETER_H
#define INTERPRETER_H


#include "Visitor.h"
#include "AST.h"
#include "Memory.h"


using namespace std;


class InterpreterException : public exception
{
    public:
        InterpreterException(const string & msg) : m_msg(msg){}
    
        virtual const char* what() const noexcept override
        {
            return m_msg.c_str();
        }
    private:
        string m_msg;
}; 


class Interpreter : public Visitor
{
    public:
        ASTValue * visit ( ASTInteger * integer);
        ASTValue * visit ( ASTBoolean * boolean);
        ASTValue * visit ( ASTNull * null);
        ASTValue * visit ( ASTVariable * variable);
        ASTValue * visit ( ASTAccessVariable * accessVar);
        ASTValue * visit ( ASTAssignVariable * assignVar);
        ASTValue * visit ( ASTFunction * fun);
        ASTValue * visit ( ASTCallFunction * callFun);
        ASTValue * visit ( ASTPrint * print);
        ASTValue * visit ( ASTBlock * block);
        ASTValue * visit ( ASTTop * top);
        ASTValue * visit ( ASTLoop * loop);
        ASTValue * visit ( ASTConditional * cond);
        ASTValue * visit ( ASTObject * obj);
        ASTValue * visit ( ASTArray * arr);
        ASTValue * visit ( ASTAssignArray * assignArr);
        ASTValue * visit ( ASTAccessArray * accessArr);
        ASTValue * visit ( ASTAssignField * assignFie);
        ASTValue * visit ( ASTAccessField * accessFie);
        ASTValue * visit ( ASTCallMethod * call);
    private:
        Env m_stack;
};

#endif