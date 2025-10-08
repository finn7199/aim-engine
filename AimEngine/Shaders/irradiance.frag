#version 330 core
out vec4 FragColor;
in vec3 WorldPos;

uniform samplerCube uEnvironmentMap;

const float PI = 3.14159265359;

float RadicalInverse_VdC(uint bits) 
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    return float(bits) * 2.3283064365386963e-10; // / 0x100000000
}

vec2 Hammersley(uint i, uint N)
{
    return vec2(float(i)/float(N), RadicalInverse_VdC(i));
}

void main()
{
    vec3 N = normalize(WorldPos);
    vec3 irradiance = vec3(0.0);

    // Importance sampling cause why not  
    // Create a coordinate system around the normal
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up = normalize(cross(N, right));
    
    const uint SAMPLE_COUNT = 1024u;
    for(uint i = 0u; i < SAMPLE_COUNT; ++i)
    {
        // Get 2 random numbers using the Hammersley sequence
        vec2 Xi = Hammersley(i, SAMPLE_COUNT);
        
        // Use the random numbers to generate a sample direction with a cosine weighted distribution.
        float phi = 2.0 * PI * Xi.x;
        float cosTheta = sqrt(1.0 - Xi.y);
        float sinTheta = sqrt(1.0 - cosTheta*cosTheta);
        
        // Convert from spherical to cartesian coordinates in tangent space
        vec3 tangentSample;
        tangentSample.x = cos(phi) * sinTheta;
        tangentSample.y = sin(phi) * sinTheta;
        tangentSample.z = cosTheta;
        
        // Convert from tangent space to world space
        vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;
        
        // Sample the environment map and add to the total
        irradiance += texture(uEnvironmentMap, sampleVec).rgb;
    }
    // Average the samples
    irradiance = irradiance / float(SAMPLE_COUNT);
    
    FragColor = vec4(irradiance, 1.0);
}