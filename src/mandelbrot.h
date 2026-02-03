#pragma once

#include "complex.h"
#include "BMP.h"
#include <cmath>
#include <string>
#include <thread>
#include <algorithm>
#include <future>


namespace MandelbrotConstants
{
    const size_t maxIterations{80};
    const size_t imageWidth{3840};
    const size_t imageHeight{2160};
    const double maxNormSquare{4.0};
}

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

double rescaling(const size_t pixelUpperBound, double lower, double upper, size_t pos)
{
    return ((upper - lower) / pixelUpperBound) * pos + lower;
}


class MandelbrotImage
{
private: 
    BMP m_image;
    double m_horLower{};
    double m_horUpper{};
    double m_verLower{};
    double m_verUpper{};

    void genBetweenScanlines(size_t lowerScanLine, size_t upperScanLine)
    {
        for (size_t y{lowerScanLine}; y < upperScanLine; ++y)
        {
            double imaginary = rescaling(MandelbrotConstants::imageHeight,
            m_verLower, m_verUpper, y);

            for (size_t x{0}; x < MandelbrotConstants::imageWidth; ++x)
            {
                double real = rescaling(MandelbrotConstants::imageWidth, 
                m_horLower, m_horUpper, x);
                size_t value = mandelbrot1(ComplexNumber {real, imaginary});
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

    void genFractal()
    {
        genBetweenScanlines(0, MandelbrotConstants::imageHeight);
    }

    void genFractalThread(const size_t numThreads)
    {
        const size_t scanPartiton = MandelbrotConstants::imageHeight / numThreads;

        std::vector<std::thread> threads;
        for (size_t i{0}; i < numThreads; ++i)
        {
            size_t start = i * scanPartiton;
            size_t end = (i == numThreads - 1) ? MandelbrotConstants::imageHeight : 
            (i + 1) * scanPartiton;

            threads.emplace_back(&MandelbrotImage::genBetweenScanlines, this, 
            start, end);
        }

        for (auto& t : threads)
        {
            t.join();
        }
    }

    void genFractalFuture(const size_t numFutures)
    {
        const size_t scanPartiton = MandelbrotConstants::imageHeight / numFutures;

        std::vector<std::future<void>> futures;
        for (size_t i{0}; i < numFutures; ++i)
        {
            size_t start = i * scanPartiton;
            size_t end = (i == numFutures - 1) ? MandelbrotConstants::imageHeight : 
            (i + 1) * scanPartiton;

            std::future<void> f = std::async(std::launch::async,
                [&, i, start, end]() -> void
                {
                    genBetweenScanlines(start, end);
                }
            );

            futures.push_back(std::move(f));
        }

        for (auto& f : futures)
        {
            f.get();
        }
    }


    void writeBMP(const std::string& fname)
    {
        m_image.write(fname);
    }
    
};
