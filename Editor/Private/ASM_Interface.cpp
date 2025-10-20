#include "EditorPch.h"
#include "ASM_Interface.h"
#pragma region BehaviorTree
#include "BT_Action.h"
#include "BT_Selector.h"
#include "BT_Sequence.h"
#pragma endregion

CASM_Interface::CASM_Interface(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CInterface_Edit{pDevice, pContext}
{
}

HRESULT CASM_Interface::Initialize()
{
	m_Templates[0] = {

		IM_COL32(160, 160, 180, 255),
		IM_COL32(100, 100, 140, 255),
		IM_COL32(110, 110, 150, 255),
		1,
		nullptr,
		nullptr,
		2,
		nullptr,
		nullptr
	};
	m_Templates[1] = {

		IM_COL32(160, 160, 180, 255),
		IM_COL32(100, 100, 140, 255),
		IM_COL32(110, 110, 150, 255),
		4,
		nullptr,
		nullptr,
		4,
		nullptr,
		nullptr
	};
	//Node tNode = {"Test",0, 0.f, 0.f, false};
	//m_Nodes.push_back(tNode);
	m_BehaviorTreeGraphDelegate.pInterface = this;

	CBT_Selector* pRoot = CBT_Selector::Create();
	m_pBehaviorTree = CBehavior_Tree::Create(m_pDevice, m_pContext, pRoot);

	m_pBlackBoard = CBlackBoard::Create();

	CBehavior_Tree::BEHAVIOR_TREE_DESC BTDesc{};
	BTDesc.pBlackBoard = m_pBlackBoard;
	m_pBehaviorTree->Initialize_Clone(&BTDesc);

	return S_OK;
}

void CASM_Interface::Update_ASM()
{
	ImGui::Begin("Animation State Machine Interface");

	ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
	if(ImGui::BeginTabBar("TabBar", tab_bar_flags))
	{
		if(ImGui::BeginTabItem("Behavior Tree"))
		{
			m_eCurrentMenu = ASM_MENU::BEHAVIOR_TREE;
			Menu_BehaviorTree();
			ImGui::EndTabItem();
		}

		if(ImGui::BeginTabItem("Anim Machine"))
		{
			m_eCurrentMenu = ASM_MENU::ANIM_MACHINE;
			Menu_AnimMachine();
			ImGui::EndTabItem();

		}
		ImGui::EndTabBar();
	}

	ImGui::End();

	if(m_eCurrentMenu == ASM_MENU::BEHAVIOR_TREE)
	{
		Graph_BehaviorTree();
	}
	else if(m_eCurrentMenu == ASM_MENU::ANIM_MACHINE)
	{
		Graph_AnimMachine();
	}

#ifdef _DEBUG
	m_pBehaviorTree->BlackBoardInfo();
	//ImGui::ShowMetricsWindow();
#endif // _DEBUG
	m_BehaviorTreeGraphDelegate.GetTemplateCount();
}

void CASM_Interface::Menu_BehaviorTree()
{
	ImGui::BeginTable("Property", 2, ImGuiTableFlags_BordersInnerV);
	ImGui::TableNextColumn();
	ImGui::Text("Behavior Tree Editor - To Be Continued...");
	ImGui::Text("Selected Node Index : %d", m_iCurrentNodeIndex);
	if(ImGui::Button("Add Node"))
	{
		_string szNodeName;
		szNodeName = "new Node";
		szNodeName += to_string(m_iNodeCount).c_str();
		MyNode tNode = {"new Node", 0, ImRect(), false, szNodeName, 0.f, 0.f};
		m_Nodes.push_back(tNode);
		++m_iNodeCount;
	}
	ImGui::SameLine();
	if(ImGui::Button("Delete Node"))
	{
		if(m_iCurrentNodeIndex >= 0 && m_iCurrentNodeIndex < static_cast<_int>(m_Nodes.size()))
		{
			Delete_Link();

			m_Nodes.erase(m_Nodes.begin() + m_iCurrentNodeIndex);
			m_iCurrentNodeIndex = -1;
		}
	}
	ImGui::TableNextColumn();
	
	BlackBoard_Setting();
	//_float3 a = *(_float3*)(m_ValueContainer["test"].second);
	ImGui::EndTable();
}

void CASM_Interface::Menu_AnimMachine()
{
	ImGui::BeginTable("Property", 2, ImGuiTableFlags_BordersInnerV);
	ImGui::TableNextColumn();
	ImGui::Text("Anim Machine Editor - To Be Continued...");
	ImGui::TableNextColumn();
	if(ImGui::Button("Add State"))
	{
	}
	ImGui::EndTable();
}

void CASM_Interface::Graph_BehaviorTree()
{
	ImGui::Begin("Behavior Tree Graph");
	GraphEditor::Show(m_BehaviorTreeGraphDelegate, m_BehaviorTreeGraphOptions, m_BehaviorTreeViewState, true);
	ImGui::End();
}

void CASM_Interface::Delete_Link()
{
	auto iter = m_Links.begin();
	while(iter != m_Links.end())
	{
		if((*iter).mInputNodeIndex == m_iCurrentNodeIndex || (*iter).mOutputNodeIndex == m_iCurrentNodeIndex)
		{
			iter = m_Links.erase(iter);
		}
		else
		{
			iter++;
		}
	}
}

void CASM_Interface::BlackBoard_Setting()
{
	ImGui::Text("Black Board Data Setting");
	if(ImGui::RadioButton("Int", reinterpret_cast<int*>(&m_eDataType), 0)){}
	ImGui::SameLine();
	if(ImGui::RadioButton("Float", reinterpret_cast<int*>(&m_eDataType), 1)){}
	ImGui::SameLine();
	if(ImGui::RadioButton("String", reinterpret_cast<int*>(&m_eDataType), 2)){}
	ImGui::SameLine();
	if(ImGui::RadioButton("Bool", reinterpret_cast<int*>(&m_eDataType), 3)){}
	ImGui::SameLine();
	if(ImGui::RadioButton("Vec3", reinterpret_cast<int*>(&m_eDataType), 4)){}
	ImGui::SameLine();
	if(ImGui::RadioButton("Vec4", reinterpret_cast<int*>(&m_eDataType), 5)){}
	ImGui::InputText("Value Name", m_strValueTag.data(), MAX_PATH);
	switch(m_eDataType)
	{
	case Editor::CASM_Interface::INT:
		ImGui::InputInt("int", &m_iInputTemp);
		break;
	case Editor::CASM_Interface::FLOAT:
		ImGui::InputFloat("float", &m_fInputTemp);
		break;
	case Editor::CASM_Interface::STRING:
		ImGui::InputText("string", m_strInputTemp.data(), MAX_PATH);
		break;
	case Editor::CASM_Interface::BOOL:
		ImGui::Checkbox("bool", &m_bInputTemp);
		break;
	case Editor::CASM_Interface::VECTOR3:
		ImGui::InputFloat3("vector3", reinterpret_cast<_float*>(&m_v3InputTemp));
		break;
	case Editor::CASM_Interface::VECTOR4:
		ImGui::InputFloat4("vector4", reinterpret_cast<_float*>(&m_v4InputTemp));
		break;
	default:
		break;
	}

	if(ImGui::Button("Add Variable"))
	{
		//pair<DATA_TYPE, VAR> _Value;
		pair<DATA_TYPE, void*> _Value;
		_Value.first = m_eDataType;
		switch(m_eDataType)
		{
		case Editor::CASM_Interface::INT:
		{
			_int* iTemp = new _int;
			*iTemp = m_iInputTemp;
			//_Value.second = m_iInputTemp;
			_Value.second = iTemp;
			break;
		}
		case Editor::CASM_Interface::FLOAT:
		{
			_float* fTemp = new _float;
			*fTemp = m_fInputTemp;
			//_Value.second = m_fInputTemp;
			_Value.second = fTemp;
			break;
		}
		case Editor::CASM_Interface::STRING:
		{
			_string* strTemp = new _string;
			*strTemp = m_strInputTemp;
			//_Value.second = m_strInputTemp;
			_Value.second = strTemp;
			break;
		}
		case Editor::CASM_Interface::BOOL:
		{
			_bool* bTemp = new _bool;
			*bTemp = m_bInputTemp;
			//_Value.second = m_bInputTemp;
			_Value.second = bTemp;
			break;
		}
		case Editor::CASM_Interface::VECTOR3:
		{
			_float3* v3Temp = new _float3;
			*v3Temp = m_v3InputTemp;
			//_Value.second = m_v3InputTemp;
			_Value.second = v3Temp;
			break;
		}
		case Editor::CASM_Interface::VECTOR4:
		{
			_float4* v4Temp = new _float4;
			*v4Temp = m_v4InputTemp;
			//_Value.second = m_v4InputTemp;
			_Value.second = v4Temp;
		}
			break;
		default:
			ASSERT_CRASH(false);
			break;
		}
		_char szValueTag[MAX_PATH];
		strcpy_s(szValueTag, m_strValueTag.c_str());
		m_ValueContainer.emplace(szValueTag, _Value);
		m_pBlackBoard->Add_Data(szValueTag, CBlackBoard::DATA_TYPE(m_ValueContainer[szValueTag].first), m_ValueContainer[szValueTag].second);
	}
}

void CASM_Interface::Graph_AnimMachine()
{
	ImGui::Begin("Anim Machine Graph");

	ImGui::End();
}

void CASM_Interface::Clear_Container()
{
	for(auto& Pair : m_ValueContainer)
	{
		switch(Pair.second.first)
		{
		case Editor::CASM_Interface::INT:
			delete static_cast<_int*>(Pair.second.second);
			break;
		case Editor::CASM_Interface::FLOAT:
			delete static_cast<_float*>(Pair.second.second);
			break;
		case Editor::CASM_Interface::STRING:
			delete static_cast<_string*>(Pair.second.second);
			break;
		case Editor::CASM_Interface::BOOL:
			delete static_cast<_bool*>(Pair.second.second);
			break;
		case Editor::CASM_Interface::VECTOR3:
			delete static_cast<_float3*>(Pair.second.second);
			break;
		case Editor::CASM_Interface::VECTOR4:
			delete static_cast<_float4*>(Pair.second.second);
			break;
		default:
			break;
		}
		Pair.second.second = nullptr;
	}
	m_ValueContainer.clear();
}

CASM_Interface* CASM_Interface::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CASM_Interface* pInstance = new CASM_Interface(pDevice, pContext);
	if(FAILED(pInstance->Initialize()))
	{
		MSG_BOX("Failed to Created : CASM_Interface");
		Safe_Release(pInstance);
	}
	return pInstance;
}

void CASM_Interface::Free()
{
	__super::Free();
	Safe_Release(m_pBehaviorTree);
	Clear_Container();
}

#pragma region GraphEdit_Definition
bool CASM_Interface::tagBTDelegate::AllowedLink(GraphEditor::NodeIndex from, GraphEditor::NodeIndex to)
{
	if(from == to)
		return false;

	const auto& outNode = pInterface->m_Nodes[from];
	const auto& inNode = pInterface->m_Nodes[to];

	const auto& outTmpl = pInterface->m_Templates[outNode.mTemplateIndex];
	const auto& inTmpl = pInterface->m_Templates[inNode.mTemplateIndex];

	for(const auto& tLink : pInterface->m_Links)
	{
		if(tLink.mInputNodeIndex == to)
		{
			if(tLink.mOutputNodeIndex == from)
				return false;
			for(const auto& nTrans : inNode.Transitions)
			{
				if(tLink.mInputSlotIndex == nTrans.mInputSlotIndex)
					return false;
			}
		}
	}

	return true;
}

void CASM_Interface::tagBTDelegate::SelectNode(GraphEditor::NodeIndex nodeIndex, bool selected)  
{  
   // Access the m_Nodes vector through the parent class instance  
   pInterface->m_Nodes[nodeIndex].mSelected = selected;
   pInterface->m_iCurrentNodeIndex = selected ? nodeIndex : -1;
}

void CASM_Interface::tagBTDelegate::MoveSelectedNodes(const ImVec2 delta)
{
	
	for(auto& node : pInterface->m_Nodes)
	{
		if(!node.mSelected)
		{
			continue;
		}
		node.x += delta.x;
		node.y += delta.y;
	}
}

void CASM_Interface::tagBTDelegate::AddLink(GraphEditor::NodeIndex inputNodeIndex, GraphEditor::SlotIndex inputSlotIndex, GraphEditor::NodeIndex outputNodeIndex, GraphEditor::SlotIndex outputSlotIndex)
{
	GraphEditor::Link tLink = {inputNodeIndex, inputSlotIndex, outputNodeIndex, outputSlotIndex};
	pInterface->m_Links.push_back(tLink);
	pInterface->m_Nodes[inputNodeIndex].Transitions.push_back(tLink);
}

void CASM_Interface::tagBTDelegate::DelLink(GraphEditor::LinkIndex linkIndex)
{
	pInterface->m_Links.erase(pInterface->m_Links.begin() + linkIndex);
}

void CASM_Interface::tagBTDelegate::CustomDraw(ImDrawList* drawList, ImRect rectangle, GraphEditor::NodeIndex nodeIndex)
{
	drawList->AddLine(rectangle.Min, rectangle.Max, IM_COL32(0, 0, 0, 255));
	drawList->AddText((rectangle.Min + rectangle.Max) * 0.5f, IM_COL32(255, 128, 64, 255), pInterface->m_Nodes[nodeIndex].strName.c_str());
}

void CASM_Interface::tagBTDelegate::RightClick(GraphEditor::NodeIndex nodeIndex, GraphEditor::SlotIndex slotIndexInput, GraphEditor::SlotIndex slotIndexOutput)
{
}

const size_t CASM_Interface::tagBTDelegate::GetTemplateCount()
{
	return sizeof(pInterface->m_Templates) / sizeof(GraphEditor::Template);
}

const GraphEditor::Template CASM_Interface::tagBTDelegate::GetTemplate(GraphEditor::TemplateIndex index)
{
	const auto& tTemplete = pInterface->m_Templates[index];
	return GraphEditor::Template{tTemplete};
}

const size_t CASM_Interface::tagBTDelegate::GetNodeCount()
{
	return pInterface->m_Nodes.size();
}

const GraphEditor::Node CASM_Interface::tagBTDelegate::GetNode(GraphEditor::NodeIndex index)
{
	const auto& tNode = pInterface->m_Nodes[index];
	return GraphEditor::Node
	{
		tNode.mName,
		tNode.mTemplateIndex,
		ImRect(ImVec2(tNode.x, tNode.y), ImVec2(tNode.x + 200, tNode.y + 200)),
		tNode.mSelected
	};
}

const size_t CASM_Interface::tagBTDelegate::GetLinkCount()
{
	return pInterface->m_Links.size();
}

const GraphEditor::Link CASM_Interface::tagBTDelegate::GetLink(GraphEditor::LinkIndex index)
{
	const auto& tLink = pInterface->m_Links[index];

	// 노드 인덱스 유효성
	IM_ASSERT(tLink.mInputNodeIndex >= 0 && tLink.mInputNodeIndex < pInterface->m_Nodes.size());
	IM_ASSERT(tLink.mOutputNodeIndex >= 0 && tLink.mOutputNodeIndex < pInterface->m_Nodes.size());

	// 템플릿에서 슬롯 개수 얻기
	const auto& inNode = pInterface->m_Nodes[tLink.mInputNodeIndex];
	const auto& outNode = pInterface->m_Nodes[tLink.mOutputNodeIndex];

	const auto& inTmpl = pInterface->m_Templates[inNode.mTemplateIndex];
	const auto& outTmpl = pInterface->m_Templates[outNode.mTemplateIndex];

	IM_ASSERT(tLink.mInputSlotIndex >= 0 && tLink.mInputSlotIndex < inTmpl.mOutputCount);
	IM_ASSERT(tLink.mOutputSlotIndex >= 0 && tLink.mOutputSlotIndex < outTmpl.mInputCount);

	return GraphEditor::Link
	{
		tLink.mInputNodeIndex,
		tLink.mInputSlotIndex,
		tLink.mOutputNodeIndex,
		tLink.mOutputSlotIndex
	};
	//return {0,0,1,0};
}
#pragma endregion