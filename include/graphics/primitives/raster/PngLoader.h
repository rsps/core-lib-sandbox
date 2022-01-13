#ifndef PNGLOADER_H
#define PNGLOADER_H

#include <graphics/primitives/raster/ImgLoader.h>
#include <cstring>
#include <iostream>
#include <vector>

namespace rsp::graphics
{

class PngLoader: public ImgLoader
{
public:
    std::vector<uint32_t> LoadImg(const std::string &aImgName);

protected:
    //struct IDAT {
    //} __attribute__((packed));  //To stop alignment;
    struct PLTE
    {
    } __attribute__((packed)); //To stop alignment;
    struct IEND
    {
    } __attribute__((packed)); //To stop alignment;
    struct IHDR
    {
        uint32_t width;
        uint32_t height;
        uint8_t bitDepth;
        uint8_t colourType;
        uint8_t compressionMethod;
        uint8_t filterMethod;
        uint8_t interlaceMethod;
    } __attribute__((packed)); //To stop alignment
    struct PNGChunk
    {
        uint32_t length;
        char type[4];
        union
        {
            uint8_t *data = nullptr;
            struct IHDR *ihdr;
            struct PLTE *plte;
            struct IEND *iend;
        };
        //uint32_t crc; //Always just at the end of the pointer
    } __attribute__((packed)); //To stop alignment

    uint8_t mPngSignature[8] = { 137, 80, 78, 71, 13, 10, 26, 10 };

    bool CheckSignature(const uint8_t *aSig, const uint8_t &aSize);
    void ReadHeader(FILE *file);
    void ReadData(FILE *file);
};

}

#endif //PNGLOADER_H
