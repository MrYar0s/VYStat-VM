#ifndef RUNTIME_INTERPRETER_HPP
#define RUNTIME_INTERPRETER_HPP

#include <cstdint>

namespace shrimp {

// Will be generated later
enum class IntrinsicType : uint16_t { PrintI, PrintF, ScanI, ScanF, SinF, SinI, CosF, CosI, SqrtF, SqrtI };

void PrintI(int val);
void PrintF(float val);
int ScanI();
float ScanF();
float SinF(float val);
float SinI(int val);
float CosF(float val);
float CosI(int val);
float SqrtF(float val);
float SqrtI(int val);

}  // namespace shrimp

#endif  // RUNTIME_INTERPRETER_HPP
