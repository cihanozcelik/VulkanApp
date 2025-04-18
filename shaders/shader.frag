#version 450

// Output color for the fragment
layout(location = 0) out vec4 outColor;

void main() {
    // Output a constant orange color
    outColor = vec4(1.0, 0.5, 0.0, 1.0);
} 