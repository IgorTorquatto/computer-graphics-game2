#include <GL/glut.h>
#include <GL/glu.h>
#include <stdlib.h>
#include "menu.h"
#include "estado.h"

#define LARGURA_BOTAO 200
#define ALTURA_BOTAO 50

extern void resetGame(); // função implementada em main.c

/* Desenha um botão (posição relativa à janela) */
static void desenhaBotaoCoord(int x, int y, int largura, int altura, const char* texto) {
    glColor3f(0.2f, 0.5f, 0.8f);
    glBegin(GL_QUADS);
        glVertex2f(x, y);
        glVertex2f(x + largura, y);
        glVertex2f(x + largura, y + altura);
        glVertex2f(x, y + altura);
    glEnd();

    // Desenha texto centralizado no botão
    glColor3f(1.0f, 1.0f, 1.0f);
    int len = 0;
    for(const char* p = texto; *p; ++p) len++;

    // aproximação para centralizar (cada caractere ~8 pixels em bitmap HELVETICA_18)
    float textWidth = len * 9.0f;
    float rx = x + (largura - textWidth) * 0.5f;
    float ry = y + (altura - 18.0f) * 0.5f + 4.0f;
    glRasterPos2f(rx, ry);
    for(const char* p = texto; *p; ++p) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
}

/* Desenha o menu (usa coordenadas da janela para centralizar) */
void desenhaMenu() {
    // Limpa (menu usa 2D)
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Pega tamanho da janela
    int ww = glutGet(GLUT_WINDOW_WIDTH);
    int wh = glutGet(GLUT_WINDOW_HEIGHT);

    // Posiciona botões centralizados horizontalmente
    int bx = (ww - LARGURA_BOTAO) / 2;
    int by_play = wh/2 + 40;
    int by_exit = wh/2 - 40;

    // Projeção 2D temporária
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, ww, 0, wh);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    desenhaBotaoCoord(bx, by_play, LARGURA_BOTAO, ALTURA_BOTAO, "Jogar");
    desenhaBotaoCoord(bx, by_exit, LARGURA_BOTAO, ALTURA_BOTAO, "Sair");

    // Restaura matrizes
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);

    glutSwapBuffers();
}

/* Tratamento de clique do mouse; usa glutGet para altura da janela */
void cliqueMenu(int button, int state, int x, int y) {
    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return;

    int ww = glutGet(GLUT_WINDOW_WIDTH);
    int wh = glutGet(GLUT_WINDOW_HEIGHT);

    // calcula y invertido (OpenGL orto 2D assume origem em bottom-left)
    int my = wh - y;

    int bx = (ww - LARGURA_BOTAO) / 2;
    int by_play = wh/2 + 40;
    int by_exit = wh/2 - 40;

    // Jogar?
    if (x >= bx && x <= bx + LARGURA_BOTAO &&
        my >= by_play && my <= by_play + ALTURA_BOTAO) {
        modoAtual = MODO_JOGO;
        resetGame(); // inicia jogo imediatamente
        return;
    }

    // Sair?
    if (x >= bx && x <= bx + LARGURA_BOTAO &&
        my >= by_exit && my <= by_exit + ALTURA_BOTAO) {
        exit(0);
    }
}
