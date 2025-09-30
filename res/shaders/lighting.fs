// Fragment Shader (lighting.fs)
#version 330 core

//in vec3 FragPos;
//in vec3 Normal;

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;

uniform vec3 ambientColor;
uniform vec3 lightDir;
uniform vec3 lightColor;
uniform sampler2D texture0;

out vec4 FragColor;

void main()
{
    // Ambient lighting
    vec3 ambient = ambientColor;

    // Directional lighting
    vec3 norm = normalize(fragNormal);
    vec3 lightDirNorm = normalize(lightDir);
    float diff = max(dot(norm, -lightDirNorm), 0.0); // Negate lightDir for direction towards the light
    vec3 diffuse = diff * lightColor;

    vec3 result = ambient + diffuse;
    vec4 texColor = texture(texture0, fragTexCoord);
    FragColor = texColor * vec4(result, 1.0);
}