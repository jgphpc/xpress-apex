if(HPX_WITH_APEX)
    include(CMakeLists.lsu)
    return()
else()
    include(CMakeLists.standalone)
    return()
endif()

