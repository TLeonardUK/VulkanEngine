#include "Engine/Graphics/GraphicsRenderPass.h"

GraphicsAttachmentIndex GraphicsRenderPassSettings::AddColorAttachment(GraphicsFormat format, bool isPresentBuffer)
{
	GraphicsRenderPassAttachment attachment;
	attachment.bIsDepthStencil = false;
	attachment.bIsPresentBuffer = isPresentBuffer;
	attachment.format = format;
	attachments.push_back(attachment);

	return static_cast<GraphicsAttachmentIndex>(attachments.size() - 1);
}

GraphicsAttachmentIndex GraphicsRenderPassSettings::AddDepthAttachment(GraphicsFormat format)
{
	GraphicsRenderPassAttachment attachment;
	attachment.bIsDepthStencil = true;
	attachment.bIsPresentBuffer = false;
	attachment.format = format;
	attachments.push_back(attachment);

	return static_cast<GraphicsAttachmentIndex>(attachments.size() - 1);
}

GraphicsSubPassIndex GraphicsRenderPassSettings::AddSubPass()
{
	GraphicsRenderPassSubPass pass;

	subPasses.push_back(pass);

	return static_cast<GraphicsSubPassIndex>(subPasses.size() - 1);
}

GraphicsSubPassDependencyIndex GraphicsRenderPassSettings::AddSubPassDependency(GraphicsSubPassIndex sourcePass, GraphicsAccessMask sourceAccessMask, GraphicsSubPassIndex destPass, GraphicsAccessMask destAccessMask)
{
	GraphicsRenderPassSubPassDependency dep;
	dep.sourcePass = sourcePass;
	dep.sourceAccessMask = sourceAccessMask;
	dep.destPass = destPass;
	dep.destAccessMask = destAccessMask;

	subPassDependencies.push_back(dep);

	return static_cast<GraphicsSubPassDependencyIndex>(subPassDependencies.size() - 1);
}