# SPDX-FileCopyrightText: Generative Bionics S.R.L.
# SPDX-License-Identifier: BSD-3-Clause

# Detect if we are doing a standalone build of the bindings, using an external dinrail
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(DINRAIL_PYTHON_BINDINGS_BUILD_STANDALONE ON)
  set(DINRAIL_COMPILE_PYTHON_BINDINGS ON)
else()
  set(DINRAIL_PYTHON_BINDINGS_BUILD_STANDALONE FALSE)
endif()

if(DINRAIL_PYTHON_BINDINGS_BUILD_STANDALONE)
  cmake_minimum_required(VERSION 3.28.3)
  project(dinrailBindings)
  include(GNUInstallDirs)
  find_package(dinrail REQUIRED)
endif()

if(DINRAIL_COMPILE_PYTHON_BINDINGS)
    find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
    find_package(pybind11 REQUIRED)

    set(NEW_LINE "\n")

    option(DINRAIL_DETECT_ACTIVE_PYTHON_SITEPACKAGES
        "Do you want dinrail to detect and use the active site-package directory? (it could be a system dir)"
        FALSE)

    if(NOT DEFINED DINRAIL_PYTHON_INSTALL_DIR)
      if(DINRAIL_DETECT_ACTIVE_PYTHON_SITEPACKAGES)
        set(DINRAIL_PYTHON_INSTALL_DIR ${Python3_SITELIB})
      else()
        execute_process(COMMAND ${Python3_EXECUTABLE} -c "import os;import sysconfig;relative_site_packages = sysconfig.get_path('purelib').replace(sysconfig.get_path('data'), '').lstrip(os.path.sep);print(relative_site_packages)"
                        OUTPUT_VARIABLE _PYTHON_INSTDIR)
        string(STRIP ${_PYTHON_INSTDIR} _PYTHON_INSTDIR_CLEAN)
        file(TO_CMAKE_PATH "${_PYTHON_INSTDIR_CLEAN}" DINRAIL_PYTHON_INSTALL_DIR)
      endif()
    endif()
    set(PYTHON_INSTDIR ${DINRAIL_PYTHON_INSTALL_DIR}/dinrail)

    # Folder of the Python package within the build tree.
    # It is used for the Python tests.
    set(DINRAIL_PYTHON_PACKAGE "${CMAKE_BINARY_DIR}/dinrail")

    # Add the bindings directory
    add_subdirectory(python)

    # Create the __init__.py file of the installed package
    set(PYTHON_INIT_NAME ${CMAKE_CURRENT_BINARY_DIR}/init_install.py)
    file(WRITE ${PYTHON_INIT_NAME} "")

    # If we are on Windows and BUILD_SHARED_LIBS is ON, handle the fact that
    # the Python interpreter does not look into PATH to find dll (see https://docs.python.org/3.8/library/os.html#os.add_dll_directory)
    if(WIN32 AND BUILD_SHARED_LIBS)
        if(IS_ABSOLUTE "${PYTHON_INSTDIR}")
            set(PYTHON_FULL_INSTDIR "${PYTHON_INSTDIR}")
        else()
            set(PYTHON_FULL_INSTDIR "${CMAKE_INSTALL_PREFIX}/${PYTHON_INSTDIR}")
        endif()
        file(RELATIVE_PATH RELATIVE_PATH_BETWEEN_INIT_PY_AND_DLL_DIRECTORY ${PYTHON_FULL_INSTDIR} ${CMAKE_INSTALL_FULL_BINDIR})
        file(APPEND ${PYTHON_INIT_NAME} "import os${NEW_LINE}")
        file(APPEND ${PYTHON_INIT_NAME} "library_dll_path = os.path.join(os.path.dirname(__file__),'${RELATIVE_PATH_BETWEEN_INIT_PY_AND_DLL_DIRECTORY}')${NEW_LINE}")
        file(APPEND ${PYTHON_INIT_NAME} "# Avoid to call add_dll_directory if not necessary,${NEW_LINE}")
        file(APPEND ${PYTHON_INIT_NAME} "# for example if the library to find are already found in the proper location in a conda environment${NEW_LINE}")
        file(APPEND ${PYTHON_INIT_NAME} "if(library_dll_path != os.path.join(os.environ.get('CONDA_PREFIX', ''),'Library','bin') and library_dll_path != os.path.join(os.environ.get('CONDA_PREFIX', ''),'bin')):${NEW_LINE}")
        file(APPEND ${PYTHON_INIT_NAME} "    if(os.path.exists(library_dll_path)):${NEW_LINE}")
        file(APPEND ${PYTHON_INIT_NAME} "        os.add_dll_directory(library_dll_path)${NEW_LINE}")
    endif()

    file(APPEND ${PYTHON_INIT_NAME} "from .bindings import *${NEW_LINE}")

    # Install the __init__.py file
    install(FILES ${PYTHON_INIT_NAME}
            DESTINATION ${PYTHON_INSTDIR}
            RENAME __init__.py)

    # Create the __init__.py file of the build tree package, used to run the tests without installing.
    set(PYTHON_BUILD_TREE_INIT_NAME ${DINRAIL_PYTHON_PACKAGE}/__init__.py)
    file(WRITE ${PYTHON_BUILD_TREE_INIT_NAME} "")
    if(WIN32 AND BUILD_SHARED_LIBS AND NOT DINRAIL_PYTHON_BINDINGS_BUILD_STANDALONE)
        file(APPEND ${PYTHON_BUILD_TREE_INIT_NAME} "import os${NEW_LINE}")
        file(APPEND ${PYTHON_BUILD_TREE_INIT_NAME} "os.add_dll_directory(r'${CMAKE_RUNTIME_OUTPUT_DIRECTORY}')${NEW_LINE}")
    endif()
    file(APPEND ${PYTHON_BUILD_TREE_INIT_NAME} "from .bindings import *${NEW_LINE}")

    # Install pip metadata files to ensure that dinrail installed via CMake is listed by pip list
    # See https://packaging.python.org/specifications/recording-installed-packages/
    # and https://packaging.python.org/en/latest/specifications/core-metadata/#core-metadata
    option(DINRAIL_PYTHON_PIP_METADATA_INSTALL "Use CMake to install Python pip metadata. Set to off if some other tool already installs it." ON)
    mark_as_advanced(DINRAIL_PYTHON_PIP_METADATA_INSTALL)
    set(DINRAIL_PYTHON_PIP_METADATA_INSTALLER "cmake" CACHE STRING "Specify the string to identify the pip Installer. Default: cmake, change this if you are using another tool.")
    mark_as_advanced(DINRAIL_PYTHON_PIP_METADATA_INSTALLER)
    if(DINRAIL_PYTHON_PIP_METADATA_INSTALL)
      file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/METADATA "")
      file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/METADATA "Metadata-Version: 2.1${NEW_LINE}")
      file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/METADATA "Name: dinrail${NEW_LINE}")
      file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/METADATA "Version: ${dinrail_VERSION}${NEW_LINE}")
      file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/INSTALLER "${DINRAIL_PYTHON_PIP_METADATA_INSTALLER}${NEW_LINE}")
      install(
        FILES "${CMAKE_CURRENT_BINARY_DIR}/METADATA" "${CMAKE_CURRENT_BINARY_DIR}/INSTALLER"
        DESTINATION ${DINRAIL_PYTHON_INSTALL_DIR}/dinrail-${dinrail_VERSION}.dist-info)
    endif()

endif()
