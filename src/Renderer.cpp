#include "Renderer.h"

void Renderer::init(Window::WindowData &_winData) noexcept
{
	Window::WindowResize windowResize;

	glfwSetWindowUserPointer(m_window, &windowResize);

	const auto &surfaceCreateCallback = [=]()
	{
		return glfwCreateWindowSurface(
			m_device->getData().vkInstance, m_window,
			nullptr, &m_device->getData().surface
		);
	};
	const auto &surfaceExtensions = getSurfaceExtensions();

	m_device->createSurface(
		surfaceCreateCallback,
		surfaceExtensions,
		_winData.width, _winData.height
	);
	m_device->createDevice();

	initSwapchain();

	glfwSetFramebufferSizeCallback(m_window, Window::WindowResize::resize);

}

void Renderer::initSwapchain() noexcept
{
	auto &deviceData = m_device->getData();
	auto &swapchainData   = deviceData.swapchainData;
	auto &swapchainImages = swapchainData.images;

	m_device->createSwapchainData(swapchainData);

	const auto &swapchainSize = swapchainImages.size();
	for(auto i = 0u; i < swapchainSize; i++)
	{
		vk::Image::createImageView(
			deviceData.logicalDevice,
			swapchainImages[i],
			swapchainData.format,
			swapchainData.imageViews[i]
		);
	}
}