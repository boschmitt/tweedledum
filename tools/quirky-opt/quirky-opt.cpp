/*------------------------------------------------------------------------------
| Part of Tweedledum Project.  This file is distributed under the MIT License.
| See accompanying file /LICENSE for details.
*-----------------------------------------------------------------------------*/
#include "mlir/InitAllDialects.h"
#include "mlir/Tools/mlir-opt/MlirOptMain.h"

#include "tweedledum/Dialect/Quirky/IR/QuirkyDialect.h"

int main(int argc, char** argv)
{
    mlir::DialectRegistry registry;

    // Register MLIR dialects.
    registry.insert<mlir::func::FuncDialect>();

    // Register QUIRKY dialects.
    registry.insert<tweedledum::quirky::QuirkyDialect>();
    
    return mlir::asMainReturnCode(
      mlir::MlirOptMain(argc, argv, "QUIRKY optimizer driver\n", registry,
        /*preloadDialectsInContext=*/false));
}
