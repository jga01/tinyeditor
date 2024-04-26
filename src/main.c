#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include "raylib-nuklear.h"

int main(int argc, char *argv[])
{
    int screen_width = 640;
    int screen_height = 480;

    const unsigned int raylib_flags = FLAG_WINDOW_RESIZABLE;

    SetConfigFlags(raylib_flags);
    InitWindow(screen_width, screen_height, "TinyEditor");

    /********************************/
    /*                              */
    /* RAYLIB VARIABLE DEFINITIONS  */
    /*                              */
    /********************************/
    Camera camera = {0};
    camera.position = (Vector3){10.0f, 10.0f, 10.0f};
    camera.target = (Vector3){0.0f, 0.0f, 0.0f};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    Vector3 cubePosition = {0.0f, 1.0f, 0.0f};
    Vector3 cubeSize = {2.0f, 2.0f, 2.0f};

    SetTargetFPS(60);

    /********************************/
    /*                              */
    /* NUKLEAR VARIABLE DEFINITIONS */
    /*                              */
    /********************************/
    /* Create the Nuklear Context */
    int fontSize = 10;
    struct nk_context *ctx = InitNuklear(fontSize);

    const float minimum_float = -10.0f;
    const float maximum_float = 10.0f;

    while (!WindowShouldClose())
    {
        /* Update the Nuklear context, along with input */
        UpdateNuklear(ctx);

        if (nk_begin(ctx, "Panel", nk_rect(10, 10, 220, 440),
                     NK_WINDOW_BORDER | NK_WINDOW_CLOSABLE))
        {
            if (nk_tree_push(ctx, NK_TREE_NODE, "Transform", NK_MINIMIZED))
            {
                nk_layout_row_dynamic(ctx, 20, 1);
                nk_label(ctx, "Location", NK_TEXT_LEFT);
                nk_property_float(ctx, "#X:", minimum_float, &cubePosition.x, maximum_float, 1.0f, 1.0f);
                nk_property_float(ctx, "#Y:", minimum_float, &cubePosition.y, maximum_float, 1.0f, 1.0f);
                nk_property_float(ctx, "#Z:", minimum_float, &cubePosition.z, maximum_float, 1.0f, 1.0f);
                nk_label(ctx, "Scale", NK_TEXT_LEFT);
                nk_property_float(ctx, "#X:", minimum_float, &cubeSize.x, maximum_float, 1.0f, 1.0f);
                nk_property_float(ctx, "#Y:", minimum_float, &cubeSize.y, maximum_float, 1.0f, 1.0f);
                nk_property_float(ctx, "#Z:", minimum_float, &cubeSize.z, maximum_float, 1.0f, 1.0f);

                nk_tree_pop(ctx);
            }
        }
        nk_end(ctx);

        /* Render */
        BeginDrawing();
        ClearBackground(RAYWHITE);

        BeginMode3D(camera);

        DrawCube(cubePosition, cubeSize.x, cubeSize.y, cubeSize.z, GRAY);
        DrawCubeWires(cubePosition, cubeSize.x, cubeSize.y, cubeSize.z, DARKGRAY);

        DrawGrid(10, 1.0f);

        EndMode3D();

        /* Render the Nuklear GUI */
        DrawNuklear(ctx);

        EndDrawing();
    }

    // De-initialize the Nuklear GUI
    UnloadNuklear(ctx);

    CloseWindow();
    return 0;
}