#include "robot.h"
#include "room.h"

s_robot initRobot(int x, int y) {
  s_robot robot;
  robot.pos = (s_pos){x, y};
  robot.moving = 0;
  robot.hasExtinguisher = 0;

  robot.healthPoints = DEFAULT_HP;

  return robot;
}

char displayRobot(s_robot* robot) {
  if(robot->hasExtinguisher == 1)
    return TILE_ROBOT_WITH_EXTINGUISHER;

  return TILE_ROBOT;
}

int isAtPos(s_robot* robot, int x, int y) {
  if(robot->pos.x == x && robot->pos.y == y) {
    return 1;
  }

  return 0;
}

vector getPath(s_room* room, s_pos dest) {
  int pathFound = 0;
  s_node* last = 0;
  s_node* currentNode = &(room->nodes[room->robot.pos.y][room->robot.pos.x]);

  for(int i=0; i<room->sizeX; i++) {
    for(int j=0; j<room->sizeY; j++) {
      s_node* node = &(room->nodes[j][i]);
      node->H = sqrt(pow(fabs(dest.x - i)*1.0, 2) + pow(fabs(dest.y - j)*1.0, 2));
    }
  }

  vector openedList;
  vector closedList;

  vector_init(&openedList);
  vector_init(&closedList);
  vector_add(&openedList, currentNode);

  while(!pathFound) {
    int currentIdx=0;
    currentNode = (s_node*)vector_get(&openedList, 0);

    for(int i=1; i < vector_total(&openedList); i++) {
      if(currentNode->H < ((s_node*)vector_get(&openedList, i))->H) {
        currentNode = (s_node*)vector_get(&openedList, i);
        currentIdx = i;
      }
    }
    vector_add(&closedList, currentNode);
    vector_delete(&openedList, currentIdx);

    if(currentNode->pos.x == dest.x && currentNode->pos.y == dest.y) {
      pathFound = 1;
      last = currentNode;
      break;
    }

    for(int i=-1; i<=1; i++) {
      for(int j=-1; j<=1; j++) {
        if(currentNode->pos.y + i < 0 || currentNode->pos.y + i >= room->sizeY || currentNode->pos.x + j < 0 || currentNode->pos.x +j >= room->sizeX)
          continue;

        if(fabs(i) == fabs(j))
          continue;

        int status = mustBeIgnored(&openedList, &closedList, &(room->nodes[currentNode->pos.y + i][currentNode->pos.x + j]));
        if(status == -1) {
          room->nodes[currentNode->pos.y + i][currentNode->pos.x + j].parent = currentNode;
          room->nodes[currentNode->pos.y + i][currentNode->pos.x + j].G = currentNode->G+1;
          vector_add(&openedList, &(room->nodes[currentNode->pos.y + i][currentNode->pos.x + j]));
        } else if(status != -2) {
          s_node* node = (s_node*)vector_get(&openedList, status);
          if(node->G > currentNode->G + 1) {
            node->G = currentNode->G + 1;
            node->parent = currentNode;
            vector_set(&openedList, status, node);
          }
        }
      }
    }

    if(vector_total(&openedList) == 0)
      pathFound = 2;
  }

  vector path;
  vector_init(&path);
  if(pathFound == 2)
    return path;

  while(last->parent != NULL) {
    vector_add(&path, last);
    last = last->parent;
  }

  vector_free(&openedList);
  vector_free(&closedList);

  return path;
}

int mustBeIgnored(vector* op, vector* cl, s_node* node) {
  for(int i=0; i<vector_total(cl); i++)
    if(vector_get(cl, i) == node)
      return -2;

  if(node->symb == TILE_WALL)
    return -2;

  for(int i=0; i<vector_total(op); i++)
    if(vector_get(op, i) == node)
      return i;

  return -1;
}

int getDistance(s_room* room, s_pos dest) {
  if(room->nodes[dest.y][dest.x].symb == TILE_WALL)
    return -1;

  vector path = getPath(room, dest);

  int distance = vector_total(&path);
  if(distance == 0)
    return -1;

  printf("---------%d\n", distance);
  vector_free(&path);

  return distance;
}

int moveTo(s_room* room, vector* vect) {
  if(room->robot.pos.x == ((s_node*)vector_get(vect, 0))->pos.x && room->robot.pos.y == ((s_node*)vector_get(vect, 0))->pos.y)
    return 1;

  s_pos currentPos = room->robot.pos;
  s_pos nextPos = (s_pos){-1, -1};
  for(int i=1; i<vector_total(vect); i++) {
    if(room->robot.pos.x == ((s_node*)vector_get(vect, i))->pos.x && room->robot.pos.y == ((s_node*)vector_get(vect, i))->pos.y) {
      nextPos = ((s_node*)vector_get(vect, i-1))->pos;
      break;
    }
  }

  if(currentPos.y > nextPos.y)
    moveRobot(room, UP);
  else if(currentPos.x < nextPos.x)
    moveRobot(room, RIGHT);
  else if(currentPos.y > nextPos.y)
    moveRobot(room, DOWN);
  else if(currentPos.x > nextPos.x)
    moveRobot(room, LEFT);


  if(nextPos.x != -1 && nextPos.y != -1) {
    if(room->nodes[nextPos.y][nextPos.x].symb == TILE_EXTINGUISHER) {
      room->nodes[nextPos.y][nextPos.x].symb = ' ';
      room->robot.hasExtinguisher = 1;
    }

    if(room->nodes[nextPos.y][nextPos.x].symb == ' ')
      room->nodes[nextPos.y][nextPos.x].robotVision = TILE_VISITED;
    else if(room->nodes[nextPos.y][nextPos.x].symb == TILE_FIRE_LVL1) {
      room->nodes[nextPos.y][nextPos.x].robotVision = TILE_FIRE_LVL1;
      room->robot.healthPoints -= 1;
    } else if(room->nodes[nextPos.y][nextPos.x].symb == TILE_FIRE_LVL2) {
      room->nodes[nextPos.y][nextPos.x].robotVision = TILE_FIRE_LVL2;
      room->robot.healthPoints -= 2;
    } else if(room->nodes[nextPos.y][nextPos.x].symb == TILE_FIRE_LVL3) {
      room->nodes[nextPos.y][nextPos.x].robotVision = TILE_FIRE_LVL3;
      room->robot.healthPoints -= 3;
    }

    for(int i=-2; i<=2; i++) {
      for(int j=-2; j<=2; j++) {
        if(nextPos.x + j < 1 || nextPos.x + j >= room->sizeX-1 || nextPos.y + i < 1 || nextPos.y + i >= room->sizeY-1)
          continue;

        s_pos dest = (s_pos){nextPos.y + i, nextPos.x + j};
        int distance = getDistance(room, dest);
        if(distance >= 0 && distance <= 2)
          room->nodes[nextPos.y + i][nextPos.x + j].robotVision = TILE_NOFIRE;
      }
    }
  }
  room->robot.moving++;

  return 0;
}
