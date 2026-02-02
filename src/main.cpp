#include "mandelbrot.h"
#include <filesystem>

int main()
{
    const int numThreads{8};
    MandelbrotImage mi{-2, 0.5, -1.2, 1.2};
    mi.genFractalThread(numThreads);
    //mi.genFractal();

    const std::filesystem::path outDir = "images";

    std::filesystem::path outPath = outDir / "test.bmp";

    mi.writeBMP(outPath.string());
    return 0;
}
