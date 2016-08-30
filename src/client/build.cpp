/*
 * Copyright (c) 2016, Egor Pugin
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     1. Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *     2. Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *     3. Neither the name of the copyright holder nor the names of
 *        its contributors may be used to endorse or promote products
 *        derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "build.h"

#include <boost/algorithm/string.hpp>

#include <cppan.h>

std::vector<std::string> extract_comments(const std::string &s);

Config generate_config(const path &fn, bool silent = true)
{
    Config conf;

    if (!fs::exists(fn))
        throw std::runtime_error("File or directory does not exist: " + fn.string());

    auto read_from_cpp = [&conf, &silent](const path &fn)
    {
        auto s = read_file(fn);
        auto comments = extract_comments(s);

        int loaded = -1;
        int i = -1;
        for (auto &comment : comments)
        {
            try
            {
                i++;
                boost::trim(comment);
                auto root = YAML::Load(comment);
                auto sz = root.size();
                if (sz == 0)
                    continue;
                conf.load(root);
                loaded = i;
                break;
            }
            catch (...)
            {
            }
        }

        if (!silent)
            conf.build_settings.silent = false;
        conf.prepare_build(fn, comments.size() > (size_t)i ? comments[i] : "");
    };

    if (fs::is_regular_file(fn))
    {
        read_from_cpp(fn);
    }
    else if (fs::is_directory(fn))
    {
        if (fs::exists(fn / CPPAN_FILENAME))
        {
            conf = Config(fn);
            conf.prepare_build(fn / CPPAN_FILENAME, read_file(fn / CPPAN_FILENAME));
        }
        else if (fs::exists(fn / "main.cpp"))
        {
            read_from_cpp(fn / "main.cpp");
        }
        else
            throw std::runtime_error("No candidates {cppan.yml|main.cpp} for reading in directory " + fn.string());
    }

    return conf;
}

int generate(const path &fn)
{
    auto conf = generate_config(fn, false);
    return conf.generate();
}

int build(const path &fn)
{
    auto conf = generate_config(fn);
    if (conf.generate())
        return 1;
    return conf.build();
}