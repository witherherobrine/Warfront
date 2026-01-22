
#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;

out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragTexCoord;

uniform mat4 mvp;
uniform mat4 matModel; // Model matrix (for world space conversion)

void main() {
    // Transform vertex position to world space
    fragPosition = vec3(matModel * vec4(vertexPosition, 1.0));
    // Transform normal to world space (ignore translation)
    fragNormal = mat3(matModel) * vertexNormal;
    fragTexCoord = vertexTexCoord;
    
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
