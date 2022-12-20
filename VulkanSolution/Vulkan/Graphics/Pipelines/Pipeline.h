/******************************************************************************
Copyright (C) 2022 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior
written consent of DigiPen Institute of Technology is prohibited.
File Name:   Texture.h
Author
	- sinil.kang	rtd99062@gmail.com
Creation Date: 12.19.2022
	Header file for Texture.
******************************************************************************/

#include <Engines/Objects/Object.h>
#include <vulkan/vulkan.h>
#include <vector>

class Graphics;

class Pipeline : public Object
{
public:
	Pipeline(VkDevice device, VkRenderPass renderPass, std::string pipelineName, const std::string& vertShader, const std::string& fragShader, VkDescriptorSetLayout* descriptorSetLayoutPtr, VkPrimitiveTopology primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	// Push Constant compatible pipeline
	Pipeline(VkDevice device, VkRenderPass renderPass, std::string pipelineName, const std::string& vertShader, const std::string& fragShader, uint32_t pushConstantSize, VkShaderStageFlags pushConstantTargetStage, VkDescriptorSetLayout* descriptorSetLayoutPtr, VkPrimitiveTopology primitiveTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, bool depthTestWrite = VK_TRUE);
	~Pipeline();

	bool Init();
	void Update(float dt);
	void Clean();
private:
	static std::vector<char> readFile(const std::string& filename);
	VkShaderModule CreateShaderModule(const std::vector<char>& code);

private:
	VkDevice device;
	VkPipelineLayout pipelineLayout;
	VkPipeline pipeline;
};