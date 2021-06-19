#pragma once

#include <string>

namespace files
{
	void    readFiles(std::string file);
}

namespace utils
{
    // Charge un fichier complet en m�moire
    // Stocke la taille du fichier dans fileSize
    // Le retour de la fonction doit �tre delete apr�s avoir appel� la fonction
    unsigned char* loadFile(std::string filename, size_t& fileSize);

    // Charge les pixels d'une image bmp
    // Stocke la longueur et la largeur dans width et height
    // Le retour de la fonction doit �tre delete apr�s avoir appel� la fonction
    unsigned char* loadBMP24Image(std::string filename, int& width, int& height);

    // Charge les pixels d'une image (bmp, jpg, png, ...)
    // Stocke la longueur et la largeur dans width et height
    // Le retour de la fonction doit �tre free() apr�s avoir appel� la fonction
    unsigned char* loadImage(std::string filename, int& width, int& height);
}
