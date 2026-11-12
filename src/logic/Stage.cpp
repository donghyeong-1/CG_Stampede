#include "logic/Stage.h"

namespace
{
Animal MakeAnimal(int id, AnimalType type, float x, float z, float speed)
{
    Animal animal;
    animal.id = id;
    animal.type = type;
    animal.position = {x, 0.0f, z};
    animal.speed = speed;
    animal.halfSize = type == AnimalType::Gazelle ? glm::vec3(0.4f, 0.7f, 0.8f) : glm::vec3(0.55f, 0.85f, 1.1f);
    return animal;
}
} // namespace

void BuildStage(GameState &state)
{
    state.animals.clear();
    state.obstacles.clear();
    state.hoveredAnimalId = -1;
    state.resultSuccess = false;
    state.time = 0.0f;

    // 임시 배치. 균열(z=25)은 가젤, 바위(z=45)는 들소로 넘도록 배치를 조정한다.
    state.animals.push_back(MakeAnimal(0, AnimalType::Gazelle, 0.0f, 2.0f, 3.0f)); // 시작 탑승 동물.
    state.animals.push_back(MakeAnimal(1, AnimalType::Bison, -2.0f, 6.0f, 3.0f));
    state.animals.push_back(MakeAnimal(2, AnimalType::Gazelle, 2.0f, 9.0f, 3.2f));
    state.animals.push_back(MakeAnimal(3, AnimalType::Bison, 1.5f, 14.0f, 2.8f));
    state.animals.push_back(MakeAnimal(4, AnimalType::Gazelle, -1.5f, 18.0f, 3.1f));
    state.animals.push_back(MakeAnimal(5, AnimalType::Bison, 0.0f, 24.0f, 3.0f));

    Obstacle crack;
    crack.type = ObstacleType::Crack;
    crack.position = {0.0f, 0.0f, 25.0f};
    crack.halfSize = {cfg::CANYON_HALF_WIDTH, 0.1f, 1.0f};
    state.obstacles.push_back(crack);

    Obstacle rock;
    rock.type = ObstacleType::Rock;
    rock.position = {0.0f, 0.0f, 45.0f};
    rock.halfSize = {1.2f, 1.0f, 1.0f};
    state.obstacles.push_back(rock);

    PlayerState player;
    player.mountedAnimalId = 0;
    state.player = player;
    state.player.position = state.animals[0].position + glm::vec3(0.0f, state.animals[0].halfSize.y * 2.0f, 0.0f);
}
