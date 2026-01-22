#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0; // The base diffuse texture of your model
uniform vec4 colDiffuse;

out vec4 finalColor;

void main() {
    // Basic texture lookup + tint
    vec4 texelColor = texture(texture0, fragTexCoord);
    finalColor = texelColor * colDiffuse * fragColor;
}
