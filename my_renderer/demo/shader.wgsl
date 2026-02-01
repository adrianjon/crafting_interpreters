struct vertex_input {
    @location(0) position: vec3f,
    @location(1) normal: vec3f,
    @location(2) color: vec3f,
};
struct vertex_output {
    @builtin(position) position: vec4f,
    @location(0) color: vec3f,
    @location(1) normal: vec3f,
};
struct my_uniforms {
    projection_matrix: mat4x4f,
    view_matrix: mat4x4f,
    model_matrix: mat4x4f,
    color: vec4f,
    time: f32,
};
@group(0) @binding(0) var<uniform> u_my_uniforms: my_uniforms;
@vertex
fn vs_main(in: vertex_input) -> vertex_output {
    var out: vertex_output;
    out.position = u_my_uniforms.projection_matrix * u_my_uniforms.view_matrix * u_my_uniforms.model_matrix * vec4f(in.position, 1.0);

    out.normal = (u_my_uniforms.model_matrix * vec4f(in.normal, 0.0)).xyz;
    out.color = in.color;
    return out;
}
@fragment 
fn fs_main(in: vertex_output) -> @location(0) vec4f {
    let normal = normalize(in.normal);
    let light_color1 = vec3f(1.0, 0.9, 0.6);
    let light_color2 = vec3f(0.6, 0.9, 1.0);
    let light_dir1 = vec3f(0.5, -0.9, 0.1);
    let light_dir2 = vec3f(0.2, 0.4, 0.3);
    let shading1 = max(0.0, dot(light_dir1, normal));
    let shading2 = max(0.0, dot(light_dir2, normal));
    let shading = shading1 * light_color1 + shading2 * light_color2;
    let color = in.color * shading;
    return vec4f(1.0f, 0.0f, 0.0f, 1.0f);
    //return vec4f(color, u_my_uniforms.color.a);
}