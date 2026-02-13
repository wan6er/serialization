#include <cstdio>
#include <iostream>
#include <thread>
#include <vector>

#include "json.hpp"
#include "serialization.h"
using json = nlohmann::json;
using namespace SERIALIZATION;

struct TestStruct : public Serialization
{
    SInt32 a = { this, "age"};
    SFloat32 b = { this, "height", 2.0f };
    TestStruct() = default;
    TestStruct(Serialization* cls, std::string const& name) : Serialization(cls, name) {}
};
struct TestStructA : public Serialization
{
    SUint32 a = { this, "1", 123 };
    TestStruct tsa = { this, "2" };
    SFloat32 b = { this, "3", 456.0f };
    TestStructA() = default;
    TestStructA(Serialization* cls, std::string const& name) : Serialization(cls, name) {}

};


int main()
{
    // auto ts = test();
    // std::cout << ts.get_json().dump() << std::endl;
    // {
    //     auto aa = ts.a;
    //     aa = 20;
    // }
    // printf("test\n");
    // std::cout << ts.get_json().dump() << std::endl;

    // TestStructA tes;
    // std::cout << tes.get_json().dump() << std::endl;

    // std::string json_str = "{\"1\":10,\"2\":{\"age\":10,\"height\":5.900000095367432},\"3\":5.900000095367432}";
    // tes.load_json(json().parse(json_str));
    // std::cout << tes.get_json().dump() << std::endl;

    {
        TestStructA test1;

        std::vector<std::thread> thrs(10000);
        int i = 0;
        for (auto& thr : thrs)
        {
            thrs[i] = std::thread([test1] (int a) {
                
            }, i);
            i++;
        }
        for (auto& thr : thrs)
        {
            thr.join();
        }
        
        std::cout << test1.get_json().dump() << std::endl;

    }
    return 0;
}