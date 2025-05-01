#include <iostream>
#include <fstream>
#include <cstdlib>
#include <chrono>
#include <ctime>
#include <SFML/Graphics.hpp>
#include <SFML/Window/Mouse.hpp>
#include <iomanip>

struct Tile {
    bool isRevealed = false;
    bool isMine     = false;
    bool isFlag     = false;
    int  adjacency  = 0;
};

void setText(sf::Text& text, float x, float y);
void initializeBoard(std::vector<std::vector<Tile>>& grid, int rowCount, int colCount, int numMines);
void clearBoard(std::vector<std::vector<Tile>>& grid, int colCount, int rowCount);
bool inBounds(int x, int y, int rowCount, int colCount);
void findAdjacentTiles(std::vector<std::vector<Tile>>& grid, int rowCount, int colCount, int x, int y, int& flagCount, int& remainingSpaces);


// This is a large int main, sorry in advance. When I was learning how to program formally before this class, we were not really
// taught not to use the int main function heavily, so I completely forgot when writing this
int main() {

    std::ifstream claw("files/config.cfg");
    int rowCount, colCount, numMines;
    claw >> colCount; claw.ignore();
    claw >> rowCount; claw.ignore();
    claw >> numMines;
    claw.close();

    if (numMines > rowCount * colCount) {
        std::cout << "Too many mines! Return code is number to reduce mineCount by.\n";
        return (numMines - rowCount * colCount);
    }

    int width  = colCount * 32;
    int height = (rowCount * 32) + 100;


    // Welcome window
    sf::RenderWindow welcomeWindow(sf::VideoMode(width+20, height+20), "Minesweeper", sf::Style::Close);
    sf::Font        font;
    if (!font.loadFromFile("files/font.ttf")) {
        return -1;
    }

    sf::Text welcome;
    welcome.setFont(font);
    welcome.setString("WELCOME TO MINESWEEPER!");
    welcome.setCharacterSize(24);
    setText(welcome, width / 2, (height / 2) - 150);
    welcome.setFillColor(sf::Color::White);
    welcome.setStyle(sf::Text::Bold | sf::Text::Underlined);

    sf::Text enterName;
    enterName.setFont(font);
    enterName.setString("Enter your name:");
    enterName.setCharacterSize(20);
    setText(enterName, width / 2, (height / 2) - 75);
    enterName.setFillColor(sf::Color::White);

    std::string userInput;
    sf::Text username;
    username.setFont(font);
    username.setCharacterSize(18);
    username.setFillColor(sf::Color::Yellow);

    sf::Text stick;
    stick.setFont(font);
    stick.setString('|');
    stick.setCharacterSize(18);
    stick.setFillColor(sf::Color::Yellow);
    setText(stick, width / 2, (height / 2) - 45);


    while (welcomeWindow.isOpen()) {
        sf::Event event;
        while (welcomeWindow.pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed:
                    welcomeWindow.close();
                    return 1;

                case sf::Event::TextEntered:
                    if (event.text.unicode < 128) {
                        bool backspace = sf::Keyboard::isKeyPressed(sf::Keyboard::BackSpace);
                        bool enter = sf::Keyboard::isKeyPressed(sf::Keyboard::Enter);

                        if (backspace && !userInput.empty()) {
                            userInput.pop_back();
                        }
                        else if (!enter && userInput.length() < 10 && isalpha(event.text.unicode)) {
                            userInput += static_cast<char>(event.text.unicode);
                        }

                        for (int i = 0; i < (int)userInput.length(); i++) {
                            if (i == 0) userInput[i] = toupper(userInput[i]);
                            else userInput[i] = tolower(userInput[i]);
                        }

                        username.setString(userInput);
                        setText(username, width / 2, (height / 2) - 45);
                        setText(stick,(width / 2) + (userInput.length() * 12) / 2,(height / 2) - 45);

                        if (enter && !userInput.empty()) {
                            welcomeWindow.close();
                        }
                    }
                    break;

                default: break;
            }
        }

        welcomeWindow.clear(sf::Color::Blue);
        welcomeWindow.draw(welcome);
        welcomeWindow.draw(enterName);
        welcomeWindow.draw(username);
        welcomeWindow.draw(stick);
        welcomeWindow.display();
    }


    // MINESWEEPER START

    sf::RenderWindow minesweeperGame(sf::VideoMode(width, height), "Minesweeper", sf::Style::Close);
    std::vector<std::vector<Tile>> grid(colCount, std::vector<Tile>(rowCount));
    initializeBoard(grid, rowCount, colCount, numMines);

    sf::Texture hiddenTiles;   if (!hiddenTiles.loadFromFile("files/images/tile_hidden.png"))   { std::cout << "Error loading a texture.";      minesweeperGame.close();   return -1; }
    sf::Texture mines;         if (!mines.loadFromFile("files/images/mine.png"))                { std::cout << "Error loading a texture.";      minesweeperGame.close();   return -1; }
    sf::Texture flag;          if (!flag.loadFromFile("files/images/flag.png"))                 { std::cout << "Error loading a texture.";      minesweeperGame.close();   return -1; }
    sf::Texture tileRevealed;  if (!tileRevealed.loadFromFile("files/images/tile_revealed.png")){ std::cout << "Error loading a texture.";      minesweeperGame.close();   return -1; }
    sf::Texture face_happy;    if (!face_happy.loadFromFile("files/images/face_happy.png"))     { std::cout << "Error loading a texture.";      minesweeperGame.close();   return -1; }
    sf::Texture face_win;      if (!face_win.loadFromFile("files/images/face_win.png"))         { std::cout << "Error loading a texture.";      minesweeperGame.close();   return -1; }
    sf::Texture face_lose;     if (!face_lose.loadFromFile("files/images/face_lose.png"))       { std::cout << "Error loading a texture.";      minesweeperGame.close();   return -1; }
    sf::Texture debug;         if (!debug.loadFromFile("files/images/debug.png"))               { std::cout << "Error loading a texture.";      minesweeperGame.close();   return -1; }
    sf::Texture pause;         if (!pause.loadFromFile("files/images/pause.png"))               { std::cout << "Error loading a texture.";      minesweeperGame.close();   return -1; }
    sf::Texture play;          if (!play.loadFromFile("files/images/play.png"))                 { std::cout << "Error loading a texture.";      minesweeperGame.close();   return -1; }
    sf::Texture leaderboard;   if (!leaderboard.loadFromFile("files/images/leaderboard.png"))   { std::cout << "Error loading a texture.";      minesweeperGame.close();   return -1; }
    sf::Texture digits;        if (!digits.loadFromFile("files/images/digits.png"))             { std::cout << "Error loading a texture.";      minesweeperGame.close();   return -1; }

    sf::Texture number1;       if (!number1.loadFromFile("files/images/number_1.png"))          { std::cout << "Error loading the 1st number!"; minesweeperGame.close(); return -1; }
    sf::Texture number2;       if (!number2.loadFromFile("files/images/number_2.png"))          { std::cout << "Error loading the 2nd number!"; minesweeperGame.close(); return -2; }
    sf::Texture number3;       if (!number3.loadFromFile("files/images/number_3.png"))          { std::cout << "Error loading the 3rd number!"; minesweeperGame.close(); return -3; }
    sf::Texture number4;       if (!number4.loadFromFile("files/images/number_4.png"))          { std::cout << "Error loading the 4th number!"; minesweeperGame.close(); return -4; }
    sf::Texture number5;       if (!number5.loadFromFile("files/images/number_5.png"))          { std::cout << "Error loading the 5th number!"; minesweeperGame.close(); return -5; }
    sf::Texture number6;       if (!number6.loadFromFile("files/images/number_6.png"))          { std::cout << "Error loading the 6th number!"; minesweeperGame.close(); return -6; }
    sf::Texture number7;       if (!number7.loadFromFile("files/images/number_7.png"))          { std::cout << "Error loading the 7th number!"; minesweeperGame.close(); return -7; }
    sf::Texture number8;       if (!number8.loadFromFile("files/images/number_8.png"))          { std::cout << "Error loading the 8th number!"; minesweeperGame.close(); return -8; }

    auto game_start         = std::chrono::high_resolution_clock::now();
    auto pause_time         = std::chrono::high_resolution_clock::now();
    auto elapsed_paused_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - pause_time).count();
    int  mins, secs;

    int remainingSpaces = (colCount*rowCount - numMines);   // When remaining spaces == 0, you win the game.
    int  flagCount = numMines; // tracks visual representation of flags
    int opened = 0; // used to not allow the winning leaderboard to open again if you click a tile.
    int winMin, winSec; // when the user wins, it sets the display to these times.
    int insert = 5;
    bool game_win  = false,
         game_lose = false,
         isPaused  = false,
         leaderboardOpen = false;


    while (minesweeperGame.isOpen()) {

        auto game_timer = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - game_start);
        int total_time = game_timer.count();
        std::string str = std::to_string(flagCount);

        // pauses time for different events
        if (!isPaused && !game_win && !game_lose) {
            total_time -= elapsed_paused_time;
            mins = total_time / 60;
            secs = total_time % 60;
        }

        if(game_win){
            pause_time = std::chrono::high_resolution_clock::now();
        }

        sf::Event event;
        while (minesweeperGame.pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed:
                    minesweeperGame.close();
                    break;

                case sf::Event::MouseButtonPressed: {
                    int mouseX = event.mouseButton.x;
                    int mouseY = event.mouseButton.y;
                    int col = mouseX / 32;
                    int row = mouseY / 32;

                    // toggle flags
                    if (event.mouseButton.button == sf::Mouse::Right) {
                        if (col >= 0 && col < colCount && row >= 0 && row < rowCount && !game_lose && !isPaused && !game_win) {
                            if (!grid[col][row].isFlag && !grid[col][row].isRevealed) {
                                grid[col][row].isFlag = true;
                                flagCount--;
                            } else if (grid[col][row].isFlag) {
                                grid[col][row].isFlag = false;
                                flagCount++;
                            }
                        }
                    }
                        // All left-click events
                    else if (event.mouseButton.button == sf::Mouse::Left) {
                        // debug button
                        if (mouseX >= (colCount * 32) - 304 && mouseX <= (colCount * 32) - 244 && mouseY >= 32 * (rowCount + 0.5) && mouseY <= (32 * (rowCount + 0.5) + 60) && !game_win && !game_lose) {
                            for (int i = 0; i < colCount; i++)
                                for (int j = 0; j < rowCount; j++)
                                    if (grid[i][j].isMine && !grid[i][j].isRevealed)
                                        grid[i][j].isRevealed = true;
                                    else if (grid[i][j].isMine && grid[i][j].isRevealed && !game_lose)
                                        grid[i][j].isRevealed = false;
                        }

                        // smiley face (reset board state && all values)
                        if (mouseX >= ((colCount / 2.0) * 32) - 32 &&mouseX <= ((colCount / 2.0) * 32) + 28 &&mouseY >= 32 * (rowCount + 0.5) &&mouseY <= (32 * (rowCount + 0.5) + 60)){
                            clearBoard(grid, colCount, rowCount);
                            initializeBoard(grid, rowCount, colCount, numMines);
                            game_start = std::chrono::high_resolution_clock::now();
                            pause_time = std::chrono::high_resolution_clock::now();
                            elapsed_paused_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - pause_time).count();
                            game_lose = false;
                            game_win = false;
                            isPaused = false;
                            opened = 0;
                            flagCount = numMines;
                            remainingSpaces = (colCount*rowCount - numMines);
                        }

                        // pause/play toggle
                        if (mouseX >= (colCount * 32) - 240 && mouseX <= (colCount * 32) - 180 && mouseY >= 32 * (rowCount + 0.5) && mouseY <= (32 * (rowCount + 0.5) + 60) && !game_win && !game_lose) {
                            if (!isPaused) {
                                isPaused = true;
                                pause_time = std::chrono::high_resolution_clock::now();

                            }
                            else {
                                if (!game_lose) {
                                    isPaused = false;
                                    auto unpause_time = std::chrono::high_resolution_clock::now();
                                    elapsed_paused_time += std::chrono::duration_cast<std::chrono::seconds>(
                                            unpause_time - pause_time).count();
                                }
                            }
                        }

                        // leaderboard button (NOT WHEN YOU WIN)
                        if (mouseX >= (colCount * 32) - 176 && mouseX <= (colCount * 32) - 116 && mouseY >= 32 * (rowCount + 0.5) && mouseY <= (32 * (rowCount + 0.5) + 60)) {
                            for (int i = 0; i < colCount; i++) {
                                for (int j = 0; j < rowCount; j++) {
                                    sf::Sprite board(tileRevealed);
                                    board.setPosition(i*32, j*32);
                                    minesweeperGame.draw(board);
                                }
                            }
                            minesweeperGame.display();

                            if (!game_lose) {
                                pause_time = std::chrono::high_resolution_clock::now();
                            }
                            sf::RenderWindow Leaderboard( sf::VideoMode(colCount * 16, (rowCount * 16) + 50),"Minesweeper", sf::Style::Close);

                            sf::Text lb;
                            lb.setFont(font);
                            lb.setString("LEADERBOARD");
                            lb.setCharacterSize(20);
                            setText(lb, ((colCount * 16) / 2), (((rowCount * 16) + 50) / 2) - 120);
                            lb.setFillColor(sf::Color::White);
                            lb.setStyle(sf::Text::Bold | sf::Text::Underlined);

                            std::ifstream leaderboardFile("files/leaderboard.txt");

                            // loads file into vector
                            std::vector<std::pair<int, std::string>> entries;
                            int minutes, seconds;
                            while (entries.size() < 5 && (leaderboardFile >> minutes)) {
                                leaderboardFile.ignore(1, ':');
                                leaderboardFile >> seconds;
                                leaderboardFile.ignore(1, ',');
                                std::string name;
                                std::getline(leaderboardFile, name);
                                entries.emplace_back(minutes * 60 + seconds, name);
                            }

                            std::string names;
                            std::ifstream file("files/leaderboard.txt");
                            if(!file.is_open())
                                std::cout << "Error reading the file" << std::endl;

                            // loads into a string so you can print
                            for(int i = 1; i <= entries.size(); i++) {
                                std::string temp;
                                std::getline(file, temp);
                                int n = temp.find(',');

                                // time and name parts
                                std::string time = temp.substr(0, n);
                                std::string name = temp.substr(n + 2);

                                if (game_win && i == insert + 1) {
                                    name += "*";
                                }

                                names += std::to_string(i) + ".\t" + time + "\t" + name + "\n\n";
                            }
                            sf::Text text2;
                            text2.setFont(font);
                            text2.setCharacterSize(18);
                            text2.setString(names);
                            text2.setStyle(sf::Text::Bold);
                            setText(text2, (colCount * 16)/2, (rowCount * 16)/2 + 50);
                            text2.setFillColor(sf::Color::White);

                            while (Leaderboard.isOpen()) {

                                mins = total_time / 60;
                                secs = total_time % 60;
                                sf::Event lbEvent;
                                while (Leaderboard.pollEvent(lbEvent))
                                    if (lbEvent.type == sf::Event::Closed) {
                                        if (!game_lose) {
                                            leaderboardOpen = false;
                                            auto unpause_time = std::chrono::high_resolution_clock::now();
                                            elapsed_paused_time += std::chrono::duration_cast<std::chrono::seconds>(unpause_time - pause_time).count();
                                        }
                                        Leaderboard.close();
                                    }

                                Leaderboard.clear(sf::Color::Blue);
                                Leaderboard.draw(lb);
                                Leaderboard.draw(text2);
                                Leaderboard.display();
                            }
                        }

                        // reveal tile
                        if (col >= 0 && col < colCount && row >= 0 && row < rowCount && !grid[col][row].isRevealed && !game_lose && !isPaused && !grid[col][row].isFlag &&!game_win) {
                            grid[col][row].isRevealed = true;
                            --remainingSpaces;
                            if (remainingSpaces == 0){  // when you win the game
                                    for (int i = 0; i < colCount; i++)
                                        for (int j = 0; j < rowCount; j++)
                                            if (grid[i][j].isMine) {
                                                grid[i][j].isRevealed = false;
                                                grid[i][j].isFlag = true;
                                            }
                                    flagCount = 0;
                                    game_win = true;
                            }
                            if (grid[col][row].adjacency == 0) { // calls recursive function
                                findAdjacentTiles(grid, rowCount, colCount, col, row, flagCount, remainingSpaces);
                                if (remainingSpaces == 0) { // when you win the game
                                    for (int i = 0; i < colCount; i++)
                                        for (int j = 0; j < rowCount; j++)
                                            if (grid[i][j].isMine) {
                                                grid[i][j].isRevealed = false;
                                                grid[i][j].isFlag = true;
                                            }
                                    flagCount = 0;
                                    game_win = true;
                                }
                            }
                            if (grid[col][row].isMine) {
                                for (int i = 0; i < colCount; i++)
                                    for (int j = 0; j < rowCount; j++)
                                        if (grid[i][j].isMine) {
                                            grid[i][j].isRevealed = true;
                                            game_lose = true;
                                        }
                                pause_time = std::chrono::high_resolution_clock::now();
                            }
                        }

                        break;
                    }
                }
                default: break;
            }
        }
        minesweeperGame.clear(sf::Color::White);

        // prints the board
        for (int i = 0; i < colCount; i++) {
            for (int j = 0; j < rowCount; j++) {
                sf::Sprite board;
                board.setPosition(i * 32, j * 32);

                if (isPaused){
                    board.setTexture(tileRevealed);
                    minesweeperGame.draw(board);
                }
                else if (grid[i][j].isRevealed && !isPaused) {
                    board.setTexture(tileRevealed);
                    minesweeperGame.draw(board);

                    if (grid[i][j].isMine) {
                        board.setTexture(mines);
                    } else {
                        switch (grid[i][j].adjacency) {
                            case 1: board.setTexture(number1); break;
                            case 2: board.setTexture(number2); break;
                            case 3: board.setTexture(number3); break;
                            case 4: board.setTexture(number4); break;
                            case 5: board.setTexture(number5); break;
                            case 6: board.setTexture(number6); break;
                            case 7: board.setTexture(number7); break;
                            case 8: board.setTexture(number8); break;
                            default: break;
                        }
                    }
                } else {
                    board.setTexture(hiddenTiles);
                    minesweeperGame.draw(board);

                    if (grid[i][j].isFlag) {
                        board.setTexture(flag);
                    }
                }


                // next 100 lines or so below display icons

                if (!game_lose && !game_win) {
                    sf::Sprite happy(face_happy);
                    happy.setPosition(((colCount / 2.0) * 32) - 32, 32 * (rowCount + 0.5));
                    minesweeperGame.draw(happy);
                }
                else if (game_win){
                    sf::Sprite win(face_win);
                    win.setPosition(((colCount / 2.0) * 32) - 32, 32 * (rowCount + 0.5));
                    minesweeperGame.draw(win);
                }
                else if (game_lose){
                    sf::Sprite lose(face_lose);
                    lose.setPosition(((colCount / 2.0) * 32) - 32, 32 * (rowCount + 0.5));
                    minesweeperGame.draw(lose);
                }

                sf::Sprite debugButton(debug);
                debugButton.setPosition((colCount * 32) - 304, 32 * (rowCount + 0.5));
                minesweeperGame.draw(debugButton);

                sf::Sprite pauseButton(pause);
                pauseButton.setPosition((colCount * 32) - 240, 32 * (rowCount + 0.5));
                minesweeperGame.draw(pauseButton);

                if ((isPaused && !game_lose) or game_win) {
                    sf::Sprite playButton(play);
                    playButton.setPosition((colCount * 32) - 240, 32 * (rowCount + 0.5));
                    minesweeperGame.draw(playButton);
                }

                sf::Sprite leaderboardButton(leaderboard);
                leaderboardButton.setPosition((colCount * 32) - 176, 32 * (rowCount + 0.5));
                minesweeperGame.draw(leaderboardButton);


                // These mins and secs statements display the clock
                if (mins < 10) {

                    sf::Sprite timeStamp(digits);
                    timeStamp.setTextureRect(sf::IntRect(0 * 21, 0, 21, 32));
                    timeStamp.setPosition((colCount * 32) - 97, 32 * (rowCount + 0.5) + 16);
                    minesweeperGame.draw(timeStamp);

                    if (game_win) {
                        timeStamp.setTextureRect(sf::IntRect(winMin * 21, 0, 21, 32));
                        timeStamp.setPosition((colCount * 32) - 76, 32 * (rowCount + 0.5) + 16);
                        minesweeperGame.draw(timeStamp);
                    }
                    else{
                        timeStamp.setTextureRect(sf::IntRect(mins * 21, 0, 21, 32));
                        timeStamp.setPosition((colCount * 32) - 76, 32 * (rowCount + 0.5) + 16);
                        minesweeperGame.draw(timeStamp);
                    }
                }
                else if (mins >= 10) {

                    if (game_win) {

                        sf::Sprite timeStamp(digits);
                        timeStamp.setTextureRect(sf::IntRect((winMin / 10) * 21, 0, 21, 32));
                        timeStamp.setPosition((colCount * 32) - 97, 32 * (rowCount + 0.5) + 16);
                        minesweeperGame.draw(timeStamp);

                        timeStamp.setTextureRect(sf::IntRect((winMin % 10) * 21, 0, 21, 32));
                        timeStamp.setPosition((colCount * 32) - 76, 32 * (rowCount + 0.5) + 16);
                        minesweeperGame.draw(timeStamp);
                    }

                    else{
                        sf::Sprite timeStamp(digits);
                        timeStamp.setTextureRect(sf::IntRect((mins / 10) * 21, 0, 21, 32));
                        timeStamp.setPosition((colCount * 32) - 97, 32 * (rowCount + 0.5) + 16);
                        minesweeperGame.draw(timeStamp);

                        timeStamp.setTextureRect(sf::IntRect((mins % 10) * 21, 0, 21, 32));
                        timeStamp.setPosition((colCount * 32) - 76, 32 * (rowCount + 0.5) + 16);
                        minesweeperGame.draw(timeStamp);
                    }
                }

                if (secs < 10) {
                    sf::Sprite timeStamp(digits);
                    timeStamp.setTextureRect(sf::IntRect(0 * 21, 0, 21, 32));
                    timeStamp.setPosition((colCount * 32) - 54, 32 * (rowCount + 0.5) + 16);
                    minesweeperGame.draw(timeStamp);

                    if (game_win) {
                        timeStamp.setTextureRect(sf::IntRect(winSec * 21, 0, 21, 32));
                        timeStamp.setPosition((colCount * 32) - 33, 32 * (rowCount + 0.5) + 16);
                        minesweeperGame.draw(timeStamp);
                    }else{
                        timeStamp.setTextureRect(sf::IntRect(secs * 21, 0, 21, 32));
                        timeStamp.setPosition((colCount * 32) - 33, 32 * (rowCount + 0.5) + 16);
                        minesweeperGame.draw(timeStamp);
                }}
                else if (secs >= 10) {
                    if(game_win) {
                        sf::Sprite timeStamp(digits);
                        timeStamp.setTextureRect(sf::IntRect((winSec / 10) * 21, 0, 21, 32));
                        timeStamp.setPosition((colCount * 32) - 54, 32 * (rowCount + 0.5) + 16);
                        minesweeperGame.draw(timeStamp);

                        timeStamp.setTextureRect(sf::IntRect((winSec % 10) * 21, 0, 21, 32));
                        timeStamp.setPosition((colCount * 32) - 33, 32 * (rowCount + 0.5) + 16);
                        minesweeperGame.draw(timeStamp);
                    }
                    else{
                        sf::Sprite timeStamp(digits);
                        timeStamp.setTextureRect(sf::IntRect((secs / 10) * 21, 0, 21, 32));
                        timeStamp.setPosition((colCount * 32) - 54, 32 * (rowCount + 0.5) + 16);
                        minesweeperGame.draw(timeStamp);

                        timeStamp.setTextureRect(sf::IntRect((secs % 10) * 21, 0, 21, 32));
                        timeStamp.setPosition((colCount * 32) - 33, 32 * (rowCount + 0.5) + 16);
                        minesweeperGame.draw(timeStamp);

                    }

                }

                // displays the mine count
                int xCoord = 33;
                double yCoord = 32 * (rowCount + 0.5) + 16;

                for (int i = 0; i < str.size(); i++) {
                    char temp = str[i];
                    int num;
                    if (temp == '-') num = 10;
                    else if (flagCount == 0) num = 0;
                    else num = temp - '0';

                    sf::Sprite mineCount(digits);
                    mineCount.setTextureRect(sf::IntRect(num * 21, 0, 21, 32));
                    xCoord += 21;
                    mineCount.setPosition(xCoord, yCoord);
                    minesweeperGame.draw(mineCount);
                    if (game_win and i == 0)
                        break;
                }

                minesweeperGame.draw(board);
            }
        }

        minesweeperGame.display();

        // What runs when you win the game (mainly leaderboard stuff).
        if (remainingSpaces == 0 && opened == 0){

            opened++;
            winMin = mins;
            winSec = secs;
            pause_time = std::chrono::high_resolution_clock::now();
            sf::RenderWindow Leaderboard(sf::VideoMode(colCount * 16, (rowCount * 16) + 50),"Minesweeper", sf::Style::Close);

            sf::Text lb;
            lb.setFont(font);
            lb.setString("LEADERBOARD");
            lb.setCharacterSize(20);
            setText(lb, ((colCount * 16) / 2), (((rowCount * 16) + 50) / 2) - 120);
            lb.setFillColor(sf::Color::White);
            lb.setStyle(sf::Text::Bold | sf::Text::Underlined);


            std::ifstream leaderboardFile("files/leaderboard.txt");

            std::vector<std::pair<int, std::string>> entries;
            int winTime = mins* 60 + secs;

            // adds your name if there's nothing in the leaderboard file
            if (leaderboardFile.peek() == std::char_traits<char>::eof()){
                entries.resize(1);
                entries[0].first = winTime;
                entries[0].second = userInput;

                std::ofstream out("files/leaderboard.txt");
                out << std::setw(2) << std::setfill('0') << mins << ':' << std::setw(2) << std::setfill('0') << secs << ", *" << entries[0].second << "\n";
            }

            // adds your name if there are already names in the file
            else {
                entries.resize(5);
                for (int i = 0; i < 5; i++) {
                    int minutes, seconds;
                    leaderboardFile >> minutes;
                    leaderboardFile.ignore(1, ':');
                    leaderboardFile >> seconds;
                    leaderboardFile.ignore(1, ',');
                    leaderboardFile.ignore();
                    std::getline(leaderboardFile, entries[i].second);
                    entries[i].first = minutes * 60 + seconds;
                }

                for (int i = 0; i < 5; i++) {
                    if (entries[i].second.empty() || winTime < entries[i].first) {
                        insert = i;
                        break;
                    }


                }
                if (insert < 5) {
                    for (int j = 4; j > insert; j--) {
                        entries[j] = entries[j - 1];
                    }
                    entries[insert].first = winTime;
                    entries[insert].second = userInput;
                }

                std::ofstream out("files/leaderboard.txt");

                for (auto & entry : entries) {
                    if (entry.second.empty()) break;
                    int minutes = entry.first / 60;
                    int seconds = entry.first % 60;
                    out << std::setw(2) << std::setfill('0') << minutes << ':' << std::setw(2) << std::setfill('0') << seconds << ", " << entry.second << "\n";
                }
            }


            // prints names to leaderboard below
            std::string names;
            std::ifstream file("files/leaderboard.txt");
            if(!file.is_open())
                std::cout << "Error reading the file" << std::endl;


            for(int i = 1; i <= entries.size(); i++) {
                std::string temp;
                std::getline(file, temp);
                int n = temp.find(',');

                std::string time = temp.substr(0, n);
                std::string name = temp.substr(n + 2);

                if (game_win && i == insert + 1) {
                    name += "*";
                }
                names += std::to_string(i) + ".\t" + time + "\t" + name + "\n\n";
            }
            sf::Text text2;
            text2.setFont(font);
            text2.setCharacterSize(18);
            text2.setString(names);
            text2.setStyle(sf::Text::Bold);
            setText(text2, (colCount * 16)/2, (rowCount * 16)/2 + 50);
            text2.setFillColor(sf::Color::White);

            while (Leaderboard.isOpen()) {
                sf::Event lbEvent;
                while (Leaderboard.pollEvent(lbEvent))
                    if (lbEvent.type == sf::Event::Closed) {
                        game_win = true;
                        Leaderboard.close();
                    }
                Leaderboard.clear(sf::Color::Blue);
                Leaderboard.draw(lb);
                Leaderboard.draw(text2);
                Leaderboard.display();
            }
        }
    }

    return 0;
}

void setText(sf::Text &text, float x, float y) {
    sf::FloatRect textRect = text.getLocalBounds();
    text.setOrigin(textRect.left + textRect.width / 2.0f,
                   textRect.top + textRect.height / 2.0f);
    text.setPosition(sf::Vector2f(x, y));
}


void clearBoard(std::vector<std::vector<Tile>>& grid, int colCount, int rowCount) {

    for (int i = 0; i < colCount; i++) {
        for (int j = 0; j < rowCount; j++) {

            grid[i][j].adjacency = 0;
            grid[i][j].isMine = false;
            grid[i][j].isRevealed = false;
            grid[i][j].isFlag = false;

        }
    }

}

void initializeBoard(std::vector<std::vector<Tile>>& grid, int rowCount, int colCount, int numMines) {

    // randomizes mines
    int mineCount = 0;
    srand(time(NULL));

    while (mineCount < numMines) {
        int x = rand() % colCount;
        int y = rand() % rowCount;

        if (!grid[x][y].isMine) {
            grid[x][y].isMine = true;
            mineCount++;
        }
    }
// finds adjacencies
    for (int i = 0; i < colCount; i++){
        for (int j = 0; j < rowCount; j++){

            if (inBounds(i-1, j-1, rowCount, colCount)) {
                if(grid[i-1][j-1].isMine)  grid[i][j].adjacency++;
            }
            if (inBounds(i-1, j, rowCount, colCount)) {
                if (grid[i-1][j].isMine)  grid[i][j].adjacency++;
            }
            if (inBounds(i-1, j+1, rowCount, colCount)) {
                if(grid[i-1][j+1].isMine)  grid[i][j].adjacency++;
            }
            if (inBounds(i, j-1, rowCount, colCount)) {
                if(grid[i][j-1].isMine)  grid[i][j].adjacency++;
            }
            if (inBounds(i, j+1,  rowCount, colCount)) {
                if(grid[i][j+1].isMine)  grid[i][j].adjacency++;
            }
            if (inBounds(i+1, j-1, rowCount, colCount)) {
                if(grid[i+1][j-1].isMine)  grid[i][j].adjacency++;
            }
            if (inBounds(i+1, j, rowCount, colCount)) {
                if(grid[i+1][j].isMine)  grid[i][j].adjacency++;
            }
            if (inBounds(i+1, j+1, rowCount, colCount)) {
                if(grid[i+1][j+1].isMine)  grid[i][j].adjacency++;
            }
        }
    }
}

bool inBounds(int x, int y, int rowCount, int colCount){

    if (x > colCount - 1 or x < 0) return false;
    else if (y > rowCount - 1 or y < 0) return false;
    else return true;
}
void findAdjacentTiles(std::vector<std::vector<Tile>>& grid, int rowCount, int colCount, int x, int y, int& flagCount, int& remainingSpaces) {

    // uses recursion to rid of empty tiles.
            if (!grid[x][y].isMine){

                if (inBounds(x-1, y-1, rowCount, colCount)) {
                    if(!grid[x - 1][y - 1].isRevealed && !grid[x-1][y-1].isFlag) {
                        {
                            grid[x - 1][y - 1].isRevealed = true;
                            --remainingSpaces;
                        }
                        if (grid[x-1][y-1].adjacency == 0 && !grid[x-1][y-1].isFlag)
                            findAdjacentTiles(grid, rowCount, colCount, x - 1, y - 1, flagCount, remainingSpaces);
                    }
                }
                if (inBounds(x-1, y, rowCount, colCount)) {
                    if (!grid[x-1][y].isRevealed&& !grid[x-1][y].isFlag) {
                        {
                            grid[x - 1][y].isRevealed = true;
                            --remainingSpaces;
                        }
                        if (grid[x - 1][y].adjacency == 0 && !grid[x-1][y].isFlag)
                            findAdjacentTiles(grid, rowCount, colCount, x - 1, y, flagCount, remainingSpaces);
                    }
                }
                if (inBounds(x-1, y+1, rowCount, colCount)) {
                    if(!grid[x-1][y+1].isRevealed&& !grid[x-1][y+1].isFlag) {
                        {
                            grid[x - 1][y + 1].isRevealed = true;
                            --remainingSpaces;
                        }
                        if(grid[x-1][y+1].adjacency == 0 && !grid[x-1][y+1].isFlag)
                            findAdjacentTiles(grid, rowCount, colCount, x - 1, y + 1, flagCount, remainingSpaces);
                    }
                }
                if (inBounds(x, y-1, rowCount, colCount)) {
                    if(!grid[x][y-1].isRevealed&& !grid[x][y-1].isFlag) {
                        {
                            grid[x][y - 1].isRevealed = true;
                            --remainingSpaces;
                        }
                        if(grid[x][y-1].adjacency == 0&& !grid[x][y-1].isFlag)
                            findAdjacentTiles(grid, rowCount, colCount, x, y - 1, flagCount, remainingSpaces);
                    }
                }
                if (inBounds(x, y+1,  rowCount, colCount)) {
                    if(!grid[x][y+1].isRevealed&& !grid[x][y+1].isFlag) {
                        {
                            grid[x][y + 1].isRevealed = true;
                            --remainingSpaces;
                        }
                        if(grid[x][y+1].adjacency == 0 && !grid[x][y+1].isFlag)
                            findAdjacentTiles(grid, rowCount, colCount, x, y + 1, flagCount, remainingSpaces);
                    }
                }
                if (inBounds(x+1, y-1, rowCount, colCount)) {
                    if(!grid[x+1][y-1].isRevealed && !grid[x+1][y-1].isFlag) {{
                        grid[x + 1][y - 1].isRevealed = true;
                        --remainingSpaces;
                    }
                        if(grid[x+1][y-1].adjacency == 0 && !grid[x+1][y-1].isFlag)
                            findAdjacentTiles(grid, rowCount, colCount, x + 1, y - 1, flagCount, remainingSpaces);
                    }
                }
                if (inBounds(x+1, y, rowCount, colCount)) {
                    if(!grid[x+1][y].isRevealed && !grid[x+1][y].isFlag) {
                        {
                            grid[x + 1][y].isRevealed = true;
                            --remainingSpaces;
                        }
                        if(grid[x+1][y].adjacency == 0 && !grid[x+1][y].isFlag)
                            findAdjacentTiles(grid, rowCount, colCount, x + 1, y, flagCount, remainingSpaces);
                    }
                }
                if (inBounds(x+1, y+1, rowCount, colCount)) {
                    if(!grid[x+1][y+1].isRevealed  && !grid[x+1][y+1].isFlag) {{
                        grid[x + 1][y + 1].isRevealed = true;
                        --remainingSpaces;
                    }
                        if (grid[x + 1][y + 1].adjacency == 0 && !grid[x+1][y+1].isFlag)
                            findAdjacentTiles(grid, rowCount, colCount, x + 1, y + 1, flagCount, remainingSpaces);
                    }
                }
            }

    }
