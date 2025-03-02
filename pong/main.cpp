//linker::system::subsystem  - Windows(/ SUBSYSTEM:WINDOWS)
//configuration::advanced::character set - not set
//linker::input::additional dependensies Msimg32.lib; Winmm.lib
#define _CRT_SECURE_NO_WARNINGS

#include "windows.h"

#include <windows.h>
#include <gdiplus.h>
#include <math.h>
#include <vector>

#include "winAPI_utils.h"

int timeout = 10000;
int time;
int currenttime=0;
int starttime = 0;

int carPos_color[3];

const int checkpointNum = 19;


float lerp(float a, float b, float x) {
    return a * (1 - x) + b * x;
}


struct sprite_ {
    float x, y, width, height, speed,angle;
    HBITMAP hBitmap;//хэндл к спрайту шарика

    void Load(const char* name)
    {
        hBitmap = LoadBMP(name);
    }

    void Show(bool centered = true)
    {
        ShowBitmap(x - (centered ? width / 2. : 0.), y - (centered ? height / 2 : 0.), width, height, hBitmap, false, true, angle);
    }

    void SetDimentionsFromBMP()
    {
        POINT dim = GetBitmapDimension(hBitmap);
        width = dim.x;
        height = dim.y;
    }
};

struct checkPoint_ {

    int nextCP_num = 0;

    const POINT list[checkpointNum] = 
    {
        501, 795, //501, 795
        410, 733, //410, 733
        237, 520, //237, 520
        270, 429, //270, 429
        445, 381, //445, 381

        446, 301, //446, 301
        139, 272, //139, 272
        84, 193, //84, 193
        128, 121, //128, 121
        463, 111, //463, 111

         538,226, //538, 226
         707,109, //707, 109
         756,182, //756, 182
         706,273, //706, 273
         592,361, //592, 361

         560,425, //560, 425
         560,548, //560, 548
         560,548, //611, 709
         587,779 //587, 779
     };

    void Show()
    {
        if (!addl_cpBrush) addl_cpBrush = CreateSolidBrush(RGB(100, 0, 0));
        SelectObject(window.context, addl_cpBrush);
        float addl_rad = 20;
        Ellipse(window.context, list[nextCP_num].x - addl_rad, list[nextCP_num].y - addl_rad, list[nextCP_num].x + addl_rad, list[nextCP_num].y + addl_rad);

        if (!cpBrush) cpBrush = CreateSolidBrush(RGB(100, 100, 0));
        SelectObject(window.context, cpBrush);

        for (int i = 0; i < checkpointNum; i++)
        {
            float rad = 5;
            Ellipse(window.context, list[i].x - rad, list[i].y - rad, list[i].x + rad, list[i].y + rad);
        }
    }

    void Control(float x, float y)
    {
        float coordXdelta = list[nextCP_num].x - x;
        float coordYdelta = list[nextCP_num].y - y;

        float length = sqrt(coordXdelta * coordXdelta + coordYdelta * coordYdelta);

        float margin = 19;
        if (length < margin) {

            timeout += 5000;

            nextCP_num++;
        }
    }

};

struct track_ {
    sprite_ sprite;
    checkPoint_ checkPoint;

    void init() {
        sprite.Load("racetrack_ps.bmp");
        sprite.SetDimentionsFromBMP();
    }
};

track_ track;

class car_ {
public:
    sprite_ sprite;

    float rotate_speed = 0;
    float movement_speed = 0;
    float movement_x = 0;
    float movement_y = 0;
    float max_speed = 7;

     car_(const char* name) {
        sprite.Load(name);
        sprite.SetDimentionsFromBMP();
        sprite.width /= 4;
        sprite.height /= 4;
        sprite.x = track.checkPoint.list[0].x + 30;
        sprite.y = track.checkPoint.list[0].y + 10;
    }

    void setAngle(float a)
    {
        sprite.angle = a;
    }


    void processMovement()
    {
        rotate_speed *= 0.7;

        if (movement_speed < 0) movement_speed = 0;
        if (movement_speed > max_speed) movement_speed = max_speed;

        float delta_x = sin(sprite.angle * 3.141519 / 180.) * movement_speed;
        float delta_y = cos(sprite.angle * 3.141519 / 180.) * movement_speed;

        float n_spd = .95 * movement_speed / max_speed;
        n_spd = pow(n_spd, 2);

        movement_x = lerp(movement_x, delta_x, 1 - n_spd);
        movement_y = lerp(movement_y, delta_y, 1 - n_spd);

        sprite.x += movement_x;
        sprite.y += movement_y;

        movement_speed *= 0.91;
    }

} ;


car_ car("car.bmp");

void InitGame()
{
    track.init();
    SetWindowPos(window.console_handle, HWND_TOPMOST, 0, 0, track.sprite.width, track.sprite.height, SWP_NOMOVE);

    
    window.width = track.sprite.width;
    window.height = track.sprite.height;

    
}

void ShowScore()
{
    //поиграем шрифтами и цветами
    SetTextColor(window.context, RGB(160, 160, 160));
    SetBkColor(window.context, RGB(0, 0, 0));
    if (!hFont) hFont = CreateFont(70, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "CALIBRI");
    auto hTmp = (HFONT)SelectObject(window.context, hFont);

    char txt[256];//буфер для текста
    char chkpts[] = "Chkpts ";
    char you_won[] = "YOU WON";
    _itoa_s(track.checkPoint.nextCP_num, txt, 10);//преобразование числовой переменной в текст. текст окажется в переменной txt

    char outstr[255];
    strcpy(outstr, chkpts);
    strcat(outstr, txt);
    strcat(outstr, "/18");
    TextOutA(window.context, 10, 10, outstr, strlen(outstr));


    char time_txt[256];
    _itoa(time, time_txt, 10);
    TextOutA(window.context, 400, 10, time_txt, strlen(time_txt));

    if (track.checkPoint.nextCP_num > 18) {
        TextOutA(window.context, window.width / 2, window.height / 2, you_won, 7);
    }
}

void ProcessInput()
{
    if (GetAsyncKeyState(VK_LEFT)) car.rotate_speed += 1;
    if (GetAsyncKeyState(VK_RIGHT)) car.rotate_speed -= 1;
    car.setAngle(car.sprite.angle + car.rotate_speed);
     
    float accel = (1 + log2(car.movement_speed + 1)) * .1;
    if (GetAsyncKeyState(VK_UP)) car.movement_speed += accel;
    if (GetAsyncKeyState(VK_DOWN)) car.movement_speed -= accel;


    //slow down if outside of track
    //POINT p;
    //GetCursorPos(&p);
    //ScreenToClient(window.console_handle,&p);
    ////auto carPos_pixel = GetPixel(testDC, p.x, p.y);
    //auto carPos_pixel = GetPixel(testDC, car.x-car.width/2., car.y-car.height/2.);
    //
    //carPos_color[0] = GetRValue(carPos_pixel);
    //carPos_color[1] = GetGValue(carPos_pixel);
    //carPos_color[2] = GetBValue(carPos_pixel);

    //if (carPos_color[0] * carPos_color[1] * carPos_color[1] == 0)
    //{
    //    movement_speed *= 0.59;
    //}
    //else
    //{
    //}

}

void showScene()
{
    track.sprite.Show(false);
    track.checkPoint.Show();
    track.checkPoint.Control(car.sprite.x, car.sprite.y);
    car.sprite.Show();
}

void InitWindow()
{
    window.console_handle = CreateWindow("edit", 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0);
    
    RECT r;
    GetClientRect(window.console_handle, &r);
    window.device_context = GetDC(window.console_handle);//из хэндла окна достаем хэндл контекста устройства для рисования
    window.width = r.right - r.left;//определяем размеры и сохраняем
    window.height = r.bottom - r.top;
    window.context = CreateCompatibleDC(window.device_context);//второй буфер
    SelectObject(window.context, CreateCompatibleBitmap(window.device_context, window.width, window.height));//привязываем окно к контексту
    GetClientRect(window.console_handle, &r);

}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    SetProcessDPIAware();
    InitWindow();//здесь инициализируем все что нужно для рисования в окне
    InitGame();//здесь инициализируем переменные игры

    //mciSendString(TEXT("play ..\\Debug\\music.mp3 repeat"), NULL, 0, NULL);
    //ShowCursor(NULL);
    
    starttime = timeGetTime();

    while (!GetAsyncKeyState(VK_ESCAPE))
    {
        currenttime = timeGetTime()-starttime;
        time = timeout - currenttime;
        showScene();//рисуем фон, ракетку и шарик
        //ShowScore();//рисуем очик и жизни
        
        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//копируем буфер в окно
        Sleep(16);//ждем 16 милисекунд (1/количество кадров в секунду)

        ProcessInput();//опрос клавиатуры
        car.processMovement();
    }

}
