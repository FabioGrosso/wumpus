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
    void AddFrontier(list<Location> &locationList, const Location &location);
    bool WithinWorld(Location& location);
    void Output();
    void FindWumpus();
    void Arrowwumpus();
    void ResetSafeLocations();
    void ResetFrontier();
    void ResetpossibleWumpus();
    void RemoveFrontier(Location& location);
    void FilterFrontier();
    void UpdateSafeLocation();
    Location UnvisitFrontier();
    void RemoveSafeLocation(int x, int y);

    list<Location> frontier;
    list<Location> possibleWumpus;
    list<Location> stenchLocations;
    list<Location> pitLocations;
    list<Location> breezeLocations;
    list<Location> safeLocations;
    list<Location> goodLocations;
    list<Location> visitedLocations;
    bool firstTry;
    bool onlyWumpus;
    WorldState worldState;
    Action previousAction;
    Action pre;

};

#endif // AGENT_H
