#version 410

layout(location = 0) in vec3 position;

// Data from CPU
uniform mat4 MVP; // ModelViewProjection Matrix
uniform mat4 MV; // ModelView idMVPMatrix
uniform vec4 cameraPosition;
uniform float heightFactor;

// Texture-related data
uniform sampler2D rgbTexture;
uniform int widthTexture;
uniform int heightTexture;


// Output to Fragment Shader
out vec2 textureCoordinate; // For texture-color
out vec3 vertexNormal; // For Lighting computation
out vec3 ToLightVector; // Vector from Vertex to Light;
out vec3 ToCameraVector; // Vector from Vertex to Camera;

uniform mat4 NM;

vec3 lpos = vec3(widthTexture/2, widthTexture + heightTexture, heightTexture/2);

vec3 n[6];
vec3 normal;

void main()
{
    int ncount = 6;
    vec3 pos = position;

    textureCoordinate = vec2(1 - (float(position.x) / (widthTexture + 1)), 1 - (float(position.z) / (heightTexture + 1)));

    vec4 t_color = texture(rgbTexture, textureCoordinate);
    vec3 color = t_color.xyz;

    pos.y = heightFactor*(0.2126*color.x + 0.7152*color.y + 0.0722*color.z);
    vec2 n_coord;
    vec3 r_normal;

    vec4 lv = vec4(lpos - pos, 0);
    ToLightVector = normalize(vec3(MV*lv));
    vec4 cv = vec4(vec3(cameraPosition) - pos, 0);
    ToCameraVector = normalize(vec3(MV*cv));

    n[0] = vec3(position.x - 1, 0 , position.z);
    n[1] = vec3(position.x - 1 , 0 , position.z + 1);
    n[2] = vec3(position.x , 0 , position.z + 1);
    n[3] = vec3(position.x + 1 , 0 , position.z);
    n[4] = vec3(position.x + 1, 0 , position.z - 1);
    n[5] = vec3(position.x , 0 , position.z - 1);        
    
    for(int i = 0; i < ncount; i++)
    {
        n_coord.x = abs(n[i].x - widthTexture)/widthTexture;
        n_coord.y = abs(n[i].z - heightTexture)/heightTexture;
        if(n_coord.x >= 0 && n_coord.x <= widthTexture && n_coord.y >=0 && n_coord.y <= heightTexture)
            t_color = texture(rgbTexture, n_coord);
        else
            t_color = texture(rgbTexture, textureCoordinate);
        n[i].y = heightFactor*(0.2126*t_color.x + 0.7152*t_color.y + 0.0722*t_color.z);
        n[i] = n[i] - pos; 
    }

    vec3 tempn = vec3(0,0,0);

    for(int i=0; i<ncount; i++)
    {
        normal = cross(n[i], n[(i+1)%ncount]);
        tempn += normal;
    }        
    r_normal = normalize(tempn);

    vertexNormal = normalize(vec3(NM * vec4(r_normal, 0)));

    gl_Position = MVP*vec4(pos,1);

}