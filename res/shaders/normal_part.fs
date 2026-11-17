#version 330
in vec2 fragTexCoord;
in vec4 fragColor;
uniform sampler2D texture0;
uniform vec3 colX, colY, colZ;
out vec4 finalColor;

void main() {
    vec4 t = texture(texture0, fragTexCoord);
    if (t.a < 0.5) discard;

    vec3 t3 = t.rgb * 2.0 - 1.0;
    vec3 n = vec3(t3.x, t3.z, t3.y);

    n = normalize(colX*n.x + colY*n.y + colZ*n.z);

    finalColor = vec4(n * 0.5 + 0.5, 1.0);
}
