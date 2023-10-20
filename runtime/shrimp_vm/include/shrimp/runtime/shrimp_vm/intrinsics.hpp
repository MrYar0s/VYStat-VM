#ifndef RUNTIME_SHRIMP_VM_INTRINSICS_HPP
#define RUNTIME_SHRIMP_VM_INTRINSICS_HPP

#include <cstdint>

namespace shrimp::runtime::intrinsics {

void PrintI(int val);
void PrintF(float val);
int ScanI();
float ScanF();
float SinF(float val);
float CosF(float val);
float SqrtF(float val);

}  // namespace shrimp::runtime::intrinsics

#endif  // RUNTIME_SHRIMP_VM_INTRINSICS_HPP
