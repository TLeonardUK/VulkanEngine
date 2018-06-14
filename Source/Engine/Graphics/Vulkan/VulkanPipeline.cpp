#pragma once

#include "Engine/Graphics/Vulkan/VulkanPipeline.h"
#include "Engine/Graphics/Vulkan/VulkanEnums.h"
#include "Engine/Graphics/Vulkan/VulkanRenderPass.h"
#include "Engine/Graphics/Vulkan/VulkanVertexBuffer.h"
#include "Engine/Graphics/Vulkan/VulkanResourceSet.h"

#include "Engine/Engine/Logging.h"

#include <cassert>

VkPolygonMode GraphicsPolygonModeToVk(GraphicsPolygonMode input)
{
	switch (input)
	{
	case GraphicsPolygonMode::Fill:	return VK_POLYGON_MODE_FILL;
	case GraphicsPolygonMode::Line:	return VK_POLYGON_MODE_LINE; 
	case GraphicsPolygonMode::Point: return VK_POLYGON_MODE_POINT; 
	}

	assert(false);
	return VK_POLYGON_MODE_MAX_ENUM;
}

VkCullModeFlags GraphicsCullModeToVk(GraphicsCullMode input)
{
	switch (input)
	{
	case GraphicsCullMode::None: return VK_CULL_MODE_NONE;
	case GraphicsCullMode::Front: return VK_CULL_MODE_FRONT_BIT;
	case GraphicsCullMode::Back: return VK_CULL_MODE_BACK_BIT;
	}

	assert(false);
	return VK_CULL_MODE_FLAG_BITS_MAX_ENUM;
}

VkFrontFace GraphicsFaceWindingOrderToVk(GraphicsFaceWindingOrder input)
{
	switch (input)
	{
	case GraphicsFaceWindingOrder::Clockwise: return VK_FRONT_FACE_CLOCKWISE;
	case GraphicsFaceWindingOrder::CounterClockwise: return VK_FRONT_FACE_COUNTER_CLOCKWISE;
	}

	assert(false);
	return VK_FRONT_FACE_MAX_ENUM;
}

VkBlendFactor GraphicsBlendFactorToVk(GraphicsBlendFactor input)
{
	switch (input)
	{
	case GraphicsBlendFactor::DstAlpha: return VK_BLEND_FACTOR_DST_ALPHA;
	case GraphicsBlendFactor::DstColor: return VK_BLEND_FACTOR_DST_ALPHA;
	case GraphicsBlendFactor::OneMinusDstAlpha: return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
	case GraphicsBlendFactor::OneMinusDstColor: return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
	case GraphicsBlendFactor::OneMinusSrcAlpha: return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	case GraphicsBlendFactor::OneMinusSrcColor: return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
	case GraphicsBlendFactor::SrcAlpha: return VK_BLEND_FACTOR_SRC_ALPHA;
	case GraphicsBlendFactor::SrcColor: return VK_BLEND_FACTOR_SRC_COLOR;
	case GraphicsBlendFactor::Zero: return VK_BLEND_FACTOR_ZERO;
	}

	assert(false);
	return VK_BLEND_FACTOR_MAX_ENUM;
}

VkBlendOp GraphicsBlendOpToVk(GraphicsBlendOp input)
{
	switch (input)
	{
	case GraphicsBlendOp::Add: return VK_BLEND_OP_ADD;
	case GraphicsBlendOp::Max: return VK_BLEND_OP_MAX;
	case GraphicsBlendOp::Min: return VK_BLEND_OP_MIN;
	case GraphicsBlendOp::ReverseSubtract: return VK_BLEND_OP_REVERSE_SUBTRACT;
	case GraphicsBlendOp::Subtract: return VK_BLEND_OP_SUBTRACT;
	}

	assert(false);
	return VK_BLEND_OP_MAX_ENUM;
}

VkCompareOp GraphicsDepthCompareOpToVk(GraphicsDepthCompareOp input)
{
	switch (input)
	{
	case GraphicsDepthCompareOp::Always: return VK_COMPARE_OP_ALWAYS;
	case GraphicsDepthCompareOp::Equal: return VK_COMPARE_OP_EQUAL;
	case GraphicsDepthCompareOp::Greater: return VK_COMPARE_OP_GREATER;
	case GraphicsDepthCompareOp::GreaterOrEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
	case GraphicsDepthCompareOp::Less: return VK_COMPARE_OP_LESS;
	case GraphicsDepthCompareOp::LessOrEqual: return VK_COMPARE_OP_LESS_OR_EQUAL;
	case GraphicsDepthCompareOp::Never: return VK_COMPARE_OP_NEVER;
	case GraphicsDepthCompareOp::NotEqual: return VK_COMPARE_OP_NOT_EQUAL;
	}

	assert(false);
	return VK_COMPARE_OP_MAX_ENUM;
}

VkCompareOp GraphicsStencilTestCompareOpToVk(GraphicsStencilTestCompareOp input)
{
	switch (input)
	{
	case GraphicsStencilTestCompareOp::Always: return VK_COMPARE_OP_ALWAYS;
	case GraphicsStencilTestCompareOp::Equal: return VK_COMPARE_OP_EQUAL;
	case GraphicsStencilTestCompareOp::Greater: return VK_COMPARE_OP_GREATER;
	case GraphicsStencilTestCompareOp::GreaterOrEqual: return VK_COMPARE_OP_GREATER_OR_EQUAL;
	case GraphicsStencilTestCompareOp::Less: return VK_COMPARE_OP_LESS;
	case GraphicsStencilTestCompareOp::LessOrEqual: return VK_COMPARE_OP_LESS_OR_EQUAL;
	case GraphicsStencilTestCompareOp::Never: return VK_COMPARE_OP_NEVER;
	case GraphicsStencilTestCompareOp::NotEqual: return VK_COMPARE_OP_NOT_EQUAL;
	}

	assert(false);
	return VK_COMPARE_OP_MAX_ENUM;
}

VkStencilOp GraphicsStencilTestOpToVk(GraphicsStencilTestOp input)
{
	switch (input)
	{
	case GraphicsStencilTestOp::DecrementAndClamp: return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
	case GraphicsStencilTestOp::DecrementAndWrap: return VK_STENCIL_OP_DECREMENT_AND_WRAP;
	case GraphicsStencilTestOp::IncrementAndClamp: return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
	case GraphicsStencilTestOp::IncrementAndWrap: return VK_STENCIL_OP_INCREMENT_AND_WRAP;
	case GraphicsStencilTestOp::Invert: return VK_STENCIL_OP_INVERT;
	case GraphicsStencilTestOp::Keep: return VK_STENCIL_OP_KEEP;
	case GraphicsStencilTestOp::Replace: return VK_STENCIL_OP_REPLACE;
	case GraphicsStencilTestOp::Zero: return VK_STENCIL_OP_ZERO;
	}

	assert(false);
	return VK_STENCIL_OP_MAX_ENUM;
}

VkShaderStageFlagBits GraphicsPipelineStageToVkShaderStage(GraphicsPipelineStage stage)
{
	switch (stage)
	{
		case GraphicsPipelineStage::Fragment:	return VK_SHADER_STAGE_FRAGMENT_BIT;
		case GraphicsPipelineStage::Vertex:		return VK_SHADER_STAGE_VERTEX_BIT;
	}

	assert(false);
	return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
}

VulkanPipeline::VulkanPipeline(
	VkDevice device,
	std::shared_ptr<Logger> logger,
	const String& name
)
	: m_device(device)
	, m_logger(logger)
	, m_name(name)
{
}

VulkanPipeline::~VulkanPipeline()
{
	FreeResources();
}

void VulkanPipeline::FreeResources()
{
	if (m_pipeline != nullptr)
	{
		vkDestroyPipeline(m_device, m_pipeline, nullptr);
		m_pipeline = nullptr;
	}
	if (m_pipelineLayout != nullptr)
	{
		vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
		m_pipelineLayout = nullptr;
	}
}

VkPipeline VulkanPipeline::GetPipeline()
{
	return m_pipeline;
}

VkPipelineLayout VulkanPipeline::GetPipelineLayout()
{
	return m_pipelineLayout;
}

bool VulkanPipeline::Build(const GraphicsPipelineSettings& settings)
{
	m_logger->WriteInfo(LogCategory::Vulkan, "Builiding new pipeline: %s", m_name.c_str());

	Array<VkPipelineShaderStageCreateInfo> shaderStages;

	for (int i = 0; i < (int)GraphicsPipelineStage::Count; i++)
	{
		if (settings.ShaderStages[i] != nullptr)
		{
			std::shared_ptr<VulkanShader> shader = std::dynamic_pointer_cast<VulkanShader>(settings.ShaderStages[i]);

			VkPipelineShaderStageCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			info.stage = GraphicsPipelineStageToVkShaderStage((GraphicsPipelineStage)i);
			info.module = shader->GetModule();
			info.pName = "main";

			shaderStages.push_back(info);
		}
	}	

	Array<VkVertexInputBindingDescription> bindingDescriptions;
	Array<VkVertexInputAttributeDescription> attributeDescriptions;

	if (settings.HasVertexFormatDescription)
	{
		if (!GraphicsBindingDescriptionToVulkan(settings.VertexFormatDescription, bindingDescriptions, attributeDescriptions))
		{
			return false;
		}
	}

	Array<VkDescriptorSetLayout> resourceLayouts(settings.ResourceSets.size());

	for (int i = 0; i < settings.ResourceSets.size(); i++)
	{
		std::shared_ptr<VulkanResourceSet> set = std::dynamic_pointer_cast<VulkanResourceSet>(settings.ResourceSets[i]);

		resourceLayouts[i] = set->GetLayout();
	}

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.empty() ? nullptr : bindingDescriptions.data();
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.empty() ? nullptr : attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = 0.0f;
	viewport.height = 0.0f;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = { 0, 0 };

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = GraphicsPolygonModeToVk(settings.PolygonMode);
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = GraphicsCullModeToVk(settings.CullMode);
	rasterizer.frontFace = GraphicsFaceWindingOrderToVk(settings.FaceWindingOrder);
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = settings.BlendEnabled ? VK_TRUE : VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = GraphicsBlendFactorToVk(settings.SrcColorBlendFactor);
	colorBlendAttachment.dstColorBlendFactor = GraphicsBlendFactorToVk(settings.DstColorBlendFactor);
	colorBlendAttachment.colorBlendOp = GraphicsBlendOpToVk(settings.ColorBlendOp);
	colorBlendAttachment.srcAlphaBlendFactor = GraphicsBlendFactorToVk(settings.SrcAlphaBlendFactor);
	colorBlendAttachment.dstAlphaBlendFactor = GraphicsBlendFactorToVk(settings.DstAlphaBlendFactor);
	colorBlendAttachment.alphaBlendOp = GraphicsBlendOpToVk(settings.AlphaBlendOp);

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = (settings.DepthTestEnabled ? VK_TRUE : VK_FALSE);
	depthStencil.depthWriteEnable = (settings.DepthWriteEnabled ? VK_TRUE : VK_FALSE);
	depthStencil.depthCompareOp = GraphicsDepthCompareOpToVk(settings.DepthCompareOp);
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f; // Optional
	depthStencil.maxDepthBounds = 1.0f; // Optional
	depthStencil.stencilTestEnable = (settings.StencilTestEnabled ? VK_TRUE : VK_FALSE);
	depthStencil.front.compareMask = settings.StencilTestReadMask;
	depthStencil.front.compareOp = GraphicsStencilTestCompareOpToVk(settings.StencilTestCompareOp);
	depthStencil.front.depthFailOp = GraphicsStencilTestOpToVk(settings.StencilTestZFailOp);
	depthStencil.front.failOp = GraphicsStencilTestOpToVk(settings.StencilTestFailOp);
	depthStencil.front.passOp = GraphicsStencilTestOpToVk(settings.StencilTestPassOp);
	depthStencil.front.reference = settings.StencilTestReference;
	depthStencil.front.writeMask = settings.StencilTestWriteMask;
	depthStencil.back = {}; // Optional

	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(resourceLayouts.size());
	pipelineLayoutInfo.pSetLayouts = resourceLayouts.empty() ? nullptr : resourceLayouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	CheckVkResultReturnOnFail(vkCreatePipelineLayout(m_device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineInfo.pStages = shaderStages.data();
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = m_pipelineLayout;
	pipelineInfo.renderPass = std::dynamic_pointer_cast<VulkanRenderPass>(settings.RenderPass)->GetRenderPass();
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	CheckVkResultReturnOnFail(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline));

	return true;
}