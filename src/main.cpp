#include "mandelbrot.h"
#include <filesystem>

//This is an implementation using the future-based mandelbrot set generator.
//The syntax for the other algorithms is essentially the same.

int main()
{
    // To create a Mandelbrot set, you need to first specify the bounds of the 
    // horizontal axis and the vertical axis.
    const double horizontalLeftLimit{-2}; // horizontal lower bound
    const double horizontalRightLimit{0.5}; // horizontal upper bound
    const double verticalLowerLimit{-1.2}; // vertical lower bound
    const double verticalUpperLimit{1.2}; // vertical upper bound

    // A Mandelbrot set image is an object in the MandelbrotImage class defined 
    // in the header file mandelbrot.h
    MandelbrotImage mi{
        horizontalLeftLimit,
        horizontalRightLimit,
        verticalLowerLimit,
        verticalUpperLimit
    };

    // Below is the future-based implementation of the Mandelbrot-set generator
    const int numFutures{8}; // my machine has 8 cores, hence I chose 8
    mi.genFractalFuture(numFutures); // the number of futures must be specified
    // when generating your fractal

    // The more primitive threads-based algorithm should be invoked via
    // mi.genFractalThreads(numThreads) where you must specify the number of threads
    // My machine has 8 cpu cores, so I would choose numThreads = 8.

    // The OpenMP-based implementation follows a similar syntax 
    // mi.genFractalOpenMP()

    // Try using the other algorithms to compare the speed of these approaches.

    // The directory where the .bmp file will be saved is specified below
    const std::filesystem::path outDir = "images";

    std::filesystem::path outPath = outDir / "test.bmp";

    mi.writeBMP(outPath.string());
    return 0;
}
