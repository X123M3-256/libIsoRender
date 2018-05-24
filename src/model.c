#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <png.h>
#include "objLoader/obj_parser.h"
#include "model.h"

#define DEBUG



int texture_load_png(texture_t* texture,const char* filename)
{
FILE *fp = fopen(filename, "rb");
	if(!fp)
	{
	return 1;
	}
png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png)
	{
	fclose(fp);
	return 1;
	}
png_infop info = png_create_info_struct(png);
	if(!info)
	{
	fclose(fp);
	return 1;
	}	
	if(setjmp(png_jmpbuf(png))) abort();//TODO Not sure what this does but I don't think it's what I want

png_init_io(png,fp);
png_read_info(png,info);

texture->width=png_get_image_width(png,info);
texture->height=png_get_image_height(png,info);

png_byte color_type=png_get_color_type(png,info);
png_byte bit_depth=png_get_bit_depth(png,info);
	if(bit_depth==16)png_set_strip_16(png);
	if(color_type==PNG_COLOR_TYPE_PALETTE)png_set_palette_to_rgb(png);

// PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
	if(color_type==PNG_COLOR_TYPE_GRAY&&bit_depth<8)png_set_expand_gray_1_2_4_to_8(png);
	if(png_get_valid(png, info, PNG_INFO_tRNS))png_set_tRNS_to_alpha(png);

// These color_type don't have an alpha channel then fill it with 0xff
	if(color_type==PNG_COLOR_TYPE_RGB||color_type==PNG_COLOR_TYPE_GRAY||color_type==PNG_COLOR_TYPE_PALETTE)png_set_filler(png, 0xFF, PNG_FILLER_AFTER);
	if(color_type == PNG_COLOR_TYPE_GRAY||color_type == PNG_COLOR_TYPE_GRAY_ALPHA)png_set_gray_to_rgb(png);

png_read_update_info(png, info);

png_bytep* row_pointers=malloc(sizeof(png_bytep)*texture->height);
	for(int y=0;y<texture->height;y++)
	{
	row_pointers[y]=malloc(sizeof(png_byte)*png_get_rowbytes(png,info));
	}

png_read_image(png,row_pointers);

texture->pixels=malloc(texture->width*texture->height*sizeof(vector3_t));

	for(int y=0;y<texture->height;y++)
	for(int x=0;x<texture->width;x++)
	{
	color_t color={(uint8_t)row_pointers[y][4*x],(uint8_t)row_pointers[y][4*x+1],(uint8_t)row_pointers[y][4*x+2]};
	texture->pixels[x+y*texture->width]=vector_from_color(color);
	}
	for(int y=0;y<texture->height;y++)
	{
	free(row_pointers[y]);
	}
free(row_pointers);
fclose(fp);
return 0;
}

int mesh_load_obj(mesh_t* mesh,const char* filename)
{
obj_scene_data obj_data;
	if (!parse_obj_scene(&obj_data, filename))
	{
	return 1;
	}
	
// Count vertices
mesh->num_vertices = obj_data.vertex_count;
mesh->num_normals = obj_data.vertex_normal_count;
mesh->num_faces=obj_data.face_count;
mesh->num_uvs=obj_data.vertex_texture_count;
mesh->num_materials=obj_data.material_count;

// Allocate arrays
mesh->vertices = malloc(mesh->num_vertices*sizeof(vector3_t));
mesh->normals = malloc(mesh->num_normals*sizeof(vector3_t));
mesh->uvs=malloc(mesh->num_uvs*sizeof(vector2_t));
mesh->faces = malloc(mesh->num_faces*sizeof(face_t));
mesh->materials=malloc(mesh->num_materials*sizeof(material_t));

	//Load vertices
	for (int i=0;i<mesh->num_vertices;i++)
	{
	mesh->vertices[i].x=obj_data.vertex_list[i]->e[0];
	mesh->vertices[i].y=obj_data.vertex_list[i]->e[1];
	mesh->vertices[i].z=-obj_data.vertex_list[i]->e[2];
	}
	//Load normals
	for (int i=0;i<mesh->num_normals;i++)
	{
	mesh->normals[i].x=obj_data.vertex_normal_list[i]->e[0];
	mesh->normals[i].y=obj_data.vertex_normal_list[i]->e[1];
	mesh->normals[i].z=-obj_data.vertex_normal_list[i]->e[2];
	}
	//Load uvs
	for (int i=0;i<mesh->num_uvs;i++)
	{
	mesh->uvs[i].x=obj_data.vertex_texture_list[i]->e[0];
	mesh->uvs[i].y=obj_data.vertex_texture_list[i]->e[1];
	}
	//Load faces
	for (int i=0;i<mesh->num_faces;i++) 
	{
	mesh->faces[i].material= obj_data.face_list[i]->material_index;
		for (int j=0;j<3;j++) 
		{
		mesh->faces[i].vertices[j] = obj_data.face_list[i]->vertex_index[j];
		mesh->faces[i].normals[j] = obj_data.face_list[i]->normal_index[j];
		mesh->faces[i].uvs[j] = obj_data.face_list[i]->texture_index[j];
		} 
	}
	//Load materials
	for(int i=0;i<mesh->num_materials;i++)
	{
	mesh->materials[i].flags=0;
	mesh->materials[i].region=0;
		if(strstr(obj_data.material_list[i]->name,"Remap1")!=NULL)
		{
		mesh->materials[i].flags|=MATERIAL_IS_REMAPPABLE;
		mesh->materials[i].region=1;
		}
		else if(strstr(obj_data.material_list[i]->name,"Remap2")!=NULL)
		{
		mesh->materials[i].flags|=MATERIAL_IS_REMAPPABLE;
		mesh->materials[i].region=2;
		}
		if(obj_data.material_list[i]->texture_filename[0]==0)
		{
		mesh->materials[i].color=vector3(obj_data.material_list[i]->diff[0],obj_data.material_list[i]->diff[1],obj_data.material_list[i]->diff[2]);
		}
		else
		{
		mesh->materials[i].flags|=MATERIAL_HAS_TEXTURE;
			if(texture_load_png(&(mesh->materials[i].texture),obj_data.material_list[i]->texture_filename))
			{
			printf("Failed to load texture \"%s\"\n",obj_data.material_list[i]->texture_filename);
			free(mesh->vertices);
			free(mesh->normals);
			free(mesh->faces);
			free(mesh->materials);
			//TODO free any textures already loaded	
			delete_obj_data(&obj_data);
			return 1;
			}
		}
	mesh->materials[i].specular_intensity=vector3_norm(vector3(obj_data.material_list[i]->spec[0],obj_data.material_list[i]->spec[1],obj_data.material_list[i]->spec[2]));
	mesh->materials[i].specular_exponent=obj_data.material_list[i]->shiny;
	}
delete_obj_data(&obj_data);
return 0;
}

void mesh_check_invariants(mesh_t* mesh)
{
//Check faces
	for(int i=0;i<mesh->num_faces;i++)
	{
	face_t* face=mesh->faces+i;
		for(int j=0;j<3;j++)
		{
		assert(face->vertices[j]>=0&&face->vertices[j]<=mesh->num_vertices);
		assert(face->normals[j]>=0&&face->normals[j]<=mesh->num_normals);
		assert(face->uvs[j]>=0&&face->uvs[j]<=mesh->num_uvs);
		}
	}
	for(int i=0;i<mesh->num_materials;i++)
	{
	assert((~(mesh->materials[i].flags&MATERIAL_HAS_TEXTURE))||(mesh->materials[i].texture.pixels!=NULL));
	}
}

uint32_t mesh_count_primitives(mesh_t* mesh)
{
return mesh->num_faces;
}

primitive_t mesh_get_primitives(mesh_t* mesh,primitive_t* primitives)
{
	for(int i=0;i<mesh->num_faces;i++)
	{
	face_t* face=mesh->faces+i;
		for(int j=0;j<3;j++)
		{
		primitives[i].vertices[j]=mesh->vertices[face->vertices[j]];
		primitives[i].normals[j]=mesh->normals[face->normals[j]];
			if(mesh->materials[face->material].flags&MATERIAL_HAS_TEXTURE)primitives[i].uvs[j]=mesh->uvs[face->uvs[j]];
		}
	primitives[i].material=mesh->materials+face->material;
	}
}
/*
void mesh_render(mesh_t* mesh,context_t* context)
{
	#ifdef DEBUG
	mesh_check_invariants(mesh);
	#endif

	for(int i=0;i<mesh->num_faces;i++)
	{
	context_draw_primitive(context,mesh_get_primitive(mesh,i));
	}
}
*/
void mesh_destroy(mesh_t* mesh)
{

}
