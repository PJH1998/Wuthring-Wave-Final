#pragma once
#include "Interface_Edit.h"
NS_BEGIN(Engine)
class CBehavior_Tree;
NS_END

NS_BEGIN(Editor)
class CASM_Interface final : public CInterface_Edit
{
	enum ASM_MENU
	{
		BEHAVIOR_TREE,
		ANIM_MACHINE,
		END
	};

#pragma region GraphEditor_Definition

	typedef struct tagBTDelegate : public GraphEditor::Delegate
	{
		CASM_Interface* pInterface {nullptr};

		bool AllowedLink(GraphEditor::NodeIndex from, GraphEditor::NodeIndex to) override;
		void SelectNode(GraphEditor::NodeIndex nodeIndex, bool selected) override;
		void MoveSelectedNodes(const ImVec2 delta) override;
		void AddLink(GraphEditor::NodeIndex inputNodeIndex, GraphEditor::SlotIndex inputSlotIndex, GraphEditor::NodeIndex outputNodeIndex, GraphEditor::SlotIndex outputSlotIndex) override;
		void DelLink(GraphEditor::LinkIndex linkIndex) override;
		void CustomDraw(ImDrawList* drawList, ImRect rectangle, GraphEditor::NodeIndex nodeIndex) override;
		void RightClick(GraphEditor::NodeIndex nodeIndex, GraphEditor::SlotIndex slotIndexInput, GraphEditor::SlotIndex slotIndexOutput) override;
		const size_t GetTemplateCount() override;
		const GraphEditor::Template GetTemplate(GraphEditor::TemplateIndex index) override;
		const size_t GetNodeCount() override;
		const GraphEditor::Node GetNode(GraphEditor::NodeIndex index) override;
		const size_t GetLinkCount() override;
		const GraphEditor::Link GetLink(GraphEditor::LinkIndex index) override;
	}BT_DELEGATE;

	struct MyNode : public GraphEditor::Node
	{
		_string strName;
		float x, y;
		vector<GraphEditor::Link> Transitions;
	};
#pragma endregion

private:
	explicit CASM_Interface(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual ~CASM_Interface() = default;

public:
	virtual	HRESULT		Initialize();
	void				Update_ASM();

	

private:
	ASM_MENU			m_eCurrentMenu = { ASM_MENU::BEHAVIOR_TREE };

#pragma region BehaviorTree_GraphEdit
	BT_DELEGATE				m_BehaviorTreeGraphDelegate;
	GraphEditor::Options	m_BehaviorTreeGraphOptions;
	GraphEditor::ViewState	m_BehaviorTreeViewState;

	vector<MyNode>			m_Nodes;
	std::vector<GraphEditor::Link> m_Links;
	GraphEditor::Template	m_Templates[2];

	_int m_iCurrentNodeIndex { -1 };
	_uint m_iNodeCount{};
#pragma endregion

	CBehavior_Tree*		m_pBehaviorTree = { nullptr };
	CBlackBoard*		m_pBlackBoard = {nullptr};

private:
	void				Menu_BehaviorTree();
	void				Graph_BehaviorTree();
	void				Delete_Link();

	void				Menu_AnimMachine();
	void				Graph_AnimMachine();

public:
	static		CASM_Interface* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual		void				Free() override;
};
NS_END
