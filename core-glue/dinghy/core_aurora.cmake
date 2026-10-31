# SPDX-License-Identifier: GPL-3.0-only
# AURORA's own fake_fm1.h with its script/WAV host driver replaced, as FM-106's clang VFS overlay does.
include(${CMAKE_CURRENT_LIST_DIR}/dinghy.cmake)
set(_aurora ${CMAKE_SOURCE_DIR}/cores/aurora)
set(_aurora_vfs ${CMAKE_BINARY_DIR}/dinghy/aurora_vfs.yaml)
file(WRITE ${_aurora_vfs} "{ \"version\": 0, \"roots\": [ { \"type\": \"file\", \"name\": \"${_aurora}/tests/host.c\", \"external-contents\": \"${CMAKE_CURRENT_LIST_DIR}/aurora_host.c\" } ] }\n")
fm1_add_core(aurora ${CMAKE_CURRENT_LIST_DIR} SOURCE ${_aurora}
             UNIT core_aurora.c DESC core_aurora_desc.c FAMILY dinghy LICENSE GPL-3.0-only
             CREDITS "AURORA by twochannels, GPL-3.0-only. On Dinghy by zvenson and Felucca by Leo Kuroshita, Hugelton Instruments, with SLOOP's changes by isod89. Audio worker after FM-106 by PatttF, key sensing after Velo by Charles Vestal. Terminus Font by Dimitar Toshkov Zhekov, SIL OFL 1.1."
             CREDITS_FILES LICENSING.md ${CMAKE_CURRENT_LIST_DIR}/licences/aurora/Terminus-LICENSE.txt
             NO_GEN VERSION_FILE ./VERSION VERSION_REGEX "^([0-9]+\\.[0-9]+\\.[0-9]+)$"
             INCLUDES ${_aurora}/firmware/gen ${CMAKE_CURRENT_LIST_DIR}
             FLAGS "-ivfsoverlay" "${_aurora_vfs}")
set_property(SOURCE ${CMAKE_CURRENT_LIST_DIR}/core_aurora.c APPEND PROPERTY OBJECT_DEPENDS
             ${CMAKE_CURRENT_LIST_DIR}/aurora_host.c ${_aurora_vfs})
dinghy_core(aurora core_aurora.c)
file(STRINGS ${_aurora}/VERSION _aurora_version LIMIT_COUNT 1)
dinghy_golden_check(aurora SCRIPT golden_aurora.txt AT 3000 SOURCE ${_aurora}/firmware/src/aurora.c
                    DEFINES DINGHY_HOST APP_VER="${_aurora_version}"
                    INCLUDES ${_aurora}/firmware/src ${_aurora}/firmware/gen)
