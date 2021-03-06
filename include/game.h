#pragma once

#include "utilities.h"
#include "manager.h"
#include "render.h"

#include <cstdint>
#include <vector>
#include <list>
#include <filesystem>

// necessary forward declarations
class Manager;
class GameManager;
class ObjectWalker;
class UIManager;

class MapManager
{
private:
	GameManager *parent;
	Renderer *renderer;
	TextureManager *texture_manager;

	std::vector<std::filesystem::path> maps;

	int current_map;
	int spawn_x, spawn_y;
	std::vector<std::vector<std::vector<TextureAccess>>> tile;
	std::vector<std::vector<bool>> collision;

public:
	MapManager(GameManager *parent);

	void loadMap(int map, bool respawn = false);

	void getSpawn(int *x, int *y);
	bool getCollision(int pos_x, int pos_y);

	void getSize(int *x, int *y);

	void render();

private:
	void resizeMapStorage(int x, int y, bool absolute = false);
};

class GameObject
{
protected:
	GameManager *parent;
	Renderer *renderer;
	InputHandler *input_handler;
	MapManager *map_manager;

	// render stuff
	ObjectWalker *object_walker;

	std::vector<TextureAccess> up;
	std::vector<TextureAccess> down;
	std::vector<TextureAccess> side;

	int current_frame;
	int loop_frame;
	int end_frame;
	int stop_frame;
	DIR dir;

	int screen_x;
	int screen_y;

	bool camera_center;

	// collision stuff
	int map_x;
	int map_y;
	int size_x;
	int size_y;

	bool collision;

protected:
	GameObject(GameManager *parent);

public:
	virtual ~GameObject();

	void setScreenPos(int x, int y, bool anim = true);
	void getScreenPos(int *x, int *y);
	void getCenter(int *x, int *y);

	bool isCameraCenter();

	void setMapPos(int x, int y, bool anim = true);
	void getMapPos(int *x, int *y);
	void getSize(int *x, int *y);
	bool checkMapCollision(int offset_x, int offset_y);
	bool checkObjectCollision(int offset_x, int offset_y);

	void render();
	virtual bool collide();
	virtual void runTick(uint64_t delta);


private:
	// ObjectWalker specific
	//void _setScreenPos(int x, int y);

	void advanceFrame(DIR dir);
	void stopFrame(DIR dir);

	friend class ObjectWalker;
};

class ObjectWalker
{
	/*
	 * a helper class to smoothly move
	 * game objects across tiles
	 */

private:
	// measured in ms/pixel
	const uint64_t SPEED = 5;
	// ms/animation frame, reccomended multiple of SPEED
	const uint64_t FRAME_TIME = SPEED * 10;

	GameObject *parent;
	int dest_x, dest_y;
	uint64_t tick;
	uint64_t movement_deadline;
	uint64_t animation_deadline;

public:
	ObjectWalker(GameObject *parent);

	void setDestination(int x, int y);
	//void cancel();

	void runTick(uint64_t delta);
};

class Player : public GameObject
{
private:
	int type;

public:
	Player(GameManager *parent, int type);
	~Player() = default;

	void runTick(uint64_t delta);
};

class StaticObject : public GameObject
{
public:
	StaticObject(
		GameManager *parent, 
		std::filesystem::path texture_path,
		int size_x, int size_y,
		int map_x, int map_y
	);
	~StaticObject() = default;
};

class PickupObject : public GameObject
{
private:
	std::string hint;

public:
	PickupObject(
		GameManager *parent, 
		std::filesystem::path texture_path,
		int size_x, int size_y,
		int map_x, int map_y,
		std::string hint
	);
	~PickupObject() = default;

	bool collide();
};

class QuizManager
{
private:
	GameManager *parent;
	//UIManager *ui_manager;

	struct QUESTION {
		std::string text;
		std::vector<std::string> answers;
		std::vector<bool> correct;
	};

	std::vector<QUESTION> questions;

	bool in_quiz;
	int question_asked;
	bool have_answer;
	std::vector<bool> answer;

public:
	QuizManager(GameManager *parent);

	void startQuiz();
	void provideAnswer(std::vector<bool> answer);

	void runTick(uint64_t delta);
};

class GameManager
{
private:
	Manager *parent;
	Renderer *renderer;
	MapManager map_manager;
	QuizManager quiz_manager;

	std::list<GameObject *> objects;
	std::vector<std::vector<GameObject *>> collision;

	uint64_t playtime;

	bool paused;

	int collectibles;
	int collected;
	std::list<std::string> hints;

public:
	GameManager(Manager *parent);
	~GameManager();

	Manager *getManager();
	Renderer *getRenderer();
	MapManager *getMapManager();
	QuizManager *getQuizManager();
	Player *getPlayer();

	void loadObject(std::filesystem::path object_path, int map_x, int map_y);
	void unloadObject(GameObject *object);

	void updateCollision();
	GameObject *getCollision(int pos_x, int pos_y);

	uint64_t getPlaytime();

	void setPaused(bool paused);
	bool getPaused();

	int getCollected();
	int getRemaining();
	int getTotalCollectibles();
	void addCollectible();
	void useCollectible();
	void addHint(std::string hint);
	std::list<std::string> getHints();

	void runTick(uint64_t delta);
};
