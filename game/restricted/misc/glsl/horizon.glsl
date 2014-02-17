#version 130

#ifdef VERTEXPROGRAM

uniform mat4 projection_matrix;
uniform mat4 view_matrix;
uniform mat4 model_matrix;

void main() {
    gl_Position = projection_matrix * view_matrix * model_matrix * gl_Vertex;
    gl_TexCoord[0] = gl_MultiTexCoord0;
    gl_FrontColor = gl_Color;
}

#endif

#ifdef FRAGMENTPROGRAM

uniform sampler2D diffuse;
uniform vec4 sector_tint;
uniform vec2 screen_size;
uniform vec3 horizon_sky_offset;

void horizon_sky_texture_coordinates(in vec4 vScreenPos, out vec2 tex_coords) {
    vec2 vTexCoord = vScreenPos.xy * horizon_sky_offset.z / screen_size.x;
    
    vTexCoord.x += horizon_sky_offset.x;
    vTexCoord.y += horizon_sky_offset.y;

    tex_coords = vTexCoord;
}

void main() {
    vec2 tex_coords;
    horizon_sky_texture_coordinates(gl_FragCoord, tex_coords);

    vec4 diffuse = texture2D(diffuse, tex_coords);
    vec4 diffuse_color = diffuse;
    vec4 diffuse_color_tint = diffuse_color * sector_tint;
    vec4 diffuse_color_tint_mix = mix(diffuse_color, diffuse_color_tint, sector_tint.a);
    gl_FragColor = diffuse_color_tint_mix;
    gl_FragColor.a = diffuse_color.a;
    gl_FragDepth = gl_DepthRange.far - 0.000002;
}

#endif
