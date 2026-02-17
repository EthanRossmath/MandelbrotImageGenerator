# Mandelbrot Set BMP Generator

![Example Mandelbrot Set](/images/test.bmp)

The purpose of this respository is to gain experience using various parallel programming libraries and techniques in C++ to generate the Mandelbrot set in a hand-made .bmp format. Currently, the Mandelbrot set can be generated using C++ standard libraries like threads and futures, as well as third party libraries like OpenMP. See [mandelbrot.h](/src/mandelbrot.h) for the various implementations.

To generate your very own Mandelbrot set you have a choice of three algorithms, summarized the in the table below. 

| Algorithm | Library | Standard? |
| ------ | -------- | ------- |
| genFractalThreads | threads | yes |
| genFractalFutures | future | yes |
| genFractalOpenMP | OpenMP | no |
