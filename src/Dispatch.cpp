#include "Dispatch.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>

using namespace std;


ASTValue * Dispatch(ASTInteger * integer, ASTCallMethod * call, ASTValue * rv)
{
    if (call->m_name == "==")
    {
        if (rv->Type() != AST::Type::Integer)
            return new ASTBoolean(false);
    }
    if (call->m_name == "!=")
    {
        if (rv->Type() != AST::Type::Integer)
            return new ASTBoolean(true);
    }
    if (rv->Type() != AST::Type::Integer)
        throw runtime_error("Error : Type was not Integer.");
    ASTInteger * rh = reinterpret_cast<ASTInteger*>(rv);

    if (call->m_name == "+")
    {
        return new ASTInteger(rh->m_value + integer->m_value);
    }
    if (call->m_name == "-")
    {
        return new ASTInteger(integer->m_value - rh->m_value);
    }
    if (call->m_name == "*")
    {
        return new ASTInteger(integer->m_value * rh->m_value);
    }
    if (call->m_name == "/")
    {
        return new ASTInteger(integer->m_value / rh->m_value);
    }
    if (call->m_name == "%")
    {
        return new ASTInteger(integer->m_value % rh->m_value);
    }
    if (call->m_name == "<=")
    {
        return new ASTBoolean(integer->m_value <= rh->m_value);
    }
    if (call->m_name == ">=")
    {
        return new ASTBoolean(integer->m_value >= rh->m_value);
    }
    if (call->m_name == "<")
    {
        return new ASTBoolean(integer->m_value < rh->m_value);
    }
    if (call->m_name == ">")
    {
        return new ASTBoolean(integer->m_value > rh->m_value);
    }
    if (call->m_name == "==")
    {
        return new ASTBoolean(integer->m_value == rh->m_value);
    }
    if (call->m_name == "!=")
    {
        return new ASTBoolean(integer->m_value != rh->m_value);
    }  
    throw runtime_error("Error : Unknown method name with integers.");

}

ASTValue * Dispatch(ASTBoolean * b, ASTCallMethod * call, ASTValue *rv)
{
    if (call->m_name == "==")
    {
        if (rv->Type() != AST::Type::BoolTrue &&
            rv->Type() != AST::Type::BoolFalse)
            return new ASTBoolean(false);
    }
    if (call->m_name == "!=")
    {
        if (rv->Type() != AST::Type::BoolTrue &&
            rv->Type() != AST::Type::BoolFalse)
            return new ASTBoolean(true);
    }
    if (rv->Type() != AST::Type::BoolTrue &&
        rv->Type() != AST::Type::BoolFalse)
        throw runtime_error("Error : Type was not Boolean.");

    ASTBoolean * rh = reinterpret_cast<ASTBoolean*>(rv);

    if (call->m_name == "&")
    {
        return new ASTBoolean(rh->m_value && b->m_value);
    }
    if (call->m_name == "|")
    {
        return new ASTBoolean(b->m_value || rh->m_value);
    }
    if (call->m_name == "==")
    {
        return new ASTBoolean(b->m_value == rh->m_value);
    }
    if (call->m_name == "!=")
    {
        return new ASTBoolean(b->m_value != rh->m_value);
    }  
    throw runtime_error("Error : Unknown method name with booleans.");

}

ASTValue * Dispatch(ASTNull * nul, ASTCallMethod * call, ASTValue *rv)
{

    if (call->m_name == "==")
    {
        if (rv->Type() == AST::Type::Null)
            return new ASTBoolean(true);
        return new ASTBoolean(false);
    }
    if (call->m_name == "!=")
    {
        if (rv->Type() != AST::Type::Null)
            return new ASTBoolean(false);
        return new ASTBoolean(true);
    }


    return new ASTNull();
}
