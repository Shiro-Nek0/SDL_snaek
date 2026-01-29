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

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

// TODO:
// make scripts to get SDL2 source code for mingw (./build/windows/deps) and android (./build/android/deps) and also execute download for android in external folder, maybe it can be unified?

const std::string WINDOW_TITLE = "snaek";
const int WINDOW_WIDTH = 1280;
const int WINDOW_HEIGHT = 720;
Vector2 windowSize = {WINDOW_WIDTH, WINDOW_HEIGHT};

enum {
  EMPTY,
  APPLE,
  PLAYER,
  PLAYERTAIL
};

std::random_device randomGenerator;
std::mt19937 gen(randomGenerator());

bool running = true;
bool isPaused = false;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

// stage
const int GRID_COLS = 15;
const int GRID_ROWS = 15;
int gameMatrix[GRID_ROWS][GRID_COLS] = {EMPTY};
Vector2 blockSize = {1, 1};

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
    bool occupied = false;

    if (randPos.x == playerHead.x && randPos.y == playerHead.y) {
      occupied = true;
    }

    for (int i = 0; i < points; i++) {
      if (randPos.x == playerTail[i].x && randPos.y == playerTail[i].y) {
        occupied = true;
        break;
      }
    }

    if (!occupied) {
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
  std::string titleWithDate = WINDOW_TITLE + " [Built: " + __DATE__ + " " + __TIME__ + "]";

  window = SDL_CreateWindow(titleWithDate.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
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

float touchStartX = 0.0f;
float touchStartY = 0.0f;
const float SWIPE_THRESHOLD = 0.05f;
void handleTouch(SDL_TouchFingerEvent tfinger) {
  float tfX = tfinger.x;
  float tfY = tfinger.y;

  float dx = tfX - touchStartX;
  float dy = tfY - touchStartY;

  if (abs(dx) > SWIPE_THRESHOLD || abs(dy) > SWIPE_THRESHOLD) {
    if (abs(dx) > abs(dy)) {
      if (dx > 0) {
        if (playerHeading.x == 0)
          playerHeading = {1, 0};
      } else {
        if (playerHeading.x == 0)
          playerHeading = {-1, 0};
      }
    } else {
      if (dy > 0) {
        if (playerHeading.y == 0)
          playerHeading = {0, 1};
      } else {
        if (playerHeading.y == 0)
          playerHeading = {0, -1};
      }
    }

    touchStartX = tfX;
    touchStartY = tfY;
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
  SDL_RenderSetViewport(renderer, NULL);
  SDL_RenderSetClipRect(renderer, NULL);

  SDL_Rect cell = {0, 0, windowSize.x, windowSize.y};
  SDL_SetRenderDrawColor(renderer, 25, 25, 25, 255);
  SDL_RenderDrawRect(renderer, &cell);
  SDL_RenderClear(renderer);
}

void drawFG() {
  int lineWidth = 2;
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

  // Use windowSize instead of WINDOW_WIDTH
  int startX = (windowSize.x - (GRID_COLS * blockSize.x)) / 2;
  int startY = (windowSize.y - (GRID_ROWS * blockSize.y)) / 2;

  for (int y = 0; y < GRID_ROWS; y++) {
    for (int x = 0; x < GRID_COLS; x++) {
      int rectX = startX + (x * blockSize.x);
      int rectY = startY + (y * blockSize.y);
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

  // Update text positioning to stay centered in the new window size
  SDL_Color textColor = {255, 255, 255, 255};
  std::string scoreText = "Points: " + std::to_string(points);
  SDL_Surface *textSurface = TTF_RenderText_Blended(f_font, scoreText.c_str(), textColor);

  if (textSurface != NULL) {
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (textTexture != NULL) {
      SDL_Rect textRect;
      textRect.w = textSurface->w;
      textRect.h = textSurface->h;
      textRect.x = (windowSize.x / 2) - (textRect.w / 2); // Dynamic center
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
  int startX = (windowSize.x - (GRID_COLS * blockSize.x)) / 2;
  int startY = (windowSize.y - (GRID_ROWS * blockSize.y)) / 2;

  SDL_Rect appleRender = {startX + (applePos.x * blockSize.x), startY + (applePos.y * blockSize.y), blockSize.x, blockSize.y};
  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  SDL_RenderFillRect(renderer, &appleRender);

  SDL_Rect playerHeadRender = {startX + (playerHead.x * blockSize.x), startY + (playerHead.y * blockSize.y), blockSize.x, blockSize.y};
  SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
  SDL_RenderFillRect(renderer, &playerHeadRender);

  for (int i = 0; i < points; i++) {
    SDL_Rect tailRect = {startX + (playerTail[i].x * blockSize.x), startY + (playerTail[i].y * blockSize.y), blockSize.x, blockSize.y};

    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
    SDL_RenderFillRect(renderer, &tailRect);
  }
}

void resizeBlocks() {
  SDL_GetWindowSize(window, &windowSize.x, &windowSize.y);

  int minDim = std::min(windowSize.x, windowSize.y);

  int size = minDim / std::max(GRID_COLS, GRID_ROWS);
  blockSize = {size, size};
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

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  ImGui::StyleColorsDark();

  ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
  ImGui_ImplSDLRenderer2_Init(renderer);

  SDL_Event windowEvent;

  Uint32 startTime = SDL_GetTicks();
  int frameCount = 0;
  int lastMoveTime = 0;
  resizeBlocks();
  reset();
  while (running) {
    if (SDL_PollEvent(&windowEvent)) {
      ImGui_ImplSDL2_ProcessEvent(&windowEvent);
      switch (windowEvent.type) {
      case SDL_WINDOWEVENT:
        switch (windowEvent.window.event) {
        case SDL_WINDOWEVENT_SIZE_CHANGED:
        case SDL_WINDOWEVENT_RESIZED:
          resizeBlocks();
          break;
        }
        break;
      case SDL_QUIT:
        running = false;
        break;
      case SDL_KEYDOWN:
        if (windowEvent.key.keysym.scancode == SDL_SCANCODE_ESCAPE ||
            windowEvent.key.keysym.scancode == SDL_SCANCODE_AC_BACK) {
          isPaused = !isPaused;
        } else if (!io.WantCaptureKeyboard && !isPaused) {
          handleInput(windowEvent.key.keysym.scancode);
        }
        break;
      // case SDL_MOUSEBUTTONDOWN:
      // case SDL_MOUSEBUTTONUP:
      //   fingerPos = {0, 0};
      //   SDL_Log("M");
      //   break;
      // case SDL_MOUSEMOTION:
      //   //fingerPos = {int(windowEvent. * 10), int(windowEvent.tfinger.y * 10)};
      //   SDL_Log("MM %i, %i", fingerPos.x, fingerPos.y);
      //   break;
      case SDL_FINGERDOWN:
        touchStartX = windowEvent.tfinger.x;
        touchStartY = windowEvent.tfinger.y;
        break;
      case SDL_FINGERMOTION:
        handleTouch(windowEvent.tfinger);
        break;
      }
    }

    // if (Mix_PlayingMusic() == 0) {
    //   Mix_PlayMusic(m_BGM, -1);
    // }

    int now = SDL_GetTicks();
    if (!isPaused && (now - lastMoveTime >= moveDelay)) {
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
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
    if (isPaused) {

      ImGui::SetNextWindowPos(
          ImVec2(windowSize.x / 2.0f, windowSize.y / 2.0f),
          ImGuiCond_Always,
          ImVec2(0.5f, 0.5f));

      ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove |
                                      ImGuiWindowFlags_NoCollapse |
                                      ImGuiWindowFlags_NoResize |
                                      ImGuiWindowFlags_AlwaysAutoResize;

      ImGui::Begin("PAUSED", NULL, window_flags);

      if (ImGui::Button("Continue", ImVec2(200, 0))) {
        isPaused = false;
      }

      if (ImGui::Button("Restart", ImVec2(200, 0))) {
        reset();
        isPaused = false;
      }

      ImGui::Separator();

      if (ImGui::Button("Quit", ImVec2(200, 0))) {
        running = false;
      }

      ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);

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