#include "Interpreter.h"
#include <iostream>
#include <string>
#include <vector>
#include <typeinfo>
#include "Dispatch.h"

using namespace std;



ASTValue * Interpreter::visit ( ASTInteger * integer)
{
    return integer;
}

ASTValue * Interpreter::visit ( ASTBoolean * boolean)
{
    return boolean;
}

ASTValue * Interpreter::visit ( ASTNull * null)
{
    return null;
}

ASTValue * Interpreter::visit ( ASTVariable * variable)
{
    ASTValue * val = variable->m_value->Accept(*this);

    m_stack.CreateVar(variable->m_name,val);

    return val;
}

ASTValue * Interpreter::visit ( ASTAccessVariable * accessVar)
{
    return m_stack.GetValue(accessVar->m_name);
}

ASTValue * Interpreter::visit ( ASTAssignVariable * assignVar)
{
    ASTValue * val = assignVar->m_value->Accept(*this);
    m_stack.AssignValue(assignVar->m_name,val);
    return val;
}

ASTValue * Interpreter::visit ( ASTFunction * fun)
{
    m_stack.CreateFun(fun->m_name,fun);
    return new ASTNull();
}

ASTValue * Interpreter::visit ( ASTCallFunction * callFun)
{
    ASTFunction * fun = m_stack.GetFun(callFun->m_name);

    if (fun->m_param.size() != callFun->m_arg.size())
        throw InterpreterException("Error : different number of arguments and parameters in function call.");

    vector<ASTValue*> arg;

    for (uint i = 0; i < fun->m_param.size(); i++)
    {
        arg.push_back(callFun->m_arg[i]->Accept(*this));
    }

    m_stack.PushStack(true);

    for (uint i = 0; i < arg.size(); i++)
    {
        m_stack.CreateVar(fun->m_param[i],arg[i]);
    }


    ASTValue * val = fun->m_body->Accept(*this);

    m_stack.PopStack();
    return val;
}

ASTValue * Interpreter::visit ( ASTPrint * print)
{
    uint cur_arg = 0;
    for (uint i = 0; i < print->m_format.size(); i++)
    {
        if (print->m_format[i] == '~')
        {
            if (cur_arg == print->m_arg.size())
                throw InterpreterException("Error : Wrong number of arguments in print.");
            string s =  print->m_arg[cur_arg++]->Accept(*this)->Print();
            cout << s;
        }
        else if (print->m_format[i] == '\\')
        {
            if (++i == print->m_format.size())
                throw InterpreterException("Error : Wrong format in print.");
            switch (print->m_format[i]){
                case '~':
                    cout << '~';
                    break;
                case 'n':
                    cout << endl;
                    break;
                case '"':
                    cout << "\"";
                    break;
                case 'r':
                    cout << "\r";
                    break;
                case 't':
                    cout << "\t";
                    break;
                case '\\':
                    cout << "\\";
                default:
                    throw InterpreterException("Error : Wrong symbol after backslash in print.");
            }
        }
        else
            cout << print->m_format[i];
    }

    return new ASTNull();
}

ASTValue * Interpreter::visit ( ASTBlock * block)
{
    if (!block->m_stmt.empty())
    {
        m_stack.PushStack();
        ASTValue * ret = nullptr;
        for (uint i = 0; i < block->m_stmt.size(); i++)
        {
            ret = block->m_stmt[i]->Accept(*this);
        }
        m_stack.PopStack();
        return ret;
    }
    return new ASTNull();
}

ASTValue * Interpreter::visit ( ASTTop * top)
{
    ASTValue * ret = nullptr;
    for (uint i = 0; i < top->m_stmt.size(); i++)
    {
        ret = top->m_stmt[i]->Accept(*this);
    }
    return ret;
}

ASTValue * Interpreter::visit ( ASTLoop * loop)
{
    AST::Type type = loop->m_cond->Accept(*this)->Type();

    while(type != AST::Type::Null && type != AST::Type::BoolFalse)
    {
        loop->m_body->Accept(*this);
        type = loop->m_cond->Accept(*this)->Type();
    }
    return new ASTNull();
}

ASTValue * Interpreter::visit ( ASTConditional * cond)
{

    AST::Type type = cond->m_cond->Accept(*this)->Type();

    if (type != AST::Type::Null && type != AST::Type::BoolFalse)
        return cond->m_if->Accept(*this);

    return cond->m_else->Accept(*this);
}


ASTValue * Interpreter::visit ( ASTArray * arr)
{
    ASTValue * ast = arr->m_size->Accept(*this);

    if (ast->Type() != AST::Type::Integer)
        throw InterpreterException("Error : Size of array has to be integer.");
    int size = reinterpret_cast<ASTInteger*>(ast)->m_value;

    for (int i = 0; i < size; i++)
    {
        m_stack.PushStack();
        ASTValue * val = arr->m_value->Accept(*this);
        arr->m_elements.push_back(val);
        m_stack.PopStack();
    }
    arr->m_heap_ref = -1;

    return arr;
}

ASTValue * Interpreter::visit ( ASTAssignArray * assignArr)
{

    ASTValue * astarr = assignArr->m_array->Accept(*this);
    if (astarr->Type() != AST::Type::Array)
        throw InterpreterException("Error : Wrong return type.");
    ASTArray * arr = reinterpret_cast<ASTArray*>(astarr);

    ASTValue * astInt = assignArr->m_index->Accept(*this);
    if (astInt->Type() != AST::Type::Integer)
        throw InterpreterException("Error : Wrong return type.");
    ASTInteger * index = reinterpret_cast<ASTInteger*>(astInt);

    ASTValue * value = assignArr->m_value->Accept(*this);

    if (value->Type() == AST::Type::Array)
        throw InterpreterException("Error : Wrong type of array element.");

    m_stack.AssignArrayElement(arr->m_heap_ref + 2 + index->m_value * 20,value);

    return value;
}



ASTValue * Interpreter::visit ( ASTAccessArray * accessArr)
{

    ASTValue * astarr = accessArr->m_array->Accept(*this);
    if (astarr->Type() != AST::Type::Array)
        throw InterpreterException("Error : Wrong return type.");
    ASTArray * arr = reinterpret_cast<ASTArray*>(astarr);

    ASTValue * astInt = accessArr->m_index->Accept(*this);
    if (astInt->Type() != AST::Type::Integer)
        throw InterpreterException("Error : Wrong return type.");
    ASTInteger * index = reinterpret_cast<ASTInteger*>(astInt);

    return arr->m_elements[index->m_value];
}




ASTValue * Interpreter::visit ( ASTObject * obj)
{
    obj->m_prev = obj->m_extends->Accept(*this);

    m_stack.PushStack();

    for (uint i = 0; i < obj->m_members.size(); i++)
    {
        obj->m_members[i]->Accept(*this);
    }
    obj->m_stack = reinterpret_cast<void*>(m_stack.GetPopStack());

    return obj;
}


ASTValue * Interpreter::visit ( ASTAssignField * assignFie)
{
    if (typeid(*assignFie->m_object) == typeid(ASTAccessVariable))
    {
        ASTAccessVariable * var = reinterpret_cast<ASTAccessVariable*>(assignFie->m_object);
        if (var->m_name == "this")
        {
            ASTValue * val = assignFie->m_value->Accept(*this);
            m_stack.AssignValue(assignFie->m_field,val);
            return val;
        }
    } 

    ASTValue * val = assignFie->m_value->Accept(*this);

    ASTValue * v = assignFie->m_object->Accept(*this);

    if (v->Type() != AST::Type::Object)
        throw InterpreterException("Error : Wrong return type.");
    ASTObject * obj = reinterpret_cast<ASTObject*>(v);

    m_stack.AssignValue(assignFie->m_field,val,reinterpret_cast<Stack*>(obj->m_stack));

    return val;
}

ASTValue * Interpreter::visit ( ASTAccessField * accessFie)
{
    if (typeid(*accessFie->m_object) == typeid(ASTAccessVariable))
    {
        ASTAccessVariable * var = reinterpret_cast<ASTAccessVariable*>(accessFie->m_object);
        if (var->m_name == "this")
        {
            return m_stack.GetValue(accessFie->m_field);
        }
    } 

    ASTValue * val = accessFie->m_object->Accept(*this);
    if (val->Type() != AST::Type::Object)
        throw InterpreterException("Error : Wrong return type.");
    ASTObject * obj = reinterpret_cast<ASTObject*>(val);

    return m_stack.GetValue(accessFie->m_field,reinterpret_cast<Stack*>(obj->m_stack));
}

ASTValue * Interpreter::visit ( ASTCallMethod * call)
{
    ASTFunction * fun = nullptr;
    if (typeid(*call->m_object) == typeid(ASTAccessVariable) &&
        reinterpret_cast<ASTAccessVariable*>(call->m_object)->m_name == "this")
    {
        fun = m_stack.GetFun(call->m_name);

        vector<ASTValue*> arg;

        for (uint i = 0; i < call->m_arg.size(); i++)    
        {
            arg.push_back(call->m_arg[i]->Accept(*this));
        }
        
        m_stack.PushStack();
    
    
        for (uint i = 0; i < fun->m_param.size(); i++)    
        {
           m_stack.CreateVar(fun->m_param[i],arg[i]);
        }
    
    
        ASTValue * val = fun->m_body->Accept(*this);
    
        m_stack.PopStack();
    
        return val;
    } 

    ASTValue * v = call->m_object->Accept(*this);

    if (v->Type() != AST::Type::Object)
    {
        if (call->m_arg.size() != 1)
            throw InterpreterException("Error : Wrong number of arguments.");
        ASTValue * rv = call->m_arg[0]->Accept(*this);

        switch (v->Type()){
            case AST::Type::Integer:
                return Dispatch(reinterpret_cast<ASTInteger*>(v),call,rv);
                break;
            case AST::Type::Null:
                return Dispatch(reinterpret_cast<ASTNull*>(v),call,rv);
                break;
            case AST::Type::BoolTrue:
            case AST::Type::BoolFalse:
                return Dispatch(reinterpret_cast<ASTBoolean*>(v),call,rv);
                break;
            default:
                break;

        };
    }

    ASTObject * obj = reinterpret_cast<ASTObject*>(v);
    Stack * stack = reinterpret_cast<Stack*>(obj->m_stack);
    if (stack->m_ref.find(call->m_name) == stack->m_ref.end())
    {
        return obj->m_prev->Accept(*this);
    }

    fun = m_stack.GetFunMethod(call->m_name,stack);



    if (fun->m_param.size() != call->m_arg.size())
        throw InterpreterException("Error : different number of arguments and parameters in method call.");


    vector<ASTValue*> arg;

    for (uint i = 0; i < call->m_arg.size(); i++)    
    {
        arg.push_back(call->m_arg[i]->Accept(*this));
    }
    
    m_stack.PushStackMethod(stack);


    for (uint i = 0; i < fun->m_param.size(); i++)    
    {
       m_stack.CreateVar(fun->m_param[i],arg[i]);
    }


    ASTValue * val = fun->m_body->Accept(*this);

    m_stack.PopStackMethod();

    return val;
}

