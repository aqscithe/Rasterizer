
#include <vector>

#include <rdr/renderer.h>
#include <scn/scene.h>

struct rdrImpl;

struct Image
{
    std::vector<float> shape;
    int width;
    int height;
    int vertexCount;
};

struct scnImpl
{
    scnImpl();
    ~scnImpl();
    void update(float deltaTime, rdrImpl* renderer);

    void showImGuiControls();

private:
    double time = 0.0;
    std::vector<rdrVertex> vertices;
    float scale = 1.f;

    std::vector<Image> images;
    bool texturesSet = false;
};