//g++ -o tanks main.cpp -lsfml-graphics -lsfml-window -lsfml-system && ./tanks
#include <cmath>
#include <SFML/Graphics.hpp>
#include <thread>
#include <chrono>
#include <iostream>

#define G 6.673889e-11
#define rezx 1366
#define rezy 768
#define projSpeed 3

using namespace std;
using namespace std::this_thread;     // sleep_for, sleep_until
using namespace std::chrono_literals; // ns, us, ms, s, h, etc.
using std::chrono::system_clock;

class Vector2{
public:
    double x = 0;
    double y = 0;

    Vector2(double x, double y){
        this->x = x;
        this->y = y;
    }

    Vector2 offset(Vector2 object){
        return Vector2((object.x - this->x) , (object.y - this->y));
    }

    Vector2 normalize(){
        return Vector2(this->x/(this->x+this->y),this->y/(this->x+this->y));
    }

    float mag(){
        return sqrt((this->x)*(this->x) + (this->y)*(this->y));
    }

};

class Plane{
public:
    Vector2 point = Vector2(0, 0);
    bool low = false;
    sf::Vertex line[2];
    int length = 0;
};

class PlaneX: public Plane{
public:
    PlaneX(Vector2 point, int length, bool isLow=false){
        this->point = point;
        this->length = length;
        this->line[0] = sf::Vertex(sf::Vector2f(this->point.x, this->point.y), (isLow)? sf::Color(100,100,100) : sf::Color(255,255,255));
        this->line[1] = sf::Vertex(sf::Vector2f(this->point.x+this->length, this->point.y), (isLow)? sf::Color(100,100,100) : sf::Color(255,255,255));
        this->low = isLow;

    }
};
class PlaneY: public Plane{
public:
    PlaneY(Vector2 point, int length, bool isLow=false){
        this->point = point;
        this->length = length;
        this->line[0] = sf::Vertex(sf::Vector2f(this->point.x, this->point.y), (isLow)? sf::Color(100,100,100) : sf::Color(255,255,255));
        this->line[1] = sf::Vertex(sf::Vector2f(this->point.x, this->point.y+this->length), (isLow)? sf::Color(100,100,100) : sf::Color(255,255,255));
        this->low = isLow;
    }
};

class Projectile{
private:
    sf::CircleShape body;
public:
    bool available = true;
    int age = 0;
    int lifespan = 5;
    float mass = 1;
    int radius;
    Vector2 velocity = Vector2(0,0);
    Vector2 pos = Vector2(0,0);

    Projectile(float mass, Vector2 velocity, Vector2 pos, int radius, sf::Color color){
        this->mass = mass;
        this->body.setPosition(pos.x, pos.y);
        this->pos = pos;
        this->velocity = velocity;
        this->body.setRadius(radius);
        this->body.setFillColor(color);
        this->radius = radius;
        this->body.setOrigin(radius, radius);
    }

/*    void UpdateVelocity (Projectile allBody[], int bodies){   // Not really useful for a top-down shooter, is it?
//        for (int b = 0; b < bodies; b++){
//            if (&allBody[b] != this){
//                Vector2 dst = this->pos.offset(allBody[b].pos);
//                float velocity = (G * allBody[b].mass)/ dst.mag();
//                this->velocity.x += velocity*dst.x;
//                this->velocity.y += velocity*dst.y;
//            }
//        }
//    }
*/

    void UpdatePosition(){
        this->pos.x += this->velocity.x;
        this->pos.y += this->velocity.y;
    }

    sf::CircleShape Render(){
        body.setPosition(pos.x, pos.y);
        return body;
    }

    void CheckBounceX(PlaneX allPlanes[], int planes){
        for (int p = 0; p < planes; p++){
            if (abs(allPlanes[p].point.y - pos.y) < radius && !allPlanes[p].low){
                if (pos.x > allPlanes[p].point.x && pos.x < allPlanes[p].point.x+allPlanes[p].length){
                    velocity.y = -velocity.y;
                    this->lifespan --;
                }
            }
        }
    }
    void CheckBounceY(PlaneY allPlanes[], int planes){
        for (int p = 0; p < planes; p++){
            if (abs(allPlanes[p].point.x - pos.x) < radius && !allPlanes[p].low){
                if (pos.y > allPlanes[p].point.y && pos.y < allPlanes[p].point.y+allPlanes[p].length){
                    velocity.x = -velocity.x;
                    this->lifespan --;
                }
            }
        }
    }
    void SetPos(Vector2 pos){
        this->pos = pos;
    }
    void SetVel(Vector2 vel){
        this->velocity = vel;
    }
};

class Tank{
public:


    sf::Sprite tank;
    sf::Sprite turret;
    int radius = 25;

    Tank(Vector2 pos, int rotation, sf::Texture& base, sf::Texture& turret){

        this->tank = sf::Sprite(base);
        this->tank.setOrigin(25, 25);
        this->tank.setRotation(rotation+90);
        this->tank.setPosition(pos.x, pos.y);


        this->turret = sf::Sprite(turret);
        this->turret.setOrigin(25, 25);
        this->turret.setRotation(rotation+90);
        this->turret.setPosition(pos.x, pos.y);


//        this->tank.setFillColor(sf::Color(255, 255, 255));
//        this->tank.setSize(sf::Vector2f(50,50));

//        this->turret.setFillColor(sf::Color(50, 155, 75));
//        this->turret.setSize(sf::Vector2f(50,10));
    }

    void Forward(PlaneX allPlanesX[], int planesX, PlaneY allPlanesY[], int planesY, int multiplier=1){
        float x = cos(this->tank.getRotation()*0.0174532777778)*multiplier;
        float y = sin(this->tank.getRotation()*0.0174532777778)*multiplier;
//        cout << x << " " << y << "\n";
        if (this->CheckBounceY(allPlanesY, planesY) == -1) { x = abs(x); y = y*0.3; }
        if (this->CheckBounceY(allPlanesY, planesY) == 1) { x = -abs(x); y = y*0.3; }
        if (this->CheckBounceX(allPlanesX, planesX) == -1) { y = abs(y); x = x*0.3; }
        if (this->CheckBounceX(allPlanesX, planesX) == 1) { y = -abs(y); x = x*0.3; }
//        cout << x+this->tank.getPosition().x << " " << y+this->tank.getPosition().y << "\n";
        this->tank.setPosition(x+this->tank.getPosition().x,y+this->tank.getPosition().y);
        this->turret.setPosition(this->tank.getPosition());
    }


    void Rotate(float rotation){
        this->tank.rotate(rotation);
    }

    bool checkDeath(Vector2 ProjectilePos){
        sf::Vector2f self_pos = this->tank.getPosition();
        return (pow(ProjectilePos.x - self_pos.x, 2) + pow(ProjectilePos.y - self_pos.y, 2) < pow(this->radius, 2));
    }

    int CheckBounceX(PlaneX allPlanes[], int planes){
//        cout << this->tank.getPosition().x << " " << this->tank.getPosition().y << "\n";
//        cout << planes << "\n";
        for (int p = 0; p < planes; p++) {
//            cout << allPlanes[p].point.y << p << "--\n";
            if (this->tank.getPosition().x > allPlanes[p].point.x && this->tank.getPosition().x < allPlanes[p].point.x + allPlanes[p].length) {
                if (abs(allPlanes[p].point.y - this->tank.getPosition().y) < radius) {
//                    cout << "Success " << p << "\n";
                    if (allPlanes[p].point.y < this->tank.getPosition().y) {
                        return -1;
                    } else {
                        return 1;
                    }
//                return (allPlanes[p].point.x - this->tank.getPosition().x / abs(allPlanes[p].point.x - this->tank.getPosition().x));
                }
            }
        }
        return 0;
    }
    int CheckBounceY(PlaneY allPlanes[], int planes){
        for (int p = 0; p < planes; p++) {
//            cout << allPlanes[p].point.y << "--\n";
            if (this->tank.getPosition().y > allPlanes[p].point.y && this->tank.getPosition().y < allPlanes[p].point.y + allPlanes[p].length) {
                if (abs(allPlanes[p].point.x - this->tank.getPosition().x) < radius) {
                    if (allPlanes[p].point.x < this->tank.getPosition().x) {
                        return -1;
                    } else {
                        return 1;
                    }
//                return (allPlanes[p].point.y - this->tank.getPosition().y / abs(allPlanes[p].point.y - this->tank.getPosition().y));
                }
            }
        }
        return 0;
    }

};

class KeySet{
public:
    int p1_horizontal = 0;
    int p1_vertical = 0;
    int p1_fire_cool_down = 10;
    float p1_canon_aim = 0;

    int p2_horizontal = 0;
    int p2_vertical = 0;
    int p2_fire_cool_down = 10;
    float p2_canon_aim = 0;
};

class Barrier{
public:
    PlaneX planesX1 = PlaneX(Vector2(0, 0), 0);
    PlaneX planesX2 = PlaneX(Vector2(0, 0), 0);
    PlaneY planesY1 = PlaneY(Vector2(0, 0), 0);
    PlaneY planesY2 = PlaneY(Vector2(0, 0), 0);
    sf::RectangleShape view;

    Barrier(Vector2 topLeft, Vector2 bottomRight) {
        planesX1.point = topLeft;
        planesX2.point = Vector2(topLeft.x, bottomRight.y);
        planesY1.point = topLeft;
        planesY2.point = Vector2(bottomRight.x, topLeft.y);
        view.setScale(bottomRight.x-topLeft.x, bottomRight.y-topLeft.y);
        view.setPosition(sf::Vector2f(topLeft.x, topLeft.y));
        view.setFillColor(sf::Color(200, 240, 100));
    }
};

void fire_projectile(Projectile *projectile, Vector2 Pos, Vector2 Vel){
    projectile->SetPos(Pos);
    projectile->SetVel(Vel);
    projectile->age = 0;
    projectile->available = false;
}

void Update(Projectile allBody[], PlaneX PlanesX[], int numX, PlaneY PlanesY[], int numY, Tank *tank1, Tank *tank2, int bodies, sf::RenderWindow *window){
    for (int b = 0; b < bodies; b++){
        window->draw((allBody[b]).Render());
    }

    for (int p = 0; p < numX; p++){
        window->draw(PlanesX[p].line, 2, sf::Lines);
    }
    for (int p = 0; p < numY; p++){
        window->draw(PlanesY[p].line, 2, sf::Lines);
    }


    window->draw(tank1->tank);
    window->draw(tank1->turret);
    window->draw(tank2->tank);
    window->draw(tank2->turret);



    window->display();
}

void FixedUpdate(Projectile AllProjectiles[], PlaneX allPlanesX[], int planesX, PlaneY allPlanesY[], int planesY, int bodies, Tank *tank1, Tank *tank2, KeySet *keys){
    int projectileLen = 16;
    while (true){
        sleep_for(5ms);

        if (keys->p1_fire_cool_down > 0){
            keys->p1_fire_cool_down -= 1;
        }
        if (keys->p2_fire_cool_down > 0){
            keys->p2_fire_cool_down -= 1;
        }
        if (keys->p1_fire_cool_down == -1){
            for (int i = 0; i <= projectileLen; i+= 2){
                if (AllProjectiles[i].available){
                    fire_projectile(&AllProjectiles[i], //TODO fire from cannon, not tank direction
                                    Vector2(tank1->turret.getPosition().x, tank1->turret.getPosition().y),
                                    Vector2(cos(tank1->turret.getRotation()*0.0174532777778)*projSpeed, sin(tank1->turret.getRotation()*0.0174532777778)*projSpeed));
                    break;
                }
            }
            keys->p1_fire_cool_down = 30;
        }
        if (keys->p2_fire_cool_down == -1){
            for (int i = 1; i <= projectileLen; i+= 2){
                if (AllProjectiles[i].available){
                    fire_projectile(&AllProjectiles[i],
                                    Vector2(tank2->turret.getPosition().x, tank2->turret.getPosition().y),
                                    Vector2(cos(tank2->turret.getRotation()*0.0174532777778)*projSpeed, sin(tank2->turret.getRotation()*0.0174532777778)*projSpeed));
                    break;
                }
            }
            keys->p2_fire_cool_down = 30;
        }


        for (int i = 0; i < projectileLen; i++){
            if (AllProjectiles[i].lifespan == 0){
                AllProjectiles[i] = Projectile(0, Vector2(0, 0), Vector2(-100, -100), 10, sf::Color(110, 110, 110));
            }
        }


        tank1->Forward(allPlanesX, planesX, allPlanesY, planesY, keys->p1_vertical);
        tank2->Forward(allPlanesX, planesX, allPlanesY, planesY, keys->p2_vertical);
        tank1->turret.rotate(keys->p1_canon_aim);
        tank2->turret.rotate(keys->p2_canon_aim);


        if (keys->p1_vertical != 0){
            tank1->Rotate(keys->p1_horizontal);
        }else{
            tank1->Rotate(keys->p1_horizontal*0.5);
        }
        if (keys->p2_vertical != 0) {
            tank2->Rotate(keys->p2_horizontal);
        }else{
            tank2->Rotate(keys->p2_horizontal*0.5);
        }

        for (int b = 0; b < bodies; b++){
            AllProjectiles[b].age += 1;
            AllProjectiles[b].CheckBounceX(allPlanesX, planesX);
            AllProjectiles[b].CheckBounceY(allPlanesY, planesY);
            if (AllProjectiles[b].age >= 15) {
                if (tank1->checkDeath(AllProjectiles[b].pos)) {
//                    tank1->tank.setFillColor(sf::Color(200, 0, 0));
                    cout << "Player 1 Death\n";
                    AllProjectiles[b] = Projectile(0, Vector2(0, 0), Vector2(-100, -100), 10, sf::Color(110, 110, 110));
                }
                if (tank2->checkDeath(AllProjectiles[b].pos)) {
                    cout << "Player 2 Death\n";
//                    tank2->tank.setFillColor(sf::Color(200, 0, 0));
                    AllProjectiles[b] = Projectile(0, Vector2(0, 0), Vector2(-100, -100), 10, sf::Color(110, 110, 110));
                }
            }
        }
        for (int b = 0; b < bodies; b++){
            (AllProjectiles[b]).UpdatePosition();
        }
//        cout << tank1->tank.getPosition().x << " " << tank1->tank.getPosition().y << "\n";
//        cout << tank2->tank.getPosition().x << " " << tank2->tank.getPosition().y << "\n";
    }
}

int main() {

    //Load textures

    sf::Texture greyTankBase;
    if(!greyTankBase.loadFromFile("./Assets/GreyBase.png")){
        cout << "Errpr loading greyTankBase.png\n";
        return 0;
    }else{
        cout << "Loaded greyTankBase.png\n";
    }
    sf::Texture greyTankTurret;
    if(!greyTankTurret.loadFromFile("./Assets/GreyTurret.png")){
        cout << "Errpr loading greyTankBase.png\n";
        return 0;
    }else{
        cout << "Loaded greyTankBase.png\n";
    }



    // Player 1 and 2 Tanks
    Tank tank1 = Tank(Vector2(100, 100), 0, greyTankBase, greyTankTurret);
    Tank tank2 = Tank(Vector2(100, 100), 45, greyTankBase, greyTankTurret);

    tank1.tank.setColor(sf::Color(255, 50, 50));
    tank2.tank.setColor(sf::Color(50, 50, 255));
    tank1.turret.setColor(sf::Color(255, 100, 100));
    tank2.turret.setColor(sf::Color(100, 100, 255));

//    tank1.tank.setTexture(greyTankBase);
//    tank2.tank.setTexture(greyTankBase);




    // Create Window
    sf::RenderWindow window(sf::VideoMode(rezx, rezy), "TanksAlot");

    // Keypress pointers object
    KeySet keys;

    // Projectiles (Max 8 per player)
    const int projectiles = 16;
    Projectile allProjectiles[projectiles] = {
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 5, sf::Color(255, 255, 255)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 5, sf::Color(255, 255, 255)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 5, sf::Color(255, 255, 255)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 5, sf::Color(255, 255, 255)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 5, sf::Color(255, 255, 255)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 5, sf::Color(255, 255, 255)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 5, sf::Color(255, 255, 255)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 5, sf::Color(255, 255, 255)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 5, sf::Color(255, 255, 255)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 5, sf::Color(255, 255, 255)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 5, sf::Color(255, 255, 255)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 5, sf::Color(255, 255, 255)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 5, sf::Color(255, 255, 255)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 5, sf::Color(255, 255, 255)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 5, sf::Color(255, 255, 255)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 5, sf::Color(255, 255, 255)),
    }; // Goofy constructor

    // Colliders in the X axis
    const int planesX = 4;
    PlaneX allPlanesX[planesX] = {
            PlaneX(Vector2(0, 0), rezx),
            PlaneX(Vector2(0, rezy), rezx),
            PlaneX(Vector2(300, rezy/2), 130, true),
            PlaneX(Vector2(600, rezy/2), 130),
    };
    // Colliders in the Y axis
    const int planesY = 3;
    PlaneY allPlanesY[planesY] = {
            PlaneY(Vector2(0, 0), rezy),
            PlaneY(Vector2(rezx, 0), rezy),
            PlaneY(Vector2(600, rezy/2), 130)
            //(*paddle1).paddle,
            //(*paddle2).paddle
    };


    // Keycapture event
    sf::Event event{};

    // Start physics thread
    std::thread thread(FixedUpdate, allProjectiles, allPlanesX, planesX, allPlanesY, planesY, projectiles, &tank1, &tank2, &keys);

    // Graphics thread
    while (window.isOpen()){

        // Key capture
        while (window.pollEvent((event))) {
            if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::T)) {
                window.close();
                exit(0);
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)){
                keys.p1_vertical = 1;
            }else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)){
                keys.p1_vertical = -1;
            }else{
                keys.p1_vertical = 0;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)){
                keys.p1_horizontal = -1;
            }else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)){
                keys.p1_horizontal = 1;
            }else{
                keys.p1_horizontal = 0;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)){
                if (keys.p1_fire_cool_down == 0){
                    keys.p1_fire_cool_down = -1;
                }
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::L)){
                keys.p1_canon_aim = -1;
            }else if (sf::Keyboard::isKeyPressed(sf::Keyboard::J)){
                keys.p1_canon_aim = 1;
            }else{
                keys.p1_canon_aim = 0;
            }




            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)){
                keys.p2_vertical = 1;
            }else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)){
                keys.p2_vertical = -1;
            }else{
                keys.p2_vertical = 0;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)){
                keys.p2_horizontal = -1;
            }else if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)){
                keys.p2_horizontal = 1;
            }else{
                keys.p2_horizontal = 0;
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)){
                if (keys.p2_fire_cool_down == 0){
                    keys.p2_fire_cool_down = -1;
                }
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::L)){
                keys.p2_canon_aim = -1;
            }else if (sf::Keyboard::isKeyPressed(sf::Keyboard::J)){
                keys.p2_canon_aim = 1;
            }else{
                keys.p2_canon_aim = 0;
            }


        }

        // Render GameObjects
        window.clear();
        Update(allProjectiles, allPlanesX, planesX, allPlanesY, planesY, &tank1, &tank2, projectiles, &window);
    }


    return 0;
}
