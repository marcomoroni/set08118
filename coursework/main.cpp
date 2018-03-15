#include <SFML\Graphics.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <memory>

#define WINDOW_HEIGHT 800
#define WINDOW_WIDTH 1200
#define M_PI 3.14159265358979323846
#define ACCENT_COLOUR Color(153, 153, 255)
#define NEUTRAL_COLOR Color(70, 70, 70)
#define GRID_COLOR Color(220, 220, 220)
#define BACKGROUND_COLOR Color(245, 245, 250)
#define CAVE_HIGHLIGHT_COLOR Color(0, 0, 0, 15)

using namespace std;
using namespace sf;

class Node
{
public:
	Node() = default;
	string name;
	CircleShape shape;
	Vector2i pos;
	vector<Node*> connections = {};
	Node* parent;
};

class AI
{
public:
	AI() = default;
	vector<Node*> openSet;
	vector<Node*> closedSet;
	Node* currentNode;
	map<Node*, float> fs;
	map<Node*, float> gs;

	// Execute one step of A*
	void nextStep();

	bool isFinished = false;
};

AI ai;

int noOfCaverns;
vector<Node*> caverns;
Node* startCavern;
Node* endCavern;
vector<Node*> finalPath;

// Visual elements
vector<CircleShape> dots; // Grid
CircleShape caveHighlight; // To highlight current cave
vector<RectangleShape> tunnels;

float biggestXCoord = 0.f; // Used to draw grid
float biggestYCoord = 0.f; // Used to draw grid
float margin = 100.f; // Used to draw grid

bool autoMode = false; // is the user going to do step by step?

// Calculate heuristic: euclidian distance from to end node
float calculate_h(Node* current, Node* end)
{
	return sqrt((end->pos.x - current->pos.x) * (end->pos.x - current->pos.x) + (end->pos.y - current->pos.y) * (end->pos.y - current->pos.y));
}

// Calculate path distance from beginning
float calculate_g(float currentG, Node* current, Node* connection)
{
	return currentG + sqrt((connection->pos.x - current->pos.x) * (connection->pos.x - current->pos.x) + (connection->pos.y - current->pos.y) * (connection->pos.y - current->pos.y));
}

// A* (one step) -----------------------------------------------------------------------------------------
void AI::nextStep()
{
	cout << "Current node: " << currentNode->name << endl;
	if (!openSet.empty())
	{

		for (auto connection : currentNode->connections)
		{
			// if connected node is not in closed set
			if (!(find(closedSet.begin(), closedSet.end(), connection) != closedSet.end()))
			{
				openSet.push_back(connection);
			}
		}

		cout << "Open set:";
		for (auto c : openSet)
		{
			cout << " " << c->name;
		}
		cout << endl;

		cout << "Closed set:";
		for (auto c : closedSet)
		{
			cout << " " << c->name;
		}
		cout << endl;

		for (auto cavern : openSet)
		{
			if (cavern != currentNode)
			{
				// Calculate evaluation function
				float tempG = calculate_g(gs[currentNode], currentNode, cavern);
				float tempF = tempG + calculate_h(cavern, endCavern);
				cout << "Evaluation function for node " << cavern->name << ": " << to_string(tempF) << endl;

				// 
				if (fs.find(cavern) == fs.end() || tempF < fs[cavern])
				{
					gs[cavern] = tempG;
					fs[cavern] = tempF;

					if (find(currentNode->connections.begin(), currentNode->connections.end(), cavern) != currentNode->connections.end())
					{
						cavern->parent = currentNode;
					}
				}
			}
		}

		// Move current node from open set to closed set
		openSet.erase(remove(openSet.begin(), openSet.end(), currentNode), openSet.end());
		closedSet.push_back(currentNode);

		// Select new current node from open set with the lowest f
		Node* newCurrentNode = openSet.at(0);
		for (auto n : openSet)
		{
			if (fs[n] <= fs[newCurrentNode])
			{
				newCurrentNode = n;
			}
		}
		currentNode = newCurrentNode;

		// If end cavern reached
		if (currentNode == endCavern)
		{
			isFinished = true;
		}

		cout << endl;

	}
}
// -------------------------------------------------------------------------------------------------------

// Recursively get parent node to recreate the final path
void reconstruct_final_path(Node* c)
{
	finalPath.push_back(c);
	if (c->parent != nullptr)
	{
		reconstruct_final_path(c->parent);
	}
}

void Load()
{
	string path = "res/input.cav";

	string buffer;

	// Load in file to buffer
	ifstream f(path);
	if (f.good())
	{
		f.seekg(0, ios::end);
		buffer.resize(f.tellg());
		f.seekg(0);
		f.read(&buffer[0], buffer.size());
		f.close();
	}
	else
	{
		throw string("Couldn't open cavern file: ") + path;
	}

	// DEBUG
	cout << "Buffer: " << buffer << endl;
	cout << endl;

	// Get numbers
	// Store the in vector `data`. When a value is read, delete it
	stringstream ss(buffer);
	vector<int> data;
	while (ss.good())
	{
		string substr;
		getline(ss, substr, ',');
		data.push_back(stoi(substr));
	}

	// Number of caves
	noOfCaverns = data.at(0);
	data.erase(data.begin());
	cout << "noOfCaverns: " << to_string(noOfCaverns) << endl;
	cout << endl;

	// Store cavern coordinates
	// and give them a name
	for (int i = 0; i < noOfCaverns; i++)
	{
		auto newCavern = new Node();
		newCavern->pos = { data.at(0), data.at(1) };
		newCavern->name = to_string(i + 1);
		data.erase(data.begin());
		data.erase(data.begin());
		caverns.push_back(newCavern);

		// First and last caverns
		if (i == 0) {
			startCavern = newCavern;
		}
		if (i == noOfCaverns - 1)
		{
			endCavern = newCavern;
		}

		// Set biggest coords
		// (for visualisation)
		if (newCavern->pos.x > biggestXCoord)
		{
			biggestXCoord = newCavern->pos.x;
		}
		if (newCavern->pos.y > biggestYCoord)
		{
			biggestYCoord = newCavern->pos.y;
		}
	}
	for (int i = 0; i < noOfCaverns; i++)
	{
		auto currentCavern = caverns.at(i);
		cout << "Cavern " << currentCavern->name << " coordinates: " << to_string(currentCavern->pos.x) << " " << to_string(currentCavern->pos.y) << endl;
	}
	cout << endl;

	// Connections
	for (int cavernNo = 0; cavernNo < noOfCaverns; cavernNo++)
	{
		auto currentCavern = caverns.at(cavernNo);
		for (int connectionNo = 0; connectionNo < noOfCaverns; connectionNo++)
		{
			// If there is a connection
			if (data.at(0) == 1)
			{
				//currentCavern->connections.push_back(caverns.at(connectionNo));
				caverns.at(connectionNo)->connections.push_back(currentCavern);
			}

			data.erase(data.begin());
		}
	}
	for (int cavernNo = 0; cavernNo < noOfCaverns; cavernNo++)
	{
		auto currentCavern = caverns.at(cavernNo);
		for (auto connection : currentCavern->connections)
		{
			cout << "Cavern " << currentCavern->name << " connects to cave " << connection->name << endl;
		}
	}
	cout << endl;

	// Setup grid (visualisation)
	for (int x = 0; x < (int)biggestXCoord + 1; x++)
	{
		for (int y = 0; y < (int)biggestYCoord + 1; y++)
		{
			CircleShape newDot;
			newDot.setPosition(x * (WINDOW_WIDTH - margin * 2) / biggestXCoord + margin, y * (WINDOW_HEIGHT - margin * 2) / biggestYCoord + margin);
			newDot.setRadius(2.f);
			newDot.setOrigin(2.f, 2.f);
			newDot.setFillColor(GRID_COLOR);
			dots.push_back(newDot);
		}
	}

	// Setup circles for caves (visualisation)
	for (auto cavern : caverns)
	{
		cavern->shape.setPosition(cavern->pos.x * (WINDOW_WIDTH - margin * 2) / biggestXCoord + margin, cavern->pos.y * (WINDOW_HEIGHT - margin * 2) / biggestYCoord + margin);	
		// Mirror on y axis, because the center in SFML the y axis is positive when going down
		cavern->shape.setPosition(cavern->shape.getPosition().x, WINDOW_HEIGHT - cavern->shape.getPosition().y);	
		cavern->shape.setRadius(10.f);
		cavern->shape.setOrigin(10.f, 10.f);
		cavern->shape.setFillColor(NEUTRAL_COLOR);

		// Different colour for first and last cavern
		if (cavern == startCavern)
		{
			//cavern->shape.setFillColor(Color(92, 214, 92, 255));
			//cavern->shape.setFillColor(Color(51, 153, 255));
			cavern->shape.setFillColor(ACCENT_COLOUR);
		}
		if (cavern == endCavern)
		{
			//cavern->shape.setFillColor(Color(255, 80, 80, 255));
			cavern->shape.setFillColor(ACCENT_COLOUR);
		}
	}

	// Setup tunnels (visualisation)
	for (auto cavern : caverns)
	{
		for (auto connection : cavern->connections)
		{
			// Set dimentions
			float lenght = sqrt((cavern->shape.getPosition().x - connection->shape.getPosition().x) * (cavern->shape.getPosition().x - connection->shape.getPosition().x) + (cavern->shape.getPosition().y - connection->shape.getPosition().y) * (cavern->shape.getPosition().y - connection->shape.getPosition().y));
			lenght = lenght - 20.f; // show that is one way only
			RectangleShape line({lenght, 3.f});
			line.setOrigin({ 0, 1.5f });
			// Set position
			line.setPosition(cavern->shape.getPosition());
			// Set rotation
			auto angle = atan2(connection->shape.getPosition().y - cavern->shape.getPosition().y, connection->shape.getPosition().x - cavern->shape.getPosition().x);
			line.setRotation(angle * (180 / M_PI));
			// Set style
			line.setFillColor(NEUTRAL_COLOR);

			tunnels.push_back(line);
		}
	}

	// Set up cave highlight (visualisation)
	caveHighlight.setRadius(20.f);
	caveHighlight.setOrigin(20.f, 20.f);
	caveHighlight.setFillColor(CAVE_HIGHLIGHT_COLOR);

	// Setup AI
	ai = AI();
	ai.openSet.push_back(startCavern);
	ai.currentNode = startCavern;
	ai.gs[startCavern] = 0.0f;
	ai.fs[startCavern] = 0.0f + calculate_h(startCavern, endCavern);

}

float inputCooldown = 0;
bool shrinkCaveHiglight = false;

void Update(RenderWindow &window)
{
	// Reset clock, recalculate deltatime
	static Clock clock;
	float dt = clock.restart().asSeconds();

	// Check and consume events
	Event event;
	while (window.pollEvent(event)) {
		if (event.type == Event::Closed) {
			window.close();
			return;
		}
	}

	// Quit via ESC Key
	if (Keyboard::isKeyPressed(Keyboard::Escape)) {
		window.close();
	}

	// Excecute an ai step
	if (!autoMode)
	{
		if (Keyboard::isKeyPressed(Keyboard::Space) && !ai.isFinished && inputCooldown < 0)
		{
			ai.nextStep();
			inputCooldown = 0.2f;
		}
	}
	else
	{
		while (!ai.isFinished)
		{
			ai.nextStep();
		}
	}

	// Final path
	if (ai.isFinished && finalPath.empty())
	{
		reconstruct_final_path(endCavern);

		// Draw coloured tunnels (visualisation)
		for (auto c : finalPath)
		{
			if (c->parent != nullptr)
			{
				// Set dimentions
				float lenght = sqrt((c->parent->shape.getPosition().x - c->shape.getPosition().x) * (c->parent->shape.getPosition().x - c->shape.getPosition().x) + (c->parent->shape.getPosition().y - c->shape.getPosition().y) * (c->parent->shape.getPosition().y - c->shape.getPosition().y));
				lenght = lenght - 20.f; // show that is one way only
				RectangleShape line({ lenght, 3.f });
				line.setOrigin({ 0, 1.5f });
				// Set position
				line.setPosition(c->parent->shape.getPosition());
				// Set rotation
				auto angle = atan2(c->shape.getPosition().y - c->parent->shape.getPosition().y, c->shape.getPosition().x - c->parent->shape.getPosition().x);
				line.setRotation(angle * (180 / M_PI));
				// Set style
				line.setFillColor(ACCENT_COLOUR);

				tunnels.push_back(line);
			}
		}

		cout << "Final path:";
		reverse(finalPath.begin(), finalPath.end());
		for (auto c : finalPath)
		{
			cout << " " << c->name;
		}
		cout << endl;
	}

	// Change mode
	if (Keyboard::isKeyPressed(Keyboard::M) && inputCooldown < 0)
	{
		autoMode = !autoMode;
	}

	if (inputCooldown >= 0) inputCooldown -= dt;

	// Highlight current node
	if (!ai.isFinished) caveHighlight.setPosition(ai.currentNode->shape.getPosition());
	else caveHighlight.setPosition({ -100.f, -100.f });

	// Cave higlight effect
	Vector2f newScale;
	if (shrinkCaveHiglight) newScale = { caveHighlight.getScale().x - 0.2f * dt, caveHighlight.getScale().y - 0.2f * dt };
	else newScale = { caveHighlight.getScale().x + 0.2f * dt, caveHighlight.getScale().y + 0.2f * dt };
	if (caveHighlight.getScale().x > 1.2f) shrinkCaveHiglight = true;
	if (caveHighlight.getScale().x < 1.0f) shrinkCaveHiglight = false;
	caveHighlight.setScale(newScale);

}

void Render(RenderWindow &window) {
	window.draw(caveHighlight);
	for (auto dot : dots) window.draw(dot);
	for (auto tunnel : tunnels) window.draw(tunnel);
	for (auto cavern : caverns) window.draw(cavern->shape);
}

int main()
{
	RenderWindow window(VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "AI coursework");
	Load();
	while (window.isOpen())
	{
		window.clear(BACKGROUND_COLOR);
		Update(window);
		Render(window);
		window.display();
	}

	return 0;
}