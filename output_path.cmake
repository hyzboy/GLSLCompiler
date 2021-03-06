﻿IF(ANDROID)
    SET(OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/out/android/${ANDROID_ABI})
ELSEIF(IOS)
    SET(OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/out/iOS/${IOS_PLATFORM})
ELSE()
    IF(WIN32)
        SET(USE_GUI ON)
    ENDIF()

    SET(OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/out/${CMAKE_SYSTEM_NAME}_${HGL_BITS})
ENDIF()

MESSAGE("SYSTEM NAME: " ${CMAKE_SYSTEM_NAME})
MESSAGE("OUTPUT DIRECTORY: " ${OUTPUT_DIRECTORY})

SET(OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIRECTORY}_Debug)
SET(OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY}_Release)
SET(OUTPUT_DIRECTORY_MINSIZEREL ${OUTPUT_DIRECTORY}_MinSizeRel)
SET(OUTPUT_DIRECTORY_RELWITHDEBINFO ${OUTPUT_DIRECTORY}_RelWithDebInfo)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIRECTORY_DEBUG})
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIRECTORY_DEBUG})
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIRECTORY_DEBUG})

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY_RELEASE})
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY_RELEASE})
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY_RELEASE})

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${OUTPUT_DIRECTORY_MINSIZEREL})
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_MINSIZEREL ${OUTPUT_DIRECTORY_MINSIZEREL})
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${OUTPUT_DIRECTORY_MINSIZEREL})

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OUTPUT_DIRECTORY_RELWITHDEBINFO})
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OUTPUT_DIRECTORY_RELWITHDEBINFO})
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OUTPUT_DIRECTORY_RELWITHDEBINFO})

