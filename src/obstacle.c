#include "obstacle.h"
#include <stdlib.h>
#include <GL/glut.h>
#include <math.h>

#define LANE_WIDTH 2.5f

static Obstacle obstacles[MAX_OBSTACLES];
extern float worldSpeed;  // Variável global do jogo para velocidade do mundo
static float spawnTimer = 0.0f;
static float spawnInterval = 1.0f;

/* Retorna o X do centro da faixa dada */
static float laneToX(int lane) {
    return lane * LANE_WIDTH;
}

void initObstacles() {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        obstacles[i].active = 0;
    }
    spawnTimer = 0.0f;
    spawnInterval = 1.0f;
}

/* Atualiza posição dos obstáculos e desativa os que passam do ponto */
void updateObstacles(float dt) {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!obstacles[i].active) continue;

        obstacles[i].z += worldSpeed * dt;
        if (obstacles[i].z > 10.0f) {
            obstacles[i].active = 0;
        }
    }
}

/* Desenha obstáculos */
void drawObstacles() {
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!obstacles[i].active) continue;

        glPushMatrix();
        // Se tipo simples, desenha um cubo na faixa central do obstáculo
        // Se obstáculo ocupa duas faixas, fará desenho na metade das faixas

        float drawX = obstacles[i].x;
        float drawY = obstacles[i].y + obstacles[i].h * 0.5f;
        float drawZ = obstacles[i].z;

        glTranslatef(drawX, drawY, drawZ);

        glColor3f(0.7f, 0.2f, 0.2f);

        if (obstacles[i].type == OBST_SINGLE) {
            glScalef(obstacles[i].w, obstacles[i].h, obstacles[i].d);
            glutSolidCube(1.0f);
        } else if (obstacles[i].type == OBST_DOUBLE) {
            // Para obstáculo duplo, desenhar dois cubos lado a lado (ou um mais largo)
            // Aqui desenhamos um cubo mais largo que ocupa 2 faixas
            glScalef(obstacles[i].w * 2.0f, obstacles[i].h, obstacles[i].d);
            glutSolidCube(1.0f);
        }

        glPopMatrix();
    }
}

/* Spawn de um obstáculo aleatório de 1 ou 2 faixas e checa se ocupa 3 faixas */
void spawnObstacle() {
    int lanes[3] = {0, 0, 0}; // 0: livre, 1: ocupada

    // Verifica ocupação dos obstáculos mais recentes próximos à zona de spawn
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!obstacles[i].active) continue;
        // Verificar apenas obstáculos recém criados perto do início (pode usar z < limiar, por exemplo -58.0)
        if (obstacles[i].z < -58.0f) {
            if (obstacles[i].type == OBST_SINGLE) {
                lanes[obstacles[i].lane] = 1;
            } else if (obstacles[i].type == OBST_DOUBLE) {
                // duplo: lane=0->ocupa esq+centro, lane=1->centro+dir
                lanes[obstacles[i].lane] = 1;
                lanes[obstacles[i].lane + 1] = 1;
            }
        }
    }

    // Checa se já há 2 faixas ocupadas — não pode ter mais!
    int ocupadas = lanes[0] + lanes[1] + lanes[2];
    if (ocupadas >= 2) {
        // Só permita spawns que NÃO bloqueiem tudo (não crie obst duplo se a terceira faixa já ocupada, não crie obst single se já outras 2 ocupadas)
        return;
    }

    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!obstacles[i].active) {
            obstacles[i].active = 1;

            // Decide tipo de obstáculo respeitando faixas livres
            int tipo = (rand() % 2);
            if (tipo == 0) { // single
                // Só permite em faixa livre
                int livres[3], nlivres = 0;
                for (int j = 0; j < 3; j++) if (!lanes[j]) livres[nlivres++] = j;
                if (nlivres == 0) return; // não há faixa livre!
                obstacles[i].type = OBST_SINGLE;
                obstacles[i].lane = livres[rand() % nlivres];
                obstacles[i].x = laneToX(obstacles[i].lane);
                obstacles[i].w = 1.0f;
            } else { // double
                // Só permite duplo se pelo menos uma dupla disponível
                // Opções: esquerda-centro (0-1) ou centro-direita (1-2)
                int duplas[2], nduplas = 0;
                if (!lanes[0] && !lanes[1]) duplas[nduplas++] = 0;
                if (!lanes[1] && !lanes[2]) duplas[nduplas++] = 1;
                if (nduplas == 0) return; // não há duplas livres!
                obstacles[i].type = OBST_DOUBLE;
                obstacles[i].lane = duplas[rand() % nduplas];
                obstacles[i].x = laneToX(obstacles[i].lane) + LANE_WIDTH * 0.5f;
                obstacles[i].w = 2.0f;
            }

            obstacles[i].y = 0.0f;
            obstacles[i].h = 2.0f + (float)(rand() % 20) * 0.05f;
            obstacles[i].d = 1.0f;
            obstacles[i].z = -60.0f - (rand() % 40);
            return;
        }
    }
}


Obstacle* getObstacles() {
    return obstacles;
}

int getMaxObstacles() {
    return MAX_OBSTACLES;
}

/* Atualiza todos os obstáculos e gerencia o spawn automático */
void obstacleUpdate(float dt) {
    spawnTimer += dt;

    updateObstacles(dt); // move e desativa obstáculos

    if (spawnTimer >= spawnInterval) {
        spawnTimer = 0.0f;
        spawnObstacle();
        spawnInterval = 0.8f - fmin(0.5f, worldSpeed * 0.01f);
        if (spawnInterval < 0.25f) spawnInterval = 0.25f;
    }
}


