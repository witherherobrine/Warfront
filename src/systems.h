

#ifndef SYSTEMS_H
#define SYSTEMS_H
void DrawSystem(void);
void PlayerInputSystem(float deltaTime);
void CameraSystem(float deltaTime);
void ParentingSystem();
void UpdateLightingUniforms(Camera camera, Shader terrainShader, int lightDirLoc, int lightColorLoc, int viewPosLoc);
void ViewDistanceManagerSystem(Texture t, Shader s);
void temporaryAIMoveSystem(void);
void AnimationSystem(void);
#endif
