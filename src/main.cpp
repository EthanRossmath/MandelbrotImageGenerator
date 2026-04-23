#include "mandelbrot.h"
#include <filesystem>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>


//This is an implementation using the future-based mandelbrot set generator.
//The syntax for the other algorithms is essentially the same.

const std::filesystem::path outDir = "images";
const std::string defaultFileName = "test.bmp";

void write_to_file(MandelbrotImage& mand_im, std::string file_name)
{
    std::filesystem::path outPath = outDir / file_name;

    mand_im.writeBMP(outPath.string());

}

bool is_numeric(const std::string& str)
{
    if (str.empty())
    {
        return false;
    }

    std::string::const_iterator it = std::find_if(str.begin(), str.end(), [](char c){
        return !std::isdigit(c);
    });

    return it == str.end();
}



int main(int argc, char* argv[])
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

    // If no arguments are given, default to doing slowest generation 
    // algorithm
    if (argc == 1)
    {
        mi.genFractal();
        write_to_file(mi, defaultFileName);
        return 0;
    }

    // Find file name
    std::string fileName{};
    for (int i{1}; i < argc; ++i)
    {
        if (std::strstr(argv[i], ".bmp"))
        {
            fileName = std::string(argv[i]);
            break;
        }
    }

    if (fileName.length() == 0)
    {
        fileName = defaultFileName;
    }

    // Find algorithm type.
    // Key: 'd' == default (no parallelization)
    //      'o' == OpenMP
    //      'f' == futures
    //      't' == threads
    char method{'d'};
    for (int i{1}; i < argc; i++)
    {

        if (std::strstr(argv[i], "open"))
        {
            method = 'o';
            break;
        }
        else if (std::strstr(argv[i], "future"))
        {
            method = 'f';
            break;
        }
        else if (std::strstr(argv[i], "thread"))
        {
            method = 't';
            break;
        }
    }

    if (method == 'd')
    {
        mi.genFractal();
        write_to_file(mi, fileName);
        return 0;
    }

    if (method == 'o')
    {
        mi.genFractalOpenMP();
        write_to_file(mi, fileName);
        return 0;
    }

    // Get number of threads, futures, or cores for threads, futures, OpenMP,
    // respectively.

    int var{1};
    for (int i{1}; i < argc; ++i)
    {
        if (is_numeric(std::string(argv[i])))
        {
            var = std::stoi(argv[i]);
        }
    }

    if (method == 'f')
    {
        mi.genFractalFuture(var);
        write_to_file(mi, fileName);
        return 0;
    }

    if (method == 't')
    {
        mi.genFractalThread(var);
        write_to_file(mi, fileName);
        return 0;
    }
    
    return 0;
}
