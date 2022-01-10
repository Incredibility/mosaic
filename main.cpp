#include <iostream>
#include <filesystem>
#include <chrono>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#define STBIR_SATURATE_INT

#include "stb_image.h"
#include "stb_image_resize.h"
#include "stb_image_write.h"

void prompt(const std::string promptStr, std::string& out) {
    std::cout << promptStr;
    std::cin >> out;
}

void prompt(const std::string promptStr, int& out) {
    while(1) {
        std::string input;
        prompt(promptStr, input);
        out = atoi(input.c_str());
        if (out >= 0)
            break;
        std::cout << " Enter a positive integer\n";
    }
}

void atoif(const char* a, int& out) {
    out = atoi(a);
}

void atoif(const char* a, float& out) {
    out = atof(a);
}

int resolveArg(const char* arg, const std::string str[2]) {
    for (int i = 0; i < 2; ++i)
        if (str[i] == arg)
            return 1;
    return 0;
}

template<typename T>
int resolveArg(const char* arg, const std::string str[2], T& out) {
    for (int i = 0; i < 2; ++i) {
        int strLen = str[i].length() + 1;
        if (!strncmp(arg, (str[i] + '=').c_str(), strLen)) {
            atoif(arg + strLen, out);
            return 1;
        }
    }
    return 0;
}

//template<bool blend>
//void assignPixel(int nRows, int nCols, int tileWidth, int tileHeight, int finalWidth, const std::vector<std::array<float, 3> >& averages,
//                 unsigned char* finalImage, const std::vector<unsigned char*>& tileImages) {
//    int nImages = tileImages.size();
//    int rowOffset = tileHeight * finalWidth;
//    for (int i = 0; i < nRows; ++i) {
//        int iIndex2 = i * rowOffset;
//        for (int j = 0; j < nCols; ++j) {
//            int leastDistance = INT_MAX;
//            int bestTile = 0;
//            int jIndex2 = j * tileWidth;
//            int jIndex3 = iIndex2 + jIndex2;
//            for (int z = 0; z < nImages; ++z) {
//                int distance = 0;
//                for (int k = 0; k < tileHeight; ++k) {
//                    int kIndex1 = jIndex3 + k * finalWidth;
//                    int kIndex2 = k * tileWidth;
//                    for (int l = 0; l < tileWidth; ++l) {
//                        int lIndex1 = 3 * (kIndex1 + l);
//                        int lIndex2 = 3 * (kIndex2 + l);
//                        for (int m = 0; m < 3; ++m) {
//                            int diff = finalImage[lIndex1 + m] - tileImages[z][lIndex2 + m];
//                            distance += diff * diff;
//                        }
//                    }
//                }
//                if (distance < leastDistance) {
//                    leastDistance = distance;
//                    bestTile = z;
//                }
//            }
//            for (int k = 0; k < tileHeight; ++k) {
//                int kIndex1 = jIndex3 + k * finalWidth;
//                int kIndex2 = k * tileWidth;
//                for (int l = 0; l < tileWidth; ++l) {
//                    int lIndex1 = 3 * (kIndex1 + l);
//                    int lIndex2 = 3 * (kIndex2 + l);
//                    for (int m = 0; m < 3; ++m)
//                        finalImage[lIndex1 + m] = !blend ? tileImages[bestTile][lIndex2 + m] : finalImage[lIndex1 + m] + tileImages[bestTile][lIndex2 + m];
//                }
//            }
//        }
//    }
//}

template<bool blend>
void assignPixel(int nRows, int nCols, int tileWidth, int tileHeight, int finalWidth, const std::vector<std::array<float, 3> >& averages,
                 const unsigned char* baseImageResized, unsigned char* finalImage, const std::vector<unsigned char*>& tileImages) {
    int nImages = tileImages.size();
    int rowOffset = tileHeight * finalWidth;
//    bool right = false;
    for (int i = 0; i < nRows; ++i) {
        int iIndex1 = i * nCols;
        int iIndex2 = i * rowOffset;
//        right = !right;
        for (int j = 0; j < nCols; ++j) {
            float leastDistance = 195076;
            int bestTile = 0;
            int jIndex1 = 3 * (iIndex1 + j);
            int jIndex2 = j * tileWidth;
            int jIndex3 = iIndex2 + jIndex2;
            for (int k = 0; k < nImages; ++k) {
                float diffs[3];
                for (int l = 0; l < 3; ++l) {
                    diffs[l] = averages[k][l] - baseImageResized[jIndex1 + l];
                    diffs[l] *= diffs[l];
                }
                float r = averages[k][0] + baseImageResized[jIndex1];
                float distance = (2 + r / 256) * diffs[0] + 4 * diffs[1] + (2 + (255 - r) / 256) * diffs[2];
                if (distance < leastDistance) {
                    leastDistance = distance;
                    bestTile = k;
                }
            }
//            if (right) {
//                for (int k = 0; k < 3; ++k) {
//                    float err = 0.5 * (baseImageResized[jIndex1 + k] - averages[bestTile][k]);
//                    if (j < nCols - 1)
//                        baseImageResized[3 * (i * nCols + j + 1) + k] += err * 7/16;
//                    if (i < nRows - 1) {
//                        if (j > 0)
//                            baseImageResized[3 * ((i + 1) * nCols + j - 1) + k] += err * 3/16;
//                        baseImageResized[3 * ((i + 1) * nCols + j) + k] += err * 5/16;
//                        if (j < nCols - 1)
//                            baseImageResized[3 * ((i + 1) * nCols + j + 1) + k] += err * 1/16;
//                    }
//                }
//            } else {
//                for (int k = 0; k < 3; ++k) {
//                    float err = 0.5 * (baseImageResized[jIndex1 + k] - averages[bestTile][k]);
//                    if (j > 0)
//                        baseImageResized[3 * (i * nCols + j - 1) + k] += err * 7/16;
//                    if (i < nRows - 1) {
//                        if (j < nCols - 1)
//                            baseImageResized[3 * ((i + 1) * nCols + j + 1) + k] += err * 3/16;
//                        baseImageResized[3 * ((i + 1) * nCols + j) + k] += err * 5/16;
//                        if (j > 0)
//                            baseImageResized[3 * ((i + 1) * nCols + j - 1) + k] += err * 1/16;
//                    }
//                }
//            }
            for (int k = 0; k < tileHeight; ++k) {
                int kIndex1 = jIndex3 + k * finalWidth;
                int kIndex2 = k * tileWidth;
                for (int l = 0; l < tileWidth; ++l) {
                    int lIndex1 = 3 * (kIndex1 + l);
                    int lIndex2 = 3 * (kIndex2 + l);
                    for (int m = 0; m < 3; ++m)
                        finalImage[lIndex1 + m] = !blend ? tileImages[bestTile][lIndex2 + m] : finalImage[lIndex1 + m] + tileImages[bestTile][lIndex2 + m];
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {
    namespace filesystem = std::filesystem;
    namespace chrono = std::chrono;

    const std::string tilePath = "C:/New Folder/mosaic/images";
    const std::array<std::string, 10> validExtensions = {".jpg", ".png", ".tga", ".bmp", ".psd", ".gif", ".hdr", ".pic", ".ppm", ".pgm"};

    int baseWidth, baseHeight;
    unsigned char* baseImage;

    int tileWidth = 0;
    int tileHeight = 0;
    int nRows = 0;
    int nCols = 0;
    int finalWidth = 0;
    int finalHeight = 0;
    int nTiles = 0;
    float scale = 0;
    float blend = 0;

    if (argc > 1) {
        std::string basePath;
        const int nValidStrArgs = 1;
        const std::string strArgStrs[nValidStrArgs][2] = {{"-i", "--input_file"}};
        std::string* strArgTargets[nValidStrArgs] = {&basePath};

        const int nValidIntArgs = 7;
        const std::string intArgStrs[nValidIntArgs][2] = {
            {"-tw", "--tile_width"},  {"-th", "--tile_height"},
            {"-r",  "--num_rows"},    {"-c",  "--num_columns"},
            {"-fw", "--final_width"}, {"-fh", "--final_height"},
            {"-t",  "--num_tiles"}
        };
        int* intArgTargets[nValidIntArgs] = {&tileWidth, &tileHeight, &nRows, &nCols, &finalWidth, &finalHeight, &nTiles};

        const int nValidFloatArgs = 2;
        const std::string floatArgStrs[nValidFloatArgs][2] = {{"-s", "--scale"}, {"-b", "--blend"}};
        float* floatArgTargets[nValidFloatArgs] = {&scale, &blend};

        int skip = 0;
        for (int i = 1; i < argc; ++i) {
            for (int j = 0; j < nValidStrArgs; ++j) {
                if (resolveArg(argv[i], strArgStrs[j])) {
                    *strArgTargets[i] = argv[std::min(i + 1, argc - 1)];
                    skip = 2;
                    break;
                }
            }
            if (skip) {
                --skip;
                continue;
            }
            for (int j = 0; j < nValidIntArgs; ++j) {
                if (resolveArg(argv[i], intArgStrs[j], *intArgTargets[j])) {
                    if (*intArgTargets[j] < 0)
                        return 0;
                    skip = 1;
                    break;
                }
            }
            if (skip) {
                --skip;
                continue;
            }
            for (int j = 0; j < nValidFloatArgs; ++j) {
                if (resolveArg(argv[i], floatArgStrs[j], *floatArgTargets[j])) {
                    if (*floatArgTargets[j] < 0)
                        return 0;
                    break;
                }
            }
        }
        baseImage = stbi_load(basePath.c_str(), &baseWidth, &baseHeight, 0, 3);
        if (!baseImage) {
            printf(" %s\n", stbi_failure_reason());
            return 0;
        }
        float rtio = static_cast<float>(baseWidth) / baseHeight;
        std::array<bool, 6> lastStates;
        float _finalWidth = finalWidth;
        float _finalHeight = finalHeight;
        scale = std::max(0.0f, scale);
        while (!(tileWidth && tileHeight && nRows && nCols && _finalWidth && _finalHeight)) {
            if (!_finalWidth) {
                if (tileWidth && nCols)
                    _finalWidth = tileWidth * nCols;
                else if (scale)
                    _finalWidth = baseWidth * scale;
                else if (_finalHeight)
                    _finalWidth = _finalHeight * rtio;
            }
            if (!_finalHeight) {
                if (tileHeight && nRows)
                    _finalHeight = tileHeight * nRows;
                else if (scale)
                    _finalHeight = baseHeight * scale;
                else if (_finalWidth)
                    _finalHeight = _finalWidth / rtio;
            }
            if (!nRows) {
                if (nTiles && nCols)
                    nRows = nTiles / nCols;
                else if (_finalHeight && tileHeight)
                    nRows = round(_finalHeight / tileHeight);
            }
            if (!nCols) {
                if (nTiles && nRows)
                    nCols = nTiles / nRows;
                else if (_finalWidth && tileWidth)
                    nCols = round(_finalWidth / tileWidth);
            }
            if (!tileWidth && _finalWidth && nCols)
                tileWidth = round(_finalWidth / nCols);
            if (!tileHeight && _finalHeight && nRows)
                tileHeight = round(_finalHeight / nRows);
            std::array<bool, 6> states = {tileWidth, tileHeight, nRows, nCols, _finalWidth, _finalHeight};
            if (states == lastStates) {
                std::cout << " Not enough arguments\n";
                return 0;
            }
            lastStates = states;
        }
        finalWidth = nCols * tileWidth;
        finalHeight = nRows * tileHeight;
        nTiles = nRows * nCols;
    } else {
        while(1) {
            std::string basePath;
            prompt("Base file: ", basePath);
            baseImage = stbi_load(basePath.c_str(), &baseWidth, &baseHeight, 0, 3);
            if (baseImage)
                break;
            printf(" %s\n\n", stbi_failure_reason());
        }
        prompt("Tile width (pixels): ", tileWidth);
        prompt("Tile height (pixels): ", tileHeight);
        prompt("Number of rows: ", nRows);
        finalHeight = tileHeight * nRows;
        nCols = round(static_cast<float>(finalHeight * baseWidth) / (baseHeight * tileWidth));
        finalWidth = tileWidth * nCols;
        nTiles = nRows * nCols;
        printf("Number of columns: %d\n\nFinal image size: %dx%d\nTotal tiles: %d\n\n", nCols, finalWidth, finalHeight, nTiles);
    }

    chrono::time_point startTime = chrono::high_resolution_clock::now();

    unsigned char* baseImageResized = static_cast<unsigned char*>(malloc(3 * nTiles));
    stbir_resize_uint8(baseImage, baseWidth, baseHeight, 0, baseImageResized, nCols, nRows, 0, 3);
    std::vector<std::string> tilePaths;
    std::for_each(filesystem::recursive_directory_iterator(tilePath), filesystem::recursive_directory_iterator(), [&](filesystem::directory_entry entry) {
        if (std::find(validExtensions.begin(), validExtensions.end(), entry.path().extension()) != validExtensions.end())
            tilePaths.push_back(entry.path().string());
    });
    int nImages = tilePaths.size();
    float fractionImages = 100.0 / nImages;
    std::vector<unsigned char*> tileImages;
    tileImages.reserve(nImages);
    std::vector<std::array<float, 3> > averages;
    averages.reserve(nImages);
    int nTilePixels = tileWidth * tileHeight;
    int nTileBytes = 3 * nTilePixels;
    int nImagesNDigits = 0;
    {
        int _nImages = nImages;
        do {
            ++nImagesNDigits;
            _nImages /= 10;
        } while(_nImages);
    }
    for (int i = 0; i < nImages; ++i) {
        int width = 0;
        int height = 0;
        unsigned char* tileImage = stbi_load(tilePaths[i].c_str(), &width, &height, 0, 3);
        int iInc = i + 1;
        if (!tileImage) {
            printf("%*d/%d %6.2f%% %s\n", nImagesNDigits, iInc, nImages, iInc * fractionImages, stbi_failure_reason());
            stbi_image_free(tileImage);
            continue;
        }
        unsigned char* tileImageResized = static_cast<unsigned char*>(malloc(nTileBytes));
        stbir_resize_uint8(tileImage, width, height, 0, tileImageResized, tileWidth, tileHeight, 0, 3);
        stbi_image_free(tileImage);
        tileImages.push_back(tileImageResized);
        int sum[3] = {};
        for (int j = 0; j < tileHeight; ++j) {
            int jIndex = j * tileWidth;
            for (int k = 0; k < tileWidth; ++k) {
                int kIndex = 3 * (jIndex + k);
                for (int l = 0; l < 3; ++l)
                    sum[l] += tileImageResized[kIndex + l];
            }
        }
        std::array<float, 3> average;
        for (int j = 0; j < 3; ++j)
            average[j] = static_cast<float>(sum[j]) / nTilePixels;
        averages.push_back(average);
        printf("%*d/%d %6.2f%%\n", nImagesNDigits, iInc, nImages, iInc * fractionImages);
    }
    tileImages.shrink_to_fit();
    averages.shrink_to_fit();
    int nFinalBytes = 3 * finalWidth * finalHeight;
    unsigned char* finalImage = static_cast<unsigned char*>(malloc(nFinalBytes));
//    stbir_resize_uint8(baseImage, baseWidth, baseHeight, 0, finalImage, finalWidth, finalHeight, 0, 3);
    std::clamp(blend, 0.0f, 1.0f);
    if (!blend)
        assignPixel<false>(nRows, nCols, tileWidth, tileHeight, finalWidth, averages, baseImageResized, finalImage, tileImages);
    else {
        stbir_resize_uint8(baseImage, baseWidth, baseHeight, 0, finalImage, finalWidth, finalHeight, 0, 3);
        for (int i = 0; i < nFinalBytes; ++i)
            finalImage[i] *= blend;
        float antiblend = 1 - blend;
        for (int i = 0; i < nImages; ++i)
            for (int j = 0; j < nTileBytes; ++j)
                tileImages[i][j] *= antiblend;
        assignPixel<true>(nRows, nCols, tileWidth, tileHeight, finalWidth, averages, baseImageResized, finalImage, tileImages);
    }

    stbi_image_free(baseImage);
    stbi_image_free(baseImageResized);
    for (unsigned char* tileImage : tileImages)
        stbi_image_free(tileImage);
    std::cout << "Saving file..." << (stbi_write_png((std::to_string(time(0)) + ".png").c_str(), finalWidth, finalHeight, 3, finalImage, 0) ? " Done\n" : "\nUnable to save file\n");
    stbi_image_free(finalImage);
    printf("Took %.3fs\n", chrono::duration<float>(chrono::high_resolution_clock::now() - startTime).count());
    if (argc == 1) {
        std::cin.ignore();
        std::cin.get();
    }
}
