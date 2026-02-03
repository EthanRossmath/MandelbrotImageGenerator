#include "mandelbrot.h"
#include <filesystem>

int main()
{
    const int numFutures{8};
    MandelbrotImage mi{-2, 0.5, -1.2, 1.2};
    mi.genFractalFuture(numFutures);
    //mi.genFractal();

    const std::filesystem::path outDir = "images";

    std::filesystem::path outPath = outDir / "test.bmp";

    mi.writeBMP(outPath.string());
    return 0;
}
