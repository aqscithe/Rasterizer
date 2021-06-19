#pragma once

#include <cassert>
#include <iostream>
#include <fstream>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <common/utils.hpp>

#pragma pack(push, 1)
struct BMPHeader
{
    char signature[2];
    unsigned int fileSize;
    char reserved1[2];
    char reserved2[2];
    unsigned int fileOffsetToPixelArray;
};

struct DIBHeader
{
    unsigned int unused_00; // DIBHeader size
    unsigned int width;
    unsigned int height;
    unsigned short unused_01; // Number of color plane (must be one)
    unsigned short bpp; // bits per pixel (24,32)
};
#pragma pack(pop)

void	files::readFiles(std::string file)
{
    std::ifstream readFile;
    readFile.open(file, std::ios::in);

    if (readFile.is_open())
    {
        std::string line;
        int row = 0;
        while (std::getline(readFile, line))
        {
            // do something
            ++row;
        }
        readFile.close();
    }
    else
    {
        std::cerr << "Unable to open file" << std::endl;
    }
}

// Charge un fichier complet en mémoire
// Stocke la taille du fichier dans fileSize
// Le retour de la fonction doit être delete après avoir appelé la fonction
unsigned char*      utils::loadFile(std::string filename, size_t& fileSize)
{
    unsigned char* data = nullptr;

#if 0

    // Open file
    FILE* file = fopen(filename, "rb");
    if (file == nullptr)
    {
        printf("Error opening file  %s\n", filename);
        return nullptr;
    }

    // Read filesize
    fseek(file, 0, SEEK_END);
    fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate Memory
    data = new unsigned char[fileSize];
    fread(data, sizeof(unsigned char), fileSize, file);

    //close file
    fclose(file);

#else
    std::ifstream file(filename, std::ios_base::binary);

    if (!file.is_open())
    {
        std::cerr << "Unable to open file" << std::endl;
        return nullptr;
    }

    file.seekg(0, std::ios_base::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios_base::beg);

    data = new unsigned char[fileSize];
    file.read((char*)data, fileSize);

#endif
    return data;
}

// Charge les pixels d'une image bmp
// Stocke la longueur et la largeur dans width et height
// Le retour de la fonction doit être delete après avoir appelé la fonction
unsigned char*      utils::loadBMP24Image(std::string filename, int& width, int& height)
{
    size_t fileSize;
    unsigned char* data = utils::loadFile(filename, fileSize);

    if (data == nullptr)
        return nullptr;

    // Read bitmap header info
    BMPHeader*      header = (BMPHeader*)data;
    DIBHeader*      dibHeader = (DIBHeader*)(data + sizeof(BMPHeader));

    assert(dibHeader->bpp == 24);
    width = dibHeader->width;
    height = dibHeader->height;

    unsigned char* pixels = new unsigned char[width * height * 3];

    // Alignment
    // Copy bmp colors to pixels
    unsigned char* pixelArray = data + header->fileOffsetToPixelArray;
    memcpy(pixels, pixelArray, width * height * 3);

    delete data;

    return pixels;
}

// Charge les pixels d'une image (bmp, jpg, png, ...)
// Stocke la longueur et la largeur dans width et height
// Le retour de la fonction doit être free() après avoir appelé la fonction
unsigned char* utils::loadImage(std::string filename, int& width, int& height)
{
    stbi_set_flip_vertically_on_load(1);
    // req_comp - la nombre de composants à récuperer 
    // eg. 4 = RGBA, 3 = RGB
    return stbi_load(filename.c_str(), &width, &height, nullptr, 4);
}
