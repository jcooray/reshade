#pragma once

#include "Runtime.hpp"

#include <algorithm>
#include <d3d10_1.h>

namespace reshade
{
	namespace runtimes
	{
		class d3d10_runtime : public runtime
		{
		public:
			d3d10_runtime(ID3D10Device *device, IDXGISwapChain *swapchain);
			~d3d10_runtime();

			bool on_init(const DXGI_SWAP_CHAIN_DESC &desc);
			void on_reset() override;
			void on_reset_effect() override;
			void on_present() override;
			void on_draw_call(UINT vertices) override;
			void on_apply_effect() override;
			void on_apply_effect_technique(const technique *technique) override;
			void on_create_depthstencil_view(ID3D10Resource *resource, ID3D10DepthStencilView *depthstencil);
			void on_delete_depthstencil_view(ID3D10DepthStencilView *depthstencil);
			void on_set_depthstencil_view(ID3D10DepthStencilView *&depthstencil);
			void on_get_depthstencil_view(ID3D10DepthStencilView *&depthstencil);
			void on_clear_depthstencil_view(ID3D10DepthStencilView *&depthstencil);
			void on_copy_resource(ID3D10Resource *&dest, ID3D10Resource *&source);

			texture *get_texture(const std::string &name) const
			{
				const auto it = std::find_if(_textures.cbegin(), _textures.cend(), [name](const std::unique_ptr<texture> &it) { return it->name == name; });

				return it != _textures.cend() ? it->get() : nullptr;
			}
			const std::vector<std::unique_ptr<technique>> &GetTechniques() const
			{
				return _techniques;
			}
			void enlarge_uniform_data_storage()
			{
				_uniform_data_storage.resize(_uniform_data_storage.size() + 128);
			}
			unsigned char *get_uniform_data_storage()
			{
				return _uniform_data_storage.data();
			}
			size_t get_uniform_data_storage_size() const
			{
				return _uniform_data_storage.size();
			}
			void add_texture(texture *x)
			{
				_textures.push_back(std::unique_ptr<texture>(x));
			}
			void add_uniform(uniform *x)
			{
				_uniforms.push_back(std::unique_ptr<uniform>(x));
			}
			void add_technique(technique *x)
			{
				_techniques.push_back(std::unique_ptr<technique>(x));
			}

			ID3D10Device *_device;
			IDXGISwapChain *_swapchain;

			ID3D10Texture2D *_backbuffer_texture;
			ID3D10RenderTargetView *_backbuffer_rtv[3];
			ID3D10ShaderResourceView *_backbuffer_texture_srv[2], *_depthstencil_texture_srv;
			std::vector<ID3D10SamplerState *> _effect_sampler_states;
			std::vector<ID3D10ShaderResourceView *> _effect_shader_resources;
			ID3D10Buffer *_constant_buffer;
			UINT _constant_buffer_size;

		private:
			struct depth_source_info
			{
				UINT width, height;
				FLOAT drawcall_count, vertices_count;
			};

			void screenshot(unsigned char *buffer) const override;
			bool update_effect(const fx::nodetree &ast, const std::vector<std::string> &pragmas, std::string &errors) override;
			bool update_texture(texture *texture, const unsigned char *data, size_t size) override;

			void detect_depth_source();
			bool create_depthstencil_replacement(ID3D10DepthStencilView *depthstencil);

			bool _is_multisampling_enabled;
			DXGI_FORMAT _backbuffer_format;
			std::unique_ptr<class d3d10_stateblock> _stateblock;
			ID3D10Texture2D *_backbuffer, *_backbuffer_resolved;
			ID3D10DepthStencilView *_depthstencil, *_depthstencil_replacement;
			ID3D10Texture2D *_depthstencil_texture;
			ID3D10DepthStencilView *_default_depthstencil;
			std::unordered_map<ID3D10DepthStencilView *, depth_source_info> _depth_source_table;
			ID3D10VertexShader *_copy_vertex_shader;
			ID3D10PixelShader *_copy_pixel_shader;
			ID3D10SamplerState *_copy_sampler;
			ID3D10RasterizerState *_effect_rasterizer_state;
		};
	}
}