//linker::system::subsystem  - Windows(/ SUBSYSTEM:WINDOWS)
//configuration::advanced::character set - not set
//linker::input::additional dependensies Msimg32.lib; Winmm.lib
#define _CRT_SECURE_NO_WARNINGS

#include "windows.h"

#include <windows.h>
#include <gdiplus.h>
#include <math.h>

int timeout = 10000;
int time;
int currenttime=0;
int starttime = 0;

const int checkpointNum = 19;
POINT checkpoint[checkpointNum];

struct {
    HWND console_handle;//хэндл окна
    HDC device_context, context;// два контекста устройства (для буферизации)
    int width, height;//сюда сохраним размеры окна которое создаст программа
} window;

void ShowBitmap(int x, int y, int x1, int y1, HBITMAP hBitmapBall, bool alpha = false, bool rotate = false, float angle = 0);

float lerp(float a, float b, float x) {
    return a * (1 - x) + b * x;
}

void RotatedBlt(HDC hDC, HDC hMemDC, float x, float y, float width, float height, float angle, float bitmapWidth, float bitmapHeight)
{
    angle *= 3.141519 / 180.;
    POINT point[3];
    point[1].x = -width / 2.; point[1].y = -height / 2.;
    point[0].x = width / 2; point[0].y = -height / 2.;
    point[2].x = width / 2; point[2].y = height / 2.;

    for (int i = 0; i < 3; i++)
    {
        float x1 = point[i].y * sin(angle) - point[i].x * cos(angle);
        float y1 = point[i].x * sin(angle) + point[i].y * cos(angle);
        point[i].x = x + x1 + width / 2.;
        point[i].y = y + y1 + height / 2.;
    }

    PlgBlt(hDC, point, hMemDC, 0, 0, bitmapWidth, bitmapHeight, NULL, 0, 0);
}

HBITMAP LoadBMP(const char* name)
{
    return (HBITMAP)LoadImageA(NULL, name, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
}

POINT GetBitmapDimension(HBITMAP bmp) {
    BITMAP bm;
    GetObject(bmp, (int)sizeof bm, &bm);
    POINT dms;
    dms.x = bm.bmWidth;
    dms.y = bm.bmHeight;

    return dms;
}


struct sprite_ {
    float x, y, width, height, speed,angle;
    HBITMAP hBitmap;//хэндл к спрайту шарика

    void Load(const char* name)
    {
        hBitmap = LoadBMP(name);
    }

    void Show()
    {
        ShowBitmap(x - width / 2., y - height / 2, width, height, hBitmap, false, true, angle);
    }

    void SetDimentionsFromBMP()
    {
        POINT dim = GetBitmapDimension(hBitmap);
        width = dim.x;
        height = dim.y;
    }
};

struct car_ {

    sprite_ sprite;

    float rotate_speed = 0;
    float movement_speed = 0;
    float movement_x = 0;
    float movement_y = 0;
    float max_speed = 7;

    void setAngle(float a)
    {
        sprite.angle = a;
    }

    void init()
    {
        sprite.Load("car.bmp");
        sprite.SetDimentionsFromBMP();
        sprite.width /= 4;
        sprite.height /= 4;
        sprite.x = checkpoint[0].x + 30;
        sprite.y = checkpoint[0].y + 10;
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

car_ car;







struct {
    int score, balls;//количество набранных очков и оставшихся "жизней"
    bool action = false;//состояние - ожидание (игрок должен нажать пробел) или игра
} game;



HBITMAP hBack;// хэндл для фонового изображения


int nextCP_num = 0;

HBRUSH cpBrush;
HBRUSH addl_cpBrush;


//cекция кода

POINT track_dms;
HDC testDC;
HDC hTrackDC;


void InitGame()
{
    //в этой секции загружаем спрайты с помощью функций gdi
    //пути относительные - файлы должны лежать рядом с .exe 
    //результат работы LoadImageA сохраняет в хэндлах битмапов, рисование спрайтов будет произовдиться с помощью этих хэндлов

    hBack = LoadBMP("racetrack_ps.bmp");
    track_dms = GetBitmapDimension(hBack);

    SetWindowPos(window.console_handle, HWND_TOPMOST, 0, 0, track_dms.x, track_dms.y, SWP_NOMOVE);
    window.width = track_dms.x;
    window.height = track_dms.y;

    
    //checkpoints
    checkpoint[0].x = 501; checkpoint[0].y = 795; //501, 795
    checkpoint[1].x = 410; checkpoint[1].y = 733; //410, 733
    checkpoint[2].x = 237; checkpoint[2].y = 520; //237, 520
    checkpoint[3].x = 270; checkpoint[3].y = 429; //270, 429
    checkpoint[4].x = 445; checkpoint[4].y = 381; //445, 381

    checkpoint[5].x = 446; checkpoint[5].y = 301; //446, 301
    checkpoint[6].x = 139; checkpoint[6].y = 272; //139, 272
    checkpoint[7].x = 84;  checkpoint[7].y = 193; //84, 193
    checkpoint[8].x = 128; checkpoint[8].y = 121; //128, 121
    checkpoint[9].x = 463; checkpoint[9].y = 111; //463, 111

    checkpoint[10].x = 538; checkpoint[10].y = 226; //538, 226
    checkpoint[11].x = 707; checkpoint[11].y = 109; //707, 109
    checkpoint[12].x = 756; checkpoint[12].y = 182; //756, 182
    checkpoint[13].x = 706; checkpoint[13].y = 273; //706, 273
    checkpoint[14].x = 592; checkpoint[14].y = 361; //592, 361

    checkpoint[15].x = 560; checkpoint[15].y = 425; //560, 425
    checkpoint[16].x = 560; checkpoint[16].y = 548; //560, 548
    checkpoint[17].x = 560; checkpoint[17].y = 548; //611, 709
    checkpoint[18].x = 587; checkpoint[18].y = 779; //587, 779

    //car
    car.init();


}

//void ProcessSound(const char* name)//проигрывание аудиофайла в формате .wav, файл должен лежать в той же папке где и программа
//{
//    PlaySound(TEXT(name), NULL, SND_FILENAME | SND_ASYNC);//переменная name содежрит имя файла. флаг ASYNC позволяет проигрывать звук паралельно с исполнением программы
//}

int carPos_color[3];

void ShowScore()
{
    //поиграем шрифтами и цветами
    SetTextColor(window.context, RGB(160, 160, 160));
    SetBkColor(window.context, RGB(0, 0, 0));
    auto hFont = CreateFont(70, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "CALIBRI");
    auto hTmp = (HFONT)SelectObject(window.context, hFont);

    char txt[256];//буфер для текста
    char chkpts[] = "Chkpts ";
    char you_won[] = "YOU WON";
    _itoa_s(nextCP_num, txt, 10);//преобразование числовой переменной в текст. текст окажется в переменной txt

    char outstr[255];
    strcpy(outstr, chkpts);
    strcat(outstr, txt);
    strcat(outstr, "/18");
    TextOutA(window.context, 10, 10, outstr, strlen(outstr));


    char time_txt[256];
    _itoa(time, time_txt, 10);
    TextOutA(window.context, 400, 10, time_txt, strlen(time_txt));

    if (nextCP_num > 18) {
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

void ShowBitmap(int x, int y, int x1, int y1, HBITMAP hBitmapBall, bool alpha, bool rotate, float angle)
{
    HBITMAP hbm, hOldbm;
    HDC hMemDC;
    BITMAP bm;

    hMemDC = CreateCompatibleDC(window.context); // Создаем контекст памяти, совместимый с контекстом отображения
    hOldbm = (HBITMAP)SelectObject(hMemDC, hBitmapBall);// Выбираем изображение bitmap в контекст памяти

    if (hOldbm) // Если не было ошибок, продолжаем работу
    {
        GetObject(hBitmapBall, sizeof(BITMAP), (LPSTR)&bm); // Определяем размеры изображения

        if (rotate) {

            RotatedBlt(window.context, hMemDC, x, y, x1, y1, angle, x1, y1);

        }
        else {
            if (alpha)
            {
                TransparentBlt(window.context, x, y, x1, y1, hMemDC, 0, 0, x1, y1, RGB(0, 0, 0));//все пиксели черного цвета будут интепретированы как прозрачные
            }
            else
            {
                StretchBlt(window.context, x, y, x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY); // Рисуем изображение bitmap
            }
        }

        SelectObject(hMemDC, hOldbm);// Восстанавливаем контекст памяти
    }

    DeleteDC(hMemDC); // Удаляем контекст памяти
}

void controlChekpoint()
{
    float coordXdelta = checkpoint[nextCP_num].x - car.sprite.x;
    float coordYdelta = checkpoint[nextCP_num].y - car.sprite.y;

    float length = sqrt(coordXdelta * coordXdelta + coordYdelta * coordYdelta);

    float margin = 19;
    if (length < margin) {

        timeout += 5000;

        nextCP_num++;
    }
}

void showCheckpoint()
{
    if (!addl_cpBrush) addl_cpBrush = CreateSolidBrush(RGB(100, 0, 0));
    SelectObject(window.context, addl_cpBrush);
    float addl_rad = 20;
    Ellipse(window.context, checkpoint[nextCP_num].x - addl_rad, checkpoint[nextCP_num].y - addl_rad, checkpoint[nextCP_num].x + addl_rad, checkpoint[nextCP_num].y + addl_rad);

    if (!cpBrush) cpBrush = CreateSolidBrush(RGB(100, 100, 0));
    SelectObject(window.context, cpBrush);

    for (int i = 0; i < checkpointNum; i++)
    {
        float rad = 5;
        Ellipse(window.context, checkpoint[i].x - rad, checkpoint[i].y - rad, checkpoint[i].x + rad, checkpoint[i].y + rad);
    }
}

void showScene()
{
    ShowBitmap(0, 0, track_dms.x, track_dms.y, hBack);//задний фон
    showCheckpoint();
    controlChekpoint();
    car.sprite.Show();
}



//-------
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
        ShowScore();//рисуем очик и жизни
        
        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//копируем буфер в окно
        Sleep(16);//ждем 16 милисекунд (1/количество кадров в секунду)

        ProcessInput();//опрос клавиатуры
        car.processMovement();
    }

}
