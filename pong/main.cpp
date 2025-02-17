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

// ������ ������ ����
//struct {
//    float x, y, width, height, speed;
//    HBITMAP hBitmap;//����� � ������� �������
//} racket;
//
//struct {
//    float x;
//    HBITMAP hBitmap;//����� � ������� ������� ����������
//} enemy;
//
//struct {
//    float x, y, rad, dx, dy, speed;
//    HBITMAP hBitmap;//����� � ������� ������
//} ball;

struct {
    float x        , y, width, height, speed;
    HBITMAP hBitmap;//����� � ������� ������
} car;

struct {
    int score, balls;//���������� ��������� ����� � ���������� "������"
    bool action = false;//��������� - �������� (����� ������ ������ ������) ��� ����
} game;

struct {
    HWND console_handle;//����� ����
    HDC device_context, context;// ��� ��������� ���������� (��� �����������)
    int width, height;//���� �������� ������� ���� ������� ������� ���������
} window;

HBITMAP hBack;// ����� ��� �������� �����������

float angle = 180;
float max_speed = 7;

int nextCP_num = 0;

HBRUSH cpBrush;
HBRUSH addl_cpBrush;

POINT checkpoint[10];
//c����� ����

POINT track_dms;
HDC testDC;
HDC hTrackDC;


void InitGame()
{
    //� ���� ������ ��������� ������� � ������� ������� gdi
    //���� ������������� - ����� ������ ������ ����� � .exe 
    //��������� ������ LoadImageA ��������� � ������� ��������, ��������� �������� ����� ������������� � ������� ���� �������
    car.hBitmap = (HBITMAP)LoadImageA(NULL, "car.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hBack = (HBITMAP)LoadImageA(NULL, "racetrack_ps.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    track_dms = GetBitmapDimension(hBack);
    SetWindowPos(window.console_handle, HWND_TOPMOST, 0, 0, track_dms.x, track_dms.y, SWP_NOMOVE);
    window.width = track_dms.x;
    window.height = track_dms.y;

    testDC = GetDC(window.console_handle);
    ShowBitmap(testDC, 0, 0, track_dms.x, track_dms.y, hBack);

    //hTrackDC = (HBITMAP)SelectObject(testDC, hBack);// �������� ����������� bitmap � �������� ������
    
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
    //racket.speed = 30;//�������� ����������� �������
    //racket.x = window.width / 2.;//������� ���������� ����
    //racket.y = window.height - racket.height;//���� ���� ���� ������ - �� ������ �������

    //enemy.x = racket.x;//� ���������� �������� ������ � �� �� ����� ��� � ������

    //ball.dy = (rand() % 65 + 35) / 100.;//��������� ������ ������ ������
    //ball.dx = -(1 - ball.dy);//��������� ������ ������ ������
    //ball.speed = 11;
    //ball.rad = 20;
    //ball.x = racket.x;//x ���������� ������ - �� ������� �������
    //ball.y = racket.y - ball.rad;//����� ����� ������ �������
    //

    //game.score = 0;
    //game.balls = 9;

}

//void ProcessSound(const char* name)//������������ ���������� � ������� .wav, ���� ������ ������ � ��� �� ����� ��� � ���������
//{
//    PlaySound(TEXT(name), NULL, SND_FILENAME | SND_ASYNC);//���������� name �������� ��� �����. ���� ASYNC ��������� ����������� ���� ���������� � ����������� ���������
//}

int carPos_color[3];

void ShowScore()
{
    //�������� �������� � �������
    SetTextColor(window.context, RGB(160, 160, 160));
    SetBkColor(window.context, RGB(0, 0, 0));
    auto hFont = CreateFont(70, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "CALIBRI");
    auto hTmp = (HFONT)SelectObject(window.context, hFont);

    char txt[32];//����� ��� ������
    _itoa_s(nextCP_num, txt, 10);//�������������� �������� ���������� � �����. ����� �������� � ���������� txt
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

    hMemDC = CreateCompatibleDC(hDC); // ������� �������� ������, ����������� � ���������� �����������
    hOldbm = (HBITMAP)SelectObject(hMemDC, hBitmapBall);// �������� ����������� bitmap � �������� ������

    if (hOldbm) // ���� �� ���� ������, ���������� ������
    {
        GetObject(hBitmapBall, sizeof(BITMAP), (LPSTR)&bm); // ���������� ������� �����������

        if (rotate) {

            RotatedBlt(window.context, hMemDC, x, y, x1, y1, angle, x1, y1);

        }
        else {
            if (alpha)
            {
                TransparentBlt(window.context, x, y, x1, y1, hMemDC, 0, 0, x1, y1, RGB(0, 0, 0));//��� ������� ������� ����� ����� ��������������� ��� ����������
            }
            else
            {
                StretchBlt(hDC, x, y, x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY); // ������ ����������� bitmap
            }
        }

        SelectObject(hMemDC, hOldbm);// ��������������� �������� ������
    }

    DeleteDC(hMemDC); // ������� �������� ������
}

void ShowRacketAndBall()
{
    ShowBitmap(window.context, 0, 0, track_dms.x, track_dms.y, hBack);//������ ���

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
//    racket.x = max(racket.x, racket.width / 2.);//���� ��������� ������ ���� ������� ������ ����, �������� �� ����
//    racket.x = min(racket.x, window.width - racket.width / 2.);//���������� ��� ������� ����
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
//    if (ball.y > window.height - ball.rad - racket.height)//����� ������� ����� ������� - ����������� �������
//    {
//        if (!tail && ball.x >= racket.x - racket.width / 2.-ball.rad && ball.x <= racket.x + racket.width / 2.+ball.rad)//����� �����, � �� �� � ������ ��������� ������
//        {
//            game.score++;//�� ������ ������� ���� ���� ����
//            ball.speed += 5. / game.score;//�� ����������� ��������� - ���������� �������� ������
//            ball.dy *= -1;//������
//            racket.width -= 10. / game.score;//������������� ��������� ������ ������� - ��� ���������
//            ProcessSound("bounce.wav");//������ ���� �������
//        }
//        else
//        {//����� �� �����
//
//            tail = true;//����� ������ ������ ���� �������
//
//            if (ball.y -ball.rad> window.height)//���� ����� ���� �� ������� ����
//            {
//                game.balls--;//��������� ���������� "������"
//
//                    ProcessSound("fail.wav");//������ ����
//
//                    if (game.balls < 0) { //�������� ������� ��������� "������"
//
//                        MessageBoxA(window.console_handle, "game over", "", MB_OK);//������� ��������� � ���������
//                        InitGame();//������������������ ����
//                    }
//
//                ball.dy = (rand() % 65 + 35) / 100.;//������ ����� ��������� ������ ��� ������
//                ball.dx = -(1 - ball.dy);
//                ball.x = racket.x;//�������������� ���������� ������ - ������ ��� �� �������
//                ball.y = racket.y - ball.rad;
//                game.action = false;//���������������� ����, ���� ����� �� ������ ������
//                tail = false;
//            }
//        }
//    }
//}
//
//void ProcessRoom()
//{
//    //������������ �����, ������� � ���. ������� - ���� ������� ����� ���� ���������, � ������, ��� ������� �� ����� ������ ������������� ����� ������� �������� ������
//    CheckWalls();
//    CheckRoof();
//    CheckFloor();
//}
//
//void ProcessBall()
//{
//    if (game.action)
//    {
//        //���� ���� � �������� ������ - ���������� �����
//        ball.x += ball.dx * ball.speed;
//        ball.y += ball.dy * ball.speed;
//    }
//    else
//    {
//        //����� - ����� "��������" � �������
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
    window.device_context = GetDC(window.console_handle);//�� ������ ���� ������� ����� ��������� ���������� ��� ���������
    window.width = r.right - r.left;//���������� ������� � ���������
    window.height = r.bottom - r.top;
    window.context = CreateCompatibleDC(window.device_context);//������ �����
    SelectObject(window.context, CreateCompatibleBitmap(window.device_context, window.width, window.height));//����������� ���� � ���������
    GetClientRect(window.console_handle, &r);

}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    
    InitWindow();//����� �������������� ��� ��� ����� ��� ��������� � ����
    InitGame();//����� �������������� ���������� ����

    mciSendString(TEXT("play ..\\Debug\\music.mp3 repeat"), NULL, 0, NULL);
    //ShowCursor(NULL);
    
    while (!GetAsyncKeyState(VK_ESCAPE))
    {
        ShowRacketAndBall();//������ ���, ������� � �����
        ShowScore();//������ ���� � �����
        RotateFutureCar();
        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//�������� ����� � ����
        Sleep(16);//���� 16 ���������� (1/���������� ������ � �������)

        ProcessInput();//����� ����������
        //LimitRacket();//���������, ����� ������� �� ������� �� �����
        //ProcessBall();//���������� �����
        //ProcessRoom();//������������ ������� �� ���� � �������, ��������� ������ � ��������
    }

}
