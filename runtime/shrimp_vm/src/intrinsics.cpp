#include <iostream>
#include <cmath>

#include <shrimp/runtime/shrimp_vm/intrinsics.hpp>

namespace shrimp {

void PrintI(int val)
{
    std::cout << val << std::endl;
}

void PrintF(float val)
{
    std::cout << val << std::endl;
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

}  // namespace shrimp