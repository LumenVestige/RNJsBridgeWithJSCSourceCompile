/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "lyra.h"

#include <atomic>
#include <iomanip>
#include <ios>
#include <memory>
#include <ostream>
#include <vector>

#ifndef _WIN32
#include <dlfcn.h>
#include <unwind.h>
#endif

#include <fbjni/detail/Log.h>

using namespace std;

namespace facebook {
namespace lyra {

namespace {

class IosFlagsSaver {
  ios_base& ios_;
  ios_base::fmtflags flags_;

 public:
  IosFlagsSaver(ios_base& ios) : ios_(ios), flags_(ios.flags()) {}

  ~IosFlagsSaver() {
    ios_.flags(flags_);
  }
};

struct BacktraceState {
  size_t skip;
  vector<InstructionPointer>& stackTrace;
};

#ifndef _MSC_VER
_Unwind_Reason_Code unwindCallback(struct _Unwind_Context* context, void* arg) {
  BacktraceState* state = reinterpret_cast<BacktraceState*>(arg);
  auto absoluteProgramCounter =
      reinterpret_cast<InstructionPointer>(_Unwind_GetIP(context));

  if (state->skip > 0) {
    --state->skip;
    return _URC_NO_REASON;
  }

  if (state->stackTrace.size() == state->stackTrace.capacity()) {
    return _URC_END_OF_STACK;
  }

  state->stackTrace.push_back(absoluteProgramCounter);

  return _URC_NO_REASON;
}
#endif

void captureBacktrace(size_t skip, vector<InstructionPointer>& stackTrace) {
  // Beware of a bug on some platforms, which makes the trace loop until the
  // buffer is full when it reaches a noexcept function. It seems to be fixed in
  // newer versions of gcc. https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56846
  // TODO(t10738439): Investigate workaround for the stack trace bug
  BacktraceState state = {skip, stackTrace};
#ifndef _WIN32
  _Unwind_Backtrace(unwindCallback, &state);
#endif
}

// this is a pointer to a function
std::atomic<LibraryIdentifierFunctionType> gLibraryIdentifierFunction{nullptr};

} // namespace

void setLibraryIdentifierFunction(LibraryIdentifierFunctionType func) {
  gLibraryIdentifierFunction.store(func, std::memory_order_relaxed);
}

std::string StackTraceElement::buildId() const {
  if (!hasBuildId_) {
    auto getBuildId =
        gLibraryIdentifierFunction.load(std::memory_order_relaxed);
    if (getBuildId) {
      buildId_ = getBuildId(libraryName());
    } else {
      buildId_ = "<unimplemented>";
    }
    hasBuildId_ = true;
  }
  return buildId_;
}

void getStackTrace(vector<InstructionPointer>& stackTrace, size_t skip) {
  stackTrace.clear();
  captureBacktrace(skip + 1, stackTrace);
}

// TODO(t10737622): Improve on-device symbolification
void getStackTraceSymbols(
    vector<StackTraceElement>& symbols,
    const vector<InstructionPointer>& trace) {
  symbols.clear();
  symbols.reserve(trace.size());

#ifndef _WIN32
  for (size_t i = 0; i < trace.size(); ++i) {
    Dl_info info;
    if (dladdr(trace[i], &info)) {
      symbols.emplace_back(
          trace[i],
          info.dli_fbase,
          info.dli_saddr,
          info.dli_fname ? info.dli_fname : "",
          info.dli_sname ? info.dli_sname : "");
    }
  }
#endif
}

ostream& operator<<(ostream& out, const StackTraceElement& elm) {
  IosFlagsSaver flags{out};

  out << "{dso=" << elm.libraryName() << " offset=" << hex << showbase
      << elm.libraryOffset();

  if (!elm.functionName().empty()) {
    out << " func=" << elm.functionName() << "()+" << elm.functionOffset();
  }

  out << " build-id=" << hex << setw(8) << elm.buildId() << "}";

  return out;
}

// TODO(t10737667): The implement a tool that parse the stack trace and
// symbolicate it
ostream& operator<<(ostream& out, const vector<StackTraceElement>& trace) {
  IosFlagsSaver flags{out};

  auto i = 0;
  out << "Backtrace:\n";
  for (auto& elm : trace) {
    out << "    #" << dec << setfill('0') << setw(2) << i++ << " " << elm
        << '\n';
  }

  return out;
}

void logStackTrace(const vector<StackTraceElement>& trace) {
  auto i = 0;
  FBJNI_LOGE("Backtrace:");
  for (auto& elm : trace) {
    if (!elm.functionName().empty()) {
      FBJNI_LOGE(
          "    #%02d |lyra|{dso=%s offset=%#tx func=%s+%#x build-id=%s}",
          i++,
          elm.libraryName().c_str(),
          elm.libraryOffset(),
          elm.functionName().c_str(),
          elm.functionOffset(),
          elm.buildId().c_str());
    } else {
      FBJNI_LOGE(
          "    #%02d |lyra|{dso=%s offset=%#tx build-id=%s}",
          i++,
          elm.libraryName().c_str(),
          elm.libraryOffset(),
          elm.buildId().c_str());
    }
  }
}

} // namespace lyra
} // namespace facebook
