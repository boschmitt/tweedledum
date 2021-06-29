# Changelog
All notable changes to this project will be documented in this file.


## [Unreleased]


## [1.1.0] - 2021-06-29
In this release there was many cosmetic changes, such as using clang-format on
all sources and restructuring the project's directory to more closely follow the
pitchfork layout conventions.

### Added
- Analysis pass to count operators.
- Analysis pass to compute instructions' ALAP/ASAP layers.
- Analysis pass to compute critical path.
- Analysis pass to cut circuit.
- Gate cancellation pass.
- Pass to inverse (take adjoint) of a circuit.
- Crude QASM 2.0 parser.
- CX-Dihedral synthesis method.
- Bridge operator.
- Bridge decomposition pass.
- Bridge mapping pass.
- Sqrt(X) operator.
- TFC parser.
- Allow user to define var order for expressions.
- Limited support for `arm64`, `ppc64le` and `s390x`.

### Change
- Rename `depth` pass to `compute_depth`.
- Restructure of the code base.
- Make `Cbit`, `Qubit`, and `Instruction` constructors public.
- Invalidate ancilla reference on release.
- Change `Unitary` operator global phase behavior.
- Rename `euler_decomp` to `one_qubit_decomp`.

### Deprecated
- Circuit `size()` method. (Use `num_instructions` instead.)

### Fixed:
- Operator must clone the ConcreteOp class on copy.


## [1.0.0] - 2021-03-29
- Initial stable version. (Before there was only darkness.)


[Unreleased]: https://github.com/boschmitt/tweedledum/compare/v1.1.0...HEAD
[1.1.0]: https://github.com/boschmitt/tweedledum/compare/v1.0.0...v1.1.0
[1.0.0]: https://github.com/boschmitt/tweedledum/releases/tag/v0.0.1