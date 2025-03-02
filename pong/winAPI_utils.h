struct {
    HWND console_handle;//����� ����
    HDC device_context, context;// ��� ��������� ���������� (��� �����������)
    int width, height;//���� �������� ������� ���� ������� ������� ���������
} window;

HBRUSH cpBrush;
HBRUSH addl_cpBrush;
HFONT hFont;

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

void ShowBitmap(int x, int y, int x1, int y1, HBITMAP hBitmapBall, bool alpha = false, bool rotate = false, float angle = 0)
{
    HBITMAP hbm, hOldbm;
    HDC hMemDC;
    BITMAP bm;

    hMemDC = CreateCompatibleDC(window.context); // ������� �������� ������, ����������� � ���������� �����������
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
                StretchBlt(window.context, x, y, x1, y1, hMemDC, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY); // ������ ����������� bitmap
            }
        }

        SelectObject(hMemDC, hOldbm);// ��������������� �������� ������
    }

    DeleteDC(hMemDC); // ������� �������� ������
}

//void ProcessSound(const char* name)//������������ ���������� � ������� .wav, ���� ������ ������ � ��� �� ����� ��� � ���������
//{
//    PlaySound(TEXT(name), NULL, SND_FILENAME | SND_ASYNC);//���������� name �������� ��� �����. ���� ASYNC ��������� ����������� ���� ���������� � ����������� ���������
//}