#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "model.h"
#include "renderer.h"
#include "palette.h"
 

int main()
{
//{LIGHT_DIFFUSE,vector3(0.65190909243,0.38733586252,-0.65190909243),0.9}
//light_t light={vector3_normalize(vector3(1.0,-atan(3.141592658979/9.0),0.0)),0.5,1.0};
light_t lights[6]={{LIGHT_HEMI,vector3(0.0,1.0,0.0),0.176},{LIGHT_DIFFUSE,vector3(0.671641,0.38733586252,-0.631561),0.9},{LIGHT_SPECULAR,vector3(0.671641,0.38733586252,-0.631561),0.3},{LIGHT_DIFFUSE,vector3(0.259037,0.03967,-0.965052),0.4},{LIGHT_DIFFUSE,vector3(-0.904066,0.412439,-0.112069),0.15},{LIGHT_DIFFUSE,vector3(0.50718,0.517623,0.689083),0.1}};
context_t context;
context_init(&context,lights,6,palette_rct2(),(1.5*sqrt(6))/1.1);

/*
material_t materials[2];
materials[0].flags=MATERIAL_HAS_TEXTURE;
materials[0].region=0;
materials[0].specular_intensity=0.0;
materials[0].specular_exponent=0.0;
texture_load_png(&(materials[0].texture),"test.png");
materials[1].flags=0;
materials[1].region=1;
materials[1].specular_intensity=0.0;
materials[1].specular_exponent=0.0;
materials[1].color.x=75.0/255.0;
materials[1].color.y=99.0/255.0;
materials[1].color.z=99.0/255.0;




vector3_t vertices[8]={{0,0,0},{1.0,0,0},{0,0,1.0},{1.0,0,1.0},{0,(1.0/sqrt(6)),0},{1.0,(1.0/sqrt(6)),0},{0,(1.0/sqrt(6)),1.0},{1.0,(1.0/sqrt(6)),1.0}};
vector3_t normals[6]={{1.0,0,0},{-1.0,0,0},{0,1.0,0},{0,-1.0,0},{0,0,1.0},{0,0,-1.0}};
vector2_t uvs[4]={{0,0},{1.0,0},{0,1.0},{1.0,1.0}};
face_t faces[12]={
	{{4,5,6},{2,2,2},{0,1,2},0},//Top
	{{7,6,5},{2,2,2},{3,2,1},0},

	{{0,1,2},{3,3,3},{0,1,2},0},//Bottom
	{{3,2,1},{3,3,3},{3,2,1},0},

	{{0,1,4},{5,5,5},{0,1,2},0},//Front
	{{5,4,1},{5,5,5},{3,2,1},0},

	{{2,3,6},{4,4,4},{0,1,2},0},//Back
	{{7,6,3},{4,4,4},{3,2,1},0},

	{{0,2,4},{1,1,1},{0,1,2},0},//Left
	{{6,4,2},{1,1,1},{3,2,1},0},

	{{1,3,5},{0,0,0},{0,1,2},0},//Right
	{{7,5,3},{0,0,0},{3,2,1},0},

};

mesh_t model;
model.vertices=vertices;
model.normals=normals;
model.uvs=uvs;
model.faces=faces;
model.materials=materials;
model.num_vertices=8;
model.num_normals=6;
model.num_faces=12;
model.num_materials=2;
*/
mesh_t mesh;
	if(mesh_load_obj(&mesh,"bmdive.obj"))
	{
	printf("Failed loading object\n");
	return 0;
	}

int num_primitives=mesh_count_primitives(&mesh);
primitive_t* primitives=malloc(sizeof(primitive_t)*num_primitives);
mesh_get_primitives(&mesh,primitives);

project_primitives(context.projection,primitives,num_primitives);
framebuffer_t framebuffer;
framebuffer_from_primitives(&framebuffer,&context,primitives,num_primitives);
framebuffer_save_bmp(&framebuffer,"test.bmp");
free(primitives);
//image_t image;
//image_from_framebuffer(&image,&framebuffer,&(context.palette));
//image_save_bmp(&image,&(context.palette),"test.bmp");

//image_t image;
//context_get_image(&context,&image);
//image_save_bmp(&image,&(context.palette),"test.bmp");

//context_destroy(&context);

return 0;
}





