# Based on CIRCT's AddCIRCT.cmake:
# https://github.com/llvm/circt/blob/main/cmake/modules/AddCIRCT.cmake

include_guard()

function(add_quirky_dialect dialect dialect_namespace)
  add_mlir_dialect(${ARGV})
  add_dependencies(quirky-headers MLIR${dialect}IncGen)
endfunction()

function(add_quirky_interface interface)
  add_mlir_interface(${ARGV})
  add_dependencies(quirky-headers MLIR${interface}IncGen)
endfunction()

function(add_quirky_doc doc_filename command output_file output_directory)
  set(LLVM_TARGET_DEFINITIONS ${doc_filename}.td)
  tablegen(MLIR ${output_file}.md ${command})
  set(GEN_DOC_FILE ${QUIRKY_BINARY_DIR}/docs/${output_directory}${output_file}.md)
  add_custom_command(
          OUTPUT ${GEN_DOC_FILE}
          COMMAND ${CMAKE_COMMAND} -E copy
                  ${CMAKE_CURRENT_BINARY_DIR}/${output_file}.md
                  ${GEN_DOC_FILE}
          DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${output_file}.md)
  add_custom_target(${output_file}DocGen DEPENDS ${GEN_DOC_FILE})
  add_dependencies(quirky-doc ${output_file}DocGen)
endfunction()

function(add_quirky_library name)
  add_mlir_library(${ARGV})
  add_quirky_library_install(${name})
endfunction()

# Adds a QUIRKY library target for installation.  This should normally only be
# called from add_quirky_library().
function(add_quirky_library_install name)
  install(TARGETS ${name} COMPONENT ${name} EXPORT QUIRKYTargets)
  set_property(GLOBAL APPEND PROPERTY QUIRKY_ALL_LIBS ${name})
  set_property(GLOBAL APPEND PROPERTY QUIRKY_EXPORTS ${name})
endfunction()

function(add_quirky_dialect_library name)
  set_property(GLOBAL APPEND PROPERTY QUIRKY_DIALECT_LIBS ${name})
  add_quirky_library(${ARGV} DEPENDS quirky-headers)
endfunction()

function(add_quirky_conversion_library name)
  set_property(GLOBAL APPEND PROPERTY QUIRKY_CONVERSION_LIBS ${name})
  add_quirky_library(${ARGV} DEPENDS quirky-headers)
endfunction()

function(add_quirky_translation_library name)
  set_property(GLOBAL APPEND PROPERTY QUIRKY_TRANSLATION_LIBS ${name})
  add_quirky_library(${ARGV} DEPENDS quirky-headers)
endfunction()
