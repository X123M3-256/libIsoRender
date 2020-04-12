#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "model.h"
#include "renderer.h"

int main()
{
context_t context;
//vector3(0.65190909243,0.38733586252+0.2,-0.65190909243)
light_t lights[9]={
{LIGHT_DIFFUSE,0,vector3_normalize(vector3(0.0,-1.0,0.0)),0.25},
{LIGHT_DIFFUSE,0,vector3_normalize(vector3(1.0,0.3,0.0)),0.32},
{LIGHT_SPECULAR,0,vector3_normalize(vector3(1,1,-1)),1.0},
{LIGHT_DIFFUSE,0,vector3_normalize(vector3(1,0.65,-1)),0.8},
{LIGHT_DIFFUSE,0,vector3(0.0,1.0,0.0),0.174},
{LIGHT_DIFFUSE,0,vector3_normalize(vector3(-1.0,0.0,0.0)),0.15},
{LIGHT_DIFFUSE,0,vector3_normalize(vector3(0.0,1.0,1.0)),0.2},
{LIGHT_DIFFUSE,0,vector3_normalize(vector3(0.65,0.816,-0.65000000)),0.25},
{LIGHT_DIFFUSE,0,vector3_normalize(vector3(-1.0,0.0,-1.0)),0.25},
};
context_init(&context,lights,9,palette_rct2(),1);
/*
vector3_t normals[9]={
vector3(-0.707107,0.000000,-0.707107),
vector3(0.000000,0.000000,-1.000000),
vector3(-1.000000,0.000000,0.000000),
vector3(0.000000,1.000000,0.000000),
vector3(-0.378002,0.925805,0.000000),
vector3(0.000000,0.925805,-0.378002),
vector3(0.000000,0.925805,0.378002),
vector3(0.378002,0.925805,0.000000),
vector3(-0.852817,0.522210,0.000000)
};

puts("A=np.array([");
for(uint32_t j=0;j<9;j++)
{
putchar('[');
	vector3_t normal=normals[j];
	for(uint32_t i=0;i<8;i++)
		{
			if(lights[i].type==LIGHT_HEMI)
			{
			printf("%f",i,0.5*(1+vector3_dot(normal,lights[i].direction)));
			}
			else if(lights[i].type==LIGHT_DIFFUSE)
			{
			printf("%f",i,fmax(vector3_dot(normal,lights[i].direction),0.0));
			}
			else
			{
			printf("%f",i,fmax(vector3_dot(normal,lights[i].direction),0.0));
			}
			if(i!=7)putchar(',');
		}
putchar(']');
	if(j!=8)putchar(',');
putchar('\n');	
}
putchar(']');
putchar(')');
putchar('\n');	
*/


mesh_t mesh;
	if(mesh_load(&mesh,"support.obj"))
	{
	printf("Failed loading object\n");
	return 0;
	}

context_begin_render(&context);
context_add_model(&context,&mesh,transform(views[0],vector3(0,0,0)),0);
context_finalize_render(&context);
image_t image;

context_render_view(&context,views[0],&image);
context_end_render(&context);


FILE* f=fopen("test.png","w");
image_write_png(&image,f);
fclose(f);

image_destroy(&image);
context_destroy(&context);

return 0;
}





