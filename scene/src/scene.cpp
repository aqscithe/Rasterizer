
#include <iostream>

#include <imgui.h>

#include <tiny_obj_loader.h>

#include <common/maths.hpp>
#include <common/utils.hpp>

#include "scene_impl.hpp"

scnImpl* scnCreate()
{
    return new scnImpl();
}

void scnDestroy(scnImpl* scene)
{
    delete scene;
}

void scnUpdate(scnImpl* scene, float deltaTime, rdrImpl* renderer)
{
    scene->update(deltaTime, renderer);
}

void scnSetImGuiContext(scnImpl* scene, struct ImGuiContext* context)
{
    ImGui::SetCurrentContext(context);
}

void scnShowImGuiControls(scnImpl* scene)
{
    scene->showImGuiControls();
}

bool loadObj(std::vector<rdrVertex>& vertices, const char* filename, float scale, std::vector<Image>& images)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn;
    std::string err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename);
    if (!warn.empty())
        printf("tinyObj warning: %s\n", warn.c_str());

    if (!err.empty())
        printf("tinyObj error: %s\n", err.c_str());

    //if (images.size() != shapes.size())
    //{
    //    std::cerr << "Images vector size " << images.size() << " is not equal to number of shapes in .obj " << shapes.size() << std::endl;
    //    exit(1);
    //}
        

    //if (ret)
    //    return false;

    for (size_t s = 0; s < shapes.size(); s++)
    {
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
        {
            int fv = shapes[s].mesh.num_face_vertices[f];
            for (size_t v = 0; v < fv; v++)
            {
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
                tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
                tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
                tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
                tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
                tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];
                tinyobj::real_t tx = attrib.texcoords[2 * idx.texcoord_index + 0];
                tinyobj::real_t ty = attrib.texcoords[2 * idx.texcoord_index + 1];

                //tinyobj::real_t r = attrib.colors[3 * idx.vertex_index + 0];
                //tinyobj::real_t g = attrib.colors[3 * idx.vertex_index + 1];
                //tinyobj::real_t b = attrib.colors[3 * idx.vertex_index + 2];

                vertices.push_back(rdrVertex{ vx * scale, vy * scale, vz * scale, nx, ny, nz, 0.f, 0.f, 0.f, 1.f, tx, ty });
            }
            index_offset += fv;

            // per-face material
            shapes[s].mesh.material_ids[f];
        }
        std::vector<float>::iterator it;
        it = images.at(s).shape.begin() + 1;
        images.at(s).shape.insert(it, (float)shapes[s].mesh.num_face_vertices.size());
    }

    return true;
}

bool loadImage(std::vector<Image>& images, int& width, int& height)
{
    const char* files[1] = {
        //"assets/vehicule/textures/b_d.tga",
        //"assets/vehicule/textures/w_d.tga",
        //"assets/vehicule/textures/w_d.tga",
        //"assets/vehicule/textures/w_d.tga",
        //"assets/vehicule/textures/w_d.tga",
        //"assets/vehicule/textures/w_d.tga",
        //"assets/vehicule/textures/a_d.tga",
        //"assets/vehicule/textures/b_d.tga",
        //"assets/vehicule/textures/a_d.tga",
        //"assets/stormtrooper/textures/t_imperial_stormtrooper_male_01_helmet_cs.tga",
        //"assets/stormtrooper/textures/t_imperial_stormtrooper_male_01_helmet_cs.tga",
        //"assets/stormtrooper/textures/t_imperial_stormtrooper_male_01_upperbody_cs.tga",
        //"assets/stormtrooper/textures/t_imperial_stormtrooper_male_01_upperbody_cs.tga",
        //"assets/stormtrooper/textures/t_imperial_stormtrooper_male_01_upperbody_cs.tga",
        //"assets/stormtrooper/textures/t_imperial_stormtrooper_male_01_helmet_cs.tga",
        
        //"assets/stormtrooper/textures/t_imperial_stormtrooper_male_01_lowerbody_cs.tga",
        
        //"assets/stormtrooper/textures/t_imperial_stormtrooper_male_01_upperbody_cs.tga",
        
        //"assets/stormtrooper/textures/t_imperial_stormtrooper_male_01_lowerbody_cs.tga",
        //"assets/eyeball/textures/Eye_D.jpg",
        //"assets/alien/textures/Alien-Animal-Base-Diffuse.jpg",
        //"assets/alien/textures/Alien-Animal_eye.jpg",
        //"assets/cottage/textures/cottage_diffuse.png",
        //"assets/santa_hat/textures/color.jpg",
        "assets/watch_tower/textures/Wood_Tower_Col.jpg",
        //"assets/calculator/textures/Calculadora_Color.png",
        //"assets/cat/textures/Cat_diffuse.jpg",
    };

    unsigned char* data = nullptr;
    int size = sizeof(files) / sizeof(files[0]);

    for (int i = 0; i < size; ++i)
    {
        data = utils::loadImage(files[i], width, height);
        if (data != nullptr)
        {
            std::vector<float> shape = std::vector<float>(data, data + width * height * 4);
            std::vector<float>::iterator it;
            it = shape.begin();
            shape.insert(it, (float)shape.size());
            images.push_back(Image{ shape, width, height, 0 });
        }
    }
    
    free(data);
    return (data != nullptr);
}



scnImpl::scnImpl()
{

    int width;
    int height;

    // HERE: Load the scene
    bool loadedImages = loadImage(images, width, height);

    if (!loadedImages)
    {
        std::cout << "Error loading images" << std::endl;
        exit(1);
    }

    //loadObj(vertices, "assets/eyeball/eyeball.obj", 1.f, images);
    //loadObj(vertices, "assets/alien/alien.obj", 0.15f, images);
    //loadObj(vertices, "assets/cottage/cottage_obj.obj", 0.15f);
    //loadObj(vertices, "assets/santa_hat/santa_hat(DEFAULT).obj", 0.3f, images);
    loadObj(vertices, "assets/watch_tower/wooden watch tower2.obj", 0.2f, images);
    //loadObj(vertices, "assets/calculator/calculadora.obj", 0.25f, images);
    //loadObj(vertices, "assets/cat/cat.obj", 0.1f, images);
    //loadObj(vertices, "assets/stormtrooper/0.obj", 1.f, images);
    //loadObj(vertices, "assets/vehicule/0.obj", 0.5f, images);

    /*
    vertices = {
        //       pos                  normal                  color              uv
        {-0.5f,-0.5f, 0.0f,      0.0f, 0.0f, 0.0f,      1.0f, 0.0f, 0.0f,     0.0f, 0.0f },
        { 0.5f,-0.5f, 0.0f,      0.0f, 0.0f, 0.0f,      0.0f, 1.0f, 0.0f,     0.0f, 0.0f },
        { 0.0f, 0.5f, 0.0f,      0.0f, 0.0f, 0.0f,      0.0f, 0.0f, 1.0f,     0.0f, 0.0f },
    };*/
    
}

scnImpl::~scnImpl()
{
    // HERE: Unload the scene
}

void scnImpl::update(float deltaTime, rdrImpl* renderer)
{
    // HERE: Update (if needed) and display the scene

    // returns the translation matrix
    // important to do the translation before the scale
    mat4x4 model = mat4::translate({ (float)cos(time) * 0.5f, (float)sin(time) * 0.1f, 0.f }) * mat4::scale(scale);
    //mat4x4 model = mat4::scale(scale);

    //matrix = matrix * mat4::rotateY((float)(time * 2.0));

    rdrSetModel(renderer, model.e);

    if (!texturesSet)
    {
        for (Image image : images)
            rdrSetTexture(renderer, image.shape.data(), image.width, image.height);
        texturesSet = true;
    }

    rdrDrawTriangles(renderer, vertices.data(), (int)vertices.size());
    

    time += deltaTime;
}

void scnImpl::showImGuiControls()
{
    ImGui::SliderFloat("scale", &scale, 0.f, 10.f);
}
