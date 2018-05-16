#include "includes.h"
#include "room.h"

int main() {
  int stop=0;
  s_room room = loadRoom(DEFAULT_ROOM);
  vector path = getBestPath(&room, room.startPos, room.extinguisherPos);
  displayRoom(&room, 0);

  int idx = vector_total(&path)-2;

  while(!stop) {
      displayRoom(&room, 1);

      if(room.robot.status == STATUS_GO_TO_EXTINGUISHER) {
        if(idx >= 0)
          idx = moveTo(&room, &path, idx);
        else
          printf("[WARNING] Unpredicted behavior\n");
      }
      else if(room.robot.status == STATUS_DETERMINE_INTERESTING_POINT) {
        vector_free(&path);
        path = nextNodePath(&room);

        idx = vector_total(&path)-2;
        printf("%d\n", idx);
      }
      else if(room.robot.status == STATUS_SEARCH_FIRE) {
        if(idx >= 0)
          idx = moveTo(&room, &path, idx);
        else
          printf("[WARNING] Unpredicted behavior\n");
      } else {
        stop = 1;
      }

      usleep(50000);
  }
  displayRoom(&room, 1);

  printf("%d\n", getDistance(&room, (s_pos){6, 7}));

  vector_free(&path);
  return EXIT_SUCCESS;
}
