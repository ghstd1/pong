#include <windows.h>
#include <gdiplus.h>
#include <math.h>

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




/*void DrawCircle(HDC hDC) {
    hdc = BeginPaint(hDC, &ps);
    // TODO: Add any drawing code here...
    RECT rect;
    GetClientRect(hWnd, &rect);

    HDC backbuffDC = CreateCompatibleDC(hdc);

    HBITMAP backbuffer = CreateCompatibleBitmap(hdc, rect.right, rect.bottom);

    int savedDC = SaveDC(backbuffDC);
    SelectObject(backbuffDC, backbuffer);
    HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
    FillRect(backbuffDC, &rect, hBrush);
    DeleteObject(hBrush);



    //Brush and Pen colours
    SelectObject(backbuffDC, GetStockObject(DC_BRUSH));
    SetDCBrushColor(backbuffDC, RGB(255, 0, 0));
    SelectObject(backbuffDC, GetStockObject(DC_PEN));
    SetDCPenColor(backbuffDC, RGB(0, 0, 0));



    //Shape Coordinates
    temp_shape.left = x1;
    temp_shape.top = y1;
    temp_shape.right = x2;
    temp_shape.bottom = y2;



    //Draw Old Shapes
    //Rectangles
   /* for (int i = 0; i < current_rect_count; i++)
    {
        Rectangle(backbuffDC, rect_list[i].left, rect_list[i].top, rect_list[i].right, rect_list[i].bottom);
    }
    //Ellipses
    for (int i = 0; i < current_ellipse_count; i++)
    {
        Ellipse(backbuffDC, ellipse_list[i].left, ellipse_list[i].top, ellipse_list[i].right, ellipse_list[i].bottom);
    }*/

    /*if (mouse_down)
    {
        if (drawCircle)
        {

            temp_shape.right = y1 + (x2 - x1);

            Ellipse(backbuffDC, temp_shape.left, temp_shape.top, temp_shape.right, temp_shape.bottom);
        }

        if (drawRect)
        {
            Rectangle(backbuffDC, temp_shape.left, temp_shape.top, temp_shape.right, temp_shape.bottom);
        }

        if (drawEllipse)
        {
            Ellipse(backbuffDC, temp_shape.left, temp_shape.top, temp_shape.right, temp_shape.bottom);
        }
    }

    BitBlt(hdc, 0, 0, rect.right, rect.bottom, backbuffDC, 0, 0, SRCCOPY);
    RestoreDC(backbuffDC, savedDC);

    DeleteObject(backbuffer);
    DeleteDC(backbuffDC);
    EndPaint(hWnd, &ps);
}*/

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
    float x, y, width, height, speed;
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

void ShowConsoleCursor(bool showFlag) //������� ������� �������� ������ �������
{
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);//����� ������������ ������
    CONSOLE_CURSOR_INFO     cursorInfo;//��������� ������ ���������� � �������
    GetConsoleCursorInfo(out, &cursorInfo);//�������� ���������� � ������� � ��������� CONSOLE_CURSOR_INFO
    cursorInfo.bVisible = showFlag;//�������� ��
    SetConsoleCursorInfo(out, &cursorInfo);//���������
}

void InitGame()
{
    //� ���� ������ ��������� ������� � ������� ������� gdi
    //���� ������������� - ����� ������ ������ ����� � .exe 
    //��������� ������ LoadImageA ��������� � ������� ��������, ��������� �������� ����� ������������� � ������� ���� �������
    /*ball.hBitmap = (HBITMAP)LoadImageA(NULL, "ball.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    racket.hBitmap = (HBITMAP)LoadImageA(NULL, "racket.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    enemy.hBitmap = (HBITMAP)LoadImageA(NULL, "racket_enemy.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);*/
    car.hBitmap = (HBITMAP)LoadImageA(NULL, "car.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hBack = (HBITMAP)LoadImageA(NULL, "racetrack_ps.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    //------------------------------------------------------
    
    POINT car_dms = GetBitmapDimension(car.hBitmap);
    car.width = car_dms.x / 4;
    car.height = car_dms.y / 4;
    car.x = window.width / 2;
    car.y = window.height / 2;

    srand(0);

    for (int i = 0; i < 10; i++)
    {
        checkpoint[i].x = rand() % window.width;
        checkpoint[i].y = rand() % window.height;

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

//void ShowScore()
//{
//    //�������� �������� � �������
//    SetTextColor(window.context, RGB(160,160,160));
//    SetBkColor(window.context, RGB(0,0,0));
//    auto hFont = CreateFont(70, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, 2, 0, "CALIBRI");
//    auto hTmp = (HFONT)SelectObject(window.context, hFont);
//
//    char txt[32];//����� ��� ������
//    _itoa_s(game.score, txt, 10);//�������������� �������� ���������� � �����. ����� �������� � ���������� txt
//    TextOutA(window.context, 10, 10, "Score", 5);
//    TextOutA(window.context, 200, 10, (LPCSTR)txt, strlen(txt));
//
//    _itoa_s(game.balls, txt, 10);
//    TextOutA(window.context, 10, 100, "Balls", 5);
//    TextOutA(window.context, 200, 100, (LPCSTR)txt, strlen(txt));
//}

float rotate_speed = 0;
float movement_speed = 0;

float movement_x = 0;
float movement_y = 0;

float lerp(float a, float b,  float x) {
    return a * (1-x) + b * x;
}

void ProcessInput()
{
    if (GetAsyncKeyState(VK_LEFT)) rotate_speed += 1;
    if (GetAsyncKeyState(VK_RIGHT)) rotate_speed -= 1;

    angle += rotate_speed;

    

    rotate_speed *= 0.7;
    float accel = (1 + log2(movement_speed + 1))*.1;
    if (GetAsyncKeyState(VK_UP)) movement_speed += accel;
    if (GetAsyncKeyState(VK_DOWN)) movement_speed -= accel;

    if (movement_speed < 0) movement_speed = 0;
    if (movement_speed > max_speed) movement_speed = max_speed;

    float delta_x = sin(angle * 3.141519 / 180.) * movement_speed;
    float delta_y = cos(angle * 3.141519 / 180.) * movement_speed;

    float n_spd = .95*movement_speed / max_speed;
    n_spd = pow(n_spd, 2);

    movement_x = lerp(movement_x, delta_x, 1-n_spd);
    movement_y = lerp(movement_y, delta_y, 1-n_spd);

    car.x += movement_x;
    car.y += movement_y;

    movement_speed *= 0.99;

}

void ShowBitmap(HDC hDC, int x, int y, int x1, int y1, HBITMAP hBitmapBall, bool alpha = false, bool rotate = false, float angle = 0)
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
    ShowBitmap(window.context, 0, 0, window.width, window.height, hBack);//������ ���

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

void InitWindow()
{
    ShowConsoleCursor(false);//������� ������
    window.console_handle = GetConsoleWindow();//����� ����� ������ ����
    RECT r;//���������� ���� �������������
    GetClientRect(window.console_handle, &r);//��������� � ��� ������� ���������� ����� ����
    window.device_context = GetDC(window.console_handle);//�� ������ ���� ������� ����� ��������� ���������� ��� ���������
    window.width = r.right - r.left;//���������� ������� � ���������
    window.height = r.bottom - r.top;
    window.context = CreateCompatibleDC(window.device_context);//������ �����
    SelectObject(window.context, CreateCompatibleBitmap(window.device_context, window.width, window.height));//����������� ���� � ���������
}

int main()
{
    SetProcessDPIAware();
    InitWindow();//����� �������������� ��� ��� ����� ��� ��������� � ����
    InitGame();//����� �������������� ���������� ����

    while (true)
    {
        ShowRacketAndBall();//������ ���, ������� � �����
        //ShowScore();//������ ���� � �����
        RotateFutureCar();
        BitBlt(window.device_context, 0, 0, window.width, window.height, window.context, 0, 0, SRCCOPY);//�������� ����� � ����
        Sleep(16);//���� 16 ���������� (1/���������� ������ � �������)

        ProcessInput();//����� ����������
        //LimitRacket();//���������, ����� ������� �� ������� �� �����
        //ProcessBall();//���������� �����
        //ProcessRoom();//������������ ������� �� ���� � �������, ��������� ������ � ��������
    }
}
