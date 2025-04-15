// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

#include "tex_dict_dx11.hpp"
#include <wrl/client.h>

#include <assert.h>

#pragma optimize("", off)

using namespace dx11;
using Microsoft::WRL::ComPtr;

void tex_wrapper_dx11::create(ID3D11Device* device)
{
	if (_texture)
		_texture->Release();
	if (_res_view)
		_res_view->Release();
	_texture = nullptr;
	_res_view = nullptr;

	auto desc = D3D11_TEXTURE2D_DESC{};
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = _size_x;
	desc.Height = _size_y;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA subResource{};
	subResource.pSysMem = _texture_data.data();
	subResource.SysMemPitch = desc.Width * 4;
	subResource.SysMemSlicePitch = 0;
	auto res = device->CreateTexture2D(&desc, &subResource, &_texture);

	auto srv_desc = D3D11_SHADER_RESOURCE_VIEW_DESC{};
	srv_desc.Format = desc.Format;
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MipLevels = desc.MipLevels;
	srv_desc.Texture2D.MostDetailedMip = 0;
	res = device->CreateShaderResourceView(_texture, &srv_desc, &_res_view);
}

bool tex_wrapper_dx11::set_tex_data(tex_dict_dx11* dict, ID3D11Device* device, const uint8_t* data, const uint32_t size_x,
	const uint32_t size_y)
{
	_texture_data.resize(size_x * size_y * 4);
	std::copy(data, data + _texture_data.size(), _texture_data.begin());

	auto changed = _size_x != size_x || _size_y != size_y;

	_size_x = size_x;
	_size_y = size_y;
	if (!_texture || changed)
	{
		create(device);
	}

	std::scoped_lock g{dict->_update_queue_lock};
	if (std::find(dict->_update_queue.begin(), dict->_update_queue.end(), this) == dict->_update_queue.end()) {
		dict->_update_queue.push_back(this);
	}

	return true;
}

bool tex_wrapper_dx11::apply_tex_changes(ID3D11DeviceContext* ctx) {
	return copy_texture_data(ctx);
}

bool tex_wrapper_dx11::copy_texture_data(ID3D11DeviceContext* ctx)
{
	auto box = D3D11_BOX{};
	box.left = 0;
	box.top = 0;
	box.front = 0;
	box.back = 1;
	box.right = _size_x;
	box.bottom = _size_y;

	ctx->UpdateSubresource(_texture, 0, &box, _texture_data.data(), _size_x * 4, _size_x * _size_y * 4);

	return true;
}

void tex_dict_dx11::clear_textures()
{
	std::scoped_lock g(_mutex);
	for (auto& tex : _textures)
		if (!tex.free)
			tex.clear_data();
	_textures.clear();
}

tex_wrapper_dx11* tex_dict_dx11::create_texture(uint32_t size_x, uint32_t size_y)
{
	std::scoped_lock g(_mutex);

	_textures.emplace_front();
	const auto ptr = &_textures.front();
	_valid_elements.emplace_back(ptr);
	return ptr;
}

void tex_dict_dx11::destroy_texture(tex_wrapper_dx11* tex)
{
	std::scoped_lock g(_mutex);
	if (!is_valid_tex(tex))
		return;

	tex->clear_data();
	_valid_elements.erase(std::find(_valid_elements.begin(), _valid_elements.end(), tex));
	_textures.remove_if([tex](const auto& elem) { return &elem == tex; });
}

void tex_dict_dx11::process_update_queue(ID3D11DeviceContext* ctx) {
	std::scoped_lock g{_update_queue_lock};
	for (auto entry : _update_queue) {
		entry->apply_tex_changes(ctx);
	}
	_update_queue.clear();
}

#pragma optimize("", on)