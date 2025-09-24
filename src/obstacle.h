#ifndef OBSTACLE_H
#define OBSTACLE_H

#define MAX_OBSTACLES 64

typedef enum {
    OBST_SINGLE,
    OBST_DOUBLE
} ObstacleType;

typedef struct {
    int active;
    ObstacleType type;
    int lane;
    float x, y, z;
    float w, h, d;
} Obstacle;

void initObstacles();
void updateObstacles(float dt);
void drawObstacles();
void spawnObstacle();
Obstacle* getObstacles();
int getMaxObstacles();
void obstacleUpdate(float dt);


#endif

