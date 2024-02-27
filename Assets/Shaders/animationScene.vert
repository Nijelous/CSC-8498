#version 420 core

uniform mat4 modelMatrix 	= mat4(1.0f);
uniform mat4 shadowMatrix 	= mat4(1.0f);

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 colour;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec4 tangent;
layout(location = 5) in vec4 jointWeights;
layout(location = 6) in ivec4 jointIndices;

layout(std140, binding = 0) uniform CamBlock{
	mat4 projMatrix;
	mat4 viewMatrix;
	mat4 invProjView;
	vec3 camPos;
} camData;

uniform mat4 joints[128];

uniform vec4 		objectColour = vec4(1,1,1,1);
uniform bool hasVertexColours = false;

out Vertex
{
	vec4 colour;
	vec2 texCoord;
	vec4 shadowProj;
	vec3 normal;
	vec3 tangent;
	vec3 binormal;
	vec3 worldPos;

} OUT;

void main(void)
{
	mat4 mvp 		  = (camData.projMatrix * camData.viewMatrix * modelMatrix);
	mat3 normalMatrix = transpose ( inverse ( mat3 ( modelMatrix )));
	vec3 wNormal = normalize ( normalMatrix * normalize (normal.xyz));
	vec3 wTangent = normalize(normalMatrix * normalize(tangent.xyz));

	vec4 localNormal = vec4(normal, 1.0f);
	vec4 localPos 	= vec4(position, 1.0f);
	vec4 skelPos 	= vec4(0,0,0,0);
	vec4 skelNormal = vec4(0,0,0,0);




	if(hasVertexColours) {
		OUT.colour		= objectColour * colour;
	}



	for(int i = 0; i < 4; ++i) {
		int   jointIndex 	= jointIndices[i];
		float jointWeight 	= jointWeights[i];

		skelPos += joints[jointIndex] * localPos * jointWeight;

		skelNormal += joints[jointIndex] * localNormal * jointWeight;
	}

	
	
	gl_Position = mvp * vec4(skelPos.xyz, 1.0);
	OUT.shadowProj 	=  shadowMatrix * vec4 ( skelPos.xyz,1);
	OUT.worldPos 	= ( modelMatrix * vec4 ( skelPos.xyz ,1)). xyz ;
	OUT.tangent = wTangent;
	OUT.binormal = cross(wTangent, wNormal) * tangent.w;
	OUT.texCoord	= vec2(texCoord.x,1-texCoord.y);
	OUT.colour		= objectColour;
	OUT.normal 		= skelNormal.xyz;
	
}