#include <stdio.h>

#define RAYLIB_NUKLEAR_IMPLEMENTATION
#include "raylib-nuklear.h"

#include "rcamera.h"

#define RAYGIZMO_IMPLEMENTATION
#include "raygizmo.h"

typedef struct TinyPanel
{
    Vector3 translation;
    Vector3 rotation;
    Vector3 scale;
    struct
    {
        Vector3 translation;
        Vector3 rotation;
        Vector3 scale;
    } update;
} TinyPanel;

typedef struct TinyModel
{
    Model model;
} TinyModel;

void tiny_panel_update(struct nk_context *ctx, TinyPanel *panel, Matrix transform)
{
    const float linear_limit = 99999.9f;
    const float angle_limit = 180.0f;

    Vector3 prev_scale = (Vector3){
        Vector3Length((Vector3){transform.m0, transform.m1, transform.m2}),
        Vector3Length((Vector3){transform.m4, transform.m5, transform.m6}),
        Vector3Length((Vector3){transform.m8, transform.m9, transform.m10})};

    Matrix result = MatrixIdentity();

    result.m0 = transform.m0 / prev_scale.x;
    result.m1 = transform.m1 / prev_scale.x;
    result.m2 = transform.m2 / prev_scale.x;

    result.m4 = transform.m4 / prev_scale.y;
    result.m5 = transform.m5 / prev_scale.y;
    result.m6 = transform.m6 / prev_scale.y;

    result.m8 = transform.m8 / prev_scale.z;
    result.m9 = transform.m9 / prev_scale.z;
    result.m10 = transform.m10 / prev_scale.z;

    Quaternion q = QuaternionFromMatrix(result);
    Vector3 prev_rotation = QuaternionToEuler(q);

    Vector3 prev_translation = (Vector3){transform.m12, transform.m13, transform.m14};

    panel->scale = prev_scale;
    panel->rotation = prev_rotation;
    panel->translation = prev_translation;

    if (nk_tree_push(ctx, NK_TREE_NODE, "Transform", NK_MINIMIZED))
    {
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_label(ctx, "Location", NK_TEXT_LEFT);
        nk_property_float(ctx, "#X:", -linear_limit, &panel->translation.x, linear_limit, 1.0f, 1.0f);
        nk_property_float(ctx, "#Y:", -linear_limit, &panel->translation.y, linear_limit, 1.0f, 1.0f);
        nk_property_float(ctx, "#Z:", -linear_limit, &panel->translation.z, linear_limit, 1.0f, 1.0f);
        nk_label(ctx, "Rotation", NK_TEXT_LEFT);
        nk_property_float(ctx, "#X:", -angle_limit, &panel->rotation.x, angle_limit, 1.0f, 1.0f);
        nk_property_float(ctx, "#Y:", -angle_limit, &panel->rotation.y, angle_limit, 1.0f, 1.0f);
        nk_property_float(ctx, "#Z:", -angle_limit, &panel->rotation.z, angle_limit, 1.0f, 1.0f);
        nk_label(ctx, "Scale", NK_TEXT_LEFT);
        nk_property_float(ctx, "#X:", -linear_limit, &panel->scale.x, linear_limit, 1.0f, 1.0f);
        nk_property_float(ctx, "#Y:", -linear_limit, &panel->scale.y, linear_limit, 1.0f, 1.0f);
        nk_property_float(ctx, "#Z:", -linear_limit, &panel->scale.z, linear_limit, 1.0f, 1.0f);

        nk_tree_pop(ctx);
    }

    panel->update.scale = Vector3Subtract(panel->scale, prev_scale);
    panel->update.rotation = Vector3Subtract(panel->rotation, prev_rotation);
    panel->update.translation = Vector3Subtract(panel->translation, prev_translation);
}

Matrix tiny_get_transform(TinyPanel panel, Matrix modelTransform)
{
    Matrix matScale = MatrixIdentity();
    Matrix matRotation = MatrixIdentity();
    Matrix matTranslation = MatrixIdentity();
    Matrix matTransform = MatrixIdentity();

    if (!Vector3Equals(panel.update.scale, (Vector3){0.0, 0.0, 0.0}) ||
        !Vector3Equals(panel.update.rotation, (Vector3){0.0, 0.0, 0.0}) ||
        !Vector3Equals(panel.update.translation, (Vector3){0.0, 0.0, 0.0}))
    {
        matScale = MatrixScale(panel.scale.x, panel.scale.y, panel.scale.z);
        matRotation = MatrixRotateXYZ(panel.rotation);
        matTranslation = MatrixTranslate(panel.translation.x,
                                         panel.translation.y,
                                         panel.translation.z);

        matTransform = MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);

        return matTransform;
    }
    else
        return modelTransform;
}

int main(int argc, char *argv[])
{
    int screen_width = 800;
    int screen_height = 600;

    const unsigned int raylib_flags = FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT;

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

    Vector3 defaultModelPosition = {0.0f, 0.0f, 0.0f};
    Vector3 defaultModelRotation = {0.0f, 0.0f, 0.0f};
    Vector3 defaultModelSize = {1.0f, 1.0f, 1.0f};

    const int MAX_MODELS = 100;
    TinyModel models[MAX_MODELS];
    models[0].model = LoadModelFromMesh(GenMeshCube(defaultModelSize.x, defaultModelSize.y, defaultModelSize.z));
    int SELECTED_MODEL = 0;
    int MODELS = 1;

    const int MAX_TEXTURES = 100;
    Texture2D textures[MAX_TEXTURES];
    textures[0] = LoadTexture("../resources/grass.png");
    textures[1] = LoadTexture("../resources/lava.png");
    textures[2] = LoadTexture("../resources/brick.png");
    textures[3] = LoadTexture("../resources/wood.png");
    textures[4] = LoadTexture("../resources/stone.png");
    int SELECTED_TEXTURE = 0;
    int TEXTURES = 5;

    RGizmo gizmo = rgizmo_create();

    Vector3 position = {0};

    Ray ray = {0};

    float sensitivity = 0.01f;

    SetTargetFPS(60);

    /********************************/
    /*                              */
    /* TINY VARIABLE DEFINITIONS    */
    /*                              */
    /********************************/
    TinyPanel panel = {.translation = defaultModelPosition, .scale = defaultModelSize, .rotation = defaultModelRotation, .update = {0}};
    nk_bool selected[MAX_TEXTURES];

    /********************************/
    /*                              */
    /* NUKLEAR VARIABLE DEFINITIONS */
    /*                              */
    /********************************/
    /* Create the Nuklear Context */
    int fontSize = 10;
    struct nk_context *ctx = InitNuklear(fontSize);

    while (!WindowShouldClose())
    {
        Vector2 mouseDelta = GetMouseDelta();
        float mouseWheelMove = GetMouseWheelMove();

        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && IsKeyDown(KEY_SPACE))
        {
            Matrix rotation_horizontal = MatrixRotate(GetCameraUp(&camera), sensitivity * mouseDelta.x);
            Matrix rotation_vertical = MatrixRotate(GetCameraRight(&camera), sensitivity * mouseDelta.y);
            Matrix rotation = MatrixMultiply(rotation_horizontal, rotation_vertical);
            Vector3 view = Vector3Subtract(camera.position, camera.target);
            view = Vector3Transform(view, rotation);
            camera.position = Vector3Add(camera.target, view);
        }

        camera.position = Vector3Add(camera.position, Vector3Scale(Vector3Subtract(camera.position, camera.target), -mouseWheelMove * 2.0f * sensitivity));

        /* Update the Nuklear context, along with input */
        UpdateNuklear(ctx);

        if (nk_begin(ctx, "Panel", nk_rect(10, 10, 220, 440),
                     NK_WINDOW_BORDER | NK_WINDOW_CLOSABLE))
        {
            nk_layout_row_dynamic(ctx, 10, 1);

            position = (Vector3){models[SELECTED_MODEL].model.transform.m12, models[SELECTED_MODEL].model.transform.m13, models[SELECTED_MODEL].model.transform.m14};
            tiny_panel_update(ctx, &panel, models[SELECTED_MODEL].model.transform);
            models[SELECTED_MODEL].model.transform = tiny_get_transform(panel, models[SELECTED_MODEL].model.transform);

            if (nk_button_label(ctx, "Add Cube"))
            {
                Mesh mesh = GenMeshCube(defaultModelSize.x, defaultModelSize.x, defaultModelSize.x);
                Model model = LoadModelFromMesh(mesh);
                models[MODELS].model = model;
                MODELS++;
                SELECTED_MODEL = MODELS - 1;
            }
            if (nk_button_label(ctx, "Add Sphere"))
            {
                Mesh mesh = GenMeshSphere(1.0, 9.0, 9.0);
                Model model = LoadModelFromMesh(mesh);
                models[MODELS].model = model;
                MODELS++;
                SELECTED_MODEL = MODELS - 1;
            }
            if (nk_button_label(ctx, "Add Cone"))
            {
                Mesh mesh = GenMeshCone(1.0, 1.0, 9.0);
                Model model = LoadModelFromMesh(mesh);
                models[MODELS].model = model;
                MODELS++;
                SELECTED_MODEL = MODELS - 1;
            }
            if (nk_button_label(ctx, "Add Cylinder"))
            {
                Mesh mesh = GenMeshCylinder(1.0, 1.0, 9.0);
                Model model = LoadModelFromMesh(mesh);
                models[MODELS].model = model;
                MODELS++;
                SELECTED_MODEL = MODELS - 1;
            }

            if (nk_tree_push(ctx, NK_TREE_NODE, "Textures", NK_MINIMIZED))
            {
                nk_layout_row_static(ctx, 36, 100, 1);
                for (int i = 0; i < TEXTURES; i++)
                {
                    if (nk_selectable_image_label(ctx, TextureToNuklear(textures[i]), " ", NK_TEXT_RIGHT, &selected[i]))
                    {
                        SELECTED_TEXTURE = i;
                        models[SELECTED_MODEL].model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = textures[SELECTED_TEXTURE];
                    }
                }
                nk_tree_pop(ctx);
            }
        }
        nk_end(ctx);

        position = (Vector3){models[SELECTED_MODEL].model.transform.m12, models[SELECTED_MODEL].model.transform.m13, models[SELECTED_MODEL].model.transform.m14};
        rgizmo_update(&gizmo, camera, position);
        models[SELECTED_MODEL].model.transform = MatrixMultiply(models[SELECTED_MODEL].model.transform, rgizmo_get_transform(gizmo, position));

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
        {
            ray = GetScreenToWorldRay(GetMousePosition(), camera);
            for (int i = 0; i < MODELS; i++)
            {
                RayCollision boxHitInfo = GetRayCollisionMesh(ray, models[i].model.meshes[0], models[i].model.transform);

                if (boxHitInfo.hit)
                {
                    SELECTED_MODEL = i;
                }
            }
        }

        /* Render */
        BeginDrawing();
        ClearBackground(DARKGRAY);
        rlEnableDepthTest();

        BeginMode3D(camera);

        for (int i = 0; i < MODELS; i++)
        {
            DrawModelEx(models[i].model, defaultModelPosition, defaultModelRotation, 0.0f, defaultModelSize, GRAY);
        }

        DrawGrid(100, 5.0f);

        EndMode3D();

        rgizmo_draw(gizmo, camera, position);

        /* Render the Nuklear GUI */
        DrawNuklear(ctx);

        EndDrawing();
    }

    /* De-initialize the Nuklear GUI */
    UnloadNuklear(ctx);

    /* De-initialize Raylib stuff */
    rgizmo_unload();
    for (int i = 0; i < MODELS; i++)
        UnloadModel(models[i].model);

    CloseWindow();
    return 0;
}