#include <GL/glut.h>
#include <GL/glu.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "estado.h"
#include "menu.h"
#include "model.h"
#include "tree.h"
#include "player.h"
#include "coin.h"
#include "hud.h"
#include <math.h>
#include "obstacle.h"

Player player;

float worldSpeed = 12.0f;
float distanciaPercorrida = 0.0f;
static const float fator = 0.1f;

Model treeModel;
float escalaArvoreDefault = 1.0f;

void mostrarEixos(){
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-10.0f, 0.1f, 0.0f);
    glVertex3f(10.0f, 0.1f, 0.0f);
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f);
    for(int x = -10; x <= 10; x += 5) {
        glRasterPos3f(x, 0.2f, 0.0f);
        char label[10];
        snprintf(label, 10, "%d", x);
        for(const char *c = label; *c; ++c) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }

    glBegin(GL_LINES);
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 5.0f, 0.0f);
    glEnd();

    for(int y = 0; y <= 5; y += 1) {
        glRasterPos3f(0.2f, y, 0.0f);
        char label[10];
        snprintf(label, 10, "%d", y);
        for(const char *c = label; *c; ++c) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }

    glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.1f, -20.0f);
    glVertex3f(0.0f, 0.1f, 20.0f);
    glEnd();

    for(int z = -20; z <= 20; z += 5) {
        glRasterPos3f(0.2f, 0.2f, z);
        char label[10];
        snprintf(label, 10, "%d", z);
        for(const char *c = label; *c; ++c) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

// Colisão AABB (mantida como estava)
int aabbCollision(float ax, float ay, float az, float aw, float ah, float ad,
                  float bx, float by, float bz, float bw, float bh, float bd) {
    if (fabs(ax - bx) * 2.0f < (aw + bw) &&
        fabs(ay - by) * 2.0f < (ah + bh) &&
        fabs(az - bz) * 2.0f < (ad + bd)) return 1;
    return 0;
}

void resetGame() {
    initPlayer(&player);
    initObstacles();         // Inicializa obstáculos
    worldSpeed = 12.0f;
    distanciaPercorrida = 0.0f;
    initCoins();
}

void update(float dt) {
    if(modoAtual != MODO_JOGO) return;

    worldSpeed += dt * 0.5f;

    updatePlayer(&player, dt);

    // Atualiza obstáculos e gera novos spawn dentro do módulo
    obstacleUpdate(dt);

    // Colisão com obstáculos (checagem dentro do main)
    Obstacle* obstacles = getObstacles();
    int maxObs = getMaxObstacles();

    for(int i = 0; i < maxObs; i++) {
        if(!obstacles[i].active) continue;

        float ph = player.height, pw = player.width, pd = player.depth;

        float obstCenterX = obstacles[i].x;
        float obstWidth = obstacles[i].w;
        float obstHeight = obstacles[i].h;
        float obstDepth = obstacles[i].d;
        float obstY = obstacles[i].y + obstHeight * 0.5f;

        // Verifica colisão AABB
        if (aabbCollision(
            player.x, player.y + ph * 0.5f, player.z, pw, ph, pd,
            obstCenterX, obstY, obstacles[i].z, obstWidth, obstHeight, obstDepth)
        ) {
            modoAtual = MODO_GAMEOVER;
        }
    }

    distanciaPercorrida += worldSpeed * dt * fator;

    updateTrees(dt, worldSpeed);

    updateCoins(dt);
}

void renderScene() {
    if(modoAtual == MODO_MENU) {
        desenhaMenu();
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float camX = player.x;
    float camY = 4.0f;
    float camZ = player.z + 8.0f;
    gluLookAt(camX, camY, camZ,
              player.x, 1.0f, player.z - 8.0f,
              0.0f, 1.0f, 0.0f);

    GLfloat light_position[] = { player.x + 5.0f, 10.0f, player.z + 5.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glDisable(GL_LIGHTING);
    glColor3f(0.9f, 0.9f, 0.9f);
    // Desenha chão
    for(int i = -100; i < 100; i++) {
        float zpos = i * 5.0f;
        glBegin(GL_QUADS);
            glVertex3f(-1.5f, 0.0f, zpos);
            glVertex3f(6.5f, 0.0f, zpos);
            glVertex3f(6.5f, 0.0f, zpos - 5.0f);
            glVertex3f(-1.5f, 0.0f, zpos - 5.0f);
        glEnd();
    }
    glEnable(GL_LIGHTING);

    // Desenha jogador
    drawPlayer(&player);
    drawObstacles();
    drawCoins3D();
    drawTrees();

    if(modoAtual == MODO_GAMEOVER) {
        int w = glutGet(GLUT_WINDOW_WIDTH), h = glutGet(GLUT_WINDOW_HEIGHT);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        gluOrtho2D(0, w, 0, h);
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();
        glDisable(GL_LIGHTING);
        glColor3f(1, 1, 1);
        glRasterPos2i(w / 2 - 80, h / 2);
        const char *s = "GAME OVER - Press R to return";
        for(const char *c = s; *c; ++c) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        glEnable(GL_LIGHTING);
        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }

    drawDistance(distanciaPercorrida);
    drawCoinsHUD(getCoinCount());

    glutSwapBuffers();
}

void idleCB() {
    static float last = 0.0f;
    float now = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    if(last == 0.0f) last = now;
    float dt = now - last;
    last = now;
    update(dt);
    glutPostRedisplay();
}

void mouseCB(int button, int state, int x, int y) {
    if(modoAtual == MODO_MENU) cliqueMenu(button, state, x, y);
}

void keyboardCB(unsigned char key, int x, int y) {
    if(key == 27) exit(0);

    if(modoAtual == MODO_GAMEOVER && (key == 'r' || key == 'R')) {
        modoAtual = MODO_MENU;
        return;
    }

    if(modoAtual != MODO_JOGO) return;

    handlePlayerInput(&player, key);
}

void specialCB(int key, int x, int y) {
    if(modoAtual != MODO_JOGO) return;
    handlePlayerSpecial(&player, key);
}

void reshape(int w, int h) {
    if(h == 0) h = 1;
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (float)w / (float)h, 0.1, 200.0);
    glMatrixMode(GL_MODELVIEW);
}

void initGL() {
    glClearColor(0.12f, 0.12f, 0.15f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    GLfloat light_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat light_diffuse[] = { 0.9f, 0.9f, 0.9f, 1.0f };
    GLfloat light_specular[] = { 0.9f, 0.9f, 0.9f, 1.0f };
    GLfloat light_position[] = { 5.0f, 10.0f, 5.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    GLfloat mat_ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat mat_diffuse[] = { 0.6f, 0.6f, 0.6f, 1.0f };
    GLfloat mat_specular[] = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat high_shininess[] = { 50.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);

    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
}

int main(int argc, char** argv) {
    srand((unsigned)time(NULL));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(1024, 600);
    glutCreateWindow("Prot tipo");
    initGL();
    resetGame();

    if(!loadOBJ("tree.obj", &treeModel)) {
        fprintf(stderr, "Falha ao carregar modelo de árvore.\n");
    } else {
        float alturaTree = treeModel.maxY - treeModel.minY;
        if (alturaTree > 0.1f) {
            escalaArvoreDefault = 2.0f / alturaTree;
        }
    }

    initTrees();

    glutDisplayFunc(renderScene);
    glutIdleFunc(idleCB);
    glutKeyboardFunc(keyboardCB);
    glutSpecialFunc(specialCB);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouseCB);

    glutMainLoop();

    freeModel(&treeModel);

    return 0;
}
