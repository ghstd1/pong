//linker::system::subsystem  - Windows(/ SUBSYSTEM:WINDOWS)
//configuration::advanced::character set - not set
//linker::input::additional dependensies Msimg32.lib; Winmm.lib

#include "windows.h"

#include <windows.h>
#include <gdiplus.h>
#include <math.h>

void ShowBitmap(HDC hDC, int x, int y, int x1, int y1, HBITMAP hBitmapBall, bool alpha = false, bool rotate = false, float angle = 0);

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


POINT GetBitmapDimension(HBITMAP bmp) {
    BITMAP bm;
    GetObject(bmp, (int)sizeof bm, &bm);
    POINT dms;
    dms.x = bm.bmWidth;
    dms.y = bm.bmHeight;

    return dms;
}

// секция данных игры
//struct {
//    float x, y, width, height, speed;
//    HBITMAP hBitmap;//хэндл к спрайту ракетки
//} racket;
//
//struct {
//    float x;
//    HBITMAP hBitmap;//хэндл к спрайту ракетки противника
//} enemy;
//
//struct {
//    float x, y, rad, dx, dy, speed;
//    HBITMAP hBitmap;//хэндл к спрайту шарика
//} ball;

struct {
    float x        , y, width, height, speed;
    HBITMAP hBitmap;//хэндл к спрайту шарика
} car;

struct {
    int score, balls;//количество набранных очков и оставшихся "жизней"
    bool action = false;//состояние - ожидание (игрок должен нажать пробел) или игра
} game;

struct {
    HWND console_handle;//хэндл окна
    HDC device_context, context;// два контекста устройства (для буферизации)
    int width, height;//сюда сохраним размеры окна которое создаст программа
} window;

HBITMAP hBack;// хэндл для фонового изображения

float angle = 180;
float max_speed = 7;

int nextCP_num = 0;

HBRUSH cpBrush;
HBRUSH addl_cpBrush;

POINT checkpoint[10];
//cекция кода

POINT track_dms;
HDC testDC;
HDC hTrackDC;


void InitGame()
{
    //в этой секции загружаем спрайты с помощью функций gdi
    //пути относительные - файлы должны лежать рядом с .exe 
    //результат работы LoadImageA сохраняет в хэндлах битмапов, рисование спрайтов будет произовдиться с помощью этих хэндлов
    car.hBitmap = (HBITMAP)LoadImageA(NULL, "car.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hBack = (HBITMAP)LoadImageA(NULL, "racetrack_ps.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    track_dms = GetBitmapDimension(hBack);
    SetWindowPos(window.console_handle, HWND_TOPMOST, 0, 0, track_dms.x, track_dms.y, SWP_NOMOVE);
    window.width = track_dms.x;
    window.height = track_dms.y;

    testDC = GetDC(window.console_handle);
    ShowBitmap(testDC, 0, 0, track_dms.x, track_dms.y, hBack);

    //hTrackDC = (HBITMAP)SelectObject(testDC, hBack);// Выбираем изображение bitmap в контекст памяти
    
    //------------------------------------------------------

    POINT car_dms = GetBitmapDimension(car.hBitmap);
    car.width = car_dms.x / 4;
    car.height = car_dms.y / 4;
    car.x = window.width / 2;
    car.y = window.height / 2;

    srand(0);

    for (int i = 0; i < 10; i++)
    {
        checkpoint[i].x = rand() % track_dms.x;
        checkpoint[i].y = rand() % track_dms.y;

    }

    //racket.width = 300;
    //racket.height = 120;
    //racket.speed = 30;//скорость перемещения ракетки
    //racket.x = window.width / 2.;//ракетка посередине окна
    //racket.y = window.height - racket.height;//чуть выше низа экрана - на высоту ракетки

    //enemy.x = racket.x;//х координату оппонета ставим в ту же точку что и игрока

    //ball.dy = (rand() % 65 + 35) / 100.;//формируем вектор полета шарика
    //ball.dx = -(1 - ball.dy);//формируем вектор полета шарика
    //ball.speed = 11;
    //ball.rad = 20;
    //ball.x = racket.x;//x координата шарика - на середие ракетки
    //ball.y = racket.y - ball.rad;//шарик лежит сверху ракетки
    //

    //game.score = 0;
    //game.balls = 9;

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

    char txt[32];//буфер для текста
    _itoa_s(nextCP_num, txt, 10);//преобразование числовой переменной в текст. текст окажется в переменной txt
    TextOutA(window.context, 10, 10, "Chkpts.", 5);
    TextOutA(window.context, 200, 10, (LPCSTR)txt, strlen(txt));
    TextOutA(window.context, 230, 10, "/10", 5);

    if (nextCP_num > 10) {
        //TextOutA(window.context, window.width / 2, window.height / 2, "YOU WIN!", 5);
    }


    char txt2[32];
    _itoa_s(carPos_color[0], txt2, 10);
    TextOutA(window.context, 230, 110, txt2, 5);


    //_itoa_s(game.balls, txt, 10);
    //TextOutA(window.context, 10, 100, "Balls", 5);
    //TextOutA(window.context, 200, 100, (LPCSTR)txt, strlen(txt));
}

float rotate_speed = 0;
float movement_speed = 0;

float movement_x = 0;
float movement_y = 0;

float lerp(float a, float b, float x) {
    return a * (1 - x) + b * x;
}

void ProcessInput()
{
    if (GetAsyncKeyState(VK_LEFT)) rotate_speed += 1;
    if (GetAsyncKeyState(VK_RIGHT)) rotate_speed -= 1;

    angle += rotate_speed;

    rotate_speed *= 0.7;
    float accel = (1 + log2(movement_speed + 1)) * .1;
    if (GetAsyncKeyState(VK_UP)) movement_speed += accel;
    if (GetAsyncKeyState(VK_DOWN)) movement_speed -= accel;

    if (movement_speed < 0) movement_speed = 0;
    if (movement_speed > max_speed) movement_speed = max_speed;

    float delta_x = sin(angle * 3.141519 / 180.) * movement_speed;
    float delta_y = cos(angle * 3.141519 / 180.) * movement_speed;

    float n_spd = .95 * movement_speed / max_speed;
    n_spd = pow(n_spd, 2);

    movement_x = lerp(movement_x, delta_x, 1 - n_spd);
    movement_y = lerp(movement_y, delta_y, 1 - n_spd);

    car.x += movement_x;
    car.y += movement_y;

    

    POINT p;
    GetCursorPos(&p);
    ScreenToClient(window.console_handle,&p);
    //auto carPos_pixel = GetPixel(testDC, p.x, p.y);
    auto carPos_pixel = GetPixel(testDC, car.x-car.width/2., car.y-car.height/2.);
    
    carPos_color[0] = GetRValue(carPos_pixel);
    carPos_color[1] = GetGValue(carPos_pixel);
    carPos_color[2] = GetBValue(carPos_pixel);

    if (carPos_color[0] * carPos_color[1] * carPos_color[1] == 0)
    {
        movement_speed *= 0.59;
    }
    else
    {
        movement_speed *= 0.99;
    }

}

void ShowBitmap(HDC hDC, int x, int y, int x1, int y1, HBITMAP hBitmapBall, bool alpha, bool rotate, float angle)
{
    HBITMAP hbm, hOldbm;
    HDC hMemDC;
    BITMAP bm;

    hMemDC = CreateCompatibleDC(hDC); // Создаем контекст памяти, совместимый с контекстом отображения
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
                StretchBlt(hDC, x, y, x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY); // Рисуем изображение bitmap
            }
        }

        SelectObject(hMemDC, hOldbm);// Восстанавливаем контекст памяти
    }

    DeleteDC(hMemDC); // Удаляем контекст памяти
}

void ShowRacketAndBall()
{
    ShowBitmap(window.context, 0, 0, track_dms.x, track_dms.y, hBack);//задний фон

    if (!addl_cpBrush) addl_cpBrush = CreateSolidBrush(RGB(100, 0, 0));
    SelectObject(window.context, addl_cpBrush);
    float addl_rad = 20;
    Ellipse(window.context, checkpoint[nextCP_num].x - addl_rad, checkpoint[nextCP_num].y - addl_rad, checkpoint[nextCP_num].x + addl_rad, checkpoint[nextCP_num].y + addl_rad);

    if (!cpBrush) cpBrush = CreateSolidBrush(RGB(100, 100, 0));
    SelectObject(window.context, cpBrush);

    for (int i = 0; i < 10; i++)
    {
        float rad = 5;
        Ellipse(window.context, checkpoint[i].x - rad, checkpoint[i].y - rad, checkpoint[i].x + rad, checkpoint[i].y + rad);
    }


    float coordXdelta = checkpoint[nextCP_num].x - car.x;
    float coordYdelta = checkpoint[nextCP_num].y - car.y;

    float length = sqrt(coordXdelta * coordXdelta + coordYdelta * coordYdelta);


    if (length < 10) {
        nextCP_num++;
    }
}

void RotateFutureCar() {
    ShowBitmap(window.context, car.x - car.width / 2., car.y - car.height / 2, car.width, car.height, car.hBitmap, false, true, angle);
    //angle += 1;
    //RotatedBlt(window.device_context, test.hBitmap, test.x, test.y, test.width, test.height, 3.0, 256.0, 256.0);
}

//void LimitRacket() 
//{
//    racket.x = max(racket.x, racket.width / 2.);//если коодината левого угла ракетки меньше нуля, присвоим ей ноль
//    racket.x = min(racket.x, window.width - racket.width / 2.);//аналогично для правого угла
//}
//
//void CheckWalls()
//{
//    if (ball.x < ball.rad || ball.x > window.width - ball.rad)
//    {
//        ball.dx *= -1;
//        ProcessSound("bounce.wav");
//    }
//}
//
//void CheckRoof()
//{
//    if (ball.y < ball.rad+racket.height)
//    {
//        ball.dy *= -1;
//        ProcessSound("bounce.wav");
//    }
//}
//
//bool tail = false;
//
//void CheckFloor()
//{
//    if (ball.y > window.height - ball.rad - racket.height)//шарик пересек линию отскока - горизонталь ракетки
//    {
//        if (!tail && ball.x >= racket.x - racket.width / 2.-ball.rad && ball.x <= racket.x + racket.width / 2.+ball.rad)//шарик отбит, и мы не в режиме обработки хвоста
//        {
//            game.score++;//за каждое отбитие даем одно очко
//            ball.speed += 5. / game.score;//но увеличиваем сложность - прибавляем скорости шарику
//            ball.dy *= -1;//отскок
//            racket.width -= 10. / game.score;//дополнительно уменьшаем ширину ракетки - для сложности
//            ProcessSound("bounce.wav");//играем звук отскока
//        }
//        else
//        {//шарик не отбит
//
//            tail = true;//дадим шарику упасть ниже ракетки
//
//            if (ball.y -ball.rad> window.height)//если шарик ушел за пределы окна
//            {
//                game.balls--;//уменьшаем количество "жизней"
//
//                    ProcessSound("fail.wav");//играем звук
//
//                    if (game.balls < 0) { //проверка условия окончания "жизней"
//
//                        MessageBoxA(window.console_handle, "game over", "", MB_OK);//выводим сообщение о проигрыше
//                        InitGame();//переинициализируем игру
//                    }
//
//                ball.dy = (rand() % 65 + 35) / 100.;//задаем новый случайный вектор для шарика
//                ball.dx = -(1 - ball.dy);
//                ball.x = racket.x;//инициализируем координаты шарика - ставим его на ракетку
//                ball.y = racket.y - ball.rad;
//                game.action = false;//приостанавливаем игру, пока игрок не нажмет пробел
//                tail = false;
//            }
//        }
//    }
//}
//
//void ProcessRoom()
//{
//    //обрабатываем стены, потолок и пол. принцип - угол падения равен углу отражения, а значит, для отскока мы можем просто инвертировать часть вектора движения шарика
//    CheckWalls();
//    CheckRoof();
//    CheckFloor();
//}
//
//void ProcessBall()
//{
//    if (game.action)
//    {
//        //если игра в активном режиме - перемещаем шарик
//        ball.x += ball.dx * ball.speed;
//        ball.y += ball.dy * ball.speed;
//    }
//    else
//    {
//        //иначе - шарик "приклеен" к ракетке
//        ball.x = racket.x;
//    }
//}


//-------
void InitWindow()
{
    SetProcessDPIAware();
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
    
    InitWindow();//здесь инициализируем все что нужно для рисования в окне
    InitGame();//здесь инициализируем переменные игры

    mciSendString(TEXT("play ..\\Debug\\music.mp3 repeat"), NULL, 0, NULL);
    //ShowCursor(NULL);
    
    while (!GetAsyncKeyState(VK_ESCAPE))
    {
        ShowRacketAndBall();//рисуем фон, ракетку и шарик
        ShowScore();//рисуем очик и жизни
        RotateFutureCar();
        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//копируем буфер в окно
        Sleep(16);//ждем 16 милисекунд (1/количество кадров в секунду)

        ProcessInput();//опрос клавиатуры
        //LimitRacket();//проверяем, чтобы ракетка не убежала за экран
        //ProcessBall();//перемещаем шарик
        //ProcessRoom();//обрабатываем отскоки от стен и каретки, попадание шарика в картетку
    }

}
