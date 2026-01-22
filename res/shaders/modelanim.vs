#version 330

// Input vertex attributes
layout (location = 0) in vec3 vertexPos;
layout (location = 1) in vec2 vertexTexCoord;
in vec4 vertexColor;
in vec4 vertexBoneIds;   // Mesh bone indices
in vec4 vertexBoneWeights;// Mesh bone weights

// Input instance attributes (from your instancing buffer)
in mat4 instanceTransform;
in float instanceFrame;   // Which row to look at

// Input uniform
uniform mat4 mvp;
uniform sampler2D uAnimTexture; // Renamed from texture0
uniform int boneCount;      // Number of bones in the skeleton (17 in your case)

out vec2 fragTexCoord;
out vec4 fragColor;

mat4 getBoneMatrix(float boneId, float frame) {

    int x = int(boneId) * 4;
    int y = int(frame);
    
    // Fetch the 4 pixels (rows)
    vec4 r0 = texelFetch(uAnimTexture, ivec2(x, y), 0);
    vec4 r1 = texelFetch(uAnimTexture, ivec2(x + 1, y), 0);
    vec4 r2 = texelFetch(uAnimTexture, ivec2(x + 2, y), 0);
    vec4 r3 = texelFetch(uAnimTexture, ivec2(x + 3, y), 0);
    
    // Create the matrix. 
    // If 'transpose' made it better but limbs are still shifted, 
    // we need to verify the constructor order.
    mat4 m = mat4(r0, r1, r2, r3); 
    return transpose(m); 
}

void main() {
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    // 1. EXTRACT and CLEAN the instance matrix immediately
    float frame = instanceTransform[3][3];
    
    mat4 realInstanceMat = instanceTransform;
    realInstanceMat[3][3] = 1.0; // If you don't do this, the model scales by 'frame'

    // 2. Bone logic (Keep this here for when you turn it back on)
    mat4 boneMat = getBoneMatrix(vertexBoneIds.x, frame) * vertexBoneWeights.x;
    boneMat += getBoneMatrix(vertexBoneIds.y, frame) * vertexBoneWeights.y;
    boneMat += getBoneMatrix(vertexBoneIds.z, frame) * vertexBoneWeights.z;
    boneMat += getBoneMatrix(vertexBoneIds.w, frame) * vertexBoneWeights.w;

    // 3. MULTIPLICATION ORDER
    // Correct: Projection * View * InstanceWorld * BoneOffset * LocalPos
    // Raylib's 'mvp' usually contains Projection * View
    gl_Position = mvp * realInstanceMat * boneMat * vec4(vertexPos, 1.0);
}
