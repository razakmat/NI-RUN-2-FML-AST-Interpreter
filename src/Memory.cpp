#include "Memory.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>
#include "JSONToAST.h"


using namespace std;


int Heap::GetCurrent()
{
    return m_current;
}

void Heap::SavePointer(const void * val)
{

    unsigned char bytes[sizeof(val)];
    copy(static_cast<const char*>(static_cast<const void*>(&val)),
              static_cast<const char*>(static_cast<const void*>(&val)) 
              + sizeof(val), bytes);

        m_data[m_current++] = sizeof(val);

        int now = m_current + sizeof(val)-1;

        for (uint i = 0; i < sizeof(val); i++){
            m_data[now--] = bytes[i];
        }
        m_current += sizeof(val);
}


void Heap::AssignToHeap(ASTValue * val)
{
    if ((uint)m_current + 100 > m_data.capacity() )
        m_data.resize(m_data.capacity() + 1000);

    AST::Type type = val->Type();
    m_data[m_current] = static_cast<unsigned char>(type);

    switch(type){
        case AST::Type::Null:
            m_current++;
            break;
        case AST::Type::Integer:
        {
            ASTInteger * v = reinterpret_cast<ASTInteger*>(val);
            m_data[m_current+1] = (v->m_value >> 24) & 0xFF;
            m_data[m_current+2] = (v->m_value >> 16) & 0xFF;
            m_data[m_current+3] = (v->m_value >> 8) & 0xFF;
            m_data[m_current+4] = v->m_value & 0xFF;
            m_current += 5;
            break;
        }
        case AST::Type::BoolTrue:
        {
            m_current ++;
            break;
        }
        case AST::Type::BoolFalse:
        {
            m_current ++;
            break;
        }
        case AST::Type::Array:
        {
            ASTArray * v = reinterpret_cast<ASTArray*>(val);
            v->m_heap_ref = m_current;
            m_data[m_current+1] = v->m_elements.size();

            m_current += 2;
            for (uint i = 0; i < v->m_elements.size(); i++)
            {
                int now = m_current;
                AssignToHeap(v->m_elements[i]);
                if (m_current > now + 5)
                    throw "Error : wrong type of element in array";
                now += 5;
                for (;m_current < now; m_current++)
                    m_data[m_current] = 0;
            }

            break;
        }
        case AST::Type::Object:
        {

            m_current++;
            ASTObject * v = reinterpret_cast<ASTObject*>(val);
            SavePointer(static_cast<const void*>(v->m_prev));
            SavePointer(static_cast<const void*>(v->m_stack));

            break;    
        }
        default:
            break;
    }
}

void Heap::AssignToSpecific(int ref, ASTValue * val)
{
    int now = m_current;
    m_current = ref;
    AssignToHeap(val);
    m_current = now;
}

void Heap::AssignToHeapFun(ASTFunction * val)
{
    vector<int> number;
    uint num = val->m_json.size();

    if ((uint)m_current + num + 100 > m_data.capacity() )
        m_data.resize(m_data.capacity() + 1000 + num);

    while (num != 0) 
    { 
        number.push_back(num%10);
        num /= 10;
    }
    m_data[m_current++] = 6;
    m_data[m_current++] = number.size();
    for (int i = (int)number.size() - 1; i >= 0; i--)
        m_data[m_current++] = number[i];
    for (uint i = 0; i < val->m_json.size(); i++)
        m_data[m_current++] = val->m_json[i];
}

ASTValue * Heap::GetFromHeap(int ref)
{
    int type = m_data[ref];

    switch (type){
        case 0:
        {
            return new ASTNull();
        }
        case 1:
        {
            ASTInteger * in = new ASTInteger((m_data[ref+1]<<24)|(m_data[ref+2]<<16)
                                    |(m_data[ref+3]<<8)|(m_data[ref+4]));
            return in;
        }
        case 2:
        {
            return new ASTBoolean(true);
        }
        case 3:
        {
            return new ASTBoolean(false);
        }
        case 4:
        {
            ASTArray * arr = new ASTArray();
            arr->m_heap_ref = ref;

            int size = m_data[ref+1];
            ref += 2;

            for (int i = 0 ; i < size; i++)
            {
                arr->m_elements.push_back(GetFromHeap(ref + i * 5));
            }

            return arr;
        }
        case 5:
        {
            ref++;
            ASTObject * obj = new ASTObject();
            int size = m_data[ref++];

            uint64_t pointer = 0;
            for(int i = size - 1; i >= 0; i--)
            {
                pointer <<= 8;
                pointer |= m_data[ref++];
            }

            obj->m_prev = reinterpret_cast<ASTValue*>(pointer);

            size = m_data[ref++];
            pointer = 0;
            for(int i = size - 1; i >= 0; i--)
            {
                pointer <<= 8;
                pointer |= m_data[ref++];
            }

            obj->m_stack = reinterpret_cast<void*>(pointer);
            return obj;

        }
    }

    throw runtime_error("Error : Something went wrong during retrival from heap.");
}

ASTFunction * Heap::GetFromHeapFun(int ref)
{
    uint sizeOfsize = m_data[ref+1];
    uint size = 0;

    for (uint i = ref + 2; i < ref + 2 + sizeOfsize; i++)
        size = size * 10 + m_data[i];

    string json;

    for(uint i = ref + 2 + sizeOfsize; i < ref + 2 + sizeOfsize + size; i++)
    {
        json.push_back(m_data[i]);
    }

    JSONToAST convert;
    AST * fun = convert.convertFun(json);

    return reinterpret_cast<ASTFunction*>(fun);
}


Env::Env()
: m_heap(new Heap())
{
    m_global_stack = new Stack();
    m_global_stack->m_prev = nullptr;
    m_stack = m_global_stack; 
    m_stack->m_fun = 0;
}

Env::~Env()
{
    delete m_heap;
    delete m_global_stack;
}


void Env::CreateVar(const string & name, ASTValue * val)
{
    auto it = m_stack->m_ref.find(name);

    if ( it != m_stack->m_ref.end() )
        throw runtime_error("Error : Variable was already declared.");

    if (val->Type() == AST::Type::Array)
    {
        ASTArray * arr = reinterpret_cast<ASTArray*>(val);
        if (arr->m_heap_ref != -1){
            m_stack->m_ref.insert({name,arr->m_heap_ref});
            return;
        }
    }

    m_stack->m_ref.insert({name,m_heap->GetCurrent()});
    m_heap->AssignToHeap(val);
}

ASTValue * Env::GetValue(const string & name,Stack * now)
{
    if (now == nullptr)
        now = m_stack;
    do{
        bool fun = now->m_fun;
        auto ref = now->m_ref.find(name);
        if (ref == now->m_ref.end())
            now = now->m_prev;
        else{
            return m_heap->GetFromHeap(ref->second);
        }
        if (fun == true) now = m_global_stack;
    }while(now != nullptr);

    throw runtime_error("Error : Variable was not declared.");
}

void Env::AssignArrayElement(int heap_ref,ASTValue * val)
{
    m_heap->AssignToSpecific(heap_ref,val);
}

ASTValue * Env::AccessArrayElement(int heap_ref)
{
    return m_heap->GetFromHeap(heap_ref);
}

ASTFunction * Env::GetFun(const string & name)
{
    Stack * now = m_stack;
    do{
        bool fun = now->m_fun;
        auto ref = now->m_ref.find(name);
        if (ref == now->m_ref.end())
            now = now->m_prev;
        else{
            return m_heap->GetFromHeapFun(ref->second);
        }
        if (fun == true) now = m_global_stack;
    }while(now != nullptr);

    throw runtime_error("Error : Variable was not declared.");

}


void Env::AssignValue(const string & name, ASTValue * val, Stack * now)
{
    if (now == nullptr)
        now = m_stack;
    do{
        bool fun = now->m_fun;
        auto ref = now->m_ref.find(name);
        if (ref == now->m_ref.end())
            now = now->m_prev;
        else{
            if (val->Type() == AST::Type::Array)
            {
                ASTArray * arr = reinterpret_cast<ASTArray*>(val);
                if (arr->m_heap_ref != -1){
                    ref->second = arr->m_heap_ref;
                    return;
                }
            }
            ref->second = m_heap->GetCurrent();
            return m_heap->AssignToHeap(val);
        }
        if (fun == true) now = m_global_stack;
    }while(now != nullptr);
    
    throw runtime_error("Error : Variable was not defined.");
}


void Env::CreateFun(const string name, ASTFunction * fun)
{
    auto it = m_stack->m_ref.find(name);

    if ( it != m_stack->m_ref.end() )
        throw runtime_error("Error : Function name was already declared.");

    m_stack->m_ref.insert({name,m_heap->GetCurrent()});
    m_heap->AssignToHeapFun(fun);

}


void Env::PushStack(bool fun)
{
    Stack * now = new Stack();
    now->m_prev = m_stack;
    m_stack = now;
    m_stack->m_fun = fun;
}
void Env::PopStack()
{
    Stack * now = m_stack;
    m_stack = m_stack->m_prev;
    delete now;
}

void Env::PushStackMethod(Stack * stack)
{
    Stack * now = new Stack();
    now->m_fun = false;
    now->m_prev = stack;
    m_saveMethod = m_stack;
    m_stack = now;
    stack->m_fun = true;
}

void Env::PopStackMethod()
{
    Stack * now = m_stack;
    now->m_prev->m_fun = false;
    m_stack = m_saveMethod;

    delete now;
}



Stack * Env::GetPopStack()
{
    Stack * now = m_stack;
    m_stack = m_stack->m_prev;
    now->m_prev = nullptr;
    now->m_fun = false;
    return now;
}