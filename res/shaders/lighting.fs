#version 330

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;

out vec4 finalColor;

// Light properties
uniform vec3 viewPos;
uniform vec3 lightDir;  // Direction *from* the light source
uniform vec3 lightColor;
uniform float ambientStrength;

// Surface properties
uniform sampler2D textureSampler;

void main() {
    // 1. Surface and Light Vectors
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(lightDir);
    vec3 V = normalize(viewPos - fragPosition); // View vector (Camera to Fragment)

    // 2. Ambient Term (Always present)
    vec3 ambient = ambientStrength * lightColor;

    // 3. Diffuse Term (Dot product of Normal and Light direction)
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * lightColor;

    // 4. Specular Term (Blinn-Phong: Halfway Vector)
    vec3 H = normalize(L + V);
    float spec = pow(max(dot(N, H), 0.0), 32.0); // Specular power (shininess)
    vec3 specular = spec * lightColor * 0.5; // Reduced intensity

    // 5. Texture Color
    vec3 texColor = texture(textureSampler, fragTexCoord).rgb;

    // Final Color (Diffuse lighting * Texture Color) + Ambient + Specular
    vec3 result = (diffuse * texColor) + (ambient * texColor) + specular;
    
    finalColor = vec4(result, 1.0);
}
