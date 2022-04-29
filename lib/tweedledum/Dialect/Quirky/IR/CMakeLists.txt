add_quirky_dialect_library(TweedledumQuirky
    QuirkyDialect.cpp
    QuirkyOps.cpp

    ADDITIONAL_HEADER_DIRS
    ${QUIRKY_MAIN_INCLUDE_DIR}/tweedledum/Dialect/Quirky/

    DEPENDS
    MLIRQuirkyIncGen

    LINK_LIBS PUBLIC
    MLIRFunc
    MLIRIR
    MLIRInferTypeOpInterface
)

add_dependencies(quirky-headers MLIRQuirkyIncGen)
