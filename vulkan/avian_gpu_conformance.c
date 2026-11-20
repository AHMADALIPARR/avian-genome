/*
 * Vulkan 1.1 compute conformance runner, with no other language runtime.
 * Contract of the SPIR-V entry point `main`:
 *   set=0 binding=0 storage buffer INPUT  of N float32 elements;
 *   set=0 binding=1 storage buffer OUTPUT of N float32 elements;
 *   local_size_x = 64; shader checks gl_GlobalInvocationID.x < N.
 * Run: avian_gpu_conformance shader.spv input.f32 expected.f32
 * All files contain IEEE-754 little-endian float32; outputs compared bitwise.
 * This is not suitable for shaders with incompatible descriptor layouts.
 */
#include <vulkan/vulkan.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#define CHECK(call) do { VkResult rc_ = (call); if (rc_ != VK_SUCCESS) { \
 fprintf(stderr,"Vulkan error %d at line %d\n",(int)rc_,__LINE__); goto cleanup; } } while (0)

static void *load_file(const char *path, size_t *length) {
 FILE *f=fopen(path,"rb"); if (!f) return NULL;
 if (fseek(f,0,SEEK_END)!=0) { fclose(f); return NULL; }
 long s=ftell(f); if (s<=0 || fseek(f,0,SEEK_SET)!=0) { fclose(f); return NULL; }
 void *data=malloc((size_t)s); if (!data) { fclose(f); return NULL; }
 if (fread(data,1,(size_t)s,f)!=(size_t)s) { free(data); fclose(f); return NULL; }
 fclose(f); *length=(size_t)s; return data;
}
static int memory_type(VkPhysicalDevice gpu, uint32_t bits, uint32_t *index) {
 VkPhysicalDeviceMemoryProperties props; vkGetPhysicalDeviceMemoryProperties(gpu,&props);
 for (uint32_t i=0;i<props.memoryTypeCount;i++)
  if ((bits & (1u<<i)) && (props.memoryTypes[i].propertyFlags &
      (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) ==
      (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT|VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) { *index=i; return 1; }
 return 0;
}
static int buffer_make(VkDevice dev,VkPhysicalDevice gpu,VkDeviceSize bytes,
                       VkBuffer *buffer,VkDeviceMemory *memory) {
 VkBufferCreateInfo b={.sType=VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
  .size=bytes,.usage=VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,.sharingMode=VK_SHARING_MODE_EXCLUSIVE};
 if(vkCreateBuffer(dev,&b,NULL,buffer)!=VK_SUCCESS)return 0;
 VkMemoryRequirements req; vkGetBufferMemoryRequirements(dev,*buffer,&req);
 uint32_t type; if(!memory_type(gpu,req.memoryTypeBits,&type))return 0;
 VkMemoryAllocateInfo a={.sType=VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
  .allocationSize=req.size,.memoryTypeIndex=type};
 if(vkAllocateMemory(dev,&a,NULL,memory)!=VK_SUCCESS)return 0;
 return vkBindBufferMemory(dev,*buffer,*memory,0)==VK_SUCCESS;
}
int main(int argc,char **argv) {
 if(argc!=4) { fprintf(stderr,"usage: %s shader.spv input.f32 expected.f32\n",argv[0]); return 2; }
 int ret=1;
 size_t shader_bytes=0,input_bytes=0,expected_bytes=0;
 void *shader=load_file(argv[1],&shader_bytes),*input=load_file(argv[2],&input_bytes),
      *expected=load_file(argv[3],&expected_bytes);
 VkInstance instance=VK_NULL_HANDLE; VkDevice dev=VK_NULL_HANDLE;
 VkBuffer buffers[2]={VK_NULL_HANDLE,VK_NULL_HANDLE};
 VkDeviceMemory memories[2]={VK_NULL_HANDLE,VK_NULL_HANDLE};
 VkDescriptorSetLayout layout=VK_NULL_HANDLE;
 VkDescriptorPool pool=VK_NULL_HANDLE; VkPipelineLayout pipeline_layout=VK_NULL_HANDLE;
 VkPipeline pipeline=VK_NULL_HANDLE; VkShaderModule module=VK_NULL_HANDLE;
 VkCommandPool cmdpool=VK_NULL_HANDLE; VkFence fence=VK_NULL_HANDLE;
 if(!shader||!input||!expected||shader_bytes%4!=0||shader_bytes<20||input_bytes!=expected_bytes||
   input_bytes%4!=0||input_bytes==0||input_bytes/4>UINT32_MAX) {
   fprintf(stderr,"invalid SPIR-V or mismatched inputs\n"); goto cleanup;
 }
 uint32_t n=(uint32_t)(input_bytes/4);
 VkApplicationInfo app={.sType=VK_STRUCTURE_TYPE_APPLICATION_INFO,.apiVersion=VK_API_VERSION_1_1};
 VkInstanceCreateInfo ici={.sType=VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,.pApplicationInfo=&app};
 CHECK(vkCreateInstance(&ici,NULL,&instance));
 uint32_t count=0; CHECK(vkEnumeratePhysicalDevices(instance,&count,NULL));
 if(!count){fprintf(stderr,"no Vulkan GPU\n");goto cleanup;}
 VkPhysicalDevice *gpus=calloc(count,sizeof(*gpus)); if(!gpus)goto cleanup;
 VkResult enumerate=vkEnumeratePhysicalDevices(instance,&count,gpus);
 if(enumerate!=VK_SUCCESS){free(gpus);goto cleanup;}
 VkPhysicalDevice gpu=VK_NULL_HANDLE;uint32_t queue_family=UINT32_MAX;
 for(uint32_t g=0;g<count && gpu==VK_NULL_HANDLE;g++){
  uint32_t qc=0;vkGetPhysicalDeviceQueueFamilyProperties(gpus[g],&qc,NULL);
  VkQueueFamilyProperties *qp=calloc(qc,sizeof(*qp));if(!qp)continue;
  vkGetPhysicalDeviceQueueFamilyProperties(gpus[g],&qc,qp);
  for(uint32_t q=0;q<qc;q++)if(qp[q].queueFlags&VK_QUEUE_COMPUTE_BIT){gpu=gpus[g];queue_family=q;break;}
  free(qp);
 }
 free(gpus);
 if(gpu==VK_NULL_HANDLE){fprintf(stderr,"no compute queue\n");goto cleanup;}
 float qprio=1.0f;
 VkDeviceQueueCreateInfo qci={.sType=VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
  .queueFamilyIndex=queue_family,.queueCount=1,.pQueuePriorities=&qprio};
 VkDeviceCreateInfo dci={.sType=VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,.queueCreateInfoCount=1,.pQueueCreateInfos=&qci};
 CHECK(vkCreateDevice(gpu,&dci,NULL,&dev));
 if(!buffer_make(dev,gpu,input_bytes,&buffers[0],&memories[0])||
    !buffer_make(dev,gpu,input_bytes,&buffers[1],&memories[1])) goto cleanup;
 void *mapped=NULL;
 CHECK(vkMapMemory(dev,memories[0],0,input_bytes,0,&mapped));
 memcpy(mapped,input,input_bytes); vkUnmapMemory(dev,memories[0]);
 VkDescriptorSetLayoutBinding binds[2]={
  {.binding=0,.descriptorType=VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,.descriptorCount=1,.stageFlags=VK_SHADER_STAGE_COMPUTE_BIT},
  {.binding=1,.descriptorType=VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,.descriptorCount=1,.stageFlags=VK_SHADER_STAGE_COMPUTE_BIT}};
 VkDescriptorSetLayoutCreateInfo lci={.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
  .bindingCount=2,.pBindings=binds};
 CHECK(vkCreateDescriptorSetLayout(dev,&lci,NULL,&layout));
 VkDescriptorPoolSize psz={.type=VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,.descriptorCount=2};
 VkDescriptorPoolCreateInfo pci={.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
  .maxSets=1,.poolSizeCount=1,.pPoolSizes=&psz};
 CHECK(vkCreateDescriptorPool(dev,&pci,NULL,&pool));
 VkDescriptorSetAllocateInfo dai={.sType=VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
  .descriptorPool=pool,.descriptorSetCount=1,.pSetLayouts=&layout};
 VkDescriptorSet set; CHECK(vkAllocateDescriptorSets(dev,&dai,&set));
 VkDescriptorBufferInfo bis[2]={{.buffer=buffers[0],.offset=0,.range=input_bytes},
  {.buffer=buffers[1],.offset=0,.range=input_bytes}};
 VkWriteDescriptorSet writes[2]={
  {.sType=VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,.dstSet=set,.dstBinding=0,
   .descriptorCount=1,.descriptorType=VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,.pBufferInfo=&bis[0]},
  {.sType=VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,.dstSet=set,.dstBinding=1,
   .descriptorCount=1,.descriptorType=VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,.pBufferInfo=&bis[1]}};
 vkUpdateDescriptorSets(dev,2,writes,0,NULL);
 VkShaderModuleCreateInfo smci={.sType=VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
  .codeSize=shader_bytes,.pCode=(const uint32_t*)shader};
 CHECK(vkCreateShaderModule(dev,&smci,NULL,&module));
 VkPipelineLayoutCreateInfo plci={.sType=VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
  .setLayoutCount=1,.pSetLayouts=&layout};
 CHECK(vkCreatePipelineLayout(dev,&plci,NULL,&pipeline_layout));
 VkPipelineShaderStageCreateInfo stage={.sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
  .stage=VK_SHADER_STAGE_COMPUTE_BIT,.module=module,.pName="main"};
 VkComputePipelineCreateInfo cpci={.sType=VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
  .stage=stage,.layout=pipeline_layout};
 CHECK(vkCreateComputePipelines(dev,VK_NULL_HANDLE,1,&cpci,NULL,&pipeline));
 VkCommandPoolCreateInfo cpoci={.sType=VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
  .queueFamilyIndex=queue_family};
 CHECK(vkCreateCommandPool(dev,&cpoci,NULL,&cmdpool));
 VkCommandBufferAllocateInfo cbai={.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
  .commandPool=cmdpool,.level=VK_COMMAND_BUFFER_LEVEL_PRIMARY,.commandBufferCount=1};
 VkCommandBuffer cmd; CHECK(vkAllocateCommandBuffers(dev,&cbai,&cmd));
 VkCommandBufferBeginInfo begin={.sType=VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
 CHECK(vkBeginCommandBuffer(cmd,&begin));
 vkCmdBindPipeline(cmd,VK_PIPELINE_BIND_POINT_COMPUTE,pipeline);
 vkCmdBindDescriptorSets(cmd,VK_PIPELINE_BIND_POINT_COMPUTE,pipeline_layout,0,1,&set,0,NULL);
 vkCmdDispatch(cmd,(n+63u)/64u,1,1);
 VkMemoryBarrier barrier={.sType=VK_STRUCTURE_TYPE_MEMORY_BARRIER,
 .srcAccessMask=VK_ACCESS_SHADER_WRITE_BIT,.dstAccessMask=VK_ACCESS_HOST_READ_BIT};
 vkCmdPipelineBarrier(cmd,VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,VK_PIPELINE_STAGE_HOST_BIT,
 0,1,&barrier,0,NULL,0,NULL);
 CHECK(vkEndCommandBuffer(cmd));
 VkQueue queue;vkGetDeviceQueue(dev,queue_family,0,&queue);
 VkFenceCreateInfo fci={.sType=VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
 CHECK(vkCreateFence(dev,&fci,NULL,&fence));
 VkSubmitInfo submit={.sType=VK_STRUCTURE_TYPE_SUBMIT_INFO,.commandBufferCount=1,.pCommandBuffers=&cmd};
 CHECK(vkQueueSubmit(queue,1,&submit,fence));
 CHECK(vkWaitForFences(dev,1,&fence,VK_TRUE,UINT64_MAX));
 CHECK(vkMapMemory(dev,memories[1],0,input_bytes,0,&mapped));
 size_t bad=0;uint32_t got=0,want=0;for(uint32_t i=0;i<n;i++){
  memcpy(&got,(char*)mapped+4u*i,4);memcpy(&want,(char*)expected+4u*i,4);
  if(got!=want){if(bad<8)fprintf(stderr,"element %u expected 0x%08x got 0x%08x\n",i,want,got);bad++;}
 }vkUnmapMemory(dev,memories[1]);
 printf("elements=%u mismatches=%zu\n",n,bad);ret=bad?1:0;
cleanup:
 if(dev){if(fence)vkDestroyFence(dev,fence,NULL);
  if(cmdpool)vkDestroyCommandPool(dev,cmdpool,NULL);
  if(pipeline)vkDestroyPipeline(dev,pipeline,NULL);
  if(module)vkDestroyShaderModule(dev,module,NULL);
  if(pool)vkDestroyDescriptorPool(dev,pool,NULL);
  if(pipeline_layout)vkDestroyPipelineLayout(dev,pipeline_layout,NULL);
  if(layout)vkDestroyDescriptorSetLayout(dev,layout,NULL);
  for(int i=0;i<2;i++){if(buffers[i])vkDestroyBuffer(dev,buffers[i],NULL);
   if(memories[i])vkFreeMemory(dev,memories[i],NULL);}
  vkDestroyDevice(dev,NULL);}
 if(instance)vkDestroyInstance(instance,NULL);
 free(shader);free(input);free(expected);return ret;
}
