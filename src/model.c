#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include "model.h"

// Carrega vértices simples de OBJ (sem faces, normais ou texturas)
int loadOBJ(const char* filename, Model* model) {
    FILE* file = fopen(filename, "r");
    if(!file) {
        fprintf(stderr, "Erro ao abrir arquivo OBJ: %s\n", filename);
        return 0;
    }

    int vertsCount = 0;
    char line[128];
    while(fgets(line, 128, file) != NULL) {
        if(strncmp(line, "v ", 2) == 0) vertsCount++;
    }

    rewind(file);
    model->numVertices = vertsCount;
    model->vertices = malloc(sizeof(float) * 3 * vertsCount);
    if(!model->vertices) {
        fclose(file);
        return 0;
    }

    int idx = 0;
    while(fgets(line, 128, file) != NULL) {
        if(strncmp(line, "v ", 2) == 0) {
            float x,y,z;
            sscanf(line + 2, "%f %f %f", &x, &y, &z);
            model->vertices[idx++] = x;
            model->vertices[idx++] = y;
            model->vertices[idx++] = z;
        }
    }
    fclose(file);

    // Calcula altura mínima e máxima (eixo Y)
    model->minY = model->vertices[1];
    model->maxY = model->vertices[1];
    for(int i=0; i < model->numVertices; i++) {
        float y = model->vertices[i * 3 + 1];
        if(y < model->minY) model->minY = y;
        if(y > model->maxY) model->maxY = y;
    }

    return 1;
}

void drawModel(const Model* model) {
    glBegin(GL_POINTS); // Desenha os vértices como pontos simples
    for(int i=0; i < model->numVertices; i++) {
        glVertex3f(model->vertices[i*3], model->vertices[i*3+1], model->vertices[i*3+2]);
    }
    glEnd();
}

void freeModel(Model* model) {
    if(model->vertices) free(model->vertices);
    model->vertices = NULL;
    model->numVertices = 0;
}
