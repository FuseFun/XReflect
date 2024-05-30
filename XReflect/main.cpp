#include <iostream>
#include <ranges>

#include "XClass.h"

// ---------------------------------------------------------------------------------------------------------------------

class TempClassA
{
    CLASS_BODY(TempClassA, void)
    
public:
    TempClassA() = default;
    virtual ~TempClassA() = default;

    std::string v0{"hello"};
    std::string v1{"world"};
    std::string v2{"!"};
};

struct TempClassA::Reflex : GetReflexType<Super, XReflex>::type
{
    Reflex()
    {
        XCLASS();
        XPROPERTY(v0);
        XPROPERTY(v1);
        XPROPERTY(v2);
    }
};
DEFINE_CLASS(TempClassA, TempClassA);

// ---------------------------------------------------------------------------------------------------------------------

class TempClassB : public TempClassA
{
    CLASS_BODY(TempClassB, TempClassA)
    
public:
    std::string v0b{"v0b"};
    std::string v1b{"v1b"};
    std::string v2b{"v2b"};
    int32 b_i32 = -3200;
    uint32 b_u32 = 3200;
};

struct TempClassB::Reflex : GetReflexType<Super, XReflex>::type
{
    Reflex()
    {
        XCLASS();
        XPROPERTY(v0b);
        XPROPERTY(v1b);
        XPROPERTY(v2b);
        XPROPERTY(b_i32);
        XPROPERTY(b_u32);
    }
};
DEFINE_CLASS(TempClassA, TempClassB);

// ---------------------------------------------------------------------------------------------------------------------

class TempClassC : public TempClassB
{
    CLASS_BODY(TempClassC, TempClassB)
    
public:
    std::string v0c{"***"};
    std::string v1c{"123"};
    std::string v2c{"你好"};
    int32 i32c = 6666;
};

struct TempClassC::Reflex : GetReflexType<Super, XReflex>::type
{
    Reflex()
    {
        XCLASS();
        XPROPERTY(v0c);
        XPROPERTY(v1c);
        XPROPERTY(v2c);
        XPROPERTY(i32c);
    }
};
DEFINE_CLASS(TempClassA, TempClassC);

// ---------------------------------------------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    // 从类名获取反射类
    XBaseClass<TempClassA>* Class = XClassFactory<TempClassA>::GetClass("TempClassC");
    if (Class)
    {
        // 从类取类名
        std::cout << "从类取类名: " << Class->GetName() << std::endl;
        
        // 从反射类实例化
        std::unique_ptr Object = Class->MakeUnique();
        
        // 从实例取类名
        std::cout << "从实例取类名: " << Object->GetClassName() << std::endl;
        
        // 遍历属性
        for (auto& property : Class->GetReflex()->GetPropertyArray())
        {
            // 处理 std::string 类型属性
            if (property.IsType<std::string>())
            {
                std::cout << std::format("PropertyName: {}, PropertyValue: {}", property.Name, *property.GetValue<std::string>(Object.get())) << std::endl;
            }
            // 处理 int32 类型属性
            if (property.IsType<int32>())
            {
                std::cout << std::format("PropertyName: {}, PropertyValue: {}", property.Name, *property.GetValue<int32>(Object.get())) << std::endl;
            }

            // 处理更多类型...
        }
        
        // 从属性名查找属性，并获取属性值
        if (const std::string* v0 = Class->GetReflex()->FindPropertyByName("v0")->GetValue<std::string>(Object.get()))
        {
            std::cout << "v0: " << *v0 << std::endl;
        }
    }
    
    return 0;
}