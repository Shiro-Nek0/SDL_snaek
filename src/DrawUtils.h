#include <SDL2/SDL.h>

enum class Align {
    TOP_LEFT,
    MIDDLE_LEFT,
    BOTTOM_LEFT,
    TOP_CENTER,
    CENTER,
    BOTTOM_CENTER,
    TOP_RIGHT,
    MIDDLE_RIGHT,
    BOTTOM_RIGHT
};

class DrawUtils {
public:
    // Helper: Returns a new rect shifted by the alignment
    static SDL_Rect GetAlignedRect(int x, int y, int w, int h, Align align) {
        SDL_Rect r = {x, y, w, h};

        // Horizontal Adjustment
        switch (align) {
            case Align::TOP_CENTER:
            case Align::CENTER:
            case Align::BOTTOM_CENTER:
                r.x -= w / 2;
                break;
            case Align::TOP_RIGHT:
            case Align::MIDDLE_RIGHT:
            case Align::BOTTOM_RIGHT:
                r.x -= w;
                break;
            default: break; // Left (default)
        }

        // Vertical Adjustment
        switch (align) {
            case Align::MIDDLE_LEFT:
            case Align::CENTER:
            case Align::MIDDLE_RIGHT:
                r.y -= h / 2;
                break;
            case Align::BOTTOM_LEFT:
            case Align::BOTTOM_CENTER:
            case Align::BOTTOM_RIGHT:
                r.y -= h;
                break;
            default: break; // Top (default)
        }

        return r;
    }

    // Wrapper to draw standard rect
    static void DrawRect(SDL_Renderer* renderer, int x, int y, int w, int h, Align align) {
        SDL_Rect r = GetAlignedRect(x, y, w, h, align);
        SDL_RenderDrawRect(renderer, &r);
    }

    // Wrapper for Filled Rect
    static void FillRect(SDL_Renderer* renderer, int x, int y, int w, int h, Align align) {
        SDL_Rect r = GetAlignedRect(x, y, w, h, align);
        SDL_RenderFillRect(renderer, &r);
    }
};