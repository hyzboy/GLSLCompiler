#pragma once

namespace hgl::shader
{
    inline bool IsIncludeSeparator(const char ch)
    {
        return ch == '/' || ch == '\\';
    }

    // Task-generated sources live under a per-job directory instead of the shader
    // library root; every other root-relative include resolves against the library.
    // Normalize first: `generated/../common/x.glsl` collapses to `common/x.glsl`
    // and must not be able to hop over the partition.
    inline bool HasGeneratedPrefix(const char *path)
    {
        static constexpr char kPrefix[] = "generated/";
        if (!path)
            return false;
        for (int i = 0; kPrefix[i]; ++i)
            if (path[i] != kPrefix[i])
                return false;
        return true;
    }

    // Normalize in place without permitting absolute paths or escape above root.
    inline bool NormalizeRootIncludePath(char *path)
    {
        if (!path || !*path || IsIncludeSeparator(*path))
            return false;

        char *source = path;
        char *target = path;
        while (*source)
        {
            const char *part = source;
            while (*source && !IsIncludeSeparator(*source))
            {
                if (*source == ':')
                    return false;
                ++source;
            }
            const auto length = source - part;
            while (IsIncludeSeparator(*source))
                ++source;

            if (length == 1 && part[0] == '.')
                continue;
            if (length == 2 && part[0] == '.' && part[1] == '.')
            {
                if (target == path)
                    return false;
                while (target > path && target[-1] != '/')
                    --target;
                if (target > path)
                    --target;
            }
            else if (length > 0)
            {
                if (target > path)
                    *target++ = '/';
                for (auto i = 0; i < length; ++i)
                    *target++ = part[i];
            }
        }
        *target = 0;
        return target > path;
    }
}
