#include "Game.h"
#include "ResourceManager.h"
#include "TextureCube.h"
#include "UI.h"
#include "SatMath.h"

#include <vector>
#include <string>
#include <fstream>
#include <random>

Game::Game()
{
	updateTimer = new Timer();
}

Game::~Game()
{
	delete updateTimer;
}

int activeToonRamp = 0;
bool toonActive = false;

bool guiDofActive = false;
bool guiDofActiveSquare = false;
bool guiDofActiveCircle = false;
bool guiDofActiveHexagonal = false;
bool guiDofActiveStar = false;
bool pressOne = false;
bool pressTwo = false;
bool pressThree = false;
bool pressFour = false;
bool pressFive = false;

float guiDofClamp = 0.01f;
float guiDofBias = 0.20f;
float guiDofFocus = 12.0f;
float guiDofThreshold = 0.0f;

constexpr int frameTimeNumSamples = 600;
int frameTimeCurrSample = 0;
float frameTimeSamples[frameTimeNumSamples];

bool guiGammaActive = true;
float guiGammaAmount = 2.2f;

int guiViewGBuffer = 0;
bool guiViewGBufferOverride = false;

void Game::initializeGame()
{
	for (int i = 0; i < frameTimeNumSamples; ++i)
		frameTimeSamples[i] = 0.016f;
	if(guiEnabled && !UI::isInit)
		UI::InitImGUI();
	
	Camera::initCameras();
	ShaderProgram::initDefault();
	Framebuffer::initFrameBuffers();
	Material::initDefaultTextures();
	gbuffer.init(windowWidth, windowHeight);
	gbuffer.setDebugNames();
	shadowFramebuffer.addDepthTarget();
	shadowFramebuffer.init(2048, 2048);
	framebufferDeferredLight.addColorTarget(GL_R11F_G11F_B10F);
	framebufferDeferredLight.init(windowWidth, windowHeight);
	ppBuffer.setFormat(GL_R11F_G11F_B10F);
	ppBuffer.init(windowWidth, windowHeight);
	framebufferTV.addDepthTarget();
	framebufferTV.addColorTarget(GL_RGB8);
	framebufferTV.init(128, 128);

	meshIsland.loadFromObj("island.obj");
	meshUmbrella.loadFromObj("umbrella.obj");
	meshHeart.loadFromObj("heart.obj");
	meshCupcake.loadFromObj("cupcake.obj");
	meshCrystal.loadFromObj("crystal.obj");
	meshStar1.loadFromObj("star.obj");
	meshStar2.loadFromObj("star.obj");
	meshStar3.loadFromObj("star.obj");

	meshSphere.initMeshSphere(32U, 32U);
	meshSkybox.initMeshSphere(32U, 32U, true);
	meshLight.initMeshSphere(6U, 6U);

	shaderSky.load("shaderSky.vert", "shaderSky.frag");
	shaderSkyDeferred.load("shaderSky.vert", "shaderSkyDeferred.frag");
	shaderPassthrough.load("PassThrough.vert", "PassThrough.frag");
	shaderDeferredPointLight.load("PassThrough.vert", "dPointLight.frag");
	shaderDeferredDirectionalLight.load("PassThrough.vert", "dDirectionalLight.frag");
	shaderDeferredEmissive.load("PassThrough.vert", "dEmissive.frag");
	shaderPostGamma.load("PassThrough.vert", "Post/Gamma.frag");
	
	shaderPostDOFX.load("PassThrough.vert", "Post/dofHorizontal.frag");
	shaderPostDOFY.load("PassThrough.vert", "Post/dofVertical.frag");
	shaderPostDofSquare.load("PassThrough.vert", "Post/dofSquare.frag");
	shaderPostDofCircle.load("PassThrough.vert", "Post/dofCircle.frag");
	shaderPostDofHexagonal.load("PassThrough.vert", "Post/dofHexagonal.frag");
	shaderPostDofStar.load("PassThrough.vert", "Post/dofStar.frag");
	uniformBufferDOF.allocateMemory(sizeof(vec4) * 2);

	
	ResourceManager::Shaders.push_back(&shaderSky);
	ResourceManager::Shaders.push_back(&shaderPassthrough);
	ResourceManager::Shaders.push_back(&shaderDeferredPointLight);
	ResourceManager::Shaders.push_back(&shaderDeferredDirectionalLight);
	ResourceManager::Shaders.push_back(&shaderDeferredEmissive);
	ResourceManager::Shaders.push_back(&shaderSkyDeferred);
	
	uniformBufferTime.allocateMemory(sizeof(float));
	uniformBufferTime.bind(1);
	uniformBufferLightScene.allocateMemory(sizeof(vec4));
	uniformBufferLightScene.bind(2);
	
	lightKey.init();
	lightFill.init();
	lightRim.init();
	lightKey.color.xyz = vec3(1.0f, 1.0f, 0.5f) * 2.0f;
	lightFill.color.xyz = vec3(1.0f, 1.0f, 0.5f) * 0.25f;
	lightRim.color.xyz = vec3(0.5f, 0.5f, 1.0f) * 4.0f;
	lightShadow.init();
	light.init();
	light.bind();
	light.quadAtten = 0.01f;
	lights.reserve(64);
	for (int i = 0; i < 64; ++i)
	{
		lights.push_back(Light());
		lights[i].init();
		lights[i].color.rgb = random3(0.3f, 1.0f);
		lights[i].color.a = 1.0f;
		lights[i].constantAtten = 0.1f;
		lights[i].linearAtten = 0.05f;
		lights[i].quadAtten = 0.05f;
	}

	uniformBufferToon.allocateMemory(sizeof(int) * 4);
	uniformBufferToon.bind(5);
	uniformBufferToon.sendUInt(false, 0);

	uniformBufferLightScene.sendVector(vec3(0.0f), 0);

	goSun = GameObject(&meshSphere, "sun.material");
	goIsland = GameObject("island.obj", "island.material");
	goUmbrella = GameObject("umbrella.obj", "umbrella.material");
	goHeart = GameObject("heart.obj", "heart.material");
	goCupcake = GameObject("cupcake.obj", "cupcake.material");
	goCrystal = GameObject("crystal.obj", "crystal.material");
	goStar1 = GameObject("star.obj", "star1.material");
	goStar2 = GameObject("star.obj", "star2.material");
	goStar3 = GameObject("star.obj", "star3.material");


	std::vector<std::string> skyboxTex;
	skyboxTex.push_back("pink/criminal-impact_ft.tga");
	skyboxTex.push_back("pink/criminal-impact_bk.tga");
	skyboxTex.push_back("pink/criminal-impact_dn.tga");
	skyboxTex.push_back("pink/criminal-impact_up.tga");
	skyboxTex.push_back("pink/criminal-impact_rt.tga");
	skyboxTex.push_back("pink/criminal-impact_lf.tga");
	skyCubeTexture = new TextureCube(skyboxTex);
	goSkybox = GameObject(&meshSkybox, skyCubeTexture);
	goSkybox.setShaderProgram(&shaderSky);
	goSkyboxDeferred = goSkybox;
	goSkyboxDeferred.setShaderProgram(&shaderSkyDeferred);
	camera2.m_pSkybox = &goSkybox;
	camera.m_pSkybox = &goSkyboxDeferred;

	rm::addEntity(&goSun);
	rm::addEntity(&camera);
	rm::addEntity(&camera2);
	rm::addEntity(&goIsland);
	rm::addEntity(&goUmbrella);
	rm::addEntity(&goHeart);
	rm::addEntity(&goCupcake);
	rm::addEntity(&goCrystal);
	rm::addEntity(&goCrystal);
	rm::addEntity(&goStar1);
	rm::addEntity(&goStar2);
	rm::addEntity(&goStar3);


	goSun.addChild(&light);
	
	goSun.setLocalPos(vec3(-10, 13, -10));
	goIsland.setLocalPos(vec3(-5, 0, -25));
	goIsland.setLocalRot(vec3(0, 180, 0));
	goUmbrella.setLocalPos(vec3(-2, 10, -15));
	goUmbrella.setLocalRot(vec3(0, 180, 0));
	goHeart.setLocalPos(vec3(-5, 15, -20));
	goHeart.setLocalRot(vec3(-90, 0, 0));
	goCupcake.setLocalPos(vec3(-5, 4.5, -5));
	goCrystal.setLocalPos(vec3(-8, 4.5, -4));

	goStar1.setLocalPos(vec3(0,11,-6));
	goStar2.setLocalPos(vec3(-15,8,-10));
	goStar3.setLocalPos(vec3(-10,10,-20));

	goSun.setScale(1.50f);
	goIsland.setScale(1.0f);
	goUmbrella.setScale(1.0f);
	goHeart.setScale(2.0f);
	goCupcake.setScale(0.3f);
	goCrystal.setScale(2.0f);

	goStar1.setScale(1.0f);
	goStar2.setScale(0.5f);
	goStar3.setScale(1.5f);

		   	 
	// These Render flags can be set once at the start (No reason to waste time calling these functions every frame).
	// Tells OpenGL to respect the depth of the scene. Fragments will not render when they are behind other geometry.
	glEnable(GL_DEPTH_TEST); 
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	   
	// Setup our main scene objects...
	float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
	camera.perspective(90.0f, aspect, 0.1f, 1000.0f);
	camera.setLocalPos(vec3(-3.2f, 8.0f, -0.4f));
	camera.setLocalRotX(-5.0f);
	camera.attachFrameBuffer(&gbuffer);
	camera.m_pClearFlag = ClearFlag::Skybox;
	camera.setRenderList(ResourceManager::Transforms);


	camera2.perspective(60.0f, 1.0f, 0.1f, 60.0f);
	camera2.setLocalRotX(-15.0f);
	camera2.setLocalRotY(180.0f);
	camera2.attachFrameBuffer(&framebufferTV);
	camera2.m_pClearFlag = ClearFlag::SolidColor;
	camera2.setRenderList(ResourceManager::Transforms);
	
	// The camera is parented to the light, so that it can follow the light's movements
	lightShadow.setLocalRot(vec3(-30, 45, 0));

	lightKey .setLocalRot(vec3(-10, -70, 0));
	lightFill.setLocalRot(vec3(-10, 20, 0));
	lightRim .setLocalRot(vec3(-10, 140, 0));


}

void Game::update()
{
	// update our clock so we have the delta time since the last update
	updateTimer->tick();

	float deltaTime = updateTimer->getElapsedTimeSeconds();
	TotalGameTime += deltaTime;
	uniformBufferTime.sendFloat(TotalGameTime, 0);

	// Gather multiple samples of framerate
	frameTimeSamples[frameTimeCurrSample] = min(deltaTime, 0.1f);
	frameTimeCurrSample = (frameTimeCurrSample + 1) % frameTimeNumSamples;
	frameTimeSamples[frameTimeCurrSample] = 1.0f;

#pragma region movementCode
	float cameraSpeedMult = 4.0f;
	float cameraRotateSpeed = 90.0f;
	float cameraMouseSpeed = 0.15f;

	if (input.shiftL || input.shiftR)
	{
		cameraSpeedMult *= 4.0f;
	}

	if (input.ctrlL || input.ctrlR)
	{
		cameraSpeedMult *= 0.5f;
	}

	if (input.moveUp)
	{
		camera.m_pLocalPosition += camera.m_pLocalRotation.up() * cameraSpeedMult * deltaTime;
	}
	if (input.moveDown)
	{
		camera.m_pLocalPosition -= camera.m_pLocalRotation.up() * cameraSpeedMult * deltaTime;
	}
	if (input.moveForward)
	{
		camera.m_pLocalPosition -= camera.m_pLocalRotation.forward() * cameraSpeedMult * deltaTime;
	}
	if (input.moveBackward)
	{
		camera.m_pLocalPosition += camera.m_pLocalRotation.forward() * cameraSpeedMult * deltaTime;
	}
	if (input.moveRight)
	{
		camera.m_pLocalPosition += camera.m_pLocalRotation.right() *  cameraSpeedMult * deltaTime;
	}
	if (input.moveLeft)
	{
		camera.m_pLocalPosition -= camera.m_pLocalRotation.right() * cameraSpeedMult * deltaTime;
	}
	if (input.rotateUp)
	{
		camera.m_pLocalRotationEuler.x += cameraRotateSpeed * deltaTime;
		camera.m_pLocalRotationEuler.x = min(camera.m_pLocalRotationEuler.x, 85.0f);
	}
	if (input.rotateDown)
	{
		camera.m_pLocalRotationEuler.x -= cameraRotateSpeed * deltaTime;
		camera.m_pLocalRotationEuler.x = max(camera.m_pLocalRotationEuler.x, -85.0f);
	}
	if (input.rotateRight)
	{
		camera.m_pLocalRotationEuler.y -= cameraRotateSpeed * deltaTime;
	}
	if (input.rotateLeft)
	{
		camera.m_pLocalRotationEuler.y += cameraRotateSpeed * deltaTime;
	}
	if (!guiEnabled)
	{
		camera.m_pLocalRotationEuler.x -= cameraMouseSpeed * input.mouseMovement.y;
		camera.m_pLocalRotationEuler.y -= cameraMouseSpeed * input.mouseMovement.x;
		camera.m_pLocalRotationEuler.x = clamp(camera.m_pLocalRotationEuler.x, -85.0f, 85.0f);
		input.mouseMovement = vec2(0.0f);
	}
#pragma endregion movementCode

	// Make the deferred lights dance around
	for (int i = 0; i < (int)lights.size(); ++i)
	{
		lights[i].m_pLocalPosition = vec3(
			sin(i + TotalGameTime) * 0.1f + ((i / 4) % 4),
			sin(TotalGameTime) * 0.1f + (i / 16),
			cos(i + TotalGameTime) * 0.1f + (i % 4)) * vec3(60,40,-20) - vec3(30,25,0);
	}
	
	lightShadow.update(deltaTime);
	lightKey.update(deltaTime);
	lightFill.update(deltaTime);
	lightRim.update(deltaTime);

	// Make the camera move around
	camera2.setLocalPos(vec3(sin(TotalGameTime) * 10.0f, sin(TotalGameTime * 4) + 4.0f, -50.0f));

	// Rotate/Move objects here 
	goHeart.setLocalRotY(TotalGameTime * 50.0f);
	goUmbrella.setLocalPos(vec3(sin(TotalGameTime) * 10.0f, sin(TotalGameTime * 4) + 10.0f, -20.0f));
	goStar1.setLocalRotY(TotalGameTime * 1000.0f);
	goStar1.setLocalPos(vec3(2, cos(TotalGameTime) * 30.0f, sin(TotalGameTime * 4) - 20.0f));
	goStar2.setLocalRotX(TotalGameTime * 2000.0f);
	goStar2.setLocalPos(vec3(sin(TotalGameTime) * -10.0f, sin(TotalGameTime * 4) + 10.0f, -5.0f));
	goStar3.setLocalRotZ(TotalGameTime * 500.0f);

	// Give our Transforms a chance to compute the latest matrices
	for (Transform* object : ResourceManager::Transforms)
	{
		object->update(deltaTime);
	}
	goSkybox.update(deltaTime);
}

void Game::draw()
{
	glClear(GL_DEPTH_BUFFER_BIT); // Clear the backbuffer
	
	// Render the scene from both camera's perspective
	// this will render all objects that are in the camera list
	camera2.render();	
	camera.render();	
	drawLights();
	// After drawing lights, begin post processing
	drawPostProcessing();
	

#if DEBUG_LOG
	if(guiEnabled)
		GUI();	
#endif

	// Commit the Back-Buffer to swap with the Front-Buffer and be displayed on the monitor.
	glutSwapBuffers();
}

void Game::drawLights()
{
	// Clear the post processing buffer
	framebufferDeferredLight.clear();

	glDisable(GL_DEPTH_TEST); // Disable depth so we don't read/write from/to the depth
	glBlendFunc(GL_ONE, GL_ONE); // Enable additive blending for the lights
	glEnable(GL_BLEND);

	shaderDeferredPointLight.bind();
	gbuffer.bindDepthAsTexture(0);
	gbuffer.bindColorAsTexture(0, 1);
	gbuffer.bindColorAsTexture(1, 2);
	gbuffer.bindColorAsTexture(2, 3);
	gbuffer.bindResolution();

	light.bind();
	light.position = camera.getView() * vec4(light.getWorldPos(), 1.0f);
	light.sendToGPU();
	framebufferDeferredLight.renderToFSQ();

	////////////////////////
   // START SHADOWS HERE //
  ////////////////////////
	shaderDeferredDirectionalLight.bind();

	// Don't forget to convert it into viewspace!
	lightShadow.bind();
	lightShadow.direction.xyz = normalize((camera.getView() * lightShadow.getLocalToWorld()).forward());
	lightShadow.sendToGPU();
	// Draw the directional light!
	framebufferDeferredLight.renderToFSQ();
	shaderDeferredDirectionalLight.unbind();
	/////////////////////////
   // FINISH SHADOWS HERE //
  /////////////////////////
	
	shaderDeferredDirectionalLight.bind();
	lightKey.bind();
	lightKey.direction.xyz = normalize((camera.getView() * lightKey.getLocalToWorld()).forward());
	lightKey.sendToGPU();
	// Draw the directional light!
	framebufferDeferredLight.renderToFSQ();
	lightFill.bind();
	lightFill.direction.xyz = normalize((camera.getView() * lightFill.getLocalToWorld()).forward());
	lightFill.sendToGPU();
	// Draw the directional light!
	framebufferDeferredLight.renderToFSQ();
	lightRim.bind();
	lightRim.direction.xyz = normalize((camera.getView() * lightRim.getLocalToWorld()).forward());
	lightRim.sendToGPU();
	// Draw the directional light!
	framebufferDeferredLight.renderToFSQ();
	shaderDeferredDirectionalLight.unbind();

  // Add on the emissive buffer
	shaderDeferredEmissive.bind();
	skyCubeTexture->bind(6);
	framebufferDeferredLight.renderToFSQ();
	skyCubeTexture->unbind(6);

	// Unbind gbuffer textures
	gbuffer.unbindTexture(5);
	gbuffer.unbindTexture(4);
	gbuffer.unbindTexture(3);
	gbuffer.unbindTexture(2);
	gbuffer.unbindTexture(1);
	gbuffer.unbindTexture(0);

	glDisable(GL_BLEND); // Disable blending so everything else can draw
}

void Game::drawPostProcessing()
{
	shaderPassthrough.bind();
	framebufferDeferredLight.bindColorAsTexture(0, 0);
	ppBuffer.drawToPost();
	shaderPassthrough.unbind();
	
#pragma region depthOfFieldPost

	if (guiDofActive)
	{
		gbuffer.bindDepthAsTexture(1);

		float a = (camera.getFarPlane() + camera.getNearPlane()) / (camera.getFarPlane() - camera.getNearPlane());
		float b = -2 * (camera.getFarPlane() * camera.getNearPlane()) / (camera.getFarPlane() - camera.getNearPlane()) / guiDofFocus;
		float focusDepth = (a + b) * 0.5f + 0.5f;

		shaderPostDOFX.bind();
		shaderPostDOFX.sendUniform("uBlurClamp", guiDofClamp * 0.2f);
		shaderPostDOFX.sendUniform("uBias", guiDofBias);
		shaderPostDOFX.sendUniform("uFocus", focusDepth);
		shaderPostDOFX.sendUniform("uThreshold", guiDofThreshold);
		ppBuffer.draw();
		shaderPostDOFX.unbind(); // unbind shader

		shaderPostDOFY.bind();
		shaderPostDOFY.sendUniform("uBlurClamp", guiDofClamp * 0.2f);
		shaderPostDOFY.sendUniform("uBias", guiDofBias);
		shaderPostDOFY.sendUniform("uFocus", focusDepth);
		shaderPostDOFY.sendUniform("uThreshold", guiDofThreshold);
		ppBuffer.draw();
		shaderPostDOFY.unbind(); // unbind shader

		// Unbind Depth
		gbuffer.unbindTexture(1);
	}

	
#pragma endregion depthOfFieldPost

#pragma region depthOfFieldPostSquare

	if (guiDofActiveSquare)
	{
		gbuffer.bindDepthAsTexture(1);

		float a = (camera.getFarPlane() + camera.getNearPlane()) / (camera.getFarPlane() - camera.getNearPlane());
		float b = -2 * (camera.getFarPlane() * camera.getNearPlane()) / (camera.getFarPlane() - camera.getNearPlane()) / guiDofFocus;
		float focusDepth = (a + b) * 0.5f + 0.5f;

		shaderPostDofSquare.bind();
		shaderPostDofSquare.sendUniform("uBlurClamp", guiDofClamp * 0.2f);
		shaderPostDofSquare.sendUniform("uBias", guiDofBias);
		shaderPostDofSquare.sendUniform("uFocus", focusDepth);
		shaderPostDofSquare.sendUniform("uFocalLength", guiDofThreshold);
		ppBuffer.draw();
		shaderPostDofSquare.unbind(); // unbind shader

		// Unbind Depth
		gbuffer.unbindTexture(1);
	}

#pragma endregion depthOfFieldPostSquare

#pragma region depthOfFieldPostCircle

	if (guiDofActiveCircle)
	{
		gbuffer.bindDepthAsTexture(1);

		float a = (camera.getFarPlane() + camera.getNearPlane()) / (camera.getFarPlane() - camera.getNearPlane());
		float b = -2 * (camera.getFarPlane() * camera.getNearPlane()) / (camera.getFarPlane() - camera.getNearPlane()) / guiDofFocus;
		float focusDepth = (a + b) * 0.5f + 0.5f;

		shaderPostDofCircle.bind();
		shaderPostDofCircle.sendUniform("uBlurClamp", guiDofClamp * 0.2f);
		shaderPostDofCircle.sendUniform("uBias", guiDofBias);
		shaderPostDofCircle.sendUniform("uFocus", focusDepth);
		shaderPostDofCircle.sendUniform("uFocalLength", guiDofThreshold);
		ppBuffer.draw();
		shaderPostDofCircle.unbind(); // unbind shader

		// Unbind Depth
		gbuffer.unbindTexture(1);
	}

#pragma endregion depthOfFieldPostCircle

#pragma region depthOfFieldPostHexagonal

	if (guiDofActiveHexagonal)
	{
		gbuffer.bindDepthAsTexture(1);

		float a = (camera.getFarPlane() + camera.getNearPlane()) / (camera.getFarPlane() - camera.getNearPlane());
		float b = -2 * (camera.getFarPlane() * camera.getNearPlane()) / (camera.getFarPlane() - camera.getNearPlane()) / guiDofFocus;
		float focusDepth = (a + b) * 0.5f + 0.5f;

		shaderPostDofHexagonal.bind();
		shaderPostDofHexagonal.sendUniform("uBlurClamp", guiDofClamp * 0.2f);
		shaderPostDofHexagonal.sendUniform("uBias", guiDofBias);
		shaderPostDofHexagonal.sendUniform("uFocus", focusDepth);
		shaderPostDofHexagonal.sendUniform("uFocalLength", guiDofThreshold);
		ppBuffer.draw();
		shaderPostDofHexagonal.unbind(); // unbind shader

		// Unbind Depth
		gbuffer.unbindTexture(1);
	}

#pragma endregion depthOfFieldPostHexagonal

#pragma region depthOfFieldPostStar

	if (guiDofActiveStar)
	{
		gbuffer.bindDepthAsTexture(1);

		float a = (camera.getFarPlane() + camera.getNearPlane()) / (camera.getFarPlane() - camera.getNearPlane());
		float b = -2 * (camera.getFarPlane() * camera.getNearPlane()) / (camera.getFarPlane() - camera.getNearPlane()) / guiDofFocus;
		float focusDepth = (a + b) * 0.5f + 0.5f;

		shaderPostDofStar.bind();
		shaderPostDofStar.sendUniform("uBlurClamp", guiDofClamp * 0.2f);
		shaderPostDofStar.sendUniform("uBias", guiDofBias);
		shaderPostDofStar.sendUniform("uFocus", focusDepth);
		shaderPostDofStar.sendUniform("uFocalLength", guiDofThreshold);
		ppBuffer.draw();
		shaderPostDofStar.unbind(); // unbind shader

		// Unbind Depth
		gbuffer.unbindTexture(1);
	}

#pragma endregion depthOfFieldPostStar

#pragma region gammaPost

	if (guiGammaActive)
	{
		shaderPostGamma.bind();
		shaderPostGamma.sendUniform("uGamma", guiGammaAmount);
		ppBuffer.draw();
		shaderPostGamma.unbind();
	}

#pragma endregion gammaPost

	shaderPassthrough.bind();
	framebufferDeferredLight.unbind();
	ppBuffer.drawToScreen();
	shaderPassthrough.unbind();
	
	if (guiViewGBufferOverride)
	{
		shaderPassthrough.bind();
		gbuffer.bindColorAsTexture(guiViewGBuffer, 0);
		Framebuffer::drawFSQ();
		shaderPassthrough.unbind();
	}

	glEnable(GL_DEPTH_TEST);
}

void Game::GUI()
{
	UI::Start(windowWidth, windowHeight);
	
	// Framerate Visualizer
	ImGui::Begin("Framerate");
	{
		float averageFramerate = 0.0f;
		for (int i = 0; i < frameTimeNumSamples; ++i)
			averageFramerate += frameTimeSamples[i];
		averageFramerate = 1.0f / ((averageFramerate-1.0f) / (frameTimeNumSamples-1));
		std::string framerate = "Framerate: " + std::to_string(averageFramerate);

		ImGui::PlotHistogram("", frameTimeSamples, frameTimeNumSamples, 0, framerate.c_str(), 0.005f, 0.2f, ImVec2(frameTimeNumSamples, 32));
	}
	ImGui::End();

	if (ImGui::TreeNode("Depth Of Field"))
	{
		ImGui::SliderFloat("Clamp Amount", &guiDofClamp, 0.0f, 1.0f, "%.5f", 2.0f);
		ImGui::SliderFloat("Bias Amount", &guiDofBias, 0.0f, 30.0f, "%.5f", 2.0f);
		ImGui::SliderFloat("Focus", &guiDofFocus, 0.0f, 150.0f, "%.5f", 2.0f);
		ImGui::SliderFloat("Threshold", &guiDofThreshold, -10.0f, 30.0f, "%.5f", 2.0f);
		ImGui::TreePop();
	}

	UI::End();
}

void Game::keyboardDown(unsigned char key, int mouseX, int mouseY)
{
	if (guiEnabled)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[(int)key] = true;
		io.AddInputCharacter((int)key); // this is what makes keyboard input work in imgui
		// This is what makes the backspace button work
		int keyModifier = glutGetModifiers();
		switch (keyModifier)
		{
		case GLUT_ACTIVE_SHIFT:
			io.KeyShift = true;
			break;

		case GLUT_ACTIVE_CTRL:
			io.KeyCtrl = true;
			break;

		case GLUT_ACTIVE_ALT:
			io.KeyAlt = true;
			break;
		}
	}

	switch(key)
	{
	case 27: // the escape key
		break;
	case 'e':
	case 'E':
	case 'e' - 96:
		input.moveUp = true;
		break;
	case 'q':
	case 'Q':
	case 'q' - 96:
		input.moveDown = true;
		break;
	case 'l':
	case 'L':
	case 'l' - 96:
		input.rotateRight = true;
		break;
	case 'j':
	case 'J':
	case 'j' - 96:
		input.rotateLeft = true;
		break;
	case 'i':
	case 'I':
	case 'i' - 96:
		input.rotateUp = true;
		break;
	case 'k':
	case 'K':
	case 'k' - 96:
		input.rotateDown = true;
		break;
	/////////////////////////////////////////////////////////////////////// 
	case '0':
		pressOne = false;
		pressTwo = false;
		pressThree = false;
		pressFour = false;
		pressFive = false;
		guiDofActive = false;
		guiDofActiveSquare = false;
		guiDofActiveCircle = false;
		guiDofActiveHexagonal = false;
		guiDofActiveStar = false;
		break;
	case '1':
		if (!pressOne)
		{
			pressOne = true;
			pressTwo = false;
			pressThree = false;
			pressFour = false;
			pressFive = false;
			guiDofActive = true;
			guiDofActiveSquare = false;
			guiDofActiveCircle = false;
			guiDofActiveHexagonal = false;
			guiDofActiveStar = false;
			guiDofClamp = 0.02f;
			guiDofBias = 23.34f;
			guiDofFocus = 14.31f;
			guiDofThreshold = 0.043f;
		}
		else
		{
			pressOne = false;
			pressTwo = false;
			pressThree = false;
			pressFour = false;
			pressFive = false;
			guiDofActive = false;
			guiDofActiveSquare = false;
			guiDofActiveCircle = false;
			guiDofActiveHexagonal = false;
			guiDofActiveStar = false;
		}
		break;
	case '2':
		if (!pressTwo)
		{
			pressTwo = true;
			pressOne = false;
			pressThree = false;
			pressFour = false;
			pressFive = false;
			guiDofActive = false;
			guiDofActiveSquare = true;
			guiDofActiveCircle = false;
			guiDofActiveHexagonal = false;
			guiDofActiveStar = false;
			guiDofClamp = 0.207f;
			guiDofBias = 30.0f;
			guiDofFocus = 18.33f;
			guiDofThreshold = 30.0f;
		}
		else
		{
			pressTwo = false;
			pressOne = false;
			pressThree = false;
			pressFour = false;
			pressFive = false;
			guiDofActive = false;
			guiDofActiveSquare = false;
			guiDofActiveCircle = false;
			guiDofActiveHexagonal = false;
			guiDofActiveStar = false;
		}
		break;
	case '3':
		if (!pressThree)
		{
			pressThree = true;
			pressOne = false;
			pressTwo = false;
			pressFour = false;
			pressFive = false;
			guiDofActive = false;
			guiDofActiveSquare = false;
			guiDofActiveCircle = true;
			guiDofActiveHexagonal = false;
			guiDofActiveStar = false;
			guiDofClamp = 0.207f;
			guiDofBias = 30.0f;
			guiDofFocus = 18.33f;
			guiDofThreshold = 30.0f;
		}
		else
		{
			pressThree = false;
			pressOne = false;
			pressTwo = false;
			pressFour = false;
			pressFive = false;
			guiDofActive = false;
			guiDofActiveSquare = false;
			guiDofActiveCircle = false;
			guiDofActiveHexagonal = false;
			guiDofActiveStar = false;
		}
		break;
	case '4':
		if (!pressFour)
		{
			pressFour = true;
			pressOne = false;
			pressTwo = false;
			pressThree = false;
			pressFive = false;
			guiDofActive = false;
			guiDofActiveSquare = false;
			guiDofActiveCircle = false;
			guiDofActiveHexagonal = true;
			guiDofActiveStar = false;
			guiDofClamp = 0.207f;
			guiDofBias = 30.0f;
			guiDofFocus = 18.33f;
			guiDofThreshold = 30.0f;
		}
		else
		{
			pressFour = false;
			pressOne = false;
			pressTwo = false;
			pressThree = false;
			pressFive = false;
			guiDofActive = false;
			guiDofActiveSquare = false;
			guiDofActiveCircle = false;
			guiDofActiveHexagonal = false;
			guiDofActiveStar = false;
		}
		break;
	case '5':
		if (!pressFive)
		{
			pressFive = true;
			pressOne = false;
			pressTwo = false;
			pressThree = false;
			pressFour = false;
			guiDofActive = false;
			guiDofActiveSquare = false;
			guiDofActiveCircle = false;
			guiDofActiveHexagonal = false;
			guiDofActiveStar = true;
			guiDofClamp = 0.207f;
			guiDofBias = 30.0f;
			guiDofFocus = 18.33f;
			guiDofThreshold = 30.0f;
		}
		else
		{
			pressFive = false;
			pressOne = false;
			pressTwo = false;
			pressThree = false;
			pressFour = false;
			guiDofActive = false;
			guiDofActiveSquare = false;
			guiDofActiveCircle = false;
			guiDofActiveHexagonal = false;
			guiDofActiveStar = false;
		}
		break;
	case 'a':
		guiDofBias -= 1.0f;
		break;
	case 'A':
		guiDofBias += 1.0f;
		break;
	case '-':
		guiDofFocus -= 1.0f;
		break;
	case '+':
		guiDofFocus += 1.0f;
		break;
	case 'n':
		guiDofThreshold -= 1.0f;
		break;
	case 'N':
		guiDofThreshold += 1.0f;
		break;
	case 'f':
		guiDofClamp -= 1.0f;
		break;
	case 'F':
		guiDofClamp += 1.0f;
		break;
	}
	////////////////////////////////////////////////////////////////////////////
}

void Game::keyboardUp(unsigned char key, int mouseX, int mouseY)
{
	if (guiEnabled)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[key] = false;

		int keyModifier = glutGetModifiers();
		io.KeyShift = false;
		io.KeyCtrl = false;
		io.KeyAlt = false;
		switch (keyModifier)
		{
		case GLUT_ACTIVE_SHIFT:
			io.KeyShift = true;
			break;

		case GLUT_ACTIVE_CTRL:
			io.KeyCtrl = true;
			break;

		case GLUT_ACTIVE_ALT:
			io.KeyAlt = true;
			break;
		}
	}

	switch(key)
	{
	case 32: // the space bar
		camera.cullingActive = !camera.cullingActive;
		break;
	case 27: // the escape key
		exit(1);
		break;
	case 'w':
	case 'W':
	case 'w' - 96:
		input.moveForward = false;
		break;
	case 's':
	case 'S':
	case 's' - 96:
		input.moveBackward = false;
		break;
	case 'd':
	case 'D':
	case 'd' - 96:
		input.moveRight = false;
		break;
	case 'a':
	case 'A':
	case 'a' - 96:
		input.moveLeft = false;
		break;
	case 'e':
	case 'E':
	case 'e' - 96:
		input.moveUp = false;
		break;
	case 'q':
	case 'Q':
	case 'q' - 96:
		input.moveDown = false;
		break;
	case 'l':
	case 'L':
	case 'l' - 96:
		input.rotateRight = false;
		break;
	case 'j':
	case 'J':
	case 'j' - 96:
		input.rotateLeft = false;
		break;
	case 'i':
	case 'I':
	case 'i' - 96:
		input.rotateUp = false;
		break;
	case 'k':
	case 'K':
	case 'k' - 96:
		input.rotateDown = false;
		break;
	}
}

void Game::keyboardSpecialDown(int key, int mouseX, int mouseY)
{
	switch (key)
	{
	case GLUT_KEY_F1:
		guiEnabled = !guiEnabled;
		if (guiEnabled)
		{
			glutWarpPointer((int)input.mousePosGUI.x, (int)input.mousePosGUI.y);
			glutSetCursor(GLUT_CURSOR_INHERIT);
		}
		else 
		{
			input.f11 = true;
			glutWarpPointer(windowWidth / 2, windowHeight / 2);
			glutSetCursor(GLUT_CURSOR_NONE);
		}
		if (!UI::isInit)
		{
			UI::InitImGUI();
		}
		break;
	case GLUT_KEY_F5:
		for (ShaderProgram* shader : ResourceManager::Shaders)
		{
			shader->reload();
		}
		break;
	case GLUT_KEY_CTRL_L:
		input.ctrlL = true;
		break;
	case GLUT_KEY_CTRL_R:
		input.ctrlR = true;
		break;
	case GLUT_KEY_SHIFT_L:
		input.shiftL = true;
		break;
	case GLUT_KEY_SHIFT_R:
		input.shiftR = true;
		break;
	case GLUT_KEY_ALT_L:
		input.altL = true;
		break;
	case GLUT_KEY_ALT_R:
		input.altR = true;
		break;
	case GLUT_KEY_UP:
		input.moveForward = true;
		break;
	case GLUT_KEY_DOWN:
		input.moveBackward = true;
		break;
	case GLUT_KEY_RIGHT:
		input.moveRight = true;
		break;
	case GLUT_KEY_LEFT:
		input.moveLeft = true;
		break;
	case GLUT_KEY_PAGE_UP:
		input.moveUp = true;
		break;
	case GLUT_KEY_PAGE_DOWN:
		input.moveDown = true;
		break;
	case GLUT_KEY_END:
		exit(1);
		break;
	}
}

void Game::keyboardSpecialUp(int key, int mouseX, int mouseY)
{
	switch (key)
	{
	case GLUT_KEY_CTRL_L:
		input.ctrlL = false;
		break;
	case GLUT_KEY_CTRL_R:
		input.ctrlR = false;
		break;
	case GLUT_KEY_SHIFT_L:
		input.shiftL = false;
		break;
	case GLUT_KEY_SHIFT_R:
		input.shiftR = false;
		break;
	case GLUT_KEY_ALT_L:
		input.altL = false;
		break;
	case GLUT_KEY_ALT_R:
		input.altR = false;
		break;
	case GLUT_KEY_UP:
		input.moveForward = false;
		break;
	case GLUT_KEY_DOWN:
		input.moveBackward = false;
		break;
	case GLUT_KEY_RIGHT:
		input.moveRight = false;
		break;
	case GLUT_KEY_LEFT:
		input.moveLeft = false;
		break;
	case GLUT_KEY_PAGE_UP:
		input.moveUp = false;
		break;
	case GLUT_KEY_PAGE_DOWN:
		input.moveDown = false;
		break;
	}
}

void Game::mouseClicked(int button, int state, int x, int y)
{
	if (guiEnabled)
	{
		ImGui::GetIO().MousePos = ImVec2((float)x, (float)y);
		ImGui::GetIO().MouseDown[0] = !state;
		if (state == GLUT_DOWN)
		{
			if (button == 3)
				ImGui::GetIO().MouseWheel += 1.0f;
			else if (button == 4)
				ImGui::GetIO().MouseWheel -= 1.0f;

		}
	}
	else
	{
	}

	if(state == GLUT_DOWN) 
	{
		switch(button)
		{
		case GLUT_LEFT_BUTTON:

			break;
		case GLUT_RIGHT_BUTTON:
		
			break;
		case GLUT_MIDDLE_BUTTON:

			break;
		}
	}
	else
	{

	}
}

void Game::mouseMoved(int x, int y)
{
	if (guiEnabled)
	{
		ImGui::GetIO().MousePos = ImVec2((float)x, (float)y);

		if (!ImGui::GetIO().WantCaptureMouse)
		{

		}
	}

	
}

void Game::mousePassive(int x, int y)
{
	if (guiEnabled)
	{
		ImGui::GetIO().MousePos = ImVec2((float)x, (float)y);
		input.mousePosGUI = vec2((float)x, (float)y);
	}
	else
	{
		if (input.f11)
		{
			input.f11 = false;
			input.mousePos = vec2((float)(windowWidth / 2), (float)(windowHeight / 2));
			input.mousePosOld = input.mousePos;
		}
		else
		{
			input.mousePos = vec2((float)x, (float)y);
			input.mouseMovement += input.mousePos - input.mousePosOld;
			input.mousePosOld = input.mousePos;
		}

		if (x < 100 || y < 100 || x > windowWidth - 100 || y > windowHeight - 100)
		{
			glutWarpPointer(windowWidth / 2, windowHeight / 2);
			input.mousePosOld = vec2((float)(windowWidth / 2), (float)(windowHeight / 2));
			input.f11 = true;
		}
	}
}

void Game::reshapeWindow(int w, int h)
{
	windowWidth = w;
	windowHeight = h;

	float aspect = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
	camera.perspective(90.0f, aspect, 0.05f, 1000.0f);
	glViewport(0, 0, w, h);
	framebufferDeferredLight.reshape(w, h);
	gbuffer.reshape(w, h);
	gbuffer.setDebugNames();
	ppBuffer.reshape(w, h);
}
