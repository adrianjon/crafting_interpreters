struct VertexInput {
	@location(0) position: vec3f,
    @location(1) texcoord: vec2f,
    @location(2) normal: vec3f,
    @location(3) color: vec3f,
};

struct VertexOutput {
	@builtin(position) position: vec4f,
    @location(0) @interpolate(flat) color: vec3f,
    @location(1) normal: vec3f,
    @location(2) uv: vec2f,
};

struct MyUniforms {
    transform: mat4x4f, // 64 bytes (0-63)
};

@group(0) @binding(0) var<uniform> uMyUniforms: MyUniforms;
@group(0) @binding(1) var<uniform> uCameraUniforms: MyUniforms;
@group(0) @binding(2) var gradientTexture: texture_2d<f32>;
@group(0) @binding(3) var textureSampler: sampler;

@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
	var out: VertexOutput;

    let model = uMyUniforms.transform;

    // view transform (camera)
    // let view = mat4x4f(
    //     1.0, 0.0, 0.0, 0.0,
    //     0.0, 1.0, 0.0, 0.0,
    //     0.0, 0.0, 1.0, 0.0,
    //     0.0, 0.0, 0.0, 1.0
    // );
    let view = uCameraUniforms.transform;

    let near = 0.1;
    let far = 100.0;
    let fov = 90.0;
    let aspect = 1.0f;
    let f = 1.0f / tan(fov * 0.5f * 3.14159265f / 180.0f); // cotangent
    let proj = mat4x4f(
    f / aspect, 0.0, 0.0,                     0.0,
    0.0,        f,   0.0,                     0.0,
    0.0,        0.0, (far)/(far - near), 1.0,
    0.0,        0.0, (-near * far)/(far - near),                    0.0
    );
    // let proj = mat4x4f(
    // 1.0, 0.0, 0.0, 0.0,
    // 0.0, 1.0, 0.0, 0.0,
    // 0.0, 0.0, 1.0, 0.0,
    // 0.0, 0.0, 0.0, 1.0
    // );

    // transform to clip space
    let world_pos = model * vec4f(in.position, 1.0);
    out.position = proj * view * world_pos;

    // let min_bound = vec2f(-1.0, -1.0);
    // let max_bound = vec2f(1.0, 1.0);
    // let normalized_pos = (in.position.xy - min_bound) / (max_bound - min_bound);
    // out.texcoord = normalized_pos * 256.0; // texture size
    out.uv = in.texcoord * 2.0 - 0.5;

	out.color = in.color;
	return out;
}

@fragment
fn fs_main(in: VertexOutput) -> @location(0) vec4f {
    let color = textureSample(gradientTexture, textureSampler, in.uv).rgb;
    //let texelCoords = vec2i(in.uv * vec2f(textureDimensions(gradientTexture)));
    //let color = textureLoad(gradientTexture, texelCoords, 0).rgb;
    //let color = in.color;
	let linear_color = pow(color, vec3f(2.2));
	return vec4f(linear_color, 1.0);
}