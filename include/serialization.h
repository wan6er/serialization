#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <map>
#include <type_traits>
#include <atomic>
#include "json.hpp"

namespace SERIALIZATION
{
    
    class SERTypeBase
    {
    public:
        virtual void serialize(nlohmann::json & json) const {}
        virtual void deserialize(nlohmann::json & json) {}
        auto & get_name() { return ser_name; }
        auto const& get_name() const { return ser_name; }

        void inc_ref() 
        { 
            int ref; 
            if (_ref) 
                ref = _ref->fetch_add(1, std::memory_order_relaxed); 
        }
        int dec_ref() 
        { 
            if (_ref && (1 == _ref->fetch_sub(1, std::memory_order_acq_rel))) 
            { 
                if (ser_val) free(ser_val); 
                if (_ref) delete _ref; 
                return 1;
            } 
            return 0;
        }

        int get_ref() const { return _ref ? _ref->load(std::memory_order_relaxed) : 0; }

        void swap(SERTypeBase& type) 
        { 
            std::swap(ser_name, type.ser_name); 
            std::swap(ser_val, type.ser_val); 
            std::swap(_ref, type._ref); 
        }
        virtual ~SERTypeBase() { dec_ref(); }

    protected:
        SERTypeBase(std::string const& name, size_t size) : ser_name(name), ser_val(malloc(size)), _ref(new std::atomic_int(0)) 
        {
            _ref->fetch_add(1, std::memory_order_relaxed);
            memset(ser_val, 0, size);
        }
        
        SERTypeBase(SERTypeBase const& type) : ser_name(type.ser_name), ser_val(type.ser_val), _ref(type._ref) { inc_ref();}
        // SERTypeBase(SERTypeBase && type) : ser_name(type.ser_name), ser_val(type.ser_val), _ref(type._ref) { inc_ref();}

        std::atomic_int* _ref;
        std::string ser_name;
        void* ser_val;
    };

    class Serialization : public SERTypeBase
    {
    public:
        typedef std::map<std::string, SERTypeBase*> Members;
    
        Serialization() : SERTypeBase("", sizeof(Members)) { new (ser_val) Members; };
        Serialization(std::string const& name) : SERTypeBase(name, sizeof(Members)) 
        { 
            new (ser_val) Members; 
        };

        Serialization(Serialization && cls) : SERTypeBase(std::move(cls)) {};
        Serialization(Serialization const& cls) : SERTypeBase(cls) {};

        virtual ~Serialization() 
        {
            if (1 == get_ref())
            {
                auto& members = *(Members*) ser_val;
                for (auto& member : members) 
                {
                    delete member.second;
                }
                members.clear();
            }
        }

        Serialization(Serialization* cls, std::string const& name) : SERTypeBase(name, sizeof(Members)) 
        { 
            new (ser_val) Members; 
            cls->add_member(this); 
        };
        

        auto get_json() 
        {
            nlohmann::json json;
            serialize(json);
            return json;
        }

        void load_json(nlohmann::json json) 
        {
            deserialize(json);
        }

        template<typename __Type>
        void add_member(__Type* member) 
        {
            auto& members = *(Members*) ser_val;
            members.insert(std::pair<std::string, SERTypeBase*>(member->get_name(), new __Type(*member)));
        }

        virtual void serialize(nlohmann::json & json) const
        {
            auto& members = *(Members*) ser_val;
            if (get_name().empty())
            {
                for (auto& member : members) {
                    member.second->serialize(json);
                }
            }
            else
            {
                for (auto& member : members) {
                    member.second->serialize(json[get_name()]);
                }
            }
        }
        virtual void deserialize(nlohmann::json & json) 
        {
            auto& members = *(Members*) ser_val;
            if (get_name().empty())
            {
                for (auto& member : members) {
                    member.second->deserialize(json);
                }
            }
            else
            {
                for (auto& member : members) {
                    member.second->deserialize(json[get_name()]);
                }
            }
        }
    };

    template<typename _Type>
    class SERType : public SERTypeBase
    {
    public:
        typedef std::enable_if_t<std::is_default_constructible<_Type>::value, _Type> ValType;

        SERType(std::string const& name) : SERTypeBase(name, sizeof(ValType)) {};
        SERType(Serialization* cls, std::string const& name) : SERTypeBase(name, sizeof(ValType)) 
        { 
            cls->add_member(this); 
        };

        template<typename __Type>
        SERType(std::string const& name, __Type&& value) : SERTypeBase(name, sizeof(ValType)) 
        { 
            *reinterpret_cast<ValType*>(ser_val) = std::forward<__Type>(value); 
        };

        template<typename __Type>
        SERType(Serialization* cls, std::string const& name, __Type&& value) : SERTypeBase(name, sizeof(ValType)) 
        { 
            cls->add_member(this); 
            *reinterpret_cast<ValType*>(ser_val) = std::forward<__Type>(value); 
        };

        SERType(SERType const& type) : SERTypeBase(type) {}

        virtual ~SERType() {}

        operator _Type&() { return ser_val; }
        operator _Type const&() const { return ser_val; }

        SERType operator=(_Type const& type) 
        { 
            get_value() = type;
            return *this; 
        }

        virtual void serialize(nlohmann::json & json) const { json[get_name()] = get_value(); }
        virtual void deserialize(nlohmann::json & json) { json.at(get_name()).get_to<ValType>(get_value()); }

        auto & get_value() { return *reinterpret_cast<ValType*>(ser_val); }
        auto const& get_value() const { return *reinterpret_cast<ValType*>(ser_val); }
    
    };

    typedef SERType<unsigned char> SUint8;
    typedef SERType<unsigned short> SUint16;
    typedef SERType<unsigned int> SUint32;
    typedef SERType<unsigned long long> SUint64;
    typedef SERType<char> SInt8;
    typedef SERType<short> SInt16;
    typedef SERType<int> SInt32;
    typedef SERType<long long> SInt64;
    typedef SERType<float> SFloat32;
    typedef SERType<double> SFloat64;


    typedef SERType<std::string> SString;
};