// Project Identifier: 950181F63D0A883F183EC0A5CC67B19928FE896A
#include <iostream>
#include <string>
#include <sstream>
#include <stack>
#include <queue>
#include <utility>
#include <cmath>
#include <vector>
#include <algorithm>
#include <stdint.h>
#include <deque>
#include <map>
#include <getopt.h>

using namespace std;
class Map{
    public:
    
    struct Location{
        //coordinates
        int level, row, col;
        
        //constructors
        Location(): level(0), row(0), col(0) {}
        Location(int f, int r, int c) : level(f), row(r), col(c){}
        Location(const Location &l) {
                    level = l.level;
                    row = l.row;
                    col = l.col;
        }
        
        //set Location
        Location operator=(const Location& a) {
            level = a.level;
            row = a.row;
            col = a.col;
            return *this;
        }
        
        //Location equals operator, true when level, row, and col are the same
        bool operator==(const Location& a) {
            if ((this->level == a.level) && (this->row == a.row) && (this->col == a.col)){
                return true;
            }
            return false;
        }
        
        // Location not equal operator, true when level, row, col aren't all the same
        bool operator!=(const Location& a) {
            if ((this->level != a.level) || (this->row != a.row) || (this->col != a.col)){
                return true;
            }
            return false;
        }

    };
                           
    struct Tile{
        // Options: . # E S H and approach
        char type, loc;
        // if tile was discovered
        bool discovered;
        
        //undiscovered default
        Tile(): type('.'), loc('n'), discovered(false) {}
        Tile(const Tile &s) {
            type = s.type;
            loc = s.loc;
            discovered = s.discovered;
        }
        
        //set Tile
        Tile operator=(const Tile& a) {
            type = a.type;
            loc = a.loc;
            discovered = a.discovered;
            return *this;
        }
    };
    
    //functions
    void mode(int argc, char * argv[]);
    void input();
    void path_search();
    void direction_check(char direction, Tile* dTile, Location* robot, bool* run);
    void backtrace();
    void backtrace_direction(char location, Location* robot);
    void output();
    void validPathOutput();
    void noPathOutput();
    
    private:
        //3D Vector of the whole map of the ship (Tiles)
        vector<vector<vector<Tile>>> ship;
        //Search container to be used as a stack or a queue
        deque<Location> search;
        
        //Start and end
        Location startL, endL;
        
        //how to route container (queue or stack) and input output
        char route_type, imode, omode;
        int level_num, size;
        
        //if there is a path or not, default to false;
        bool path = false;
};

// help statements
void help(char *argv[]) {
    cout << "Usage: " << argv[0] << "-p|s --output(M|L)" << endl;
    cout << "Help: Back to the Ship Game. You start at Start and if a path to the hangar exists, the robot finds it and shows you how to get there \n";
    cout << "Flags: --stack, -s: if set, use stack-based routing scheme \n";
    cout << "--queue, -q: if set, use queue-based routing scheme \n";
    cout << "--output (M|L), -o(M|L): Output file format, M = map L = List, defaults to map";
}

//Get the input, output, and search modes from command line
void Map::mode(int argc, char * argv[]) {
    //if stay false then need to go to error
    bool haveOutputType = false;
    bool haveRouteType = false;
    string mode;
    
    //Check lab for getopt help
    opterr = false;
    //option but long_op
    int opt;
    int optionCount = 0;
    option long_options[] = {
        { "stack", no_argument, nullptr, 's'},
        { "queue", no_argument, nullptr, 'q'},
        { "output", required_argument, nullptr, 'o'},
        { "help", no_argument, nullptr, 'h'},
        { nullptr, 0, nullptr, '\0'}
    };

    // help, output, stack, and queue flags
    while ((opt = getopt_long(argc, argv, "sqo:h", long_options, &optionCount)) != -1) {
        switch (opt) {
            case 'h':
                help(argv);
                exit(0);

            case 'o':
                mode = optarg;
                haveOutputType = true;
                omode = static_cast<char>(mode[0]);
                break;
                    
            case 's':
                if (haveRouteType) {
                    cerr << "Error: Multiple routing modes specified" << endl;
                    exit(1);
                }
                haveRouteType = true;
                route_type = 's';
                break;
                    
            case 'q':
                if (haveRouteType) {
                    cerr << "Error: Multiple routing modes specified" << endl;
                    exit(1);
                }
                haveRouteType = true;
                route_type = 'q';
                break;

            default:
                cerr << "Error: invalid flag" << endl;
                exit(1);
        }
    }
    
    // Error if there's no route type identified
    if (!haveRouteType) {
        cerr << "Error: No routing mode specified" << endl;
        exit(1);
    }
    
    // map defeault
    if (!haveOutputType) {
        omode = 'M';
        haveOutputType = true;
    }
}

void Map::input(){
    //Initialize a deafult Tile
    Tile sq;
    
    //read
    string junk, input;
    cin >> imode >> level_num >> size;
    if (level_num > 10) {
        cerr << "Error: Invalid map level" << endl;
        exit(1);
    }
    
    ship.resize(static_cast<unsigned long>(level_num), vector<vector<Tile>>(static_cast<unsigned long>(size), vector<Tile>(static_cast<unsigned long>(size), sq)));
    
    // if input mode == map
    if (imode == 'M') {
        int currR = 0;
        int currF = 0;
        //getline(cin, junk);
        while (getline(cin, input)) {
            //checks for comment and blank line
            if (input[0] != '/' && input != "") {
                for (int currC = 0; currC < size; ++currC) {
                    //current char = string indexed to correct char by col (array)
                    char currChar = input[static_cast<std::string::size_type>(currC)];
                    
                    // error checking, is the char a real char?
                    if (!(currChar == 'S' || currChar == 'H' || currChar == 'E' || currChar == '#' || currChar == '.')) {
                        cerr << "Error: Invalid map character" << endl;
                        exit(1);
                    }
                    
                    // check to see if char is S and set startL to all the same vals
                    if (currChar == 'S') {
                        startL.level = currF;
                        startL.row = currR;
                        startL.col = currC;
                    }
                    // store the Tile type
                    ship[static_cast<vector<vector<vector<Tile>>>::size_type>(currF)][static_cast<vector<vector<Tile>>::size_type>(currR)][static_cast<vector<Tile>::size_type>(currC)].type = currChar;
                }
                //increase row by 1
                ++currR;
                //if at the end of the row, reset row and increase level by 1, avoids three nested for loops
                if (currR == size) {
                    ++currF;
                    currR = 0;
                }
            }
        }
    }
    // if input mode == list
    if (imode == 'L') {
        int clevel, crow, ccol;
        char input;
        while (cin >> input) {
            if (input != '/' && input != ' ') {
                // read in each line: level space row space col space type, input = junk
                cin >> clevel >> input >> crow >> input >> ccol >> input >> sq.type >> input;
                
                // check for any errors like out of bounds size or incorrect char
                if (clevel >= level_num) {
                    cerr << "Error: Invalid map level" << endl;
                    exit(1);
                }
                if (crow >= size) {
                    cerr << "Error: Invalid map row" << endl;
                    exit(1);
                }
                if (ccol >= size) {
                    cerr << "Error: Invalid map column" << endl;
                    exit(1);
                }
                if (!(sq.type == 'S' || sq.type == 'H' || sq.type == 'E' || sq.type == '#' || sq.type == '.')) {
                    cerr << "Error: Invalid map character" << endl;
                    exit(1);
                }
                
                // if Tile is the start, make the level, row, and col of that location startL
                if (sq.type == 'S') {
                    startL = {clevel, crow, ccol};
                }
                
                //make the current Tile a part of ship
                ship[static_cast<vector<vector<vector<Tile>>>::size_type>(clevel)][static_cast<vector<vector<Tile>>::size_type>(crow)][static_cast<vector<Tile>::size_type>(ccol)] = sq;
            } else {
                getline(cin, junk);
            }
        }
    }
}

void Map::direction_check(char direction, Tile* dTile, Location* robot, bool* run){
    //defaults to north
    int r = 0;
    int c = 0;
    
    if (direction == 'n'){
        r = -1;
        c = 0;
    }
    if (direction == 'e'){
        r = 0;
        c = 1;
    } else if (direction == 's'){
        r = 1;
        c = 0;
    } else if (direction == 'w'){
        r = 0;
        c = -1;
    }
    
    if  (dTile->type != '#' && !dTile->discovered) {
        //go into if tile is the hanger and then break out of loop
        if (dTile->type == 'H') {
            path = true;
            endL.level = robot->level;
            
            int temp_row = static_cast<int>(robot->row);
            int temp_col = static_cast<int>(robot->col);
            temp_row = temp_row + r;
            temp_col = temp_col + c;
            
            endL.row = static_cast<int>(temp_row);
            endL.col = static_cast<int>(temp_col);
            
            if (direction == 'n'){ ship[static_cast<vector<vector<vector<Tile>>>::size_type>(endL.level)][static_cast<vector<vector<Tile>>::size_type>(endL.row)][static_cast<vector<Tile>::size_type>(endL.col)].loc = 's'; }
            else if (direction == 'e'){ ship[static_cast<vector<vector<vector<Tile>>>::size_type>(endL.level)][static_cast<vector<vector<Tile>>::size_type>(endL.row)][static_cast<vector<Tile>::size_type>(endL.col)].loc = 'w'; }
            else if (direction == 's'){ ship[static_cast<vector<vector<vector<Tile>>>::size_type>(endL.level)][static_cast<vector<vector<Tile>>::size_type>(endL.row)][static_cast<vector<Tile>::size_type>(endL.col)].loc = 'n'; }
            else if (direction == 'w'){ ship[static_cast<vector<vector<vector<Tile>>>::size_type>(endL.level)][static_cast<vector<vector<Tile>>::size_type>(endL.row)][static_cast<vector<Tile>::size_type>(endL.col)].loc = 'e'; }
            
            *run = false;
        } else {
            // is discovered
            dTile->discovered = true;
            
            // direction came from
            if (direction == 'n'){ dTile->loc = 's'; }
            else if (direction == 'e'){ dTile->loc = 'w'; }
            else if (direction == 's'){ dTile->loc = 'n'; }
            else if (direction == 'w'){ dTile->loc = 'e'; }
            
            //push to search
            int temp_row = static_cast<int>(robot->row);
            int temp_col = static_cast<int>(robot->col);
            temp_row = temp_row + r;
            temp_col = temp_col + c;
            int new_row = static_cast<int>(temp_row);
            int new_col = static_cast<int>(temp_col);
            
            Location newL(robot->level, new_row, new_col);
            search.push_back(newL);
        }
    }
}


void Map::path_search() {
    //push start location into search container
    search.push_back(startL);
    //change start tile to discovered
    ship[static_cast<vector<vector<vector<Tile>>>::size_type>(startL.level)][static_cast<vector<vector<Tile>>::size_type>(startL.row)][static_cast<vector<Tile>::size_type>(startL.col)].discovered = true;
    Location robotL;
    
    //keep running search till hit hangar and run == false;
    bool run = true;
    //while the search container still has tiles to search off of
    while (!search.empty() && run){
        // stack LIFO
        if (route_type == 's') {
            robotL = search.back();
            search.pop_back();
        } else {
            robotL = search.front();
            search.pop_front();
        }
        //current = cTile
        Tile* current = &ship[(unsigned long)robotL.level][(unsigned long)robotL.row][(unsigned long)robotL.col];
        
        // direction checks no wall and not discovered
        if  (robotL.row != 0) {
            //northern = dTile
            Tile* northern = &ship[static_cast<std::vector<std::vector<std::vector<Tile>>>::size_type>(robotL.level)][static_cast<std::vector<std::vector<std::vector<Tile>>>::size_type>(robotL.row) - 1][static_cast<std::vector<std::vector<std::vector<Tile>>>::size_type>(robotL.col)];

            direction_check('n', northern, &robotL, &run);
        }
        if  (robotL.col < (size - 1)) {
            Tile* eastern = &ship[static_cast<std::vector<std::vector<std::vector<Tile>>>::size_type>(robotL.level)][static_cast<std::vector<std::vector<std::vector<Tile>>>::size_type>(robotL.row)][(static_cast<std::vector<std::vector<std::vector<Tile>>>::size_type>(robotL.col)) + 1];
            direction_check('e', eastern, &robotL, &run);
        }
        if  (robotL.row < (size - 1)){
            Tile* southern = &ship[static_cast<std::vector<std::vector<std::vector<Tile>>>::size_type>(robotL.level)][(static_cast<std::vector<std::vector<std::vector<Tile>>>::size_type>(robotL.row)) + 1][static_cast<std::vector<std::vector<std::vector<Tile>>>::size_type>(robotL.col)];
            direction_check('s', southern, &robotL, &run);
        }
        if  (robotL.col != 0){
            Tile* western = &ship[static_cast<std::vector<std::vector<std::vector<Tile>>>::size_type>(robotL.level)][static_cast<std::vector<std::vector<std::vector<Tile>>>::size_type>(robotL.row)][(static_cast<std::vector<std::vector<std::vector<Tile>>>::size_type>(robotL.col)) - 1];
            direction_check('w', western, &robotL, &run);
        }
        
        // ELEVATOR
        if (current->type == 'E') {
            // check levels 0 to level_num for elevators in same coord
            for (int current_level = 0; current_level < level_num; ++current_level) {
                Tile* current_elevator = &ship[static_cast<std::vector<std::vector<std::vector<Tile>>>::size_type>(current_level)][static_cast<std::vector<std::vector<std::vector<Tile>>>::size_type>(robotL.row)][static_cast<std::vector<std::vector<std::vector<Tile>>>::size_type>(robotL.col)];
                if (current_elevator->type == 'E' && !current_elevator->discovered) {
                    // approached from current level
                    current_elevator->loc = static_cast<char>('0' + robotL.level);
                    current_elevator->discovered = true;
                    Location newL(current_level, robotL.row, robotL.col);
                    search.push_back(newL);
                }
            }
        }
    }
}

void Map::backtrace_direction(char direction, Location* robot){
   //defaults to north
    int r = 0;
    int c = 0;
    if (direction == 'n'){
        r = -1;
    }
    if (direction == 'e'){
        c = 1;
    } else if (direction == 's'){
        r = 1;
    } else if (direction == 'w'){
        c = -1;
    }
    
    int temp_row = static_cast<int>(robot->row);
    int temp_col = static_cast<int>(robot->col);
    temp_row = temp_row + r;
    temp_col = temp_col + c;
    
    int new_row = static_cast<int>(temp_row);
    int new_col = static_cast<int>(temp_col);
    
    Location newL(robot->level, new_row, new_col);
    search.push_back(newL);
    
    //change type to opposite of loc to then be able to print out
    if (direction == 'n'){
        ship.at(static_cast<size_t>(newL.level)).at(static_cast<size_t>(newL.row)).at(static_cast<size_t>(newL.col)).type = 's';
    } else if (direction == 'e') {
        ship.at(static_cast<size_t>(newL.level)).at(static_cast<size_t>(newL.row)).at(static_cast<size_t>(newL.col)).type = 'w';
    } else if (direction == 's') {
        ship.at(static_cast<size_t>(newL.level)).at(static_cast<size_t>(newL.row)).at(static_cast<size_t>(newL.col)).type = 'n';
    } else if (direction == 'w') {
        ship.at(static_cast<size_t>(newL.level)).at(static_cast<size_t>(newL.row)).at(static_cast<size_t>(newL.col)).type = 'e';
    }
    *robot = newL;
}

void Map::backtrace() {
    if (!path){
        return;
    }
    // clear the search container
    search.clear();
    
    Location robotL = endL;
    while (robotL != startL){
        
        //char ctype = current.type;
        char cloc = ship[static_cast<unsigned>(robotL.level)][static_cast<unsigned>(robotL.row)][static_cast<unsigned>(robotL.col)].loc;

        // came from a direction
        if (cloc == 'n' || cloc == 'e' || cloc == 's' || cloc == 'w') {
            backtrace_direction(cloc, &robotL);
        } else {
            int next_level = ship[static_cast<unsigned>(robotL.level)][static_cast<unsigned>(robotL.row)][static_cast<unsigned>(robotL.col)].loc - '0';
            Location newL(next_level, robotL.row, robotL.col);
            search.push_back(newL);
            ship[static_cast<unsigned>(next_level)][static_cast<unsigned>(newL.row)][static_cast<unsigned>(newL.col)].type = static_cast<char>('0' + robotL.level);
            robotL = newL;
            continue;
        }
    }
}

void Map::validPathOutput(){
    
    //if omode is list:
    if (omode == 'L') {
        // print each Tile
        cout << "//path taken\n";
        while (!search.empty()) {
            Location currentL = search.back();
            Tile currentS = ship[static_cast<vector<vector<vector<Tile>>>::size_type>(currentL.level)][static_cast<vector<vector<Tile>>::size_type>(currentL.row)][static_cast<vector<Tile>::size_type>(currentL.col)];
            cout << '(' << currentL.level << ',' << currentL.row << ',' << currentL.col << ',' << currentS.type << ')' << "\n";
            
            // remove tile from search
            search.pop_back();
        }
    }
    
    //defaults to map omode
    else {
        // print each Tile
        cout << "Start in level " << startL.level << ", row " << startL.row << ", column " << startL.col << "\n";
        for (int level = 0; level < level_num; ++level) {
            // print level_num numbers
            cout << "//level " << level << "\n";
            for (int row = 0; row < size; ++row) {
                for (int col = 0; col < size; ++col) {
                    cout << static_cast<char>(ship[static_cast<unsigned>(level)][static_cast<unsigned>(row)][static_cast<unsigned>(col)].type);
                }
                cout << "\n";
            }
        }
    }
}

void Map::noPathOutput(){
    //if omode is list:
    if (omode == 'L') {
            cout << "//path taken";
    } else {
        // print all tiles
        for (int level = 0; level < level_num; ++level) {
            // print level numbers
            cout << "//level " << level << endl;
            for (int row = 0; row < size; ++row) {
                for (int col = 0; col < size; ++col) {
                    cout << ship[static_cast<size_t>(level)][static_cast<size_t>(row)][static_cast<size_t>(col)].type;
                }
                if (level != level_num - 1 || row != size - 1) {
                    cout << "\n";
                }
            }
        }
    }
}

void Map::output(){
    if (path){
        validPathOutput();
    } else{
        noPathOutput();
    }
}

int main(int argc,char * argv[]) {
    ios_base::sync_with_stdio(false);
    Map Map_game;
    Map_game.mode(argc, argv);
    Map_game.input();
    Map_game.path_search();
    Map_game.backtrace();
    Map_game.output();
    return 0;
}