#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define WALL '#'
#define PATH '.'
#define CURSOR 'o'
#define EXPLORED 'o'
#define FINAL_PATH 'X'
#define EXPLORED_COLOR "\033[0;31m"    // Light Red
#define FINAL_PATH_COLOR "\033[1;32m"  // Bold Green
#define RESET "\033[0m"
#define WALL_COLOR "\033[1;34m"    // Bold Blue
#define PATH_COLOR "\033[1;32m"    // Bold Green
#define CURSOR_COLOR "\033[1;31m"  // Bold Red

typedef struct {  // This defines a Point struct with x and y coordinates.
  int x, y;       //  It's used to represent positions in the maze.
} Point;
// These are global variables:
char** maze;  // A 2D array of characters representing the maze
int width, height;  // Dimensions of the maze

typedef struct {  // This defines a Queue struct used for breadth-first search:
  Point* items;         // items: An array of Points
  int front, rear, size;  // Indexes of the queue and maximum size
} Queue;

Queue* createQueue(int size) {
  Queue* q = malloc(sizeof(Queue)); // Allocates memory for the Queue struct
  q->items = malloc(size * sizeof(Point));  // Allocates memory for the items array
  q->front = -1;
  q->rear = -1; // Initializes front and rear to -1 (empty queue)
  q->size = size; // Sets the size
  return q;   // Returns the newly created queue
}
// Checks if the queue is empty by seeing if front is -1. 
bool isEmptyQueue(Queue* q) { return q->front == -1; }

void enqueue(Queue* q, Point item) {
  if (q->rear == q->size - 1) return; // If the queue is full, it returns without doing anything
  if (q->front == -1) q->front = 0; // If the queue was empty, it sets front to 0
  q->rear++;
  q->items[q->rear] = item; // Increments rear and adds the item
}
// Removes and returns the front item from the queue
Point dequeue(Queue* q) {
  Point item = {-1, -1};
  if (isEmptyQueue(q)) return item;
  item = q->items[q->front];
  q->front++;
  if (q->front > q->rear) {
    q->front = q->rear = -1;
  }
  return item;
}

void clear_screen() { printf("\033[H\033[J"); }

void init_maze() {
  maze = malloc(height * sizeof(char*));  // Allocates memory for the 2D maze array
  for (int i = 0; i < height; i++) {
    maze[i] = malloc(width * sizeof(char));
    for (int j = 0; j < width; j++) {
      maze[i][j] = WALL;                  // Fills the maze with WALL characters
    }
  }
}

void print_maze() {
  for (int i = 0; i < height; i++) {  // Iterates through each cell in the maze
    for (int j = 0; j < width; j++) {
      switch (maze[i][j]) { // Uses a switch statement to determine the appropriate color for each cell type
        case WALL:
          printf("%s%c%s", WALL_COLOR, WALL, RESET); // Prints the cell with its color, then resets the color
          break;
        case PATH:
          printf("%s%c%s", PATH_COLOR, PATH, RESET);
          break;
        case EXPLORED:
          printf("%s%c%s", EXPLORED_COLOR, EXPLORED, RESET);
          break;
        case FINAL_PATH:
          printf("%s%c%s", FINAL_PATH_COLOR, FINAL_PATH, RESET);
          break;
        default:
          printf("%c", maze[i][j]);
      }
    }
    printf("\n");
  }
}

bool is_valid(int x, int y) { // Checks if given coordinates are within the maze boundaries.
  return x >= 0 && x < width && y >= 0 && y < height;
}

void carve_path(int x, int y) { // Recursive backtracking algorithm to generate the maze
  int directions[4][2] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
  int dir_order[4] = {0, 1, 2, 3};

  maze[y][x] = PATH;

  // Fisher-Yates shuffle
  for (int i = 3; i > 0; i--) {
    int j = rand() % (i + 1);
    int temp = dir_order[i];
    dir_order[i] = dir_order[j];
    dir_order[j] = temp;
  }

  for (int i = 0; i < 4; i++) {
    int nx = x + 2 * directions[dir_order[i]][0];
    int ny = y + 2 * directions[dir_order[i]][1];

    if (is_valid(nx, ny) && maze[ny][nx] == WALL) {
      maze[y + directions[dir_order[i]][1]][x + directions[dir_order[i]][0]] =
          PATH;

      // Visualization
      clear_screen();
      maze[ny][nx] = CURSOR;
      print_maze();
      maze[ny][nx] = PATH;
      usleep(50000);  // 50ms delay

      carve_path(nx, ny);
    }
  }
}

void generate_maze() {
  srand(time(NULL));
  init_maze();

  // Create entrance and exit
  maze[0][1] = PATH;
  maze[height - 1][width - 2] = PATH;

  carve_path(1, 1);
}

bool solveMaze() {
  bool** visited = malloc(height * sizeof(bool*));
  Point** parent = malloc(height * sizeof(Point*));
  for (int i = 0; i < height; i++) {
    visited[i] = calloc(width, sizeof(bool));
    parent[i] = malloc(width * sizeof(Point));
  }

  Queue* q = createQueue(width * height);
  Point start = {1, 0};
  Point end = {width - 2, height - 1};
  Point current;

  enqueue(q, start);
  visited[start.y][start.x] = true;

  int dx[] = {-1, 1, 0, 0};
  int dy[] = {0, 0, -1, 1};

  bool found = false;
  while (!isEmptyQueue(q)) {
    current = dequeue(q);

    if (current.x == end.x && current.y == end.y) {
      found = true;
      break;
    }

    for (int i = 0; i < 4; i++) {
      int nx = current.x + dx[i];
      int ny = current.y + dy[i];

      if (nx >= 0 && nx < width && ny >= 0 && ny < height &&
          maze[ny][nx] == PATH && !visited[ny][nx]) {
        Point next = {nx, ny};
        enqueue(q, next);
        visited[ny][nx] = true;
        parent[ny][nx] = current;

        maze[ny][nx] = EXPLORED;
        clear_screen();
        print_maze();
        usleep(50000);  // 5ms delay
      }
    }
  }

  if (found) {
    Point backtrack = end;
    while (!(backtrack.x == start.x && backtrack.y == start.y)) {
      maze[backtrack.y][backtrack.x] = FINAL_PATH;
      backtrack = parent[backtrack.y][backtrack.x];
      clear_screen();
      print_maze();
      usleep(50000);  // 5ms delay
    }
    maze[start.y][start.x] = FINAL_PATH;  // Mark the start point
  }

  // Free allocated memory
  for (int i = 0; i < height; i++) {
    free(visited[i]);
    free(parent[i]);
  }
  free(visited);
  free(parent);
  free(q->items);
  free(q);

  return found;
}

int main() {
  printf("Enter maze width (odd number): ");
  scanf("%d", &width);
  printf("Enter maze height (odd number): ");
  scanf("%d", &height);

  if (width % 2 == 0) width++;
  if (height % 2 == 0) height++;

  generate_maze();
  clear_screen();
  print_maze();

  printf("Maze generated. Press Enter to start solving...");
  getchar();  // Consume newline from previous input
  getchar();  // Wait for Enter key

  if (solveMaze()) {
    printf("Maze solved!\n");
  } else {
    printf("No solution found.\n");
  }

  // Free allocated memory
  for (int i = 0; i < height; i++) {
    free(maze[i]);
  }
  free(maze);

  return 0;
}