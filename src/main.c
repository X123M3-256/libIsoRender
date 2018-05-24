#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "model.h"
#include "renderer.h"
#include "palette.h"
 

texture_t texture_load_bmp(const char* filename)
{
FILE* file=fopen(filename,"rb");
    if(file==NULL)exit(1337);
fseek(file,0,SEEK_END);
int length=ftell(file);
fseek(file,0,SEEK_SET);
uint8_t* data=malloc(length);
fread(data,1,length,file);
fclose(file);

uint8_t* pixels=data+*((uint32_t*)(data+10));
uint32_t width=*((uint32_t*)(data+18));
uint32_t height=*((uint32_t*)(data+22));
uint32_t stride=width*3+((4-(width*3)%4)%4);

texture_t texture;
texture.width=width;
texture.height=height;
texture.pixels=malloc(sizeof(vector3_t)*width*height);

    for(uint32_t y=0;y<height;y++)
    {
        for(uint32_t x=0;x<width;x++)
        {
        texture.pixels[x+y*width]=vector_from_color(color(pixels[3*x+y*stride+2],pixels[3*x+y*stride+1],pixels[3*x+y*stride]));
        }
    }
free(data);
return texture;
}


int main()
{
light_t light={vector3_normalize(vector3(1.0,-atan(3.141592658979/9.0),0.0)),0.5,1.0};
context_t context;
context_init(&context,light,palette_rct2(),1.0);

material_t materials[2];
materials[0].flags=MATERIAL_HAS_TEXTURE;
materials[0].region=0;
materials[0].specular_intensity=0.0;
materials[0].specular_exponent=0.0;
materials[0].texture=texture_load_bmp("calcitetop.bmp");
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


primitive_t primitives[12];
mesh_get_primitives(&model,primitives);

primitives[0].material=NULL;
primitives[1].material=NULL;

transform_primitives(context.projection,primitives,12);
framebuffer_t framebuffer;
framebuffer_from_primitives(&framebuffer,&context,primitives,12);
//framebuffer_save_bmp(&framebuffer,"test.bmp");
image_t image;
image_from_framebuffer(&image,&framebuffer,&(context.palette));
image_save_bmp(&image,&(context.palette),"test.bmp");

//image_t image;
//context_get_image(&context,&image);
//image_save_bmp(&image,&(context.palette),"test.bmp");
//context_destroy(&context);
return 0;
}





