//
// Copyright (C) 2002-2005  3Dlabs Inc. Ltd.
// Copyright (C) 2017 Google, Inc.
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    Neither the name of 3Dlabs Inc. Ltd. nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//

#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <set>
#include <string.h>

#include <glslang/Public/ShaderLang.h>
#include "RootIncludePath.h"

// All includes, including nested and system includes, are root-relative.
class DirStackFileIncluder : public glslang::TShader::Includer {
public:
    DirStackFileIncluder() = default;

    virtual IncludeResult* includeLocal(const char* headerName,
                                        const char* includerName,
                                        size_t inclusionDepth) override
    {
        return readLocalPath(headerName, includerName, (int)inclusionDepth);
    }

    virtual IncludeResult* includeSystem(const char* headerName,
                                         const char* /*includerName*/,
                                         size_t /*inclusionDepth*/) override
    {
        return readSystemPath(headerName);
    }

    virtual void setRootDirectory(const std::string& dir)
    {
        directoryStack.clear();
        directoryStack.push_back(dir);
    }

    virtual void releaseInclude(IncludeResult* result) override
    {
        if (result != nullptr) {
            delete [] static_cast<tUserDataElement*>(result->userData);
            delete result;
        }
    }

    virtual std::set<std::string> getIncludedFiles()
    {
        return includedFiles;
    }

    virtual ~DirStackFileIncluder() override { }

protected:
    typedef char tUserDataElement;
    std::vector<std::string> directoryStack;
    std::set<std::string> includedFiles;

    virtual IncludeResult* readLocalPath(const char* headerName, const char* /*includerName*/, int /*depth*/)
    {
        if (directoryStack.size() != 1 || !headerName || !*headerName)
            return nullptr;

        for (auto it = directoryStack.rbegin(); it != directoryStack.rend(); ++it) {
            std::string path = *it + '/' + headerName;
            std::replace(path.begin(), path.end(), '\\', '/');
            const auto relative_start = it->length() + 1;
            if (!hgl::shader::NormalizeRootIncludePath(path.data() + relative_start))
                return nullptr;
            path.resize(relative_start + strlen(path.c_str() + relative_start));
            std::ifstream file(path, std::ios_base::binary | std::ios_base::ate);
            if (file) {
                includedFiles.insert(path);
                return newIncludeResult(path, file, (int)file.tellg());
            }
        }

        return nullptr;
    }

    virtual IncludeResult* readSystemPath(const char* headerName)
    {
        return readLocalPath(headerName, nullptr, 0);
    }

    // Do actual reading of the file, filling in a new include result.
    virtual IncludeResult* newIncludeResult(const std::string& path, std::ifstream& file, int length) const
    {
        char* content = new tUserDataElement [length];
        file.seekg(0, file.beg);
        file.read(content, length);
        return new IncludeResult(path, content, length, content);
    }

};
