#include <GL/glut.h>
#include <GL/glu.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "estado.h"
#include "menu.h"
#include "model.h"
#include "tree.h"

#define MAX_OBS 64
#define LANE_X(i) (i * 2.5f) // lanes: 0,1,2 -> x = -2.5,0,2.5
#define GRAVITY -30.0f

typedef enum { P_RUNNING, P_JUMPING, P_SLIDING } PlayerState;

typedef struct {
    int lane; // 0..2
    float x,y,z;
    float vy;
    PlayerState state;
    float width, height, depth;
} Player;

typedef struct {
    int active;
    int lane;
    float x,y,z;
    float w,h,d;
} Obstacle;

/* Globals */
Player player;
Obstacle obsPool[MAX_OBS];

float worldSpeed = 12.0f; // units per second, increases with time
float spawnTimer = 0.0f;
float spawnInterval = 1.0f;
float distanciaPercorrida = 0.0f;
static const float fator = 0.1f;


Model obstacleModel; // Modelo carregado para os obstáculos
float escalaObstaculo = 1.0f; // escala para o obstáculo

Model treeModel;
float escalaArvoreDefault = 1.0f;

Model roadModel;




/* Prototypes */
void resetGame();
void spawnObstacle();
int aabbCollision(float ax, float ay, float az, float aw, float ah, float ad,
                  float bx, float by, float bz, float bw, float bh, float bd);

void mostrarEixos(){

    glDisable(GL_LIGHTING); // Cor pura
    glDisable(GL_DEPTH_TEST); // Evita clipping

    // Eixo X (vermelho): -10 a 10
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glColor3f(1.0f, 0.0f, 0.0f); // Vermelho
    glVertex3f(-10.0f, 0.1f, 0.0f);
    glVertex3f(10.0f, 0.1f, 0.0f);
    glEnd();

    // Marcadores num ricos no eixo X
    glColor3f(1.0f, 1.0f, 1.0f);
    for(int x = -10; x <= 10; x += 5) {
        glRasterPos3f(x, 0.2f, 0.0f);
        char label[10];
        snprintf(label, 10, "%d", x);
        for(const char *c = label; *c; ++c) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }

    // Eixo Y (verde): 0 a 5
    glBegin(GL_LINES);
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    glVertex3f(0.0f, 5.0f, 0.0f);
    glEnd();

    // Marcadores no eixo Y
    for(int y = 0; y <= 5; y += 1) {
        glRasterPos3f(0.2f, y, 0.0f);
        char label[10];
        snprintf(label, 10, "%d", y);
        for(const char *c = label; *c; ++c) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }

    // Eixo Z (azul): -20 a 20
    glBegin(GL_LINES);
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.1f, -20.0f);
    glVertex3f(0.0f, 0.1f, 20.0f);
    glEnd();

    // Marcadores no eixo Z
    for(int z = -20; z <= 20; z += 5) {
        glRasterPos3f(0.2f, 0.2f, z);
        char label[10];
        snprintf(label, 10, "%d", z);
        for(const char *c = label; *c; ++c) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }

    glEnable(GL_DEPTH_TEST); // Reativa
    glEnable(GL_LIGHTING); // Reativa
}

/* Timer callback end of slide */
void endSlide(int value) {
    player.height = 2.0f;
    player.state = P_RUNNING;
}

/* Reset game state */
void resetGame() {
    player.lane = 1; player.x = LANE_X(1); player.y = 0; player.z = 0;
    player.vy = 0; player.state = P_RUNNING;
    player.width = 1.0f; player.height = 2.0f; player.depth = 1.0f;
    for(int i=0;i<MAX_OBS;i++) obsPool[i].active = 0;
    worldSpeed = 12.0f;
    spawnTimer = 0.0f;
    spawnInterval = 1.0f;
}

/* Spawn obstacle in a free pool slot */
void spawnObstacle() {
    for(int i=0;i<MAX_OBS;i++){
        if(!obsPool[i].active){
            obsPool[i].active = 1;
            obsPool[i].lane = rand()%3;
            obsPool[i].x = LANE_X(obsPool[i].lane);
            obsPool[i].y = 0;
            obsPool[i].z = -40.0f - (rand()%20);
            obsPool[i].w = 1.0f; obsPool[i].h = 2.2f; obsPool[i].d = 1.0f;
            return;
        }
    }
}

/* AABB collision test (centered boxes) */
int aabbCollision(float ax, float ay, float az, float aw, float ah, float ad,
                  float bx, float by, float bz, float bw, float bh, float bd) {
    if (fabs(ax - bx) * 2.0f < (aw + bw) &&
        fabs(ay - by) * 2.0f < (ah + bh) &&
        fabs(az - bz) * 2.0f < (ad + bd)) return 1;
    return 0;
}

/* Update game logic (called each frame with dt) */
void update(float dt) {
    if(modoAtual != MODO_JOGO) return;

    // speed up slowly
    worldSpeed += dt * 0.5f;

    // snap to lane (could be smoothed)
    player.x = LANE_X(player.lane);

    // jump physics
    if(player.state == P_JUMPING) {
        player.vy += GRAVITY * dt;
        player.y += player.vy * dt;
        if(player.y <= 0.0f) { player.y = 0.0f; player.vy = 0.0f; player.state = P_RUNNING; }
    }

    // move obstacles toward player
    for(int i=0;i<MAX_OBS;i++){
        if(!obsPool[i].active) continue;
        obsPool[i].z += worldSpeed * dt;
        if(obsPool[i].z > 10.0f) obsPool[i].active = 0;
        else {
            float ph = player.height, pw = player.width, pd = player.depth;
            float oby = obsPool[i].y + obsPool[i].h*0.5f;
            if (aabbCollision(player.x, player.y + ph*0.5f, player.z, pw, ph, pd,
                              obsPool[i].x, oby, obsPool[i].z, obsPool[i].w, obsPool[i].h, obsPool[i].d)) {
               // modoAtual = MODO_GAMEOVER;
            }
        }
    }

    // spawn logic
    spawnTimer += dt;
    if(spawnTimer >= spawnInterval) {
        spawnTimer = 0;
        spawnObstacle();
        spawnInterval = 0.8f - fmin(0.5f, worldSpeed * 0.01f);
        if(spawnInterval < 0.25f) spawnInterval = 0.25f;
    }

	distanciaPercorrida += worldSpeed * dt * fator;
    updateTrees(dt, worldSpeed);

}

/* Draw helper: scaled cube centered at x,y,z (y is base)
   We translate to (x, y + sy*0.5f, z) before scaling cube(1).
*/
void drawCube(float x, float y, float z, float sx, float sy, float sz) {
    glPushMatrix();
    glTranslatef(x, y + sy * 0.5f, z);
    glScalef(sx, sy, sz);
    glutSolidCube(1.0f);
    glPopMatrix();
}

/* Draw the loaded OBJ model as points (simple) */
void drawModel(const Model* model); // Prototype from model.h included

/* Render scene: if menu -> call desenhaMenu (which does its own swapbuffers),
   otherwise draw 3D world (and swapbuffers here).

*/

void drawDistance() {
    int w = glutGet(GLUT_WINDOW_WIDTH);
    int h = glutGet(GLUT_WINDOW_HEIGHT);

    char buffer[50];
    snprintf(buffer, sizeof(buffer), "%.1f m", distanciaPercorrida);

    // Configurar matriz para texto 2D
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 1.0f, 1.0f);

    // Posição: 10 px do topo e 10 px da direita
    glRasterPos2i(w - 10 * strlen(buffer), h - 20);

    for(const char* c = buffer; *c; ++c) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    glEnable(GL_LIGHTING);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}


void renderScene() {
    if(modoAtual == MODO_MENU) {
        desenhaMenu();
        return;
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Camera: atrás e acima do jogador, olhando para frente
    float camX = player.x;
    float camY = 4.0f;
    float camZ = player.z + 8.0f;
    gluLookAt(camX, camY, camZ,
              player.x, 1.0f, player.z - 8.0f,
              0.0f, 1.0f, 0.0f);

    //mostrarEixos();

    GLfloat light_position[] = { player.x + 5.0f, 10.0f, player.z + 5.0f, 1.0f };
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    // Ativa uso da cor atual para material (para que glColor3f funcione com iluminação)
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // Desenha o chão (sem iluminação para cor pura)
    glDisable(GL_LIGHTING);
    glColor3f(0.9f, 0.9f, 0.9f);
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

    /*float tileLength = 10.0f;
	int numTiles = 25;

	for(int i = -1; i < numTiles - 1; i++) {
		glPushMatrix();
		glTranslatef(0.0f, 0.0f, i * tileLength);
		glScalef(3.5f, 2.0f, 1.0f);
		drawModel(&roadModel);
		glPopMatrix();
	}*/

    // Jogador (cubo azul)
    glColor3f(0.1f, 0.4f, 0.9f);
    drawCube(player.x, player.y, player.z, player.width, player.height, player.depth);

    // Obstáculos usando modelo OBJ
    for(int i=0; i<MAX_OBS; i++){
        if(!obsPool[i].active) continue;
        glPushMatrix();
        glTranslatef(obsPool[i].x, obsPool[i].y + 0.5f, obsPool[i].z);

        glRotatef(90.0f, 1.0f, 0.0f, 0.0f);   // Deita o modelo para ficar em pé
        glRotatef(180.0f, 0.0f, 1.0f, 0.0f); // Gira para olhar para frente
        glRotatef(180.0f, 0.0f, 0.0f, 1.0f); // Gira 180 graus no eixo Z para virar atrás

        glScalef(escalaObstaculo, escalaObstaculo, escalaObstaculo);

        // Define cor para o gato e desenha modelo com iluminação
        glColor3f(0.7f, 0.2f, 0.2f);

        drawModel(&obstacleModel);
        glPopMatrix();
    }


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
        glColor3f(1,1,1);
        glRasterPos2i(w/2 - 80, h/2);
        const char *s = "GAME OVER - Press R to return";
        for(const char *c = s; *c; ++c) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
        glEnable(GL_LIGHTING);

        glPopMatrix();
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
    }

    drawDistance();

    glutSwapBuffers();
}


/* Idle / timing */
void idleCB() {
    static float last = 0.0f;
    float now = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    if(last == 0.0f) last = now;
    float dt = now - last;
    last = now;
    update(dt);
    glutPostRedisplay();
}

/* Mouse callback routes to menu when in menu */
void mouseCB(int button, int state, int x, int y) {
    if(modoAtual == MODO_MENU) cliqueMenu(button, state, x, y);
}

/* Keyboard */
void keyboardCB(unsigned char key, int x, int y) {
    if(key == 27) exit(0);

    if(modoAtual == MODO_GAMEOVER && (key == 'r' || key == 'R')) {
        modoAtual = MODO_MENU;
        return;
    }

    if(modoAtual != MODO_JOGO) return;

    switch(key) {
    case 'w':
    case 'W':
    case ' ':
        if(player.state == P_RUNNING) {
            player.state = P_JUMPING;
            player.vy = 12.0f;
        }
        break;
    case 's':
    case 'S':
        if(player.state == P_RUNNING) {
            player.state = P_SLIDING;
            player.height = 1.0f;
            glutTimerFunc(600, endSlide, 0);
        }
        break;
    case 'a':
    case 'A':
        if(player.lane > 0) {
            player.lane--;
        }
        break;
    case 'd':
    case 'D':
        if(player.lane < 2) {
            player.lane++;
        }
        break;
    }
}

/* Special keys - lane switching */
void specialCB(int key, int x, int y) {
    if(modoAtual != MODO_JOGO) return;

    switch(key) {
    case GLUT_KEY_LEFT:
        if(player.lane > 0) {
            player.lane--;
        }
        break;
    case GLUT_KEY_RIGHT:
        if(player.lane < 2) {
            player.lane++;
        }
        break;
    case GLUT_KEY_UP:
        if(player.state == P_RUNNING) {
            player.state = P_JUMPING;
            player.vy = 12.0f;
        }
        break;
    case GLUT_KEY_DOWN:
        if(player.state == P_RUNNING) {
            player.state = P_SLIDING;
            player.height = 1.0f;
            glutTimerFunc(600, endSlide, 0);
        }
        break;
    }
}

/* Window reshape: sets perspective projection for 3D */
void reshape(int w, int h) {
    if(h == 0) h = 1;
    glViewport(0,0,w,h);

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

    GLfloat light_ambient[]  = { 0.2f, 0.2f, 0.2f, 1.0f };
    GLfloat light_diffuse[]  = { 0.9f, 0.9f, 0.9f, 1.0f };
    GLfloat light_specular[] = { 0.9f, 0.9f, 0.9f, 1.0f };
    GLfloat light_position[] = { 5.0f, 10.0f, 5.0f, 1.0f };

    glLightfv(GL_LIGHT0, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    GLfloat mat_ambient[]    = { 0.3f, 0.3f, 0.3f, 1.0f };
    GLfloat mat_diffuse[]    = { 0.6f, 0.6f, 0.6f, 1.0f };
    GLfloat mat_specular[]   = { 0.8f, 0.8f, 0.8f, 1.0f };
    GLfloat high_shininess[] = { 50.0f };

    glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);

    // Ativa uso da cor atual para material ambiente e difuso
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
}


/* Main */
int main(int argc, char** argv) {
    srand((unsigned)time(NULL));
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
    glutInitWindowSize(1024, 600);
    glutCreateWindow("Prot tipo");

    initGL();
    resetGame();

    // Carrega modelo de obstáculo OBJ
	  if(!loadOBJ("cat.obj", &obstacleModel)) {
		fprintf(stderr, "Falha ao carregar modelo de gato.\n");
	}
	else {
		float alturaModelo = obstacleModel.maxY - obstacleModel.minY;
		float alturaJogador = 2.0f;
		escalaObstaculo = alturaJogador / alturaModelo;
	}

	if(!loadOBJ("tree.obj", &treeModel)) {
		fprintf(stderr, "Falha ao carregar modelo de árvore.\n");
	}
	else {
		float alturaTree = treeModel.maxY - treeModel.minY;
		if (alturaTree > 0.1f) {
			escalaArvoreDefault = 2.0f / alturaTree;
		}
	}

	if(!loadOBJ("road_triangulated.obj", &roadModel)) {
    fprintf(stderr, "Falha ao carregar modelo da estrada.\n");
}


	initTrees();


    glutDisplayFunc(renderScene);
    glutIdleFunc(idleCB);
    glutKeyboardFunc(keyboardCB);
    glutSpecialFunc(specialCB);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouseCB);

    glutMainLoop();

    freeModel(&obstacleModel);
    freeModel(&treeModel);


    return 0;
}
