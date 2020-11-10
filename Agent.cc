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
    if (pre == GOFORWARD){
        Location tmp;
        if (worldState.agentOrientation == RIGHT) {
            tmp.X = worldState.agentLocation.X + 1;
            tmp.Y = worldState.agentLocation.Y;
        }else if (worldState.agentOrientation == LEFT) {
            tmp.X = worldState.agentLocation.X - 1;
            tmp.Y = worldState.agentLocation.Y;
        }else if (worldState.agentOrientation == UP) {
            tmp.X = worldState.agentLocation.X;
            tmp.Y = worldState.agentLocation.Y + 1;
        }else if (worldState.agentOrientation == DOWN) {
            tmp.X = worldState.agentLocation.X;
            tmp.Y = worldState.agentLocation.Y - 1;
        }
        AddLocation(pitLocations, tmp);
        FilterFrontier();
    }
    worldState.agentLocation = Location(1,1);
    worldState.agentOrientation = RIGHT;
    worldState.agentHasArrow = true;
    worldState.agentHasGold = false;
    worldState.wumpusAlive = true;
    previousAction = CLIMB;
    actionList.clear();
    if (firstTry) {
        worldState.wumpusLocation = Location(0,0);
        worldState.goldLocation = Location(0,0);
        worldState.worldSize = 0;
    }

    else {
        if (!(worldState.goldLocation == Location(0,0))) {
            list<Action> actionList2;
            list<Action> actionList1;
            Location shootLocation = worldState.agentLocation;
            Orientation ori = worldState.agentOrientation;
            if (!(worldState.wumpusLocation == Location(0,0))){
                shootLocation = stenchLocations.front();
                if (shootLocation.X == worldState.wumpusLocation.X){
                    if (shootLocation.Y > worldState.wumpusLocation.Y){
                        ori = DOWN;
                    }else {
                        ori = UP;
                    }
                }else {
                    if (shootLocation.X > worldState.wumpusLocation.X){
                        ori = LEFT;
                    }else {
                        ori = RIGHT;
                    }
                }
                actionList2 = searchEngine.FindPath(worldState.agentLocation, worldState.agentOrientation, shootLocation, ori);
                actionList.splice(actionList.end(), actionList2);
                actionList.push_back(SHOOT);
            }
            actionList1 = searchEngine.FindPath(shootLocation, ori, worldState.goldLocation, worldState.agentOrientation);
            actionList.splice(actionList.end(), actionList1);
            actionList.push_back(GRAB);
            worldState.agentHasGold = true;
        }
    }
}

Action Agent::Process (Percept& percept)
{
	list<Action> actionList2;
	Location shootLocation;
	Orientation ori;
	UpdateState (percept);
    if (percept.Glitter) {
        actionList.clear();
        actionList.push_back(GRAB);
        actionList2 = searchEngine.FindPath(worldState.agentLocation, worldState.agentOrientation, Location(1,1), worldState.agentOrientation);
        actionList.splice(actionList.end(), actionList2);
    }
    if (actionList.empty()) {
	    if (worldState.agentHasGold && (worldState.agentLocation == Location(1,1))) {
	        actionList.push_back(CLIMB);
	    }
	    else if (visitedLocations.size() < safeLocations.size()){
            actionList2 = searchEngine.FindPath(worldState.agentLocation, worldState.agentOrientation, UnvisitSafeLoction (), worldState.agentOrientation);
            actionList.splice(actionList.end(), actionList2);
        }
        else if (!worldState.agentHasArrow && worldState.wumpusAlive) {
            actionList.clear();
            actionList2 = searchEngine.FindPath(worldState.agentLocation, worldState.agentOrientation, Location(1,1), worldState.agentOrientation);
            actionList.splice(actionList.end(), actionList2);
            actionList.push_back(CLIMB);
        }
	    else if (worldState.wumpusAlive && !(worldState.wumpusLocation == Location(0, 0)) &&
                 worldState.agentHasArrow){
            shootLocation = stenchLocations.front();
            if (shootLocation.X == worldState.wumpusLocation.X){
                if (shootLocation.Y > worldState.wumpusLocation.Y){
                    ori = DOWN;
                }else {
                    ori = UP;
                }
            }else {
                if (shootLocation.X > worldState.wumpusLocation.X){
                    ori = LEFT;
                }else {
                    ori = RIGHT;
                }
            }
            actionList2 = searchEngine.FindPath(worldState.agentLocation, worldState.agentOrientation, shootLocation, ori);
            actionList.splice(actionList.end(), actionList2);
            actionList.push_back(SHOOT);
	    }
	    else if (worldState.wumpusLocation == Location(0,0) && !possibleWumpus.empty() && worldState.agentHasArrow) {
            cout << "22222222222222222222222222222";
	        Location target = possibleWumpus.front();
//	        possibleWumpus.pop_front();
            shootLocation = stenchLocations.front();
            if (shootLocation.X == target.X){
                if (shootLocation.Y > target.Y){
                    ori = DOWN;
                }else {
                    ori = UP;
                }
            }else {
                if (shootLocation.X > target.X){
                    ori = LEFT;
                }else {
                    ori = RIGHT;
                }
            }
            actionList2 = searchEngine.FindPath(worldState.agentLocation, worldState.agentOrientation, shootLocation, ori);
            actionList.splice(actionList.end(), actionList2);
            actionList.push_back(SHOOT);
	    }
	    else if (!frontier.empty()){
            cout << "alllllllllllll";
            Location unsafeLocation = UnvisitFrontier();
            AddLocation(safeLocations, unsafeLocation);
            actionList2 = searchEngine.FindPath(worldState.agentLocation, worldState.agentOrientation, unsafeLocation, worldState.agentOrientation);
            safeLocations.remove(unsafeLocation);
            searchEngine.RemoveSafeLocation(unsafeLocation.X, unsafeLocation.Y);
            actionList.splice(actionList.end(), actionList2);
	    }
	}
        Action action = actionList.front();
        actionList.pop_front();
        previousAction = action;
        pre = action;
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
                    ResetFrontier ();
                    ResetpossibleWumpus();
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
            worldState.agentHasArrow = false;
            if (percept.Scream){
                worldState.wumpusAlive = false;
//                possibleWumpus.clear();
                switch (worldState.agentOrientation) {
                    case RIGHT:
                        worldState.wumpusLocation = Location(worldState.agentLocation.X + 1, worldState.agentLocation.Y);
                        break;
                    case UP:
                        worldState.wumpusLocation = Location(worldState.agentLocation.X, worldState.agentLocation.Y + 1);
                        break;
                    case LEFT:
                        worldState.wumpusLocation = Location(worldState.agentLocation.X - 1, worldState.agentLocation.Y);
                        break;
                    case DOWN:
                        worldState.wumpusLocation = Location(worldState.agentLocation.X, worldState.agentLocation.Y - 1);
                        break;
                }
                if (!InList(pitLocations,worldState.wumpusLocation)) {
                    AddLocation(safeLocations,worldState.wumpusLocation);
                }
//                if (onlyWumpus) {
//                    AddLocation(safeLocations,worldState.wumpusLocation);
//                }
            }else {
                switch (worldState.agentOrientation) {
                    case RIGHT:
                        possibleWumpus.remove(Location(worldState.agentLocation.X + 1, worldState.agentLocation.Y));
                        break;
                    case UP:
                        possibleWumpus.remove(Location(worldState.agentLocation.X, worldState.agentLocation.Y + 1));
                        break;
                    case LEFT:
                        possibleWumpus.remove(Location(worldState.agentLocation.X - 1, worldState.agentLocation.Y));
                        break;
                    case DOWN:
                        possibleWumpus.remove(Location(worldState.agentLocation.X, worldState.agentLocation.Y - 1));
                        break;
                }
            }
            break;
    }

    // update safe locations, visit locations, stench locations and breeze locations
    AddLocation (safeLocations, worldState.agentLocation);
    AddLocation (visitedLocations, worldState.agentLocation);
    frontier.remove(worldState.agentLocation);
    if (percept.Stench) {
        AddLocation (stenchLocations, worldState.agentLocation);
    }
    if (!percept.Stench) {
        AddLocation (goodLocations, worldState.agentLocation);
        AddAdjacentLocations (goodLocations, worldState.agentLocation);
    }
    if (percept.Breeze) {
        AddLocation (breezeLocations, worldState.agentLocation);
        AddFrontier(frontier,worldState.agentLocation);
    }
    if ((!percept.Stench && !percept.Breeze) || (!percept.Breeze && !worldState.wumpusAlive)) {
        AddAdjacentLocations (safeLocations, worldState.agentLocation);
//        RemoveFrontier (worldState.agentLocation);
    }
    // Rule 2f find the Wumpus
    if (worldState.wumpusLocation == Location(0,0) && !stenchLocations.empty() && worldState.wumpusAlive) {
        Arrowwumpus();
    }
    // if we found the Wumpus, check if we could add some safe locations
    if (worldState.wumpusLocation == Location(0,0)){
    } else{
        UpdateSafeLocation();
    }
    if (!(worldState.wumpusLocation == Location(0,0)) && worldState.agentLocation == worldState.wumpusLocation) {
        onlyWumpus = true;
    }
    if (!percept.Breeze) {
        RemoveFrontier (worldState.agentLocation);
    }
    Output();
}

void Agent::Arrowwumpus() {
    Location location1, location2;
    list<Location>::iterator itr1, itr2;
    list<Location> tmps;
    list<Location> adjacents;
    for (itr1 = stenchLocations.begin(); itr1 != stenchLocations.end(); ++itr1) {
        location1 = *itr1;
        adjacents.clear();
        AddAdjacentLocations(adjacents, location1);
        if (possibleWumpus.empty()) {
            possibleWumpus = adjacents;
        } else {
            tmps = possibleWumpus;
            possibleWumpus.clear();
            for (itr2 = tmps.begin(); itr2 != tmps.end(); ++itr2) {
                location2 = *itr2;
                if (InList(adjacents, location2)) {
                    possibleWumpus.push_back(location2);
                }
            }
        }
    }
    tmps = possibleWumpus;
    possibleWumpus.clear();
    for (itr1 = tmps.begin(); itr1 != tmps.end(); ++itr1) {
        location1 = *itr1;
        if (InList(goodLocations, location1)) continue;
        possibleWumpus.push_back(location1);
    }
    if (possibleWumpus.size() == 1){
        worldState.wumpusLocation = possibleWumpus.front();
    }
}

void Agent::FindWumpus () {
    Location location1, location2;
    location1 = stenchLocations.back();
    for (list<Location>::iterator itr = stenchLocations.begin(); itr != stenchLocations.end(); ++itr) {
        location2 = *itr;
        if (location2.X == location1.X - 1 && location2.Y == location1.Y + 1){
            cout << "ori1111111111111111111111111111111111111111111111111111111: " ;
            if (InList (goodLocations,Location(location1.X - 1,location1.Y))){
                worldState.wumpusLocation = Location(location1.X,location1.Y + 1);
            }else if (InList (goodLocations,Location(location1.X,location1.Y + 1))){
                worldState.wumpusLocation = Location(location1.X - 1,location1.Y);
            }
        }else if (location2.X == location1.X + 1 && location2.Y == location1.Y - 1){
            if (InList (goodLocations,Location(location1.X,location1.Y - 1))){
                worldState.wumpusLocation = Location(location1.X + 1,location1.Y);
            }else if (InList (goodLocations,Location(location1.X + 1,location1.Y))){
                worldState.wumpusLocation = Location(location1.X,location1.Y - 1);
            }
        }else if (location2.X == location1.X - 1 && location2.Y == location1.Y - 1){
            if (InList (goodLocations,Location(location1.X,location1.Y - 1))){
                worldState.wumpusLocation = Location(location1.X - 1,location1.Y);
            }else if (InList (goodLocations,Location(location1.X - 1,location1.Y))){
                worldState.wumpusLocation = Location(location1.X,location1.Y - 1);
            }
        }else if (location2.X == location1.X + 1 && location2.Y == location1.Y + 1){
            if (InList (goodLocations,Location(location1.X,location1.Y + 1))){
                worldState.wumpusLocation = Location(location1.X + 1,location1.Y);
            }else if (InList (goodLocations,Location(location1.X + 1,location1.Y))){
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
    return Location(0,0);
}

Location Agent::UnvisitFrontier () {
    for (list<Location>::iterator itr = frontier.begin(); itr != frontier.end(); ++itr) {
        Location location = *itr;
        // try to find a unsafe unvisited location within the world
        if (!InList (visitedLocations, location) && WithinWorld (location)) {
            cout << "tmp unsafe Location: (" << location.X << "," <<location.Y << ")\n";
            return location;
        }
    }
    return Location(0,0);
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

void Agent::RemoveFrontier(Location& location) {
    Location loc1 = Location(location.X, location.Y + 1);
    Location loc2 = Location(location.X, location.Y - 1);
    Location loc3 = Location(location.X + 1, location.Y);
    Location loc4 = Location(location.X - 1, location.Y);
    list<Location> tmpLocations = frontier;
    frontier.clear();
    for (list<Location>::iterator itr = tmpLocations.begin(); itr != tmpLocations.end(); ++itr) {
        Location location1 = *itr;
        if (location1 == loc1 || location1 == loc2 || location1 == loc3 || location1 == loc4) continue;
        frontier.push_back(location1);
    }
}

void Agent::FilterFrontier() {
    list<Location> tmpLocations = frontier;
    frontier.clear();
    for (list<Location>::iterator itr = tmpLocations.begin(); itr != tmpLocations.end(); ++itr) {
        Location location = *itr;
        if (InList(pitLocations, location)) continue;
        frontier.push_back(location);
    }
}

void Agent::AddFrontier (list<Location> &locationList, const Location &location) {
    int worldSize = worldState.worldSize;
    if (((worldSize == 0) || (location.Y < worldSize)) && !InList(safeLocations,Location(location.X, location.Y + 1)) && !InList(pitLocations,Location(location.X, location.Y + 1))) {
        AddLocation (locationList, Location(location.X, location.Y + 1));
    }
    if (location.Y > 1 && !InList(safeLocations,Location(location.X, location.Y - 1)) && !InList(pitLocations,Location(location.X, location.Y - 1))) {
        AddLocation (locationList, Location(location.X, location.Y - 1));
    }
    if (location.X > 1 && !InList(safeLocations,Location(location.X - 1, location.Y)) && !InList(pitLocations,Location(location.X - 1, location.Y))) {
        AddLocation (locationList, Location(location.X - 1, location.Y));
    }
    if (((worldSize == 0) || (location.X < worldSize)) && !InList(safeLocations,Location(location.X + 1, location.Y)) && !InList(pitLocations,Location(location.X + 1, location.Y))) {
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

void Agent::ResetFrontier() {
    int worldSize = worldState.worldSize;
    list<Location> tmpLocations = frontier;
    frontier.clear();
    for (list<Location>::iterator itr = tmpLocations.begin(); itr != tmpLocations.end(); ++itr) {
        Location location = *itr;
        if ((location.X < 1) || (location.Y < 1)) continue;
        if ((worldSize > 0) && ((location.X > worldSize) || (location.Y > worldSize))) {
            continue;
        }
        frontier.push_back(location);
    }
}

void Agent::ResetpossibleWumpus() {
    int worldSize = worldState.worldSize;
    list<Location> tmpLocations = possibleWumpus;
    possibleWumpus.clear();
    for (list<Location>::iterator itr = tmpLocations.begin(); itr != tmpLocations.end(); ++itr) {
        Location location = *itr;
        if ((location.X < 1) || (location.Y < 1)) continue;
        if ((worldSize > 0) && ((location.X > worldSize) || (location.Y > worldSize))) {
            continue;
        }
        possibleWumpus.push_back(location);
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
    if (worldState.wumpusAlive == true){
        safeLocations.remove(worldState.wumpusLocation);
        searchEngine.RemoveSafeLocation(worldState.wumpusLocation.X,worldState.wumpusLocation.Y);
    }
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
    cout << "onlyWumpus: " << onlyWumpus << endl;
    cout << "World Size: " << worldState.worldSize << endl;
    cout << "current Location: (" << worldState.agentLocation.X << "," << worldState.agentLocation.Y << ")\n";
    cout << "wumpus Location: (" << worldState.wumpusLocation.X << "," <<worldState.wumpusLocation.Y << ")\n";
    cout << "Gold Location: (" << worldState.goldLocation.X << "," << worldState.goldLocation.Y << ")\n";
    cout << "possibleWumpus:";
    for (itr = possibleWumpus.begin(); itr != possibleWumpus.end(); ++itr) {
        location = *itr;
        cout << " (" << location.X << "," << location.Y << ")";
    }
    cout << endl;
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
    cout << "Frontier:";
    for (itr = frontier.begin(); itr != frontier.end(); ++itr) {
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
    cout << "pit Locations:";
    for (itr = pitLocations.begin(); itr != pitLocations.end(); ++itr) {
        location = *itr;
        cout << " (" << location.X << "," << location.Y << ")";
    }
    cout << endl;
    cout << "Good Locations:";
    for (itr = goodLocations.begin(); itr != goodLocations.end(); ++itr) {
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

