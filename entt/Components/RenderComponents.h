#pragma once


struct RenderNodeDeleter
{
	void operator()(IRenderNode* r) {
		gEnv->p3DEngine->DeleteRenderNode(r);
	}
};

using TRenderNodePtr = std::unique_ptr<IRenderNode, RenderNodeDeleter>;

namespace Components::Render
{
	struct RenderComponent
	{
		TRenderNodePtr renderNode;
	};

	struct MeshComponent
	{
		_smart_ptr<IStatObj> mesh;
	};


}