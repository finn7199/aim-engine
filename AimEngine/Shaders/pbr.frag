#version 330 core
out vec4 FragColor;

in vec3 WorldPos;
in vec3 Normal;

// Material properties
uniform vec3  uAlbedo;
uniform float uMetallic;
uniform float uRoughness;
uniform float uAo;

// Light structs
struct DirLight {
    vec3 direction;
    vec3 color;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
};

// Light uniforms
uniform DirLight uDirLight;
uniform SpotLight uSpotLight;
uniform bool uSpotLightEnabled;

uniform vec3 uViewPos;

// IBL uniforms
uniform samplerCube uIrradianceMap;
uniform samplerCube uPrefilterMap;
uniform sampler2D uBrdfLUT;

const float PI = 3.14159265359;

// PBR cook-torrance BRDF
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Universal PBR calculation for a single light source
vec3 calculatePBR(vec3 lightDir, vec3 lightColor, vec3 N, vec3 V, vec3 F0, vec3 albedo, float metallic, float roughness) {
    vec3 H = normalize(V + lightDir);

    // Cook-Torrance BRDF
    float NDF = DistributionGGX(N, H, roughness);
    float G   = GeometrySmith(N, V, lightDir, roughness);
    vec3  F   = fresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, lightDir), 0.0) + 0.001; // Add 0.001 to prevent division by zero
    vec3 specular     = numerator / denominator;

    float NdotL = max(dot(N, lightDir), 0.0);
    return (kD * albedo / PI + specular) * lightColor * NdotL;
}

void main()
{		
    // Basic vectors and material properties
    vec3 N = normalize(Normal);
    vec3 V = normalize(uViewPos - WorldPos);
    vec3 R = reflect(-V, N); 

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, uAlbedo, uMetallic);
    
    // 1. DIRECT LIGHTING CALCULATION
    // Initialize outgoing radiance from direct lights (sun, spotlight, etc.)
    vec3 Lo = vec3(0.0);

    // Directional Light
    Lo += calculatePBR(normalize(-uDirLight.direction), uDirLight.color, N, V, F0, uAlbedo, uMetallic, uRoughness);
    
    // Spot Light
    if (uSpotLightEnabled) {
        vec3 lightDir = normalize(uSpotLight.position - WorldPos);
        
        float distance = length(uSpotLight.position - WorldPos);
        float attenuation = 1.0 / (uSpotLight.constant + uSpotLight.linear * distance + uSpotLight.quadratic * (distance * distance));
        
        float theta = dot(lightDir, normalize(-uSpotLight.direction));
        float epsilon = uSpotLight.cutOff - uSpotLight.outerCutOff;
        float intensity = clamp((theta - uSpotLight.outerCutOff) / epsilon, 0.0, 1.0);

        vec3 spotLightColor = uSpotLight.color * attenuation * intensity;
        Lo += calculatePBR(lightDir, spotLightColor, N, V, F0, uAlbedo, uMetallic, uRoughness);
    }
    
    // 2. IBL CALCULATION
    // Get the fresnel term for reflections
    vec3 F = fresnelSchlick(max(dot(N, V), 0.0), F0);
    
    // Get ratios for diffuse and specular light
    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - uMetallic; // Non-metals have no diffuse component
    
    // Diffuse IBL
    vec3 irradiance = texture(uIrradianceMap, N).rgb;
    vec3 diffuse    = irradiance * uAlbedo;
    
    // Specular IBL
    const float MAX_REFLECTION_LOD = 4.0; // Corresponds to number of mip levels in prefilter map
    vec3 prefilteredColor = textureLod(uPrefilterMap, R,  uRoughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(uBrdfLUT, vec2(max(dot(N, V), 0.0), uRoughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);
    
    // Combine ambient IBL parts
    vec3 ambient = (kD * diffuse + specular) * uAo;
    
    // 3. FINAL COMBINATION
    vec3 color = ambient + Lo; // Add ambient and direct lighting

    // HDR tonemapping and gamma correction
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color, 1.0);
}