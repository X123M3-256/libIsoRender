#define NOMINMAX
#define _USE_MATH_DEFINES
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <embree3/rtcore.h>
#include "raytrace.h"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void rt_error(void* user_ptr,enum RTCError error,const char* str)
	{
	printf("error %d: %s\n", error, str);
	exit(1);
	}

device_t device_init()
{
device_t device = rtcNewDevice(NULL);
	if (!device)
    	{
	printf("error %d: cannot create device\n", rtcGetDeviceError(NULL));
	exit(1);
	}
rtcSetDeviceErrorFunction(device,rt_error,NULL);
return device;
}

void device_destroy(device_t device)
	{
	rtcReleaseDevice(device);
	}

void scene_init(scene_t* scene,device_t device)
{
scene->num_meshes=0;
scene->mask=0;
scene->embree_device=device;
scene->embree_scene=rtcNewScene(device);
scene->x_max=-INFINITY;
scene->y_max=-INFINITY;
scene->z_max=-INFINITY;
scene->x_min=INFINITY;
scene->y_min=INFINITY;
scene->z_min=INFINITY;
}

void scene_finalize(scene_t* scene)
{
rtcCommitScene(scene->embree_scene);
}

void scene_destroy(scene_t* scene)
{
rtcReleaseScene(scene->embree_scene);
}

float min(float x,float y)
{
return x<=y?x:y;
}

float max(float x,float y)
{
return x>=y?x:y;
}


void scene_add_model(scene_t* scene,mesh_t* mesh,vertex_t (*transform)(vector3_t,vector3_t,void*),void* data,int mask)
	{
	//Add mesh to list of meshes
	assert(scene->num_meshes<MAX_MESHES);
	scene->meshes[scene->num_meshes]=mesh;
		if(mask)scene->mask|=((uint64_t)1)<<scene->num_meshes;
	scene->num_meshes++;
	//Create Embree geometry
	RTCGeometry geom=rtcNewGeometry(scene->embree_device,RTC_GEOMETRY_TYPE_TRIANGLE);
	rtcSetGeometryVertexAttributeCount(geom,1);
	float* vertices=(float*)rtcSetNewGeometryBuffer(geom,RTC_BUFFER_TYPE_VERTEX,0,RTC_FORMAT_FLOAT3,3*sizeof(float),mesh->num_vertices);
	float* normals=(float*)rtcSetNewGeometryBuffer(geom,RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE,0,RTC_FORMAT_FLOAT3,3*sizeof(float),mesh->num_vertices);
;	
	unsigned int* indices=(unsigned int*) rtcSetNewGeometryBuffer(geom,RTC_BUFFER_TYPE_INDEX,0,RTC_FORMAT_UINT3,3*sizeof(unsigned int),mesh->num_faces);
		if(!(vertices&&indices))
		{
		printf("Failed allocating geometry buffer\n");
		}

		for(uint32_t i=0;i<mesh->num_vertices;i++)
		{
		vertex_t transformed_vertex=transform(mesh->vertices[i],mesh->normals[i],data);
		vertices[3*i+0]=transformed_vertex.vertex.x;
		vertices[3*i+1]=transformed_vertex.vertex.y;
		vertices[3*i+2]=transformed_vertex.vertex.z;
		normals[3*i+0]=transformed_vertex.normal.x;
		normals[3*i+1]=transformed_vertex.normal.y;
		normals[3*i+2]=transformed_vertex.normal.z;
		scene->x_max=max(scene->x_max,transformed_vertex.vertex.x);
		scene->y_max=max(scene->y_max,transformed_vertex.vertex.y);
		scene->z_max=max(scene->z_max,transformed_vertex.vertex.z);
		scene->x_min=min(scene->x_min,transformed_vertex.vertex.x);
		scene->y_min=min(scene->y_min,transformed_vertex.vertex.y);
		scene->z_min=min(scene->z_min,transformed_vertex.vertex.z);
		}

	
	
		for(uint32_t i=0;i<mesh->num_faces;i++)
		{
		indices[3*i+0]=mesh->faces[i].indices[0];
		indices[3*i+1]=mesh->faces[i].indices[1];
		indices[3*i+2]=mesh->faces[i].indices[2];
		}
	rtcCommitGeometry(geom);
	//Add geometry to scene
	rtcAttachGeometry(scene->embree_scene,geom);
	rtcReleaseGeometry(geom);
	}


int scene_trace_ray(scene_t* scene,vector3_t origin,vector3_t direction,ray_hit_t* hit)
{
struct RTCIntersectContext context;
rtcInitIntersectContext(&context);

struct RTCRayHit rayhit;
rayhit.ray.org_x=origin.x;
rayhit.ray.org_y=origin.y;
rayhit.ray.org_z=origin.z;
rayhit.ray.dir_x=direction.x;
rayhit.ray.dir_y=direction.y;
rayhit.ray.dir_z=direction.z;
rayhit.ray.tnear=0;
rayhit.ray.tfar=INFINITY;
rayhit.ray.mask=0;
rayhit.ray.flags=0;
rayhit.hit.geomID=RTC_INVALID_GEOMETRY_ID;
rayhit.hit.instID[0]=RTC_INVALID_GEOMETRY_ID;

rtcIntersect1(scene->embree_scene,&context,&rayhit);

	if(rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID)
	{
	hit->mesh_index=rayhit.hit.geomID;
	hit->face_index=rayhit.hit.primID;
	hit->u=rayhit.hit.u;
	hit->v=rayhit.hit.v;
	//Interpolate normal
	float position_components[3];
	float normal_components[3];
	rtcInterpolate0(rtcGetGeometry(scene->embree_scene,rayhit.hit.geomID),rayhit.hit.primID,rayhit.hit.u,rayhit.hit.v,RTC_BUFFER_TYPE_VERTEX,0,position_components,3);
	rtcInterpolate0(rtcGetGeometry(scene->embree_scene,rayhit.hit.geomID),rayhit.hit.primID,rayhit.hit.u,rayhit.hit.v,RTC_BUFFER_TYPE_VERTEX_ATTRIBUTE,0,normal_components,3);
	hit->position=vector3(position_components[0],position_components[1],position_components[2]);
	hit->normal=vector3_normalize(vector3(normal_components[0],normal_components[1],normal_components[2]));
	hit->distance=rayhit.ray.tfar;
	return 1;
	}
return 0;
}

int scene_trace_occlusion_ray(scene_t* scene,vector3_t origin,vector3_t direction)
{
struct RTCIntersectContext context;
rtcInitIntersectContext(&context);

struct RTCRay ray;
ray.org_x=origin.x;
ray.org_y=origin.y;
ray.org_z=origin.z;
ray.dir_x=direction.x;
ray.dir_y=direction.y;
ray.dir_z=direction.z;
ray.tnear=1e-5;
ray.tfar=INFINITY;
ray.mask=0;
ray.flags=0;

rtcOccluded1(scene->embree_scene,&context,&ray);

return ray.tfar<=0.0;
}


