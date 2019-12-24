#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "model.h"
#include "renderer.h"

int main()
{
context_t context;

light_t lights[6]={{LIGHT_HEMI,vector3(0.0,1.0,0.0),0.176},{LIGHT_DIFFUSE,vector3(0.671641,0.38733586252,-0.631561),0.9},{LIGHT_SPECULAR,vector3(0.671641,0.38733586252,-0.631561),0.3},{LIGHT_DIFFUSE,vector3(0.259037,0.03967,-0.965052),0.4},{LIGHT_DIFFUSE,vector3(-0.904066,0.412439,-0.112069),0.15},{LIGHT_DIFFUSE,vector3(0.50718,0.517623,0.689083),0.1}};
context_init(&context,lights,6,palette_rct2(),1);

mesh_t mesh;
	if(mesh_load(&mesh,"test.obj"))
	{
	printf("Failed loading object\n");
	return 0;
	}

context_begin_render(&context);
context_add_model(&context,&mesh,transform(views[0],vector3(0,0,0)));
context_finalize_render(&context);
image_t image;

context_render_view(&context,views[2],&image);
context_end_render(&context);


FILE* f=fopen("test.png","w");
image_write_png(&image,f);
fclose(f);

image_destroy(&image);
context_destroy(&context);

return 0;
}





