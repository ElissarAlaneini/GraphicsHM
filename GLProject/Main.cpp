#include "libs/glad_loader_docs.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include "PolygonWithTexture.cpp"
#include <learnopengl/shader_m.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>
#include "stb_image.h"

using namespace glm;


const char *CARD_FACE_PATHS[5] = {
    "./textures/assassins_creed/card1.png",
    "./textures/assassins_creed/card2.png",
    "./textures/assassins_creed/card3.png",
    "./textures/assassins_creed/card4.png",
    "./textures/assassins_creed/card5.png"
};

const char *CARD_BACK_PATH =
    "./textures/assassins_creed/card_back.png"; 

const char *POSTER_PATH = "./textures/assassins_creed/ac_odyssey_clean.png";



const char *WALLPAPER_PATH = "./textures/table_wood.jpg";

const char *FLOOR_PATH ="./textures/assassins_creed/IMG-20261120-WA0010 (1).jpg";

const char *TABLE_WOOD_PATH ="./textures/assassins_creed/IMG-20261120-WA0016.jpg";
const char *TABLE_CLOTH_PATH = "./textures/table_cloth.jpg"; 
const char *DARK_WOOD_PATH = "./textures/dark_wood.jpg";
const char *WHITE_PATH = "./textures/white.jpg"; 
const char *ZONE_DECK_PATH = "./textures/zone_deck.jpg";
const char *ZONE_PLAY_PATH = "./textures/zone_play.jpg";
const char *ZONE_OPPONENT_PATH = "./textures/zone_opponent.jpg";
const char *CLOCK_FACE_PATH = "./textures/clock_face_ac.jpg"; 
const char *GRAMOPHONE_MODEL_PATH = "./models/gramophone/gramophone.obj"; 


const float CARD_W = 0.95f;	 
const float CARD_H = 1.40f;	   
const float CARD_GAP = 0.20f; 
const float TABLE_TOP = 1.2f;  
const float TABLE_SIZE = 7.6f;

const bool PLAYED_CARDS_STAND = false;

const float CARD_STAND_TIME = 0.7f;
const float CARD_MOVE_TIME = 0.9f;	
const float CAMERA_MOVE_TIME = 0.9f;
const float CARD_LEAN_DEGREES = 12.0f; 


const int TOTAL_TEAMS = 30;
enum DecorId
{
	DECOR_LAMP,
	DECOR_CLOCK,
	DECOR_GRAMOPHONE,
	DECOR_HIDDEN_BLADE,
	DECOR_COUNT
};
struct DecorInfo
{
	const char *name;
	int teamsUsed;
	float idf;
	bool selected;
};
DecorInfo decorCatalog[DECOR_COUNT] = {
	{"Floor lamp (مصباح أرضي)", 9, 0.0f, false},
	{"Wall clock (ساعة جدارية)", 6, 0.0f, false},
	{"Gramophone 3D model (غراموفون)", 2, 0.0f, false},
	{"Hidden Blade (النصل الخفي)", 3, 0.0f, false}}; 


int SCR_WIDTH = 1280;
int SCR_HEIGHT = 720;
const float FOV_DEGREES = 60.0f;


struct CamPose
{
	vec3 pos;
	float yaw;
	float pitch;
};
const CamPose POSE_TOP = {vec3(0.0f, 9.0f, 0.15f), -90.0f, -89.0f};
const CamPose POSE_PLAYER = {vec3(0.0f, 5.0f, 5.6f), -90.0f, -32.0f};

Camera camera(POSE_TOP.pos, vec3(0.0f, 1.0f, 0.0f), POSE_TOP.yaw, POSE_TOP.pitch);
CamPose camFrom = POSE_TOP, camTo = POSE_TOP;
float camT = 1.0f; 
int viewState = 1;		 
bool freeCam = false;	 
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

const float PI = 3.14159265f;

float smooth01(float t)
{
	t = std::max(0.0f, std::min(1.0f, t));
	return t * t * (3.0f - 2.0f * t);
}


unsigned int loadTexture(const char *path)
{
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height, nrChannels;
	unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
	if (data)
	{
		GLenum format = GL_RGB;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 4)
			format = GL_RGBA;
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
		std::cout << "Failed to load texture: " << path << std::endl;
	stbi_image_free(data);
	return texture;
}

PolygonWithTexture makeQuad(vec3 p0, vec3 p1, vec3 p2, vec3 p3, float uRep, float vRep, GLuint &tex, vec3 color = vec3(1.0f))
{
	std::vector<vec3> v = {p0, p1, p2, p3};
	std::vector<vec2> t = {vec2(0.0f, 0.0f), vec2(uRep, 0.0f), vec2(uRep, vRep), vec2(0.0f, vRep)};
	PolygonWithTexture quad = PolygonWithTexture(v, t, color);
	quad.setTexture(tex);
	return quad;
}

PolygonWithTexture makeFlatRect(float cx, float y, float cz, float w, float d, float uRep, float vRep, GLuint &tex, vec3 color = vec3(1.0f))
{
	return makeQuad(vec3(cx - w / 2, y, cz + d / 2), vec3(cx + w / 2, y, cz + d / 2),
					vec3(cx + w / 2, y, cz - d / 2), vec3(cx - w / 2, y, cz - d / 2), uRep, vRep, tex, color);
}

PolygonWithTexture makeDisc(int segments, GLuint &tex, vec3 color = vec3(1.0f))
{
	std::vector<vec3> v;
	std::vector<vec2> t;
	v.push_back(vec3(0.0f, 0.0f, 0.0f));
	t.push_back(vec2(0.5f, 0.5f));
	for (int i = 0; i <= segments; i++)
	{
		float a = 2.0f * PI * i / segments;
		v.push_back(vec3(std::cos(a), std::sin(a), 0.0f));
		t.push_back(vec2(0.5f + 0.5f * std::cos(a), 0.5f + 0.5f * std::sin(a)));
	}
	PolygonWithTexture disc = PolygonWithTexture(v, t, color);
	disc.setTexture(tex);
	return disc;
}

PolygonWithTexture makePointed(GLuint &tex, vec3 color = vec3(1.0f))
{
	std::vector<vec3> v = {vec3(-0.5f, 0.0f, 0.0f), vec3(0.5f, 0.0f, 0.0f), vec3(0.5f, 0.85f, 0.0f), vec3(0.0f, 1.0f, 0.0f), vec3(-0.5f, 0.85f, 0.0f)};
	std::vector<vec2> t = {vec2(0.0f, 0.0f), vec2(1.0f, 0.0f), vec2(1.0f, 0.85f), vec2(0.5f, 1.0f), vec2(0.0f, 0.85f)};
	PolygonWithTexture shape = PolygonWithTexture(v, t, color);
	shape.setTexture(tex);
	return shape;
}

class Box
{
public:
	Box(GLuint &tex, vec3 color = vec3(1.0f))
	{
		const vec3 faceVerts[6][4] = {
			{vec3(-.5f, -.5f, .5f), vec3(.5f, -.5f, .5f), vec3(.5f, .5f, .5f), vec3(-.5f, .5f, .5f)},	  // أمام
			{vec3(.5f, -.5f, -.5f), vec3(-.5f, -.5f, -.5f), vec3(-.5f, .5f, -.5f), vec3(.5f, .5f, -.5f)}, // خلف
			{vec3(-.5f, -.5f, -.5f), vec3(-.5f, -.5f, .5f), vec3(-.5f, .5f, .5f), vec3(-.5f, .5f, -.5f)}, // يسار
			{vec3(.5f, -.5f, .5f), vec3(.5f, -.5f, -.5f), vec3(.5f, .5f, -.5f), vec3(.5f, .5f, .5f)},	  // يمين
			{vec3(-.5f, -.5f, -.5f), vec3(.5f, -.5f, -.5f), vec3(.5f, -.5f, .5f), vec3(-.5f, -.5f, .5f)}, // تحت
			{vec3(-.5f, .5f, .5f), vec3(.5f, .5f, .5f), vec3(.5f, .5f, -.5f), vec3(-.5f, .5f, -.5f)}};	  // فوق
		std::vector<vec2> t = {vec2(0.0f, 0.0f), vec2(1.0f, 0.0f), vec2(1.0f, 1.0f), vec2(0.0f, 1.0f)};
		for (int f = 0; f < 6; f++)
		{
			std::vector<vec3> v(faceVerts[f], faceVerts[f] + 4);
			faces.push_back(PolygonWithTexture(v, t, color));
			faces.back().setTexture(tex);
		}
	}

	void draw(Shader &shader, mat4 transformation)
	{
		for (size_t i = 0; i < faces.size(); i++)
		{
			faces[i].transformation(transformation);
			faces[i].draw(shader);
		}
	}

private:
	std::vector<PolygonWithTexture> faces;
};

struct PlacedBox
{
	Box box;
	vec3 pos;
	vec3 size;
	PlacedBox(GLuint &tex, vec3 color, vec3 p, vec3 s) : box(tex, color), pos(p), size(s) {}

	void draw(Shader &shader)
	{
		mat4 transformation = mat4(1.0f);
		transformation = glm::translate(transformation, pos);
		transformation = glm::scale(transformation, size);
		box.draw(shader, transformation);
	}
};


struct Card
{
	PolygonWithTexture front, back;
	vec3 hinge;	
	vec3 from, to;	 
	float moveT;	 
	float stand;	 
	bool moving;	 
	bool inPlay;	 
	Card(PolygonWithTexture f, PolygonWithTexture b, vec3 h)
		: front(f), back(b), hinge(h), from(h), to(h), moveT(0.0f), stand(0.0f), moving(false), inPlay(false) {}
};

std::vector<Card> cards;

float slotX(int i) { return (i - 2) * (CARD_W + CARD_GAP); }

const float OPPONENT_Z = -2.55f;
const float PLAY_Z = -0.30f;
const float STOCK_Z = 1.95f;
const float ZONE_W = 6.0f;
const float ZONE_D = 1.9f;

vec3 stockHinge(int i) { return vec3(slotX(i), TABLE_TOP + 0.02f, STOCK_Z + CARD_H / 2); }
vec3 playHinge(int i) { return vec3(slotX(i), TABLE_TOP + 0.02f, PLAY_Z + CARD_H / 2); }

void drawRandomCard()
{
	std::vector<int> candidates;
	for (int i = 0; i < (int)cards.size(); i++)
		if (!cards[i].inPlay)
			candidates.push_back(i);
	if (candidates.empty())
		return; 

	int pick = candidates[std::rand() % candidates.size()];
	cards[pick].inPlay = true;
	cards[pick].moving = true;
	cards[pick].moveT = 0.0f;
	cards[pick].from = cards[pick].hinge;
	cards[pick].to = playHinge(pick); 
}

void updateCards(float dt)
{
	for (size_t i = 0; i < cards.size(); i++)
	{
		Card &c = cards[i];

		bool shouldStand = false;
		if (viewState == 2)
			shouldStand = c.inPlay ? PLAYED_CARDS_STAND : true;
		float target = shouldStand ? 1.0f : 0.0f;

		float step = dt / CARD_STAND_TIME;
		if (c.stand < target)
			c.stand = std::min(c.stand + step, target);
		else if (c.stand > target)
			c.stand = std::max(c.stand - step, target);

		if (c.moving)
		{
			c.moveT += dt / CARD_MOVE_TIME;
			if (c.moveT >= 1.0f)
			{
				c.moveT = 1.0f;
				c.moving = false;
			}
			float s = smooth01(c.moveT);
			c.hinge = c.from + (c.to - c.from) * s + vec3(0.0f, 0.9f * std::sin(PI * s), 0.0f);
		}
	}
}



void moveCameraTo(CamPose to)
{
	camFrom.pos = camera.Position;
	camFrom.yaw = camera.Yaw;
	camFrom.pitch = camera.Pitch;
	camTo = to;
	camT = 0.0f;
}

void updateCamera(float dt)
{
	if (freeCam || camT >= 1.0f)
		return;
	camT = std::min(1.0f, camT + dt / CAMERA_MOVE_TIME);
	float s = smooth01(camT);
	camera.Position = camFrom.pos + (camTo.pos - camFrom.pos) * s;
	camera.Yaw = camFrom.yaw + (camTo.yaw - camFrom.yaw) * s;
	camera.Pitch = camFrom.pitch + (camTo.pitch - camFrom.pitch) * s;
	camera.ProcessMouseMovement(0.0f, 0.0f); 
}


void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

void selectDecorByIdf()
{
	int order[DECOR_COUNT];
	for (int i = 0; i < DECOR_COUNT; i++)
	{
		decorCatalog[i].idf = std::log((float)TOTAL_TEAMS / (float)std::max(1, decorCatalog[i].teamsUsed));
		order[i] = i;
	}
	for (int i = 0; i < DECOR_COUNT; i++)
		for (int j = i + 1; j < DECOR_COUNT; j++)
			if (decorCatalog[order[j]].idf > decorCatalog[order[i]].idf)
				std::swap(order[i], order[j]);
	for (int k = 0; k < 3; k++)
		decorCatalog[order[k]].selected = true;

	std::cout << "---- Decor IDF (N = " << TOTAL_TEAMS << ") ----" << std::endl;
	for (int i = 0; i < DECOR_COUNT; i++)
		std::cout << decorCatalog[i].name << "  df=" << decorCatalog[i].teamsUsed
				  << "  idf=" << decorCatalog[i].idf << (decorCatalog[i].selected ? "   <== used" : "") << std::endl;
}

int main()
{
	std::srand((unsigned int)std::time(nullptr));

	glfwInit();
	GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Card Game - Graphics Homework", NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetKeyCallback(window, key_callback);

	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

	glEnable(GL_DEPTH_TEST);

	stbi_set_flip_vertically_on_load(true);

	Shader ourShader("./shaders/vs/L4.vs", "./shaders/fs/L4.fs");

	selectDecorByIdf();

	GLuint texWhite = loadTexture(WHITE_PATH);
	GLuint texWallpaper = loadTexture(WALLPAPER_PATH);
	GLuint texFloor = loadTexture(FLOOR_PATH);
	GLuint texTableWood = loadTexture(TABLE_WOOD_PATH);
	GLuint texCloth = loadTexture(TABLE_CLOTH_PATH);
	GLuint texDarkWood = loadTexture(DARK_WOOD_PATH);
	GLuint texZoneDeck = loadTexture(ZONE_DECK_PATH);
	GLuint texZonePlay = loadTexture(ZONE_PLAY_PATH);
	GLuint texZoneOpp = loadTexture(ZONE_OPPONENT_PATH);
	GLuint texPoster = loadTexture(POSTER_PATH);
	GLuint texCardBack = loadTexture(CARD_BACK_PATH);
	GLuint texCardFace[5];
	for (int i = 0; i < 5; i++)
		texCardFace[i] = loadTexture(CARD_FACE_PATHS[i]);

	
	PolygonWithTexture floorQuad = makeQuad(vec3(-8, 0, 7), vec3(8, 0, 7), vec3(8, 0, -7), vec3(-8, 0, -7), 8.0f, 7.0f, texFloor, vec3(0.9f));
	PolygonWithTexture wallBack = makeQuad(vec3(-8, 0, -7), vec3(8, 0, -7), vec3(8, 5, -7), vec3(-8, 5, -7), 8.0f, 2.5f, texWallpaper, vec3(0.85f));
	PolygonWithTexture wallFront = makeQuad(vec3(8, 0, 7), vec3(-8, 0, 7), vec3(-8, 5, 7), vec3(8, 5, 7), 8.0f, 2.5f, texWallpaper, vec3(0.85f));
	PolygonWithTexture wallLeft = makeQuad(vec3(-8, 0, 7), vec3(-8, 0, -7), vec3(-8, 5, -7), vec3(-8, 5, 7), 7.0f, 2.5f, texWallpaper, vec3(0.85f));
	PolygonWithTexture wallRight = makeQuad(vec3(8, 0, -7), vec3(8, 0, 7), vec3(8, 5, 7), vec3(8, 5, -7), 7.0f, 2.5f, texWallpaper, vec3(0.85f));

	std::vector<PlacedBox> roomTrim;
	roomTrim.push_back(PlacedBox(texDarkWood, vec3(1.0f), vec3(0.0f, 0.18f, -6.93f), vec3(16.0f, 0.36f, 0.14f)));
	roomTrim.push_back(PlacedBox(texDarkWood, vec3(1.0f), vec3(0.0f, 0.18f, 6.93f), vec3(16.0f, 0.36f, 0.14f)));
	roomTrim.push_back(PlacedBox(texDarkWood, vec3(1.0f), vec3(-7.93f, 0.18f, 0.0f), vec3(0.14f, 0.36f, 14.0f)));
	roomTrim.push_back(PlacedBox(texDarkWood, vec3(1.0f), vec3(7.93f, 0.18f, 0.0f), vec3(0.14f, 0.36f, 14.0f)));

	
	std::vector<PlacedBox> tableParts;
	tableParts.push_back(PlacedBox(texTableWood, vec3(1.0f), vec3(0.0f, TABLE_TOP - 0.125f, 0.0f), vec3(TABLE_SIZE, 0.25f, TABLE_SIZE))); // السطح
	float legOffset = TABLE_SIZE / 2 - 0.35f;
	for (int sx = -1; sx <= 1; sx += 2)
		for (int sz = -1; sz <= 1; sz += 2)
			tableParts.push_back(PlacedBox(texDarkWood, vec3(1.0f), vec3(sx * legOffset, (TABLE_TOP - 0.25f) / 2, sz * legOffset), vec3(0.4f, TABLE_TOP - 0.25f, 0.4f)));

	PolygonWithTexture tableCloth = makeFlatRect(0.0f, TABLE_TOP + 0.002f, 0.0f, TABLE_SIZE - 0.5f, TABLE_SIZE - 0.5f, 5.0f, 5.0f, texCloth);
	PolygonWithTexture zoneOpponent = makeFlatRect(0.0f, TABLE_TOP + 0.004f, OPPONENT_Z, ZONE_W, ZONE_D, 1.0f, 1.0f, texZoneOpp);
	PolygonWithTexture zonePlay = makeFlatRect(0.0f, TABLE_TOP + 0.004f, PLAY_Z, ZONE_W, ZONE_D, 1.0f, 1.0f, texZonePlay);
	PolygonWithTexture zoneStock = makeFlatRect(0.0f, TABLE_TOP + 0.004f, STOCK_Z, ZONE_W, ZONE_D, 1.0f, 1.0f, texZoneDeck);

	
	for (int i = 0; i < 5; i++)
	{
		float hw = CARD_W / 2;
		std::vector<vec3> frontV = {vec3(-hw, 0, 0), vec3(hw, 0, 0), vec3(hw, CARD_H, 0), vec3(-hw, CARD_H, 0)};
		std::vector<vec2> frontT = {vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 1)};
		std::vector<vec3> backV = {vec3(-hw, 0, -0.003f), vec3(hw, 0, -0.003f), vec3(hw, CARD_H, -0.003f), vec3(-hw, CARD_H, -0.003f)};
		std::vector<vec2> backT = {vec2(1, 0), vec2(0, 0), vec2(0, 1), vec2(1, 1)};

		PolygonWithTexture front = PolygonWithTexture(frontV, frontT);
		front.setTexture(texCardFace[i]);
		PolygonWithTexture back = PolygonWithTexture(backV, backT);
		back.setTexture(texCardBack);
		cards.push_back(Card(front, back, stockHinge(i)));
	}

	
	const vec2 posterOutline[] = {
		vec2(-0.50f, -0.50f), vec2(0.50f, -0.50f), vec2(0.50f, 0.00f), vec2(0.43f, 0.07f),
		vec2(0.45f, 0.12f), vec2(0.36f, 0.17f), vec2(0.38f, 0.24f), vec2(0.27f, 0.28f),
		vec2(0.29f, 0.35f), vec2(0.17f, 0.38f), vec2(0.18f, 0.45f), vec2(0.05f, 0.46f),
		vec2(0.02f, 0.50f), vec2(-0.50f, 0.50f)};
	const int posterPoints = sizeof(posterOutline) / sizeof(posterOutline[0]);
	std::vector<vec3> posterV;
	std::vector<vec2> posterT;
	posterV.push_back(vec3(0.0f, 0.0f, 0.0f));
	posterT.push_back(vec2(0.5f, 0.5f));
	for (int i = 0; i <= posterPoints; i++)
	{
		vec2 p = posterOutline[i % posterPoints]; 
		posterV.push_back(vec3(p.x, p.y, 0.0f));
		posterT.push_back(vec2(p.x + 0.5f, p.y + 0.5f));
	}
	PolygonWithTexture poster = PolygonWithTexture(posterV, posterT, vec3(0.92f));
	poster.setTexture(texPoster);

	std::vector<vec3> tapeV = {vec3(-0.5f, -0.5f, 0), vec3(0.5f, -0.5f, 0), vec3(0.5f, 0.5f, 0), vec3(-0.5f, 0.5f, 0)};
	std::vector<vec2> tapeT = {vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 1)};
	PolygonWithTexture tape = PolygonWithTexture(tapeV, tapeT, vec3(0.62f, 0.56f, 0.42f));
	tape.setTexture(texWhite);

	
	std::vector<PlacedBox> lampParts;
	std::vector<PolygonWithTexture> lampShade;
	if (decorCatalog[DECOR_LAMP].selected)
	{
		const float lx = 7.0f, lz = -6.0f;
		lampParts.push_back(PlacedBox(texDarkWood, vec3(1.0f), vec3(lx, 0.04f, lz), vec3(0.7f, 0.08f, 0.7f)));						 
		lampParts.push_back(PlacedBox(texWhite, vec3(0.45f, 0.36f, 0.16f), vec3(lx, 1.55f, lz), vec3(0.07f, 3.0f, 0.07f)));		 
		vec3 warm = vec3(1.0f, 0.80f, 0.48f);
		float bw = 0.45f, tw = 0.25f, yb = 2.9f, yt = 3.55f;
		lampShade.push_back(makeQuad(vec3(lx - bw, yb, lz + bw), vec3(lx + bw, yb, lz + bw), vec3(lx + tw, yt, lz + tw), vec3(lx - tw, yt, lz + tw), 1.0f, 1.0f, texWhite, warm));
		lampShade.push_back(makeQuad(vec3(lx + bw, yb, lz - bw), vec3(lx - bw, yb, lz - bw), vec3(lx - tw, yt, lz - tw), vec3(lx + tw, yt, lz - tw), 1.0f, 1.0f, texWhite, warm));
		lampShade.push_back(makeQuad(vec3(lx - bw, yb, lz - bw), vec3(lx - bw, yb, lz + bw), vec3(lx - tw, yt, lz + tw), vec3(lx - tw, yt, lz - tw), 1.0f, 1.0f, texWhite, warm));
		lampShade.push_back(makeQuad(vec3(lx + bw, yb, lz + bw), vec3(lx + bw, yb, lz - bw), vec3(lx + tw, yt, lz - tw), vec3(lx + tw, yt, lz + tw), 1.0f, 1.0f, texWhite, warm));
	}


	GLuint texClockFace = 0;
	PolygonWithTexture *clockRing = nullptr, *clockFrame = nullptr, *clockFace = nullptr, *clockCap = nullptr;
	PolygonWithTexture *hourHand = nullptr, *minuteHand = nullptr, *secondHand = nullptr;
	const vec3 CLOCK_POS = vec3(7.99f, 3.4f, -4.0f);
	if (decorCatalog[DECOR_CLOCK].selected)
	{
		texClockFace = loadTexture(CLOCK_FACE_PATH);
		clockRing = new PolygonWithTexture(makeDisc(48, texDarkWood));									   
		clockFrame = new PolygonWithTexture(makeDisc(48, texWhite, vec3(0.62f, 0.46f, 0.18f)));		   
		clockFace = new PolygonWithTexture(makeDisc(48, texClockFace));									   
		clockCap = new PolygonWithTexture(makeDisc(24, texWhite, vec3(0.70f, 0.52f, 0.20f)));		   
		hourHand = new PolygonWithTexture(makePointed(texWhite, vec3(0.10f, 0.06f, 0.04f)));		   
		minuteHand = new PolygonWithTexture(makePointed(texWhite, vec3(0.10f, 0.06f, 0.04f)));		 
		secondHand = new PolygonWithTexture(makePointed(texWhite, vec3(0.65f, 0.07f, 0.09f)));		   
	}

	Model *gramophone = nullptr;
	if (decorCatalog[DECOR_GRAMOPHONE].selected)
		gramophone = new Model(GRAMOPHONE_MODEL_PATH);

	std::vector<PlacedBox> bladeParts;
	PolygonWithTexture *bladeSteel = nullptr;
	const vec3 BLADE_POS = vec3(-2.4f, 3.0f, -7.0f); 
	if (decorCatalog[DECOR_HIDDEN_BLADE].selected)
	{
		const float bx = BLADE_POS.x, by = BLADE_POS.y, bz = BLADE_POS.z;
		bladeParts.push_back(PlacedBox(texDarkWood, vec3(1.0f), vec3(bx, by, bz + 0.03f), vec3(2.2f, 1.0f, 0.06f)));						   
		bladeParts.push_back(PlacedBox(texWhite, vec3(0.38f, 0.06f, 0.09f), vec3(bx, by, bz + 0.065f), vec3(2.0f, 0.8f, 0.02f)));		   
		bladeParts.push_back(PlacedBox(texWhite, vec3(0.30f, 0.17f, 0.08f), vec3(bx - 0.50f, by - 0.05f, bz + 0.11f), vec3(0.75f, 0.30f, 0.08f)));  
		bladeParts.push_back(PlacedBox(texWhite, vec3(0.62f, 0.64f, 0.68f), vec3(bx - 0.50f, by - 0.05f, bz + 0.155f), vec3(0.55f, 0.20f, 0.03f))); 
		bladeParts.push_back(PlacedBox(texWhite, vec3(0.55f, 0.08f, 0.10f), vec3(bx - 0.70f, by - 0.05f, bz + 0.11f), vec3(0.07f, 0.34f, 0.10f)));  
		bladeParts.push_back(PlacedBox(texWhite, vec3(0.55f, 0.08f, 0.10f), vec3(bx - 0.32f, by - 0.05f, bz + 0.11f), vec3(0.07f, 0.34f, 0.10f)));  
		bladeParts.push_back(PlacedBox(texWhite, vec3(0.78f, 0.58f, 0.24f), vec3(bx - 0.10f, by - 0.05f, bz + 0.11f), vec3(0.06f, 0.34f, 0.10f)));  
		bladeSteel = new PolygonWithTexture(makePointed(texWhite, vec3(0.85f, 0.87f, 0.90f)));												
	}

	ourShader.use();

	std::cout << "Controls: 1 = top view | 2 = player view | G = draw random card | 3 = free camera (W/A/S/D + mouse) | ESC = exit" << std::endl;

	while (!glfwWindowShouldClose(window))
	{
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);
		updateCamera(deltaTime);
		updateCards(deltaTime);

		glClearColor(0.04f, 0.04f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mat4 projection = perspective(radians(FOV_DEGREES), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		ourShader.setMat4("projection", projection);
		mat4 view = camera.GetViewMatrix();
		ourShader.setMat4("view", view);

		mat4 transformation; 

		transformation = mat4(1.0f);
		floorQuad.transformation(transformation);
		wallBack.transformation(transformation);
		wallFront.transformation(transformation);
		wallLeft.transformation(transformation);
		wallRight.transformation(transformation);
		floorQuad.draw(ourShader);
		wallBack.draw(ourShader);
		wallFront.draw(ourShader);
		wallLeft.draw(ourShader);
		wallRight.draw(ourShader);
		for (size_t i = 0; i < roomTrim.size(); i++)
			roomTrim[i].draw(ourShader);

		for (size_t i = 0; i < lampParts.size(); i++)
			lampParts[i].draw(ourShader);
		for (size_t i = 0; i < lampShade.size(); i++)
		{
			lampShade[i].transformation(mat4(1.0f));
			lampShade[i].draw(ourShader);
		}

		if (clockFrame)
		{
			mat4 clockBase = mat4(1.0f);
			clockBase = glm::translate(clockBase, CLOCK_POS);
			clockBase = glm::rotate(clockBase, radians(-90.0f), vec3(0.0f, 1.0f, 0.0f));

			transformation = clockBase;
			transformation = glm::scale(transformation, vec3(1.0f, 1.0f, 1.0f)); 
			clockRing->transformation(transformation);
			clockRing->draw(ourShader);

			transformation = clockBase;
			transformation = glm::translate(transformation, vec3(0.0f, 0.0f, 0.007f));
			transformation = glm::scale(transformation, vec3(0.90f, 0.90f, 1.0f)); 
			clockFrame->transformation(transformation);
			clockFrame->draw(ourShader);

			transformation = clockBase;
			transformation = glm::translate(transformation, vec3(0.0f, 0.0f, 0.015f));
			transformation = glm::scale(transformation, vec3(0.82f, 0.82f, 1.0f)); 
			clockFace->transformation(transformation);
			clockFace->draw(ourShader);

			std::time_t now = std::time(nullptr);
			std::tm *lt = std::localtime(&now);
			float sec = (float)lt->tm_sec;
			float minute = lt->tm_min + sec / 60.0f;
			float hour = (lt->tm_hour % 12) + minute / 60.0f;

			struct HandSpec
			{
				PolygonWithTexture *hand;
				float angle;
				float length;
				float width;
				float z;
			};
			HandSpec hands[3] = {
				{hourHand, hour / 12.0f * 2.0f * PI, 0.45f, 0.09f, 0.030f},
				{minuteHand, minute / 60.0f * 2.0f * PI, 0.68f, 0.065f, 0.038f},
				{secondHand, sec / 60.0f * 2.0f * PI, 0.72f, 0.03f, 0.046f}};
			for (int h = 0; h < 3; h++)
			{
				transformation = clockBase;
				transformation = glm::translate(transformation, vec3(0.0f, 0.0f, hands[h].z));
				transformation = glm::rotate(transformation, -hands[h].angle, vec3(0.0f, 0.0f, 1.0f));
				transformation = glm::scale(transformation, vec3(hands[h].width, hands[h].length, 1.0f));
				hands[h].hand->transformation(transformation);
				hands[h].hand->draw(ourShader);
			}

			transformation = clockBase;
			transformation = glm::translate(transformation, vec3(0.0f, 0.0f, 0.054f));
			transformation = glm::scale(transformation, vec3(0.06f, 0.06f, 1.0f)); 
			clockCap->transformation(transformation);
			clockCap->draw(ourShader);
		}

		if (gramophone)
		{
			transformation = mat4(1.0f);
			transformation = glm::translate(transformation, vec3(-6.3f, 0.0f, 0.1f));
			transformation = glm::rotate(transformation, radians(70.0f), vec3(0.0f, 1.0f, 0.0f));
			transformation = glm::scale(transformation, vec3(0.8f, 0.8f, 0.8f));
			ourShader.setVec3("objectColor", vec3(1.0f, 1.0f, 1.0f));
			ourShader.setMat4("model", transformation);
			gramophone->Draw(ourShader);
		}

		for (size_t i = 0; i < bladeParts.size(); i++)
			bladeParts[i].draw(ourShader);
		if (bladeSteel)
		{
			transformation = mat4(1.0f);
			transformation = glm::translate(transformation, vec3(BLADE_POS.x - 0.10f, BLADE_POS.y - 0.05f, BLADE_POS.z + 0.11f));
			transformation = glm::rotate(transformation, radians(-90.0f), vec3(0.0f, 0.0f, 1.0f));
			transformation = glm::scale(transformation, vec3(0.14f, 1.0f, 1.0f));
			bladeSteel->transformation(transformation);
			bladeSteel->draw(ourShader);
		}

		for (size_t i = 0; i < tableParts.size(); i++)
			tableParts[i].draw(ourShader);
		tableCloth.transformation(mat4(1.0f));
		zoneOpponent.transformation(mat4(1.0f));
		zonePlay.transformation(mat4(1.0f));
		zoneStock.transformation(mat4(1.0f));
		tableCloth.draw(ourShader);
		zoneOpponent.draw(ourShader);
		zonePlay.draw(ourShader);
		zoneStock.draw(ourShader);

		transformation = mat4(1.0f);
		transformation = glm::translate(transformation, vec3(2.2f, 2.8f, -6.985f));
		transformation = glm::scale(transformation, vec3(2.4f, 3.2f, 1.0f));
		poster.transformation(transformation);
		poster.draw(ourShader);

		transformation = mat4(1.0f);
		transformation = glm::translate(transformation, vec3(1.05f, 4.35f, -6.975f));
		transformation = glm::rotate(transformation, radians(-35.0f), vec3(0.0f, 0.0f, 1.0f));
		transformation = glm::scale(transformation, vec3(0.5f, 0.16f, 1.0f));
		tape.transformation(transformation);
		tape.draw(ourShader);

		transformation = mat4(1.0f);
		transformation = glm::translate(transformation, vec3(3.35f, 1.25f, -6.975f));
		transformation = glm::rotate(transformation, radians(30.0f), vec3(0.0f, 0.0f, 1.0f));
		transformation = glm::scale(transformation, vec3(0.5f, 0.16f, 1.0f));
		tape.transformation(transformation);
		tape.draw(ourShader);

		for (size_t i = 0; i < cards.size(); i++)
		{
			Card &c = cards[i];

			float standAmount = smooth01(c.stand);
			float angleX = radians(-90.0f + standAmount * (90.0f - CARD_LEAN_DEGREES));

			float s = c.moving ? smooth01(c.moveT) : 0.0f;
			float roll = 0.20f * std::sin(PI * s);
			float pop = 1.0f + 0.15f * std::sin(PI * s);

			transformation = mat4(1.0f);
			transformation = glm::translate(transformation, c.hinge);
			transformation = glm::rotate(transformation, roll, vec3(0.0f, 0.0f, 1.0f));
			transformation = glm::rotate(transformation, angleX, vec3(1.0f, 0.0f, 0.0f));
			transformation = glm::scale(transformation, vec3(pop, pop, pop));

			c.front.transformation(transformation);
			c.back.transformation(transformation);
			c.front.draw(ourShader);
			c.back.draw(ourShader);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (!freeCam)
		return;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if (action != GLFW_PRESS)
		return;

	if (key == GLFW_KEY_1)
	{
		viewState = 1;
		freeCam = false;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		moveCameraTo(POSE_TOP);
	}
	else if (key == GLFW_KEY_2)
	{
		viewState = 2;
		freeCam = false;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		moveCameraTo(POSE_PLAYER);
	}
	else if (key == GLFW_KEY_3)
	{
		freeCam = true;
		firstMouse = true;
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	else if (key == GLFW_KEY_G)
	{
		drawRandomCard(); 
	}
}

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn)
{
	if (!freeCam)
		return;

	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
	if (height == 0)
		return;
	SCR_WIDTH = width;
	SCR_HEIGHT = height;
	glViewport(0, 0, width, height);
}
