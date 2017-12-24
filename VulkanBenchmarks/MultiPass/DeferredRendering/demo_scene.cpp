#include <logger.h>
#include <vulkan_infrastructure_context.h>
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <vulkan_shader.h>
#include <mutex>
#include <algorithm>
#include "demo_scene.h"
#include "vulkan_application.h"
#include "imgui_impl_glfw_vulkan.h"
#include "imgui.h"
#include <functional>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>


// Vulkan clip space has inverted Y and half Z.
static const Mat4f s_ClipCorrectionMat{
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, -1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 0.5f, 0.0f,
	0.0f, 0.0f, 0.5f, 1.0f
};

static std::mt19937 s_Rng;

static auto RealRangeRng(const f32 begin, const f32 end)
{
	std::uniform_real_distribution<f32> distribution{ begin, end };
	return distribution(s_Rng);
}

// Private functions -------------------------------------------------

//Model loading --------------------------------------
static Vec3f AssVector(const aiVector3D vec) noexcept
{
	return Vec3f{ vec.x, vec.y, vec.z };
}

static Quatf AssQuat(const aiQuaternion q) noexcept
{
	return Quatf{ q.w, q.x, q.y, q.z };
}

void DemoScene::LoadMeshes(const aiScene* scene) noexcept
{
	for (auto i = 0; i < scene->mNumMeshes; ++i) {
		auto mesh = new VulkanMesh;

		auto* aiMesh{ scene->mMeshes[i] };

		if (aiMesh->HasPositions()) {
			std::vector<Vertex> vertices;
			vertices.resize(aiMesh->mNumVertices);

			for (auto j = 0; j < vertices.size(); ++j) {
				vertices[j].position = Vec3f{ aiMesh->mVertices[j].x, aiMesh->mVertices[j].y, aiMesh->mVertices[j].z };
				vertices[j].normal = Vec3f{ aiMesh->mNormals[j].x, aiMesh->mNormals[j].y, aiMesh->mNormals[j].z };
				vertices[j].tangent = Vec3f{ aiMesh->mTangents[j].x, aiMesh->mTangents[j].y, aiMesh->mTangents[j].z };
				vertices[j].texcoord = Vec2f{ aiMesh->mTextureCoords[0][j].x, aiMesh->mTextureCoords[0][j].y };
			}

			mesh->AddVertices(vertices);
		}
		else {
			ERROR_LOG("Mesh has no vertices!");
			delete mesh;
			return;
		}

		if (aiMesh->HasFaces()) {
			std::vector<unsigned int> indices;

			for (auto k = 0; k < aiMesh->mNumFaces; ++k) {
				const auto& face = aiMesh->mFaces[k];
				assert(face.mNumIndices == 3);
				indices.push_back(face.mIndices[0]);
				indices.push_back(face.mIndices[1]);
				indices.push_back(face.mIndices[2]);
			}

			mesh->AddIndices(indices);
		}

		mesh->CreateBuffers();

		mesh->SetMaterialIndex(aiMesh->mMaterialIndex);

		m_Meshes.push_back(mesh);
	}
}

void DemoScene::LoadMaterials(const aiScene* scene) noexcept
{
	DemoMaterial material;
	material.diffuse = Vec4f{ 1.0f };
	material.specular = Vec4f{ 1.0f };

	auto GetFileName = [](std::string path) -> std::string
	{
		auto sepUnix = '/';

		auto sepWindows = '\\';

		auto n = path.rfind(sepUnix);

		if (n == std::string::npos) {
			n = path.rfind(sepWindows);
		}

		if (n != std::string::npos) {
			return path.substr(n + 1);
		}

		return "";
	};

	for (auto i = 0; i < scene->mNumMaterials; ++i) {
		const auto aiMaterial = scene->mMaterials[i];

		aiString path;
		aiGetMaterialTexture(aiMaterial, aiTextureType_DIFFUSE, 0, &path);

		material.textures[TEX_DIFFUSE] = G_ResourceManager.Get<VulkanTexture>("textures/" + GetFileName(path.data),
		                                                                      TEX_DIFFUSE,
		                                                                      VK_FORMAT_R8G8B8A8_UNORM,
		                                                                      VK_IMAGE_ASPECT_COLOR_BIT);

		aiMaterial->GetTexture(aiTextureType_SPECULAR, 0, &path);

		material.textures[TEX_SPECULAR] = G_ResourceManager.Get<VulkanTexture>("textures/" + GetFileName(path.data),
		                                                                       TEX_SPECULAR,
		                                                                       VK_FORMAT_R8G8B8A8_UNORM,
		                                                                       VK_IMAGE_ASPECT_COLOR_BIT);

		aiMaterial->GetTexture(aiTextureType_NORMALS, 0, &path);

		material.textures[TEX_NORMAL] = G_ResourceManager.Get<VulkanTexture>("textures/" + GetFileName(path.data),
		                                                                     TEX_NORMAL,
		                                                                     VK_FORMAT_R8G8B8A8_UNORM,
		                                                                     VK_IMAGE_ASPECT_COLOR_BIT);

		m_Materials.push_back(material);
	}
}

DemoEntity* DemoScene::LoadNode(const aiNode* aiNode) noexcept
{
	const std::string name = aiNode->mName.data;

	auto entity = new DemoEntity;
	entity->SetName(name);

	if (aiNode->mNumMeshes > 0) {
		const auto mesh = m_Meshes[aiNode->mMeshes[0]];

		entity->SetMesh(mesh);
		entity->SetMaterial(&m_Materials[mesh->GetMaterialIndex()]);
	}

	aiVector3D aiPosition;
	aiQuaternion aiOrientation;
	aiVector3D aiScaling;

	aiNode->mTransformation.Decompose(aiScaling, aiOrientation, aiPosition);

	entity->SetPosition(AssVector(aiPosition));
	entity->SetOrientation(AssQuat(aiOrientation));
	entity->SetScale(AssVector(aiScaling));


	// recursion for all the children
	for (int i = 0; i < aiNode->mNumChildren; ++i) {
		Entity* child{ LoadNode(aiNode->mChildren[i]) };

		entity->AddChild(child);
	}

	return entity;
}

std::unique_ptr<DemoEntity> DemoScene::LoadModel(const std::string& fileName) noexcept
{
	auto result = std::make_unique<DemoEntity>();

	const auto scn = aiImportFile(fileName.c_str(),
	                              aiProcess_GenSmoothNormals |
	                              aiProcess_FixInfacingNormals |
	                              aiProcess_Triangulate |
	                              aiProcess_CalcTangentSpace | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices |
	                              aiProcess_FixInfacingNormals | aiProcess_SortByPType);

	if (!scn) {
		ERROR_LOG("Failed to load scene: " + fileName);
		return nullptr;
	}

	if (scn->HasMeshes()) {
		LoadMeshes(scn);
	}

	if (scn->HasMaterials()) {
		LoadMaterials(scn);
	}

	const auto rootNode = scn->mRootNode;

	if (rootNode) {
		for (auto i = 0; i < rootNode->mNumChildren; ++i) {
			const auto child = rootNode->mChildren[i];

			const auto entity = LoadNode(child);

			result->AddChild(entity);
		}
	}
	else {
		ERROR_LOG("The model has no root node.");
		return nullptr;
	}

	aiReleaseImport(scn);

	return std::move(result);
}

// ---------------------------------------------------

bool DemoScene::CreateTextureSampler() noexcept
{
	// All textures will be sampled with the same sampler for this scene.
	VkSamplerCreateInfo samplerCreateInfo{};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.anisotropyEnable = VK_TRUE;

	// If the device supports anisotropic filtering...
	if (G_VulkanDevice.GetPhysicalDevice().features.samplerAnisotropy) {
		samplerCreateInfo.maxAnisotropy = 16;
	}
	else {
		samplerCreateInfo.maxAnisotropy = 0;
	}

	samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
	samplerCreateInfo.compareEnable = VK_FALSE;
	samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerCreateInfo.mipLodBias = 0.0f;
	samplerCreateInfo.minLod = 0.0f;
	samplerCreateInfo.maxLod = 0.0f;

	VkResult result{ vkCreateSampler(G_VulkanDevice, &samplerCreateInfo, nullptr, &m_TextureSampler) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create texture sampler.");
		return false;
	}

	return true;
}

bool DemoScene::PrepareUniforms() noexcept
{
	//First create a descriptor pool to allocate descriptors from.
	//Descriptors a.k.a uniforms

	// We need 1 descriptor set for the scene matrices.
	VkDescriptorPoolSize sceneMatricesPoolSize{};
	sceneMatricesPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	sceneMatricesPoolSize.descriptorCount = 1;

	//and 1 descriptor set for the shared material.
	VkDescriptorPoolSize materialPoolSize{};

	// The material descriptor sets will contain image samplers (Textures) only.
	// the rest of the material attributes will use a push constant block.
	materialPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

	// 3 textures per material of each entity.
	materialPoolSize.descriptorCount = m_Materials.size() * 3;

	// Descriptor pool size for the deferred shading resolution pass (display pass)
	VkDescriptorPoolSize gBufferPoolSize{};
	gBufferPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	gBufferPoolSize.descriptorCount = 5; // x4 (position, normal, specular, albedo, depth textures)

	// 1 UBO for the light array (display pass)
	VkDescriptorPoolSize lightsPoolSize{};
	lightsPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	lightsPoolSize.descriptorCount = 1; // x1

	std::vector<VkDescriptorPoolSize> descriptorPoolSizes{
		sceneMatricesPoolSize,
		materialPoolSize,
		gBufferPoolSize,
		lightsPoolSize
	};

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;

	//1 set for each entity's material plus one for the scene matrices plus one for the display pass light ubo and input textures
	descriptorPoolCreateInfo.maxSets = m_Materials.size() + 2;
	descriptorPoolCreateInfo.poolSizeCount = static_cast<ui32>(descriptorPoolSizes.size());
	descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();

	const auto& device = G_VulkanDevice;

	VkResult result{
		vkCreateDescriptorPool(device,
		                       &descriptorPoolCreateInfo,
		                       nullptr,
		                       &m_DescriptorPool)
	};

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create descriptor pool.");
		return false;
	}

	//Now a descriptor set has to be created, but before that, its layout
	//has to be defined.
	//First the bindings have to be defined...
	//Descriptor set 0 - scene matrices;
	VkDescriptorSetLayoutBinding vertexShaderUboBinding{};
	vertexShaderUboBinding.binding = 0; //bind at location 0.
	vertexShaderUboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //It's a uniform buffer
	vertexShaderUboBinding.descriptorCount = 1; //1 descriptor.
	vertexShaderUboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; //bound to the vertex shader stage

	std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings{ vertexShaderUboBinding };

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.bindingCount = static_cast<ui32>(descriptorSetLayoutBindings.size());
	descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();

	result = vkCreateDescriptorSetLayout(device,
	                                     &descriptorSetLayoutCreateInfo,
	                                     nullptr,
	                                     &m_DescriptorSetLayouts.sceneMatrices);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create scene matrices descriptor set layout.");
		return false;
	}

	descriptorSetLayoutBindings.clear();

	//Descriptor set 1 - entity materials
	VkDescriptorSetLayoutBinding diffuseTextureShaderBinding{};
	diffuseTextureShaderBinding.binding = TEX_DIFFUSE; //bind at location 0.
	diffuseTextureShaderBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; //Binding samplers.
	diffuseTextureShaderBinding.descriptorCount = 1;
	diffuseTextureShaderBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; //bound to the fragment shader stage.

	descriptorSetLayoutBindings.push_back(diffuseTextureShaderBinding);

	VkDescriptorSetLayoutBinding specularTextureShaderBinding{};
	specularTextureShaderBinding.binding = TEX_SPECULAR; //bind at location 1.
	specularTextureShaderBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	specularTextureShaderBinding.descriptorCount = 1;
	specularTextureShaderBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	descriptorSetLayoutBindings.push_back(specularTextureShaderBinding);

	VkDescriptorSetLayoutBinding normalTextureShaderBinding{};
	normalTextureShaderBinding.binding = TEX_NORMAL; //bind at location 2
	normalTextureShaderBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	normalTextureShaderBinding.descriptorCount = 1;
	normalTextureShaderBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	descriptorSetLayoutBindings.push_back(normalTextureShaderBinding);

	descriptorSetLayoutCreateInfo.bindingCount = static_cast<ui32>(descriptorSetLayoutBindings.size());
	descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();

	result = vkCreateDescriptorSetLayout(device,
	                                     &descriptorSetLayoutCreateInfo,
	                                     nullptr,
	                                     &m_DescriptorSetLayouts.material);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create materials descriptor set layout.");
		return false;
	}

	descriptorSetLayoutBindings.clear();

	// Create descriptor bindings for the gBuffer and Lights descriptor set layout.
	VkDescriptorSetLayoutBinding positionTexBinding{};
	positionTexBinding.binding = 0;
	positionTexBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	positionTexBinding.descriptorCount = 1;
	positionTexBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	descriptorSetLayoutBindings.push_back(positionTexBinding);

	VkDescriptorSetLayoutBinding normalTexBinding{};
	normalTexBinding.binding = 1;
	normalTexBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	normalTexBinding.descriptorCount = 1;
	normalTexBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	descriptorSetLayoutBindings.push_back(normalTexBinding);

	VkDescriptorSetLayoutBinding albedoTexBinding{};
	albedoTexBinding.binding = 2;
	albedoTexBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	albedoTexBinding.descriptorCount = 1;
	albedoTexBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	descriptorSetLayoutBindings.push_back(albedoTexBinding);

	VkDescriptorSetLayoutBinding specularTexBinding{};
	specularTexBinding.binding = 3;
	specularTexBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	specularTexBinding.descriptorCount = 1;
	specularTexBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	descriptorSetLayoutBindings.push_back(specularTexBinding);

	VkDescriptorSetLayoutBinding depthTexBinding{};
	depthTexBinding.binding = 4;
	depthTexBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	depthTexBinding.descriptorCount = 1;
	depthTexBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	descriptorSetLayoutBindings.push_back(depthTexBinding);

	VkDescriptorSetLayoutBinding lightsUboBinding{};
	lightsUboBinding.binding = 5;
	lightsUboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	lightsUboBinding.descriptorCount = 1;
	lightsUboBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	descriptorSetLayoutBindings.push_back(lightsUboBinding);

	descriptorSetLayoutCreateInfo.bindingCount = static_cast<ui32>(descriptorSetLayoutBindings.size());
	descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();

	result = vkCreateDescriptorSetLayout(device,
	                                     &descriptorSetLayoutCreateInfo,
	                                     nullptr,
	                                     &m_DescriptorSetLayouts.gBufferAndLights);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create g buffer and lights descriptor set layout.");
		return false;
	}

	// After the descriptor set layouts have been defined and created,
	// the pipeline layout can be defined. The pipeline layout will need the
	// descriptor layouts created above plus the constant ranges that will be defined bellow.
	// The pipeline layout tells the driver how the uniforms (and other data) will be organized
	// and passed to the shaders.
	// Create the pipeline layout.
	// Here we specify to the the pipeline if we use an push constants or descriptor sets.

	// Use a push constant block for the model matrix of each object...
	std::array<VkPushConstantRange, 2> pushConstantRanges{};
	pushConstantRanges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	pushConstantRanges[0].size = sizeof(Mat4f); //1 4x4 matrix
	pushConstantRanges[0].offset = 0;

	// ... and a push constant range for the diffuse and specular vectors of the material.
	pushConstantRanges[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	pushConstantRanges[1].size = 2 * sizeof(Vec4f); // 2 vectors
	pushConstantRanges[1].offset = 64;

	std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
		m_DescriptorSetLayouts.sceneMatrices,
		m_DescriptorSetLayouts.material
	};

	// Deferred pass pipeline layout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<ui32>(descriptorSetLayouts.size());
	pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutCreateInfo.pushConstantRangeCount = static_cast<ui32>(pushConstantRanges.size());
	pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges.data();

	result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &m_PipelineLayouts.deferred);
	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create deferred pass pipeline layout.");
		return false;
	}

	descriptorSetLayouts.clear();
	descriptorSetLayouts.insert(descriptorSetLayouts.cbegin(),
	                            { m_DescriptorSetLayouts.gBufferAndLights });

	VkPushConstantRange pushConstantRange{};
	pushConstantRange.size = sizeof(int);
	pushConstantRange.offset = 0;
	pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	// Display/lighting pass pipeline layout
	pipelineLayoutCreateInfo.setLayoutCount = static_cast<ui32>(descriptorSetLayouts.size());
	pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstantRange;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;

	result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &m_PipelineLayouts.display);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create display/lighting pass pipeline layout.");
		return false;
	}

	// Now all the layouts have been defined. It is time to allocate the descriptor sets and write to them.
	// First, allocate the scene matrices descriptor set.
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = m_DescriptorPool; //Allocate from this pool.
	descriptorSetAllocateInfo.descriptorSetCount = 1; //1 descriptor set.
	descriptorSetAllocateInfo.pSetLayouts = &m_DescriptorSetLayouts.sceneMatrices; //With this layout.

	result = vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &m_DescriptorSets.sceneMatrices);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to allocate scene matrices descriptor set.");
		return false;
	}

	// Create the buffer object that will store the uniforms.
	if (!device.CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
	                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	                         m_Ubos.matrices,
	                         sizeof(MatricesUbo))) {
		ERROR_LOG("Failed to create uniform buffer.");
		return false;
	}

	// Now that the descriptor set is allocated we have to write into it and update the uniforms.
	VkWriteDescriptorSet uniformDescriptorWrite{};
	uniformDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uniformDescriptorWrite.dstSet = m_DescriptorSets.sceneMatrices; //Write to this set
	uniformDescriptorWrite.dstBinding = 0; //at this binding
	uniformDescriptorWrite.dstArrayElement = 0; //It is not an array.
	uniformDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //It is a uniform buffer.
	uniformDescriptorWrite.descriptorCount = 1; // 1 descriptor set.
	uniformDescriptorWrite.pBufferInfo = &m_Ubos.matrices.descriptorBufferInfo; // And here is the buffer description.

	std::vector<VkWriteDescriptorSet> writeDescriptorSets{ uniformDescriptorWrite };

	// Update the descriptor set.
	vkUpdateDescriptorSets(device,
	                       static_cast<ui32>(writeDescriptorSets.size()),
	                       writeDescriptorSets.data(),
	                       0,
	                       nullptr);

	writeDescriptorSets.clear();

	// Allocate 1 descriptor set for each entity's material
	descriptorSetAllocateInfo.descriptorSetCount = 1; //1 descriptor set.
	descriptorSetAllocateInfo.pSetLayouts = &m_DescriptorSetLayouts.material; //With this layout.

	for (auto& mat : m_Materials) {
		result = vkAllocateDescriptorSets(device,
		                                  &descriptorSetAllocateInfo,
		                                  &mat.descriptorSet);

		if (result != VK_SUCCESS) {
			ERROR_LOG("Failed to allocate material descriptor set.");
			return false;
		}


		// Define the descriptor image info structures for each texture type.
		VkDescriptorImageInfo diffuseTextureImageInfo{};
		diffuseTextureImageInfo.imageView = mat.textures[TEX_DIFFUSE]->GetImageView();
		diffuseTextureImageInfo.imageLayout = mat.textures[TEX_DIFFUSE]->GetImageLayout();
		diffuseTextureImageInfo.sampler = m_TextureSampler;

		VkDescriptorImageInfo specularTextureImageInfo{};
		specularTextureImageInfo.imageView = mat.textures[TEX_SPECULAR]->GetImageView();
		specularTextureImageInfo.imageLayout = mat.textures[TEX_SPECULAR]->GetImageLayout();
		specularTextureImageInfo.sampler = m_TextureSampler;

		VkDescriptorImageInfo normalTextureImageInfo{};
		normalTextureImageInfo.imageView = mat.textures[TEX_NORMAL]->GetImageView();
		normalTextureImageInfo.imageLayout = mat.textures[TEX_NORMAL]->GetImageLayout();
		normalTextureImageInfo.sampler = m_TextureSampler;

		// Define the descriptor writes for each texture type.
		VkWriteDescriptorSet diffuseTextureDescriptorWrite{};
		diffuseTextureDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		diffuseTextureDescriptorWrite.dstSet = mat.descriptorSet;
		diffuseTextureDescriptorWrite.dstBinding = TEX_DIFFUSE;
		diffuseTextureDescriptorWrite.dstArrayElement = 0;
		diffuseTextureDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		diffuseTextureDescriptorWrite.descriptorCount = 1;
		diffuseTextureDescriptorWrite.pImageInfo = &diffuseTextureImageInfo;

		writeDescriptorSets.push_back(diffuseTextureDescriptorWrite);

		VkWriteDescriptorSet specularTextureDescriptorWrite{};
		specularTextureDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		specularTextureDescriptorWrite.dstSet = mat.descriptorSet;
		specularTextureDescriptorWrite.dstBinding = TEX_SPECULAR;
		specularTextureDescriptorWrite.dstArrayElement = 0;
		specularTextureDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		specularTextureDescriptorWrite.descriptorCount = 1;
		specularTextureDescriptorWrite.pImageInfo = &specularTextureImageInfo;

		writeDescriptorSets.push_back(specularTextureDescriptorWrite);

		VkWriteDescriptorSet normalTextureDescriptorWrite{};
		normalTextureDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		normalTextureDescriptorWrite.dstSet = mat.descriptorSet;
		normalTextureDescriptorWrite.dstBinding = TEX_NORMAL;
		normalTextureDescriptorWrite.dstArrayElement = 0;
		normalTextureDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		normalTextureDescriptorWrite.descriptorCount = 1;
		normalTextureDescriptorWrite.pImageInfo = &normalTextureImageInfo;

		writeDescriptorSets.push_back(normalTextureDescriptorWrite);

		// Finally update the material's descriptor set.
		vkUpdateDescriptorSets(device,
		                       static_cast<ui32>(writeDescriptorSets.size()),
		                       writeDescriptorSets.data(),
		                       0,
		                       nullptr);

		writeDescriptorSets.clear();
	}

	descriptorSetAllocateInfo.descriptorSetCount = 1; //1 descriptor set.
	descriptorSetAllocateInfo.pSetLayouts = &m_DescriptorSetLayouts.gBufferAndLights; //With this layout.

	result = vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, &m_DescriptorSets.display);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create the display/lighting resolution descriptor set.");
		return false;
	}

	const auto sampler = m_GBuffer.GetSampler();

	const auto& positionAttachment = m_GBuffer.GetAttachment(m_AttachmentIndices.position);
	VkDescriptorImageInfo positionImageInfo{};
	positionImageInfo.imageView = positionAttachment.GetImageView();
	positionImageInfo.imageLayout = positionAttachment.GetDescription().finalLayout;
	positionImageInfo.sampler = sampler;

	const auto& normalAttachment = m_GBuffer.GetAttachment(m_AttachmentIndices.normal);
	VkDescriptorImageInfo normalImageInfo{};
	normalImageInfo.imageView = normalAttachment.GetImageView();
	normalImageInfo.imageLayout = normalAttachment.GetDescription().finalLayout;
	normalImageInfo.sampler = sampler;

	const auto& albedoAttachment = m_GBuffer.GetAttachment(m_AttachmentIndices.albedo);
	VkDescriptorImageInfo albedoImageInfo{};
	albedoImageInfo.imageView = albedoAttachment.GetImageView();
	albedoImageInfo.imageLayout = albedoAttachment.GetDescription().finalLayout;
	albedoImageInfo.sampler = sampler;

	const auto& specularAttachment = m_GBuffer.GetAttachment(m_AttachmentIndices.specular);
	VkDescriptorImageInfo specularImageInfo{};
	specularImageInfo.imageView = specularAttachment.GetImageView();
	specularImageInfo.imageLayout = specularAttachment.GetDescription().finalLayout;
	specularImageInfo.sampler = sampler;

	const auto& depthAttachment = m_GBuffer.GetAttachment(m_AttachmentIndices.depth);
	VkDescriptorImageInfo depthImageInfo{};
	depthImageInfo.imageView = depthAttachment.GetImageView();
	depthImageInfo.imageLayout = depthAttachment.GetDescription().finalLayout;
	depthImageInfo.sampler = sampler;

	VkWriteDescriptorSet positionImageDescriptorWrite{};
	positionImageDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	positionImageDescriptorWrite.dstSet = m_DescriptorSets.display;
	positionImageDescriptorWrite.dstBinding = m_AttachmentIndices.position;
	positionImageDescriptorWrite.dstArrayElement = 0;
	positionImageDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	positionImageDescriptorWrite.descriptorCount = 1;
	positionImageDescriptorWrite.pImageInfo = &positionImageInfo;

	writeDescriptorSets.push_back(positionImageDescriptorWrite);

	VkWriteDescriptorSet normalImageDescriptorWrite{};
	normalImageDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	normalImageDescriptorWrite.dstSet = m_DescriptorSets.display;
	normalImageDescriptorWrite.dstBinding = m_AttachmentIndices.normal;
	normalImageDescriptorWrite.dstArrayElement = 0;
	normalImageDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	normalImageDescriptorWrite.descriptorCount = 1;
	normalImageDescriptorWrite.pImageInfo = &normalImageInfo;

	writeDescriptorSets.push_back(normalImageDescriptorWrite);

	VkWriteDescriptorSet albedoImageDescriptorWrite{};
	albedoImageDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	albedoImageDescriptorWrite.dstSet = m_DescriptorSets.display;
	albedoImageDescriptorWrite.dstBinding = m_AttachmentIndices.albedo;
	albedoImageDescriptorWrite.dstArrayElement = 0;
	albedoImageDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	albedoImageDescriptorWrite.descriptorCount = 1;
	albedoImageDescriptorWrite.pImageInfo = &albedoImageInfo;

	writeDescriptorSets.push_back(albedoImageDescriptorWrite);

	VkWriteDescriptorSet specularImageDescriptorWrite{};
	specularImageDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	specularImageDescriptorWrite.dstSet = m_DescriptorSets.display;
	specularImageDescriptorWrite.dstBinding = m_AttachmentIndices.specular;
	specularImageDescriptorWrite.dstArrayElement = 0;
	specularImageDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	specularImageDescriptorWrite.descriptorCount = 1;
	specularImageDescriptorWrite.pImageInfo = &specularImageInfo;

	writeDescriptorSets.push_back(specularImageDescriptorWrite);

	VkWriteDescriptorSet depthImageDescriptorWrite{};
	depthImageDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	depthImageDescriptorWrite.dstSet = m_DescriptorSets.display;
	depthImageDescriptorWrite.dstBinding = m_AttachmentIndices.depth;
	depthImageDescriptorWrite.dstArrayElement = 0;
	depthImageDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	depthImageDescriptorWrite.descriptorCount = 1;
	depthImageDescriptorWrite.pImageInfo = &depthImageInfo;

	writeDescriptorSets.push_back(depthImageDescriptorWrite);

	if (!device.CreateBuffer(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
	                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	                         m_Ubos.lights,
	                         sizeof(LightsUbo))) {
		ERROR_LOG("Failed to create uniform buffer.");
		return false;
	}

	uniformDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	uniformDescriptorWrite.dstSet = m_DescriptorSets.display;
	uniformDescriptorWrite.dstBinding = writeDescriptorSets.size();
	uniformDescriptorWrite.dstArrayElement = 0;
	uniformDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uniformDescriptorWrite.descriptorCount = 1;
	uniformDescriptorWrite.pBufferInfo = &m_Ubos.lights.descriptorBufferInfo;

	writeDescriptorSets.push_back(uniformDescriptorWrite);

	//Update the display descriptor set.
	vkUpdateDescriptorSets(device,
	                       static_cast<ui32>(writeDescriptorSets.size()),
	                       writeDescriptorSets.data(),
	                       0,
	                       nullptr);

	m_Ubos.matrices.Map();
	m_Ubos.lights.Map();

	return true;
}

bool DemoScene::CreatePipelines(VkExtent2D swapChainExtent, VkRenderPass displayRenderPass) noexcept
{
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState{};
	inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyState.flags = VK_NULL_HANDLE;
	inputAssemblyState.primitiveRestartEnable = VK_FALSE;

	VkPipelineRasterizationStateCreateInfo rasterizationState{};
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.flags = VK_NULL_HANDLE;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	rasterizationState.lineWidth = 1.0f;

	VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
	colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
			VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachmentState.blendEnable = VK_FALSE;
	colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachmentStates{ 4, colorBlendAttachmentState };

	VkPipelineColorBlendStateCreateInfo colorBlendState{};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendState.logicOpEnable = VK_FALSE;
	colorBlendState.logicOp = VK_LOGIC_OP_COPY;
	colorBlendState.attachmentCount = colorBlendAttachmentStates.size();
	colorBlendState.pAttachments = colorBlendAttachmentStates.data();

	for (auto& blendConstant : colorBlendState.blendConstants) {
		blendConstant = 0.0f;
	}

	VkPipelineDepthStencilStateCreateInfo depthStencilState{};
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.depthTestEnable = VK_TRUE;
	depthStencilState.depthWriteEnable = VK_TRUE;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilState.depthBoundsTestEnable = VK_FALSE;
	depthStencilState.stencilTestEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.width = static_cast<float>(swapChainExtent.width);
	viewport.height = static_cast<float>(swapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.extent = swapChainExtent;
	scissor.offset = VkOffset2D{ 0, 0 };

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineMultisampleStateCreateInfo multisampleState{};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.sampleShadingEnable = VK_FALSE;
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleState.minSampleShading = 1.0f;
	multisampleState.pSampleMask = nullptr;
	multisampleState.alphaToCoverageEnable = VK_FALSE;
	multisampleState.alphaToOneEnable = VK_FALSE;

	std::vector<VkDynamicState> dynamicStateToEnable = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates = dynamicStateToEnable.data();
	dynamicState.dynamicStateCount = static_cast<ui32>(dynamicStateToEnable.size());
	dynamicState.flags = VK_NULL_HANDLE;

	// Solid rendering pipeline
	// Load shaders
	VulkanShader* vertexShader{ G_ResourceManager.Get<VulkanShader>("sdr/deferred.vert.spv") };

	if (!vertexShader) {
		ERROR_LOG("Failed to load vertex shader.");
		return false;
	}

	VulkanShader* fragmentShader{ G_ResourceManager.Get<VulkanShader>("sdr/deferred.frag.spv") };

	if (!fragmentShader) {
		ERROR_LOG("Failed to load fragment shader.");
		return false;
	}

	VkPipelineShaderStageCreateInfo vertexShaderStage{};
	vertexShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStage.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderStage.module = *vertexShader;
	vertexShaderStage.pName = "main"; //Shader function entry point.

	VkPipelineShaderStageCreateInfo fragmentShaderStage{};
	fragmentShaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderStage.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderStage.module = *fragmentShader;
	fragmentShaderStage.pName = "main"; //Shader function entry point.

	std::vector<VkPipelineShaderStageCreateInfo> shaderStages{
		vertexShaderStage,
		fragmentShaderStage
	};

	auto vertexInputBindingDescription = VulkanMesh::GetVertexInputBindingDescription();

	auto vertexInputAttributeDescriptions = VulkanMesh::GetVertexInputAttributeDescriptions();

	// Vertex binding information and attribute descriptions.
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<ui32>(vertexInputAttributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();

	VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.renderPass = m_GBuffer.GetRenderPass();
	pipelineCreateInfo.layout = m_PipelineLayouts.deferred;
	pipelineCreateInfo.flags = VK_NULL_HANDLE;
	pipelineCreateInfo.basePipelineIndex = -1;
	pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.pDynamicState = &dynamicState;
	pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCreateInfo.pStages = shaderStages.data();

	VkResult result{
		vkCreateGraphicsPipelines(G_VulkanDevice,
		                          m_PipelineCache,
		                          1,
		                          &pipelineCreateInfo,
		                          nullptr,
		                          &m_Pipelines.deferred)
	};

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create pipeline.");
		return false;
	}

	// Display pipeline
	// The display pipeline only has 1 color attachment
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = &colorBlendAttachmentState;

	vertexShader = G_ResourceManager.Get<VulkanShader>("sdr/display.vert.spv");

	if (!vertexShader) {
		ERROR_LOG("Failed to load vertex shader.");
		return false;
	}

	fragmentShader = G_ResourceManager.Get<VulkanShader>("sdr/display.frag.spv");

	if (!fragmentShader) {
		ERROR_LOG("Failed to load vertex shader.");
		return false;
	}

	// change the shader modules
	vertexShaderStage.module = *vertexShader;
	fragmentShaderStage.module = *fragmentShader;

	shaderStages.clear();
	shaderStages.insert(shaderStages.cbegin(), { vertexShaderStage, fragmentShaderStage });

	pipelineCreateInfo.pStages = shaderStages.data();

	// Assign the correct pipeline layout
	pipelineCreateInfo.layout = m_PipelineLayouts.display;

	VkPipelineVertexInputStateCreateInfo emptyInputState{};
	emptyInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	emptyInputState.vertexAttributeDescriptionCount = 0;
	emptyInputState.pVertexAttributeDescriptions = VK_NULL_HANDLE;
	emptyInputState.vertexBindingDescriptionCount = 0;
	emptyInputState.pVertexBindingDescriptions = VK_NULL_HANDLE;

	pipelineCreateInfo.pVertexInputState = &emptyInputState;

	rasterizationState.cullMode = VK_CULL_MODE_FRONT_BIT;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;

	pipelineCreateInfo.pRasterizationState = &rasterizationState;

	// Assign the correct render pass.
	pipelineCreateInfo.renderPass = displayRenderPass;

	result = vkCreateGraphicsPipelines(G_VulkanDevice,
	                                   m_PipelineCache,
	                                   1,
	                                   &pipelineCreateInfo,
	                                   nullptr,
	                                   &m_Pipelines.display);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create pipeline.");
		return false;
	}

	return true;
}

bool DemoScene::InitializeImGUI(const VkRenderPass renderPass) noexcept
{
	VkDescriptorPoolSize pool_size[11] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1 }
	};

	VkDescriptorPoolCreateInfo pool_info{};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	pool_info.maxSets = 1;
	pool_info.poolSizeCount = 11;
	pool_info.pPoolSizes = pool_size;
	VkResult result{ vkCreateDescriptorPool(G_VulkanDevice, &pool_info, nullptr, &m_ImGUIDescriptorPool) };

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create ImGUI descriptor pool.");
		return false;
	}

	ImGui_ImplGlfwVulkan_Init_Data init_data{};
	init_data.allocator = nullptr;
	init_data.gpu = G_VulkanDevice.GetPhysicalDevice();
	init_data.device = G_VulkanDevice;
	init_data.render_pass = renderPass;
	init_data.pipeline_cache = m_PipelineCache;
	init_data.descriptor_pool = m_ImGUIDescriptorPool;
	init_data.check_vk_result = [](auto res)
	{
		if (res != VK_SUCCESS) { ERROR_LOG("ImGUI Vulkan intialization failed!"); }
	};

	if (!ImGui_ImplGlfwVulkan_Init(G_Application.GetWindow(), true, &init_data)) {
		ERROR_LOG("Failed to initialize ImGUI.");
		return false;
	}

	const VkCommandBuffer commandBuffer{ G_VulkanDevice.CreateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY) };

	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	result = vkBeginCommandBuffer(commandBuffer, &begin_info);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to begin command buffer for Font uploading (ImGUI)");
		return false;
	}

	ImGui_ImplGlfwVulkan_CreateFontsTexture(commandBuffer);

	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	VkFence fence;
	result = vkCreateFence(G_VulkanDevice, &fenceCreateInfo, nullptr, &fence);

	if (result != VK_SUCCESS) {
		ERROR_LOG("Failed to create fence.");
		return false;
	}

	vkEndCommandBuffer(commandBuffer);

	if (!G_VulkanDevice.SubmitCommandBuffer(commandBuffer,
	                                        G_VulkanDevice.GetQueue(QueueFamily::TRANSFER),
	                                        fence)) {
		ERROR_LOG("Failed to submit command buffer for Font uploading.");
		return false;
	}


	ImGui_ImplGlfwVulkan_InvalidateFontUploadObjects();

	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_Header] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);

	return true;
}

// -------------------------------------------------------------------
DemoScene::~DemoScene()
{
	const auto& device = G_VulkanDevice;

	for (auto mesh : m_Meshes) {
		delete mesh;
	}
	m_Meshes.clear();

	vkDestroySampler(device, m_TextureSampler, nullptr);

	//No need to free descriptor sets. They are taken care of by the Vulkan driver.
	//Just destroy the descriptor set layout and the descriptor pool.
	vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayouts.sceneMatrices, nullptr);
	vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayouts.material, nullptr);
	vkDestroyDescriptorSetLayout(device, m_DescriptorSetLayouts.gBufferAndLights, nullptr);

	vkDestroyDescriptorPool(device, m_DescriptorPool, nullptr);
	vkDestroyDescriptorPool(device, m_ImGUIDescriptorPool, nullptr);

	vkDestroyPipelineLayout(device, m_PipelineLayouts.deferred, nullptr);
	vkDestroyPipelineLayout(device, m_PipelineLayouts.display, nullptr);

	vkDestroyPipeline(device, m_Pipelines.deferred, nullptr);
	vkDestroyPipeline(device, m_Pipelines.display, nullptr);

	ImGui_ImplGlfwVulkan_Shutdown();
}

bool DemoScene::Initialize(const VkExtent2D swapChainExtent, VkRenderPass displayRenderPass) noexcept
{
	using namespace std::chrono;
	const auto seed = high_resolution_clock::now().time_since_epoch().count();
	s_Rng = std::mt19937(seed);

	if (!m_PipelineCache.Create()) {
		return false;
	}

	if (!CreateTextureSampler()) {
		return false;
	}

	const Vec2ui size{ swapChainExtent.width, swapChainExtent.height };

	m_AttachmentIndices.position = m_GBuffer.AddAttachment(size,
	                                                       1,
	                                                       VK_FORMAT_R16G16B16A16_SFLOAT,
	                                                       AttachmentType::COLOR,
	                                                       true);

	m_AttachmentIndices.normal = m_GBuffer.AddAttachment(size,
	                                                     1,
	                                                     VK_FORMAT_R16G16B16A16_SFLOAT,
	                                                     AttachmentType::COLOR,
	                                                     true);

	m_AttachmentIndices.albedo = m_GBuffer.AddAttachment(size,
	                                                     1,
	                                                     VK_FORMAT_R8G8B8A8_UNORM,
	                                                     AttachmentType::COLOR,
	                                                     true);

	m_AttachmentIndices.specular = m_GBuffer.AddAttachment(size,
	                                                       1,
	                                                       VK_FORMAT_R8G8B8A8_UNORM,
	                                                       AttachmentType::COLOR,
	                                                       true);

	m_AttachmentIndices.depth = m_GBuffer.AddAttachment(size,
	                                                    1,
	                                                    VK_FORMAT_D32_SFLOAT,
	                                                    AttachmentType::DEPTH,
	                                                    true);

	if (!m_GBuffer.Create(size,
	                      VK_FILTER_NEAREST,
	                      VK_FILTER_NEAREST,
	                      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)) {
		ERROR_LOG("Failed to create GBuffer!");
		return false;
	}

	m_Entities.push_back(LoadModel("models/scene.fbx"));

	for (auto& entity : m_Entities) {
		entity->Update(0.0f);
	}

	if (!PrepareUniforms()) {
		ERROR_LOG("Failed to prepare the scene's uniforms");
		return false;
	}

	if (!CreatePipelines(swapChainExtent, displayRenderPass)) {
		ERROR_LOG("Failed to create scene's pipelines.");
		return false;
	}

	if (!InitializeImGUI(displayRenderPass)) {
		return false;
	}

	return true;
}

void DemoScene::Update(VkExtent2D swapChainExtent, i64 msec, f64 dt) noexcept
{
	for (auto& entity : m_Entities) {
		entity->Update(dt);
	}

	MatricesUbo ubo{};
	float radius = 20.0f;
	Vec3f eye{ sin(msec / 3000.0f) * radius, 3.0f, cos(msec / 3000.0f) * radius };
	ubo.view = glm::lookAt(eye, Vec3f{ 0.0, 6.0f, 0.0f }, Vec3f{ 0.0f, 1.0f, 0.0f });
	//ubo.view = glm::rotate(ubo.view, msec / 3000.0f, Vec3f{ 0.0, 1.0, 0.0 });

	f32 aspect{ static_cast<f32>(swapChainExtent.width) / static_cast<f32>(swapChainExtent.height) };

	ubo.projection = s_ClipCorrectionMat * glm::perspective(glm::radians(60.0f), aspect, 0.1f, 2000.0f);

	m_Ubos.matrices.Fill(&ubo, sizeof ubo);

	m_Lights.positions[0] = Vec4f{ sin(msec / 1000.0f) * 20.0f, 10.0f, cos(msec / 1000.0f) * 20.0f - 10.0f, 0.0f };
	m_Lights.colors[0] = Vec4f{ 1.0f, 1.0f, 1.0f, 1.0 };
	m_Lights.radi[0] = Vec4f{ 80.0f };

	m_Lights.positions[1] = Vec4f{ cos(msec / 500.0f) * 10.0f, 5.0f, sin(msec / 500.0f) * 10.0f - 10.0f, 0.0f };
	m_Lights.colors[1] = Vec4f{ 0.0f, 0.0f, 1.0f, 1.0 };
	m_Lights.radi[1] = Vec4f{ 80.0f };

	m_Lights.positions[2] = Vec4f{ cos(msec / 3000.0f) * 10.0f, 20.0f, sin(msec / 3000.0f) * 10.0f - 10.0f, 0.0f };
	m_Lights.colors[2] = Vec4f{ 0.0f, 1.0f, 0.0f, 1.0 };
	m_Lights.radi[2] = Vec4f{ 120.0f };

	m_Lights.positions[3] = Vec4f{ cos(msec / 200.0f) * 5.0f, 10.0f, sin(msec / 200.0f) * 5.0f - 10.0f, 0.0f };
	m_Lights.colors[3] = Vec4f{ 1.0f, 0.0f, 0.0f, 1.0 };
	m_Lights.radi[3] = Vec4f{ 120.0f };

	m_Lights.eyePos = eye;

	m_Ubos.lights.Fill(&m_Lights, sizeof m_Lights);
}

void DemoScene::DrawEntity(DemoEntity* entity, VkCommandBuffer commandBuffer) noexcept
{
	const auto mesh = entity->GetMesh();
	if (mesh) {
		const auto material = entity->GetMaterial();

		if (!material) {
			LOG("HAVE MESH> NO MATERIAL");
			return;
		}

		std::vector<VkDescriptorSet> descriptorSets{
			m_DescriptorSets.sceneMatrices,
			material->descriptorSet
		};

		vkCmdBindDescriptorSets(commandBuffer,
		                        VK_PIPELINE_BIND_POINT_GRAPHICS,
		                        m_PipelineLayouts.deferred,
		                        0,
		                        static_cast<ui32>(descriptorSets.size()),
		                        descriptorSets.data(),
		                        0,
		                        nullptr);

		const auto& xform = entity->GetXform();

		vkCmdPushConstants(commandBuffer,
		                   m_PipelineLayouts.deferred,
		                   VK_SHADER_STAGE_VERTEX_BIT,
		                   0,
		                   sizeof(Mat4f),
		                   &xform);

		std::array<Vec4f, 2> materialProperties{ material->diffuse, material->specular };

		vkCmdPushConstants(commandBuffer,
		                   m_PipelineLayouts.deferred,
		                   VK_SHADER_STAGE_FRAGMENT_BIT,
		                   sizeof(Mat4f),
		                   2 * sizeof(Vec4f),
		                   materialProperties.data());

		mesh->Draw(commandBuffer);
	}

	for (auto child : entity->GetChildren()) {
		const auto c = static_cast<DemoEntity*>(child);
		DrawEntity(c, commandBuffer);
	}
}

void DemoScene::Draw(VkCommandBuffer commandBuffer) noexcept
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipelines.deferred);

	for (const auto& entity : m_Entities) {
		DrawEntity(entity.get(), commandBuffer);
	}
}

static std::vector<float> f{ 1.0, 2.0, 3.0, 4.0 };

void DemoScene::DrawFullscreenQuad(VkCommandBuffer commandBuffer) const noexcept
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipelines.display);
	vkCmdBindDescriptorSets(commandBuffer,
	                        VK_PIPELINE_BIND_POINT_GRAPHICS,
	                        m_PipelineLayouts.display,
	                        0,
	                        1,
	                        &m_DescriptorSets.display,
	                        0,
	                        nullptr);

	vkCmdPushConstants(commandBuffer,
	                   m_PipelineLayouts.display,
	                   VK_SHADER_STAGE_FRAGMENT_BIT,
	                   0,
	                   sizeof(int),
	                   &m_CurrentAttachment);

	// Draws a triangle that fills the whole screen. 3 VS invocations only and gets turned into a quad due to clipping.
	// tc[0, 0] is at the top left. No need for vertex buffers. Positions and texture coordinates are generated in the vertex shader
	// using gl_VertexIndex. see display.vert
	vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	auto& application = G_Application;

	ImGui_ImplGlfwVulkan_NewFrame();

	if (!application.resultsCalculated) {
		// 1. Show a simple window.
		// Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
		ImGui::Begin("Metrics");
		ImGui::Text("Device name: %s", G_VulkanDevice.GetPhysicalDevice().properties.deviceName);

		const auto deviceType = G_VulkanDevice.GetPhysicalDevice().properties.deviceType;

		const char* type{ nullptr };

		switch (deviceType) {
		case VK_PHYSICAL_DEVICE_TYPE_OTHER:
			type = "Other";
			break;
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			type = "Integrated GPU";
			break;
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			type = "Discrete GPU";
			break;
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			type = "Virtual GPU";
			break;
		case VK_PHYSICAL_DEVICE_TYPE_CPU:
			type = "CPU";
			break;
		default:;
		}

		ImGui::Text("Device ID: %d", G_VulkanDevice.GetPhysicalDevice().properties.deviceID);
		ImGui::Text("Device type: %s", type);
		ImGui::Text("Vendor: %d", G_VulkanDevice.GetPhysicalDevice().properties.vendorID);
		ImGui::Text("Driver Version: %d", G_VulkanDevice.GetPhysicalDevice().properties.driverVersion);

		ImGui::NewLine();
		ImGui::Separator();
		ImGui::NewLine();

		if (ImGui::Combo("Active attachment", &m_CurrentAttachment, m_AttachmentComboItems.data(),
			m_AttachmentComboItems.size())) {
			LOG("Value changed to: " + std::to_string(m_CurrentAttachment));
		}

		ImGui::NewLine();
		ImGui::Separator();
		ImGui::NewLine();

		char buff[60];

		ImGui::Text("Average value real time graphs (refresh per sec)");
		snprintf(buff, 60, "FPS\nAvg: %f\nMin: %f\nMax: %f", application.averageFps, application.minFps,
			application.maxFps);
		ImGui::PlotLines(buff, application.fpsAverages.data(), application.fpsAverages.size(), 0, "",
			0.0, application.maxFps, ImVec2(0, 80));

		snprintf(buff, 60, "Frame time (ms)\nAvg: %f ms\nMin: %f ms\nMax: %f ms", application.wholeFrameAverage, application.minWholeFrame,
			application.maxWholeFrame);
		ImGui::PlotLines(buff, application.wholeFrameAverages.data(), application.wholeFrameAverages.size(), 0, "",
			0.0, application.maxWholeFrame, ImVec2(0, 80));

		snprintf(buff, 60, "CPU time (ms)\nAvg: %f ms\nMin: %f ms\nMax: %f ms", application.cpuTimeAverage, application.minCpuTime,
			application.maxCpuTime);
		ImGui::PlotLines(buff, application.cpuTimeAverages.data(), application.cpuTimeAverages.size(), 0, "",
			0.0, application.maxCpuTime, ImVec2(0, 80));

		snprintf(buff, 60, "GPU time (ms)\nAvg: %f ms\nMin: %f ms\nMax: %f ms", application.gpuTimeAverage, application.minGpuTime,
			application.maxGpuTime);
		ImGui::PlotLines(buff, application.gpuTimeAverages.data(), application.gpuTimeAverages.size(), 0, "",
			0.0, application.maxGpuTime, ImVec2(0, 80));

		ImGui::NewLine();
		ImGui::Text("Running time: %f s", application.GetTimer().GetSec());
		ImGui::Text("Frame count: %d", application.frameCount);
		ImGui::End();
	} else {
		ImGui::Begin("Benchmark Results");
		ImGui::Text("Device name: %s", G_VulkanDevice.GetPhysicalDevice().properties.deviceName);

		const auto deviceType = G_VulkanDevice.GetPhysicalDevice().properties.deviceType;

		const char* type{ nullptr };

		switch (deviceType) {
		case VK_PHYSICAL_DEVICE_TYPE_OTHER:
			type = "Other";
			break;
		case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			type = "Integrated GPU";
			break;
		case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			type = "Discrete GPU";
			break;
		case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
			type = "Virtual GPU";
			break;
		case VK_PHYSICAL_DEVICE_TYPE_CPU:
			type = "CPU";
			break;
		default:;
		}

		ImGui::Text("Device ID: %d", G_VulkanDevice.GetPhysicalDevice().properties.deviceID);
		ImGui::Text("Device type: %s", type);
		ImGui::Text("Vendor: %d", G_VulkanDevice.GetPhysicalDevice().properties.vendorID);
		ImGui::Text("Driver Version: %d", G_VulkanDevice.GetPhysicalDevice().properties.driverVersion);

		ImGui::NewLine();
		ImGui::Separator();
		ImGui::NewLine();

		char buff[60];

		snprintf(buff, 60, "Frame time (ms)\nAvg: %f ms\nMin: %f ms\nMax: %f ms", application.avgTotalFrameTime, application.minTotalFrameTime,
			application.maxTotalFrameTime);
		ImGui::PlotLines(buff, application.totalFrameTimeSamples.data(), application.totalFrameTimeSamples.size(), 0, "",
			application.minTotalFrameTime, application.maxTotalFrameTime, ImVec2(1750, 100));

		ImGui::NewLine();

		snprintf(buff, 60, "CPU time (ms)\nAvg: %f ms\nMin: %f ms\nMax: %f ms", application.avgTotalCpuTime, application.minTotalCpuTime,
			application.maxTotalCpuTime);
		ImGui::PlotLines(buff, application.totalCpuTimeSamples.data(), application.totalCpuTimeSamples.size(), 0, "",
			application.minTotalCpuTime, application.maxTotalCpuTime, ImVec2(1750, 100));

		ImGui::NewLine();

		snprintf(buff, 60, "GPU time (ms)\nAvg: %f ms\nMin: %f ms\nMax: %f ms", application.avgTotalGpuTime, application.minTotalGpuTime,
			application.maxTotalGpuTime);
		ImGui::PlotLines(buff, application.totalGpuTimeSamples.data(), application.totalGpuTimeSamples.size(), 0, "",
			application.minTotalGpuTime, application.maxTotalGpuTime, ImVec2(1750, 100));

		ImGui::NewLine();

		if (ImGui::Button("Save to CSV")) {
			LOG("Button Pressed");
		}

		ImGui::End();
	}

	ImGui_ImplGlfwVulkan_Render(commandBuffer);
}

const VulkanRenderTarget& DemoScene::GetGBuffer() const noexcept
{
	return m_GBuffer;
}
