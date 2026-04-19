#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <memory>
#include <chrono>
#include <cstdint>

using namespace std;

// ==================== UTILITY FUNCTIONS ====================

class AnimationHelper {
public:
    static float easeOutCubic(float t) {
        return 1 - pow(1 - t, 3);
    }

    static float easeInOutQuad(float t) {
        return t < 0.5 ? 2 * t * t : 1 - pow(-2 * t + 2, 2) / 2;
    }
};

// ==================== DATA STRUCTURES ====================

struct Location {
    string name;
    float x, y;
    vector<pair<int, float>> edges;
};

class User {
public:
    string username;
    string password;
    string role;
    float rating;
    int totalRides;

    User() : rating(5.0f), totalRides(0) {}
    User(string u, string p, string r) : username(u), password(p), role(r), rating(5.0f), totalRides(0) {}
};

struct Driver {
    string username;
    float x, y;
    bool available;
    int locationIndex;
    float rating;
    int totalTrips;
    float earnings;

    Driver() : available(true), rating(5.0f), totalTrips(0), earnings(0.0f) {}
    Driver(string u, float px, float py, int loc) : username(u), x(px), y(py), available(true),
        locationIndex(loc), rating(5.0f), totalTrips(0), earnings(0.0f) {
    }
};

struct RideRequest {
    string riderName;
    int pickupLocation;
    int dropLocation;
    float fare;
    time_t timestamp;
    string status;

    RideRequest(string r, int p, int d, float f) : riderName(r), pickupLocation(p),
        dropLocation(d), fare(f), status("pending") {
        timestamp = time(nullptr);
    }
};

struct DriverDistance {
    Driver* driver;
    float distance;

    bool operator>(const DriverDistance& other) const {
        return distance > other.distance;
    }
};

struct Notification {
    string message;
    sf::Color color;
    float alpha;
    float lifetime;

    Notification(string msg, sf::Color col) : message(msg), color(col), alpha(255), lifetime(3.0f) {}
};

// ==================== GRAPH & PATHFINDING ====================

class IslamabadMap {
private:
    vector<Location> locations;

public:
    IslamabadMap() {
        initializeMap();
    }

    void initializeMap() {
        locations = {
            {"Blue Area", 400, 300},
            {"F-6 Markaz", 350, 400},
            {"F-7 Markaz", 450, 400},
            {"F-8 Markaz", 550, 400},
            {"Jinnah Super", 300, 350},
            {"Aabpara Market", 350, 250},
            {"G-9 Markaz", 500, 500},
            {"G-10 Markaz", 600, 500},
            {"Bahria Town", 700, 600},
            {"DHA Phase 2", 650, 350},
            {"Centaurus Mall", 420, 320},
            {"Faisal Mosque", 500, 200}
        };

        locations[0].edges = { {1, 120}, {2, 110}, {5, 80}, {10, 30} };
        locations[1].edges = { {0, 120}, {2, 100}, {4, 80} };
        locations[2].edges = { {0, 110}, {1, 100}, {3, 100}, {6, 120} };
        locations[3].edges = { {2, 100}, {7, 150}, {9, 120} };
        locations[4].edges = { {1, 80}, {5, 100} };
        locations[5].edges = { {0, 80}, {4, 100}, {11, 90} };
        locations[6].edges = { {2, 120}, {7, 100} };
        locations[7].edges = { {3, 150}, {6, 100}, {8, 150} };
        locations[8].edges = { {7, 150}, {9, 100} };
        locations[9].edges = { {3, 120}, {8, 100}, {10, 250} };
        locations[10].edges = { {0, 30}, {9, 250} };
        locations[11].edges = { {5, 90}, {0, 120} };
    }

    vector<int> dijkstra(int start, int end) {
        int n = locations.size();
        vector<float> dist(n, INFINITY);
        vector<int> prev(n, -1);
        vector<bool> visited(n, false);

        dist[start] = 0;
        priority_queue<pair<float, int>, vector<pair<float, int>>, greater<>> pq;
        pq.push({ 0, start });

        while (!pq.empty()) {
            int u = pq.top().second;
            pq.pop();

            if (visited[u]) continue;
            visited[u] = true;

            for (auto& edge : locations[u].edges) {
                int v = edge.first;
                float weight = edge.second;

                if (dist[u] + weight < dist[v]) {
                    dist[v] = dist[u] + weight;
                    prev[v] = u;
                    pq.push({ dist[v], v });
                }
            }
        }

        vector<int> path;
        for (int at = end; at != -1; at = prev[at]) {
            path.push_back(at);
        }
        reverse(path.begin(), path.end());
        return path;
    }

    float calculateDistance(int start, int end) {
        vector<int> path = dijkstra(start, end);
        float total = 0;
        for (size_t i = 0; i < path.size() - 1; i++) {
            for (auto& edge : locations[path[i]].edges) {
                if (edge.first == path[i + 1]) {
                    total += edge.second;
                    break;
                }
            }
        }
        return total;
    }

    vector<Location>& getLocations() { return locations; }
    Location& getLocation(int idx) { return locations[idx]; }
    int getLocationCount() { return locations.size(); }
};

// ==================== FILE HANDLING ====================

class FileManager {
public:
    static void saveUsers(const unordered_map<string, User>& users) {
        ofstream file("users.txt");
        for (const auto& pair : users) {
            const User& u = pair.second;
            file << u.username << "," << u.password << "," << u.role << ","
                << u.rating << "," << u.totalRides << "\n";
        }
        file.close();
    }

    static void loadUsers(unordered_map<string, User>& users) {
        ifstream file("users.txt");
        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string username, password, role;
            float rating;
            int totalRides;

            getline(ss, username, ',');
            getline(ss, password, ',');
            getline(ss, role, ',');
            ss >> rating;
            ss.ignore();
            ss >> totalRides;

            User u(username, password, role);
            u.rating = rating;
            u.totalRides = totalRides;
            users[username] = u;
        }
        file.close();
    }

    static void saveRideHistory(const RideRequest& ride, const string& driverName, float distance) {
        ofstream file("ride_history.txt", ios::app);
        file << ride.riderName << "," << driverName << ","
            << ride.pickupLocation << "," << ride.dropLocation << ","
            << distance << "," << ride.fare << "," << ride.timestamp << "\n";
        file.close();
    }

    static vector<string> loadRideHistory(const string& username) {
        vector<string> history;
        ifstream file("ride_history.txt");
        string line;

        while (getline(file, line)) {
            stringstream ss(line);
            string rider, driver;
            getline(ss, rider, ',');

            if (rider == username) {
                history.push_back(line);
            }
            else {
                getline(ss, driver, ',');
                if (driver == username) {
                    history.push_back(line);
                }
            }
        }
        file.close();
        return history;
    }
};

// ==================== MAIN APPLICATION ====================

class VelocityGoApp {
private:
    sf::RenderWindow window;
    IslamabadMap map;
    unordered_map<string, User> users;
    vector<Driver> drivers;

    enum Screen { SPLASH, LOGIN, SIGNUP, RIDER_DASHBOARD, DRIVER_DASHBOARD, BOOKING, TRACKING, RIDE_HISTORY };
    Screen currentScreen;

    User* currentUser;
    string inputUsername, inputPassword;
    bool inputtingUsername;

    int selectedPickup, selectedDrop;
    Driver* assignedDriver;
    vector<int> currentPath;

    sf::Font font;

    // Animation & UI state
    float screenTransition;
    bool transitioning;
    vector<Notification> notifications;
    sf::Clock clock;
    sf::Clock animClock;
    float hoverButton;
    int hoveredButtonIndex;

    // Ride animation
    float rideProgress;
    bool rideInProgress;
    sf::Vector2f carPosition;

    // Error message
    string errorMessage;
    float errorAlpha;

public:
    VelocityGoApp() : currentScreen(SPLASH), currentUser(nullptr),
        inputtingUsername(true), selectedPickup(-1), selectedDrop(-1),
        assignedDriver(nullptr), screenTransition(0), transitioning(false),
        hoverButton(0), hoveredButtonIndex(-1), rideProgress(0),
        rideInProgress(false), errorAlpha(0) {

        window.create(sf::VideoMode({ 1000, 750 }), "VelocityGo - Premium Ride Hailing");
        window.setFramerateLimit(60);

        if (!font.openFromFile("DejaVuSans.ttf")) {
            cout << "Font loading failed. Using default.\n";
        }

        FileManager::loadUsers(users);
        initializeDrivers();
    }

    void initializeDrivers() {
        drivers = {
            Driver("Ahmed Khan", 400, 300, 0),
            Driver("Sara Ali", 350, 400, 1),
            Driver("Bilal Sheikh", 550, 400, 3),
            Driver("Fatima Noor", 500, 500, 6),
            Driver("Hassan Malik", 700, 600, 8)
        };

        // Set realistic stats
        for (auto& d : drivers) {
            d.rating = 4.5f + (rand() % 10) / 10.0f;
            d.totalTrips = 100 + rand() % 500;
            d.earnings = d.totalTrips * 350.0f;
        }
    }

    void run() {
        while (window.isOpen()) {
            float deltaTime = clock.restart().asSeconds();
            handleEvents();
            update(deltaTime);
            render();
        }
    }

    void update(float dt) {
        // Update notifications
        for (auto it = notifications.begin(); it != notifications.end();) {
            it->lifetime -= dt;
            if (it->lifetime <= 0) {
                it = notifications.erase(it);
            }
            else {
                if (it->lifetime < 0.5f) {
                    it->alpha = 255 * (it->lifetime / 0.5f);
                }
                ++it;
            }
        }

        // Update error message fade
        if (errorAlpha > 0) {
            errorAlpha -= dt * 200;
            if (errorAlpha < 0) errorAlpha = 0;
        }

        // Update ride progress animation
        if (rideInProgress && currentScreen == TRACKING) {
            rideProgress += dt * 0.2f;
            if (rideProgress > 1.0f) rideProgress = 1.0f;

            // Animate car along path
            if (!currentPath.empty() && rideProgress < 1.0f) {
                float totalDist = 0;
                for (size_t i = 0; i < currentPath.size() - 1; i++) {
                    Location& l1 = map.getLocation(currentPath[i]);
                    Location& l2 = map.getLocation(currentPath[i + 1]);
                    totalDist += sqrt(pow(l2.x - l1.x, 2) + pow(l2.y - l1.y, 2));
                }

                float targetDist = totalDist * AnimationHelper::easeInOutQuad(rideProgress);
                float currentDist = 0;

                for (size_t i = 0; i < currentPath.size() - 1; i++) {
                    Location& l1 = map.getLocation(currentPath[i]);
                    Location& l2 = map.getLocation(currentPath[i + 1]);
                    float segmentDist = sqrt(pow(l2.x - l1.x, 2) + pow(l2.y - l1.y, 2));

                    if (currentDist + segmentDist >= targetDist) {
                        float t = (targetDist - currentDist) / segmentDist;
                        carPosition.x = l1.x + (l2.x - l1.x) * t;
                        carPosition.y = l1.y + (l2.y - l1.y) * t;
                        break;
                    }
                    currentDist += segmentDist;
                }
            }
        }

        // Splash screen transition
        if (currentScreen == SPLASH) {
            float elapsed = animClock.getElapsedTime().asSeconds();
            if (elapsed > 2.5f) {
                changeScreen(LOGIN);
            }
        }
    }

    void handleEvents() {
        while (true) {
            auto event = window.pollEvent();
            if (!event) break;

            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            if (const auto* textEvent = event->getIf<sf::Event::TextEntered>()) {
                handleTextInput(textEvent->unicode);
            }

            if (event->is<sf::Event::MouseButtonPressed>()) {
                handleMouseClick(sf::Mouse::getPosition(window));
            }

            // Track mouse movement for hover effects
            if (const auto* moveEvent = event->getIf<sf::Event::MouseMoved>()) {
                handleMouseMove({ moveEvent->position.x, moveEvent->position.y });
            }
        }
    }

    void handleMouseMove(sf::Vector2i pos) {
        hoveredButtonIndex = -1;

        if (currentScreen == LOGIN) {
            if (isButtonHovered(pos, 350, 450, 300, 55)) hoveredButtonIndex = 0;
            else if (isButtonHovered(pos, 350, 520, 300, 55)) hoveredButtonIndex = 1;
        }
        else if (currentScreen == SIGNUP) {
            if (isButtonHovered(pos, 300, 500, 150, 55)) hoveredButtonIndex = 0;
            else if (isButtonHovered(pos, 550, 500, 150, 55)) hoveredButtonIndex = 1;
            else if (isButtonHovered(pos, 350, 580, 300, 50)) hoveredButtonIndex = 2;
        }
        else if (currentScreen == RIDER_DASHBOARD) {
            if (isButtonHovered(pos, 300, 350, 400, 65)) hoveredButtonIndex = 0;
            else if (isButtonHovered(pos, 300, 435, 400, 65)) hoveredButtonIndex = 1;
            else if (isButtonHovered(pos, 300, 520, 400, 65)) hoveredButtonIndex = 2;
        }
    }

    void handleTextInput(char32_t unicode) {
        if (currentScreen == LOGIN || currentScreen == SIGNUP) {
            if (unicode == 8) { // Backspace
                if (inputtingUsername && !inputUsername.empty())
                    inputUsername.pop_back();
                else if (!inputtingUsername && !inputPassword.empty())
                    inputPassword.pop_back();
            }
            else if (unicode == 9 || unicode == 13) { // Tab or Enter
                inputtingUsername = !inputtingUsername;
            }
            else if (unicode < 128 && unicode >= 32) {
                if (inputtingUsername && inputUsername.length() < 20)
                    inputUsername += static_cast<char>(unicode);
                else if (!inputtingUsername && inputPassword.length() < 20)
                    inputPassword += static_cast<char>(unicode);
            }
        }
    }

    void handleMouseClick(sf::Vector2i pos) {
        if (currentScreen == LOGIN) {
            if (isButtonClicked(pos, 350, 450, 300, 55)) {
                if (authenticateUser()) {
                    if (currentUser->role == "rider")
                        changeScreen(RIDER_DASHBOARD);
                    else
                        changeScreen(DRIVER_DASHBOARD);
                    addNotification("Welcome back, " + currentUser->username + "!", sf::Color(100, 200, 100));
                }
                else {
                    showError("Invalid username or password!");
                }
            }
            else if (isButtonClicked(pos, 350, 520, 300, 55)) {
                changeScreen(SIGNUP);
                inputUsername = "";
                inputPassword = "";
                inputtingUsername = true;
            }
        }
        else if (currentScreen == SIGNUP) {
            if (isButtonClicked(pos, 300, 500, 150, 55)) {
                if (registerUser("rider")) {
                    addNotification("Account created successfully!", sf::Color(100, 200, 100));
                }
            }
            else if (isButtonClicked(pos, 550, 500, 150, 55)) {
                if (registerUser("driver")) {
                    addNotification("Driver account created!", sf::Color(100, 200, 100));
                }
            }
            else if (isButtonClicked(pos, 350, 580, 300, 50)) {
                changeScreen(LOGIN);
            }
        }
        else if (currentScreen == RIDER_DASHBOARD) {
            if (isButtonClicked(pos, 300, 350, 400, 65)) {
                changeScreen(BOOKING);
                selectedPickup = -1;
                selectedDrop = -1;
            }
            else if (isButtonClicked(pos, 300, 435, 400, 65)) {
                changeScreen(RIDE_HISTORY);
            }
            else if (isButtonClicked(pos, 300, 520, 400, 65)) {
                logout();
            }
        }
        else if (currentScreen == DRIVER_DASHBOARD) {
            if (isButtonClicked(pos, 350, 520, 300, 60)) {
                logout();
            }
        }
        else if (currentScreen == BOOKING) {
            handleMapClick(pos);
        }
        else if (currentScreen == TRACKING) {
            if (isButtonClicked(pos, 700, 650, 200, 60)) {
                completeRide();
            }
            else if (isButtonClicked(pos, 50, 650, 200, 60)) {
                cancelRide();
            }
        }
        else if (currentScreen == RIDE_HISTORY) {
            if (isButtonClicked(pos, 400, 650, 200, 60)) {
                changeScreen(RIDER_DASHBOARD);
            }
        }
    }

    void handleMapClick(sf::Vector2i pos) {
        for (size_t i = 0; i < map.getLocationCount(); i++) {
            Location& loc = map.getLocation(i);
            float dx = pos.x - loc.x;
            float dy = pos.y - loc.y;

            if (sqrt(dx * dx + dy * dy) < 20) {
                if (selectedPickup == -1) {
                    selectedPickup = i;
                    addNotification("Pickup: " + loc.name, sf::Color(100, 200, 100));
                }
                else if (selectedDrop == -1 && (int)i != selectedPickup) {
                    selectedDrop = i;
                    addNotification("Drop: " + loc.name, sf::Color(200, 100, 100));
                    findNearestDriver();
                }
                break;
            }
        }

        if (isButtonClicked(pos, 50, 650, 150, 50)) {
            changeScreen(RIDER_DASHBOARD);
        }
    }

    bool isButtonClicked(sf::Vector2i pos, int x, int y, int w, int h) {
        return pos.x >= x && pos.x <= x + w && pos.y >= y && pos.y <= y + h;
    }

    bool isButtonHovered(sf::Vector2i pos, int x, int y, int w, int h) {
        return pos.x >= x && pos.x <= x + w && pos.y >= y && pos.y <= y + h;
    }

    bool authenticateUser() {
        if (users.find(inputUsername) != users.end()) {
            if (users[inputUsername].password == inputPassword) {
                currentUser = &users[inputUsername];
                return true;
            }
        }
        return false;
    }

    bool registerUser(const string& role) {
        if (inputUsername.empty() || inputPassword.empty()) {
            showError("Please fill all fields!");
            return false;
        }

        if (users.find(inputUsername) != users.end()) {
            showError("Username already exists!");
            return false;
        }

        User newUser(inputUsername, inputPassword, role);
        users[inputUsername] = newUser;
        FileManager::saveUsers(users);
        changeScreen(LOGIN);
        inputUsername = "";
        inputPassword = "";
        return true;
    }

    void findNearestDriver() {
        priority_queue<DriverDistance, vector<DriverDistance>, greater<DriverDistance>> pq;
        Location& pickup = map.getLocation(selectedPickup);

        for (auto& driver : drivers) {
            if (driver.available) {
                float dx = driver.x - pickup.x;
                float dy = driver.y - pickup.y;
                float dist = sqrt(dx * dx + dy * dy);
                pq.push({ &driver, dist });
            }
        }

        if (!pq.empty()) {
            assignedDriver = pq.top().driver;
            assignedDriver->available = false;
            currentPath = map.dijkstra(selectedPickup, selectedDrop);
            carPosition = sf::Vector2f(assignedDriver->x, assignedDriver->y);
            rideProgress = 0;
            rideInProgress = true;
            changeScreen(TRACKING);
            addNotification("Driver " + assignedDriver->username + " assigned!", sf::Color(100, 150, 255));
        }
        else {
            showError("No drivers available!");
        }
    }

    void completeRide() {
        if (assignedDriver) {
            float distance = map.calculateDistance(selectedPickup, selectedDrop);
            float fare = distance * 0.5f + 50;

            RideRequest ride(currentUser->username, selectedPickup, selectedDrop, fare);
            FileManager::saveRideHistory(ride, assignedDriver->username, distance);

            currentUser->totalRides++;
            assignedDriver->available = true;
            assignedDriver->totalTrips++;
            assignedDriver->earnings += fare;

            FileManager::saveUsers(users);

            addNotification("Ride completed! Fare: Rs. " + to_string((int)fare), sf::Color(100, 200, 100));

            assignedDriver = nullptr;
            rideInProgress = false;
            changeScreen(RIDER_DASHBOARD);
        }
    }

    void cancelRide() {
        if (assignedDriver) {
            assignedDriver->available = true;
            assignedDriver = nullptr;
            rideInProgress = false;
            addNotification("Ride cancelled", sf::Color(255, 150, 100));
            changeScreen(BOOKING);
        }
    }

    void logout() {
        currentUser = nullptr;
        changeScreen(LOGIN);
        inputUsername = "";
        inputPassword = "";
        inputtingUsername = true;
        addNotification("Logged out successfully", sf::Color(150, 150, 150));
    }

    void changeScreen(Screen newScreen) {
        currentScreen = newScreen;
        if (newScreen == SPLASH) {
            animClock.restart();
        }
    }

    void addNotification(const string& msg, sf::Color color) {
        notifications.push_back(Notification(msg, color));
    }

    void showError(const string& msg) {
        errorMessage = msg;
        errorAlpha = 255;
    }

    void render() {
        window.clear(sf::Color(240, 242, 245));

        switch (currentScreen) {
        case SPLASH: renderSplash(); break;
        case LOGIN: renderLogin(); break;
        case SIGNUP: renderSignup(); break;
        case RIDER_DASHBOARD: renderRiderDashboard(); break;
        case DRIVER_DASHBOARD: renderDriverDashboard(); break;
        case BOOKING: renderBooking(); break;
        case TRACKING: renderTracking(); break;
        case RIDE_HISTORY: renderRideHistory(); break;
        }

        // Render notifications
        float yOffset = 20;
        for (auto& notif : notifications)
        {
            sf::Color col = notif.color;
            col.a = static_cast<std::uint8_t>(notif.alpha);  // FIX
            drawNotification(notif.message, 20, yOffset, col);
            yOffset += 60;
        }

        // Render error message
        if (errorAlpha > 0)
        {
            sf::Color errCol{ 255, 100, 100, static_cast<std::uint8_t>(errorAlpha) };
            drawNotification(
                errorMessage,
                window.getSize().x / 2.0f - 150.f,
                20.f,
                errCol
            );
        }

        window.display();
    }

    void renderSplash() {
        float elapsed = animClock.getElapsedTime().asSeconds();
        float alpha = 255;

        if (elapsed > 2.0f) {
            alpha = 255 * (1.0f - (elapsed - 2.0f) / 0.5f);
        }

        // Animated gradient background
        sf::RectangleShape bg(sf::Vector2f(1000.f, 750.f));
        bg.setFillColor(sf::Color{ 20, 30, 50, static_cast<std::uint8_t>(alpha) });
        window.draw(bg);

        // Logo with pulse effect
        float scale = 1.0f + std::sin(elapsed * 3.f) * 0.05f;

        drawTextCentered(
            "VelocityGo",
            500.f,
            300.f,
            static_cast<int>(72 * scale),
            sf::Color{ 100, 200, 255, static_cast<std::uint8_t>(alpha) }
        );

        drawTextCentered(
            "Premium Ride Hailing",
            500.f,
            380.f,
            24,
            sf::Color{ 200, 200, 200, static_cast<std::uint8_t>(alpha) }
        );

        // Loading animation
        float progress = (std::sin(elapsed * 4.f) + 1.f) / 2.f;

        drawProgressBar(
            350.f,
            450.f,
            300.f,
            8.f,
            progress,
            sf::Color{ 100, 200, 255, static_cast<std::uint8_t>(alpha) }
        );
    }

    void renderLogin() {
        // Modern gradient header
        drawGradientRect(0, 0, 1000, 200, sf::Color(50, 100, 200), sf::Color(100, 150, 255));

        drawTextCentered("VelocityGo", 500, 50, 56, sf::Color::White);
        drawTextCentered("Your Premium Ride Experience", 500, 120, 20, sf::Color(240, 240, 240));

        // Card-style container
        drawRoundedRect(300, 250, 400, 320, 15, sf::Color(255, 255, 255), sf::Color(220, 220, 220));

        drawText("Username", 330, 280, 18, sf::Color(80, 80, 80));
        drawInputField(330, 310, 340, 45, inputUsername + (inputtingUsername ? "|" : ""),
            inputtingUsername, sf::Color(100, 150, 255));

        drawText("Password", 330, 380, 18, sf::Color(80, 80, 80));
        string maskedPass(inputPassword.size(), '•');
        drawInputField(330, 410, 340, 45, maskedPass + (!inputtingUsername ? "|" : ""),
            !inputtingUsername, sf::Color(100, 150, 255));

        drawModernButton(350, 450, 300, 55, "Sign In", sf::Color(50, 150, 255), hoveredButtonIndex == 0);
        drawModernButton(350, 520, 300, 55, "Create Account", sf::Color(100, 200, 100), hoveredButtonIndex == 1);

        drawTextCentered("Fast • Safe • Reliable", 500, 640, 16, sf::Color(150, 150, 150));
    }

    void renderSignup() {
        drawGradientRect(0, 0, 1000, 200, sf::Color(100, 50, 200), sf::Color(150, 100, 255));

        drawTextCentered("Join VelocityGo", 500, 60, 48, sf::Color::White);
        drawTextCentered("Start your journey today", 500, 120, 18, sf::Color(240, 240, 240));

        drawRoundedRect(275, 220, 450, 380, 15, sf::Color(255, 255, 255), sf::Color(220, 220, 220));

        drawText("Create Username", 305, 250, 18, sf::Color(80, 80, 80));
        drawInputField(305, 280, 390, 45, inputUsername + (inputtingUsername ? "|" : ""),
            inputtingUsername, sf::Color(150, 100, 255));

        drawText("Create Password", 305, 350, 18, sf::Color(80, 80, 80));
        string maskedPass(inputPassword.size(), '*');
        drawInputField(305, 380, 390, 45, maskedPass + (!inputtingUsername ? "|" : ""),
            !inputtingUsername, sf::Color(150, 100, 255));

        drawText("Select Account Type:", 305, 450, 18, sf::Color(80, 80, 80));
        drawModernButton(300, 500, 150, 55, "Rider", sf::Color(100, 150, 255), hoveredButtonIndex == 0);
        drawModernButton(550, 500, 150, 55, "Driver", sf::Color(255, 150, 100), hoveredButtonIndex == 1);

        drawModernButton(350, 580, 300, 50, "Back to Login", sf::Color(150, 150, 150), hoveredButtonIndex == 2);
    }

    void renderRiderDashboard() {
        drawGradientRect(0, 0, 1000, 180, sf::Color(50, 150, 255), sf::Color(100, 200, 255));

        drawText("Welcome, " + currentUser->username + "!", 50, 40, 36, sf::Color::White);

        // Stats cards
        string ratingStr = to_string(currentUser->rating).substr(0, 3);
        drawStatCard(50, 100, 200, 60, "Rating", ratingStr + " *", sf::Color(255, 200, 100));
        drawStatCard(270, 100, 200, 60, "Total Rides", to_string(currentUser->totalRides), sf::Color(100, 200, 150));

        // Main action cards
        drawRoundedRect(250, 250, 500, 350, 15, sf::Color(255, 255, 255), sf::Color(220, 220, 220));

        drawTextCentered("Rider Dashboard", 500, 280, 32, sf::Color(50, 50, 50));

        drawActionCard(300, 350, 400, 65, "Book a Ride",
            "Find drivers near you", sf::Color(50, 150, 255), hoveredButtonIndex == 0);

        drawActionCard(300, 435, 400, 65, "Ride History",
            "View past trips", sf::Color(100, 200, 150), hoveredButtonIndex == 1);

        drawActionCard(300, 520, 400, 65, "Logout",
            "Sign out of account", sf::Color(255, 100, 100), hoveredButtonIndex == 2);
    }

    void renderDriverDashboard() {
        drawGradientRect(0, 0, 1000, 180, sf::Color(255, 150, 50), sf::Color(255, 200, 100));

        drawText("Driver Dashboard", 50, 40, 36, sf::Color::White);

        // Find current driver
        Driver* currentDriver = nullptr;
        for (auto& d : drivers) {
            if (d.username == currentUser->username) {
                currentDriver = &d;
                break;
            }
        }

        if (currentDriver) {
            drawStatCard(50, 100, 180, 60, "Rating", to_string(currentDriver->rating).substr(0, 3) + " *",
                sf::Color(255, 200, 100));
            drawStatCard(250, 100, 180, 60, "Total Trips", to_string(currentDriver->totalTrips),
                sf::Color(100, 200, 150));
            drawStatCard(450, 100, 180, 60, "Earnings", "Rs. " + to_string((int)currentDriver->earnings),
                sf::Color(100, 150, 255));
            drawStatCard(650, 100, 180, 60, "Status", currentDriver->available ? "Available" : "Busy",
                currentDriver->available ? sf::Color(100, 200, 100) : sf::Color(255, 150, 100));
        }

        drawRoundedRect(200, 250, 600, 300, 15, sf::Color(255, 255, 255), sf::Color(220, 220, 220));

        drawTextCentered("Driver Information", 500, 280, 28, sf::Color(50, 50, 50));

        if (currentDriver) {
            drawText("Location: " + map.getLocation(currentDriver->locationIndex).name, 250, 340, 20, sf::Color(80, 80, 80));
            drawText("Average fare per trip: Rs. " + to_string((int)(currentDriver->earnings / max(currentDriver->totalTrips, 1))),
                250, 380, 20, sf::Color(80, 80, 80));
            drawText("Account Type: Professional Driver", 250, 420, 20, sf::Color(80, 80, 80));

            // Status indicator
            sf::CircleShape statusDot(8);
            statusDot.setPosition(sf::Vector2f(250, 465));
            statusDot.setFillColor(currentDriver->available ? sf::Color(100, 200, 100) : sf::Color(255, 100, 100));
            window.draw(statusDot);
            drawText(currentDriver->available ? "Available for rides" : "Currently on a trip",
                270, 460, 20, sf::Color(80, 80, 80));
        }

        drawModernButton(350, 520, 300, 60, "Logout", sf::Color(255, 100, 100), false);
    }

    void renderBooking() {
        drawGradientRect(0, 0, 1000, 100, sf::Color(50, 150, 255), sf::Color(100, 200, 255));
        drawText("Book Your Ride", 50, 35, 32, sf::Color::White);

        // Instructions panel
        drawRoundedRect(750, 120, 230, 150, 10, sf::Color(255, 255, 255), sf::Color(220, 220, 220));
        drawText("How to book:", 770, 135, 18, sf::Color(50, 50, 50));
        drawText("1. Select pickup", 770, 165, 14, sf::Color(100, 100, 100));
        drawText("2. Select destination", 770, 190, 14, sf::Color(100, 100, 100));
        drawText("3. Confirm booking", 770, 215, 14, sf::Color(100, 100, 100));

        if (selectedPickup != -1) {
            drawText("Pickup: " + map.getLocation(selectedPickup).name, 770, 245, 14, sf::Color(50, 150, 255));
        }
        if (selectedDrop != -1) {
            drawText("Drop: " + map.getLocation(selectedDrop).name, 770, 265, 14, sf::Color(255, 100, 100));
        }

        // Draw map
        drawMap();

        drawModernButton(50, 650, 150, 50, "Back", sf::Color(150, 150, 150), false);
    }

    void renderTracking() {
        drawGradientRect(0, 0, 1000, 100, sf::Color(100, 200, 100), sf::Color(150, 255, 150));
        drawText("Ride in Progress", 50, 35, 32, sf::Color::White);

        // Driver info panel
        if (assignedDriver) {
            drawRoundedRect(750, 120, 230, 200, 10, sf::Color(255, 255, 255), sf::Color(220, 220, 220));
            drawText("Driver Details", 770, 135, 20, sf::Color(50, 50, 50));
            drawText(assignedDriver->username, 770, 170, 18, sf::Color(80, 80, 80));
            drawText("Rating: " + to_string(assignedDriver->rating).substr(0, 3) + " *", 770, 200, 16, sf::Color(255, 200, 100));
            drawText("Trips: " + to_string(assignedDriver->totalTrips), 770, 225, 14, sf::Color(100, 100, 100));

            float distance = map.calculateDistance(selectedPickup, selectedDrop);
            float fare = distance * 0.5f + 50;
            drawText("Fare: Rs. " + to_string((int)fare), 770, 260, 16, sf::Color(100, 150, 255));
            drawText("Distance: " + to_string((int)distance) + " m", 770, 285, 14, sf::Color(100, 100, 100));
        }

        // Progress bar
        drawRoundedRect(50, 620, 700, 15, 7, sf::Color(220, 220, 220), sf::Color(200, 200, 200));
        drawProgressBar(50, 620, 700, 15, rideProgress, sf::Color(100, 200, 100));

        string progressText = to_string((int)(rideProgress * 100)) + "% Complete";
        drawText(progressText, 350, 595, 16, sf::Color(80, 80, 80));

        // Draw map with animated car
        drawMapWithCar();

        drawModernButton(50, 650, 200, 60, "Cancel Ride", sf::Color(255, 100, 100), false);
        drawModernButton(700, 650, 200, 60, "Complete Ride", sf::Color(100, 200, 100), false);
    }

    void renderRideHistory() {
        drawGradientRect(0, 0, 1000, 100, sf::Color(100, 150, 255), sf::Color(150, 200, 255));
        drawText("Ride History", 50, 35, 32, sf::Color::White);

        vector<string> history = FileManager::loadRideHistory(currentUser->username);

        drawRoundedRect(50, 120, 900, 500, 15, sf::Color(255, 255, 255), sf::Color(220, 220, 220));

        if (history.empty()) {
            drawTextCentered("No ride history yet", 500, 350, 24, sf::Color(150, 150, 150));
            drawTextCentered("Book your first ride to get started!", 500, 390, 16, sf::Color(180, 180, 180));
        }
        else {
            drawText("Recent Trips", 80, 145, 24, sf::Color(50, 50, 50));

            int yPos = 190;
            int count = 0;
            int maxItems = min(6, (int)history.size());

            for (int i = (int)history.size() - 1; i >= 0 && count < maxItems; i--, count++) {
                stringstream ss(history[i]);
                string rider, driver, pickup, drop, dist, fare, timestamp;

                getline(ss, rider, ',');
                getline(ss, driver, ',');
                getline(ss, pickup, ',');
                getline(ss, drop, ',');
                getline(ss, dist, ',');
                getline(ss, fare, ',');
                getline(ss, timestamp, ',');

                int pickupIdx = stoi(pickup);
                int dropIdx = stoi(drop);

                drawRideHistoryCard(80, (float)yPos, 840, 65, rider, driver,
                    map.getLocation(pickupIdx).name,
                    map.getLocation(dropIdx).name,
                    stof(fare), count % 2 == 0);
                yPos += 75;
            }
        }

        drawModernButton(400, 650, 200, 60, "Dashboard", sf::Color(100, 150, 255), false);
    }

    // ==================== DRAWING HELPER FUNCTIONS ====================

    void drawMap() {
        // Draw edges
        for (int i = 0; i < map.getLocationCount(); i++) {
            Location& loc = map.getLocation(i);
            for (auto& edge : loc.edges) {
                Location& dest = map.getLocation(edge.first);
                sf::Vertex line[2];

                line[0].position = { loc.x, loc.y };
                line[0].color = sf::Color{ 200, 200, 200 };

                line[1].position = { dest.x, dest.y };
                line[1].color = sf::Color{ 200, 200, 200 };

                window.draw(line, 2, sf::PrimitiveType::Lines);

            }
        }

        // Draw path if selected
        if (!currentPath.empty()) {
            for (size_t i = 0; i < currentPath.size() - 1; i++) {
                Location& l1 = map.getLocation(currentPath[i]);
                Location& l2 = map.getLocation(currentPath[i + 1]);
                sf::Vertex line[2];

                line[0].position = { l1.x, l1.y };
                line[0].color = sf::Color{ 100, 200, 255, 200 };

                line[1].position = { l2.x, l2.y };
                line[1].color = sf::Color{ 100, 200, 255, 200 };

                window.draw(line, 2, sf::PrimitiveType::Lines);

            }
        }

        // Draw locations
        for (int i = 0; i < map.getLocationCount(); i++) {
            Location& loc = map.getLocation(i);

            sf::Color nodeColor = sf::Color(100, 100, 100);
            if (i == selectedPickup) nodeColor = sf::Color(100, 200, 100);
            else if (i == selectedDrop) nodeColor = sf::Color(255, 100, 100);

            sf::CircleShape node(15);
            node.setPosition(sf::Vector2f(loc.x - 15, loc.y - 15));
            node.setFillColor(nodeColor);
            node.setOutlineThickness(3);
            node.setOutlineColor(sf::Color::White);
            window.draw(node);

            drawText(loc.name, loc.x - 50, loc.y + 20, 12, sf::Color(50, 50, 50));
        }

        // Draw available drivers
        for (auto& driver : drivers) {
            if (driver.available) {
                sf::CircleShape car(8);
                car.setPosition(sf::Vector2f(driver.x - 8, driver.y - 8));
                car.setFillColor(sf::Color(255, 200, 100));
                car.setOutlineThickness(2);
                car.setOutlineColor(sf::Color::White);
                window.draw(car);
            }
        }
    }

    void drawMapWithCar() {
        drawMap();

        // Draw animated car
        if (rideInProgress) {
            sf::CircleShape car(12);
            car.setPosition(sf::Vector2f(carPosition.x - 12, carPosition.y - 12));
            car.setFillColor(sf::Color(255, 200, 50));
            car.setOutlineThickness(3);
            car.setOutlineColor(sf::Color::White);
            window.draw(car);

            // Pulsing effect
            float pulse = 1.0f + sin(animClock.getElapsedTime().asSeconds() * 5) * 0.2f;
            sf::CircleShape glow(12 * pulse);
            glow.setPosition(sf::Vector2f(carPosition.x - 12 * pulse, carPosition.y - 12 * pulse));
            glow.setFillColor(sf::Color(255, 200, 50, 50));
            window.draw(glow);
        }
    }

    void drawText(const string& text, float x, float y, int size, sf::Color color) {
        sf::Text t(font);
        t.setString(text);
        t.setCharacterSize(size);
        t.setFillColor(color);
        t.setPosition(sf::Vector2f(x, y));
        window.draw(t);
    }

    void drawTextCentered(const string& text, float x, float y, int size, sf::Color color) {
        sf::Text t(font);
        t.setString(text);
        t.setCharacterSize(size);
        t.setFillColor(color);

        sf::FloatRect bounds = t.getLocalBounds();
        t.setPosition(sf::Vector2f(x - bounds.size.x / 2, y - bounds.size.y / 2));
        window.draw(t);
    }

    void drawRoundedRect(float x, float y, float w, float h, float radius, sf::Color fill, sf::Color outline) {
        sf::RectangleShape rect(sf::Vector2f(w, h));
        rect.setPosition(sf::Vector2f(x, y));
        rect.setFillColor(fill);
        rect.setOutlineThickness(2);
        rect.setOutlineColor(outline);
        window.draw(rect);
    }

    void drawGradientRect(float x, float y, float w, float h, sf::Color top, sf::Color bottom) {
        sf::Vertex gradient[6];

        // Triangle 1
        gradient[0].position = { x, y };
        gradient[0].color = top;

        gradient[1].position = { x + w, y };
        gradient[1].color = top;

        gradient[2].position = { x + w, y + h };
        gradient[2].color = bottom;

        // Triangle 2
        gradient[3].position = { x + w, y + h };
        gradient[3].color = bottom;

        gradient[4].position = { x, y + h };
        gradient[4].color = bottom;

        gradient[5].position = { x, y };
        gradient[5].color = top;

        window.draw(gradient, 6, sf::PrimitiveType::Triangles);

    }

    void drawModernButton(float x, float y, float w, float h, const string& text, sf::Color color, bool hovered) {
        sf::Color btnColor = color;
        if (hovered) {
            btnColor.r = min(255, (int)(btnColor.r * 1.1f));
            btnColor.g = min(255, (int)(btnColor.g * 1.1f));
            btnColor.b = min(255, (int)(btnColor.b * 1.1f));
        }

        sf::RectangleShape btn(sf::Vector2f(w, h));
        btn.setPosition(sf::Vector2f(x, y));
        btn.setFillColor(btnColor);
        window.draw(btn);

        if (hovered) {
            sf::RectangleShape shadow(sf::Vector2f(w, h));
            shadow.setPosition(sf::Vector2f(x + 2, y + 2));
            shadow.setFillColor(sf::Color(0, 0, 0, 30));
            window.draw(shadow);
        }

        drawTextCentered(text, x + w / 2, y + h / 2, 18, sf::Color::White);
    }

    void drawInputField(float x, float y, float w, float h, const string& text, bool active, sf::Color accentColor) {
        sf::RectangleShape field(sf::Vector2f(w, h));
        field.setPosition(sf::Vector2f(x, y));
        field.setFillColor(sf::Color(250, 250, 250));
        field.setOutlineThickness(2);
        field.setOutlineColor(active ? accentColor : sf::Color(220, 220, 220));
        window.draw(field);

        drawText(text, x + 10, y + h / 2 - 10, 16, sf::Color(50, 50, 50));
    }

    void drawProgressBar(float x, float y, float w, float h, float progress, sf::Color color) {
        sf::RectangleShape fill(sf::Vector2f(w * progress, h));
        fill.setPosition(sf::Vector2f(x, y));
        fill.setFillColor(color);
        window.draw(fill);
    }

    void drawStatCard(float x, float y, float w, float h, const string& label, const string& value, sf::Color accentColor) {
        drawRoundedRect(x, y, w, h, 5, sf::Color(255, 255, 255, 230), accentColor);
        drawText(label, x + 10, y + 10, 12, sf::Color(100, 100, 100));
        drawText(value, x + 10, y + 30, 18, accentColor);
    }

    void drawActionCard(float x, float y, float w, float h, const string& title, const string& subtitle, sf::Color color, bool hovered) {
        sf::Color cardColor = hovered ? sf::Color(245, 245, 245) : sf::Color(255, 255, 255);
        drawRoundedRect(x, y, w, h, 8, cardColor, color);

        sf::RectangleShape accent(sf::Vector2f(5, h));
        accent.setPosition(sf::Vector2f(x, y));
        accent.setFillColor(color);
        window.draw(accent);

        drawText(title, x + 20, y + 15, 20, sf::Color(50, 50, 50));
        drawText(subtitle, x + 20, y + 40, 14, sf::Color(120, 120, 120));
    }

    void drawRideHistoryCard(float x, float y, float w, float h, const string& rider, const string& driver,
        const string& pickup, const string& drop, float fare, bool alternate) {
        sf::Color bgColor = alternate ? sf::Color(248, 248, 248) : sf::Color(255, 255, 255);
        drawRoundedRect(x, y, w, h, 5, bgColor, sf::Color(230, 230, 230));

        drawText(pickup + " -> " + drop, x + 15, y + 10, 16, sf::Color(50, 50, 50));
        drawText("Driver: " + driver, x + 15, y + 35, 14, sf::Color(100, 100, 100));
        drawText("Rs. " + to_string((int)fare), x + w - 100, y + 20, 18, sf::Color(100, 150, 255));
    }

    void drawNotification(const string& msg, float x, float y, sf::Color color) {
        sf::Color borderColor(
            (color.r > 20) ? color.r - 20 : 0,
            (color.g > 20) ? color.g - 20 : 0,
            (color.b > 20) ? color.b - 20 : 0,
            color.a
        );
        drawRoundedRect(x, y, 300, 50, 5, color, borderColor);
        drawText(msg, x + 15, y + 15, 14, sf::Color::White);
    }
};

// ==================== MAIN ====================

int main() {
    VelocityGoApp app;
    app.run();
    return 0;
}