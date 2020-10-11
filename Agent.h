// Agent.h

#ifndef AGENT_H
#define AGENT_H

#include "Action.h"
#include "Percept.h"

#include "Location.h"
#include "Orientation.h"
#include "Search.h"
#include <list>

class Agent
{
public:
	Agent ();
	~Agent ();
	void Initialize ();
	Action Process (Percept& percept);
	void GameOver (int score);

	list<Action> actionList;
	SearchEngine searchEngine;
    void UpdateState (Percept& percept);
    Location UnvisitSafeLoction();
    void Move();
    bool InList(list<Location>& locationList, const Location& location);
    void AddLocation(list<Location>& locationList, const Location& location);
    void AddAdjacentLocations(list<Location>& locationList, const Location& location);
    bool WithinWorld(Location& location);
    void Output();
    void FindWumpus();
    void ResetSafeLocations();
    void UpdateSafeLocation();
    void RemoveSafeLocation(int x, int y);

    list<Location> stenchLocations;
    list<Location> breezeLocations;
    list<Location> safeLocations;
    list<Location> visitedLocations;
    bool firstTry;
    WorldState worldState;
    Action previousAction;

};

#endif // AGENT_H
