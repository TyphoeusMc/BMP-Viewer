#ifndef BMP_H
#define BMP_H

#include <Windows.h>

//定义BMP文件头结构用于读取
typedef struct myBITBMPFILEHEADER
{
    WORD bfType;
    DWORD bfSize;
    WORD bfReserved1;
    WORD bfReserved2;
    DWORD bfOffBits;
}myBITBMPFILEHEADER;

//定义BMP位图信息头结构
typedef struct myBITMAPINFOHEADER{
    DWORD      biSize;
    LONG        biWidth;
    LONG        biHeight;
    WORD       biPlanes;
    WORD       biBitCount;
    DWORD      biCompression;
    DWORD      biSizeImage;
    LONG        biXPelsPerMeter;
    LONG        biYPelsPerMeter;
    DWORD      biClrUsed;
    DWORD      biClrImportant;
}myBITMAPINFOHEADER;

#endif // BMP_H
