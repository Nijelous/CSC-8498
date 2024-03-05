#version 460 core

#extension GL_ARB_bindless_texture : require

uniform sampler2D iconTex;

layout(std140, binding = 5) uniform IconBlock {
	bool isOn;
	bool useTexture;
} iconData; 

layout(std140, binding = 6) uniform TextureHandles {
	sampler2D handles[64];
} texHandles;

layout(std140, binding = 7) uniform TextureHandleIDs{
	int albedoIndex;
	int normalIndex;
	int depthIndex;
	int shadowIndex;
	int albedoLightIndex;
	int specLightIndex;
} texIndices;
in Vertex
{
    vec2 texCoord;
}IN;

out vec4 fragColor;

void main(void)
{ 
     if(iconData.isOn){
          fragColor = texture(iconTex, IN.texCoord);
     }
     else{
          fragColor = texture(iconTex, IN.texCoord) * vec4(0.06, 0.06, 0.06, 0.2);
     }
}