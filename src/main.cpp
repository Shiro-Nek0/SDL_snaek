#include "DrawUtils.h"

#include <SDL.h>
#include <SDL_error.h>
#include <SDL_events.h>
#include <SDL_image.h>
#include <SDL_keycode.h>
#include <SDL_log.h>
#include <SDL_mixer.h>
#include <SDL_rect.h>
#include <SDL_render.h>
#include <SDL_scancode.h>
#include <SDL_surface.h>
#include <SDL_timer.h>
#include <SDL_ttf.h>
#include <SDL_video.h>
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <random>
#include <stdio.h>
#include <string>

// TODO:
// make scripts to get SDL2 source code for mingw (./build/windows/deps) and android (./build/android/deps) and also execute download for android in external folder, maybe it can be unified?
// make the playerTail set stuff in gameMatrix so the apple doesnt spawn where the snake is
using namespace std;

const string WINDOW_TITLE = "snaek";
const int WINDOW_WIDTH = 400;
const int WINDOW_HEIGHT = 400;

struct Vector2 {
  int x;
  int y;
};

enum {
  EMPTY,
  APPLE,
  PLAYER,
  PLAYERTAIL
};

random_device randomGenerator;
mt19937 gen(randomGenerator());

bool running = true;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

// stage
const int GRID_COLS = 15;
const int GRID_ROWS = 15;
int gameMatrix[GRID_ROWS][GRID_COLS] = {EMPTY};
Vector2 blockSize = {WINDOW_WIDTH / GRID_COLS, WINDOW_HEIGHT / GRID_ROWS};

// assets
int fontSize = 32;
TTF_Font *f_font = NULL;
Mix_Music *m_BGM = NULL;
Mix_Chunk *s_Point = NULL;
Mix_Chunk *s_Death = NULL;

// player
Vector2 playerHead = {0, 0};
Vector2 playerTail[GRID_COLS * GRID_ROWS] = {0};
Vector2 playerHeading = {0, 0};
int points = 0;
int moveDelay = 150;
int currentFPS = 0;

// apple
Vector2 applePos = {0, 0};

void updateTailPosition(int newHeadX, int newHeadY) {
  for (int i = points - 1; i > 0; i--) {
    playerTail[i] = playerTail[i - 1];
  }

  if (points > 0) {
    playerTail[0] = {playerHead.x, playerHead.y};
  }

  playerHead = {newHeadX, newHeadY};
}

Vector2 getRandPos() {
  std::uniform_int_distribution<> distrX(0, GRID_COLS - 1);
  std::uniform_int_distribution<> distrY(0, GRID_ROWS - 1);
  int randX = distrX(gen);
  int randY = distrY(gen);

  return Vector2{randX, randY};
}

void setApplePos() {
  while (true) {
    Vector2 randPos = getRandPos();

    if (gameMatrix[randPos.x][randPos.y] == EMPTY) {

      applePos = randPos;
      gameMatrix[randPos.x][randPos.y] = APPLE;

      break;
    }
  }
}

void setPlayerPos() {
  while (true) {
    Vector2 randPos = getRandPos();

    if (gameMatrix[randPos.x][randPos.y] == EMPTY) {

      playerHead = randPos;
      gameMatrix[randPos.x][randPos.y] = PLAYER;

      break;
    }
  }

  playerHeading = {0, 0};
  std::fill_n(playerTail, GRID_COLS * GRID_ROWS, Vector2{0, 0});
}

bool initializeSDL() {
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL could not initialize! SDL Error: %s\n", SDL_GetError());
    return false;
  }

  if (TTF_Init() < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_ttf could not initialize! TTF_Error: %s\n", TTF_GetError());
    return false;
  }

  int imgFlags = IMG_INIT_PNG;
  if (!(IMG_Init(imgFlags) & imgFlags)) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
    return false;
  }

  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
    return false;
  }
  return true;
}

bool loadAssets() {
  // //Load prompt texture
  // if( !gPromptTexture.loadFromFile(ASSET_PATH "21_sound_effects_and_music/prompt.png" ) )
  // {
  //     printf( "Failed to load prompt texture!\n" );
  //     return false;
  // }

  SDL_Surface *icon = IMG_Load(ASSET_PATH "icon.png");
  if (icon == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not load icon: %s\n", IMG_GetError());
    return false;
  }

  SDL_SetWindowIcon(window, icon);
  SDL_FreeSurface(icon);

  f_font = TTF_OpenFont(ASSET_PATH "font.ttf", fontSize);

  if (f_font == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load font: %s\n", TTF_GetError());
    return false;
  }

  m_BGM = Mix_LoadMUS(ASSET_PATH "bgm.wav");
  if (m_BGM == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load bgm music! SDL_mixer Error: %s\n", Mix_GetError());
    return false;
  }

  s_Point = Mix_LoadWAV(ASSET_PATH "point.wav");
  if (s_Point == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load point sound effect! SDL_mixer Error: %s\n", Mix_GetError());
    return false;
  }

  s_Death = Mix_LoadWAV(ASSET_PATH "death.wav");
  if (s_Death == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Failed to load death sound effect! SDL_mixer Error: %s\n", Mix_GetError());
    return false;
  }

  return true;
}

bool initializeWin() {
  window = SDL_CreateWindow(WINDOW_TITLE.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  if (!window) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Window could not be created! SDL_Error:  %s\n", SDL_GetError());
    SDL_Quit();
    return false;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED /* | SDL_RENDERER_PRESENTVSYNC*/);
  if (!renderer) {
    SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Renderer could not be created! SDL_Error:  %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return false;
  }

  SDL_RenderSetLogicalSize(renderer, WINDOW_WIDTH, WINDOW_HEIGHT);

  return true;
}

void reset() {
  points = 0;

  setApplePos();
  setPlayerPos();
}

void handleInput(SDL_Scancode key) {
  switch (key) {
  case SDL_SCANCODE_W:
    if (playerHeading.y == 0) {
      playerHeading = {0, -1};
    }
    break;

  case SDL_SCANCODE_S:
    if (playerHeading.y == 0) {
      playerHeading = {0, 1};
    }
    break;

  case SDL_SCANCODE_A:
    if (playerHeading.x == 0) {
      playerHeading = {-1, 0};
    }
    break;

  case SDL_SCANCODE_D:
    if (playerHeading.x == 0) {
      playerHeading = {1, 0};
    }
    break;

  case SDL_SCANCODE_R:
    reset();
    break;

  case SDL_SCANCODE_ESCAPE:
    running = false;
    break;

  case SDL_SCANCODE_M:
    if (Mix_PlayingMusic() == 0) {
      Mix_PlayMusic(m_BGM, -1);
    } else {
      if (Mix_PausedMusic() == 1) {
        Mix_ResumeMusic();
      } else {
        Mix_PauseMusic();
      }
    }
    break;

  default:
    break;
  }
}

void close() {
  // gPromptTexture.free();

  TTF_CloseFont(f_font);
  Mix_FreeMusic(m_BGM);
  Mix_FreeChunk(s_Point);
  Mix_FreeChunk(s_Death);

  f_font = NULL;
  s_Point = NULL;
  s_Death = NULL;
  m_BGM = NULL;

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  window = NULL;
  renderer = NULL;

  TTF_Quit();
  Mix_Quit();
  IMG_Quit();
  SDL_Quit();
}

void drawBG() {
  SDL_Rect cell = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
  SDL_SetRenderDrawColor(renderer, 25, 25, 25, 255);
  SDL_RenderDrawRect(renderer, &cell);
  SDL_RenderClear(renderer);
}

void drawFG() {
  int lineWidth = 2;
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

  for (int y = 0; y < GRID_ROWS; y++) {
    for (int x = 0; x < GRID_COLS; x++) {
      int rectX = x * blockSize.x;
      int rectY = y * blockSize.y;
      int rectW = blockSize.x;
      int rectH = blockSize.y;

      SDL_Rect borders[4] = {
          {rectX, rectY, rectW, lineWidth},
          {rectX, rectY + rectH - lineWidth, rectW, lineWidth},
          {rectX, rectY, lineWidth, rectH},
          {rectX + rectW - lineWidth, rectY, lineWidth, rectH}};

      SDL_RenderFillRects(renderer, borders, 4);
    }
  }

  SDL_Color textColor = {255, 255, 255, 255};
  std::string scoreText = "Points: " + std::to_string(points);

  SDL_Surface *textSurface = TTF_RenderText_Blended(f_font, scoreText.c_str(), textColor);

  if (textSurface != NULL) {
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    if (textTexture != NULL) {
      SDL_Rect textRect;
      textRect.w = textSurface->w;
      textRect.h = textSurface->h;
      textRect.x = (WINDOW_WIDTH / 2) - (textRect.w / 2);
      textRect.y = 15;

      SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

      SDL_DestroyTexture(textTexture);
    }
    SDL_FreeSurface(textSurface);
  }

  SDL_Color fpsColor = {255, 255, 0, 255};
  std::string fpsText = "FPS: " + std::to_string(currentFPS);
  SDL_Surface *fpsSurface = TTF_RenderText_Blended(f_font, fpsText.c_str(), fpsColor);

  if (fpsSurface != NULL) {
    SDL_Texture *fpsTexture = SDL_CreateTextureFromSurface(renderer, fpsSurface);
    if (fpsTexture != NULL) {
      SDL_Rect fpsRect;
      fpsRect.w = fpsSurface->w;
      fpsRect.h = fpsSurface->h;
      fpsRect.x = 5;
      fpsRect.y = 5;
      SDL_RenderCopy(renderer, fpsTexture, NULL, &fpsRect);
      SDL_DestroyTexture(fpsTexture);
    }
    SDL_FreeSurface(fpsSurface);
  }
}

void drawPlayer() {
  SDL_Rect appleRender = {applePos.x * blockSize.x, applePos.y * blockSize.y, blockSize.x, blockSize.y};
  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  SDL_RenderFillRect(renderer, &appleRender);

  SDL_Rect playerHeadRender = {playerHead.x * blockSize.x, playerHead.y * blockSize.y, blockSize.x, blockSize.y};
  SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
  SDL_RenderFillRect(renderer, &playerHeadRender);

  for (int i = 0; i < points; i++) {
    SDL_Rect tailRect = {playerTail[i].x * blockSize.x, playerTail[i].y * blockSize.y, blockSize.x, blockSize.y};

    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
    SDL_RenderFillRect(renderer, &tailRect);
  }
}

int main(int argc, char *argv[]) {
  if (!initializeSDL()) {
    return EXIT_FAILURE;
  }

  if (!loadAssets()) {
    return EXIT_FAILURE;
  }

  if (!initializeWin()) {
    return EXIT_FAILURE;
  }

  SDL_Event windowEvent;

  Uint32 startTime = SDL_GetTicks();
  int frameCount = 0;
  int lastMoveTime = 0;

  reset();
  while (running) {
    if (SDL_PollEvent(&windowEvent)) {
      switch (windowEvent.type) {
      case SDL_QUIT:
        running = false;
        break;
      case SDL_KEYDOWN:
        handleInput(windowEvent.key.keysym.scancode);
        break;
      case SDL_KEYUP:
        break;
      }
    }

    // if (Mix_PlayingMusic() == 0) {
    //   Mix_PlayMusic(m_BGM, -1);
    // }

    if (playerHead.x == applePos.x && playerHead.y == applePos.y) {
      points += 1;
      setApplePos();
      Mix_PlayChannel(-1, s_Point, 0);
    }

    int now = SDL_GetTicks();
    if (now - lastMoveTime >= moveDelay) {
      for (int i = points; i > 0; i--) {
        playerTail[i] = playerTail[i - 1];
      }

      if (points > 0) {
        playerTail[0] = playerHead;
      }

      playerHead.x += playerHeading.x;
      playerHead.y += playerHeading.y;

      if (playerHead.x < 0) {
        playerHead.x = GRID_COLS - 1;
      } else if (playerHead.x >= GRID_COLS) {
        playerHead.x = 0;
      }
      if (playerHead.y < 0) {
        playerHead.y = GRID_ROWS - 1;
      } else if (playerHead.y >= GRID_ROWS) {
        playerHead.y = 0;
      }

      if (playerHead.x == applePos.x && playerHead.y == applePos.y) {
        points++;
        setApplePos();
        Mix_PlayChannel(-1, s_Point, 0);
      }

      for (int i = 0; i < points; i++) {
        if (playerHead.x == playerTail[i].x && playerHead.y == playerTail[i].y) {
          Mix_PlayChannel(-1, s_Death, 0);
          reset();
          break;
        }
      }

      lastMoveTime = now;
    }

    drawBG();
    drawPlayer();
    drawFG();
    SDL_RenderPresent(renderer);

    frameCount++;
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - startTime >= 1000) {
      currentFPS = frameCount;

      // string title = WINDOW_TITLE + " - FPS: " + to_string(frameCount);
      // SDL_SetWindowTitle(window, title.c_str());

      frameCount = 0;
      startTime = currentTime;
    }
  }

  close();
  return EXIT_SUCCESS;
}