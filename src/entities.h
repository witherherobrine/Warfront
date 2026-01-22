

#ifndef ENTITIES_H
#define ENTITIES_H
// --- Public ECS Functions ---
EntityId create_entity();
void destroy_entity(EntityId id);
void attach_transform(EntityId id, Vector3 pos, Vector3 rotAx, float rotAng, Vector3 scale);
void attach_render(EntityId id, Model model);
void attach_player_tag(EntityId id);
void attach_camera(EntityId id, float fovY, float pitch, float yaw, bool isActive);
void attach_parent(EntityId id, EntityId parentId, Vector3 localOffset);
void attach_terrain_chunk(EntityId id, int gx, int gz, float worldSize, float resolution);
void attach_soldier(EntityId id);
void attach_animations(EntityId id, ModelAnimation* anims);



void TerrainSetupSystem(Shader shader);
void RecalculateMeshNormals(Mesh *mesh);
EntityId GetActiveCameraId(void);
void SetActiveCamera(EntityId id);
#endif
