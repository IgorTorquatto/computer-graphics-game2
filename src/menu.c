#include <GL/glut.h>
#include <GL/glu.h>
#include <stdlib.h>
#include "menu.h"
#include "estado.h"

#define LARGURA_BOTAO 240
#define ALTURA_BOTAO 60

extern void resetGame(); // fun��o definida em main.c

static void desenhaBotaoCoord(int x, int y, int largura, int altura, const char* texto) {
    glColor3f(0.5f, 0.7f, 1.0f); // azul claro
    glBegin(GL_QUADS);
        glVertex2f((float)x, (float)y);
        glVertex2f((float)(x + largura), (float)y);
        glVertex2f((float)(x + largura), (float)(y + altura));
        glVertex2f((float)x, (float)(y + altura));
    glEnd();

    glColor3f(1.0f, 1.0f, 1.0f); // branco
    int len = 0;
    const char* p;
    for (p = texto; *p; ++p) ++len;
    float approxCharW = 9.0f; // aproxima��o para GLUT_BITMAP_HELVETICA_18
    float textWidth = len * approxCharW;
    float rx = x + (largura - textWidth) * 0.5f;
    float ry = y + altura / 2 - 9; // centralizar verticalmente
    glRasterPos2f(rx, ry);
    for (p = texto; *p; ++p) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
}

/* desenha menu (usa proje��o ortogr�fica tempor�ria) */
void desenhaMenu() {
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int ww = glutGet(GLUT_WINDOW_WIDTH);
    int wh = glutGet(GLUT_WINDOW_HEIGHT);

    // Posi��es centralizadas dos bot�es
    int bx = (ww - LARGURA_BOTAO) / 2;
    int by_play = wh / 2 + 40;
    int by_exit = wh / 2 - 40;

    // Proje��o 2D tempor�ria
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, ww, 0, wh);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // T�tulo simples
    glColor3f(1, 1, 1);
    const char* title = "Nome do nosso jogo";
    float titleX = (ww - (int)strlen(title) * 10) * 0.5f;
    glRasterPos2f(titleX, wh - 80);
    for (const char* c = title; *c; ++c) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);

    // Bot�es
    desenhaBotaoCoord(bx, by_play, LARGURA_BOTAO, ALTURA_BOTAO, "JOGAR");
    desenhaBotaoCoord(bx, by_exit, LARGURA_BOTAO, ALTURA_BOTAO, "SAIR");

    // Restaura matrizes
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    // Restaura contexto 3D
    glEnable(GL_DEPTH_TEST);

    glutSwapBuffers();
}

/* tratar clique do mouse */
void cliqueMenu(int button, int state, int x, int y) {
    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;

    int ww = glutGet(GLUT_WINDOW_WIDTH);
    int wh = glutGet(GLUT_WINDOW_HEIGHT);
    int my = wh - y; // converte para coordenadas bottom-left

    int bx = (ww - LARGURA_BOTAO) / 2;
    int by_play = wh / 2 + 40;
    int by_exit = wh / 2 - 40;

    // Jogar
    if (x >= bx && x <= bx + LARGURA_BOTAO &&
        my >= by_play && my <= by_play + ALTURA_BOTAO) {
        modoAtual = MODO_JOGO;
        resetGame(); // inicia o jogo imediatamente
        return;
    }

    // Sair
    if (x >= bx && x <= bx + LARGURA_BOTAO &&
        my >= by_exit && my <= by_exit + ALTURA_BOTAO) {
        exit(0);
    }
}
