
if (BUILD_STATIC_EXECUTABLES)
    INSTALL(FILES consolidate.py task_scatterplot.py DESTINATION bin
            PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
            GROUP_EXECUTE GROUP_READ
            WORLD_EXECUTE WORLD_READ)
else()
    INSTALL(FILES apex_exec apex_pthread_exec consolidate.py task_scatterplot.py DESTINATION bin
            PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
            GROUP_EXECUTE GROUP_READ
            WORLD_EXECUTE WORLD_READ)
endif()

