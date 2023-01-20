#include <fractal/fractal.hpp>

namespace frac {
	void MainWindow::setup() {
		FRAC_LOG("Setup Called");

		// Set framerate
		setFrameRate(-1);
		ci::gl::enableVerticalSync(true);

		// Setup ImGui
		ImGui::Initialize();
		ImGui::StyleColorsDark();
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		ImGui::GetIO().FontGlobalScale = 1.0f;

		// Set up rendering settings
		ci::gl::enableDepthWrite();
		ci::gl::enableDepthRead();

		// Load the settings file
		FRAC_LOG(fmt::format("Loading settings from {}", FRACTAL_SETTINGS_PATH));
		std::fstream settingsFile(FRACTAL_SETTINGS_PATH, std::ios::in);
		if (settingsFile.is_open()) {
			settingsFile >> m_settings;
			settingsFile.close();
		} else {
			FRAC_ERROR("Failed to open settings file");
			quit();
		}

		// Validate settings
		if (m_settings["menus"].is_null()) {
			FRAC_ERROR("Settings file does not contain a \"menus\" object");
			quit();
		}

		if (m_settings["menus"]["fractalInfo"].is_null()) {
			FRAC_ERROR("Settings file does not contain a \"menus.fractalInfo\" object");
			quit();
		}
	}

	void MainWindow::draw() {
		ci::gl::clear(ci::Color(0.2f, 0.2f, 0.2f));

		drawImGui();
	}

	void MainWindow::drawImGui() {
		// Fractal Information Window
		json fractalInfo = m_settings["menus"]["fractalInfo"];
		FRAC_LOG(fractalInfo.dump(4));
		ImGui::SetNextWindowPos({(float)fractalInfo["posX"], (float)fractalInfo["posY"]},
								ImGuiCond_Once);
		ImGui::SetNextWindowSize({(float)fractalInfo["width"], (float)fractalInfo["height"]},
								 ImGuiCond_Once);
		ImGui::Begin("Fractal Info", nullptr);
		{
			ImGui::Text("Fractal Type: Mandelbrot");
			ImGui::Text("%s", fmt::format("Re:   {}", 0).c_str());
			ImGui::Text("%s", fmt::format("Im:   {}", 0).c_str());
			ImGui::Text("%s", fmt::format("Zoom: {}x", 1).c_str());
		}
		ImGui::End();
	}

	void MainWindow::mouseMove(ci::app::MouseEvent event) { m_mousePos = event.getPos(); }
} // namespace frac

/*

class TestClass:
	def __init__(self):
		self.test = 0

	def testFunc(self):
		print("Test")

 class Class2(TestClass):
	def __init__(self):
		super().__init__()
		self.test2 = 0

 */