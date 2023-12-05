#include <iostream>
#include <cmath>

#include <shrimp/runtime/interpreter/intrinsics.hpp>

namespace shrimp::runtime::intrinsics {

void PrintI(int val)
{
    std::cout << val << std::endl;
}

void PrintF(float val)
{
    std::cout << val << std::endl;
}

void PrintStr(const std::string &str)
{
    std::cout << str << std::endl;
}

std::string Concat(const std::string &str1, const std::string &str2)
{
    return str1 + str2;
}

std::string Substr(const std::string &str, size_t pos, size_t len)
{
    return str.substr(pos, len);
}

int ScanI()
{
    int val;
    std::cin >> val;
    return val;
}

float ScanF()
{
    float val;
    std::cin >> val;
    return val;
}

float SinF(float val)
{
    return std::sin(val);
}

float CosF(float val)
{
    return std::cos(val);
}

float SqrtF(float val)
{
    return std::sqrt(val);
}

}  // namespace shrimp::runtime::intrinsics
