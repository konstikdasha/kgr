#version 330 core
layout(location = 0) in vec4 vertex;
layout(location = 1) in vec4 normal;
layout(location = 2) in vec2 texCoords;
layout(location = 3) in vec2 intsVar;
layout(location = 4) in vec3 tangent;
layout(location = 5) in vec3 bitangent;


out vec2 vTexCoords;
out vec3 vFragPosition;
out vec3 vNormal;
out vec3 skyBoxTex;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform int type;
uniform float move;

void main()
{
  if (type == 1) {
        skyBoxTex = vec3(vertex[0], vertex[1], vertex[2]);
        vec4 pos = projection * view * vertex;
        gl_Position = pos.xyww;
  }
  else if (type == 0){
          gl_Position = projection * view * model * vertex;

          vTexCoords = texCoords;
          vFragPosition = vec3(model * vertex);
          vNormal = normalize(mat3(transpose(inverse(model))) * normal.xyz);

         mat3 normalMatrix = transpose(inverse(mat3(model)));
         vec3 T = normalize(normalMatrix * tangent);
         vec3 N = normalize(normalMatrix * vNormal);
         T = normalize(T - dot(T, N) * N);
         vec3 B = cross(N, T);
    
         TBN = transpose(mat3(T, B, N));    
  }
  else if (type == 2){
         vec3 instMove;
         if (gl_InstanceID % 2 == 0) {
            instMove = vec3(sin(intsVar.x + move*2) - 20, cos(move)*intsVar.y, cos(intsVar.x + move) + 500);
         } 
         else {
            instMove = vec3(sin(intsVar.x + move) - 20, sin(move)*intsVar.x, cos(intsVar.y + move*2) + 500);
         }
         gl_Position = projection * view * model * (vertex+vec4(instMove, 0.0f));
         vTexCoords = texCoords;
         vNormal = normalize(mat3(transpose(inverse(model))) * normal.xyz);
         vFragPosition = vec3(model * vertex) + vec3(intsVar, 1.0f);

         mat3 normalMatrix = transpose(inverse(mat3(model)));
         vec3 T = normalize(normalMatrix * tangent);
         vec3 N = normalize(normalMatrix * vNormal);
         T = normalize(T - dot(T, N) * N);
         vec3 B = cross(N, T);
    
         TBN = transpose(mat3(T, B, N));    
  }
}