/*
原创性：独立实现
*/

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>

#include "tracing_Whitted.hpp"
#include "tracing_MC.hpp"
#include "fxaa.hpp"

#include "scene_parser.hpp"
#include "image.hpp"
#include "camera.hpp"
#include "group.hpp"

#include <omp.h>
#include <chrono>
#include <iomanip>

int main(int argc, char *argv[]) {

    for (int argNum = 1; argNum < argc; ++argNum) {
        std::cout << "Argument " << argNum << " is: " << argv[argNum] << std::endl;
    }

    if (argc != 3) {
        std::cout << "Usage: ./bin/PA1 <input scene file> <output bmp file>" << std::endl;
        return 1;
    }
    std::string inputFile = argv[1];
    std::string outputFile = argv[2];  // only bmp is allowed.

    SceneParser Parser(inputFile.c_str());

    int W = Parser.getCamera()->getWidth();
    int H = Parser.getCamera()->getHeight();

    for (int i = 0; i < Parser.getNumTextures(); i++)
        Parser.getTexture(i)->gammaCorrection(Parser.getGamma());

    Image image(W, H);

    auto start = std::chrono::high_resolution_clock::now();

    if (Parser.getModel() == 0) {
        printf("model = Whitted-Style Ray Tracing\n");
        #pragma omp parallel for collapse(2)\
            schedule(guided) num_threads(Parser.getOmpThreads())
        for (int y = 0; y < H; y++)
            for (int x = 0; x < W; x++)
                image.SetPixel(x, y, tracingWhitted(x, y, Parser));
    }

    else {
        printf("model = Monte Carlo Ray Tracing\n");
        std::cout << std::fixed << std::setprecision(1);
        int cnt = 0;
        #pragma omp parallel for collapse(2)\
            schedule(guided) num_threads(Parser.getOmpThreads())
        for (int y = 0; y < H; y++)
            for (int x = 0; x < W; x++) {
                image.SetPixel(x, y, tracingMC(x, y, Parser));
                if (x + 1 == H) {
                    #pragma omp critical
                    {
                        ++cnt;
                        std::cout << "\rrate = " << cnt << " / " << H << " = " <<
                            100. * cnt / H << "%" << std::flush;
                        if (cnt % 80 == 0)
                            image.SaveImage(outputFile.c_str());
                    }
                }
            }
        std::cout << std::endl;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast <std::chrono::milliseconds> (end - start);

    printf("rendering time: %.2lfs\n", duration.count() / 1000.);

    if (Parser.getAntialias() & 2) fxaa(image);
    image.gammaCorrection(1 / Parser.getGamma());
    image.SaveImage(outputFile.c_str());

    return 0;
}
