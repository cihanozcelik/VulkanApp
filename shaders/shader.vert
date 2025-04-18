#version 450

// Hardcoded vertices for a simple triangle
vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

// Output position to the rasterizer
out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    // Use the built-in gl_VertexIndex to select the vertex
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
} 