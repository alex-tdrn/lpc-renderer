#version 460 core

struct Light
{
	vec3 color;
	vec3 direction;
};
uniform Light light;
uniform vec3 diffuseColor;
uniform vec3 specularColor;
uniform float shininess;
uniform vec3 ambientColor;
uniform float ambientStrength;

in VS_OUT
{
	vec3 position;
	vec3 normal;
} fs_in;

out vec4 fragColor;


void main()
{
	if(length(gl_PointCoord - vec2(0.5f)) > 0.5f)
		discard;
	const vec3 normal = normalize(fs_in.normal);
	const vec3 viewDirection = normalize(-fs_in.position);
	const vec3 lightDirection = normalize(-light.direction);
	vec3 diffuse = diffuseColor * max(dot(normal, lightDirection), 0.0);
	vec3 halfwayDir = normalize(lightDirection + viewDirection);
	vec3 specular =  specularColor * pow(max(dot(normal, halfwayDir), 0.0), shininess);
	vec3 finalColor = light.color * (diffuse + specular) + ambientColor * ambientStrength;
    fragColor = vec4(finalColor, 1.0f);
}