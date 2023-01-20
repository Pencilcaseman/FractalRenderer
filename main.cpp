#include <cinderbox/cinderbox.hh>
#include <iostream>
#include <librapid/librapid.hpp>

#include "glsl-optimizer/src/glsl/glsl_optimizer.h"

#include <fractal/fractal.hpp>
#include <fractal/debug.hpp>

namespace lrc = librapid;

static glslopt_ctx *glslGlobalContext = nullptr;

static bool init(glslopt_target target = kGlslTargetOpenGL) {
	glslGlobalContext = glslopt_initialize(target);
	if (!glslGlobalContext) return false;
	return true;
}

static void shutdown() {
	if (glslGlobalContext) {
		glslopt_cleanup(glslGlobalContext);
		glslGlobalContext = nullptr;
	}
}

std::string loadFile(const std::string &filename) {
	std::fstream file(filename, std::fstream::in);
	std::stringstream stream;
	stream << file.rdbuf();
	return stream.str();
}

bool saveFile(const std::string &filename, const std::string &data) {
	std::fstream file(filename, std::fstream::out);
	if (!file.is_open()) return false;
	file << data;
	return true;
}

static bool compileShader(const std::string &dstFilename, const std::string &srcFilename,
						  bool vertexShader) {
	const std::string originalShader = loadFile(srcFilename);

	const glslopt_shader_type type = vertexShader ? kGlslOptShaderVertex : kGlslOptShaderFragment;

	glslopt_shader *shader = glslopt_optimize(glslGlobalContext, type, originalShader.c_str(), 0);
	if (!glslopt_get_status(shader)) {
		FRAC_WARN(fmt::format("Failed to compile shader: {}", glslopt_get_log(shader)));
		return false;
	}

	const std::string optimizedShader(glslopt_get_output(shader));

	if (!saveFile(dstFilename, optimizedShader)) return false;
	return true;
}

class MainWindow : public ci::app::App {
public:
	MainWindow() = default;

	void setup() override {
		FRAC_LOG("Hello, world!");

		init();

		m_cam.lookAt(lrc::Vec3f(0, 0, -1), lrc::Vec3f(0, 0, 0));

		setFrameRate(-1);
		ci::gl::enableVerticalSync(true);
		addAssetDirectory(FRACTAL_RENDERER_ROOT_DIR "/shaders");

		// optimise shaders
		compileShader(FRACTAL_RENDERER_ROOT_DIR "/shaders/optimised/mandelbrotFrag.frag",
					  FRACTAL_RENDERER_ROOT_DIR "/shaders/mandelbrotFrag.frag",
					  false);

		try {
			m_glsl = ci::gl::GlslProg::create(loadAsset("genericVertex.vert"),
											  loadAsset("mandelbrotFrag.frag"));
		} catch (std::exception &e) {
			FRAC_ERROR(fmt::format("Failed to load common textures and shaders: {}", e.what()));
			quit();
		}

		m_cube = ci::gl::Batch::create(ci::geom::Rect(ci::Rectf(-0.5, -0.5, 1, 1)), m_glsl);

		ci::gl::enableDepthWrite();
		ci::gl::enableDepthRead();
	}

	// Run on shutdown
	void cleanup() override { shutdown(); }

	void draw() override {
		FRAC_LOG("Hello, World");

		ci::gl::clear(ci::Color(0.2f, 0.2f, 0.2f));

		ci::gl::setMatrices(m_cam);

		ci::vec2 mousePos(getMousePos().x, getMousePos().y);
		mousePos -= getWindowPos();
		mousePos /= getWindowSize();
		mousePos -= ci::vec2(0.5, 0.5);
		mousePos.y *= -1; // Make y positive up

		// Set resolution uniform
		m_glsl->uniform("u_resolution", ci::vec2 {getWindowWidth(), getWindowHeight()});
		m_glsl->uniform("u_mouse", mousePos);

		// GLint mousePos = glGetUniformLocation(m_glsl->getHandle(), "mousePos");
		// glUniform2f(mousePos, float(getMousePos().x), float(getMousePos().y));
		m_cube->draw();

		// Draw the frame rate on the screen
		ci::gl::setMatricesWindow(getWindowSize());
		std::string frameString = "Frame Rate: " + std::to_string(getAverageFps());
		ci::gl::drawStringCentered(
		  frameString, ci::vec2(getWindowWidth() / 2, 50), ci::ColorA(0.5, 0.5, 0.5, 1), m_font);
		ci::gl::drawStringCentered(fmt::format("Mouse: {:.3f} {:.3f}\n", mousePos.x, mousePos.y),
								   ci::vec2(getWindowWidth() / 2, 100),
								   ci::ColorA(0.5, 0.5, 0.5, 1),
								   m_font);
	}

	ci::CameraPersp m_cam;
	ci::gl::BatchRef m_cube;
	ci::gl::GlslProgRef m_glsl;
	ci::Font m_font = ci::Font("Arial", 24);
};

CINDER_APP(MainWindow, ci::app::RendererGl(ci::app::RendererGl::Options().msaa(4)))
