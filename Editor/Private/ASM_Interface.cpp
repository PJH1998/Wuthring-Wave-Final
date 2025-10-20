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
	//Action Templete
	//m_Templates[0] = {
	//
	//	IM_COL32(160, 160, 180, 255),
	//	IM_COL32(100, 100, 140, 255),
	//	IM_COL32(110, 110, 150, 255),
	//	1,
	//	nullptr,
	//	nullptr,
	//	0,
	//	nullptr,
	//	nullptr
	//};
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

void CASM_Interface::Update_ASM(_float fTimeDelta)
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

	BehaviorTree_Setting();

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
			Delete_Transitions((*iter));
			
			m_Templates[m_Nodes[(*iter).mInputNodeIndex].mTemplateIndex].mOutputCount--;
			if(m_Templates[m_Nodes[(*iter).mInputNodeIndex].mTemplateIndex].mOutputCount < 1)
				m_Templates[m_Nodes[(*iter).mInputNodeIndex].mTemplateIndex].mOutputCount = 1;

			iter = m_Links.erase(iter);
		}
		else
		{
			iter++;
		}
	}
}

_bool CASM_Interface::Allowed_LInkEx(GraphEditor::Link& tLink)
{
	for(auto& tTrans : m_Nodes[tLink.mInputNodeIndex].Transitions)
		if(tLink.mInputSlotIndex == tTrans.mInputSlotIndex)
			return false;

	return true;
}

void CASM_Interface::BehaviorTree_Setting()
{
	ImGui::Text("Behavior Tree Editor - To Be Continued...");
	ImGui::Text("Selected Node Index : %d", m_iCurrentNodeIndex);
	if(m_iCurrentNodeIndex > -1)
	{
		ImGui::Text(m_Nodes[m_iCurrentNodeIndex].strName.c_str());
		ImGui::Text("x : %.3f", m_Nodes[m_iCurrentNodeIndex].x);
		ImGui::SameLine();
		ImGui::Text("y : %.3f", m_Nodes[m_iCurrentNodeIndex].y);
		if(ImGui::Button("Add Slot"))
		{
			if(BT_TYPE::ACTION != m_Nodes[m_iCurrentNodeIndex].eType)
				m_Templates[m_Nodes[m_iCurrentNodeIndex].mTemplateIndex].mOutputCount++;
		}
	}
	ImGui::Separator();
	if(ImGui::RadioButton("Action", reinterpret_cast<int*>(&m_eNodeType), 0)){}
	ImGui::SameLine();
	if(ImGui::RadioButton("Selector", reinterpret_cast<int*>(&m_eNodeType), 1)){}
	ImGui::SameLine();
	if(ImGui::RadioButton("Sequence", reinterpret_cast<int*>(&m_eNodeType), 2)){}

	if(ImGui::Button("Add Node"))
	{
		_string szNodeName;
		szNodeName = "Count";
		szNodeName += to_string(m_iNodeCount).c_str();
		switch(m_eNodeType)
		{
		case Editor::CASM_Interface::ACTION:
		{
			MyNode tNode = {"Action", m_Templates.size(), ImRect(), false, szNodeName, 0.f, 0.f, m_eNodeType};
			m_Nodes.push_back(tNode);
			break;
		}
		case Editor::CASM_Interface::SELECTOR:
		{
			MyNode tNode = {"Selector", m_Templates.size(), ImRect(), false, szNodeName, 0.f, 0.f, m_eNodeType};
			m_Nodes.push_back(tNode);
			break;
		}
		case Editor::CASM_Interface::SEQUENCE:
		{
			MyNode tNode = {"Sequence", m_Templates.size(), ImRect(), false, szNodeName, 0.f, 0.f, m_eNodeType};
			m_Nodes.push_back(tNode);
			break;
		}
		default:
			break;
		}
		
		//노드 동적 템플릿
		Create_Template(m_eNodeType);
		
		++m_iNodeCount;
	}
	ImGui::SameLine();
	if(ImGui::Button("Delete Node"))
	{
		if(m_iCurrentNodeIndex >= 0 && m_iCurrentNodeIndex < static_cast<_int>(m_Nodes.size()))
		{
			Delete_Link();
			//m_Links[0] == ImGuizmo::;
			m_Templates.erase(m_Templates.begin() + m_iCurrentNodeIndex);
			m_Nodes.erase(m_Nodes.begin() + m_iCurrentNodeIndex);
			size_t iNumNodes = m_Nodes.size();
			for(size_t i = 0; i < iNumNodes; ++i)
			{
				m_Nodes[i].mTemplateIndex = i;
			}
			m_iCurrentNodeIndex = -1;
		}
	}
}

void CASM_Interface::BlackBoard_Setting()
{
	ImGui::Text("Black Board Data Setting");

	if(!m_ValueContainer.empty())
	{
		const _char* szPreview = m_strValueKey.c_str();
		if(ImGui::BeginCombo("Select Data", szPreview))
		{
			_int iGuiID{};
			for(auto& Pair : m_ValueContainer)
			{
				ImGui::PushID(iGuiID);
				const _bool isSelected = Pair.first == m_strValueKey;
				if(ImGui::Selectable(Pair.first.c_str(), isSelected))
				{
					m_strValueKey = Pair.first;
				}
				ImGui::PopID();
			}
			ImGui::EndCombo();
		}
		switch(m_ValueContainer[m_strValueKey].first)
		{
		case Editor::CASM_Interface::INT:
			ImGui::Text("Int");
			ImGui::InputInt(m_strValueKey.c_str(), static_cast<_int*>(m_ValueContainer[m_strValueKey].second));
			break;
		case Editor::CASM_Interface::FLOAT:
			ImGui::Text("Float");
			ImGui::InputFloat(m_strValueKey.c_str(), static_cast<_float*>(m_ValueContainer[m_strValueKey].second));
			break;
		case Editor::CASM_Interface::STRING:
		{
			char strBuffer[MAX_PATH] = {};
			strcpy_s(strBuffer, MAX_PATH, m_strValueKey.c_str());
			strcat_s(strBuffer, MAX_PATH, " : ");
			strcat_s(strBuffer, MAX_PATH, static_cast<_string*>(m_ValueContainer[m_strValueKey].second)->c_str());
			ImGui::Text(strBuffer);
			break;
		}
		case Editor::CASM_Interface::BOOL:
			ImGui::Text("Bool");
			ImGui::Checkbox(m_strValueKey.c_str(), static_cast<_bool*>(m_ValueContainer[m_strValueKey].second));
			break;
		case Editor::CASM_Interface::VECTOR3:
			ImGui::Text("Float3");
			ImGui::InputFloat3(m_strValueKey.c_str(), static_cast<_float*>(m_ValueContainer[m_strValueKey].second));
			break;
		case Editor::CASM_Interface::VECTOR4:
			ImGui::Text("Float4");
			ImGui::InputFloat4(m_strValueKey.c_str(), static_cast<_float*>(m_ValueContainer[m_strValueKey].second));
			break;
		default:
			break;
		}
	}
	ImGui::Separator();
#pragma region DATA_SELECT
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
#pragma endregion
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
		//m_strValueTag를 바로 map 키로 대입하거나 함수 매개변수로 대입하면 size 0인 상황 발생.
		_char szValueTag[MAX_PATH];
		strcpy_s(szValueTag, m_strValueTag.c_str());
		m_ValueContainer.emplace(szValueTag, _Value);
		m_pBlackBoard->Add_Data(szValueTag, CBlackBoard::DATA_TYPE(m_ValueContainer[szValueTag].first), m_ValueContainer[szValueTag].second);
		//m_strValueKey = m_strValueTag; 대입 시 m_strValueKey이 size 0 인 상태로 복사됨.
		m_strValueKey = szValueTag;
//#ifdef _DEBUG
//		cout << "a: \"" << m_strValueKey << "\" size=" << m_strValueKey.size() << "\n";
//		cout << "b: \"" << m_strValueTag << "\" size=" << m_strValueTag.size() << "\n";
//		cout << "&a=" << (const void*)&m_strValueKey << " &b=" << (const void*)&m_strValueTag << "\n";
//		cout << "a.data=" << static_cast<const void*>(m_strValueKey.data())
//			<< " b.data=" << static_cast<const void*>(m_strValueTag.data()) << "\n";
//#endif // _DEBUG

	}
#ifdef _DEBUG

#endif // _DEBUG

}

void CASM_Interface::Create_Template(BT_TYPE eType)
{
	/*
		IM_COL32(160, 160, 180, 255),
		IM_COL32(100, 100, 140, 255),
		IM_COL32(110, 110, 150, 255),
		1,
		nullptr,
		nullptr,
		0,
		nullptr,
		nullptr
	*/
	GraphEditor::Template tTemplate{};
	tTemplate.mInputCount = 1;
	switch(eType)
	{
	case Editor::CASM_Interface::ACTION:
	{
		tTemplate.mHeaderColor = IM_COL32(10, 120, 200, 255);
		tTemplate.mBackgroundColor = IM_COL32(0, 80, 160, 255);
		tTemplate.mBackgroundColorOver = IM_COL32(20, 100, 180, 255);
		tTemplate.mOutputCount = 0;
		break;
	}

	case Editor::CASM_Interface::SELECTOR:
	{
		tTemplate.mHeaderColor = IM_COL32(160, 160, 180, 255);
		tTemplate.mBackgroundColor = IM_COL32(100, 100, 140, 255);
		tTemplate.mBackgroundColorOver = IM_COL32(110, 110, 150, 255);
		tTemplate.mOutputCount = 1;
		break;
	}
	case Editor::CASM_Interface::SEQUENCE:
	{
		tTemplate.mHeaderColor = IM_COL32(10, 200, 10, 255);
		tTemplate.mBackgroundColor = IM_COL32(0, 160, 0, 255);
		tTemplate.mBackgroundColorOver = IM_COL32(20, 180, 20, 255);
		tTemplate.mOutputCount = 1;
		break;
	}
	default:
		break;
	}
	//tTemplate.mInputNames = nullptr;
	//tTemplate.mInputColors = nullptr;
	//tTemplate.mOutputNames = nullptr;
	//tTemplate.mOutputColors = nullptr;
	m_Templates.push_back(tTemplate);
}

#ifdef _DEBUG
void CASM_Interface::Safe_Delete_Variable(const _string& strVariableTag)
{
	if(m_ValueContainer.find(strVariableTag) == m_ValueContainer.end())
		return;
	m_pBlackBoard->Unbind_Data(strVariableTag);
	switch(m_ValueContainer[strVariableTag].first)
	{
	case Editor::CASM_Interface::INT:
		delete static_cast<_int*>(m_ValueContainer[strVariableTag].second);
		break;
	case Editor::CASM_Interface::FLOAT:
		delete static_cast<_float*>(m_ValueContainer[strVariableTag].second);
		break;
	case Editor::CASM_Interface::STRING:
		delete static_cast<_string*>(m_ValueContainer[strVariableTag].second);
		break;
	case Editor::CASM_Interface::BOOL:
		delete static_cast<_bool*>(m_ValueContainer[strVariableTag].second);
		break;
	case Editor::CASM_Interface::VECTOR3:
		delete static_cast<_float3*>(m_ValueContainer[strVariableTag].second);
		break;
	case Editor::CASM_Interface::VECTOR4:
		delete static_cast<_float4*>(m_ValueContainer[strVariableTag].second);
		break;
	default:
		break;
	}
	m_ValueContainer.erase(strVariableTag);
	if(!m_ValueContainer.empty())
		m_strValueKey = m_ValueContainer.begin()->first;
}
#endif // _DEBUG

void CASM_Interface::Delete_Transitions(const GraphEditor::Link& tLink)
{
	for(auto& tNode : m_Nodes)
	{
		auto iter = tNode.Transitions.begin();
		while(iter != tNode.Transitions.end())
		{
			if(tLink.mInputNodeIndex == (*iter).mInputNodeIndex &&
				tLink.mInputSlotIndex == (*iter).mInputSlotIndex &&
				tLink.mOutputNodeIndex == (*iter).mOutputNodeIndex &&
				tLink.mOutputSlotIndex == (*iter).mOutputSlotIndex)
				iter = tNode.Transitions.erase(iter);
			else
				iter++;
		}
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

	for(const auto& tLink : pInterface->m_Links)
	{
		if(tLink.mInputNodeIndex == to)
		{
			if(tLink.mOutputNodeIndex == from)
				return false;
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
			continue;

		node.x += delta.x;
		node.y += delta.y;
	}
}

void CASM_Interface::tagBTDelegate::AddLink(GraphEditor::NodeIndex inputNodeIndex, GraphEditor::SlotIndex inputSlotIndex, GraphEditor::NodeIndex outputNodeIndex, GraphEditor::SlotIndex outputSlotIndex)
{
	//Link 구조체 변경으로 수정작업 필요
	MyLink tLink = {inputNodeIndex, inputSlotIndex, outputNodeIndex, outputSlotIndex};
	if(false == pInterface->Allowed_LInkEx(tLink))
		return;
	pInterface->m_Links.push_back(tLink);
	pInterface->m_Nodes[inputNodeIndex].Transitions.push_back(tLink);
}

void CASM_Interface::tagBTDelegate::DelLink(GraphEditor::LinkIndex linkIndex)
{
	//링크의 인풋 슬롯 인덱스가 작을 때, 인덱스 매칭이 이뤄지지 않아서 에러 발생
	const GraphEditor::Link tLink = pInterface->m_Links[linkIndex];
	pInterface->Delete_Transitions(tLink);
	
	pInterface->m_Templates[pInterface->m_Nodes[tLink.mInputNodeIndex].mTemplateIndex].mOutputCount--;
	if(pInterface->m_Templates[pInterface->m_Nodes[tLink.mInputNodeIndex].mTemplateIndex].mOutputCount < 1)
		pInterface->m_Templates[pInterface->m_Nodes[tLink.mInputNodeIndex].mTemplateIndex].mOutputCount = 1;

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
	//return sizeof(pInterface->m_Templates.data()) / sizeof(GraphEditor::Template);
	return pInterface->m_Templates.size();
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