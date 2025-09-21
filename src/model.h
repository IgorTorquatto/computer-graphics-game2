#ifndef MODEL_H
#define MODEL_H

typedef struct {
    int numVertices;
    float* vertices;    // x,y,z por v�rtice
    int numFaces;
    int* faces;        // �ndices dos v�rtices, 3 por face (tri�ngulos)
    float minY, maxY;  // altura para escala
} Model;

int loadOBJ(const char* filename, Model* model);
void drawModel(const Model* model);
void freeModel(Model* model);

#endif
