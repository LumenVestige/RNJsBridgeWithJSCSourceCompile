#pragma once

// ==== 导出宏定义（跨平台） ====
#if defined(_WIN32)
#ifdef TRACE_EXPORTS
    #define TRACE_API __declspec(dllexport)
  #else
    #define TRACE_API __declspec(dllimport)
  #endif
#elif defined(__GNUC__) && __GNUC__ >= 4
#define TRACE_API __attribute__((visibility("default")))
#else
#define TRACE_API
#endif

// ==== Android 平台兼容处理 ====
#if defined(__ANDROID__)
#include <android/trace.h>
#else
// 非 Android 平台提供空宏，防止链接错误
  #define ATrace_beginSection(name)
  #define ATrace_endSection()
#endif

namespace trace {

    class TRACE_API TraceSection {
    public:
        explicit TraceSection(const char* sectionName);
        ~TraceSection();

        TraceSection(const TraceSection&) = delete;
        TraceSection& operator=(const TraceSection&) = delete;

    private:
        const char* name_;
    };

} // namespace trace

// ==== 便捷宏：自动 trace 当前作用域 ====
#define TRACE_SECTION(name) ::trace::TraceSection _trace_section_##__LINE__ (name)
