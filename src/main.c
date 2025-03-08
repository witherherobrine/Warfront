

# include <stdio.h>
#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"

#include "entity.h"
#include "pathmanager.h"
#include "bullet.h"
#include "gamemaster.h"






int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();
	printf("%i",screenHeight);
    InitWindow(screenWidth, screenHeight, "Warfront");
    // InitWindow(720, 540, "Warfront");
	// ToggleFullscreen();
    // Define the camera to look into our 3d world (position, target, up vector)
    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 4.0f, 4.01f };    // Camera position
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 60.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera projection type

    int cameraMode = CAMERA_FIRST_PERSON;
	
    DisableCursor();                    // Limit cursor to relative movement inside the window
    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------


	int entityCount = 10;
    Entity entities[entityCount];

	extern int bulletCacheAmount;
	extern Bullet bullets[];
	extern int bulletFireIndex;
	
	extern BoundingBox testaabb;

	// Model model = 
	// Model model = LoadModel("../res/models/JasperCarmack_Community_SRC.iqm");
	// Texture2D texture = LoadTexture("../res/models/T_Jasper_Masc_BaseColor.png"); // Load model texture
    // model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;  
	
	for(unsigned int i = 0; i < entityCount; i++){
		entities[i] = entity_createEntity();
		entities[i].id = i;
		if(i == entityCount-1){
			entities[i].player = true;
		}
	}
	
	gamemaster_setupNodes(entities, entityCount);
	
	// entities[2].position = (Vector3){2,0,0};
	bullet_initBulletCache();
	
	bool moving = false;
	
	Image groundMap  = LoadImage("../res/images/groundMap.png");
	Mesh ground = GenMeshHeightmap(groundMap, (Vector3){100.0f,1.0f,100.0f});
	
    Material groundMat =  LoadMaterialDefault();  
	Texture2D texture = LoadTexture("../res/textures/grass.png"); // Load model texture
	SetTextureWrap(texture, TEXTURE_WRAP_REPEAT);
	groundMat.maps[0].texture = texture;
	
	InitAudioDevice();
	Wave gunFile = LoadWave("../res/audio/shot.wav");
	Sound  gunSound = LoadSoundFromWave(gunFile);
	
	
	int moveFrame = 0;
	int shootPlayerFrame = 0;
	
	
	
	Model gon = LoadModel("../res/models/gon.m3d");
	
    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        if (IsKeyPressed(KEY_ONE))
        {
            cameraMode = CAMERA_FIRST_PERSON;
            camera.up = (Vector3){ 0.0f, 1.0f, 0.0f }; // Reset roll
        }
		moving = false;
	   if (IsKeyDown(KEY_W) || 
		   IsKeyDown(KEY_A) ||
		   IsKeyDown(KEY_S) ||
		   IsKeyDown(KEY_D)){
		   
		   moving = true;   
	   }
	   if(IsKeyPressed(KEY_ONE)){
		   gamemaster_moveTest(entities, 0, entities[entityCount-1].position);
	   }
	   if(IsKeyPressed(KEY_TWO)){
		   gamemaster_moveTest(entities, 1, entities[entityCount-1].position);
	   }
	   
	   moveFrame++;
	   if(moveFrame > 30){
		   moveFrame = 0;
		   float rotate = entities[0].rotation + 90.0f;
			// bullet_fireBullet(entities[0].position, (Vector3){sin(rotate*(M_PI/180)),0,cos(rotate*(M_PI/180))},1.0f,0);
			
			
			Vector3 qVec = (Vector3){(float)GetRandomValue(-50, 50),10.0f,(float)GetRandomValue(-50, 50)};
			entity_queryBox(&entities[0],qVec);
		   // path_manager_moveGroup(entities, 5);
	   }
	   
        UpdateCamera(&camera, cameraMode);                  // Update camera
		BeginDrawing();
		ClearBackground(SKYBLUE);
		BeginMode3D(camera);
		DrawPlane((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector2){ 128.0f, 128.0f }, DARKGREEN); // Draw ground
		
		// entities[2].model.bindPose[6].rotation
		// DrawLine3D((Vector3){0,0,0}, entities[0].anims[entities[0].animIndex].framePoses[entities[0].animFrame][6].translation, RED);
		entities[entityCount-1].position = Vector3Add(camera.position,(Vector3){0,-1.3,0});
		
		// Draw player cube
		if (cameraMode == CAMERA_THIRD_PERSON)
		{
			DrawCube(camera.target, 0.5f, 0.5f, 0.5f, PURPLE);
			// DrawCubeWires(camera.target, 0.5f, 0.5f, 0.5f, DARKPURPLE);
		}
		
		
		// float yaw = atan2f(forwardXZ.x, -forwardXZ.z);
		// yaw = radiansToDegrees(yaw);
		
		Vector3 ang = GetCameraForward(&camera);
		entities[entityCount-1].rotation = atan2f(ang.x,ang.z) - M_PI/2;
		
		for (unsigned int i = 0; i < entityCount; i++){
			entity_updateEntity(&entities[i]);
				
			for (unsigned int j = 0; j < bulletCacheAmount; j++){
				if(!bullets[j].enabled){
					continue;
				}
				
				bullet_updateBullet(&bullets[j]);
				RayCollision hit = GetRayCollisionBox(bullets[j].ray, entities[i].aabb);
			
				if(entities[i].id != bullets[j].id && entities[i].alive && hit.hit && hit.distance <= bullets[j].velocity){
					entities[i].alive = false;
					entities[i].animFrame = 0;
					// bullets[j].enabled = false;
					// bullets[j].velocity = 0.0f;
				}
			}			
			if(CheckCollisionBoxes(testaabb, entities[i].aabb) && entities[i].alive){
				entity_setEntityTarget(&entities[0], entities[i].position);
			}
			
			DrawModelWiresEx(entities[i].model, (Vector3){0,0,0}, (Vector3){0,1.0f,0},0,(Vector3){1.0,1.0f,1.0f},WHITE);
			// DrawModelEx(entities[i].model, (Vector3){0,0,0}, (Vector3){0,1.0f,0},0,(Vector3){1.0,1.0f,1.0f},WHITE);
		}
		
		for (unsigned int i = 0; i < bulletCacheAmount; i++){
			DrawLine3D(bullets[i].ray.position, Vector3Add(bullets[i].ray.position, Vector3Scale(bullets[i].ray.direction, bullets[i].velocity)), YELLOW);
			bullets[i].ray.position = Vector3Add(bullets[i].ray.position,Vector3Add(Vector3Scale(bullets[i].ray.direction,bullets[i].velocity),(Vector3){0,-.001,0}));
			// DrawRay(bulletCache[i].ray, RED);
		}
		//DrawMesh(ground,groundMat,MatrixIdentity());   
		if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
			shootPlayerFrame++;
			if(shootPlayerFrame >5){
				shootPlayerFrame = 0;
				PlaySound(gunSound);  
				Vector3 pos = entities[entityCount-1].anims[entities[entityCount-1].animIndex].framePoses[entities[entityCount-1].animFrame][8].translation;
				pos = Vector3Transform(pos,entities[entityCount-1].model.transform);
				// pos = Vector3Add(pos, entities[entityCount-1].position);
				
				printf("pos: {%f, %f, %f}\n",pos.x, pos.y, pos.z);
				
				bullet_fireBullet(pos, GetCameraForward(&camera), 10.0f, entities[entityCount-1].id);	
			}
		}
			DrawBoundingBox(testaabb, RED);
			
			
			          // Transform *transform = &anim.framePoses[animCurrentFrame][boneSocketIndex[i]];
                    // Quaternion inRotation = characterModel.bindPose[boneSocketIndex[i]].rotation;
                    // Quaternion outRotation = transform->rotation;
                    // Quaternion rotate = QuaternionMultiply(outRotation, QuaternionInvert(inRotation));
                    // Matrix matrixTransform = QuaternionToMatrix(rotate);
                    // matrixTransform = MatrixMultiply(matrixTransform, MatrixTranslate(transform->translation.x, transform->translation.y, transform->translation.z));
                    // matrixTransform = MatrixMultiply(matrixTransform, characterModel.transform);
                    // DrawMesh(equipModel[i].meshes[0], equipModel[i].materials[1], matrixTransform);
			
			
			int entIndex = entityCount-1;
			
			Transform transform = entities[entIndex].anims[entities[entIndex].animIndex].framePoses[entities[entIndex].animFrame][8];
			// transform.translation = Vector3Multiply(transform.translation, entities[2].position);
			Quaternion inRotation = entities[entIndex].model.bindPose[6].rotation;
			Quaternion outRotation = transform.rotation;
			Quaternion rotate = QuaternionMultiply(outRotation, QuaternionInvert(inRotation));
			// Matrix matrixTransform = QuaternionToMatrix(outRotation);
			Matrix matrixTransform = MatrixRotate((Vector3){0,1,0},(float)(-90.f * (M_PI/180)));
			matrixTransform = MatrixMultiply(matrixTransform, MatrixTranslate(transform.translation.x, transform.translation.y, transform.translation.z));
			matrixTransform = MatrixMultiply(matrixTransform, entities[entIndex].model.transform);
			
			// DrawModel(gon, (Vector3){0,0,0}, 1.0f, WHITE);
			DrawMesh(gon.meshes[0], gon.materials[1], matrixTransform);
			
			// Vector3 gunpos = Vector3Add(entities[2].anims[entities[2].animIndex].framePoses[entities[2].animFrame][6].translation, entities[2].position);
			// DrawModelEx(gon, gunpos, (Vector3){0,1.0f,0},-90.0f,(Vector3){1.0,1.0f,1.0f},WHITE);
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

		
		DrawFPS(1000, 10); 

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}