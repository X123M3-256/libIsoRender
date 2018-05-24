#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED
#include <stdlib.h>
#include <stdint.h>
#include "renderer.h"
#include "vectormath.h"
#include "color.h"

typedef struct
{
size_t vertices[3];
size_t normals[3];
size_t uvs[3];
size_t material;
}face_t;



typedef struct
{
vector3_t* vertices;
vector3_t* normals;
vector2_t* uvs;
face_t* faces;
material_t* materials;
size_t num_vertices;
size_t num_normals;
size_t num_uvs;
size_t num_faces;
size_t num_materials;
}mesh_t;


int texture_load_png(texture_t* texture,const char* filename);
int mesh_load_obj(mesh_t* mesh,const char* filename);
uint32_t mesh_count_primitives(mesh_t* mesh);
primitive_t mesh_get_primitives(mesh_t* mesh,primitive_t* primitives);

#endif // MODEL_H_INCLUDED
