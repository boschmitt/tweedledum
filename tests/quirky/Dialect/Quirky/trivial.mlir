// RUN: quirky-opt %s -allow-unregistered-dialect | FileCheck %s

// CHECK-LABEL: func @main(%{{.*}}: !quirky.qubit, %{{.*}}: !quirky.qubit) -> !quirky.qubit {
func @main(%q0: !quirky.qubit, %q1: !quirky.qubit) -> !quirky.qubit {
    // CHECK: %{{.*}} = quirky.x %{{.*}} : !quirky.qubit, !quirky.qubit
    %1 = quirky.x %q0, %q1 : !quirky.qubit, !quirky.qubit
    // CHECK: %{{.*}} = quirky.x adj %{{.*}} : !quirky.qubit
    %2 = quirky.x adj %1 : !quirky.qubit
    // CHECK: return %{{.*}}
    return %q0 : !quirky.qubit
// CHECK: }
}