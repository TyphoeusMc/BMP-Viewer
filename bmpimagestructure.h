#ifndef BMPIMAGESTRUCTURE_H
#define BMPIMAGESTRUCTURE_H
#pragma once

#include <Windows.h>

typedef struct BMPImage
{
    BITMAPFILEHEADER fileHeader;
    BITMAPINFOHEADER infoHeader;
    uchar *imageBuffer;
    uchar *colorTable;
    int bytePerLine;
}BMPImage;

#endif // BMPIMAGESTRUCTURE_H
