# use it to get relative path rather than absolute path by __FILE__
function(force_redefine_file_macro_for_sources targetname)
    get_target_property(source_files "${targetname}" SOURCES)
    foreach(sourcefile ${source_files})
        # Get source file's current list of compile definitions.
        get_property(defs SOURCE "${sourcefile}"
                PROPERTY COMPILE_DEFINITIONS)
        # Get the relative path of the source file in project directory
        get_filename_component(filepath "${sourcefile}" ABSOLUTE)
        string(REPLACE ${PROJECT_SOURCE_DIR}/ "" relpath ${filepath})
        list(APPEND defs "__FILE__=\"${relpath}\"")
        # Set the updated compile definitions on the source file.
        set_property(
                SOURCE "${sourcefile}"
                PROPERTY COMPILE_DEFINITIONS ${defs}
        )
    endforeach()
endfunction()


# use it to set target without inputting common settings
function(mocker_build_target targetname targetsource)
    foreach(v IN LISTS ARGN)
        LIST(APPEND targetsource ${v})
    endforeach()
    add_executable(${targetname} ${targetsource})
    add_dependencies(${targetname} mocker)
    force_redefine_file_macro_for_sources(${targetname})  # __FILE__
    target_link_libraries(${targetname} ${LIB_LIB})
endfunction()

