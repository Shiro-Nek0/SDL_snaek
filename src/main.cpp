#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_ttf.h>
#include <SDL_events.h>
#include <SDL_scancode.h>
#include <SDL_video.h>
#include <cstddef>
#include <cstdlib>
#include <stdio.h>
#include <string>

using namespace std;

const string WINDOW_TITLE = "snaek";
const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 480;
const int FRAMERATE = 60;
bool running = true;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;

TTF_Font *f_font = NULL;
Mix_Music *m_BGM = NULL;
Mix_Chunk *s_Point = NULL;
Mix_Chunk *s_Death = NULL;

bool initialize() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    fprintf(stderr, "SDL could not initialize! SDL Error: %s\n", SDL_GetError());
    return false;
  }

  if (TTF_Init() < 0) {
    fprintf(stderr, "SDL_ttf could not initialize! TTF_Error: %s\n", TTF_GetError());
    return false;
  }

  int imgFlags = IMG_INIT_PNG;
  if (!(IMG_Init(imgFlags) & imgFlags)) {
    fprintf(stderr, "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
    return false;
  }

  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    fprintf(stderr, "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
    return false;
  }
  return true;
}

bool loadMedia() {
  // //Load prompt texture
  // if( !gPromptTexture.loadFromFile( "21_sound_effects_and_music/prompt.png" ) )
  // {
  //     printf( "Failed to load prompt texture!\n" );
  //     return false;
  // }

  f_font = TTF_OpenFont("assets/font.ttf", 24);

  if (f_font == NULL) {
    fprintf(stderr, "Failed to load font: %s\n", TTF_GetError());
    return false;
  }

  m_BGM = Mix_LoadMUS("assets/bgm.wav");
  if (m_BGM == NULL) {
    fprintf(stderr, "Failed to load bgm music! SDL_mixer Error: %s\n", Mix_GetError());
    return false;
  }

  s_Point = Mix_LoadWAV("assets/point.wav");
  if (s_Point == NULL) {
    fprintf(stderr, "Failed to load point sound effect! SDL_mixer Error: %s\n", Mix_GetError());
    return false;
  }

  s_Death = Mix_LoadWAV("assets/death.wav");
  if (s_Death == NULL) {
    fprintf(stderr, "Failed to load death sound effect! SDL_mixer Error: %s\n", Mix_GetError());
    return false;
  }

  return true;
}

void reset();
void handleInput() {
  const Uint8 *currentKeyStates = SDL_GetKeyboardState(NULL);

  if (currentKeyStates[SDL_SCANCODE_W]) {
  }

  if (currentKeyStates[SDL_SCANCODE_S]) {
  }

  if (currentKeyStates[SDL_SCANCODE_A]) {
  }

  if (currentKeyStates[SDL_SCANCODE_D]) {
  }

  if (currentKeyStates[SDL_SCANCODE_R]) {
    reset();
  }

  if (currentKeyStates[SDL_SCANCODE_M]) {
    if (Mix_PlayingMusic() == 0) {
      Mix_PlayMusic(m_BGM, -1);
    } else {
      if (Mix_PausedMusic() == 1) {
        Mix_ResumeMusic();
      } else {
        Mix_PauseMusic();
      }
    }
  }

  if (currentKeyStates[SDL_SCANCODE_ESCAPE]) {
    running = false;
  }
}

void reset() {
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

int main(int argc, char *argv[]) {
  if (!initialize()) {
    return EXIT_FAILURE;
  }

  if (!loadMedia()) {
    return EXIT_FAILURE;
  }

  window = SDL_CreateWindow("SDL2 Window - FPS: 0", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

  if (!window) {
    fprintf(stderr, "Window could not be created! SDL_Error:  %s\n", SDL_GetError());
    SDL_Quit();
    return EXIT_FAILURE;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    fprintf(stderr, "Renderer could not be created! SDL_Error:  %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return EXIT_FAILURE;
  }

  SDL_Event windowEvent;

  Uint32 startTime = SDL_GetTicks();
  int frameCount = 0;

  while (running) {
    if (SDL_PollEvent(&windowEvent)) {
      switch (windowEvent.type) {
      case SDL_QUIT:
        running = false;
        break;
      }
    }

    // if (Mix_PlayingMusic() == 0) {
    //   Mix_PlayMusic(m_BGM, -1);
    // }

    handleInput();

    SDL_SetRenderDrawColor(renderer, 25, 25, 25, 255);
    SDL_RenderClear(renderer);

    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Surface *textSurface = TTF_RenderText_Solid(f_font, "Pts: 0", textColor);

    if (!textSurface) {
      printf("Failed to create text surface: %s\n", TTF_GetError());
      return EXIT_FAILURE;
    }

    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);

    if (!textTexture) {
      printf("Failed to create text texture: %s\n", SDL_GetError());
      return EXIT_FAILURE;
    }

    SDL_Rect textRect = {WINDOW_WIDTH/2-textSurface->w/2, 20, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);

    SDL_RenderPresent(renderer);

    frameCount++;
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - startTime >= 1000) {
      string title = WINDOW_TITLE + " - FPS: " + to_string(frameCount);
      SDL_SetWindowTitle(window, title.c_str());

      frameCount = 0;
      startTime = currentTime;
    }
  }

  close();
  return EXIT_SUCCESS;
}