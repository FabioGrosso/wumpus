// Agent.cc
//
// This code works only for the testworld that comes with the simulator.

#include <iostream>
#include <list>
#include "Agent.h"
#include <algorithm>

using namespace std;

Agent::Agent ()
{
    firstTry = true;
}

Agent::~Agent ()
{

}

void Agent::Initialize ()
{
    worldState.agentLocation = Location(1,1);
    worldState.agentOrientation = RIGHT;
    worldState.agentHasArrow = true;
    worldState.agentHasGold = false;
    actionList.clear();
    previousAction = CLIMB;
    if (firstTry) {
        worldState.wumpusLocation = Location(0,0);
        worldState.goldLocation = Location(0,0);
        worldState.worldSize = 0;
    } else {
        if (worldState.goldLocation == Location(0,0)) {
        }
        // Rule 5 go to the gold location directly
        else {
            actionList = searchEngine.FindPath(Location(1,1), RIGHT, worldState.goldLocation, RIGHT);
            actionList.splice(actionList.end(), actionList);
			actionList.push_back(GRAB);
			worldState.agentHasGold = true;
        }
    }
}

Action Agent::Process (Percept& percept)
{
	list<Action> actionList2;
	UpdateState (percept);
	// Rule 2b and 6
    if (percept.Glitter) {
        actionList.clear();
        actionList.push_back(GRAB);
        actionList2 = searchEngine.FindPath(worldState.agentLocation, worldState.agentOrientation, Location(1,1), RIGHT);
        actionList.splice(actionList.end(), actionList2);
    }
    // Rule 4
    if (actionList.empty()) {
	    if (worldState.agentHasGold && (worldState.agentLocation == Location(1,1))) {
	        actionList.push_back(CLIMB);
	    }
	    // Rule 7
	    else {
            actionList2 = searchEngine.FindPath(worldState.agentLocation, worldState.agentOrientation, UnvisitSafeLoction (), RIGHT);
            actionList.splice(actionList.end(), actionList2);
        }
	}
    if (actionList.empty()) {
        Action action = GOFORWARD;
        previousAction = action;
        return action;
    }
        Action action = actionList.front();
        actionList.pop_front();
        previousAction = action;
        return action;
}

void Agent::GameOver (int score)
{
    firstTry = false;
}

void Agent::UpdateState (Percept& percept) {
    int orientationInt = (int) worldState.agentOrientation;
    switch (previousAction) {
        case GOFORWARD:

            if (percept.Bump) {
                // Rule 2c try to set the world size
                if (worldState.agentOrientation == RIGHT) {
                    worldState.worldSize = worldState.agentLocation.X;
                }
                if (worldState.agentOrientation == UP) {
                    worldState.worldSize = worldState.agentLocation.Y;
                }
                if (worldState.worldSize > 0) {
                    // if we got the world size, we could remove some safe locations outside the world
                    ResetSafeLocations ();
                }
            } else {
                Move();
            }
            break;

        case TURNLEFT:
            worldState.agentOrientation = (Orientation) ((orientationInt + 1) % 4);
            break;

        case TURNRIGHT:
            orientationInt--;
            if (orientationInt < 0) orientationInt = 3;
            worldState.agentOrientation = (Orientation) orientationInt;
            break;

        // Rule 3
        case GRAB:
            worldState.agentHasGold = true;
            worldState.goldLocation = worldState.agentLocation;
            break;

        case CLIMB:
            break;

        case SHOOT:
            break;
    }

    // update safe locations, visit locations, stench locations and breeze locations
    AddLocation (safeLocations, worldState.agentLocation);
    AddLocation (visitedLocations, worldState.agentLocation);
    if (percept.Stench) {
        AddLocation (stenchLocations, worldState.agentLocation);
    }
    if (percept.Breeze) {
        AddLocation (breezeLocations, worldState.agentLocation);
    }
    if (!percept.Stench && !percept.Breeze) {
        AddAdjacentLocations (safeLocations, worldState.agentLocation);
    }
    // Rule 2f find the Wumpus
    if (worldState.wumpusLocation == Location(0,0) && stenchLocations.size() > 1) {
        FindWumpus ();
    }
    // if we found the Wumpus, check if we could add some safe locations
    if (worldState.wumpusLocation == Location(0,0)){
    } else{
        UpdateSafeLocation();
    }
    Output();
}

void Agent::FindWumpus () {
    Location location1, location2;
    location1 = stenchLocations.back();
    for (list<Location>::iterator itr = stenchLocations.begin(); itr != stenchLocations.end(); ++itr) {
        location2 = *itr;
        if (location2.X == location1.X - 1 && location2.Y == location1.Y + 1){
            if (InList (safeLocations,Location(location1.X - 1,location1.Y))){
                worldState.wumpusLocation = Location(location1.X,location1.Y + 1);
            }else if (InList (safeLocations,Location(location1.X,location1.Y + 1))){
                worldState.wumpusLocation = Location(location1.X - 1,location1.Y);
            }
        }else if (location2.X == location1.X + 1 && location2.Y == location1.Y - 1){
            if (InList (safeLocations,Location(location1.X,location1.Y - 1))){
                worldState.wumpusLocation = Location(location1.X + 1,location1.Y);
            }else if (InList (safeLocations,Location(location1.X + 1,location1.Y))){
                worldState.wumpusLocation = Location(location1.X,location1.Y - 1);
            }
        }else if (location2.X == location1.X - 1 && location2.Y == location1.Y - 1){
            if (InList (safeLocations,Location(location1.X,location1.Y - 1))){
                worldState.wumpusLocation = Location(location1.X - 1,location1.Y);
            }else if (InList (safeLocations,Location(location1.X - 1,location1.Y))){
                worldState.wumpusLocation = Location(location1.X,location1.Y - 1);
            }
        }else if (location2.X == location1.X + 1 && location2.Y == location1.Y + 1){
            if (InList (safeLocations,Location(location1.X,location1.Y + 1))){
                worldState.wumpusLocation = Location(location1.X + 1,location1.Y);
            }else if (InList (safeLocations,Location(location1.X + 1,location1.Y))){
                worldState.wumpusLocation = Location(location1.X,location1.Y + 1);
            }
        }
    }
}


Location Agent::UnvisitSafeLoction () {
    for (list<Location>::iterator itr = safeLocations.begin(); itr != safeLocations.end(); ++itr) {
        Location location = *itr;
        // try to find a safe unvisited location within the world
        if (!InList (visitedLocations, location) && WithinWorld (location)) {
            cout << "tmp Location: (" << location.X << "," <<location.Y << ")\n";
            return location;
        }
    }
}

// update state after go forward
void Agent::Move () {
    switch (worldState.agentOrientation) {
        case RIGHT:
            worldState.agentLocation.X++;
            break;
        case UP:
            worldState.agentLocation.Y++;
            break;
        case LEFT:
            worldState.agentLocation.X--;
            break;
        case DOWN:
            worldState.agentLocation.Y--;
            break;
    }
}

// check if a location is in the corresponding list
bool Agent::InList (list<Location> &locationList, const Location &location) {
    if (find(locationList.begin(), locationList.end(), location) != locationList.end()) {
        return true;
    }
    return false;
}

// add a location to the corresponding list and the searchEngine
void Agent::AddLocation (list<Location> &locationList, const Location &location) {
    if (!InList (locationList, location)) {
        locationList.push_back(location);}
    if (locationList == safeLocations){
        searchEngine.AddSafeLocation(location.X,location.Y);
    }
}

// add adjacent locations to corresponding list (if within the world)
void Agent::AddAdjacentLocations (list<Location> &locationList, const Location &location) {
    int worldSize = worldState.worldSize;
    if ((worldSize == 0) || (location.Y < worldSize)) {
        AddLocation (locationList, Location(location.X, location.Y + 1));
    }
    if (location.Y > 1) {
        AddLocation (locationList, Location(location.X, location.Y - 1));
    }
    if (location.X > 1) {
        AddLocation (locationList, Location(location.X - 1, location.Y));
    }
    if ((worldSize == 0) || (location.X < worldSize)) {
        AddLocation (locationList, Location(location.X + 1, location.Y));
    }
}

// remove safe location outside the world
void Agent::ResetSafeLocations () {
    int worldSize = worldState.worldSize;
    list<Location> tmpLocations = safeLocations;
    safeLocations.clear();
    for (list<Location>::iterator itr = tmpLocations.begin(); itr != tmpLocations.end(); ++itr) {
        Location location = *itr;
        searchEngine.RemoveSafeLocation(location.X,location.Y);
        if ((location.X < 1) || (location.Y < 1)) continue;
        if ((worldSize > 0) && ((location.X > worldSize) || (location.Y > worldSize))) {
            continue;
        }
        safeLocations.push_back(location);
        searchEngine.AddSafeLocation(location.X,location.Y);
    }
}

// Try to use the Wumpus location to add some new safe location
void Agent::UpdateSafeLocation () {
    for (list<Location>::iterator itr = stenchLocations.begin(); itr != stenchLocations.end(); ++itr) {
        Location location = *itr;
        // if a stench location is also a breeze location, do nothing
        if (InList(breezeLocations,location)) continue;
        //otherwise add adjacent locations to safe locations.
        AddAdjacentLocations (safeLocations, location);
    }
    // don't forget to remove the Wumpus location from the safe location list
    safeLocations.remove(worldState.wumpusLocation);
    searchEngine.RemoveSafeLocation(worldState.wumpusLocation.X,worldState.wumpusLocation.Y);
}

// check if a location is within the world
bool Agent::WithinWorld (Location& location) {
    int worldSize = worldState.worldSize;
    if ((location.X < 1) || (location.Y < 1)) return false;
    if ((worldSize > 0) && ((location.X > worldSize) || (location.Y > worldSize))) return false;
    return true;
}


void Agent::Output () {
    list<Location>::iterator itr;
    Location location;
    cout << "World Size: " << worldState.worldSize << endl;
    cout << "current Location: (" << worldState.agentLocation.X << "," << worldState.agentLocation.Y << ")\n";
    cout << "wumpus Location: (" << worldState.wumpusLocation.X << "," <<worldState.wumpusLocation.Y << ")\n";
    cout << "Gold Location: (" << worldState.goldLocation.X << "," << worldState.goldLocation.Y << ")\n";
    cout << "Visited Locations:";
    for (itr = visitedLocations.begin(); itr != visitedLocations.end(); ++itr) {
        location = *itr;
        cout << " (" << location.X << "," << location.Y << ")";
    }
    cout << endl;
    cout << "Safe Locations:";
    for (itr = safeLocations.begin(); itr != safeLocations.end(); ++itr) {
        location = *itr;
        cout << " (" << location.X << "," << location.Y << ")";
    }
    cout << endl;
    cout << "stench Locations:";
    for (itr = stenchLocations.begin(); itr != stenchLocations.end(); ++itr) {
        location = *itr;
        cout << " (" << location.X << "," << location.Y << ")";
    }
    cout << endl;
    cout << "Action List:";
    for (list<Action>::iterator itr_a = actionList.begin(); itr_a != actionList.end(); ++itr_a) {
        cout << " ";
        PrintAction(*itr_a);
    }
    cout << endl;
    cout << endl;
}

