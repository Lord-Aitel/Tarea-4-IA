#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <memory>
#include <algorithm>

const int MAP_WIDTH = 80;
const int MAP_HEIGHT = 25;
const int MIN_ROOM_SIZE = 8;
const char WALL_CHAR = '#';
const char FLOOR_CHAR = '.';

struct Room {
    int x, y, width, height;

    Room(int x_, int y_, int w_, int h_) : x(x_), y(y_), width(w_), height(h_) {}

    int centerX() const { return x + width / 2; }
    int centerY() const { return y + height / 2; }
};

struct BSPNode {
    Room area;
    std::unique_ptr<BSPNode> left;
    std::unique_ptr<BSPNode> right;
    Room* room = nullptr; 

    BSPNode(Room area_) : area(area_) {}

    bool isLeaf() const { return !left && !right; }
};

std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());

bool splitNode(BSPNode& node) {
    if (!node.isLeaf()) return false;

    bool splitH = rng() % 2;
    if (node.area.width > node.area.height && node.area.width / node.area.height >= 1.25) splitH = false;
    else if (node.area.height > node.area.width && node.area.height / node.area.width >= 1.25) splitH = true;

    int max = (splitH ? node.area.height : node.area.width) - MIN_ROOM_SIZE;
    if (max < MIN_ROOM_SIZE) return false;

    std::uniform_int_distribution<int> dist(MIN_ROOM_SIZE, max);
    int split = dist(rng);

    if (splitH) {
        node.left = std::make_unique<BSPNode>(Room(node.area.x, node.area.y, node.area.width, split));
        node.right = std::make_unique<BSPNode>(Room(node.area.x, node.area.y + split, node.area.width, node.area.height - split));
    } else {
        node.left = std::make_unique<BSPNode>(Room(node.area.x, node.area.y, split, node.area.height));
        node.right = std::make_unique<BSPNode>(Room(node.area.x + split, node.area.y, node.area.width - split, node.area.height));
    }
    return true;
}

void createBSP(BSPNode& node) {
    if (splitNode(node)) {
        createBSP(*node.left);
        createBSP(*node.right);
    }
}

void createRooms(BSPNode& node, std::vector<std::vector<char>>& map, std::vector<Room*>& roomList) {
    if (node.isLeaf()) {
    std::uniform_int_distribution<int> wDist(MIN_ROOM_SIZE, node.area.width); 
    std::uniform_int_distribution<int> hDist(MIN_ROOM_SIZE, node.area.height); 

        int w = wDist(rng);
        int h = hDist(rng);

        std::uniform_int_distribution<int> xDist(0, node.area.width - w);
        std::uniform_int_distribution<int> yDist(0, node.area.height - h);

        int x = node.area.x + xDist(rng);
        int y = node.area.y + yDist(rng);

        node.room = new Room(x, y, w, h);
        roomList.push_back(node.room);

        for (int i = y; i < y + h; ++i)
            for (int j = x; j < x + w; ++j)
                map[i][j] = FLOOR_CHAR;
    } else {
        if (node.left) createRooms(*node.left, map, roomList);
        if (node.right) createRooms(*node.right, map, roomList);
    }
}

void createCorridor(std::vector<std::vector<char>>& map, Room* a, Room* b) {
    int x1 = a->centerX(), y1 = a->centerY();
    int x2 = b->centerX(), y2 = b->centerY();

    if (rng() % 2) {
        for (int x = std::min(x1, x2); x <= std::max(x1, x2); ++x) map[y1][x] = FLOOR_CHAR;
        for (int y = std::min(y1, y2); y <= std::max(y1, y2); ++y) map[y][x2] = FLOOR_CHAR;
    } else {
        for (int y = std::min(y1, y2); y <= std::max(y1, y2); ++y) map[y][x1] = FLOOR_CHAR;
        for (int x = std::min(x1, x2); x <= std::max(x1, x2); ++x) map[y2][x] = FLOOR_CHAR;
    }
}

void connectRooms(std::vector<Room*>& rooms, std::vector<std::vector<char>>& map) {
    for (size_t i = 1; i < rooms.size(); ++i) {
        createCorridor(map, rooms[i - 1], rooms[i]);
    }
}

void initializeMap(std::vector<std::vector<char>>& map) {
    map.assign(MAP_HEIGHT, std::vector<char>(MAP_WIDTH, WALL_CHAR));
}

void printMap(const std::vector<std::vector<char>>& map) {
    for (const auto& row : map) {
        for (char cell : row) std::cout << cell;
        std::cout << '\n';
    }
}

int main() {
    std::vector<std::vector<char>> map;
    initializeMap(map);

    BSPNode root(Room(0, 0, MAP_WIDTH, MAP_HEIGHT));
    createBSP(root);

    std::vector<Room*> rooms;
    createRooms(root, map, rooms);
    connectRooms(rooms, map);

    printMap(map);

    for (Room* r : rooms) delete r;
    return 0;
}