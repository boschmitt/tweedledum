# get_property(dialect_libs GLOBAL PROPERTY MLIR_DIALECT_LIBS)
# get_property(conversion_libs GLOBAL PROPERTY MLIR_CONVERSION_LIBS)

# set(LLVM_LINK_COMPONENTS
#   Support
#   )

# set(LIBS
#         ${dialect_libs}
#         ${conversion_libs}
        
        
# )
# add_llvm_tool(quirky-opt quirky-opt.cpp)

# llvm_update_compile_flags(quirky-opt)
# target_link_libraries(quirky-opt PRIVATE ${LIBS})

set(LLVM_LINK_COMPONENTS
    Support
)

add_llvm_tool(quirky-opt quirky-opt.cpp)
llvm_update_compile_flags(quirky-opt)

target_link_libraries(quirky-opt
    PRIVATE
    TweedledumQuirky

    MLIRIR
    MLIROptLib
    MLIRSupport
)

