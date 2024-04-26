#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include "raylib-nuklear.h"

int main(int argc, char *argv[])
{
    InitWindow(640, 480, "TinyEditor");

    // Create the Nuklear Context
    int fontSize = 10;
    struct nk_context *ctx = InitNuklear(fontSize);

    while (!WindowShouldClose())
    {
        // Update the Nuklear context, along with input
        UpdateNuklear(ctx);

        if (nk_begin(ctx, "Panel", nk_rect(100, 100, 220, 220),
                     NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_CLOSABLE))
        {
            nk_layout_row_static(ctx, 50, 150, 1);
            if (nk_button_label(ctx, "Button"))
            {
                // Button was clicked!
            }
        }
        nk_end(ctx);

        // Render
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Render the Nuklear GUI
        DrawNuklear(ctx);

        EndDrawing();
    }

    // De-initialize the Nuklear GUI
    UnloadNuklear(ctx);

    CloseWindow();
    return 0;
}