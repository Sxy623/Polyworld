#version 330 core
layout (location = 0) out vec4 FragColor;
//out vec4 FragColor;

in GS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec3 viewNormal;
    vec4 FragPosLightSpace;
} fs_in;

struct ParallelLight{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
struct PointLight{
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};
#define NR_POINT_LIGHTS 1

uniform vec3 viewPos;

uniform vec3 water_color;
uniform ParallelLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];

uniform sampler2D BlurShadow;

vec3 CalcParallelLight(ParallelLight light, vec3 normal, vec3 viewDir, float visibility);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow);
float ShadowCalculation(vec4 fragPosLightSpace);

void main() {
    float visibility = ShadowCalculation(fs_in.FragPosLightSpace);

    //Light calculate
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 factory = CalcParallelLight(dirLight, fs_in.Normal, viewDir, visibility);

    vec3 aFragColor = water_color * factory;

    FragColor = vec4(aFragColor, 0.6f);
}

float ShadowCalculation(vec4 fragPosLightSpace){
    vec3 lightDir = normalize(-dirLight.direction);
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    if(projCoords.z > 1.0f)
        return 1.0f;

    float bias = max(0.008 * dot(fs_in.Normal, lightDir), 0.0055);
    bias = 0.00051f;
//    bias = 0.0f;
    float currentDepth = projCoords.z;
    float closestDepth = texture(BlurShadow, projCoords.xy).r;
//    shadow = currentDepth + bias > closestDepth ? 1.0 : 0.0;
    float shadow = clamp(exp(-20.0f * (currentDepth - closestDepth - bias)), 0.0, 1.0);
//    return (currentDepth - closestDepth - bias);
    return shadow;
}
vec3 CalcParallelLight(ParallelLight light, vec3 normal, vec3 viewDir, float visibility){
    vec3 lightDir = normalize(-light.direction);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 8.0f);
    // combine results
    vec3 ambient = light.ambient;
    vec3 diffuse = light.diffuse * diff;
    vec3 specular = light.specular * spec;
    return ambient + (diffuse + specular) * visibility;
}
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, float shadow){
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    // combine results
    vec3 ambient = light.ambient;
    vec3 diffuse = light.diffuse * diff;
    ambient *= attenuation;
    diffuse *= attenuation;
    return ambient + diffuse * (1.0f - shadow);
}
