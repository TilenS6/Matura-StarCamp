#include "phisics/phisics.h"

/*
struct PhTextureTris {
    int idA, idB, idC;
    double normA, normB, normC;
};

class PhTexture
{
    bool inited;
    FastCont<PhTextureTris> indiciesTrises; // -> SDL_Vertex(3x render_pos, 3x colour, 3x texture_normal)
    SDL_Texture *texture;

public:
    PhTexture();
    void setTexture(SDL_Texture *);
    void push_indicie(int, int, int);
    void render(Camera *);
};
*/

PhTexture::PhTexture() {
    inited = false;
    texture = nullptr;
    orgPath = "";
}

void PhTexture::setTexture(Camera *cam, string path) {
    texture = IMG_LoadTexture(cam->r, path.c_str());

    if (texture != NULL) {
        orgPath = path;
        inited = true;
    }
}
void PhTexture::push_indicie(int idA, int idB, int idC, Point normA, Point normB, Point normC) {
    if (normA.x < 0 || normA.x > 1 || normA.y < 0 || normA.y > 1 ||
        normB.x < 0 || normB.x > 1 || normB.y < 0 || normB.y > 1 ||
        normC.x < 0 || normC.x > 1 || normC.y < 0 || normC.y > 1) {
        cout << "E: @ PhTexture::push_indicie: norms out of bounds!!!\n";
        return;
    }
    PhTextureTris tris = {idA, idB, idC, normA, normB, normC};
    indiciesTrises.push_back(tris);
}
void PhTexture::render(Camera *cam, PhWorld *w) {
    if (!inited)
        cout << "W: @PhTexture::render - texture not initialized!\n";
    if (indiciesTrises.size == 0)
        cout << "W: @PhTexture::render - indiciesTrises not seted!\n";
    if (texture == nullptr)
        return;

    // ----------------------

    SDL_Vertex vert[indiciesTrises.size * 3];
    int c = 0;

    for (int i = 0; i < indiciesTrises.size; ++i) {
        // Point point = {p[i][0], p[i][1]};
        PhTextureTris tr = *indiciesTrises.at_index(i);
        Point rend = w->points.at_id(tr.idA)->getRenderPos(cam);
        vert[c++] = {
            {(float)rend.x, (float)rend.y},         // position on screen
            {255, 255, 255, 255},                   // colour
            {(float)tr.normA.x, (float)tr.normA.y}, // texture normals
        };

        rend = w->points.at_id(tr.idB)->getRenderPos(cam);
        vert[c++] = {
            {(float)rend.x, (float)rend.y},         // position on screen
            {255, 255, 255, 255},                   // colour
            {(float)tr.normB.x, (float)tr.normB.y}, // texture normals
        };

        rend = w->points.at_id(tr.idC)->getRenderPos(cam);
        vert[c++] = {
            {(float)rend.x, (float)rend.y},         // position on screen
            {255, 255, 255, 255},                   // colour
            {(float)tr.normC.x, (float)tr.normC.y}, // texture normals
        };
    }

    SDL_RenderGeometry(cam->r, texture, vert, indiciesTrises.size * 3, NULL, 0);

#pragma message("tole na clientu ne rendra!")
}