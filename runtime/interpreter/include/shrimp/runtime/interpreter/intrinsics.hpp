#ifndef RUNTIME_INTERPRETER_INTRINSICS_HPP
#define RUNTIME_INTERPRETER_INTRINSICS_HPP

#include <cstdint>
#include <string>

namespace shrimp::runtime::intrinsics {

void PrintI(int val);
void PrintF(float val);
void PrintStr(const std::string &str);
std::string Concat(const std::string &str1, const std::string &str2);
std::string Substr(const std::string &str, size_t pos, size_t len);
int ScanI();
float ScanF();
float SinF(float val);
float CosF(float val);
float SqrtF(float val);

}  // namespace shrimp::runtime::intrinsics

#endif  // RUNTIME_INTERPRETER_INTRINSICS_HPP
