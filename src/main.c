#include <stdio.h>

#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include "raylib-nuklear.h"

#define RAYGIZMO_IMPLEMENTATION
#include "raygizmo.h"

typedef struct TinyPanel
{
    Vector3 position;
    Vector3 rotation;
    Vector3 size;
} TinyPanel;

void tiny_panel_update(TinyPanel *panel, Matrix transform)
{
    panel->position = (Vector3){transform.m12, transform.m13, transform.m14};

    panel->rotation = (Vector3){atan2(transform.m6, transform.m10),
                                atan2(-transform.m2, sqrt(transform.m6 * transform.m6 + transform.m10 * transform.m10)),
                                atan2(transform.m1, transform.m0)};
}

Matrix tiny_get_transform(TinyPanel panel, Vector3 prev_rotation, Vector3 position)
{
        if ((panel.rotation.x - prev_rotation.x) != 0.0f)
        {
            return MatrixMultiply(
                MatrixMultiply(
                    MatrixTranslate(-position.x, -position.y, -position.z),
                    MatrixRotate(X_AXIS, panel.rotation.x - prev_rotation.x)),
                MatrixTranslate(position.x, position.y, position.z));
        }
        else if ((panel.rotation.y - prev_rotation.y) != 0.0f)
        {
            return MatrixMultiply(
                MatrixMultiply(
                    MatrixTranslate(-position.x, -position.y, -position.z),
                    MatrixRotate(Y_AXIS, panel.rotation.y - prev_rotation.y)),
                MatrixTranslate(position.x, position.y, position.z));
        }
        else if ((panel.rotation.z - prev_rotation.z) != 0.0f)
        {
            return MatrixMultiply(
                MatrixMultiply(
                    MatrixTranslate(-position.x, -position.y, -position.z),
                    MatrixRotate(Z_AXIS, panel.rotation.z - prev_rotation.z)),
                MatrixTranslate(position.x, position.y, position.z));
        }

    return MatrixIdentity();
}

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

    Vector3 defaultModelPosition = {0.0f, 1.0f, 0.0f};
    Vector3 defaultModelRotation = {0.0f, 0.0f, 0.0f};
    Vector3 defaultModelSize = {1.0f, 1.0f, 1.0f};

    Model model = LoadModelFromMesh(GenMeshCube(defaultModelSize.x, defaultModelSize.y, defaultModelSize.z));
    RGizmo gizmo = rgizmo_create();

    Vector3 position = {0};

    SetTargetFPS(60);

    /********************************/
    /*                              */
    /* TINY VARIABLE DEFINITIONS    */
    /*                              */
    /********************************/
    TinyPanel panel = {.position = defaultModelPosition, .rotation = defaultModelRotation, .size = defaultModelSize};

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

        tiny_panel_update(&panel, model.transform);
        Vector3 prev_rotation = panel.rotation;

        if (nk_begin(ctx, "Panel", nk_rect(10, 10, 220, 440),
                     NK_WINDOW_BORDER | NK_WINDOW_CLOSABLE))
        {
            if (nk_tree_push(ctx, NK_TREE_NODE, "Transform", NK_MINIMIZED))
            {
                nk_layout_row_dynamic(ctx, 20, 1);
                nk_label(ctx, "Location", NK_TEXT_LEFT);
                nk_property_float(ctx, "#X:", minimum_float, &panel.position.x, maximum_float, 1.0f, 1.0f);
                nk_property_float(ctx, "#Y:", minimum_float, &panel.position.y, maximum_float, 1.0f, 1.0f);
                nk_property_float(ctx, "#Z:", minimum_float, &panel.position.z, maximum_float, 1.0f, 1.0f);
                nk_label(ctx, "Rotation", NK_TEXT_LEFT);
                nk_property_float(ctx, "#X:", -3.14f, &panel.rotation.x, 3.14f, 0.1f, 0.1f);
                nk_property_float(ctx, "#Y:", -3.14f, &panel.rotation.y, 3.14f, 0.1f, 0.1f);
                nk_property_float(ctx, "#Z:", -3.14f, &panel.rotation.z, 3.14f, 0.1f, 0.1f);
                nk_label(ctx, "Scale", NK_TEXT_LEFT);
                nk_property_float(ctx, "#X:", minimum_float, &panel.size.x, maximum_float, 1.0f, 1.0f);
                nk_property_float(ctx, "#Y:", minimum_float, &panel.size.y, maximum_float, 1.0f, 1.0f);
                nk_property_float(ctx, "#Z:", minimum_float, &panel.size.z, maximum_float, 1.0f, 1.0f);

                nk_tree_pop(ctx);
            }
        }
        nk_end(ctx);

        model.transform.m12 = panel.position.x;
        model.transform.m13 = panel.position.y;
        model.transform.m14 = panel.position.z;

        position = (Vector3){model.transform.m12, model.transform.m13, model.transform.m14};
        model.transform = MatrixMultiply(model.transform, tiny_get_transform(panel, prev_rotation, position));

        position = (Vector3){model.transform.m12, model.transform.m13, model.transform.m14};
        rgizmo_update(&gizmo, camera, position);
        model.transform = MatrixMultiply(model.transform, rgizmo_get_tranform(gizmo, position));

        /* Render */
        BeginDrawing();
        ClearBackground(DARKGRAY);
        rlEnableDepthTest();

        BeginMode3D(camera);

        DrawModelEx(model, defaultModelPosition, defaultModelRotation, 0.0f, defaultModelSize, GRAY);

        DrawGrid(100, 1.0f);

        rgizmo_draw(gizmo, camera, position);

        EndMode3D();

        /* Render the Nuklear GUI */
        DrawNuklear(ctx);

        EndDrawing();
    }

    /* De-initialize the Nuklear GUI */
    UnloadNuklear(ctx);

    /* De-initialize Raylib stuff */
    rgizmo_unload();
    UnloadModel(model);

    CloseWindow();
    return 0;
}