#pragma once

#include <any>
#include <format>
#include <unordered_map>
#include <cassert>

typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;

#define check(x) assert(x)
#define checkNoEntry() assert(false)

// ---------------------------------------------------------------------------------------------------------------------

namespace XClassHelpers
{
    template<typename Type, std::enable_if_t<std::is_class_v<Type>, int> = 0>
    const char* GetClassName(Type* o = nullptr)
    {
        return typeid(Type).name() + (sizeof("class ") - 1);
    }
}

template<typename Type>
class XBaseClass;

template<typename Type, std::enable_if_t<std::is_class_v<Type>, int> = 0>
class XClassFactory
{
    friend XBaseClass<Type>;

public:
    XClassFactory(const XClassFactory&) = delete;
    XClassFactory& operator=(const XClassFactory&) = delete;

    static XBaseClass<Type>* GetClass(const char* ClassName)
    {
        return Get().GetClassInternal(ClassName);
    }

    static XBaseClass<Type>* GetClass(const std::string& ClassName)
    {
        return Get().GetClassInternal(ClassName.c_str());
    }

protected:
    XClassFactory() = default;
    virtual ~XClassFactory() = default;

    static XClassFactory<Type>& Get()
    {
        static XClassFactory<Type> instance;
        return instance;
    }

    static void RegClass(XBaseClass<Type>* Class)
    {
        Get().RegClassInternal(Class);
    }

    bool RegClassInternal(XBaseClass<Type>* Class)
    {
        check(Class);
        const std::string_view className(Class->GetName());
        if (Classes.find(className) == Classes.end()) // 不存在
        {
            Classes.emplace(className, Class);
            return true;
        }

        return false;
    }

    XBaseClass<Type>* GetClassInternal(const char* ClassName)
    {
        check(ClassName);
        const std::string_view className(ClassName);
        const auto it = Classes.find(className);
        if (it != Classes.end()) // 存在
        {
            const std::any& any = it->second;
            check(any.has_value());
            XBaseClass<Type>* found = std::any_cast<XBaseClass<Type>*>(any);
            check(found);
            return found;
        }

        return nullptr;
    }

    std::unordered_map<std::string_view, std::any> Classes{};
};

struct XProperty
{
    XProperty() = default;
    XProperty(std::string InName, std::any InType, size_t InOffset, size_t InSize)
        : Name(std::move(InName)), Type(std::move(InType)), Offset(InOffset), Size(InSize)
    {}

    std::string Name{};
    std::any Type{};
    size_t Offset{ 0 };
    size_t Size{ 0 };

    template<typename PropertyType>
    PropertyType* GetValue(void* InO) const
    {
        check(this);
        if (!Type.has_value())
        {
            return nullptr;
        }

        try { void* found = std::any_cast<PropertyType*>(Type); }
        catch (...)
        {
            return nullptr;
        }

        return (PropertyType*)(static_cast<uint8*>(InO) + Offset);
    }

    template<typename PropertyType>
    bool IsType() const
    {
        check(this);
        return Type.has_value() ? Type.type() == typeid(PropertyType*) : false;
    }

    const char* GetTypeName() const
    {
        check(this);
        return Type.has_value() ? Type.type().name() : nullptr;
    }
};

struct XReflex
{
    XReflex() = default;
    virtual ~XReflex() = default;

    const XProperty* FindPropertyByName(const std::string& InName) const
    {
        for (const XProperty& property : Props)
        {
            if (property.Name == InName)
                return &property;
        }

        return nullptr;
    }

    const std::list<XProperty>& GetPropertyArray() const
    {
        return Props;
    }

protected:
    std::list<const char*> Classes{};
    std::list<XProperty> Props{};
};

#define XCLASS() Classes.emplace_back(XClassHelpers::GetClassName<Self>())
#define XPROPERTY(PROPERTY) Props.emplace_back(#PROPERTY, static_cast<decltype(Self::PROPERTY)*>(nullptr), offsetof(Self, Self::PROPERTY), sizeof(decltype(Self::PROPERTY)))

template <typename Type, typename DefaultType, typename = void>
struct GetReflexType
{
    using type = DefaultType;
};

template <typename Type, typename DefaultType>
struct GetReflexType<Type, DefaultType, std::void_t<typename Type::Reflex>>
{
    using type = typename Type::Reflex;
};

template<typename Type>
class XBaseClass
{
public:
    using SuperReflexType = XReflex;
    using ReflexType = typename GetReflexType<Type, SuperReflexType>::type;
    static_assert(std::is_base_of_v<SuperReflexType, ReflexType>);

    virtual const char* GetName() const { return Name; }
    virtual Type* New() = 0;
    virtual std::shared_ptr<Type> MakeShared() = 0;
    virtual std::unique_ptr<Type> MakeUnique() = 0;

    auto GetReflex()
    {
        check(Reflex);
        return Reflex.get();
    }

protected:
    virtual ~XBaseClass() = default;

    void RegClass()
    {
        XClassFactory<Type>::RegClass(this);
    }

    const char* Name{nullptr};
    std::unique_ptr<ReflexType> Reflex{};
};

template<typename BaseType, typename Type, std::enable_if_t<std::is_base_of_v<BaseType, Type>, int> = 0>
class XSubclass : public XBaseClass<BaseType>
{
public:
    using SuperReflexType = typename XBaseClass<BaseType>::ReflexType;
    using ReflexType = typename GetReflexType<Type, SuperReflexType>::type;
    static_assert(std::is_base_of_v<SuperReflexType, ReflexType>);

    XSubclass()
    {
        this->Name = XClassHelpers::GetClassName<Type>();
        this->Reflex = std::make_unique<ReflexType>();
        this->RegClass();
    }

    BaseType* New() override { return new (std::nothrow) Type(); }
    std::shared_ptr<BaseType> MakeShared() override { return std::make_shared<Type>(); }
    std::unique_ptr<BaseType> MakeUnique() override { return std::make_unique<Type>(); }
};

#define DEFINE_CLASS(T, CLASS) XSubclass<T, CLASS> CLASS##_C
#define CLASS_BODY(SELF, SUPER) public: virtual const char* GetClassName() const /* override */ { return XClassHelpers::GetClassName(this); } using Self = SELF; using Super = SUPER; struct Reflex; private: