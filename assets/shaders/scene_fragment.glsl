#version 330 core

const float PI = 3.1415926535897932384626433832795;

uniform vec3 light_color;
uniform vec3 light_position;
uniform vec3 light_direction;

uniform vec3 light_color2;
uniform vec3 light_position2;
uniform vec3 light_direction2;

uniform vec3 object_color;
uniform sampler2D textureSampler;
uniform bool useTexture = true;
uniform bool useCarLight;
uniform bool lightsOn;


const float shading_ambient_strength    = 0.1;
const float shading_diffuse_strength    = 1.0;
const float shading_specular_strength   = 1.0;

const float shading_ambient_strength2 = 0.7;
const float shading_diffuse_strength2 = 0.6;
const float shading_specular_strength2 = 0.3;

uniform float light_cutoff_outer;
uniform float light_cutoff_inner;
uniform float light_near_plane;
uniform float light_far_plane;

uniform float light_cutoff_inner2;
uniform float light_cutoff_outer2;
uniform float light_near_plane2;
uniform float light_far_plane2;

uniform float intensity;

uniform vec3 view_position;

uniform sampler2D shadow_map;

in vec3 fragment_position;
in vec4 fragment_position_light_space;
in vec4 fragment_position_light_space2;
in vec3 fragment_normal;
in vec2 vertexUV;

in vec4 gl_FragCoord;

out vec4 result;

vec3 ambient_color(vec3 light_color_arg) {
    return shading_ambient_strength * light_color_arg;
}

vec3 diffuse_color(vec3 light_color_arg, vec3 light_position_arg, vec3 light_direction_arg) {
    return shading_diffuse_strength * light_color_arg * max(dot(normalize(fragment_normal), light_direction_arg), 0.0f);
}

vec3 specular_color(vec3 light_color_arg, vec3 light_position_arg, vec3 light_direction_arg) {
    vec3 view_direction = normalize(view_position - fragment_position);
    vec3 reflect_light_direction = reflect(-light_direction_arg, normalize(fragment_normal));
    return shading_specular_strength * light_color_arg * pow(max(dot(reflect_light_direction, view_direction), 0.0f), 32);
}

float shadow_scalar() {
    // this function returns 1.0 when the surface receives light, and 0.0 when it is in a shadow
    // perform perspective divide
    vec3 normalized_device_coordinates = fragment_position_light_space.xyz / fragment_position_light_space.w;
    // transform to [0,1] range
    normalized_device_coordinates = normalized_device_coordinates * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragment_position_light_space as coords)
    float closest_depth = texture(shadow_map, normalized_device_coordinates.xy).r;
    // get depth of current fragment from light's perspective
    float current_depth = normalized_device_coordinates.z;
    // check whether current frag pos is in shadow
    float bias = 0.003;// bias applied in depth map: see shadow_vertex.glsl
    float shadow = ((current_depth - bias) < closest_depth) ? 1.0 : 0.0;

    if (normalized_device_coordinates.z > 1.0)
    shadow = 0.0;
    return shadow;
}

float spotlight_scalar() {
    float theta = dot(normalize(fragment_position - light_position), light_direction);

    if (theta > light_cutoff_inner) {
        return 1.0;
    } else if (theta > light_cutoff_outer) {
        return (1.0 - cos(PI * (theta - light_cutoff_outer) / (light_cutoff_inner - light_cutoff_outer))) / 2.0;
    } else {
        return 0.0;
    }
}

void main()
{
    vec3 ambient = vec3(0.0f);
    vec3 diffuse = vec3(0.0f);
    vec3 specular = vec3(0.0f);

    vec3 diffuse2 = vec3(0.0f);
    vec3 specular2 = vec3(0.0f);

    vec3 light_dir = normalize(light_position - fragment_position);

    float scalar = shadow_scalar() * spotlight_scalar();
    ambient = ambient_color(light_color);
    diffuse = scalar * diffuse_color(light_color, light_position, light_dir);
    specular = scalar * specular_color(light_color, light_position, light_dir);
    vec3 lightColor = vec3(0.0f);

    vec3 objColor = useTexture ? texture(textureSampler, vertexUV).rgb : object_color;

    vec3 color;
    if (useCarLight){
        diffuse2 =  spotlight_scalar()* shading_diffuse_strength2 *diffuse_color(light_color2, light_position2, light_direction);
        specular2 =  spotlight_scalar()*shading_specular_strength2 *specular_color(light_color2, light_position2, light_direction);
        lightColor = specular2 + diffuse2 + diffuse + specular;
    }

    color = ((intensity * (ambient + ambient)) + lightColor) * objColor;

    result = vec4(color, 1.0f);
}

