# Copyright (c) Facebook, Inc. and its affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


file(GLOB SOURCES
        ./*.cpp
)

add_library(js-bridge SHARED ${SOURCES})

target_link_libraries(js-bridge fbjni folly_json boost glog glog_init android log trace)

target_include_directories(js-bridge
        PRIVATE
        ${CMAKE_SOURCE_DIR}
)
