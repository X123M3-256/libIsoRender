#include<stdio.h>
#include<stdlib.h>
#include<math.h>
#include<string.h>
#include<stdbool.h>
#include<assert.h>
#include<errno.h>
#include "renderer.h"
#include "palette.h"
#include "vectormath.h"
#include "model.h"

//3.67 metres per tile
#define SQRT_2 1.4142135623731
#define SQRT1_2 0.707106781
#define SQRT_3 1.73205080757
#define SQRT_6 2.44948974278



void texture_init(texture_t* texture,uint16_t width,uint16_t height)
	{
	texture->width=width;
	texture->height=height;
	texture->pixels=malloc(width*height*sizeof(color_t));
	}

float wrap_coord(float coord)
{
return fmax(0.0,fmin(1.0,coord-floor(coord)));
}

vector3_t texture_sample(texture_t* texture,vector2_t coord)
	{
	uint16_t tex_x=(uint32_t)(texture->width*wrap_coord(coord.x));
	uint16_t tex_y=(uint32_t)(texture->height*wrap_coord(coord.y));
	assert(tex_x<texture->width&&tex_y<texture->height);
	return texture->pixels[tex_y*texture->width+tex_x];
	}

void texture_destroy(texture_t* texture)
	{
	free(texture->pixels);
	}


void context_init(context_t* context,light_t* lights,uint32_t num_lights,palette_t palette,float upt)
	{
	context->lights=lights;
	context->num_lights=num_lights;
	//Dimetric projection
	const transform_t projection={
	{32.0/upt		,0.0	   		,-32.0/upt,
	 -16.0/upt	,-16.0*SQRT_6/upt	,-16.0/upt,
	 16.0*SQRT_3/upt	,-16.0*SQRT_2/upt	,16.0*SQRT_3/upt},
	 {0,0,0}
	 };
	context->projection=projection;
	context->view_vector=vector3_normalize(matrix_vector(matrix_inverse(projection.matrix),vector3(0,0,-1)));
	context->palette=palette;
	}
void context_rotate(context_t* context)
{
matrix_t rotation=matrix(-context->projection.matrix.entries[2],context->projection.matrix.entries[1],context->projection.matrix.entries[0],-context->projection.matrix.entries[5],context->projection.matrix.entries[4],context->projection.matrix.entries[3],-context->projection.matrix.entries[8],context->projection.matrix.entries[7],context->projection.matrix.entries[6]);
context->projection=transform(rotation,context->projection.translation);
context->view_vector=vector3(-context->view_vector.z,context->view_vector.y,context->view_vector.x);
	for(int i=0;i<context->num_lights;i++)
	{
	context->lights[i].direction=vector3(-context->lights[i].direction.z,context->lights[i].direction.y,context->lights[i].direction.x);
	}
}


int32_t int_wrap(int32_t x,int32_t wrap)
{
	if(x>=0)return x-(wrap*(x/wrap));
	else return x+(wrap*(1+x/wrap));
}
int32_t clamp(int32_t x,int32_t a,int32_t b)
	{
		if(x<a)return a;
		else if(x>b)return b;
		else return x;
	}
bool int_in_range(int32_t x,int32_t a,int32_t b)
	{
		if(b>=a&&x>=a&&x<b)return true;
		else if(b<a&&x<=a&&x>b)return true;
		else return false;
	}

//Specular shading code from Blender. Not sure what it does
float spec(float inp, int hard)  
	{
		if (inp>=1.0f) return 1.0f;
		else if (inp<=0.0f) return 0.0f;
        
        float b1=inp*inp;
        	if (b1<0.01f) b1= 0.01f;
        
		if ((hard & 1)==0)  inp= 1.0f;
		if (hard&2)  inp*= b1;
        b1*= b1;
		if (hard&4)  inp*= b1;
        b1*= b1;
		if (hard&8)  inp*= b1;
        b1*= b1;
		if (hard&16) inp*= b1;
        b1*= b1;

		if (b1<0.001f) b1= 0.0f;

		if (hard&32) inp*= b1;
        b1*= b1;
		if (hard&64) inp*=b1;
        b1*= b1;
		if (hard&128) inp*=b1;

		if (b1<0.001f) b1= 0.0f;

        	if (hard & 256)
		{
                b1*= b1;
                inp*=b1;
        	}
        return inp;
	}

float cook_torr_spec(vector3_t n,vector3_t l,vector3_t v,int hard)
	{
        vector3_t h=vector3_normalize(vector3_add(v,l));

        float nh=vector3_dot(n,h);
        	if(nh<0.0f) return 0.0f;
        float nv=vector3_dot(n,v);
        	if (nv<0.0f) nv= 0.0f;

        return spec(nh, hard)/(0.1f+nv);
	}


vector3_t shade_fragment(light_t* lights,uint32_t num_lights,vector3_t normal,vector3_t view,vector3_t color,vector3_t specular_color,uint32_t specular_hardness)
	{
	vector3_t output_color=vector3(0,0,0);
		for(uint32_t i=0;i<num_lights;i++)
		{
			if(lights[i].type==LIGHT_HEMI)
			{
			float diffuse_factor=0.5*lights[i].intensity*(1+vector3_dot(normal,lights[i].direction));
			output_color=vector3_add(vector3_mult(color,diffuse_factor),output_color);
			}
			else if(lights[i].type==LIGHT_DIFFUSE)
			{
			float diffuse_factor=lights[i].intensity*fmax(vector3_dot(normal,lights[i].direction),0.0);
			output_color=vector3_add(vector3_mult(color,diffuse_factor),output_color);
			}
			else
			{
			float diffuse_factor=lights[i].intensity*fmax(vector3_dot(normal,lights[i].direction),0.0);
			//vector3_t reflected_light_direction=vector3_sub(vector3_mult(normal,2.0*vector3_dot(lights[i].direction,normal)),lights[i].direction);
			
			float specular_factor=lights[i].intensity*cook_torr_spec(normal,lights[i].direction,view,specular_hardness);
			output_color=vector3_add(vector3_add(vector3_mult(color,diffuse_factor),vector3_mult(specular_color,specular_factor)),output_color);	
			}	
		}
	return output_color; 
	}
/*
void context_draw_primitive(context_t* context,primitive_t primitive)
	{
	transform_t view=context->projection;
	vector3_t tv[3]={transform_vector(view,primitive.vertices[0]),transform_vector(view,primitive.vertices[1]),transform_vector(view,primitive.vertices[2])};
	
	int32_t x_min=clamp((int32_t)(floor(fmin(fmin(tv[0].x,tv[1].x),tv[2].x))+floor(context->width/2.0)),0,context->width-1);
	int32_t x_max=clamp((int32_t)(ceil(fmax(fmax(tv[0].x,tv[1].x),tv[2].x))+ceil(context->width/2.0)),0,context->width-1);
	int32_t y_min=clamp((int32_t)(floor(fmin(fmin(tv[0].y,tv[1].y),tv[2].y))+floor(context->height/2.0)),0,context->height-1);
	int32_t y_max=clamp((int32_t)(ceil(fmax(fmax(tv[0].y,tv[1].y),tv[2].y))+ceil(context->height/2.0)),0,context->height-1);
		for(int32_t y=y_min;y<=y_max;y++)
		for(int32_t x=x_min;x<=x_max;x++)
		{
		//Do not factorize
	float point_x=(float)x-((float)context->width+1.0)/2.0;
	float point_y=(float)y-((float)context->height+1.0)/2.0;
	float denominator=(tv[1].y-tv[2].y)*(tv[0].x-tv[2].x)+(tv[2].x-tv[1].x)*(tv[0].y-tv[2].y);
	float a=((tv[1].y-tv[2].y)*(point_x-tv[2].x)+(tv[2].x-tv[1].x)*(point_y-tv[2].y))/denominator;
	float b=((tv[2].y-tv[0].y)*(point_x-tv[2].x)+(tv[0].x-tv[2].x)*(point_y-tv[2].y))/denominator;
	float c=1.0-a-b;
		if(a>=0.0&&b>=0.0&&c>=0.0&&a<=1.0&&b<=1.0&&c<=1.0)
		{
		float depth=a*tv[0].z+b*tv[1].z+c*tv[2].z;
			if(depth<context->fragments[x+y*context->width].depth)
			{
			//Interpolate normal
			vector3_t normal=vector3_normalize(vector3_add(vector3_add(vector3_mult(primitive.normals[0],a),vector3_mult(primitive.normals[1],b)),vector3_mult(primitive.normals[2],c)));
			//Interpolate UV
			vector2_t tex_coord;
			tex_coord.x=primitive.uvs[0].x*a+primitive.uvs[1].x*b+primitive.uvs[2].x*c;
			tex_coord.y=primitive.uvs[0].y*a+primitive.uvs[1].y*b+primitive.uvs[2].y*c;

			vector3_t color;
				if(primitive.material->flags&MATERIAL_HAS_TEXTURE)color=texture_sample(&(primitive.material->texture),tex_coord);
				else color=primitive.material->color;
			context->fragments[x+y*context->width].color=shade_fragment(&(context->light),normal,color);
			context->fragments[x+y*context->width].depth=depth;
			context->fragments[x+y*context->width].region=primitive.material->region;
			}
		}
	}
}

void context_destroy(context_t* context)
	{
	free(context->fragments);
	}


bool context_compare_regions(context_t* context,uint32_t x1,uint32_t y1,uint32_t x2,uint32_t y2)
{
return context_index(context,x1,y1).region==context_index(context,x2,y2).region;
}
*/


//Given Cartesian coordinates of a point and a reference triangle, compute the barycentric coordinates
vector3_t compute_barycentric_coordinates(vector2_t point,vector3_t* tri_points)
{
float denominator=(tri_points[1].y-tri_points[2].y)*(tri_points[0].x-tri_points[2].x)+(tri_points[2].x-tri_points[1].x)*(tri_points[0].y-tri_points[2].y);
float a=((tri_points[1].y-tri_points[2].y)*(point.x-tri_points[2].x)+(tri_points[2].x-tri_points[1].x)*(point.y-tri_points[2].y))/denominator;	
float b=((tri_points[2].y-tri_points[0].y)*(point.x-tri_points[2].x)+(tri_points[0].x-tri_points[2].x)*(point.y-tri_points[2].y))/denominator;
float c=1.0-a-b;
return vector3(a,b,c);
}

//Given the barycentric coordinates of a point, test if the point is inside the triangle
int internal_point(vector3_t coords)
{
return coords.x>=0.0&&coords.y>=0.0&&coords.z>=0.0&&coords.x<=1.0&&coords.y<=1.0&&coords.z<=1.0;
}


void transform_primitives_generic(transform_t transform,primitive_t* primitives,uint32_t num_primitives,uint32_t preserve_normals)
{
matrix_t normal_matrix=matrix_transpose(matrix_inverse(transform.matrix));
	for(int i=0;i<num_primitives;i++)
	for(int j=0;j<3;j++)
	{
	primitives[i].vertices[j]=transform_vector(transform,primitives[i].vertices[j]);
	}
	if(!preserve_normals)
	{
		for(int i=0;i<num_primitives;i++)
		for(int j=0;j<3;j++)
		{
		primitives[i].normals[j]=matrix_vector(normal_matrix,primitives[i].normals[j]);
		}
	}
}

void transform_primitives(transform_t transform,primitive_t* primitives,uint32_t num_primitives)
{
transform_primitives_generic(transform,primitives,num_primitives,0);
}

void project_primitives(transform_t transform,primitive_t* primitives,uint32_t num_primitives)
{
transform_primitives_generic(transform,primitives,num_primitives,1);
}

//Returns the index of the frontmost primitive intersecting the line given by X=x,Y=y, or -1 if no such primitive was found.
int primitive_sample_point(primitive_t* primitives,uint32_t num_primitives,transform_t transform,vector2_t point,vector3_t* result_weights)
{
int primitive_index=-1;
float current_depth;
	for(uint32_t i=0;i<num_primitives;i++)
	{
	primitive_t primitive=primitives[i];
	vector3_t weights=compute_barycentric_coordinates(point,primitive.vertices);
		if(internal_point(weights))
		{
		float depth=vector3_dot(weights,vector3(primitive.vertices[0].z,primitive.vertices[1].z,primitive.vertices[2].z));
			if(primitive_index==-1||depth<current_depth)
			{
			primitive_index=i;
			*result_weights=weights;
			current_depth=depth;	
			}
		}

	}
return primitive_index;
}
int context_sample_point(context_t* context,primitive_t* primitives,uint32_t num_primitives,vector2_t point,fragment_t* fragment)
{
vector3_t weights;
int primitive_index=primitive_sample_point(primitives,num_primitives,context->projection,point,&weights);
	if(primitive_index==-1)return 0;
primitive_t primitive=primitives[primitive_index];

	if(primitive.material)
	{
	//Compute normal vector
	vector3_t normal=vector3_normalize(vector3_add(vector3_add(vector3_mult(primitive.normals[0],weights.x),vector3_mult(primitive.normals[1],weights.y)),vector3_mult(primitive.normals[2],weights.z)));
	//Compute surface color
	vector3_t color;
		if(primitive.material->flags&MATERIAL_HAS_TEXTURE)
		{
		vector2_t tex_coord=vector2_add(vector2_add(vector2_mult(primitive.uvs[0],weights.x),vector2_mult(primitive.uvs[1],weights.y)),vector2_mult(primitive.uvs[2],weights.z));
		color=texture_sample(&(primitive.material->texture),tex_coord);
		}
		else color=primitive.material->color;

	fragment->color=shade_fragment(context->lights,context->num_lights,normal,context->view_vector,color,primitive.material->specular_color,primitive.material->specular_hardness);
	fragment->region=primitive.material->region;
	}
	else
	{
	fragment->color=vector3(0.0,0.0,0.0);
	fragment->region=FRAGMENT_UNUSED;
	}
return 1;
}




rect_t rect(int xl,int xu,int yl,int yu)
{
rect_t result={xl,yl,xu,yu};
return result;
}
rect_t rect_enclose_point(rect_t r,float x,float y)
{
return rect((int)fmin(r.x_lower,floor(x)),(int)fmax(r.x_upper,ceil(x)),
	    (int)fmin(r.y_lower,floor(y)),(int)fmax(r.y_upper,ceil(y)));
}

rect_t primitives_get_bounds(primitive_t* primitives,uint32_t num_primitives)
{
if(num_primitives==0)return rect(0,1,0,1);
rect_t bounds=rect((int)floor(primitives[0].vertices[0].x),(int)ceil(primitives[0].vertices[0].x),(int)floor(primitives[0].vertices[0].y),(int)ceil(primitives[0].vertices[0].y)
);
	for(int i=0;i<num_primitives;i++)
	for(int j=0;j<3;j++)
	{
	bounds=rect_enclose_point(bounds,primitives[i].vertices[j].x,primitives[i].vertices[j].y);
	}
return bounds;
}


#define FRAMEBUFFER_INDEX(fbf,x,y) (framebuffer->fragments[(x)+(y)*framebuffer->width])


rect_t framebuffer_get_bounds(framebuffer_t* framebuffer)
{
int found_pixel=0;
rect_t bounds;
	for(uint32_t y=0;y<framebuffer->height;y++)
	for(uint32_t x=0;x<framebuffer->width;x++)
	{
		if(FRAMEBUFFER_INDEX(framebuffer,x,y).region!=FRAGMENT_UNUSED)
		{
			if(found_pixel)bounds=rect_enclose_point(bounds,x,y);
			else
			{
			bounds=rect(x,x+1,y,y+1);
			found_pixel=1;
			}
		}
	}
	//If the image is empty, just set the size as 1 pixel
	if(!found_pixel)return rect(0,0,0,0);
	else return bounds;
}

void framebuffer_from_primitives(framebuffer_t* framebuffer,context_t* context,primitive_t* primitives,uint32_t num_primitives)
{
rect_t bounds=primitives_get_bounds(primitives,num_primitives);
framebuffer->width=bounds.x_upper-bounds.x_lower+1;
framebuffer->height=bounds.y_upper-bounds.y_lower;
framebuffer->offset=vector2((float)(bounds.x_lower)-0.5,(float)(bounds.y_lower));
framebuffer->fragments=malloc(framebuffer->width*framebuffer->height*sizeof(fragment_t));
	for(int i=0;i<framebuffer->width*framebuffer->height;i++)
	{
	framebuffer->fragments[i].color=vector3(0.0,0.0,0.0);
	framebuffer->fragments[i].region=FRAGMENT_UNUSED;
	}



	for(int y=0;y<framebuffer->height;y++)
	for(int x=0;x<framebuffer->width;x++)
	{
	vector2_t sample_point=vector2_add(vector2(x,y),framebuffer->offset);
	fragment_t centre_sample;
		if(context_sample_point(context,primitives,num_primitives,sample_point,&centre_sample))
		{
		vector2_t subsample_points[4]={{-0.25,-0.25},{0.25,-0.25},{-0.25,0.25},{0.25,0.25}};
		vector3_t subsample_total=vector3(0.0,0.0,0.0);
		float num_subsamples=0.0;
			for(int i=0;i<4;i++)
			{
			fragment_t subsample;
				if(context_sample_point(context,primitives,num_primitives,vector2_add(sample_point,subsample_points[i]),&subsample)&&subsample.region==centre_sample.region)
				{
				subsample_total=vector3_add(subsample_total,subsample.color);
				num_subsamples+=1.0;
				}
			}
		framebuffer->fragments[x+y*framebuffer->width].color=num_subsamples!=0.0?vector3_mult(subsample_total,1.0/num_subsamples):centre_sample.color;
		framebuffer->fragments[x+y*framebuffer->width].region=centre_sample.region;
		}
	}
}


void framebuffer_destroy(framebuffer_t* framebuffer)
{
free(framebuffer->fragments);
}

void framebuffer_save_bmp(framebuffer_t* framebuffer,char* filename)
{
int padding=(4-(framebuffer->width*3)%4)%4;
int data_size=framebuffer->height*(framebuffer->width*3+padding);

uint8_t bitmap_header[54];
memset(bitmap_header,0,54);

bitmap_header[0]='B';
bitmap_header[1]='M';
*((uint32_t*)(bitmap_header+2))=54+data_size;
*((uint32_t*)(bitmap_header+10))=54;
*((uint32_t*)(bitmap_header+14))=40;
*((uint32_t*)(bitmap_header+18))=framebuffer->width;
*((uint32_t*)(bitmap_header+22))=framebuffer->height;
*((uint16_t*)(bitmap_header+26))=1;
*((uint16_t*)(bitmap_header+28))=24;
*((uint32_t*)(bitmap_header+38))=2834;
*((uint32_t*)(bitmap_header+42))=2834;

FILE* file=fopen(filename,"wb");
	if(file==NULL)
	{
	printf("File open failed %d\n",errno);
	return;
	}

fwrite(bitmap_header,1,54,file);

	for(int32_t y=framebuffer->height-1;y>=0;y--)
	{
		for(uint32_t x=0;x<framebuffer->width;x++)
		{
		color_t color=color_from_vector(framebuffer->fragments[x+y*framebuffer->width].color);
		fputc(color.b,file);
		fputc(color.g,file);
		fputc(color.r,file);
		}
		for(uint32_t k=0;k<padding;k++)fputc(0,file);
	}

fclose(file);
}
void image_from_framebuffer(image_t* image,framebuffer_t* framebuffer,palette_t* palette)
{
rect_t bounding_box=framebuffer_get_bounds(framebuffer);
image->width=1+bounding_box.x_upper-bounding_box.x_lower;
image->height=1+bounding_box.y_upper-bounding_box.y_lower;
image->x_offset=bounding_box.x_lower+floor(framebuffer->offset.x);
image->y_offset=bounding_box.y_lower+floor(framebuffer->offset.y)-1;//1 compensates for error not sure why it's needed TODO work out why it's needed
image->pixels=calloc(image->width*image->height,sizeof(uint8_t));

	for(int y=bounding_box.y_lower;y<=bounding_box.y_upper;y++)
	{
		for(int x=bounding_box.x_lower;x<=bounding_box.x_upper;x++)
		{
		fragment_t fragment=FRAMEBUFFER_INDEX(framebuffer,x,y);
			if(fragment.region!=FRAGMENT_UNUSED)
			{
			vector3_t error;
			image->pixels[(x-bounding_box.x_lower)+(y-bounding_box.y_lower)*image->width]=palette_get_nearest(palette,fragment.region&REGION_MASK,fragment.color,&error);
						
			//Distribute error onto neighbouring points
			//int points[4][2]={{x+1,y},{x-1,y+1},{x,y+1},{x+1,y+1}};
			//float weights[4]={7.0/16.0,3.0/16.0,5.0/16.0,1.0/16.0};
			//	for(int i=0;i<4;i++)
			//	if(points[i][0]>=0&&points[i][0]<framebuffer->width-1&&points[i][1]>=0&&points[i][1]<framebuffer->height-1&&
			//	   FRAMEBUFFER_INDEX(framebuffer,x,y).region==FRAMEBUFFER_INDEX(framebuffer,points[i][0],points[i][1]).region)
			//	{		
			//	FRAMEBUFFER_INDEX(framebuffer,points[i][0],points[i][1]).color=vector3_add(vector3_mult(error,weights[i]),FRAMEBUFFER_INDEX(framebuffer,points[i][0],points[i][1]).color);
			//	}
			}
		}
	}
free(framebuffer->fragments);
}

void image_save_bmp(image_t* image,palette_t* palette,char* filename)
{
int padding=(4-(image->width*3)%4)%4;

int data_size=image->height*(image->width*3+padding);

uint8_t bitmap_header[54];
memset(bitmap_header,0,54);

bitmap_header[0]='B';
bitmap_header[1]='M';
*((uint32_t*)(bitmap_header+2))=54+data_size;
*((uint32_t*)(bitmap_header+10))=54;
*((uint32_t*)(bitmap_header+14))=40;
*((uint32_t*)(bitmap_header+18))=image->width;
*((uint32_t*)(bitmap_header+22))=image->height;
*((uint16_t*)(bitmap_header+26))=1;
*((uint16_t*)(bitmap_header+28))=24;
*((uint32_t*)(bitmap_header+38))=2834;
*((uint32_t*)(bitmap_header+42))=2834;


FILE* file=fopen(filename,"wb");
	if(file==NULL)
	{
	printf("File open failed %d\n",errno);
	return;
	}

fwrite(bitmap_header,1,54,file);

	for(int32_t y=image->height-1;y>=0;y--)
	{
		for(uint32_t x=0;x<image->width;x++)
		{
		color_t color=palette->colors[image->pixels[y*image->width+x]];
		fputc(color.b,file);
		fputc(color.g,file);
		fputc(color.r,file);
		}
		for(uint32_t k=0;k<padding;k++)fputc(0,file);
	}

fclose(file);
}

