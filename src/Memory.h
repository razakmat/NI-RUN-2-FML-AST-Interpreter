#ifndef MEMORY_H
#define MEMORY_H


#include <unordered_map>
#include <string>
#include <vector>

#include "AST.h"

using namespace std;

class Heap
{
    public:
        int GetCurrent();
        void AssignToHeap(ASTValue * val);
        void AssignToSpecific(int ref,ASTValue * val);
        void AssignToHeapFun(ASTFunction * val);
        ASTValue * GetFromHeap(int ref);
        ASTFunction * GetFromHeapFun(int ref);
    private:
        void SavePointer(const void * val);
        vector<unsigned char> m_data;
        int m_current = 0;

};

class Stack
{
    public:
        unordered_map<string, int> m_ref;
        Stack * m_prev = nullptr;
        bool m_fun;

};

class Env
{
    public:
                Env();
                ~Env();
        void    CreateVar(const string & name, ASTValue * val);
        void    AssignValue(const string & name, ASTValue * val,Stack * now = nullptr);
        ASTValue * GetValue(const string & name, Stack * now = nullptr);
        void AssignArrayElement(int heap_ref, ASTValue * val);
        ASTValue * AccessArrayElement(int heap_ref);
        ASTFunction * GetFun(const string & name);
        void    CreateFun(const string name, ASTFunction * fun);
        void    PushStack(bool fun = false);
        void    PopStack();
        void    PushStackMethod(Stack * stack);
        void    PopStackMethod();
        Stack * GetPopStack();
    private:
        unordered_map<string, int> m_refFun;
        Heap * m_heap;
        Stack * m_stack;
        Stack * m_global_stack;
        Stack * m_saveMethod;

};



#endif