#pragma once

struct ImDrawList;


struct ID3D11Device;

namespace blur
{
	void initialize(ImDrawList* drawList, ID3D11DeviceContext* pContext);
	void InitResources(ID3D11Device* pDevice);

}