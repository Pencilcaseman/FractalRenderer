#version 150 core

uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;

out vec4 o_color;

void main(void) {
    vec2 st = (gl_FragCoord.xy) / u_resolution.xy - u_mouse.xy;
    st.x -= 0.2;

    vec2 uv = (st * 2.0) - 1.0;
    uv.x *= u_resolution.x / u_resolution.y;

    vec3 col = vec3(0.0);

    float scale = 1.0;
    vec2 c = uv * scale;
    vec2 z = vec2(0.0);

    float i = 0.0;
    float max_iter = 100.0;

    // Mandelbrot with smooth coloring and 4x4 anti-aliasing
    for (int n = 0; n < 4; n++) {
        for (int m = 0; m < 4; m++) {
            vec2 p = (uv + vec2(n, m) / u_resolution.xy) * scale;
            z = vec2(0.0);
            i = 0.0;
            for (i = 0.0; i < max_iter; i++) {
                z = vec2((z.x * z.x) - (z.y * z.y), 2.0 * z.x * z.y) + p;
                if (dot(z, z) > 4.0) {
                    break;
                }
            }

            // Stepped colouring
            // col += 0.5 * cos(vec3(0.0, 0.5, 1.0) * 3.1415 + i) + 0.5;

            // Smooth coloring with logarithmic spirals (bit dodgy)
            // float v = i + 1.0 - log(log(dot(z, z))) / log(2.0);
            // col += 0.5 + 0.5 * cos(v + vec3(0.0, 0.5, 1.0) * 3.1415);

            // This one is awesome hehe
            float v = -log(log(dot(z, z))) / log(2.0);
            float a = atan(z.y, z.x);
            col += 0.5 + 0.5 * cos(v * 0.1 + a + vec3(0.0, 0.5, 1.0) * 3.1415);

            // Weird orange thing
            // float a = sin(i / max_iter * 3.1415) * 0.5 + 0.5 + 0.5 * sin(i / max_iter * 3.1415 * 2.0) * 0.5 + 0.5 + 0.25 * sin(i / max_iter * 3.1415 * 4.0) * 0.5 + 0.5;
            // float b = atan(z.y, z.x) / 3.1415 * 0.5 + 0.5;
            // float c = -log(log(dot(z, z))) / log(2.0) / 4.0;
            // col += vec3(a, b, c);

            // Darker colourings
            // float a = atan(z.y, z.x) / 3.1415 * 0.5 + 0.5;
            // float b = -log(log(dot(z, z))) / log(2.0) / 4.0;
            // col += vec3(0.1, 0.6, 0.8) * a + vec3(0.8, 0.1, 0.6) * b;
        }
    }

    col /= 16.0;

    o_color = vec4(col, 1.0);
}