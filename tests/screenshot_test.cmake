# A scripted run of the free set (stone_play --fresh) whose screenshot must
# have the SHA-256 recorded for it (ENHANCEMENTS.md, Tests). Run by CTest:
#   cmake -DPLAYER=... -DPACKAGE=... -DKEYS=... -DTICKS=... -DOUT=... -DSHA256=...
#         -P screenshot_test.cmake
# A changed picture fails it; OUT is left to look at.
execute_process(COMMAND "${PLAYER}" "${PACKAGE}" --fresh --ticks "${TICKS}" --keys "${KEYS}"
                        --screenshot "${OUT}"
                RESULT_VARIABLE result)
if(NOT result EQUAL 0)
    message(FATAL_ERROR "stone_play failed: ${result}")
endif()
file(SHA256 "${OUT}" hash)
if(NOT hash STREQUAL SHA256)
    message(FATAL_ERROR "${OUT} has SHA-256 ${hash}, not ${SHA256}")
endif()
