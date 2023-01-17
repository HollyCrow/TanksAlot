//g++ -o tanks main.cpp -lsfml-graphics -lsfml-window -lsfml-system && ./tanks
// If this breaks: 1. make sure sfml is installed, 2. Give up.
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

class Barrier{ // Exceptionally lazy class.
public:
    bool low = false;
    PlaneX planesX1 = PlaneX(Vector2(0, 0), 0);
    PlaneX planesX2 = PlaneX(Vector2(0, 0), 0);
    PlaneY planesY1 = PlaneY(Vector2(0, 0), 0);
    PlaneY planesY2 = PlaneY(Vector2(0, 0), 0);
    sf::RectangleShape view;

    Barrier(Vector2 topLeft, Vector2 bottomRight, bool isLow=false) {
        this->planesX1 = PlaneX(topLeft, bottomRight.x-topLeft.x, isLow);
        this->planesX2 = PlaneX(Vector2(topLeft.x, bottomRight.y), bottomRight.x-topLeft.x, isLow);
        this->planesY1 = PlaneY(topLeft, bottomRight.y-topLeft.y, isLow);
        this->planesY2 = PlaneY(Vector2(bottomRight.x, topLeft.y), bottomRight.y-topLeft.y, isLow);
        this->low = isLow;
        this->view.setSize(sf::Vector2f(bottomRight.x-topLeft.x, bottomRight.y-topLeft.y));
        this->view.setPosition(sf::Vector2f(topLeft.x, topLeft.y));
        this->view.setFillColor((isLow)? sf::Color(50, 50, 50) : sf::Color(255, 255, 255));
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

    void CheckBounceX(PlaneX allPlanes[], int planes, Barrier allBarriers[], int barriers){
        for (int p = 0; p < planes; p++){
            if (abs(allPlanes[p].point.y - pos.y) < radius && !allPlanes[p].low){
                if (pos.x > allPlanes[p].point.x && pos.x < allPlanes[p].point.x+allPlanes[p].length){
                    velocity.y = -velocity.y;
                    this->lifespan --;
                }
            }
        }
        for (int b = 0; b < barriers; b++){
            if (!allBarriers[b].low){
                if (abs(allBarriers[b].planesX1.point.y - pos.y) < radius && !allBarriers[b].planesX1.low){
                    if (pos.x > allBarriers[b].planesX1.point.x && pos.x < allBarriers[b].planesX1.point.x+allBarriers[b].planesX1.length){
                        velocity.y = -velocity.y;
                        this->lifespan --;
                    }
                }
                if (abs(allBarriers[b].planesX2.point.y - pos.y) < radius && !allBarriers[b].planesX2.low){
                    if (pos.x > allBarriers[b].planesX2.point.x && pos.x < allBarriers[b].planesX2.point.x+allBarriers[b].planesX2.length){
                        velocity.y = -velocity.y;
                        this->lifespan --;
                    }
                }
            }
        }
    }
    void CheckBounceY(PlaneY allPlanes[], int planes, Barrier allBarriers[], int barriers){
        for (int p = 0; p < planes; p++){
            if (abs(allPlanes[p].point.x - pos.x) < radius && !allPlanes[p].low){
                if (pos.y > allPlanes[p].point.y && pos.y < allPlanes[p].point.y+allPlanes[p].length){
                    velocity.x = -velocity.x;
                    this->lifespan --;
                }
            }
        }

        for (int b = 0; b < barriers; b++){
            if (!allBarriers[b].low){
                if (abs(allBarriers[b].planesY1.point.x - pos.x) < radius && !allBarriers[b].planesY1.low){
                    if (pos.y > allBarriers[b].planesY1.point.y && pos.y < allBarriers[b].planesY1.point.y+allBarriers[b].planesY1.length){
                        velocity.x = -velocity.x;
                        this->lifespan --;
                    }
                }
                if (abs(allBarriers[b].planesY2.point.x - pos.x) < radius && !allBarriers[b].planesY2.low){
                    if (pos.y > allBarriers[b].planesY2.point.y && pos.y < allBarriers[b].planesY2.point.y+allBarriers[b].planesY2.length){
                        velocity.x = -velocity.x;
                        this->lifespan --;
                    }
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

    void Forward(PlaneX allPlanesX[], int planesX, PlaneY allPlanesY[], int planesY, int multiplier, Barrier allBarriers[], int barriers){
        float x = cos(this->tank.getRotation()*0.0174532777778)*multiplier;
        float y = sin(this->tank.getRotation()*0.0174532777778)*multiplier;
//        cout << x << " " << y << "\n";
        if (this->CheckBounceY(allPlanesY, planesY, allBarriers, barriers) == -1) { x = abs(x); y = y*0.3; }
        if (this->CheckBounceY(allPlanesY, planesY, allBarriers, barriers) == 1) { x = -abs(x); y = y*0.3; }
        if (this->CheckBounceX(allPlanesX, planesX, allBarriers, barriers) == -1) { y = abs(y); x = x*0.3; }
        if (this->CheckBounceX(allPlanesX, planesX, allBarriers, barriers) == 1) { y = -abs(y); x = x*0.3; }
//        cout << x+this->tank.getPosition().x << " " << y+this->tank.getPosition().y << "\n";
        this->tank.setPosition(x+this->tank.getPosition().x,y+this->tank.getPosition().y);
        this->turret.setPosition(this->tank.getPosition());
    }

    void setPosition(Vector2 pos){
        this->tank.setPosition(pos.x, pos.y);
        this->turret.setPosition(pos.x, pos.y);
    }

    void Rotate(float rotation){
        this->tank.rotate(rotation);
    }

    bool checkDeath(Vector2 ProjectilePos){
        sf::Vector2f self_pos = this->tank.getPosition();
        return (pow(ProjectilePos.x - self_pos.x, 2) + pow(ProjectilePos.y - self_pos.y, 2) < pow(this->radius, 2));
    }

    int CheckBounceX(PlaneX allPlanes[], int planes, Barrier allBarriers[], int barriers){
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
        for (int b = 0; b < barriers; b++){
            if (this->tank.getPosition().x > allBarriers[b].planesX1.point.x && this->tank.getPosition().x < allBarriers[b].planesX1.point.x + allBarriers[b].planesX1.length) {
                if (abs(allBarriers[b].planesX1.point.y - this->tank.getPosition().y) < radius) {
//                    cout << "Success " << p << "\n";
                    if (allBarriers[b].planesX1.point.y < this->tank.getPosition().y) {
                        return -1;
                    } else {
                        return 1;
                    }
//                return (allPlanes[p].point.x - this->tank.getPosition().x / abs(allPlanes[p].point.x - this->tank.getPosition().x));
                }
            }
            if (this->tank.getPosition().x > allBarriers[b].planesX2.point.x && this->tank.getPosition().x < allBarriers[b].planesX2.point.x + allBarriers[b].planesX2.length) {
                if (abs(allBarriers[b].planesX2.point.y - this->tank.getPosition().y) < radius) {
//                    cout << "Success " << p << "\n";
                    if (allBarriers[b].planesX2.point.y < this->tank.getPosition().y) {
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
    int CheckBounceY(PlaneY allPlanes[], int planes, Barrier allBarriers[], int barriers){
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
        for (int b = 0; b < barriers; b++){
            if (this->tank.getPosition().y > allBarriers[b].planesY1.point.y && this->tank.getPosition().y < allBarriers[b].planesY1.point.y + allBarriers[b].planesY1.length) {
                if (abs(allBarriers[b].planesY1.point.x - this->tank.getPosition().x) < radius) {
//                    cout << "Success " << p << "\n";
                    if (allBarriers[b].planesY1.point.x < this->tank.getPosition().x) {
                        return -1;
                    } else {
                        return 1;
                    }
//                return (allPlanes[p].point.x - this->tank.getPosition().x / abs(allPlanes[p].point.x - this->tank.getPosition().x));
                }
            }
            if (this->tank.getPosition().y > allBarriers[b].planesY2.point.y && this->tank.getPosition().y < allBarriers[b].planesY2.point.y + allBarriers[b].planesY2.length) {
                if (abs(allBarriers[b].planesY2.point.x - this->tank.getPosition().x) < radius) {
//                    cout << "Success " << p << "\n";
                    if (allBarriers[b].planesY2.point.x < this->tank.getPosition().x) {
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

// 683
// 384
// 1366x768

/*
const Barrier state1[9] = {
        Barrier(Vector2(483, 0), Vector2(883, 200), false),
        Barrier(Vector2(483, 568), Vector2(883, 768), false),
        Barrier(Vector2(150, 150), Vector2(175, 300), false),
        Barrier(Vector2(150, 300), Vector2(175, 400), true),
        Barrier(Vector2(150, 400), Vector2(175, 768-150), false),
        Barrier(Vector2(1366-175, 150), Vector2(1366-150, 300), false),
        Barrier(Vector2(1366-175, 300), Vector2(1366-150, 400), true),
        Barrier(Vector2(1366-175, 400), Vector2(1366-150, 768-150), false),
        Barrier(Vector2(483, 374), Vector2(883, 394), true),
};

const Barrier state2[] = {
        Barrier(Vector2(483, 0), Vector2(883, 768), true),
        Barrier(Vector2(150, 150), Vector2(175, 300), false),
        Barrier(Vector2(150, 768-300), Vector2(175, 768-150), false),
        Barrier(Vector2(1366-175, 150), Vector2(1366-150, 300), false),
        Barrier(Vector2(1366-175, 768-300), Vector2(1366-150, 768-150), false),
};

const Barrier state3[] = {
        Barrier(Vector2(663, 0), Vector2(703, 304), false),
        Barrier(Vector2(663, 464), Vector2(703, 768), false),
        Barrier(Vector2(663-300, 184), Vector2(703-300, 584), false),
        Barrier(Vector2(663+300, 184), Vector2(703+300, 584), false),
};

const Barrier state4[] = {
        Barrier(Vector2(628, 23), Vector2(732, 329), false),
        Barrier(Vector2(627, 427), Vector2(735, 730), false),
        Barrier(Vector2(522, 273), Vector2(529, 483), true),
        Barrier(Vector2(829, 272), Vector2(837, 482), true),
        Barrier(Vector2(343, 323), Vector2(401, 433), false),
        Barrier(Vector2(958, 325), Vector2(1017, 433), false),
        Barrier(Vector2(59, 247), Vector2(272, 278), false),
        Barrier(Vector2(61, 478), Vector2(272, 509), false),
        Barrier(Vector2(1087, 247), Vector2(1299, 278), false),
        Barrier(Vector2(1088, 478), Vector2(1300, 509), false),
};



const Barrier state5[] = {
        //const scale = 26.29 // just put this somewhere I guess

// Row 1
        Barrier(Vector2(2*26.29, 2*26.29), Vector2(4*26.29, 4*26.29), false),
        Barrier(Vector2(8*26.29, 2*26.29), Vector2(10*26.29, 4*26.29), false),
        Barrier(Vector2(14*26.29, 2*26.29), Vector2(16*26.29, 4*26.29), false),
        Barrier(Vector2(20*26.29, 2*26.29), Vector2(22*26.29, 4*26.29), false),
        Barrier(Vector2(26*26.29, 2*26.29), Vector2(28*26.29, 4*26.29), false),
        Barrier(Vector2(32*26.29, 2*26.29), Vector2(34*26.29, 4*26.29), false),
        Barrier(Vector2(38*26.29, 2*26.29), Vector2(40*26.29, 4*26.29), false),

// Row 2
        Barrier(Vector2(4*26.29, 6*26.29), Vector2(6*26.29, 8*26.29), false),
        Barrier(Vector2(10*26.29, 6*26.29), Vector2(12*26.29, 8*26.29), false),
        Barrier(Vector2(16*26.29, 6*26.29), Vector2(18*26.29, 8*26.29), false),
        Barrier(Vector2(22*26.29, 6*26.29), Vector2(24*26.29, 8*26.29), false),
        Barrier(Vector2(28*26.29, 6*26.29), Vector2(30*26.29, 8*26.29), false),
        Barrier(Vector2(34*26.29, 6*26.29), Vector2(36*26.29, 8*26.29), false),
        Barrier(Vector2(40*26.29, 6*26.29), Vector2(42*26.29, 8*26.29), false),

// Row 3
        Barrier(Vector2(6*26.29, 10*26.29), Vector2(8*26.29, 12*26.29), false),
        Barrier(Vector2(12*26.29, 10*26.29), Vector2(14*26.29, 12*26.29), false),
        Barrier(Vector2(18*26.29, 10*26.29), Vector2(20*26.29, 12*26.29), false),
        Barrier(Vector2(24*26.29, 10*26.29), Vector2(26*26.29, 12*26.29), false),
        Barrier(Vector2(30*26.29, 10*26.29), Vector2(32*26.29, 12*26.29), false),
        Barrier(Vector2(36*26.29, 10*26.29), Vector2(38*26.29, 12*26.29), false),
        Barrier(Vector2(42*26.29, 10*26.29), Vector2(44*26.29, 12*26.29), false)
};*/



void fire_projectile(Projectile *projectile, Vector2 Pos, Vector2 Vel){
    projectile->SetPos(Pos);
    projectile->SetVel(Vel);
    projectile->age = 0;
    projectile->available = false;
}

void Update(Projectile allProjectiles[], PlaneX PlanesX[], int numX, PlaneY PlanesY[], int numY, Tank *tank1, Tank *tank2, int projectiles, Barrier allBarriers[], int barriers, sf::RenderWindow *window){

    for (int p = 0; p < numX; p++){
        window->draw(PlanesX[p].line, 2, sf::Lines);
    }
    for (int p = 0; p < numY; p++){
        window->draw(PlanesY[p].line, 2, sf::Lines);
    }

    for (int b = 0; b < barriers; b++){
        window->draw(allBarriers[b].view);
    }




    window->draw(tank1->tank);
    window->draw(tank1->turret);
    window->draw(tank2->tank);
    window->draw(tank2->turret);
    for (int b = 0; b < projectiles; b++){
        window->draw((allProjectiles[b]).Render());
    }




}

void FixedUpdate(Projectile AllProjectiles[], int bodies, PlaneX allPlanesX[], int planesX, PlaneY allPlanesY[], int planesY, Barrier allBarriers[], int barriers, Tank *tank1, Tank *tank2, KeySet *keys, bool *isTitleScreen, sf::Sprite *titleScreen, sf::Texture blueWin, sf::Texture redWin){
    int projectileLen = 16;
    while (true){
        if(!*isTitleScreen){
            sleep_for(5ms);

            if (keys->p1_fire_cool_down > 0) {
                keys->p1_fire_cool_down -= 1;
            }
            if (keys->p2_fire_cool_down > 0) {
                keys->p2_fire_cool_down -= 1;
            }
            if (keys->p1_fire_cool_down == -1) {
                for (int i = 0; i <= projectileLen; i += 2) {
                    if (AllProjectiles[i].available) {
                        fire_projectile(&AllProjectiles[i], //TODO fire from cannon, not tank direction
                                        Vector2(tank1->turret.getPosition().x, tank1->turret.getPosition().y),
                                        Vector2(cos(tank1->turret.getRotation() * 0.0174532777778) * projSpeed,
                                                sin(tank1->turret.getRotation() * 0.0174532777778) * projSpeed));
                        break;
                    }
                }
                keys->p1_fire_cool_down = 30;
            }
            if (keys->p2_fire_cool_down == -1) {
                for (int i = 1; i <= projectileLen; i += 2) {
                    if (AllProjectiles[i].available) {
                        fire_projectile(&AllProjectiles[i],
                                        Vector2(tank2->turret.getPosition().x, tank2->turret.getPosition().y),
                                        Vector2(cos(tank2->turret.getRotation() * 0.0174532777778) * projSpeed,
                                                sin(tank2->turret.getRotation() * 0.0174532777778) * projSpeed));
                        break;
                    }
                }
                keys->p2_fire_cool_down = 30;
            }


            for (int i = 0; i < projectileLen; i++) {
                if (AllProjectiles[i].lifespan == 0) {
                    AllProjectiles[i] = Projectile(0, Vector2(0, 0), Vector2(-100, -100), 10, sf::Color(110, 110, 110));
                }
            }


            tank1->Forward(allPlanesX, planesX, allPlanesY, planesY, keys->p1_vertical, allBarriers, barriers);
            tank2->Forward(allPlanesX, planesX, allPlanesY, planesY, keys->p2_vertical, allBarriers, barriers);
            tank1->turret.rotate(keys->p1_canon_aim);
            tank2->turret.rotate(keys->p2_canon_aim);


            if (keys->p1_vertical != 0) {
                tank1->Rotate(keys->p1_horizontal);
            } else {
                tank1->Rotate(keys->p1_horizontal * 0.5);
            }
            if (keys->p2_vertical != 0) {
                tank2->Rotate(keys->p2_horizontal);
            } else {
                tank2->Rotate(keys->p2_horizontal * 0.5);
            }

            for (int b = 0; b < bodies; b++) {
                AllProjectiles[b].age += 1;
                AllProjectiles[b].CheckBounceX(allPlanesX, planesX, allBarriers, barriers);
                AllProjectiles[b].CheckBounceY(allPlanesY, planesY, allBarriers, barriers);
                if (AllProjectiles[b].age >= 15) {
                    if (tank1->checkDeath(AllProjectiles[b].pos)) {
//                    tank1->tank.setFillColor(sf::Color(200, 0, 0));
                        cout << "Player 1 Death\n";
                        *isTitleScreen = true;
                        titleScreen->setTexture(redWin);
                        AllProjectiles[b] = Projectile(0, Vector2(0, 0), Vector2(-100, -100), 10,
                                                       sf::Color(110, 110, 110));
                    }
                    if (tank2->checkDeath(AllProjectiles[b].pos)) {
                        cout << "Player 2 Death\n";
                        *isTitleScreen = true;
                        titleScreen->setTexture(blueWin);


//                    tank2->tank.setFillColor(sf::Color(200, 0, 0));
                        AllProjectiles[b] = Projectile(0, Vector2(0, 0), Vector2(-100, -100), 10,
                                                       sf::Color(110, 110, 110));
                    }
                }
            }
            for (int b = 0; b < bodies; b++) {
                (AllProjectiles[b]).UpdatePosition();
            }
//        cout << tank1->tank.getPosition().x << " " << tank1->tank.getPosition().y << "\n";
//        cout << tank2->tank.getPosition().x << " " << tank2->tank.getPosition().y << "\n";
        }
    }
}

int main() {

    //Load textures
    sf::Texture greyTankBase;
    if(!greyTankBase.loadFromFile("./Assets/TankBase.png")){
        cout << "Errpr loading greyTankBase.png\n";
        return 0;
    }else{
        cout << "Loaded greyTankBase.png\n";
    }
    sf::Texture greyTankTurret;
    if(!greyTankTurret.loadFromFile("./Assets/TankTurret.png")){
        cout << "Errpr loading greyTankBase.png\n";
        return 0;
    }else{
        cout << "Loaded greyTankBase.png\n";
    }
    sf::Texture titleScreenImage;
    if(!titleScreenImage.loadFromFile("./Assets/TitleScreen.png")){
        cout << "Error loading titleScreen.png\n";
        return 0;
    }else{
        cout << "Loaded titleScreen.png\n";
    }
    sf::Texture redScreenImage;
    if(!redScreenImage.loadFromFile("./Assets/BlueScreen.png")){
        cout << "Error loading RedScreen.png\n";
        return 0;
    }else{
        cout << "Loaded RedScreen.png\n";
    }
    sf::Texture blueScreenImage;
    if(!blueScreenImage.loadFromFile("./Assets/RedScreen.png")){
        cout << "Error loading BlueScreen.png\n";
        return 0;
    }else{
        cout << "Loaded BlueScreen.png\n";
    }




    // Player 1 and 2 Tanks
    Tank tank1 = Tank(Vector2(100, 100), 0, greyTankBase, greyTankTurret);
    Tank tank2 = Tank(Vector2(100, 100), 45, greyTankBase, greyTankTurret);

    // Set tank base & turret hue
    tank1.tank.setColor(sf::Color(255, 50, 50));
    tank2.tank.setColor(sf::Color(50, 50, 255));
    tank1.turret.setColor(sf::Color(255, 100, 100));
    tank2.turret.setColor(sf::Color(100, 100, 255));



    // Title screen
    sf::Sprite titleScreen = sf::Sprite(titleScreenImage);
//
//    titleScreen.setTexture(titleScreenImage);


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
    const int planesX = 2;
    PlaneX allPlanesX[planesX] = {
            PlaneX(Vector2(0, 0), rezx),
            PlaneX(Vector2(0, rezy), rezx),

    };
    // Colliders in the Y axis
    const int planesY = 2;
    PlaneY allPlanesY[planesY] = {
            PlaneY(Vector2(0, 0), rezy),
            PlaneY(Vector2(rezx, 0), rezy),

            //(*paddle1).paddle,
            //(*paddle2).paddle
    };

    int barriers = 10;
    Barrier allBarriers[] = {
            // Row 1
            Barrier(Vector2(628, 0), Vector2(732, 329), false),
            Barrier(Vector2(627, 427), Vector2(735, 768), false),
            Barrier(Vector2(522, 273), Vector2(529, 483), true),
            Barrier(Vector2(829, 272), Vector2(837, 482), true),
            Barrier(Vector2(343, 323), Vector2(401, 433), false),
            Barrier(Vector2(958, 325), Vector2(1017, 433), false),
            Barrier(Vector2(59, 247), Vector2(272, 278), false),
            Barrier(Vector2(61, 478), Vector2(272, 509), false),
            Barrier(Vector2(1087, 247), Vector2(1299, 278), false),
            Barrier(Vector2(1088, 478), Vector2(1300, 509), false),
    };


    bool inTitleScreen = true;


    // Keycapture event
    sf::Event event{};

    // Start physics thread
    std::thread thread(FixedUpdate, allProjectiles, projectiles, allPlanesX, planesX, allPlanesY, planesY, allBarriers, barriers, &tank1, &tank2, &keys, &inTitleScreen, &titleScreen, blueScreenImage, redScreenImage);

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


            if (sf::Keyboard::isKeyPressed(sf::Keyboard::X) || sf::Keyboard::isKeyPressed(sf::Keyboard::M)){
                inTitleScreen = false;
                tank1.setPosition(Vector2(100, 384));
                tank2.setPosition(Vector2(1266, 384));
            }
        }

        // Render GameObjects
        window.clear();
        window.draw(allBarriers[0].view);

        Update(allProjectiles, allPlanesX, planesX, allPlanesY, planesY, &tank1, &tank2, projectiles, allBarriers, barriers, &window);
        if (inTitleScreen){
            window.draw(titleScreen);
        }

        window.display();

    }


    return 0;
}
