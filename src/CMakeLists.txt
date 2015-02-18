
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}   -O0 -g3 -ggdb -fno-inline-functions")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

#add_compile_options(-std=c++11 -Wall -Wextra -O0 -g3 -ggdb3 -fno-inline-functions) #todo: exchange with add_compile_options

append_cxx_compiler_flags("-std=c++11 -ggdb3 -Wall -Wextra" "GCC" CMAKE_CXX_FLAGS)

#append_cxx_compiler_flags("-Weffc++ -pedantic -Waggregate-return -Wcast-align -Wcast-qual -Wconversion -Wdisabled-optimization -Wfloat-equal -Wformat=2 -Wformat-nonliteral -Wformat-security  -Wformat-y2k -Wimport  -Winit-self  -Winline -Winvalid-pch -Wmissing-field-initializers -Wmissing-format-attribute -Wmissing-include-dirs -Wmissing-noreturn -Wpacked  -Wpadded -Wpointer-arith -Wredundant-decls -Wshadow -Wstack-protector -Wstrict-aliasing=2 -Wswitch-default -Wswitch-enum -Wunreachable-code -Wunused -Wunused-parameter -Wvariadic-macros -Wwrite-strings" "GCC" CMAKE_CXX_FLAGS)




#if(OPT_PERFORMANCE)
#	append_cxx_compiler_flags("-O3" "GCC" CMAKE_CXX_FLAGS)
#else()
#	append_cxx_compiler_flags("-O0 -g3 -ggdb -fno-inline-functions" "GCC" CMAKE_CXX_FLAGS)
#endif()
#append_cxx_compiler_flags("-I/home/niki/opt/include -L/home/niki/opt/lib" "GCC" CMAKE_CXX_FLAGS)


add_executable (sequencing main.cpp)
target_link_libraries    (sequencing -ldl)
target_link_libraries    (sequencing sdsl divsufsort divsufsort64)
target_link_libraries    (sequencing ${GLOG_LIBRARY})
target_link_libraries    (sequencing ${GTEST_LIBRARY})
target_link_libraries    (sequencing ${GFLAGS_LIBRARY})
target_link_libraries    (sequencing ${CMAKE_THREAD_LIBS_INIT} )
#cotire(sequencing)

