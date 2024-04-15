#include <raylib.h>
#include <iostream>
#include <vector>

using namespace std;

//Klasa lasera
class Laser
{
    private:
        Vector2 position;
        int speed;
        
    public:
        bool laserstate;
        Laser(Vector2 position, int speed)
        {
            this -> position = position;
            this -> speed = speed;
            laserstate = true;
        }
        void Draw(Color color)
        {
            if(laserstate)
                DrawRectangle(position.x, position.y, 4, 15, color);
        }
        void Moving()
        {
            position.y +=speed;
            if(laserstate)
            {
                if(position.y > GetScreenHeight() || position.y < 0)
                    laserstate = false;
            }
        }

        Rectangle HitBox()
        {
            Rectangle rect;
            rect.x = position.x;
            rect.y = position.y;
            rect.width = 4;
            rect.height = 15;
            return rect;
        }
};


//Klasa dla statku gracza
class Statek
{
    private:
    Texture2D image;
    Vector2 position;
    double timerlasera;
    public:
    vector<Laser> lasers; //vector dla laserow
    Statek()
    {
        image = LoadTexture("Textures/statek.png");
        position.x =(GetScreenWidth() - image.width)/2;
        position.y = GetScreenHeight() - 100;
        timerlasera = 0;
    }
    ~Statek()
    {
        UnloadTexture(image);
    }
    void Draw()
    {
        DrawTextureV(image, position, WHITE);
    }
    void MoveLeft()
    {
        position.x-=5;
    if(position.x<0)
        position.x=0;
    }
    void MoveRight()
    {
        position.x+=5;
    if(position.x > GetScreenWidth() - image.width)
        position.x = GetScreenWidth() - image.width;
    }
    void MoveDown()
    {
        position.y+=5;
    if(position.y>GetScreenHeight()-image.height)
        position.y=GetScreenHeight()-image.height;
    }
    void MoveUp()
    {
        position.y-=5;
    if(position.y < GetScreenHeight() - 300)
        position.y = GetScreenHeight() - 300;
    }
    void Fire()
    {
        //Sprawdzanie czasu przed nastepnym strzalem lasera
        if(GetTime() - timerlasera >= 0.5){
        lasers.push_back(Laser({position.x + image.width/2 - 2, position.y},-5));
        timerlasera = GetTime();
        }
    }

    Rectangle HitBox()
    {
        return {position.x, position.y, float(image.width), float(image.height)};
    }
};

//Klasa kosmitow
class Kosmit
{
    private:

    public:
    Texture2D kosmitImages[3] = {};
    int typ;
    Vector2 position;
    
    Kosmit(int typ, Vector2 position)
    {
        this -> typ = typ;
        this -> position = position;
        //Wgranie obrazkow kosmitow
        if(kosmitImages[typ - 1].id == 0)
        {
            switch (typ)
            {
            case 1:
                kosmitImages[0] = LoadTexture("Textures/kosmit1.png");
                break;
            case 2:
                kosmitImages[1] = LoadTexture("Textures/kosmit2.png");
                break;
            case 3:
                kosmitImages[2] = LoadTexture("Textures/kosmit3.png");
                break;
            default:
                kosmitImages[0] = LoadTexture("Textures/kosmit1.png");
                break;
            }
        }
    }

    /*void UnloadImages()
    {
        for(int i = 0; i < 4; i++)
        UnloadTexture(kosmitImages[i]);
    }
    */
   
    void Update(int kierunek)
    {
        position.x += kierunek;
    }
    void Draw()
    {
        DrawTextureV(kosmitImages[typ - 1], position, WHITE);
    }
    int GetType()
    {   
        return typ;
    }

    Rectangle HitBox()
    {
        return{position.x, position.y, float(kosmitImages[typ - 1].width), float(kosmitImages[typ - 1].height)};
    }
};
//Klasa gry
class GameLoop
{
    private:
    Statek statek;
    int kierunekKosmitow;
    vector<Laser> laserKosmitow;
    constexpr static float timerLaseraKosmitow = 0.5;
    float lastLaserKosmitow;
    int waveCount; // Licznik rund
    constexpr static int maxWaves = 2; // Maksymalna liczba rund
    int speedkosmity;

    void DeletingLasers()
    {
        //Usuwanie nieaktywnych laserow
        for(auto it = statek.lasers.begin(); it != statek.lasers.end();)
        {
            if(!it -> laserstate)
                it = statek.lasers.erase(it);
            else
                ++ it;
        }

        for(auto it = laserKosmitow.begin(); it != laserKosmitow.end();)
        {
            if(!it -> laserstate)
                it = laserKosmitow.erase(it);
            else
                ++ it;
        }
    }

    //Lasery dla kosmitow uwzgledniajac czas stralu
    void FireKosmity()
    {
        double currentTime = GetTime();
        if(currentTime - lastLaserKosmitow >= timerLaseraKosmitow && !kosmity.empty())
        {
        int randomIndex = GetRandomValue(0, kosmity.size() - 1);
        Kosmit& kosmit = kosmity[randomIndex];
        laserKosmitow.push_back(Laser({kosmit.position.x + kosmit.kosmitImages[kosmit.typ - 1].width/2, kosmit.position.y + kosmit.kosmitImages[kosmit.typ - 1].height}, 5));
        lastLaserKosmitow = GetTime();
        }
    }
    
    void SprawdzenieKolizji()
    {
        //Lasery statku

        for(auto& laser: statek.lasers)
        {
            auto it = kosmity.begin();
            while (it != kosmity.end())
            {
                if(CheckCollisionRecs(it -> HitBox(), laser.HitBox()))
                {
                    it = kosmity.erase(it);
                    laser.laserstate = false;
                }else 
                {
                    ++ it;
                }
            }
        }

        //Lasery Kosmitow

        for(auto& laser: laserKosmitow)
        {
            if(CheckCollisionRecs(laser.HitBox(), statek.HitBox()))
            {
                laser.laserstate = false;
                lives --;
                if(lives == 0)
                {
                    KoniecGry();
                }
            }
        }

        //Kolizja Kosmitow ze statkiem

        for(auto& kosmit: kosmity)
        {
            if(CheckCollisionRecs(kosmit.HitBox(), statek.HitBox()))
            {
                KoniecGry();
            }
        }
    }

    void KoniecGry()
    {
        gamestate = false;
        
    }

    
        

    public:
    //Tworzenie kosmitow 
    vector<Kosmit> kosmity;
    int lives = 3;
    bool gamestate = true;
 
    vector<Kosmit> NewKosmits()
    {
        vector<Kosmit> kosmity;
        for(int row = 0; row < 5; row++)
        {
            for(int column = 0; column < 11; column++)
            {
                int typkosmita;
                /*if(row == 0)
                    typkosmita = 3;
                    else if(row == 1 || 2)
                        typkosmita = 2;
                        else
                            typkosmita = 1;
                            */
                switch (row)
                {
                case 0:
                    typkosmita = 3;
                    break;
                case 1:
                    typkosmita = 2;
                    break;
                case 2:
                    typkosmita = 2;
                    break;
                default:
                    typkosmita = 1;
                    break;
                }

                float x =150 + column * 55;
                float y =row * 55;
                kosmity.push_back(Kosmit(typkosmita, {x, y}));
            }
        }
        return kosmity;
    }
    GameLoop()
    {
        kosmity = NewKosmits();
       speedkosmity = 2;
        kierunekKosmitow = speedkosmity;
        lastLaserKosmitow = 0.0;
        waveCount = 0;
        
    }
    ~GameLoop()
    {
        
    }

    void NewWave()
    {
        // Tworzenie nowej fali kosmitów po zniszczeniu obecnej fali
        if (kosmity.empty())
        {
            if (waveCount < maxWaves)
            {
                kosmity = NewKosmits();
                speedkosmity += 2;
                kierunekKosmitow = speedkosmity;
                lastLaserKosmitow = 0.0;
                waveCount++; // Zwiększenie licznika rund
            }
            else
            {
                KoniecGry(); // Zakończenie gry po osiągnięciu maksymalnej liczby rund
            }
        }
    }


    void Draw()
    {
        statek.Draw();

        for(auto& laser: statek.lasers)
            laser.Draw(GREEN);

        for(auto& kosmit: kosmity)
            kosmit.Draw();

        for(auto& laser: laserKosmitow)
            laser.Draw(RED);
    }
    void Update()
    {
        if(gamestate)
        {
            for(auto& laser: statek.lasers){
                laser.Moving();
            }
            DeletingLasers();
            KosmitMoving(speedkosmity);
            FireKosmity();
            for(auto& laser: laserKosmitow)
            {
                laser.Moving();
            }
            NewWave();
            SprawdzenieKolizji();
            
        }
    }

    void Sterowanie()
    {
        if(gamestate)
        {
            if(IsKeyDown(KEY_LEFT)) statek.MoveLeft();
            if(IsKeyDown(KEY_RIGHT))  statek.MoveRight();
            if(IsKeyDown(KEY_DOWN)) statek.MoveDown();
            if(IsKeyDown(KEY_UP)) statek.MoveUp();
            if(IsKeyDown(KEY_SPACE)) statek.Fire();
        }
    }

    void KosmitMovingDown(int distance)
    {
        for(auto& kosmit : kosmity){
            if(kosmit.position.y > GetScreenHeight() - kosmit.HitBox().height)
            {
                KoniecGry();
            }else
            {
                kosmit.position.y += distance;
            }
            
        }
    }

    void KosmitMoving(int speedkosmity)
    {
            for(auto& kosmit : kosmity){
                if(kosmit.position.x + kosmit.kosmitImages[kosmit.typ - 1].width > GetScreenWidth())
                {
                    kierunekKosmitow = -speedkosmity;
                    KosmitMovingDown(5);
                }
                if(kosmit.position.x < 0)
                {
                    kierunekKosmitow = speedkosmity;
                    KosmitMovingDown(5);
                }
                kosmit.Update(kierunekKosmitow);
                    }
    }
};




int main()
{
    //Window resolution
    int windowSize = 900;


    //Window Creating
    InitWindow(windowSize, windowSize, "Space Invaders");
    SetTargetFPS(60); //FPS gry

    GameLoop gameloop;
    
    // Sprawdza czy okno nie jest zamkniente
    while(WindowShouldClose() == false)
    {
        gameloop.Sterowanie();
        gameloop.Update();
        //Tworzenie obiektow
        BeginDrawing();
        //Tworzenie backgroundu
        ClearBackground(BLACK);
        
        
        
        
        
        gameloop.Draw();
        EndDrawing();
    }

    // Window closing
    CloseWindow();
   
}