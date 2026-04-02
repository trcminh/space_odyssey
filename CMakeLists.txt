cmake_minimum_required(VERSION 3.10)
project(SpaceOdyssey)

set(CMAKE_CXX_STANDARD 17)

# Ghi đè đường dẫn xuất file .exe ra nằm ngang hàng với thư mục asset để tránh lỗi thiếu file
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

# Khai báo đường dẫn tương đối
set(SDL2_DIR ${CMAKE_SOURCE_DIR}/SDL2)
set(SDL2_IMAGE_DIR ${CMAKE_SOURCE_DIR}/SDL2_image)
set(SDL2_TTF_DIR ${CMAKE_SOURCE_DIR}/SDL2_ttf)
set(SDL2_MIXER_DIR ${CMAKE_SOURCE_DIR}/SDL2_mixer)

# Tìm Headers (Hỗ trợ cấu trúc thư mục lộn xộn hoặc nhiều chuẩn phiên bản)
include_directories(
    ${SDL2_DIR}/include
    ${SDL2_DIR}/include/SDL2
    ${SDL2_DIR}/include-config-/SDL2
    ${SDL2_DIR}/include-config-debug/SDL2
    ${SDL2_IMAGE_DIR}/include
    ${SDL2_IMAGE_DIR}/include/SDL2
    ${SDL2_TTF_DIR}/include
    ${SDL2_TTF_DIR}/include/SDL2
    ${SDL2_MIXER_DIR}/include
    ${SDL2_MIXER_DIR}/include/SDL2
)

# Tìm File thư viện (Hỗ trợ cả file .a/.lib rời rạc hoặc để trong thư mục lib)
link_directories(
    ${SDL2_DIR}/lib/x64
    ${SDL2_DIR}/lib
    ${SDL2_DIR}
    ${SDL2_IMAGE_DIR}/lib/x64
    ${SDL2_IMAGE_DIR}/lib
    ${SDL2_IMAGE_DIR}
    ${SDL2_TTF_DIR}/lib/x64
    ${SDL2_TTF_DIR}/lib
    ${SDL2_TTF_DIR}
    ${SDL2_MIXER_DIR}/lib/x64
    ${SDL2_MIXER_DIR}/lib
    ${SDL2_MIXER_DIR}
)

file(GLOB SOURCES "src/*.cpp")

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} 
    mingw32 
    SDL2main 
    SDL2 
    SDL2_image 
    SDL2_ttf
    SDL2_mixer
)

# Script tự động copy .dll của DLL MinGW (nếu tìm thấy) sang cùng thư mục gốc
function(copy_dlls DIR)
    file(GLOB DLLS "${DIR}/lib/x64/*.dll" "${DIR}/lib/*.dll" "${DIR}/*.dll")
    foreach(DLL ${DLLS})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${DLL}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
        )
    endforeach()
endfunction()

copy_dlls(${SDL2_DIR})
copy_dlls(${SDL2_IMAGE_DIR})
copy_dlls(${SDL2_TTF_DIR})
copy_dlls(${SDL2_MIXER_DIR})
