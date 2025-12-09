// skeletal_animation.cpp

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/animator.h>
#include <learnopengl/model_animation.h>
#include <glm/gtx/string_cast.hpp>


#include <iostream>


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 800;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// movement
glm::vec3 charPosition_p1 = glm::vec3(0.0f, 0.0f, -2.0f);
glm::vec3 charPosition_p2 = glm::vec3(0.0f, 0.0f, 2.0f);
glm::vec3 charFront_p1 = glm::vec3(0.0f, 0.0f, 1.0f); // initial forward
glm::vec3 charFront_p2 = glm::vec3(0.0f, 0.0f, -1.0f); // initial forward
glm::vec3 charFrontTarget_p1 = glm::vec3(0.0f, 0.0f, 1.0f); // initial forward
glm::vec3 charFrontTarget_p2 = glm::vec3(0.0f, 0.0f, 1.0f); // initial forward
float charSpeed = 2.5f;

float verticalVelocity_p1 = 0.0f;
float verticalVelocity_p2 = 0.0f;

bool isGrounded_p1 = true;
bool isGrounded_p2 = true;

const float gravity = -10.0f;
const float jumpForce = 6.0f;
const float groundHeight = 0.0f; // your ground Y

// Hat Type
enum HatType
{
	Ghost = 1,
	Slime,
	Mario
};
HatType currentHatType = HatType::Ghost;


enum AnimState {
	IDLE = 1,
	IDLE_PUNCH,
	PUNCH_IDLE,
	IDLE_CROUCH,
	CROUCH_IDLE,
	IDLE_WALK,
	WALK_IDLE,
	WALK,
	CROUCH,

	CROUCH_HIT,
	HIT_CROUCH,

	CROUCH_BLOCK,
	BLOCK_CROUCH,

	IDLE_BLOCK,
	BLOCK_IDLE,

	IDLE_HIT,
	HIT_IDLE,

	IDLE_JUMP,
	JUMP_IDLE,
	
	IDLE_KICK,
	KICK_IDLE

};

struct PlayerControls {
	int moveLeft;
	int moveRight;
	int jump;
	int jumpKick;
	int punch;
	int crouch;
	int testStandBlock;
	int testCrouchBlock;
	int testHurt;
};

PlayerControls P1_Controls = {
	GLFW_KEY_A,       // moveLeft
	GLFW_KEY_D,       // moveRight
	GLFW_KEY_W,       // jump
	GLFW_KEY_G,       // jump kick
	GLFW_KEY_F,       // punch
	GLFW_KEY_S,        // crouch
	GLFW_KEY_1,        // testStandBlock
	GLFW_KEY_2,        // testCrouchBlock
	GLFW_KEY_3        // testHurt
};

PlayerControls P2_Controls = {
	GLFW_KEY_LEFT,
	GLFW_KEY_RIGHT,
	GLFW_KEY_UP,
	GLFW_KEY_K,    // jump kick
	GLFW_KEY_J,    // punch
	GLFW_KEY_DOWN,
	GLFW_KEY_1,        // testStandBlock
	GLFW_KEY_2,        // testCrouchBlock
	GLFW_KEY_3        // testHurt
};

enum AnimState P1_charState = IDLE;
float P1_blendAmount = 0.0f;
float P1_blendRate = 0.13f;


enum AnimState P2_charState = IDLE;
float P2_blendAmount = 0.0f;
float P2_blendRate = 0.13f;

// camera
float cameraRadius = 10.0f;          // distance from model
float orbitYaw = 0.0f;        // horizontal angle (degrees)
float orbitPitch = 20.0f;     // vertical angle (degrees)
float smoothSpeed = 8.0f;  // higher = faster interpolation
float targetYaw = orbitYaw;
float targetPitch = orbitPitch;

// Hit Distance
const float MIN_Z = -5.0f;
const float MAX_Z = 5.0f;

float moveCollideDistanceOffset = 1.5f;
float hitDistanceOffset = 2.5f;

float P1_hitDelayTimer = 0.0f;
float P2_hitDelayTimer = 0.0f;

float P1_punchDelayTimer = 0.0f;
float P2_punchDelayTimer = 0.0f;

const float JUMPKICK_HIT_DELAY = 1.3f;         // your jump kick delay
const float PUNCH_HIT_DELAY = 0.35f;   // new punch delay (tweak as you want)

// Health
float P1_HP = 100.0f;
float P2_HP = 100.0f;

float P1_MaxHP = 100.0f;
float P2_MaxHP = 100.0f;

unsigned int quadVAO = 0, quadVBO = 0;

float skyboxVertices[] = {
	// positions
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};

float cubeVertices[] = {
	// positions          
	-0.5f, -0.5f, -0.5f,  // 0
	 0.5f, -0.5f, -0.5f,  // 1
	 0.5f,  0.5f, -0.5f,  // 2
	-0.5f,  0.5f, -0.5f,  // 3
	-0.5f, -0.5f,  0.5f,  // 4
	 0.5f, -0.5f,  0.5f,  // 5
	 0.5f,  0.5f,  0.5f,  // 6
	-0.5f,  0.5f,  0.5f   // 7
};

unsigned int cubeIndices[] = {
	// front face
	0, 1, 2,
	2, 3, 0,

	// right face
	1, 5, 6,
	6, 2, 1,

	// back face
	7, 6, 5,
	5, 4, 7,

	// left face
	4, 0, 3,
	3, 7, 4,

	// bottom face
	4, 5, 1,
	1, 0, 4,

	// top face
	3, 2, 6,
	6, 7, 3
};

// Freeze frame
float hitStopTimer = 0.0f;      // remaining freeze time
float hitStopDuration_hit = 0.24f;   // strong hit
float hitStopDuration_block = 0.12f; // small block freeze

// Camera Shake
float cameraShakeTimer = 1.0f;
float cameraShakeIntensity = 0.0f;
float cameraMaxShakeIntensity = 0.5f; // fix intensity here

float shakeIntensity_hit = 0.5f;
float shakeIntensity_block = 0.2f;
float shakeDecaySpeed = 5.0f;

void BridgeAnimation(Animator& animator, Animation& startAnim, Animation& endAnim, AnimState endState, float delayTime, float& blendAmount, float& blendRate, AnimState& charState);
void UpdatePlayerAnimation(
	GLFWwindow* window,
	Animator& animator,
	Animation& idleAnim,
	Animation& walkAnim,
	Animation& punchAnim,
	Animation& crouchAnim,
	Animation& crouchBlockAnim,
	Animation& standBlockAnim,
	Animation& standHitAnim,
	Animation& jumpAnim,
	Animation& jumpKickAnim,
	AnimState& state,
	float& blendAmount,
	float blendRate,
	const PlayerControls& controls,
	const PlayerControls& victimControls,
	glm::vec3 attackerPos,
	glm::vec3 victimPos,
	AnimState& victimState
);

void DrawBar(Shader& uiShader, float x, float y, float width, float height, float percent, const glm::vec3& color);

unsigned int loadCubemap(vector<std::string> faces);

bool CheckHit(const glm::vec3& attackerPos, const glm::vec3& victimPos, float hitDistanceOffset)
{
	float dist = glm::distance(attackerPos, victimPos);
	return dist <= hitDistanceOffset;
}

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
	stbi_set_flip_vertically_on_load(true);

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader ourShader("anim_model.vs", "anim_model.fs");
	Shader hatShader(
		FileSystem::getPath("src/8.guest/2020/skeletal_animation/1.model_loading.vs").c_str(),
		FileSystem::getPath("src/8.guest/2020/skeletal_animation/1.model_loading.fs").c_str()
	);
	Shader uiShader(
		FileSystem::getPath("src/8.guest/2020/skeletal_animation/ui_shader.vs").c_str(),
		FileSystem::getPath("src/8.guest/2020/skeletal_animation/ui_shader.fs").c_str()
	);

	Shader skyboxShader(
		FileSystem::getPath("src/8.guest/2020/skeletal_animation/skybox.vs").c_str(),
		FileSystem::getPath("src/8.guest/2020/skeletal_animation/skybox.fs").c_str()
	);

	// load models
	// -----------
	// idle 3.3, walk 2.06, run 0.83, punch 1.03, kick 1.6

	Model P1_Model(FileSystem::getPath("resources/objects/Fighting/P1_Idle.dae"));
	Animation P1_idleAnimation(FileSystem::getPath("resources/objects/Fighting/P1_Idle.dae"), &P1_Model);
	Animation P1_walkAnimation(FileSystem::getPath("resources/objects/Fighting/P1_Walk.dae"), &P1_Model);
	Animation P1_punchAnimation(FileSystem::getPath("resources/objects/Fighting/P1_Punch.dae"), &P1_Model);
	Animation P1_crouchAnimation(FileSystem::getPath("resources/objects/Fighting/P1_Crouch.dae"), &P1_Model);
	Animation P1_crouchBlockAnimation(FileSystem::getPath("resources/objects/Fighting/P1_Crouch_Block.dae"), &P1_Model);
	Animation P1_standBlockAnimation(FileSystem::getPath("resources/objects/Fighting/P1_Stand_Block.dae"), &P1_Model);
	Animation P1_standHitAnimation(FileSystem::getPath("resources/objects/Fighting/P1_Stand_Hit.dae"), &P1_Model);
	Animation P1_jumpAnimation(FileSystem::getPath("resources/objects/Fighting/P1_Jumping.dae"), &P1_Model);
	Animation P1_jumpKickAnimation(FileSystem::getPath("resources/objects/Fighting/P1_TopKick.dae"), &P1_Model);

	Animator P1_animator(&P1_idleAnimation);
	
	Model P2_Model(FileSystem::getPath("resources/objects/Fighting/P2_Idle.dae"));
	Animation P2_idleAnimation(FileSystem::getPath("resources/objects/Fighting/P2_Idle.dae"), &P2_Model);
	Animation P2_walkAnimation(FileSystem::getPath("resources/objects/Fighting/P2_Walk.dae"), &P2_Model);
	Animation P2_punchAnimation(FileSystem::getPath("resources/objects/Fighting/P2_Punch.dae"), &P2_Model);
	Animation P2_crouchAnimation(FileSystem::getPath("resources/objects/Fighting/P2_Crouch.dae"), &P2_Model);
	Animation P2_crouchBlockAnimation(FileSystem::getPath("resources/objects/Fighting/P2_Crouch_Block.dae"), &P2_Model);
	Animation P2_standBlockAnimation(FileSystem::getPath("resources/objects/Fighting/P2_Stand_Block.dae"), &P2_Model);
	Animation P2_standHitAnimation(FileSystem::getPath("resources/objects/Fighting/P2_Stand_Hit.dae"), &P2_Model);
	Animation P2_jumpAnimation(FileSystem::getPath("resources/objects/Fighting/P2_Jumping.dae"), &P2_Model);
	Animation P2_jumpKickAnimation(FileSystem::getPath("resources/objects/Fighting/P2_TopKick.dae"), &P2_Model);

	Animator P2_animator(&P2_idleAnimation);

	unsigned int skyboxVAO, skyboxVBO;
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	// VBO VAO for platform
	unsigned int VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	// VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

	// EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

	// vertex attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	vector<std::string> faces
	{
		FileSystem::getPath("resources/textures/skybox/right.jpg"),
		FileSystem::getPath("resources/textures/skybox/left.jpg"),
		FileSystem::getPath("resources/textures/skybox/top.jpg"),
		FileSystem::getPath("resources/textures/skybox/bottom.jpg"),
		FileSystem::getPath("resources/textures/skybox/front.jpg"),
		FileSystem::getPath("resources/textures/skybox/back.jpg")
	};

	stbi_set_flip_vertically_on_load(false);
	unsigned int cubemapTexture = loadCubemap(faces);
	stbi_set_flip_vertically_on_load(true);

	// draw in wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		float gameplayDelta = deltaTime;
		if (hitStopTimer > 0.0f) {
			hitStopTimer -= deltaTime;
			gameplayDelta = 0.0f;   // freeze animation & gameplay
		}

		// input
		// -----
		processInput(window);
	
		UpdatePlayerAnimation(
			window,
			P1_animator,
			P1_idleAnimation,
			P1_walkAnimation,
			P1_punchAnimation,
			P1_crouchAnimation,
			P1_crouchBlockAnimation,
			P1_standBlockAnimation,
			P1_standHitAnimation,
			P1_jumpAnimation,
			P1_jumpKickAnimation,
			P1_charState,
			P1_blendAmount,
			P1_blendRate,
			P1_Controls,
			P2_Controls,
			charPosition_p1,      // attacker
			charPosition_p2,      // victim
			P2_charState          // victim receives hit state
		);

		UpdatePlayerAnimation(
			window,
			P2_animator,
			P2_idleAnimation,
			P2_walkAnimation,
			P2_punchAnimation,
			P2_crouchAnimation,
			P2_crouchBlockAnimation,
			P2_standBlockAnimation,
			P2_standHitAnimation,
			P2_jumpAnimation,
			P2_jumpKickAnimation,
			P2_charState,
			P2_blendAmount,
			P2_blendRate,
			P2_Controls,
			P1_Controls,
			charPosition_p2,      // attacker
			charPosition_p1,      // victim
			P1_charState
		);


		// lerp rotation
		//float t = 0.1;
		//charFront = glm::mix(charFront, charFrontTarget, t);

		P1_animator.UpdateAnimation(gameplayDelta);
		P2_animator.UpdateAnimation(gameplayDelta);

		// render
		// ------
		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		

		// don't forget to enable shader before setting uniforms
		ourShader.use();

		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

		// Smoothly interpolate camera orbit
		float lerpFactor = 1.0f - expf(-smoothSpeed * deltaTime);
		orbitYaw = glm::mix(orbitYaw, targetYaw, lerpFactor);
		orbitPitch = glm::mix(orbitPitch, targetPitch, lerpFactor);

		glm::vec3 cameraPos(-10.0f, 2.0f, 0.0f);  // e.g. behind the origin, at z = +10

		if (cameraShakeTimer > 0.0f)
		{
			printf("cameraShakeTimer = %f\n", cameraShakeTimer);
			cameraShakeTimer -= deltaTime;

			float shake = cameraShakeIntensity * (cameraShakeTimer);
			cameraPos.x += (rand() % 1000 / 1000.0f - 0.5f) * shake;
			cameraPos.y += (rand() % 1000 / 1000.0f - 0.5f) * shake;
			cameraPos.z += (rand() % 1000 / 1000.0f - 0.5f) * shake;

			// decay shake intensity smoothly
			cameraShakeIntensity -= shakeDecaySpeed * deltaTime;
			if (cameraShakeIntensity < 0.0f) cameraShakeIntensity = cameraMaxShakeIntensity;
		}

		glm::mat4 view = glm::lookAt(
			cameraPos,
			glm::vec3(0.0f, 2.0f, 0.0f),   // look at world origin + some Y offset
			glm::vec3(0.0f, 1.0f, 0.0f)    // up vector
		);

		

		ourShader.use();

		ourShader.setMat4("projection", projection);
		ourShader.setMat4("view", view);
		ourShader.setVec3("colorTint", glm::vec3(1.0f));

		auto transforms = P1_animator.GetFinalBoneMatrices();
		for (int i = 0; i < transforms.size(); ++i)
			ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

		// render the loaded model
		glm::mat4 P1model = glm::mat4(1.0f);

		P1model = glm::translate(P1model, charPosition_p1);
		P1model = glm::rotate(P1model, 0.0f, glm::vec3(0, 1, 0));
		P1model = glm::scale(P1model, glm::vec3(1.0f));

		ourShader.setMat4("model", P1model);
		ourShader.setVec3("colorTint", glm::vec3(1.0f));
		P1_Model.Draw(ourShader);

		// render the loaded model
		glm::mat4 P2model = glm::mat4(1.0f);

		P2model = glm::translate(P2model, charPosition_p2);
		P2model = glm::rotate(P2model, glm::radians(180.f), glm::vec3(0, 1, 0));
		P2model = glm::scale(P2model, glm::vec3(1.0f));

		transforms = P2_animator.GetFinalBoneMatrices();
		for (int i = 0; i < transforms.size(); ++i)
			ourShader.setMat4("finalBonesMatrices[" + std::to_string(i) + "]", transforms[i]);

		ourShader.setMat4("model", P2model);
		ourShader.setVec3("colorTint", glm::vec3(1.0f));
		P2_Model.Draw(ourShader);

		// Platform
		// Draw platforms
		ourShader.use();


		glm::mat4 model = glm::mat4(1.0f);

		// change cube size here
		model = glm::translate(model, glm::vec3(0.0f, -1.5f, 0.0f));
		model = glm::scale(model, glm::vec3(5.0f, 3.0f, 20.0f));   // half size

		ourShader.setMat4("model", model);
		ourShader.setVec3("colorTint", glm::vec3(137.0f / 256.0f, 97.0f / 256.0f, 0.0f)); // red tint

		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);



		glDepthFunc(GL_LEQUAL);

		skyboxShader.use();

		// remove translation from the view matrix
		glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
		skyboxShader.setMat4("view", skyboxView);
		skyboxShader.setMat4("projection", projection);

		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glDepthFunc(GL_LESS);

		float barWidth = 300.0f;
		float barHeight = 25.0f;


		// ---- Draw 2D UI ----
		glDisable(GL_DEPTH_TEST);

		// switch to simple 2D shader
		uiShader.use();

		//printf("P1 hp = %f P2 HP = %f\n", P1_HP, P2_HP);

		// Set orthographic projection once for the UI shader
		glm::mat4 ortho = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT);
		uiShader.setMat4("projection", ortho);

		// --- P1 HP Bar ---
		// 1. Draw Background (Max HP - dark grey)
		DrawBar(uiShader, 50, 750, barWidth, barHeight, 1.0f, glm::vec3(0.2f, 0.2f, 0.2f));
		// 2. Draw Foreground (Current HP - green)
		DrawBar(uiShader, 50, 750, barWidth, barHeight, P1_HP / P1_MaxHP, glm::vec3(0.0f, 1.0f, 0.0f));

		// --- P2 HP Bar ---
		// 1. Draw Background (Max HP - dark grey)
		DrawBar(uiShader, SCR_WIDTH - barWidth - 50, 750, barWidth, barHeight, 1.0f, glm::vec3(0.2f, 0.2f, 0.2f));
		// 2. Draw Foreground (Current HP - red)
		DrawBar(uiShader, SCR_WIDTH - barWidth - 50, 750, barWidth, barHeight, P2_HP / P2_MaxHP, glm::vec3(1.0f, 0.0f, 0.0f));

		// restore depth test for next frame
		glEnable(GL_DEPTH_TEST);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{

	glm::vec3 moveDir_p1(0.0f);
	glm::vec3 moveDir_p2(0.0f);

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	/*
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		moveDir += glm::vec3(0.0f, 0.0f, -1.0f);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		moveDir += glm::vec3(0.0f, 0.0f, 1.0f);
	*/

	if (P1_charState != AnimState::IDLE_PUNCH && P1_charState != AnimState::PUNCH_IDLE &&
		P1_charState != AnimState::IDLE_BLOCK && P1_charState != AnimState::BLOCK_IDLE &&
		P1_charState != AnimState::CROUCH_BLOCK && P1_charState != AnimState::BLOCK_CROUCH &&
		P1_charState != AnimState::IDLE_HIT && P1_charState != AnimState::HIT_IDLE &&
		P1_charState != AnimState::CROUCH_HIT && P1_charState != AnimState::HIT_CROUCH &&
		P1_charState != AnimState::CROUCH && P1_charState != AnimState::IDLE_CROUCH && P1_charState != AnimState::CROUCH_IDLE)
	{
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			moveDir_p1 += glm::vec3(0.0f, 0.0f, -1.0f);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			moveDir_p1 += glm::vec3(0.0f, 0.0f, 1.0f);
	}

	if (P2_charState != AnimState::IDLE_PUNCH && P2_charState != AnimState::PUNCH_IDLE &&
		P2_charState != AnimState::IDLE_BLOCK && P2_charState != AnimState::BLOCK_IDLE &&
		P2_charState != AnimState::CROUCH_BLOCK && P2_charState != AnimState::BLOCK_CROUCH &&
		P2_charState != AnimState::IDLE_HIT && P2_charState != AnimState::HIT_IDLE &&
		P2_charState != AnimState::CROUCH_HIT && P2_charState != AnimState::HIT_CROUCH &&
		P2_charState != AnimState::CROUCH && P2_charState != AnimState::IDLE_CROUCH && P2_charState != AnimState::CROUCH_IDLE)
	{
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
			moveDir_p2 += glm::vec3(0.0f, 0.0f, -1.0f);
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
			moveDir_p2 += glm::vec3(0.0f, 0.0f, 1.0f);
	}

	if (isGrounded_p1 && glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		verticalVelocity_p1 = jumpForce;
		isGrounded_p1 = false;
	}

	// P2 JUMP KEY
	if (isGrounded_p2 && glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
	{
		verticalVelocity_p2 = jumpForce;
		isGrounded_p2 = false;
	}

	if (!isGrounded_p1)
		verticalVelocity_p1 += gravity * deltaTime;

	if (!isGrounded_p2)
		verticalVelocity_p2 += gravity * deltaTime;

	// ----- P1 MOVEMENT -----
	if (glm::length(moveDir_p1) > 0.0f)
	{
		moveDir_p1 = glm::normalize(moveDir_p1);

		glm::vec3 newPosP1 = charPosition_p1 + moveDir_p1 * charSpeed * deltaTime;
		float futureDist = std::abs(newPosP1.z - charPosition_p2.z);
		//printf("newPosP1.z = %f charPosition_p2.z = %f P1 futureDist = %f \n", newPosP1.z, charPosition_p2.z, futureDist);
		if (futureDist > moveCollideDistanceOffset)
		{
			// Apply boundary limit
			newPosP1.z = glm::clamp(newPosP1.z, MIN_Z, MAX_Z);

			charPosition_p1 = newPosP1;
			charFrontTarget_p1 = moveDir_p1;
		}
	}

	// ----- P2 MOVEMENT -----
	if (glm::length(moveDir_p2) > 0.0f)
	{
		moveDir_p2 = glm::normalize(moveDir_p2);

		glm::vec3 newPosP2 = charPosition_p2 + moveDir_p2 * charSpeed * deltaTime;
		float futureDist = std::abs(newPosP2.z - charPosition_p1.z);

		//printf("newPosP2.z = %f charPosition_p1.z = %f P1 futureDist = %f \n", newPosP2.z, charPosition_p1.z, futureDist);
		if (futureDist > moveCollideDistanceOffset)
		{
			// Apply boundary limit
			newPosP2.z = glm::clamp(newPosP2.z, MIN_Z, MAX_Z);

			charPosition_p2 = newPosP2;
			charFrontTarget_p2 = moveDir_p2;
		}
	}

	// ----- P1 -----
	charPosition_p1.y += verticalVelocity_p1 * deltaTime;

	if (charPosition_p1.y <= groundHeight)
	{
		charPosition_p1.y = groundHeight;
		verticalVelocity_p1 = 0.0f;
		isGrounded_p1 = true;

		// P1_charState = AnimState::IDLE;  <-- optional
	}

	// ----- P2 -----
	charPosition_p2.y += verticalVelocity_p2 * deltaTime;

	if (charPosition_p2.y <= groundHeight)
	{
		charPosition_p2.y = groundHeight;
		verticalVelocity_p2 = 0.0f;
		isGrounded_p2 = true;

		// P2_charState = AnimState::IDLE;
	}

	// update y pos

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	// Apply to target rotation only
	targetYaw -= xoffset;
	targetPitch -= yoffset;

	// Clamp pitch
	if (targetPitch > 89.0f) targetPitch = 89.0f;
	if (targetPitch < -89.0f) targetPitch = -89.0f;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

void BridgeAnimation(Animator& animator, Animation& startAnim, Animation& endAnim, AnimState endState, float delayTime, float& blendAmount, float& blendRate, AnimState& charState)
{
	if (animator.m_CurrentTime > delayTime)
	{
		blendAmount += blendRate;
		blendAmount = fmod(blendAmount, 1.0f);
		animator.PlayAnimation(&startAnim, &endAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
		if (blendAmount > 0.9f) {
			blendAmount = 0.0f;
			float startTime = animator.m_CurrentTime2;
			animator.PlayAnimation(&endAnim, NULL, startTime, 0.0f, blendAmount);
			charState = endState;
			//printf("-------------- charState change to %d\n", charState);
		}
		//printf("charState = %d\n", charState);
	}

}

bool IsHoldingBack(GLFWwindow* window, const PlayerControls& controls, const glm::vec3& selfPos, const glm::vec3& enemyPos)
{
	float dz = enemyPos.z - selfPos.z;

	if (dz > 0)
	{
		return glfwGetKey(window, controls.moveLeft) == GLFW_PRESS;
	}
	else 
	{
		return glfwGetKey(window, controls.moveRight) == GLFW_PRESS;
	}
}

void UpdatePlayerAnimation(
	GLFWwindow* window,
	Animator& animator,
	Animation& idleAnim,
	Animation& walkAnim,
	Animation& punchAnim,
	Animation& crouchAnim,
	Animation& crouchBlockAnim,
	Animation& standBlockAnim,
	Animation& standHitAnim,
	Animation& jumpAnim,
	Animation& jumpKickAnim,
	AnimState& state,
	float& blendAmount,
	float blendRate,
	const PlayerControls& controls,
	const PlayerControls& victimControls,
	glm::vec3 attackerPos,
	glm::vec3 victimPos,
	AnimState& victimState
)
{	

	float& kickTimer = (&state == &P1_charState) ? P1_hitDelayTimer : P2_hitDelayTimer;
	float& punchTimer = (&state == &P1_charState) ? P1_punchDelayTimer : P2_punchDelayTimer;
	float& currentHP = (&state == &P1_charState) ? P1_HP : P2_HP;
	float& victimHP = (&state == &P1_charState) ? P2_HP : P1_HP;

	// ========================= DELAYED KICK DAMAGE =========================
	if (kickTimer > 0.0f)
	{
		kickTimer -= deltaTime;

		if (kickTimer <= 0.0f)
		{
			bool crouching =
				state == CROUCH ||
				state == IDLE_CROUCH ||
				state == CROUCH_IDLE;

			bool blocking = IsHoldingBack(window, controls, attackerPos, victimPos);

			if (blocking)
			{
				state = crouching ? CROUCH_HIT : IDLE_BLOCK;

				if (state == CROUCH_HIT)
				{
					hitStopTimer = hitStopDuration_hit;
					cameraShakeTimer = 0.5f;
					currentHP -= 5.0f;
				}
				else
				{
					hitStopTimer = hitStopDuration_block;
					cameraShakeTimer = 0.3f;
					currentHP -= 2.0f;
				}
			}
			else
			{
				state = crouching ? CROUCH_HIT : IDLE_HIT;

				hitStopTimer = hitStopDuration_hit;
				cameraShakeTimer = 0.5f;
				currentHP -= 5.0f;
			}

			blendAmount = 0.0f;
		}
	}

	// ========================= DELAYED PUNCH DAMAGE =========================
	if (punchTimer > 0.0f)
	{
		punchTimer -= deltaTime;

		if (punchTimer <= 0.0f)
		{
			bool blocking = IsHoldingBack(window, controls, attackerPos, victimPos);

			if (blocking)
			{
				state = IDLE_BLOCK;

				// block hit-stop + reduced shake
				hitStopTimer = hitStopDuration_block;
				cameraShakeTimer = 0.3f;

				currentHP -= 2.0f;
			}
			else
			{
				state = IDLE_HIT;

				// normal hit-stop + full shake
				hitStopTimer = hitStopDuration_hit;
				cameraShakeTimer = 0.5f;

				currentHP -= 5.0f;
			}

			blendAmount = 0.0f;
		}
	}

	

	switch (state) {

		// ========================= IDLE =========================
	case IDLE:
		if (glfwGetKey(window, controls.moveLeft) == GLFW_PRESS ||
			glfwGetKey(window, controls.moveRight) == GLFW_PRESS)
		{
			blendAmount = 0.0f;
			animator.PlayAnimation(&idleAnim, &walkAnim, animator.m_CurrentTime, 0.0f, blendAmount);
			state = IDLE_WALK;
		}
		else if (glfwGetKey(window, controls.punch) == GLFW_PRESS) {        // Punch
			// Check hit range
			if (CheckHit(attackerPos, victimPos, hitDistanceOffset))
			{
				if (&victimState == &P1_charState)
					P1_punchDelayTimer = PUNCH_HIT_DELAY;

				if (&victimState == &P2_charState)
					P2_punchDelayTimer = PUNCH_HIT_DELAY;
			}

			blendAmount = 0.0f;
			animator.PlayAnimation(&idleAnim, &punchAnim, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);
			state = AnimState::IDLE_PUNCH;
			return;
		}
		else if (glfwGetKey(window, controls.crouch) == GLFW_PRESS) {    // Crouch
			blendAmount = 0.0f;
			animator.PlayAnimation(&idleAnim, &crouchAnim, animator.m_CurrentTime, 0.0f, blendAmount);
			state = IDLE_CROUCH;
		}
		else if (glfwGetKey(window, controls.testStandBlock) == GLFW_PRESS) {   // Block
			blendAmount = 0.0f;
			animator.PlayAnimation(&idleAnim, &standBlockAnim, animator.m_CurrentTime, 0.0f, blendAmount);
			state = IDLE_BLOCK;
		}
		else if (glfwGetKey(window, controls.testHurt) == GLFW_PRESS) {  // Hit
			blendAmount = 0.0f;
			animator.PlayAnimation(&idleAnim, &standHitAnim, animator.m_CurrentTime, 0.0f, blendAmount);
			state = IDLE_HIT;
		}
		else if (glfwGetKey(window, controls.jump) == GLFW_PRESS) {      // Jump

			
			blendAmount = 0.0f;
			animator.PlayAnimation(&idleAnim, &jumpAnim, animator.m_CurrentTime, 0.0f, blendAmount);
			state = IDLE_JUMP;
		}
		if (glfwGetKey(window, controls.jumpKick) == GLFW_PRESS) 
		{
			if (CheckHit(attackerPos, victimPos, hitDistanceOffset))
			{
				if (&victimState == &P1_charState)
					P1_hitDelayTimer = JUMPKICK_HIT_DELAY;

				if (&victimState == &P2_charState)
					P2_hitDelayTimer = JUMPKICK_HIT_DELAY;
			}

			

			blendAmount = 0.0f;
			animator.PlayAnimation(&idleAnim, &jumpKickAnim, animator.m_CurrentTime, 0.0f, blendAmount);
			state = IDLE_KICK;
		}
		break;

		// ========================= CROUCH =========================
	case IDLE_CROUCH:
		BridgeAnimation(animator, idleAnim, crouchAnim, CROUCH, 0, blendAmount, blendRate, state);
		break;

	case CROUCH:
		if (glfwGetKey(window, controls.crouch) == GLFW_RELEASE) {
			blendAmount = 0.0f;
			animator.PlayAnimation(&crouchAnim, &idleAnim, animator.m_CurrentTime, 0.0f, blendAmount);
			state = CROUCH_IDLE;
		}
		else if (glfwGetKey(window, controls.testCrouchBlock) == GLFW_PRESS) {
			blendAmount = 0.0f;
			animator.PlayAnimation(&crouchAnim, &crouchBlockAnim, animator.m_CurrentTime, 0.0f, blendAmount);
			state = CROUCH_BLOCK;
		}
		else if (glfwGetKey(window, controls.testHurt) == GLFW_PRESS) {
			blendAmount = 0.0f;
			animator.PlayAnimation(&crouchAnim, &standHitAnim, animator.m_CurrentTime, 0.0f, blendAmount);
			state = CROUCH_HIT;
		}
		break;

	case CROUCH_IDLE:
		BridgeAnimation(animator, crouchAnim, idleAnim, IDLE, 0, blendAmount, blendRate, state);
		break;

	case CROUCH_HIT:
		BridgeAnimation(animator, crouchAnim, standHitAnim, HIT_IDLE, 0, blendAmount, blendRate, state);
		break;

	case HIT_CROUCH:
		BridgeAnimation(animator, standHitAnim, idleAnim, IDLE, 0.2f, blendAmount, blendRate, state);
		break;

	case CROUCH_BLOCK:
		BridgeAnimation(animator, crouchAnim, crouchBlockAnim, BLOCK_CROUCH, 0, blendAmount, blendRate, state);
		break;

	case BLOCK_CROUCH:
		BridgeAnimation(animator, crouchBlockAnim, crouchAnim, CROUCH, 0.2f, blendAmount, blendRate, state);
		break;

		// ========================= WALK =========================
	case IDLE_WALK:
		BridgeAnimation(animator, idleAnim, walkAnim, WALK, 0, blendAmount, blendRate, state);
		break;

	case WALK:
		animator.PlayAnimation(&walkAnim, NULL, animator.m_CurrentTime, animator.m_CurrentTime2, blendAmount);

		if (glfwGetKey(window, controls.punch) == GLFW_PRESS) {
			blendAmount = 0.0f;
			animator.PlayAnimation(&idleAnim, &punchAnim, animator.m_CurrentTime, 0.0f, blendAmount);
			state = IDLE_PUNCH;
		}
		else if (glfwGetKey(window, controls.crouch) == GLFW_PRESS) {
			blendAmount = 0.0f;
			animator.PlayAnimation(&idleAnim, &crouchAnim, animator.m_CurrentTime, 0.0f, blendAmount);
			state = IDLE_CROUCH;
		}
		else if (glfwGetKey(window, controls.moveLeft) != GLFW_PRESS &&
			glfwGetKey(window, controls.moveRight) != GLFW_PRESS)
		{
			state = WALK_IDLE;
		}
		break;

	case WALK_IDLE:
		BridgeAnimation(animator, walkAnim, idleAnim, IDLE, 0, blendAmount, blendRate, state);
		break;

		// ========================= PUNCH =========================
	case IDLE_PUNCH:
		BridgeAnimation(animator, idleAnim, punchAnim, PUNCH_IDLE, 0, blendAmount, blendRate, state);
		break;

	case PUNCH_IDLE:
		BridgeAnimation(animator, punchAnim, idleAnim, IDLE, 0.7f, blendAmount, blendRate, state);
		break;

		// ========================= Kick =========================
	case IDLE_KICK:
		BridgeAnimation(animator, idleAnim, jumpKickAnim, KICK_IDLE, 0, blendAmount, blendRate, state);
		break;

	case KICK_IDLE:
		BridgeAnimation(animator, jumpKickAnim, idleAnim, IDLE, 2.0f, blendAmount, blendRate, state);
		break;

		// ========================= Hit =========================
	case IDLE_HIT:
		BridgeAnimation(animator, idleAnim, standHitAnim, HIT_IDLE, 0, blendAmount, blendRate, state);
		break;

	case HIT_IDLE:
		BridgeAnimation(animator, standHitAnim, idleAnim, IDLE, 1.0f, blendAmount, blendRate, state);
		break;

		// ========================= Block =========================
	case IDLE_BLOCK:
		BridgeAnimation(animator, idleAnim, standBlockAnim, BLOCK_IDLE, 0, blendAmount, blendRate, state);
		break;

	case BLOCK_IDLE:
		BridgeAnimation(animator, standBlockAnim, idleAnim, IDLE, 0.5f, blendAmount, blendRate, state);
		break;
	}

}

void DrawBar(Shader& uiShader, float x, float y, float width, float height, float percent, const glm::vec3& color)
{
	uiShader.use();

	uiShader.setVec3("barColor", color.x, color.y, color.z);

	// 1. Set Projection
	glm::mat4 ortho = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT);
	uiShader.setMat4("projection", ortho);

	glDisable(GL_DEPTH_TEST);

	// 2. Calculate vertices (only for the filled bar portion)
	float vertices[] = {
		x, y,
		x + width * percent, y,
		x + width * percent, y + height,
		x, y + height
	};

	// 3. Update VBO data
	// ... VAO/VBO binding and attribute setup ...
	// You must re-send the data because the bar width (percent) changes!

	GLuint VAO, VBO; // In-function declaration for simplicity
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// 4. Draw
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	// 5. Cleanup
	glBindVertexArray(0);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);

	glEnable(GL_DEPTH_TEST);
}

unsigned int loadCubemap(vector<std::string> faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else
		{
			std::cout << "Cubemap failed to load at: " << faces[i] << std::endl;
		}
		stbi_image_free(data);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}