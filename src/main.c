

# include <stdio.h>
#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"

#include "entity.h"
#include "pathmanager.h"
#include "bullet.h"
#include "gamemaster.h"
#include "rllight.h"
#include "terrain.h"
#include "worldobject.h"



extern int bulletCacheAmount;
extern Bullet* bullets[];
extern BoundingBox testaabb;

Color* pixels;
int MAP_WIDTH;
int MAP_HEIGHT;



int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();
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


	// Model model = 
	// Model model = LoadModel("../res/models/JasperCarmack_Community_SRC.iqm");
	// Texture2D texture = LoadTexture("../res/models/T_Jasper_Masc_BaseColor.png"); // Load model texture
    // model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;  

	
	for(unsigned int i =0; i < 10; i++){
		worldobject_generateEntity(i==0, (Vector3){(float)GetRandomValue(-15, 15)/0.5f,2.3f,(float)GetRandomValue(-15, 15)/0.5f});
	}
	int currentArrSize = worldObjects.count;
	for(unsigned int i =0; i < bulletCacheAmount; i++){
		worldobject_generateBullet();
		WorldObject* wo = worldObjects.data[currentArrSize + i];
		Bullet *genBullet = (Bullet *)wo->data;
		bullets[i] = genBullet;
	}
	
	// gamemaster_setupNodes(entities, entityCount);
	
	// bullet_initBulletCache();
	
	bool moving = false;
	
	char* GROUND_PATH = "../res/images/groundMap3.jpg";
	
	Mesh ground  = createTerrain(GROUND_PATH, 200, 50);
	
    // Load basic lighting shader
    Shader shader = LoadShader(TextFormat("../res/shaders/lighting.vs"), TextFormat("../res/shaders/lighting.fs"));
	bool valid = IsShaderValid(shader);
	
	if(!valid){
		printf("ruh roh raggy");
		exit(1);
	}
	
    // Get some required shader locations
    shader.locs[SHADER_LOC_VECTOR_VIEW] = GetShaderLocation(shader, "viewPos");
    // NOTE: "matModel" location name is automatically assigned on shader loading, 
    // no need to get the location again if using that uniform name
    shader.locs[SHADER_LOC_MATRIX_MODEL] = GetShaderLocation(shader, "matModel");
    
    // Ambient light level (some basic lighting)

	
    // int ambientLoc = GetShaderLocation(shader, "ambient");
    // SetShaderValue(shader, ambientLoc, (float[4]){ 0,0,0, 1.0f }, SHADER_UNIFORM_VEC4);

    // Light lights[4] = { 0 };
    // lights[0] = CreateLight(LIGHT_POINT, (Vector3){ -2, 5, -2 }, Vector3Zero(), WHITE, shader);
    // lights[1] = CreateLight(LIGHT_POINT, (Vector3){ 2, 5, 2 }, Vector3Zero(), RED, shader);
    // lights[2] = CreateLight(LIGHT_POINT, (Vector3){ -2, 5, 2 }, Vector3Zero(), GREEN, shader);
    // lights[3] = CreateLight(LIGHT_POINT, (Vector3){ 2, 5, -2 }, Vector3Zero(), BLUE, shader);
	
	
	int ambientLoc = GetShaderLocation(shader, "ambientColor");
	int lightDirLoc = GetShaderLocation(shader, "lightDir");
	int lightColorLoc = GetShaderLocation(shader, "lightColor");
	
	Vector3 ambientColor = { 0.1f, 0.1f, 0.1f }; // Example ambient color
	Vector3 lightDir = { 0.707, -0.707, 0.0}; // Example light direction (down)
	Vector3 lightColor = { 2.0f, 2.0f, 2.0f }; // Example light color (white)

	SetShaderValue(shader, ambientLoc, &ambientColor, SHADER_UNIFORM_VEC3);
	SetShaderValue(shader, lightDirLoc, &lightDir, SHADER_UNIFORM_VEC3);
	SetShaderValue(shader, lightColorLoc, &lightColor, SHADER_UNIFORM_VEC3);

	
	for(int i = 0; i < worldObjects.count; i++){
		WorldObject *currentWorldObject = (WorldObject *)worldObjects.data[i];
		if(currentWorldObject->type == OBJECT_ENTITY){
			Entity *entityData = (Entity *)currentWorldObject->data;			
			entity_updateEntityShader(entityData, shader);
			entityData->model.materials[1].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture("../res/textures/brotex.png"); // Set map diffuse texture
		}
	}
	
	
	Model gModel = LoadModelFromMesh(ground);
	
	gModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture("../res/textures/grass.png"); // Set map diffuse texture
	gModel.materials[0].shader = shader;

	
    // Material groundMat =  LoadMaterialDefault();  
	// Texture2D texture = LoadTexture("../res/textures/grass.png"); // Load model texture
	// SetTextureWrap(texture, TEXTURE_WRAP_REPEAT);
	// groundMat.maps[0].texture = texture;
	
	InitAudioDevice();
	Wave gunFile = LoadWave("../res/audio/shot.wav");
	Sound  gunSound = LoadSoundFromWave(gunFile);
	
	
	int moveFrame = 0;
	int shootPlayerFrame = 0;
	
	
	
	Model gon = LoadModel("../res/models/gon.m3d");
	for(unsigned int i = 0; i < gon.materialCount; i++){
		gon.materials[i].shader = shader;
	}
	
	Model DEBUG_CUBE = LoadModel("../res/models/cubie.m3d");
	for(unsigned int i = 0; i < DEBUG_CUBE.materialCount; i++){
		DEBUG_CUBE.materials[i].shader = shader;
	}
	
	
	
	float* vertTest = gModel.meshes[0].vertices;
	
	int vertIndex = 0;
	int vertFrame = 0;
	
	
    Image groundMap = LoadImage(GROUND_PATH);
	MAP_WIDTH = groundMap.width;
	MAP_HEIGHT = groundMap.height;
	pixels = LoadImageColors(groundMap);
	
	int PLAYER_INDEX = 0;
	
    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
		moving = false;
	   if (IsKeyDown(KEY_W) || 
		   IsKeyDown(KEY_A) ||
		   IsKeyDown(KEY_S) ||
		   IsKeyDown(KEY_D)){
		   
		   moving = true;   
	   }
	   if(IsKeyDown(KEY_Q)){
		   camera.position.y+=0.5f;
	   }
	   if(IsKeyDown(KEY_E)){
		   camera.position.y-=0.5f;
	   }
	   // if(IsKeyPressed(KEY_ONE)){
		   // gamemaster_moveTest(entities, 0, entities[entityCount-1].position);
	   // }
	   // if(IsKeyPressed(KEY_TWO)){
		   // gamemaster_moveTest(entities, 1, entities[entityCount-1].position);
	   // }
	   
		float newDist = yPointXZTest((Vector3){200,50,200},camera.position.x, camera.position.z)+2;
	   CameraMoveUp(&camera,newDist - camera.position.y);
	   
	   moveFrame++;
	   if(moveFrame > 30){
		   moveFrame = 0;
			// bullet_fireBullet(entities[0].position, (Vector3){sin(rotate*(M_PI/180)),0,cos(rotate*(M_PI/180))},1.0f,0);
			
			
			Vector3 qVec = (Vector3){(float)GetRandomValue(-50, 50),10.0f,(float)GetRandomValue(-50, 50)};
			// entity_queryBox(&entities[0],qVec);
		   // path_manager_moveGroup(entities, 5);
	   }
        UpdateCamera(&camera, cameraMode);                  // Update camera
		BeginDrawing();
		ClearBackground(LIGHTGRAY);
		BeginMode3D(camera);
		
		
		// BeginShaderMode(shader);

		// DrawPlane((Vector3){ 0.0f, 0.0f, 0.0f }, (Vector2){ 128.0f, 128.0f }, WHITE); // Draw ground
		
		// entities[2].model.bindPose[6].rotation
		// DrawLine3D((Vector3){0,0,0}, entities[0].anims[entities[0].animIndex].framePoses[entities[0].animFrame][6].translation, RED);
		// entities[PLAYER_INDEX].position = Vector3Add(camera.position,(Vector3){0,-1.3,0});
		
		// Vector3 ang = GetCameraForward(&camera);
		// entities[PLAYER_INDEX].rotation = atan2f(ang.x,ang.z) - M_PI/2;
		
		
		Vector3 lStart = (Vector3){0,20,0};
		Vector3 lEnd = (Vector3){0};
		lEnd = (Vector3){vertTest[vertIndex], vertTest[vertIndex+1], vertTest[vertIndex+2]};
		// camera.position = lEnd;
		
		vertFrame++;
		if(vertFrame > 15){
			// printf("(%i,%i,%i)\n",(int)vertTest[vertIndex], (int)vertTest[vertIndex+1], (int)	vertTest[vertIndex+2]);
			vertIndex+=3;
			vertFrame = 0;
		}
		
		
		DrawLine3D(lStart, lEnd, WHITE);
		
		// for (unsigned int i = 0; i < entityCount; i++){
			// entity_updateEntity(&entities[i]);
				
			// for (unsigned int j = 0; j < bulletCacheAmount; j++){
				// if(!bullets[j].enabled){
					// continue;
				// }
				
				// bullet_updateBullet(&bullets[j]);
				// RayCollision hit = GetRayCollisionBox(bullets[j].ray, entities[i].aabb);
			
				// if(entities[i].id != bullets[j].id && entities[i].alive && hit.hit && hit.distance <= bullets[j].velocity){
					// entities[i].alive = false;
					// entities[i].animFrame = 0;
					// bullets[j].enabled = false;
					// bullets[j].velocity = 0.0f;
				// }
			// }			
			// if(CheckCollisionBoxes(testaabb, entities[i].aabb) && entities[i].alive){
				// entity_setEntityTarget(&entities[0], entities[i].position);
			// }
		// }
		for(int i = 0; i < worldObjects.count; i++){
			
			WorldObject *currentWorldObject = (WorldObject *)worldObjects.data[i];
			
			// DrawBoundingBox(currentWorldObject->aabb, RED);	
	
		
			if(currentWorldObject->type == OBJECT_ENTITY){
				Entity *entityData = (Entity *)currentWorldObject->data;	
				worldobject_updateAABB_entity(&currentWorldObject->aabb, entityData);
				if(i == PLAYER_INDEX){
					entityData->position = Vector3Add(camera.position,(Vector3){0,-1.3,0});
					Vector3 ang = GetCameraForward(&camera);
					entityData->rotation = atan2f(ang.x,ang.z) - M_PI/2;
				}
				
				if(!entityData->alive){
					continue;
				}

				entity_updateEntity(entityData);
				DrawModelEx(entityData->model, (Vector3){0,0,0}, (Vector3){0,1.0f,0},0,(Vector3){1.0,1.0f,1.0f},WHITE);
			}
			if(currentWorldObject->type == OBJECT_BULLET){
				
				Bullet *bulletData = (Bullet *)currentWorldObject->data;
				worldobject_updateAABB_bullet(&currentWorldObject->aabb, bulletData);
				
				if(!bulletData->enabled){
					continue;
				}
				
				for (int j = 0; j < worldObjects.count; j++) {
					WorldObject *wo2 = (WorldObject *)worldObjects.data[j];

					// Ensure we're checking against an alive entity and not the bullet itself
					if (wo2->type == OBJECT_ENTITY) {
						Entity* entity = (Entity *)wo2->data;

						// Perform AABB collision check
						if (!entity->player && CheckCollisionBoxes(currentWorldObject->aabb, wo2->aabb)) {
							// Collision detected!
							
							RayCollision hit = GetRayCollisionBox(bulletData->ray, wo2->aabb);
						
							if(hit.hit){
								bulletData->enabled =false;	
								entity->alive = false;
							}
							

							// TraceLog(LOG_INFO, "Bullet %d hit Entity %d", bullet->id, entity->id);

							// Handle the collision:
							// entity->alive = false; // Example: mark entity as dead
							// bullet->enabled = false; // Example: disable the bullet


							// You might want to add more specific collision handling here,
							// like applying damage, triggering effects, etc.

							// Optionally, break out of the inner loop since the bullet hit something
						}
					}
				}
				bullet_updateBullet(bulletData);
				
				DrawLine3D(bulletData->ray.position, Vector3Add(bulletData->ray.position, Vector3Scale(bulletData->ray.direction, bulletData->velocity)), BLACK);
				// bulletData->ray.position = Vector3Add(bulletData->ray.position,Vector3Add(Vector3Scale(bulletData->ray.direction,bulletData->velocity),(Vector3){0,-.001,0}));
			}
		}
		
		DrawModel(gModel, (Vector3){0,0,0}, 1.0f, WHITE);
		// 	DrawModelWires(gModel, (Vector3){0,0,0}, 1.0f, WHITE);

		if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
			shootPlayerFrame++;
			if(shootPlayerFrame >5){
				shootPlayerFrame = 0;
				
				//i want values .9 -> 1.1
				float ranV = (GetRandomValue(95, 105) / 100.0f);
				printf("ranV:%f\n",ranV);
				SetSoundPitch(gunSound, ranV);
				// SetSoundPitch(gunSound, 0.03f);

				PlaySound(gunSound);  
				
				WorldObject *currentWorldObject = (WorldObject *)worldObjects.data[PLAYER_INDEX];
				Entity *entityData = (Entity *)currentWorldObject->data;
				
				Vector3 pos = entityData->anims[entityData->animIndex].framePoses[entityData->animFrame][8].translation;
				pos = Vector3Transform(pos,entityData->model.transform);
				// Vector3 pos2 = (Vector3){0,0,0};
				// printf("pos: {%f, %f, %f}\n",pos.x, pos.y, pos.z);
				
				bullet_fireBullet(camera.position,GetCameraForward(&camera), 20.0f, 0);	
			}
		}
		DrawBoundingBox(testaabb, RED);	
		DrawModel(gon, (Vector3){0,2,0}, 1.0f, WHITE);
		DrawModel(DEBUG_CUBE,(Vector3){5,0,5}, 1.0f, WHITE);
		EndShaderMode();
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