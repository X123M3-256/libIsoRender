#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include "image.h"
#include "sort_r.h"

typedef struct
{
int x;
int y;
int width;
int height;
}rect_t;

void insert_rect(rect_t* rects,int num_rects,int i,int x,int y,int width,int height)
{
memmove(rects+i+2,rects+i+1,(num_rects-i-1)*sizeof(rect_t));
rects[i+1].x=x;
rects[i+1].y=y;
rects[i+1].width=width;
rects[i+1].height=height;
}

void delete_rect(rect_t* rects,int num_rects,int i)
{
memmove(rects+i,rects+i+1,(num_rects-i-1)*sizeof(rect_t));
}

int print_rects(rect_t* rects,int num_rects)
{
	for(int i=0;i<num_rects;i++)
	{
	printf("\tx %d y %d w %d h %d\n",rects[i].x,rects[i].y,rects[i].width,rects[i].height);
	}
}

int cmp_width(const void* a,const void* b,void* arg)
{
image_t* images=(image_t*)arg;
int h1=images[*((int*)a)].width;
int h2=images[*((int*)b)].width;
	if(h1==h2)return 0;
	else if(h1<h2)return 1;
	else return -1;
}

int cmp_height(const void* a,const void* b,void* arg)
{
image_t* images=(image_t*)arg;
int h1=images[*((int*)a)].height;
int h2=images[*((int*)b)].height;
	if(h1==h2)return 0;
	else if(h1<h2)return 1;
	else return -1;
}

int cmp_area(const void* a,const void* b,void* arg)
{
image_t* images=(image_t*)arg;
int p1=images[*((int*)a)].width*images[*((int*)a)].height;
int p2=images[*((int*)b)].width*images[*((int*)b)].height;
	if(p1==p2)return 0;
	else if(p1<p2)return 1;
	else return -1;
}


int cmp_perimeter(const void* a,const void* b,void* arg)
{
image_t* images=(image_t*)arg;
int p1=images[*((int*)a)].width+images[*((int*)a)].height;
int p2=images[*((int*)b)].width+images[*((int*)b)].height;
	if(p1==p2)return 0;
	else if(p1<p2)return 1;
	else return -1;
}

int cmp_max(const void* a,const void* b,void* arg)
{
image_t* images=(image_t*)arg;
int p1=fmax(images[*((int*)a)].width,images[*((int*)a)].height);
int p2=fmax(images[*((int*)b)].width,images[*((int*)b)].height);
	if(p1==p2)return 0;
	else if(p1<p2)return 1;
	else return -1;
}

int pack_rects_fixed_with_comparator(image_t* images,int num_images,int width,int height,int* x_coords,int* y_coords,int (*compare)(const void*,const void*, void*))
{
//printf("Size %d %d\n",width,height);
int* permutation=calloc(num_images,sizeof(int));
	for(int i=0;i<num_images;i++)permutation[i]=i;
sort_r(permutation,num_images,sizeof(int),compare,images);

int max_empty_spaces=10000;
int num_empty_spaces=1;
rect_t* empty_spaces=calloc(max_empty_spaces,sizeof(rect_t));
empty_spaces[0].x=0;
empty_spaces[0].y=0;
empty_spaces[0].width=width;
empty_spaces[0].height=height;

int i;
	for(i=0;i<num_images;i++)
	{
	image_t* image=images+permutation[i];
	//printf("Image w %d h %d\n",image->width,image->height);
	//printf("Free spaces:\n");
	//print_rects(empty_spaces,num_empty_spaces);
	int j;
	int num_created_rects=-1;
	rect_t created_rects[2];
		for(j=num_empty_spaces-1;j>=0;j--)
		{
		rect_t space=empty_spaces[j];
		x_coords[permutation[i]]=space.x;
		y_coords[permutation[i]]=space.y;
			if(space.width>image->width&&space.height>image->height)
			{
			num_created_rects=2;
				if(space.height-image->height<space.width-image->width)
				{
				//Split horizontally
				//printf("Splitting horizontally\n");
				created_rects[0].x=empty_spaces[j].x;	
				created_rects[0].y=empty_spaces[j].y+image->height;
				created_rects[0].width=empty_spaces[j].width;
				created_rects[0].height=empty_spaces[j].height-image->height;
				created_rects[1].x=empty_spaces[j].x+image->width;
				created_rects[1].y=empty_spaces[j].y;
				created_rects[1].width=empty_spaces[j].width-image->width;
				created_rects[1].height=image->height;
				}
				else
				{
				//Split vertically
				//printf("Splitting vertically\n");
				created_rects[0].x=empty_spaces[j].x+image->width;	
				created_rects[0].y=empty_spaces[j].y;
				created_rects[0].width=empty_spaces[j].width-image->width;
				created_rects[0].height=empty_spaces[j].height;
				created_rects[1].x=empty_spaces[j].x;
				created_rects[1].y=empty_spaces[j].y+image->height;
				created_rects[1].width=image->width;
				created_rects[1].height=empty_spaces[j].height-image->height;
				}
			break;
			}
			else if(space.width==image->width&&space.height>image->height)
			{
			num_created_rects=1;
			created_rects[0].x=empty_spaces[j].x;
			created_rects[0].y=empty_spaces[j].y+image->height;
			created_rects[0].width=empty_spaces[j].width;
			created_rects[0].height=empty_spaces[j].height-image->height;
			break;
			}
			else if(space.height==image->height&&space.width>image->width)
			{
			num_created_rects=1;
			created_rects[0].x=empty_spaces[j].x+image->width;	
			created_rects[0].y=empty_spaces[j].y;
			created_rects[0].width=empty_spaces[j].width-image->width;
			created_rects[0].height=empty_spaces[j].height;
			break;
			}
			else if(space.width==image->width&&space.height==image->height)
			{
			num_created_rects=0;
			break;
			}
		}
		if(num_created_rects<0)
		{
//		printf("Insertion failed at %d\n",i);
		free(permutation);
		free(empty_spaces);
		return 0;
		}

	//Delete used space
	memmove(empty_spaces+j,empty_spaces+j+1,(num_empty_spaces-j-1)*sizeof(rect_t));
	//Append newly created rects to end
	memcpy(empty_spaces+num_empty_spaces-1,created_rects,num_created_rects*sizeof(rect_t));
	num_empty_spaces+=num_created_rects-1;	
	}

//printf("Free spaces:\n");
//print_rects(empty_spaces,num_empty_spaces);

free(permutation);
free(empty_spaces);
return 1;
}


int pack_rects_fixed(image_t* images,int num_images,int width,int height,int* x_coords,int* y_coords)
{
//TODO Area almost always works best - it might not be worth checking the others
	if(pack_rects_fixed_with_comparator(images,num_images,width,height,x_coords,y_coords,cmp_area))return 1;
	if(pack_rects_fixed_with_comparator(images,num_images,width,height,x_coords,y_coords,cmp_perimeter))return 1;
	if(pack_rects_fixed_with_comparator(images,num_images,width,height,x_coords,y_coords,cmp_max))return 1;
	if(pack_rects_fixed_with_comparator(images,num_images,width,height,x_coords,y_coords,cmp_width))return 1;
	if(pack_rects_fixed_with_comparator(images,num_images,width,height,x_coords,y_coords,cmp_height))return 1;
return 0;
}

int pack_rects(image_t* images,int num_images,int* width_ptr,int* height_ptr,int* x_coords,int* y_coords)
{
int size=256;
	while(!pack_rects_fixed(images,num_images,size,size,x_coords,y_coords))size*=2;


//Use binary search to find smallest square that can fit the images
int lower_size=size/2;
int upper_size=size;
	while(upper_size-lower_size>2)
	{
	int mid_size=(upper_size+lower_size)/2;
		if(pack_rects_fixed(images,num_images,mid_size,mid_size,x_coords,y_coords))upper_size=mid_size;
		else lower_size=mid_size;
	}

//Use binary search to determine how much the height can be reduced
int upper_height=upper_size;
int lower_height=0;
	while(upper_height-lower_height>2)
	{
	int mid_height=(upper_height+lower_height)/2;
		if(pack_rects_fixed(images,num_images,upper_size,mid_height,x_coords,y_coords))upper_height=mid_height;
		else lower_height=mid_height;
	}

//Use binary search to determine how much the width can be reduced
int upper_width=upper_size;
int lower_width=0;
	while(upper_width-lower_width>2)
	{
	int mid_width=(upper_width+lower_width)/2;
		if(pack_rects_fixed(images,num_images,mid_width,upper_height,x_coords,y_coords))upper_width=mid_width;
		else lower_width=mid_width;
	}


//Check which resulted in smaller area
int width,height;
	if(upper_width<upper_height)
	{
	width=upper_width;
	height=upper_size;
	}
	else
	{
	width=upper_size;
	height=upper_height;
	}

assert(pack_rects_fixed(images,num_images,width,height,x_coords,y_coords));
*width_ptr=width;
*height_ptr=height;
}

void image_create_atlas(image_t* output,image_t* images,int num_images,int* x_coords,int* y_coords)
{
int width,height;
pack_rects(images,num_images,&width,&height,x_coords,y_coords);

image_new(output,width,height,0,0,0);
memset(output->pixels,0,width*height);
int used_pixels=0;
	for(int i=0;i<num_images;i++)
	{
	image_blit(output,images+i,x_coords[i]-images[i].x_offset,y_coords[i]-images[i].y_offset);
	used_pixels+=images[i].width*images[i].height;
	}

printf("Width %d Height %d\n",width,height);
printf("Packing efficiency %.1f%\n",(100.0*used_pixels)/(width*height));
}
