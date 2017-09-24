#pragma once

#include "GLUtils.hpp"
#include <IRenderingDevice.hpp>
#include "GLShaderProgram.hpp"
#include "RenderingPassBase.hpp"

namespace Poly
{
	class World;
	struct PostprocessQuad;

	class DEVICE_DLLEXPORT GLRenderingDevice : public IRenderingDevice
	{
	private:
		enum class eGeometryRenderPassType
		{
			BLINN_PHONG,
			TRANSPARENT_GEOMETRY,
			DEBUG_NORMALS,
			TEXT_2D,
			_COUNT
		};

		enum class ePostprocessRenderPassType
		{
			VINETTE,
			BACKGROUND,
			FOREGROUND,
			BACKGROUND_LIGHT,
			FOREGROUND_LIGHT,
			_COUNT
		};

		struct InputOutputBind
		{
			InputOutputBind(const String& name, RenderingTargetBase* target) : Name(name), Target(target) {}
			String Name;
			RenderingTargetBase* Target;
		};

	public:
#if defined(_WIN32)
		GLRenderingDevice(HWND HWnd, RECT Rect);
#elif defined(__linux__)
		GLRenderingDevice(Display* display, Window window, GLXFBConfig fbConfig, const ScreenSize& size);
#elif defined(__APPLE__)
		GLRenderingDevice(void* window, const ScreenSize& size); //TODO creation API
#else
#error "Unsupported platform :("
#endif

		~GLRenderingDevice();

		GLRenderingDevice(const GLRenderingDevice&) = delete;
		void operator=(const GLRenderingDevice&) = delete;

		void Resize(const ScreenSize& size) override;
		const ScreenSize& GetScreenSize() const override { return ScreenDim; }

		void RenderWorld(World* world) override;
		void Init() override;

		std::unique_ptr<ITextureDeviceProxy> CreateTexture(size_t width, size_t height, eTextureUsageType usage) override;
		std::unique_ptr<ITextFieldBufferDeviceProxy> CreateTextFieldBuffer() override;
		std::unique_ptr<IMeshDeviceProxy> CreateMesh() override;

	private:
		void InitPrograms();
		void EndFrame();

		void CleanUpResources();

		template<typename T>
		void RegisterGeometryPass(eGeometryRenderPassType type,
			const std::initializer_list<InputOutputBind>& inputs = {},
			const std::initializer_list<InputOutputBind>& outputs = {});

		template<typename T, class... Args_t>
		void RegisterGeometryPassWithArgs(eGeometryRenderPassType type,
			const std::initializer_list<InputOutputBind>& inputs,
			const std::initializer_list<InputOutputBind>& outputs, Args_t&&... args);

		void RegisterPostprocessPass(ePostprocessRenderPassType type, const String& fragShaderName,
			const std::initializer_list<InputOutputBind>& inputs = {},
			const std::initializer_list<InputOutputBind>& outputs = {});

		template <typename T, typename... Args>
		T* CreateRenderingTarget(Args&&... args);

#if defined(_WIN32)
		HDC hDC;
		HWND hWnd;
		HGLRC hRC;
#elif defined(__linux__)
		Display* display;
		Window window;
		GLXContext context;
#elif defined(__APPLE__)
		void* window;
        void* view;
#else
#error "Unsupported platform :("
#endif

		Dynarray<std::unique_ptr<RenderingTargetBase>> RenderingTargets;

		EnumArray<std::unique_ptr<RenderingPassBase>, eGeometryRenderPassType> GeometryRenderingPasses;
		EnumArray<std::unique_ptr<RenderingPassBase>, ePostprocessRenderPassType> PostprocessRenderingPasses;

		std::unique_ptr<PostprocessQuad> PostprocessRenderingQuad;

		ScreenSize ScreenDim;
	};

	extern GLRenderingDevice* gRenderingDevice;
}

extern "C"
{
#if defined(_WIN32)
	DEVICE_DLLEXPORT Poly::IRenderingDevice* __stdcall PolyCreateRenderingDevice(HWND hwnd, RECT rect);
#elif defined(__linux__)
	DEVICE_DLLEXPORT Poly::IRenderingDevice* PolyCreateRenderingDevice(Display* display, Window window, GLXFBConfig fbConfig, const Poly::ScreenSize& size);
#elif defined(__APPLE__)
	DEVICE_DLLEXPORT Poly::IRenderingDevice* PolyCreateRenderingDevice(void* window, const Poly::ScreenSize& size);
#else
#error "Unsupported platform :("
#endif
}
