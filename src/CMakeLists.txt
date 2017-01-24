add_executable (strinalyze main.cpp)
target_link_libraries    (strinalyze -ldl)
target_link_libraries    (strinalyze glog ${GLOG_LIBRARY})
target_link_libraries    (strinalyze gflags ${GFLAGS_LIBRARY})
target_link_libraries    (strinalyze ${CMAKE_THREAD_LIBS_INIT} )

