//g++ -o gravity main.cpp -lsfml-graphics -lsfml-window -lsfml-system && ./gravity
#include <cmath>
#include <SFML/Graphics.hpp>
#include <thread>
#include <chrono>
#include <iostream>

#define G 6.673889e-11
#define rezx 1000
#define rezy 1000
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

class PlaneX{
public:
    Vector2 point = Vector2(0, 0);
    sf::Vertex line[2];
    int length = 0;
    PlaneX(Vector2 point, int length){
        this->point = point;
        this->length = length;
        this->line[0] = sf::Vertex(sf::Vector2f(this->point.x, this->point.y));
        this->line[1] = sf::Vertex(sf::Vector2f(this->point.x+this->length, this->point.y));
    }
};
class PlaneY{
public:
    Vector2 point = Vector2(0, 0);
    sf::Vertex line[2];
    int length = 0;
    PlaneY(Vector2 point, int length){
        this->point = point;
        this->length = length;
        this->line[0] = sf::Vertex(sf::Vector2f(this->point.x, this->point.y));
        this->line[1] = sf::Vertex(sf::Vector2f(this->point.x, this->point.y+this->length));
    }
};

class Projectile{
private:
    sf::CircleShape body;
public:
    bool available = true;
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
            if (abs(allPlanes[p].point.y - pos.y) < radius){
                if (pos.x > allPlanes[p].point.x && pos.x < allPlanes[p].point.x+allPlanes[p].length){
                    velocity.y = -velocity.y;
                    this->lifespan --;
                }
            }
        }
    }
    void CheckBounceY(PlaneY allPlanes[], int planes){
        for (int p = 0; p < planes; p++){
            if (abs(allPlanes[p].point.x - pos.x) < radius){
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
    sf::RectangleShape tank;

    Tank(Vector2 pos, int rotation){
        this->tank.setOrigin(25, 25);
        this->tank.setRotation(rotation);
        this->tank.setPosition(pos.x, pos.y);

        this->tank.setFillColor(sf::Color(255, 255, 255));
        this->tank.setSize(sf::Vector2f(50,50));
    }

    void Forward(int multiplier=1){
        this->tank.setPosition(cos(this->tank.getRotation()*0.0174532777778)*multiplier+this->tank.getPosition().x,
                        sin(this->tank.getRotation()*0.0174532777778)*multiplier+this->tank.getPosition().y);
    }

    void Rotate(float rotation){
        this->tank.rotate(rotation);
    }
};

class KeySet{
public:
    int p1_horizontal = 0;
    int p1_vertical = 0;
    int p1_fire_cooldown = 10;

    int p2_horizontal = 0;
    int p2_vertical = 0;
    int p2_fire_cooldown = 10;
};

void fire_projectile(Projectile *projectile, Vector2 Pos, Vector2 Vel){
    projectile->SetPos(Pos);
    projectile->SetVel(Vel);
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
    window->draw(tank2->tank);


    window->display();
}

void FixedUpdate(Projectile AllProjectiles[], PlaneX allPlanesX[], int planesX, PlaneY allPlanesY[], int planesY, int bodies, Tank *tank1, Tank *tank2, KeySet *keys){

    int projectileLen = 16;
    while (true){
        sleep_for(5ms);

        if (keys->p1_fire_cooldown == -1){
            for (int i = 0; i <= projectileLen; i+= 2){
                if (AllProjectiles[i].available){
                    fire_projectile(&AllProjectiles[i],
                                    Vector2(tank1->tank.getPosition().x, tank1->tank.getPosition().y),
                                    Vector2(cos(tank1->tank.getRotation()*0.0174532777778)*projSpeed, sin(tank1->tank.getRotation()*0.0174532777778)*projSpeed));
                    keys->p1_fire_cooldown = 30;
                    break;
                }
            }
        }
        if (keys->p2_fire_cooldown == -1){
            for (int i = 1; i <= projectileLen; i+= 2){
                if (AllProjectiles[i].available){
                    fire_projectile(&AllProjectiles[i],
                                    Vector2(tank2->tank.getPosition().x, tank2->tank.getPosition().y),
                                    Vector2(cos(tank2->tank.getRotation()*0.0174532777778)*projSpeed, sin(tank2->tank.getRotation()*0.0174532777778)*projSpeed));
                    keys->p2_fire_cooldown = 30;
                    break;
                }
            }
        }

        for (int i = 0; i < projectileLen; i++){
            if (AllProjectiles[i].lifespan == 0){
                AllProjectiles[i] = Projectile(0, Vector2(0, 0), Vector2(-100, -100), 10, sf::Color(110, 110, 110));
            }
        }


        if (keys->p1_fire_cooldown > 0){
            keys->p1_fire_cooldown -= 1;
        }
        if (keys->p2_fire_cooldown > 0){
            keys->p2_fire_cooldown -= 1;
        }

        tank1->Forward(keys->p1_vertical);
        tank2->Forward(keys->p2_vertical);

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
            AllProjectiles[b].CheckBounceX(allPlanesX, planesX);
            AllProjectiles[b].CheckBounceY(allPlanesY, planesY);
        }
        for (int b = 0; b < bodies; b++){
            (AllProjectiles[b]).UpdatePosition();
        }
        cout << tank1->tank.getPosition().x << " " << tank1->tank.getPosition().y << "\n";
//        cout << tank2->tank.getPosition().x << " " << tank2->tank.getPosition().y << "\n";
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode(rezx, rezy), "TanksAlot");

    KeySet keys;

    // Bodies
    const int projectiles = 16;
    Projectile allProjectiles[projectiles] = {
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 10, sf::Color(110, 110, 110)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 10, sf::Color(110, 110, 110)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 10, sf::Color(110, 110, 110)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 10, sf::Color(110, 110, 110)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 10, sf::Color(110, 110, 110)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 10, sf::Color(110, 110, 110)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 10, sf::Color(110, 110, 110)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 10, sf::Color(110, 110, 110)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 10, sf::Color(110, 110, 110)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 10, sf::Color(110, 110, 110)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 10, sf::Color(110, 110, 110)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 10, sf::Color(110, 110, 110)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 10, sf::Color(110, 110, 110)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 10, sf::Color(110, 110, 110)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 10, sf::Color(110, 110, 110)),
            Projectile(0, Vector2(0, 0), Vector2(-100, -100), 10, sf::Color(110, 110, 110)),
    };

    const int planesX = 2;
    PlaneX allPlanesX[planesX] = {
            PlaneX(Vector2(0, 0), rezx),
            PlaneX(Vector2(0, rezy), rezx)
    };
    const int planesY = 2;
    PlaneY allPlanesY[planesY] = {
            PlaneY(Vector2(0, 0), rezy),
            PlaneY(Vector2(rezx, 0), rezy),
            //(*paddle1).paddle,
            //(*paddle2).paddle
    };


    Tank tank1 = Tank(Vector2(100, 100), 0);
    Tank tank2 = Tank(Vector2(50, 50), 45);


    sf::Event event{};

    std::thread thread(FixedUpdate, allProjectiles, allPlanesX, planesX, allPlanesY, planesY, projectiles, &tank1, &tank2, &keys);

    while (window.isOpen()){
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
                if (keys.p1_fire_cooldown == 0){
                    keys.p1_fire_cooldown = -1;
                }
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
                if (keys.p2_fire_cooldown == 0){
                    keys.p2_fire_cooldown = -1;
                }
            }


        }
        window.clear();
        Update(allProjectiles, allPlanesX, planesX, allPlanesY, planesY, &tank1, &tank2, projectiles, &window);
    }


    return 0;
}
