#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
uniform sampler2D normalMap;
uniform vec3 lightPos;
uniform vec2 carPos;
uniform vec2 boxSize;
uniform vec2 pivot;
uniform float scale;
out vec4 finalColor;

void main() {
    vec4 texel = texture(texture0, fragTexCoord);
    if (texel.a < 0.01) discard;

    vec3 n = normalize(texture(normalMap, fragTexCoord).rgb * 2.0 - 1.0);

    vec2 boxPx = vec2(fragTexCoord.x, 1.0 - fragTexCoord.y) * boxSize;
    vec2 world = carPos + (boxPx - pivot) * scale;

    vec3 L = normalize(lightPos - vec3(world, 0.0));
    float diff = max(dot(n, L), 0.0);

    finalColor = vec4(texel.rgb * (0.2 + diff), texel.a) * fragColor;
}
