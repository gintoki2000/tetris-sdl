#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#define GAME_BOARD_WIDTH            7
#define GAME_BOARD_HEIGHT           11
#define BLOCK_SIZE                  30
#define GAME_WIDTH                  210
#define GAME_HEIGHT                 330
#define TOTAL_SHAPE                 5
#define TOTAL_COLOR                 7


const char T_SHAPE[] = {
  3, 2,
  1, 1, 1,
  0, 1, 0
};

const char Z_SHAPE[] = {
  2, 3,
  1, 0,
  1, 1,
  0, 1
};

const char L_SHAPE[] = {
  2, 3,
  1, 0,
  1, 0,
  1, 1
};

const char SQUARE_SHAPE[] = {
  2, 2,
  1, 1,
  1, 1,
};

const char I_SHAPE[] = {
  1, 4,
  1, 1, 1, 1
};

const char* SHAPES[] = {
  T_SHAPE,
  Z_SHAPE,
  L_SHAPE,
  SQUARE_SHAPE,
  I_SHAPE
};

const int COLORS[] = {
  0xffcc6666,
  0xff66cc66,
  0xff6666cc,
  0xffcccc66,
  0xffcc66cc,
  0xff66cccc,
  0xffdaaa00
};

typedef struct
{
  void (*reset)   ();
  void (*doGameLogic)   ();

} GameState;

typedef struct
{
  int           width, height;
  int*          data;
} Shape;



//sdl
SDL_Window*       g_window = NULL;
SDL_Renderer*     g_renderer = NULL;
SDL_Event         g_main_event;

unsigned int      g_delay = 16;
bool              g_is_game_running = true;
GameState         g_game_state;
unsigned int      g_game_speed = 200;
unsigned int      g_last_move_time;

int               g_score;
int               g_game_board[GAME_BOARD_HEIGHT][GAME_BOARD_WIDTH];
Shape*            g_current_shape = NULL;
Shape*            g_next_shape = NULL;
int               g_shape_x;
int               g_shape_y;

//main function
bool init(); //init sdl
void loop(); //game loop
void quit(); //clean up and quit sdl

//game state functions
void titleReset();
void tileDoGameLogic();

void inGameReset();
void inGameInput();
void inGameTick();
void inGameDoGameLogic();

void gameOverReset();
void gameOverDoGameLogic();


//game board function
void setGameState(GameState new_state);
void tryMoveDown();
void tryMoveLeft();
void tryMoveRight();
void tryRotateLeft();
void tryRotateRight();
void nextShape();
bool isCollidedWithGameBoard(Shape* shape, int x, int y);
void fillShapeToBoard();



const GameState TITLE_STATE     =     {titleReset, tileDoGameLogic};
const GameState IN_GAME_STATE   =     {inGameReset, inGameDoGameLogic};
const GameState GAME_OVER_STATE =     {gameOverReset, gameOverDoGameLogic};


//utils functions
void setColor(int color);
void clearScreen();
void present();
void drawBlock(int x, int y, int color);
//bool isKeyJustPressed(SDL_Scancode);

//shape functions
Shape*    createShape();
void      destroyShape(Shape* shape);
void      paintShape(Shape* shape, int x, int y);

int main(int argc, char** argv)
{
  if(init ())
    loop ();
  quit ();

  return 0;
}

bool init()
{
  if(SDL_Init(SDL_INIT_EVERYTHING) == -1)
    {
      printf("Could not init sdl: %s\n", SDL_GetError ());
      return false;
    }
  g_window = SDL_CreateWindow ("tertris sdl", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               GAME_WIDTH, GAME_HEIGHT,
                               SDL_WINDOW_SHOWN
             );
  if(!g_window)
    {
      printf("Could not create window: %s\n", SDL_GetError ());
      return false;
    }
  g_renderer = SDL_CreateRenderer (g_window, -1, SDL_RENDERER_ACCELERATED);
  if(!g_renderer)
    {
      printf("Could not create renderer :%s\n", SDL_GetError ());
      return false;
    }

  SDL_SetRenderDrawBlendMode (g_renderer, SDL_BLENDMODE_BLEND);
  srand (SDL_GetTicks ());
  setGameState (IN_GAME_STATE);
  return true;
}

void loop()
{
  unsigned int start_ticks;
  unsigned int used_ticks;
  while(g_is_game_running)
    {
      start_ticks = SDL_GetTicks ();
      clearScreen();
      g_game_state.doGameLogic();
      present ();
      used_ticks = SDL_GetTicks () - start_ticks;
      if(used_ticks < g_delay)
        {
          SDL_Delay (g_delay - used_ticks);
        }
    }
}

void quit()
{
    destroyShape (g_current_shape);
    destroyShape (g_next_shape);
  SDL_DestroyWindow (g_window);
  SDL_DestroyRenderer (g_renderer);
  SDL_Quit();
}

void titleReset()
{

}


void tileDoGameLogic()
{

}

void inGameReset()
{
  g_next_shape = createShape ();
  nextShape ();
  for(int x = 0; x < GAME_BOARD_WIDTH; x++)
    {
      for(int y = 0; y < GAME_BOARD_HEIGHT; y++)
        {
          g_game_board[y][x] = 0;
        }
    }
  g_last_move_time = SDL_GetTicks ();
}

void inGameInput()
{
   while (SDL_PollEvent (&g_main_event))
    {
      if(g_main_event.type == SDL_QUIT)
        g_is_game_running = false;
    }
}

void clearRow(int row)
{
  for(int i = row; i > 0; --i)
    {
      for(int j = 0; j < GAME_BOARD_WIDTH; ++j)
        {
          g_game_board[i][j] = g_game_board[i - 1][j];
        }
    }
}

SDL_bool isFilledRow(int row)
{
  for(int j = 0; j < GAME_BOARD_WIDTH; ++j)
    {
      if(g_game_board[row][j] == 0)
        {
          return SDL_FALSE;
        }
    }
  return SDL_TRUE;
}

void inGameTick()
{



  const unsigned char* key_states = SDL_GetKeyboardState (NULL);

  if(key_states[SDL_SCANCODE_LEFT])
    {
      tryMoveLeft ();
    }
  else if(key_states[SDL_SCANCODE_RIGHT])
    {
      tryMoveRight ();
    }
  else if (key_states[SDL_SCANCODE_UP])
    {
      tryRotateLeft ();
    }
  else if (key_states[SDL_SCANCODE_DOWN])
    {
      tryRotateRight ();
    }
  else if (key_states[SDL_SCANCODE_SPACE])
    {

    }

  if(SDL_GetTicks () - g_last_move_time >= g_game_speed)
    {
      tryMoveDown();
      g_last_move_time = SDL_GetTicks ();
    }

}

void inGameDoGameLogic()
{
  inGameInput ();
  inGameTick ();
  //render
  for(int x = 0; x < GAME_BOARD_WIDTH; ++x)
    {
      for(int y = 0; y < GAME_BOARD_HEIGHT; ++y)
        {
          if(g_game_board[y][x])
            {
              drawBlock (x, y, g_game_board[y][x]);
            }
        }
    }
  paintShape (g_current_shape, g_shape_x, g_shape_y);

}


void gameOverReset()
{

}

void gameOverDoGameLogic()
{

}

void setGameState(GameState new_state)
{
  g_game_state = new_state;
  g_game_state.reset();
}

void tryMoveDown()
{
  if(isCollidedWithGameBoard(g_current_shape, g_shape_x, g_shape_y + 1))
    {
      fillShapeToBoard();
    }
  else
    {
      ++g_shape_y;
    }
}

void tryMoveLeft()
{
  if(g_shape_x > 0 && !isCollidedWithGameBoard(g_current_shape, g_shape_x - 1, g_shape_y))
    {
      --g_shape_x;
    }
}

void tryMoveRight()
{
  if(g_shape_x + g_current_shape->width < GAME_BOARD_WIDTH
    && !isCollidedWithGameBoard(g_current_shape, g_shape_x + 1, g_shape_y))
    {
      ++g_shape_x;
    }
}

bool isCollidedWithGameBoard(Shape* shape, int x, int y)
{
  if(y + shape->height > GAME_BOARD_HEIGHT)
    {
      return true;
    }
  for(int px = 0; px < shape->width; ++px)
    {
      for(int py = 0; py < shape->height; ++py)
        {
          if(shape->data[px + py * shape->width]
             && g_game_board[py + y][px + x])
            {
              return true;
            }
        }
    }
  return false;
}

void fillShapeToBoard()
{
  for(int x = 0; x < g_current_shape->width; ++x)
    {
      for(int y = 0; y < g_current_shape->height; ++y)
        {
          if(g_current_shape->data[x + y * g_current_shape->width])
            g_game_board[y + g_shape_y][x + g_shape_x] = g_current_shape->data[x + y * g_current_shape->width];
        }
    }
  nextShape();
}

void nextShape()
{
  destroyShape (g_current_shape);
  g_current_shape = g_next_shape;
  g_next_shape = createShape ();
  g_shape_x = 2;
  g_shape_y = 0;
}

void tryRotateLeft()
{
  Shape* rotated_shape = (Shape*) malloc (sizeof (Shape));
  rotated_shape->width = g_current_shape->height;
  rotated_shape->height = g_current_shape->width;
  rotated_shape->data = (int*) malloc (sizeof (int) * rotated_shape->width * rotated_shape->height);

  for(int px = 0; px < g_current_shape->width; ++px)
    {
      for(int py = 0; py < g_current_shape->height; ++py)
        {
          int new_x = -py + g_current_shape->height - 1;
          int new_y = px;

          rotated_shape->data[new_x + new_y * rotated_shape->width]
            = g_current_shape->data[px + py * g_current_shape->width];
        }
    }
  if(isCollidedWithGameBoard (rotated_shape, g_shape_x, g_shape_y))
    {
      destroyShape (rotated_shape);
    }
  else
    {
      destroyShape (g_current_shape);
      g_current_shape = rotated_shape;
    }
}

void tryRotateRight()
{
  Shape* rotated_shape = (Shape*) malloc (sizeof (Shape));
  rotated_shape->width = g_current_shape->height;
  rotated_shape->height = g_current_shape->width;
  rotated_shape->data = (int*) malloc (sizeof (int) * rotated_shape->width * rotated_shape->height);

  for(int px = 0; px < g_current_shape->width; ++px)
    {
      for(int py = 0; py < g_current_shape->height; ++py)
        {
          int new_x = py;
          int new_y = -px + g_current_shape->width - 1;

          rotated_shape->data[new_x + new_y * rotated_shape->width]
            = g_current_shape->data[px + py * g_current_shape->width];
        }
    }
  if(isCollidedWithGameBoard (rotated_shape, g_shape_x, g_shape_y))
    {
      destroyShape (rotated_shape);
    }
  else
    {
      destroyShape (g_current_shape);
      g_current_shape = rotated_shape;
    }
}

void setColor(int color)
{
  unsigned char a = (color >> 24) & 0xff;
  unsigned char r = (color >> 16) & 0xff;
  unsigned char g = (color >> 8 ) & 0xff;
  unsigned char b = color & 0xff;
  SDL_SetRenderDrawColor (g_renderer, r, g, b, a);
}

void clearScreen()
{
  setColor (0xffffffff);
  SDL_RenderClear (g_renderer);
}

void present()
{
  SDL_RenderPresent (g_renderer);
}

void drawBlock(int x, int y, int color)
{
  static SDL_Rect rect;
  rect.w = BLOCK_SIZE - 2;
  rect.h = BLOCK_SIZE - 2;
  rect.x = (x * BLOCK_SIZE) + 1;
  rect.y = (y * BLOCK_SIZE) + 1;
  setColor (color);
  SDL_RenderFillRect (g_renderer, &rect);

  setColor (0xff000000);

  rect.w = BLOCK_SIZE;
  rect.h = BLOCK_SIZE;
  rect.x = BLOCK_SIZE * x;
  rect.y = BLOCK_SIZE * y;

  SDL_RenderDrawRect (g_renderer, &rect);
}

//shape
Shape* createShape()
{
  const char* data = SHAPES[rand () % TOTAL_SHAPE];
  const int color = COLORS[rand () % TOTAL_COLOR];
  Shape* shape = (Shape*) malloc (sizeof (Shape));

  shape->width = data[0];
  shape->height = data[1];

  shape->data = (int*) malloc (shape->width * shape->height * sizeof (int));
  for(int x = 0; x < shape->width; ++x)
    {
      for(int y = 0; y < shape->height; ++y)
        {
          int index = x + y * shape->width;
          if(data[index + 2])
            {
              shape->data[index] = color;
            }
          else
            {
              shape->data[index] = 0;
            }
        }
    }
  return shape;
}

void destroyShape(Shape* shape)
{
  if(!shape) return;
  free(shape->data);
  free(shape);
}



void paintShape(Shape* shape, int x, int y)
{
  for(int sx = 0; sx < shape->width; ++sx)
    {
      for(int sy = 0; sy < shape->height; ++sy)
        {
          int color = shape->data[sy * shape->width + sx];
          if(color)
            drawBlock (sx + x, sy + y, color);
        }
    }
}

