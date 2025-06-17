#ifndef RNPACKAGE_HELLO_CXX_MODULE_H_
#define RNPACKAGE_HELLO_CXX_MODULE_H_

#include "CxxModule.h"

class HelloCxxModule : public facebook::xplat::module::CxxModule {
 public:
  HelloCxxModule();

  std::string getName() override;

  auto getMethods() -> std::vector<Method> override;
};

#endif  // RNPACKAGE_HELLO_CXX_MODULE_H_
