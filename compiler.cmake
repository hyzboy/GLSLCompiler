IF(WIN32)

    if(MINGW)
        SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -std=c99 -g -fchar8_t")
        SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++2a -g -fchar8_t -Wall")

        SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -std=c99 -O2 -fchar8_t")
        SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++2a -O2 -fchar8_t")

        add_definitions(-D_WIN32_WINNT=0x0601)
    endif()

    if(MSVC)

        OPTION(MSVC_USE_DLL "use MSVC DLL" ON)

        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /std:c11")
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++14")

        if(MSVC_USE_DLL)
            SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MDd")
            SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")

            SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MD")
            SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
        else()
            SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MTd")
            SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

            SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MT")
            SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
        endif()

        add_definitions(-D_CRT_SECURE_NO_WARNINGS)

        add_compile_options(/wd4244)    # ->int     精度丢失
        add_compile_options(/wd4305)    # ->float   精度丢失
        add_compile_options(/wd4311)    # template
        add_compile_options(/wd4800)    # ->bool    性能损失
        add_compile_options(/wd4804)    # unsafe compare
        add_compile_options(/wd4805)    # unsafe compare
        add_compile_options(/wd4819)    # ansi->unicode
        add_compile_options(/wd4996)    # sprintf/sscanf unsafe
    endif()

ELSE()
    IF(NOT ANDROID)
        IF(APPLE)
            SET(USE_CLANG       ON)
        ELSE()
            OPTION(USE_CLANG    OFF)
        ENDIF()

        if(USE_CLANG)
            SET(CMAKE_C_COMPILER /usr/bin/clang)
            SET(CMAKE_CXX_COMPILER /usr/bin/clang++)
        endif()
    ENDIF()

    OPTION(USE_CHAR8_T OFF)

    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11")

    IF(USE_CHAR8_T)
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fchar8_t")
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fchar8_t")
    ENDIF()

    SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -ggdb3")
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -ggdb3")

    SET(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Ofast")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast")
ENDIF()

MESSAGE("C Compiler: " ${CMAKE_C_COMPILER})
MESSAGE("C++ Compiler: " ${CMAKE_CXX_COMPILER})
MESSAGE("C Flag: " ${CMAKE_C_FLAGS})
MESSAGE("C++ Flag: " ${CMAKE_CXX_FLAGS})
