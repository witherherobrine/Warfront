

# include <stdio.h>
#include "raylib.h"
#include "rcamera.h"

#define MAX_COLUMNS 5



int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();
	printf("%i",screenHeight);
    InitWindow(screenWidth, screenHeight, "Warfront");
	printf("%i",screenHeight);
	// ToggleFullscreen();
    // Define the camera to look into our 3d world (position, target, up vector)
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 2.0f, 4.0f };    // Camera position
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 60.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    int cameraMode = CAMERA_FIRST_PERSON;

    // Generates some random columns
    float heights[MAX_COLUMNS] = { 0 };
    Vector3 positions[MAX_COLUMNS] = { 0 };
    Color colors[MAX_COLUMNS] = { 0 };

    for (int i = 0; i < MAX_COLUMNS; i++)
    {
        heights[i] = 2.5f;
        positions[i] = (Vector3){ (float)GetRandomValue(-15, 15), heights[i]/2.0f, (float)GetRandomValue(-15, 15) };
        colors[i] = (Color){200,200,200,255};
    }

    DisableCursor();                    // Limit cursor to relative movement inside the window

    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        // Switch camera mode
        if (IsKeyPressed(KEY_ONE))
        {
            cameraMode = CAMERA_FREE;
            camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; // Reset roll
        }

        if (IsKeyPressed(KEY_TWO))
        {
            cameraMode = CAMERA_FIRST_PERSON;
            camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; // Reset roll
        }

        if (IsKeyPressed(KEY_THREE))
        {
            cameraMode = CAMERA_THIRD_PERSON;
            camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; // Reset roll
        }

        if (IsKeyPressed(KEY_FOUR))
        {
            cameraMode = CAMERA_ORBITAL;
            camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; // Reset roll
        }

        UpdateCamera(&camera, cameraMode);                  // Update camera

		BeginDrawing();

		ClearBackground(SKYBLUE);

		BeginMode3D(camera);

		DrawPlane((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector2){ 128.0f, 128.0f }, DARKGREEN); // Draw ground


		positions[0].x+=.001f;
		// Draw some cubes around
		for (int i = 0; i < MAX_COLUMNS; i++)
		{
			DrawCube(positions[i], 0.5f, heights[i], 0.5f, colors[i]);
		}

		// Draw player cube
		if (cameraMode == CAMERA_THIRD_PERSON)
		{
			DrawCube(camera.target, 0.5f, 0.5f, 0.5f, PURPLE);
			DrawCubeWires(camera.target, 0.5f, 0.5f, 0.5f, DARKPURPLE);
		}
		DrawLine3D((Vector3){0,0,0},positions[0], BLACK);
		EndMode3D();


		DrawRectangle(GetScreenWidth()/2,GetScreenHeight()/2,5,5, BLACK);

		// Draw info boxes
		DrawRectangle(5, 5, 330, 100, Fade(SKYBLUE, 0.5f));
		DrawRectangleLines(5, 5, 330, 100, BLUE);

		DrawText("Camera controls:", 15, 15, 10, BLACK);
		DrawText("- Move keys: W, A, S, D, Space, Left-Ctrl", 15, 30, 10, BLACK);
		DrawText("- Look around: arrow keys or mouse", 15, 45, 10, BLACK);
		DrawText("- Camera mode keys: 1, 2, 3, 4", 15, 60, 10, BLACK);
		DrawText("- Zoom keys: num-plus, num-minus or mouse scroll", 15, 75, 10, BLACK);
		DrawText("- Camera projection key: P", 15, 90, 10, BLACK);

		DrawRectangle(600, 5, 195, 100, Fade(SKYBLUE, 0.5f));
		DrawRectangleLines(600, 5, 195, 100, BLUE);

		DrawText("Camera status:", 610, 15, 10, BLACK);
		DrawText(TextFormat("- Mode: %s", (cameraMode == CAMERA_FREE) ? "FREE" :
										  (cameraMode == CAMERA_FIRST_PERSON) ? "FIRST_PERSON" :
										  (cameraMode == CAMERA_THIRD_PERSON) ? "THIRD_PERSON" :
										  (cameraMode == CAMERA_ORBITAL) ? "ORBITAL" : "CUSTOM"), 610, 30, 10, BLACK);
		DrawText(TextFormat("- Projection: %s", (camera.projection == CAMERA_PERSPECTIVE) ? "PERSPECTIVE" :
												(camera.projection == CAMERA_ORTHOGRAPHIC) ? "ORTHOGRAPHIC" : "CUSTOM"), 610, 45, 10, BLACK);
		DrawText(TextFormat("- Position: (%06.3f, %06.3f, %06.3f)", camera.position.x, camera.position.y, camera.position.z), 610, 60, 10, BLACK);
		DrawText(TextFormat("- Target: (%06.3f, %06.3f, %06.3f)", camera.target.x, camera.target.y, camera.target.z), 610, 75, 10, BLACK);
		DrawText(TextFormat("- Up: (%06.3f, %06.3f, %06.3f)", camera.up.x, camera.up.y, camera.up.z), 610, 90, 10, BLACK);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}