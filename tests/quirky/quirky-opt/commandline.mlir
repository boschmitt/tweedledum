// RUN: quirky-opt --help | FileCheck %s --check-prefix=HELP
// RUN: quirky-opt --show-dialects | FileCheck %s --check-prefix=DIALECT

// HELP: OVERVIEW: QUIRKY optimizer driver

// DIALECT: Available Dialects:
// DIALECT-NEXT: builtin
// DIALECT-NEXT: func
// DIALECT-NEXT: quirky
