#ifndef MODEL_H
#define MODEL_H

typedef struct {
    int numVertices;
    float* vertices; // vetor 3 floats por vértice (x, y, z)
    float minY, maxY; // para calcular altura do modelo
} Model;

int loadOBJ(const char* filename, Model* model);
void drawModel(const Model* model);
void freeModel(Model* model);

#endif
