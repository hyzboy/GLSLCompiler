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

    // [0] is the shader library root; it is the only root for every include whose
    // normalized path does not carry the `generated/` prefix.
    virtual bool setRootDirectory(const std::string& dir)
    {
        if (dir.empty())
            return false;

        directoryStack.clear();
        directoryStack.push_back(dir);
        return true;
    }

    // Optional second root, used only for `generated/` includes produced by the
    // current build job.  Keeping it a separate root means a same-named file in
    // the library (or in the CWD) can never shadow a task-generated one.
    virtual bool setGeneratedRootDirectory(const std::string& dir)
    {
        if (dir.empty())
            return false;

        generatedRoot = dir;
        return true;
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
    std::string generatedRoot;
    std::set<std::string> includedFiles;

    virtual IncludeResult* readLocalPath(const char* headerName, const char* /*includerName*/, int /*depth*/)
    {
        if (directoryStack.size() != 1 || !headerName || !*headerName)
            return nullptr;

        std::string relative(headerName);
        std::replace(relative.begin(), relative.end(), '\\', '/');

        // Normalize before choosing a root: folding happens here, so
        // `generated/../common/x.glsl` lands in the library partition.
        if (!hgl::shader::NormalizeRootIncludePath(relative.data()))
            return nullptr;
        relative.resize(strlen(relative.c_str()));

        const std::string* root = &directoryStack[0];
        if (hgl::shader::HasGeneratedPrefix(relative.c_str()))
        {
            if (generatedRoot.empty())
                return nullptr;

            root = &generatedRoot;
        }

        std::string path = *root + '/' + relative;
        std::replace(path.begin(), path.end(), '\\', '/');

        std::ifstream file(path, std::ios_base::binary | std::ios_base::ate);
        if (!file)
            return nullptr;

        includedFiles.insert(path);
        return newIncludeResult(path, file, (int)file.tellg());
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
