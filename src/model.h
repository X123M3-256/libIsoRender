#ifndef MODEL_H_INCLUDED
#define MODEL_H_INCLUDED
#include <stdlib.h>
#include <stdint.h>
#include "vectormath.h"
#include "color.h"

typedef struct //Texture
	{
	uint16_t width;
	uint16_t height;
	vector3_t* pixels;
	}texture_t;

enum material_flags
{
MATERIAL_HAS_TEXTURE=1,
MATERIAL_IS_REMAPPABLE=2,
MATERIAL_IS_MASK=4,
MATERIAL_NO_AO=8,
MATERIAL_BACKGROUND_AA=16,
};

typedef struct
{
uint8_t flags;
uint8_t region;
float specular_exponent;
vector3_t specular_color;
	union
	{
	texture_t texture;
	vector3_t color;
	};
}material_t;

typedef struct
{
size_t material;
size_t indices[3];
}face_t;



typedef struct
{
vector3_t* vertices;
vector3_t* normals;
vector2_t* uvs;
face_t* faces;
material_t* materials;
size_t num_vertices;
size_t num_faces;
size_t num_materials;
}mesh_t;

void texture_init(texture_t* texture,uint16_t width,uint16_t height);
int texture_load_png(texture_t* texture,const char* filename);
vector3_t texture_sample(texture_t* texture,vector2_t coord);
int texture_load_png(texture_t* texture,const char* filename);
void texture_destroy(texture_t* texture);

int mesh_load(mesh_t* mesh,const char* filename);
void mesh_destroy(mesh_t* mesh);

#endif // MODEL_H_INCLUDED
