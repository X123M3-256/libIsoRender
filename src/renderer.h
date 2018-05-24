#ifndef RENDERER_H_INCLUDED
#define RENDERER_H_INCLUDED
#include <image.h>
#include "palette.h"
#include "vectormath.h"

#define RENDER_WIDTH 255
#define RENDER_HEIGHT 256
#define RENDER_PIXELS (RENDER_WIDTH*RENDER_HEIGHT)
#define UNITS_PER_TILE 4096
#define UNITS_PER_PIXEL 128
#define FRAGMENT_UNUSED 255
#define REGION_MASK 0x7

typedef struct //Rect
	{
	int32_t x_lower;
	int32_t y_lower;
	int32_t x_upper;
	int32_t y_upper;
	}rect_t;

typedef struct //Fragment
	{
	vector3_t color;
	uint8_t region;
	}fragment_t;

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
	MATERIAL_IS_MASK=4
	};

	typedef struct
	{
	uint8_t flags;
	uint8_t region;
	float specular_intensity;
	float specular_exponent;
		union
		{
		texture_t texture;
		vector3_t color;
		};
	}material_t;



typedef struct //Light
	{
	vector3_t direction;
	float ambient;
	float diffuse;
	}light_t;


typedef struct //Primitive
	{
	vector3_t vertices[3];
	vector3_t normals[3];
	vector2_t uvs[3];
	material_t* material;
	}primitive_t;



typedef struct
{
uint16_t width;
uint16_t height;
vector2_t offset;
fragment_t* fragments;
}framebuffer_t;

typedef struct
	{
	light_t light;
	transform_t projection;
	palette_t palette;
	}context_t;

void texture_destroy(texture_t* texture);

extern void context_init(context_t* context,light_t light,palette_t palette,float units_per_tile);
extern void context_rotate(context_t* context);
extern void context_map(context_t* context,fragment_t (*f)(fragment_t,void*),void* data);
//extern void context_reduce(context_t* context,int32_t (*f)(fragment_t,int32_t))
extern void context_clear(context_t* context);
extern void context_draw_primitive(context_t* context,primitive_t primitive);
extern void context_get_image(context_t* render,image_t* image);
extern void context_get_depth(context_t* render,image_t* depth);
extern void context_destroy(context_t* context);

void transform_primitives(transform_t transform,primitive_t* primitives,uint32_t num_primitives);

void framebuffer_init(framebuffer_t* framebuffer,uint32_t width,uint32_t height);
void framebuffer_from_primitives(framebuffer_t* framebuffer,context_t* context,primitive_t* primitives,uint32_t num_primitives);
void framebuffer_save_bmp(framebuffer_t* framebuffer,char* filename);
void framebuffer_destroy(framebuffer_t* framebuffer);


void image_from_framebuffer(image_t* image,framebuffer_t* framebuffer,palette_t* palette);
void image_save_bmp(image_t* image,palette_t* palette,char* filename);
void image_destroy(image_t* image);



//void render_save_bmp(render_t* render,char* filename);
#endif // RENDERER_H_INCLUDED
