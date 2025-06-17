#include "TraceSection.h"
#include <string>
namespace trace {

    TraceSection::TraceSection(const char* sectionName)
            : name_(sectionName) {
        std::string final_sectionName = std::string("native:") + name_;
        ATrace_beginSection(final_sectionName.c_str());
    }

    TraceSection::~TraceSection() {
        ATrace_endSection();
    }

}
