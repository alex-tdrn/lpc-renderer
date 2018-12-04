#version 420 core

struct Light
{
	vec3 color;
	vec3 direction;
};
uniform Light light;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float shininess;
uniform float ambientStrength;

in VS_OUT
{
	vec3 position;
	vec3 normal;
} fs_in;

out vec4 fragColor;

const vec3 normal = normalize(fs_in.normal);
const vec3 viewDirection = normalize(-fs_in.position);

void main()
{
	vec3 diffuse = diffuseColor * max(dot(normal, light.direction), 0.0);
	vec3 halfwayDir = normalize(light.direction + viewDirection);
	vec3 specular =  specularColor * pow(max(dot(normal, halfwayDir), 0.0), shininess);
	vec3 finalColor = light.color * (diffuse + specular) + diffuseColor * ambientStrength;
    fragColor = vec4(finalColor, 1.0f);
}