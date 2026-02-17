#pragma once

// custom header files
#include "complex.h"
#include "BMP.h"

// C++ standard libraries
#include <cmath>
#include <string>
#include <thread>
#include <algorithm>
#include <future>

// Third party libraries
#include <omp.h>


// Configuration Constants
namespace MandelbrotConstants
{
    // Specifies the number of iterations that the Mandelbrot recurrence relation
    // will run.
    const size_t maxIterations{80};

    // If any complex number's modulus exceeds 4.0, then it is not in the 
    // Mandelbrot set
    const double maxNormSquare{4.0};

    // Width and height of the image in pixels
    const size_t imageWidth{3840};
    const size_t imageHeight{2160};
}

// An algorithm to assign a colour to a pixel based on when the Mandelbrot
// algorithm terminates.
Pixel valueToRGB(size_t value)
{
    double colourParam = static_cast<double>(value) / MandelbrotConstants::maxIterations;

    Pixel pixel{};

    pixel.red = static_cast<uint8_t>(16 * (1 - colourParam) * (1 - colourParam) 
    * colourParam * colourParam  *  255);
    pixel.green = static_cast<uint8_t>(9 * (1 - colourParam) * colourParam 
    * colourParam * colourParam * 255);
    pixel.blue = static_cast<uint8_t>(9 * (1 - colourParam) * (1 - colourParam) 
    * (1 - colourParam) * colourParam  *  255);

    return pixel;
}

// The core Mandelbrot algorithm to be run pixel-by-pixel
size_t mandelbrot1(const ComplexNumber& num)
{
    ComplexNumber iterNum{0.0, 0.0};

    for (size_t i{1}; i < MandelbrotConstants::maxIterations; ++i)
    {
        iterNum = (iterNum * iterNum) + num;

        if (normsquare(iterNum) > MandelbrotConstants::maxNormSquare)
        {
            return i;
        }
    }
    return 0;
}

// A helper rescaling function to correctly map pixels to the complex plane
double rescaling(const size_t pixelUpperBound, double lower, double upper, size_t pos)
{
    return ((upper - lower) / pixelUpperBound) * pos + lower;
}


// The core class for creating Mandelbrot set images
class MandelbrotImage
{
private: 
    // Raw pixel data
    BMP m_image; 

    // Horizontal and vertical bounds in the complex plane of image
    double m_horLower{};
    double m_horUpper{};
    double m_verLower{};
    double m_verUpper{};

    // Algorithm each thread will run to compute rgb values of each pixel.
    // Each thread will work with disjoint chunks of pixels, with the 
    // domain being specified by the lower scanline (inclusive) and the
    // upper scanline (excluded). This prevents race conditions and 
    // optimizes for contiguous memory access.
    void genBetweenScanlines(size_t lowerScanLine, size_t upperScanLine)
    {
        for (size_t y{lowerScanLine}; y < upperScanLine; ++y)
        {
            // Need to rescale the vertical pixel coordinate (between 0 and 
            // the image heigh specified in MandelbrotConstants) and the
            // imaginary complex plane coordinate (specified by m_verLower and
            // m_verUpper)
            double imaginary = rescaling(MandelbrotConstants::imageHeight,
            m_verLower, m_verUpper, y);

            for (size_t x{0}; x < MandelbrotConstants::imageWidth; ++x)
            {
                // Similar scaling required for horizontal pixel coordinate
                // to real complex plane coordinate
                double real = rescaling(MandelbrotConstants::imageWidth, 
                m_horLower, m_horUpper, x);

                // Call the main mandelbrot set algorithm on rescaled real
                // and imaginary coordinates, using custom complex number header
                size_t value = mandelbrot1(ComplexNumber {real, imaginary});

                // Call pixel colour generator using helper function.
                m_image.getPixel(x, y) = valueToRGB(value);
            }

        }
    }

public:
    MandelbrotImage(double horLower, double horUpper, double verLower, double verUpper)
    : m_image{MandelbrotConstants::imageWidth, MandelbrotConstants::imageHeight}
    , m_horLower{horLower}
    , m_horUpper{horUpper}
    , m_verLower{verLower}
    , m_verUpper{verUpper}
    {  
        assert((horLower < horUpper) && (verLower < verUpper));
    }

    // Non parallel implementation of Mandelbrot set generator (VERY SLOW)
    void genFractal()
    {
        genBetweenScanlines(0, MandelbrotConstants::imageHeight);
    }

    // Thread-based generator
    void genFractalThread(const size_t numThreads)
    {
        // Need to break up the .bmp image into non-overlapping domains.
        // The number of domains determined by the number of threads used
        const size_t scanPartiton = MandelbrotConstants::imageHeight / numThreads;

        // Create a vector threads
        std::vector<std::thread> threads;

        // For each thread, compute the upper and lower bounds of scanline
        // coordinates and initilize computation of pixel values
        for (size_t i{0}; i < numThreads; ++i)
        {
            size_t start = i * scanPartiton;
            size_t end = (i == numThreads - 1) ? MandelbrotConstants::imageHeight : 
            (i + 1) * scanPartiton;

            threads.emplace_back(&MandelbrotImage::genBetweenScanlines, this, 
            start, end);
        }

        // Join the threads together upon completion
        for (auto& t : threads)
        {
            t.join();
        }
    }

    // Future-based implementation 
    void genFractalFuture(const size_t numFutures)
    {
        // Compute number of  non-overlapping domains
        const size_t scanPartiton = MandelbrotConstants::imageHeight / numFutures;

        // Create a vector of futures
        std::vector<std::future<void>> futures;

        // For each future, need to break up the domain of scanlines to 
        // prevent race conditions
        for (size_t i{0}; i < numFutures; ++i)
        {
            size_t start = i * scanPartiton;
            size_t end = (i == numFutures - 1) ? MandelbrotConstants::imageHeight : 
            (i + 1) * scanPartiton;

            // Each future is a void function, updating the m_image private
            // member variable
            std::future<void> f = std::async(std::launch::async,
                [&, i, start, end]() -> void
                {
                    genBetweenScanlines(start, end);
                }
            );

            // Can only change ownership of a future
            futures.push_back(std::move(f));
        }

        // Get all future values when ready
        for (auto& f : futures)
        {
            f.get();
        }
    }

    void genFractalOpenMP(const size_t numThreads)
    {

    }

    // Call the .write method from the BMP class on m_image to generate image
    void writeBMP(const std::string& fname)
    {
        m_image.write(fname);
    }
    
};
