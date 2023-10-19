#ifndef CORE_INTERPRETER_INTRINSICS_HPP
#define CORE_INTERPRETER_INTRINSICS_HPP

#include <cstdint>

namespace shrimp {

void PrintI(int val);
void PrintF(float val);
int ScanI();
float ScanF();
float SinF(float val);
float CosF(float val);
float SqrtF(float val);

}  // namespace shrimp

#endif  // CORE_INTERPRETER_INTRINSICS_HPP
