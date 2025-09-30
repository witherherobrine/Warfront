// Vertex Shader (lighting.vs)
#version 330 core


//uniform mat4 matModel;
//uniform mat4 view;
//uniform mat4 projection;

//out vec3 FragPos;
//out vec3 Normal;


in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;




void main()
{

    //FragPos = vec3(matModel * vec4(aPos, 1.0));
   // Normal = mat3(transpose(inverse(matModel))) * aNormal; // Correct normal transformation
    //gl_Position = projection * view * vec4(FragPos, 1.0);
	
	
    // Send vertex attributes to fragment shader
    fragPosition = vec3(matModel*vec4(vertexPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragNormal = normalize(vec3(matNormal*vec4(vertexNormal, 1.0)));

    // Calculate final vertex position
    gl_Position = mvp*vec4(vertexPosition, 1.0);
}