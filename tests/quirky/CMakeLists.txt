configure_lit_site_cfg(
        ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.py.in
        ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py
        MAIN_CONFIG
        ${CMAKE_CURRENT_SOURCE_DIR}/lit.cfg.py
)

set(QUIRKY_OPT_TEST_DEPENDS
        FileCheck count not
        quirky-opt
        )

add_lit_testsuite(check-quirky-opt "Running the quirky-opt regression tests"
        ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS ${QUIRKY_OPT_TEST_DEPENDS}
        )

set_target_properties(check-quirky-opt PROPERTIES FOLDER "Tests")

add_lit_testsuites(QUIRKY_OPT ${CMAKE_CURRENT_SOURCE_DIR} DEPENDS ${QUIRKY_OPT_TEST_DEPENDS})
