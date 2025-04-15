#pragma once

struct IDXGISwapChain;
class ISwapChainDx11
{
public:
	char pad_07[ 0x170 ];
	IDXGISwapChain* m_swap_chain;
};
