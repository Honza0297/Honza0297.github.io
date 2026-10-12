# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

# If CMAKE_DISABLE_SOURCE_CHANGES is set to true and the source directory is an
# existing directory in our source tree, calling file(MAKE_DIRECTORY) on it
# would cause a fatal error, even though it would be a no-op.
if(NOT EXISTS "/home/jaberan/esp/v5.5.1/esp-idf/components/bootloader/subproject")
  file(MAKE_DIRECTORY "/home/jaberan/esp/v5.5.1/esp-idf/components/bootloader/subproject")
endif()
file(MAKE_DIRECTORY
  "/home/jaberan/repos/hub/Honza0297.github.io/examples/C6/assignment-05-esp-now/build/bootloader"
  "/home/jaberan/repos/hub/Honza0297.github.io/examples/C6/assignment-05-esp-now/build/bootloader-prefix"
  "/home/jaberan/repos/hub/Honza0297.github.io/examples/C6/assignment-05-esp-now/build/bootloader-prefix/tmp"
  "/home/jaberan/repos/hub/Honza0297.github.io/examples/C6/assignment-05-esp-now/build/bootloader-prefix/src/bootloader-stamp"
  "/home/jaberan/repos/hub/Honza0297.github.io/examples/C6/assignment-05-esp-now/build/bootloader-prefix/src"
  "/home/jaberan/repos/hub/Honza0297.github.io/examples/C6/assignment-05-esp-now/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/jaberan/repos/hub/Honza0297.github.io/examples/C6/assignment-05-esp-now/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/jaberan/repos/hub/Honza0297.github.io/examples/C6/assignment-05-esp-now/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
